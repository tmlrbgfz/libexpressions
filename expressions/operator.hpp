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
#ifndef __LIBEXPRESSIONS_OPERATOR__
#define __LIBEXPRESSIONS_OPERATOR__

#include <memory>
#include <vector>
#include <iterator>
#include <llvm/Support/Casting.h>

#include "libexpressions/expressions/expression_node.hpp"
#include "libexpressions/iht/iht_factory.hpp"

namespace libexpressions {
    typedef std::vector<ExpressionNodePtr> OperandContainer;

    //Associativity to be defined by the application/theory
    class Operator final : public ExpressionNode {
        friend class IHT::IHTFactory<ExpressionNode>;
    public:
        typedef libexpressions::OperandContainer OperandContainer;

        class Iterator {
        private:
            Operator const *operatr;
            size_t index;
        public:
            Iterator(Operator const *paramOperatr = nullptr, size_t paramIndex = 0)
              : operatr(paramOperatr), index(paramIndex) {}
            Iterator(Iterator const &) = default;
            Iterator(Iterator &&) = default;
            ~Iterator() = default;

            Iterator operator++(int) {
                return Iterator(operatr, index++);
            }
            Iterator& operator++() {
                ++index;
                return *this;
            }
            Iterator operator+(ssize_t offset) {
                if(offset >= 0) {
                    return Iterator(operatr, index+static_cast<size_t>(offset));
                } else {
                    auto negatedOffset = static_cast<size_t>(-offset);
                    return Iterator(operatr, index - negatedOffset);
                }
            }
            Iterator operator+(size_t offset) {
                return Iterator(operatr, index+offset);
            }
            Iterator& operator+=(ssize_t offset) {
                return *this = this->operator+(offset);
            }
            Iterator operator-(ssize_t offset) {
                return this->operator+(-offset);
            }
            ssize_t operator-(Iterator const &other) {
                return static_cast<ssize_t>(this->index - other.index);
            }
            Iterator operator-=(ssize_t offset) {
                return *this = this->operator-(offset);
            }
            Iterator operator--(int) {
                return Iterator(operatr, index--);
            }
            Iterator& operator--() {
                --index;
                return *this;
            }
            ExpressionNodePtr const &operator*() const {
                if(index == 0) {
                    return operatr->nodeOperator;
                } else {
                    return operatr->operands.at(index-1);
                }
            }
            Iterator& operator=(Iterator const &other) {
                operatr = other.operatr;
                index = other.index;
                return *this;
            }

            bool operator==(Iterator const &other) const {
                return operatr == other.operatr and
                       index == other.index;
            }
            bool operator!=(Iterator const &other) const {
                return not (*this == other);
            }

            friend void swap(Iterator &iter1, Iterator &iter2) {
                using namespace std;
                swap(iter1.operatr, iter2.operatr);
                swap(iter1.index, iter2.index);
            }
        public:
            typedef ssize_t difference_type;
            typedef ExpressionNodePtr value_type;
            typedef void pointer;
            typedef ExpressionNodePtr const& reference;
            typedef std::random_access_iterator_tag iterator_category;
        };

        typedef size_t PathElement;
        typedef std::vector<PathElement> Path;
    private:
        ExpressionNodePtr const nodeOperator;
        OperandContainer const operands;
        IHT::hash_type const hashCache;
    protected:
        Operator(ExpressionNodePtr const &strOperator, OperandContainer &&paramOperands)
            : ExpressionNode(ExpressionNodeKind::EXPRESSION_OPERATOR),
              nodeOperator(strOperator),
              operands(std::move(paramOperands)),
              hashCache([this]() {
                  size_t result = this->nodeOperator->hash();
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
        ~Operator() = default;

        ExpressionNodePtr const &getOperator() const {
            return nodeOperator;
        }

        OperandContainer const &getOperands() const {
            return operands;
        }

        IHT::hash_type hash() const {
            return hashCache;
        }

        std::string toString() const {
            std::string result = "(" + nodeOperator->toString();
            for(auto &operand : this->operands) {
                result += std::string(" ") + operand->toString();
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
                    return this->nodeOperator->equal_to(op->nodeOperator.get());
                }
            }
            return false;
        }

        static constexpr bool classof(ExpressionNode const *node) {
            return node->getKind() == ExpressionNodeKind::EXPRESSION_OPERATOR;
        }

        Iterator begin() const {
            return Iterator(this, 0);
        }
        Iterator end() const {
            return Iterator(this, operands.size()+1);
        }
        Iterator cbegin() const {
            return Iterator(this, 0);
        }
        Iterator cend() const {
            return Iterator(this, operands.size()+1);
        }

    public:
        ExpressionNodePtr followPath(ExpressionNodePtr const &ptr, Operator::Path const &path) {
            ExpressionNodePtr current = ptr;
            for(Operator::Path::const_iterator iter = path.begin(); iter != path.end(); ++iter) {
                if(not llvm::isa<Operator>(current.get())) {
                    return nullptr;
                } else {
                    Operator const *op = llvm::dyn_cast<Operator const>(current.get());
                    if(op->getOperands().size() <= *iter) {
                        if(*iter == 0) {
                            current = op->getOperator();
                        } else {
                            current = op->getOperands().at((*iter)-1);
                        }
                    }
                }
            }
            return current;
        }
    };

    typedef std::shared_ptr<Operator> OperatorPtr;
}

#endif //__LIBEXPRESSIONS_OPERATOR__

