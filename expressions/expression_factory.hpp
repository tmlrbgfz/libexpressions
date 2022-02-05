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
#include <gsl/gsl_assert>

#include "libexpressions/expressions/expression_node.hpp"
#include "libexpressions/expressions/atom.hpp"
#include "libexpressions/expressions/operator.hpp"
#include "libexpressions/iht/iht_factory.hpp"
#include "libexpressions/utils/variadic-insert.hpp"

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

        template<typename ...Args>
        ExpressionNodePtr makeExpression(Args&&... args) {
            auto operandVector = getOperandVector(std::forward<Args>(args)...);
            if(operandVector.size() == 1) {
                return operandVector.front();
            } else {
                return std::static_pointer_cast<ExpressionNode const>(
                    factory->createNode<Operator>(std::move(operandVector))
                );
            }
        }
    };
}

