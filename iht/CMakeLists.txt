add_library(libexpressions_iht INTERFACE)

target_sources(libexpressions_iht
    INTERFACE
        iht_factory.hpp
        iht_node.hpp
        iht_node_type_visitor.hpp)

target_include_directories(libexpressions_iht INTERFACE ${LIBEXPRESSIONS_INCLUDE_ROOT})
