add_library(libexpressions_parsers OBJECT)

target_sources(libexpressions_parsers
    PRIVATE
        ASTConversion.cpp
        s-expression-parser.cpp
    PUBLIC
        ASTConversion.hpp
        ast.hpp
        s-expression-parser.hpp)

target_include_directories(libexpressions_parsers PRIVATE ${LIBEXPRESSIONS_INCLUDE_ROOT})
