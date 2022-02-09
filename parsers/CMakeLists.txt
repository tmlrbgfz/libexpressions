add_library(libexpressions_parsers OBJECT)

target_sources(libexpressions_parsers
    PRIVATE
        ASTConversion.cpp
    PUBLIC
        ASTConversion.hpp
        ast.hpp)

target_include_directories(libexpressions_parsers PRIVATE ${LIBEXPRESSIONS_INCLUDE_ROOT})

add_subdirectory(s-expressions)
