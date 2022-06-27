#include "libexpressions/utils/expression-tree-visit.hpp"

#include <iterator>
#include <gsl/gsl_assert>

namespace libexpressions {

template<>
size_t getChildNodeIndex(libexpressions::ExpressionNodePtr const *parent, libexpressions::ExpressionNodePtr const *child) {
    libexpressions::Operator const *op = dynamic_cast<libexpressions::Operator const*>(parent->get());
    Expects(op != nullptr); // ptr is a child of previous node. Thus, previous node must have children. Thus, previous node must be an operator.
    auto iter = std::find(op->begin(), op->end(), *child);
    Expects(op->end() != iter); // We expect to find some child equal to ptr
    return static_cast<size_t>(std::distance(op->begin(), iter));
}

std::tuple<libexpressions::Operator::Iterator, libexpressions::Operator::Iterator> getChildrenIteratorsForExpressionNode(libexpressions::ExpressionNodePtr const &nodePtr) {
    if( auto ptr = dynamic_cast<libexpressions::Operator const*>(nodePtr.get()); ptr != nullptr ) {
        return std::make_tuple(ptr->begin(), ptr->end());
    }
    return std::make_tuple(libexpressions::Operator::Iterator(), libexpressions::Operator::Iterator());
}

}

