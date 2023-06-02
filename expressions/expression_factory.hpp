/* MIT License
 * 
 * Copyright (c) 2022 Niklas Krafczyk
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#pragma once

#include <type_traits>
#include <memory>
#include <map>
#include <set>
#include <iostream>

#include "libexpressions/expressions/expression_node.hpp"
#include "libexpressions/expressions/atom.hpp"
#include "libexpressions/expressions/operator.hpp"
#include "libexpressions/expressions/expression_visit_helper.hpp"
#include "libexpressions/iht/iht_factory.hpp"
#include "libexpressions/utils/variadic-insert.hpp"
#include "libexpressions/utils/trie_node.hpp"
#include "libexpressions/utils/tree-visit.hpp"
#include "libexpressions/utils/expression-tree-visit.hpp"

namespace libexpressions {
    class ExpressionFactory {
    private:
        IHT::IHTFactory<ExpressionNode> *factory;

        // A family of functions to create an operand vector for a list of operands given as a list of arguments to the function
        // Enable this function for cases where the first two arguments are Iterators
        // This function is enabled for cases where the first two arguments are Iterators
        template<typename IIterator1, typename IIterator2, typename ...Args, typename = std::enable_if_t<
            std::conjunction_v<
                std::is_same<typename std::decay<decltype(*std::declval<IIterator1>())>::type, ExpressionNodePtr>,
                std::is_same<typename std::decay<decltype(*std::declval<IIterator2>())>::type, ExpressionNodePtr>
            >>>
        std::vector<ExpressionNodePtr> getOperandVector(IIterator1 &&first, IIterator2 &&last, Args&&... args) {
            std::vector<ExpressionNodePtr> operands(std::forward<IIterator1>(first), std::forward<IIterator2>(last));
            auto further = getOperandVector(std::forward<Args>(args)...);
            operands.insert(operands.end(), further.begin(), further.end());
            return operands;
        }
        // This function is used for cases where the first argument is an expression node pointer
        template<typename ...Args>
        std::vector<ExpressionNodePtr> getOperandVector(ExpressionNodePtr operand, Args&&... args) {
            std::vector<ExpressionNodePtr> operands;
            operands.emplace_back(operand);
            auto further = getOperandVector(std::forward<Args>(args)...);
            operands.insert(operands.end(), further.begin(), further.end());
            return operands;
        }
        // This function is used for cases where the first argument is a string
        template<typename ...Args>
        std::vector<ExpressionNodePtr> getOperandVector(std::string operand, Args&&... args) {
            std::vector<ExpressionNodePtr> operands;
            operands.emplace_back(std::static_pointer_cast<ExpressionNode const>(factory->createNode<Atom>(operand)));
            auto further = getOperandVector(std::forward<Args>(args)...);
            operands.insert(operands.end(), further.begin(), further.end());
            return operands;
        }
        // Slightly optimized version for adding whole containers.
        // This does not copy by using the iterators but simply copies the
        // whole vector and then appends to it.
        template<typename ...Args>
        std::vector<ExpressionNodePtr> getOperandVector(std::vector<ExpressionNodePtr> operands, Args&&... args) {
            auto further = getOperandVector(std::forward<Args>(args)...);
            operands.insert(operands.end(), further.begin(), further.end());
            return operands;
        }
        // This function is used for no arguments
        std::vector<ExpressionNodePtr> getOperandVector() {
            return {};
        }
    public:
        static ExpressionFactory * get() {
            static std::unique_ptr<ExpressionFactory> singleton(new ExpressionFactory(IHT::IHTFactory<ExpressionNode>::get()));
            return singleton.get();
        }

        ExpressionFactory(IHT::IHTFactory<ExpressionNode> *paramFactory) : factory(paramFactory) {}

        // Creates an expression composed of other expressions, i.e. an
        // operator. If the expressions given as arguments were produced with a
        // different underlying IHT factory, the behaviour of other operations
        // on the resulting expression and on expressions in which the
        // resulting expression is a subexpression is undefined. Most
        // importantly, comparisons for equality are effected, as
        // `exp1->equal_to(exp2) != (exp1 == exp2)` if `exp1` and `exp2` were
        // produced with different underlying IHT factories.
        template<typename ...Args>
        ExpressionNodePtr makeExpression(Args&&... args) {
            auto operandVector = getOperandVector(std::forward<Args>(args)...);
            return std::static_pointer_cast<ExpressionNode const>(
                factory->createNode<Operator>(std::move(operandVector))
            );
        }
        ExpressionNodePtr makeIdentifier(std::string const &arg) {
            return std::static_pointer_cast<ExpressionNode const>(
                factory->createNode<Atom>(arg));
        }
        ExpressionNodePtr makeNewIdentifier(std::string const &prefix) {
            std::optional<IHT::IHTNodePtr<ExpressionNode>> node;
            std::string id = prefix;
            size_t suffix = 0;
            do {
                node = factory->tryCreateNewNode<Atom>(id);
                if(not node.has_value()) {
                    id = prefix + "_" + std::to_string(suffix++);
                    //id = prefix + std::to_string(std::hash<std::string>{}(id));
                }
            } while(not node.has_value()); 
            return std::static_pointer_cast<ExpressionNode const>(node.value());
        }

        ExpressionNodePtr reproduceExpressionInThisFactory(ExpressionNodePtr const &expressionToReproduce) {
            if(this->factory->hasEquivalentNode(expressionToReproduce)) {
                return expressionToReproduce;
            }
            TrieNode<Operator::PathElement, ExpressionNodePtr> data;

            class ExpressionReproductionVisitor {
            private:
                ExpressionFactory *factory;
                TrieNode<Operator::PathElement, ExpressionNodePtr>::const_iterator begin;
                TrieNode<Operator::PathElement, ExpressionNodePtr>::const_iterator end;
            public:
                ExpressionReproductionVisitor(ExpressionFactory *factoryToUse, TrieNode<Operator::PathElement, ExpressionNodePtr>::const_iterator iterBegin, TrieNode<Operator::PathElement, ExpressionNodePtr>::const_iterator iterEnd)
                : factory(factoryToUse), begin(iterBegin), end(iterEnd) { }
                ExpressionNodePtr operator()(Atom const *atom) {
                    return factory->makeIdentifier(atom->getSymbol());
                }
                ExpressionNodePtr operator()(Operator const *op) {
                    std::vector<ExpressionNodePtr> operands;
                    std::transform(begin, end, std::back_inserter(operands), [](auto const &element) {
                        return std::get<std::unique_ptr<TrieNode<Operator::PathElement, ExpressionNodePtr>>>(element)->value();
                    });
                    assert(op->getSize() == operands.size());
                    return factory->makeExpression(operands);
                }
            };

            auto reproducer = [&data,factory=this](auto const &node, auto const &path) {
                auto &trieNode = data[path];
                ExpressionReproductionVisitor visitor(factory, trieNode.begin(), trieNode.end());
                trieNode = libexpressions::visit(node.get(), visitor);
            };
            traverseTree<TreeTraversalOrder::POSTFIX_TRAVERSAL>(getChildrenIteratorsForExpressionNode,
                                                                reproducer,
                                                                expressionToReproduce);
            assert(data.value() == expressionToReproduce);
            return data.value();
        }

        std::optional<ExpressionNodePtr> tryReproduceExpressionInThisFactory(ExpressionNodePtr const &expressionToReproduce) {
            if(this->factory->hasEquivalentNode(expressionToReproduce)) {
                return std::nullopt;
            } else {
                return reproduceExpressionInThisFactory(expressionToReproduce);
            }
        }

        ExpressionNodePtr modifyExpression(ExpressionNodePtr const &expressionToModify, std::map<Operator::Path, ExpressionNodePtr> const &modificationMap) {
            // If there are no modifications or there are no modifications that
            // address any nodes existing within the expression, the code below
            // does not produce the desired results. We need to catch these
            // situations.
            if(modificationMap.empty()) {
                return expressionToModify;
            } else {
                for(auto const &[path, _] : modificationMap) {
                    // Any path that does not lead to a node in the expression is invalid.
                    if(Operator::followPath(expressionToModify, path) == nullptr) {
                        throw std::runtime_error("Invalid path given for expression modification.");
                    }
                }
            }

            // The actual work is happening here
            // Get the positions of the modifications in a Trie
            TrieNode<Operator::PathElement, ExpressionNodePtr> data;
            for(auto const &[positionToModify, modification] : modificationMap) {
                data[positionToModify] = modification;
            }
            // Traverse the expression to modify and change expressions if and only if
            // - There is a node (modification) at the same position in the Trie or
            // - A child node of the Trie node corresponding to the current node contains a node (modification)
            traverseTree<TreeTraversalOrder::POSTFIX_TRAVERSAL>(getChildrenIteratorsForExpressionNode,
                                            [&data,this](auto const &node, auto const &path){
                                                if(not data[path].hasValue()) {
                                                    if(std::none_of(data[path].begin(), data[path].end(), [](auto const &mapElement){
                                                        return std::get<std::unique_ptr<TrieNode<Operator::PathElement, ExpressionNodePtr>>>(mapElement)->hasValue();
                                                    })) {
                                                        data[path] = node;
                                                    } else {
                                                        assert(dynamic_cast<Operator const*>(node.get()) != nullptr);
                                                        assert(dynamic_cast<Operator const*>(node.get())->getSize() == data[path].size());
                                                        std::vector<ExpressionNodePtr> operands(data[path].size());
                                                        for(auto &[idx, ptrToData] : data[path]) {
                                                            operands.at(idx) = ptrToData->value();
                                                        }
                                                        data[path] = this->makeExpression(operands);
                                                    }
                                                } else {
                                                    // Do nothing if there is already a value assigned. This means, this node is to be modified.
                                                }
                                            },
                                            expressionToModify);
            assert(data.hasValue());
            return data.value();
        }
    };
}

