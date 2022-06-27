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

#include <gsl/gsl_assert>
#include <type_traits>

#include "libexpressions/expressions/atom.hpp"
#include "libexpressions/expressions/operator.hpp"
#include "libexpressions/expressions/expression_node_kind.hpp"

namespace libexpressions {
    template<typename T, typename Fn, typename Node>
    auto cast_and_call(Fn &&f, Node const *node)
    -> std::result_of_t<Fn(T const*)> {
        Expects(dynamic_cast<T const*>(node) != nullptr);
        return f(dynamic_cast<T const*>(node));
    }

    // This function shall determine the Kind of the Node given as the first
    // parameter and shall use cast_and_call to cast the node and call the
    // function given as the second argument with it.
    template<typename Fn>
    auto visit(libexpressions::ExpressionNode const *node, Fn &&f)
      -> std::result_of_t<Fn(libexpressions::Operator const *)> {
        auto kind = node->getKind();
        Expects(kind == libexpressions::ExpressionNodeKind::EXPRESSION_OPERATOR or
                kind == libexpressions::ExpressionNodeKind::EXPRESSION_ATOM);
        switch(kind) {
        case libexpressions::ExpressionNodeKind::EXPRESSION_OPERATOR:
            return cast_and_call<typename libexpressions::Operator, Fn, libexpressions::ExpressionNode>(std::forward<Fn>(f), node);
            break;
        case libexpressions::ExpressionNodeKind::EXPRESSION_ATOM:
            return cast_and_call<typename libexpressions::Atom, Fn, libexpressions::ExpressionNode>(std::forward<Fn>(f), node);
            break;
        case libexpressions::ExpressionNodeKind::LAST_EXPRESSION_NODE:
        default:
            throw std::runtime_error("Visiting an object of invalid type");
        }
    }
}

