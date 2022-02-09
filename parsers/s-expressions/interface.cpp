#include "libexpressions/parsers/s-expressions/interface.hpp"

#include "libexpressions/parsers/s-expressions/s-expression-parser.hpp"
#include "libexpressions/parsers/s-expressions/s-expression-generator.hpp"

#include <iostream>

namespace libexpressions::parsers {
    SExpressionRepresentationInterface::~SExpressionRepresentationInterface() { }
    libexpressions::parsers::ExpressionList<std::string> SExpressionRepresentationInterface::stringToExpressionList(std::string const &input) const {
        return parseSExpressions(input);
    }
    std::string SExpressionRepresentationInterface::expressionListToString(libexpressions::parsers::ExpressionList<std::string> const &input) const {
        std::string result;
        for(auto const &exp : input) {
            result += sExpressionToString(exp);
        }
        return result;
    }
}

