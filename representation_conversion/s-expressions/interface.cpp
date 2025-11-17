#include "libexpressions/representation_conversion/s-expressions/interface.hpp"

#include "libexpressions/representation_conversion/s-expressions/s-expression-parser.hpp"
#include "libexpressions/representation_conversion/s-expressions/s-expression-generator.hpp"

namespace libexpressions::representation {
    SExpressionRepresentationInterface::~SExpressionRepresentationInterface() { }
    libexpressions::representation::ExpressionList<std::string> SExpressionRepresentationInterface::stringToExpressionList(std::string const &input) const {
        return parseSExpressions(input);
    }
    std::string SExpressionRepresentationInterface::expressionListToString(libexpressions::representation::ExpressionList<std::string> const &input) const {
        std::string result;
        for(auto const &exp : input) {
            result += sExpressionToString(exp);
        }
        return result;
    }
}

