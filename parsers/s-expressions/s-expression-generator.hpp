/*
 * MIT License
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

#include <algorithm>

#include "libexpressions/utils/tree-visit.hpp"
#include "libexpressions/parsers/ast.hpp"

namespace libexpressions {

template<>
size_t getChildNodeIndex(libexpressions::parsers::Operand<std::string> const *parent, libexpressions::parsers::Operand<std::string> const *child) {
    auto const &operands = std::get<libexpressions::parsers::Operator<std::string>>(*parent).operands;
    return static_cast<size_t>(std::distance(operands.cbegin(), std::find(operands.cbegin(), operands.cend(), *child)));
}

namespace parsers {

std::string sExpressionToString(libexpressions::parsers::Expression<std::string> const &input) {
    std::vector<std::string> resultStack;
    struct {
        typedef std::vector<Operand<std::string>>::const_iterator Iterator;
        std::tuple<Iterator, Iterator> operator()(Operator<std::string> const &op) const {
            return { op.operands.cbegin(), op.operands.cend() };
        }
        std::tuple<Iterator, Iterator> operator()(AtomicProposition<std::string> const &/*at*/) const {
            return { Iterator(), Iterator() };
        }
        std::tuple<Iterator, Iterator> operator()(Operand<std::string> const &op) const {
            return std::visit(*this, op);
        }
    } childGetter;
    auto printer = [&resultStack](auto const &node, auto const &/*path*/){
        class printerHelper {
            public:
            std::vector<std::string> &resultStack;
            printerHelper(std::vector<std::string> &result) : resultStack(result) { }
            void operator()(Operator<std::string> const &op) const {
                auto const numOfOperands = op.operands.size();
                std::string result = "(";
                if(numOfOperands > 0) {
                    auto const offset = static_cast<ssize_t>(resultStack.size() - numOfOperands);
                    auto const iterBeginningOfOperandRange = resultStack.cbegin() + offset;
                    auto iter = iterBeginningOfOperandRange;
                    result += *iter;
                    while(++iter != resultStack.cend()) {
                        result += " ";
                        result += *iter;
                    }
                    resultStack.erase(iterBeginningOfOperandRange, resultStack.cend());
                }
                result += ")";
                resultStack.push_back(std::move(result));
            }
            void operator()(AtomicProposition<std::string> const &atom) const {
                resultStack.push_back(atom);
            }
        } printerObj(resultStack);
        return std::visit(printerObj, node);
    };
    traverseTree<TreeTraversalOrder::POSTFIX_TRAVERSAL>(childGetter, printer, static_cast<Operand<std::string>>(input));
    return resultStack.back();
}

}
}

