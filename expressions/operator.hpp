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

#include <memory>
#include <vector>
#include <iterator>

#include "libexpressions/expressions/expression_node.hpp"
#include "libexpressions/iht/iht_factory.hpp"

namespace libexpressions {
    typedef std::vector<ExpressionNodePtr> OperandContainer;

    //Associativity to be defined by the application/theory
    class Operator final : public ExpressionNode {
        friend class IHT::IHTFactory<ExpressionNode>;
    public:
        typedef libexpressions::OperandContainer OperandContainer;
        typedef OperandContainer::const_iterator Iterator;
        typedef size_t PathElement;
        typedef std::vector<PathElement> Path;
    private:
        OperandContainer const operands;
        IHT::hash_type const hashCache;
    protected:
        Operator(OperandContainer &&paramOperands)
            : ExpressionNode(ExpressionNodeKind::EXPRESSION_OPERATOR),
              operands(std::move(paramOperands)),
              hashCache([this]() {
                  size_t result = 0;
                  size_t maxLowestByte = result & 0xFFu;
                  for(auto const &operand : this->operands) {
                      size_t opHash = operand->hash();
                      result ^= opHash;
                      maxLowestByte = std::max(maxLowestByte, opHash & 0xFFu);
                  }
                  result = (result & ~0xFFu) | ((maxLowestByte + 1) & 0xFFu);
                  return result;
              }()) { }
    public:
        virtual ~Operator() = default;

        size_t getSize() const {
            return this->operands.size();
        }

        ExpressionNodePtr const &getOperator() const {
            return this->operands.front();
        }

        OperandContainer const &getOperands() const {
            return operands;
        }

        IHT::hash_type hash() const {
            return hashCache;
        }

        std::string toString() const {
            std::string result = "(";
            if(this->operands.size() > 0) {
                auto iter = this->operands.cbegin();
                result += (*iter)->toString();
                while(++iter != this->operands.cend()) {
                    result += std::string(" ") + (*iter)->toString();
                }
            }
            result += ")";
            return result;
        }

        bool equal_to(ExpressionNode const *other) const {
            if(Operator::classof(other)) {
                Operator const *op = static_cast<Operator const *>(other);
                if(this->operands.size() == op->operands.size()) {
                    for(decltype(this->operands.size()) i = 0; i < this->operands.size(); ++i) {
                        if(!this->operands.at(i)->equal_to(op->operands.at(i).get())) {
                            return false;
                        }
                    }
                    return true;
                }
            }
            return false;
        }

        static constexpr bool classof(ExpressionNode const *node) {
            return node->getKind() == ExpressionNodeKind::EXPRESSION_OPERATOR;
        }

        Iterator begin() const {
            return this->operands.begin();
        }
        Iterator end() const {
            return this->operands.end();
        }
        Iterator cbegin() const {
            return this->operands.cbegin();
        }
        Iterator cend() const {
            return this->operands.cend();
        }

    public:
        static ExpressionNodePtr followPath(ExpressionNodePtr const &ptr, Operator::Path const &path) {
            ExpressionNodePtr current = ptr;
            for(Operator::Path::const_iterator iter = path.begin(); iter != path.end(); ++iter) {
                if(not dynamic_cast<Operator const*>(current.get())) {
                    return nullptr;
                } else {
                    Operator const *op = dynamic_cast<Operator const *>(current.get());
                    if(op->getOperands().size() > *iter) {
                        current = op->getOperands().at((*iter));
                    } else {
                        return nullptr;
                    }
                }
            }
            return current;
        }
    };

    typedef std::shared_ptr<Operator> OperatorPtr;
}

