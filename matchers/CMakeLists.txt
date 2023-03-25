add_library(libexpressions_matchers OBJECT)

target_sources(libexpressions_matchers
    PUBLIC
        hierarchyMatchers.hpp
        matcherConnectives.hpp
        matchers.hpp
    PRIVATE
        hierarchyMatchers.cpp
        matcherConnectives.cpp
        matchers.cpp)

target_include_directories(libexpressions_matchers PUBLIC ${LIBEXPRESSIONS_INCLUDE_ROOT})

add_subdirectory(expressions)
