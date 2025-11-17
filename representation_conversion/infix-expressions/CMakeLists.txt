add_library(libexpressions_representation_infixexpressions OBJECT)

target_sources(libexpressions_representation_infixexpressions
    PRIVATE
        interface.cpp
    PUBLIC
        interface.hpp)

target_include_directories(libexpressions_representation_infixexpressions PRIVATE ${LIBEXPRESSIONS_INCLUDE_ROOT})
