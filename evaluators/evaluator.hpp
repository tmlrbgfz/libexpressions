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

#include "libexpressions/expressions/expression_node.hpp"
#include "libexpressions/expressions/atom.hpp"
#include "libexpressions/expressions/operator.hpp"
#include "libexpressions/expressions/expression_visit_helper.hpp"
#include "libexpressions/utils/expression-tree-visit.hpp"
#include "libexpressions/utils/trie_node.hpp"

namespace libexpressions {
    template<typename TypeRepresentationType, typename ValueRepresentationType>
    class Semantics {
    public:
        typedef std::tuple<TypeRepresentationType, ValueRepresentationType> EvaluationState;
    public:
        virtual EvaluationState evaluateNonOperatorTerminal(std::string const &c) = 0;
        virtual EvaluationState evaluateOperatorTerminal(std::string const &c) = 0;
        virtual EvaluationState evaluateOperator(EvaluationState const &op, std::vector<EvaluationState> const &operands) = 0;
    };

    template<typename TypeRepresentationType, typename ValueRepresentationType>
    typename Semantics<TypeRepresentationType, ValueRepresentationType>::EvaluationState
    evaluateExpression(libexpressions::ExpressionNodePtr const &node, Semantics<TypeRepresentationType, ValueRepresentationType> &semantics) {
        typedef Operator::Path Path;
        using Semantics = Semantics<TypeRepresentationType, ValueRepresentationType>;
        using TrieNode = TrieNode<Path::value_type, typename Semantics::EvaluationState>;

        class ExpressionNodeEvaluationVisitor {
        private:
            Path const &position;
            TrieNode const &values;
            Semantics &semantics;
        public:
            ExpressionNodeEvaluationVisitor(Path const &pos, TrieNode const &data, Semantics &sem)
             : position(pos), values(data), semantics(sem) { }
            typename Semantics::EvaluationState operator()(libexpressions::Operator const *opNode) {
                auto &op = values.at(0).value();
                std::vector<typename Semantics::EvaluationState> operands;
                for(size_t idx = 1; idx < opNode->getSize(); ++idx) {
                    operands.push_back(values.at(idx).value());
                }
                return semantics.evaluateOperator(op, operands);
            }
            typename Semantics::EvaluationState operator()(libexpressions::Atom const *atom) {
                if(not position.empty() and position.back() == 0) {
                    return semantics.evaluateOperatorTerminal(atom->toString());
                } else {
                    return semantics.evaluateNonOperatorTerminal(atom->toString());
                }
            }
        };

        TrieNode evaluationResults;
        auto evaluate = [&evaluationResults,&semantics](auto const &nodeInLambda, auto const &path) {
            evaluationResults[path] = libexpressions::visit(nodeInLambda.get(), ExpressionNodeEvaluationVisitor{path, evaluationResults[path], semantics});
            return evaluationResults.at(path);
        };
        traverseTree<TreeTraversalOrder::POSTFIX_TRAVERSAL>(getChildrenIteratorsForExpressionNode,
                                                            evaluate,
                                                            node);
        return evaluationResults.value();
    }
}

