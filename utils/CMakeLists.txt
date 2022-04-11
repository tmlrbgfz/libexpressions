add_library(libexpressions_utils OBJECT)

target_sources(libexpressions_utils
    PRIVATE
        expression-tree-visit.cpp
    PUBLIC
        expression-tree-visit.hpp
        tree-visit.hpp
        variadic-insert.hpp
        variadic_type_helper.hpp
        version.hpp)

target_include_directories(libexpressions_utils PUBLIC ${LIBEXPRESSIONS_INCLUDE_ROOT})
