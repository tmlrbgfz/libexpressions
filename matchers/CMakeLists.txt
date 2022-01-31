add_library(libexpressions_matchers INTERFACE)

target_sources(libexpressions_matchers
    INTERFACE
        hierarchyMatchers.hpp
        matcherConnectives.hpp
        matchers.hpp)

target_include_directories(libexpressions_matchers INTERFACE ${LIBEXPRESSIONS_INCLUDE_ROOT})

add_subdirectory(expressions)
