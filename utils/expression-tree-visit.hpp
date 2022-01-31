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
#ifndef __LIBEXPRESSIONS_EXPRESSION_TREE_VISIT__
#define __LIBEXPRESSIONS_EXPRESSION_TREE_VISIT__

#include <iterator>

#include "libexpressions/utils/tree-visit.hpp"
#include "libexpressions/expressions/operator.hpp"

template<>
size_t getChildNodeIndex(libexpressions::ExpressionNodePtr const *parent, libexpressions::ExpressionNodePtr const *child) {
    libexpressions::Operator const *op = llvm::dyn_cast<libexpressions::Operator const>(parent->get());
    Expects(op != nullptr); // ptr is a child of previous node. Thus, previous node must have children. Thus, previous node must be an operator.
    auto iter = std::find(op->begin(), op->end(), *child);
    Expects(op->end() != iter); // We expect to find some child equal to ptr
    return static_cast<size_t>(std::distance(op->begin(), iter));
}

#endif //__LIBEXPRESSIONS_EXPRESSION_TREE_VISIT__

