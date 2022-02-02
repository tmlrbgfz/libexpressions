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
#ifndef __LIBEXPRESSIONS_AST__
#define __LIBEXPRESSIONS_AST__

#include <string>
#include <vector>
#include <variant>

//#include <boost/fusion/include/adapt_struct.hpp>
//#include <boost/variant/recursive_wrapper.hpp>

namespace libexpressions::parsers {

template<typename T>
using AtomicProposition = T;

template<typename T>
struct Operator_t;

template<typename T>
using Operator = struct Operator_t<T>;

template<typename T>
using Operand = std::variant<AtomicProposition<T>, Operator<T>>;

template<typename T>
struct Operator_t {
    std::vector<Operand<T>> operands;
};

template<typename T>
using Expression = Operand<T>;

}

// BOOST_FUSION_ADAPT_TPL_STRUCT(
//     (T),
//     (struct libexpressions::parsers::Operator_t)(T),
//     (libexpressions::parsers::OperandWrapper<T>, thisOperator)
//     (std::vector<libexpressions::parsers::Operand<T>>, operands)
// )

#endif //__LIBEXPRESSIONS_AST__

