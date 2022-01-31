add_library(libexpressions_expressionNodes OBJECT)
target_sources(libexpressions_expressionNodes
    INTERFACE
        atom.hpp
        expression_factory.hpp
        expression_node.hpp
        expression_node_kind.hpp
        expression_visit_helper.hpp
        expression_visitor.hpp
        operator.hpp
    PRIVATE
        expression_node.cpp
    )

target_include_directories(libexpressions_expressionNodes PUBLIC ${LIBEXPRESSIONS_INCLUDE_ROOT})
