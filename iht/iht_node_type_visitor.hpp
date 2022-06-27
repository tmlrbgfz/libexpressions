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
#include <gsl/gsl_assert>

#include "iht_node.hpp"

namespace IHT {
    template<typename T, typename Fn, typename Node>
    struct cast_and_call_impl_t {
        auto operator()(Fn &&f, Node const *node) const
        -> typename std::result_of<Fn(T const*)>::type {
            Expects(dynamic_cast<T const*>(node));
            return f(dynamic_cast<T const*>(node));
        }
    };
    template<typename T, typename Fn, typename Node>
    using cast_and_call = struct cast_and_call_impl_t<T, Fn, Node>;

    // This function shall determine the Kind of the Node given as the second
    // parameter and shall use cast_and_call to cast the node and call the
    // function given as the first argument with it. If the signature of the
    // function given as the first argument is already determined, this will
    // not change the called function. However, 
    template<typename Fn, class NodeType>
    auto visit(Fn &&f, NodeType const *node) -> typename std::result_of<Fn(NodeType const *)>::type;
}

