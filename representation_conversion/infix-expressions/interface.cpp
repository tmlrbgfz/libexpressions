#include "libexpressions/representation_conversion/infix-expressions/interface.hpp"

#include "libexpressions/representation_conversion/infix-expressions/infix-expression-generator.hpp"

#include <stdexcept>

namespace libexpressions::representation {
    InfixExpressionRepresentationInterface::~InfixExpressionRepresentationInterface() { }
    libexpressions::representation::ExpressionList<std::string> InfixExpressionRepresentationInterface::stringToExpressionList(std::string const &input) const {
        throw std::runtime_error("Not implemented.");
    }
    std::string InfixExpressionRepresentationInterface::expressionListToString(libexpressions::representation::ExpressionList<std::string> const &input) const {
        std::string result;
        for(auto const &exp : input) {
            result += infixExpressionToString(exp);
        }
        return result;
    }
}

