add_library(libexpressions_utils INTERFACE)

target_sources(libexpressions_utils
    INTERFACE
        tree-visit.hpp
        variadic-insert.hpp
        variadic_type_helper.hpp
        version.hpp)

target_include_directories(libexpressions_utils INTERFACE ${LIBEXPRESSIONS_INCLUDE_ROOT})
