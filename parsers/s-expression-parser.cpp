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
#include "libexpressions/parsers/s-expression-parser.hpp"

#include <boost/spirit/include/support_info.hpp>

#include <boost/spirit/include/karma_grammar.hpp>
#include <boost/spirit/include/karma_alternative.hpp>
#include <boost/spirit/include/karma_sequence.hpp>
#include <boost/spirit/include/karma_plus.hpp>
#include <boost/spirit/include/karma_optional.hpp>
#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/karma_auto.hpp>
#include <vector>

#include "libexpressions/parsers/ast.hpp"
#include "libexpressions/expressions/expression_node.hpp"
#include "libexpressions/expressions/atom.hpp"
#include "libexpressions/expressions/operator.hpp"
#include "libexpressions/parsers/ASTConversion.hpp"

using namespace libexpressions::parsers;

namespace boost::spirit::traits {

template<typename T>
struct extract_from_attribute<Operator<T>, struct Operator_t<T>>
{
    typedef struct Operator_t<T> const &type;

    template <typename Context>
    static type call(Operator<T> const& attr, Context& /*context*/) {
        return attr.value();
    }
};
template<typename T>
struct assign_to_attribute_from_value<Operator<T>, Operand<T>>
{
    static void call(Operand<T> const &val, Operator<T> &attr) {
        attr = std::get<Operator<T>>(val);
    }
};
template<typename T>
struct assign_to_attribute_from_value<Operator<T>, std::vector<Operand<T>>>
{
    static void call(std::vector<Operand<T>> const &val, Operator<T> &attr) {
        attr.operands = val;
    }
};
template<typename T>
struct extract_from_attribute<Operand<T>, Operator<T>>
{
    typedef Operator<T> const &type;

    template <typename Context>
    static type call(Operand<T> const& attr, Context& /*context*/) {
        return std::get<Operator<T>>(attr);
    }
};
template<typename T>
struct extract_from_attribute<Operand<T>, AtomicProposition<T>>
{
    typedef AtomicProposition<T> const &type;

    template <typename Context>
    static type call(Operand<T> const& attr, Context& /*context*/) {
        return std::get<AtomicProposition<T>>(attr);
    }
};
template<typename T>
struct extract_from_attribute<Operator<T>, std::vector<Operand<T>>>
{
    typedef std::vector<Operand<T>> const &type;
    
    template <typename Context>
    static type call(Operator<T> const &attr, Context& /*context*/) {
        return attr.operands;
    }
};
template<typename T>
struct extract_from_attribute<Operand<T>, boost::variant<AtomicProposition<T>, Operator<T>>> {
    typedef boost::variant<AtomicProposition<T>, Operator<T>> type;

    template <typename Context>
    static type call(Operand<T> const& attr, Context& /*context*/) {
        type result;
        struct visitor_t{
            type &result;
            visitor_t(type &paramResult) : result(paramResult) {}
            void operator()(AtomicProposition<T> const &t) { result = t; }
            void operator()(Operator<T> const &u) { result = u; }
        } visitor(result);
        std::visit(visitor, attr);
        return result;
    }
};
template<typename T>
struct extract_from_container<std::vector<Operand<T>>, std::vector<boost::variant<AtomicProposition<T>, Operator<T>>>> {
    typedef std::vector<boost::variant<AtomicProposition<T>, Operator<T>>> type;

    template <typename Context>
    static type call(std::vector<Operand<T>> const& attr, Context& /*context*/) {
        type result;
        struct visitor_t{
            type &result;
            visitor_t(type &paramResult) : result(paramResult) {}
            void operator()(AtomicProposition<T> const &t) { result.push_back(t); }
            void operator()(Operator<T> const &u) { result.push_back(u); }
        } visitor(result);

        for(auto const &elem : attr) {
            std::visit(visitor, elem);
        }
        return result;
    }
};
}

template<typename Iterator, typename T>
struct SExpressionKarmaGrammar : boost::spirit::karma::grammar<Iterator, Expression<T>()> {
    SExpressionKarmaGrammar() : SExpressionKarmaGrammar::base_type(ruleExpression) {
        namespace karma = boost::spirit::karma;
        namespace spirit = boost::spirit;
        using boost::spirit::karma::eps;
        
        std::string const apCharacterClass = "_a-zA-Z!%&/=?+*<>@.,0-9-";
        ruleAtomicProposition = +(spirit::ascii::char_(apCharacterClass));

        ruleOperand %= (ruleAtomicProposition | ruleOperator);
        ruleOperandList %= ruleOperand % karma::lit(' ');
        ruleOperator %= (
                          karma::lit('(') << ruleOperandList << karma::lit(')')
                        );
        ruleExpression %= ruleOperand;

        ruleAtomicProposition.name("AtomicProposition");
        ruleOperand.name("Operand");
        ruleOperandList.name("OperandList");
        ruleOperator.name("Operator");
        ruleExpression.name("Expression");

        //debug(ruleOperand);
    }

    boost::spirit::karma::rule<Iterator, AtomicProposition<T>()> ruleAtomicProposition;
    boost::spirit::karma::rule<Iterator, Operator<T>()> ruleOperator;
    boost::spirit::karma::rule<Iterator, Operand<T>()> ruleOperand;
    boost::spirit::karma::rule<Iterator, std::vector<Operand<T>>()> ruleOperandList;
    boost::spirit::karma::rule<Iterator, Expression<T>()> ruleExpression;
};

std::string generateSExpression(libexpressions::ExpressionNodePtr const &exp) {
    SExpressionKarmaGrammar<std::back_insert_iterator<std::string>, std::string> grammar;
    Expression<std::string> data = generateASTFromExpression(exp);
    std::string result;
    try {
    bool r = boost::spirit::karma::generate(
                std::back_inserter(result),
                grammar,
                data
             );
        if(r) {
            return result;
        } else {
            std::cerr << "Failed to generate an S-expression" << std::endl;
            return "";
        }
    } catch(std::exception &e) {
        std::cerr << e.what() << std::endl;
        throw;
    }
}
