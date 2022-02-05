find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

bison_target(s-expression-parser s-expression-parser.y s-expression-parser.tab.cpp)
flex_target(s-expression-lexer s-expression-parser.l s-expression-parser.lex.cpp)

add_library(libexpressions_parsers OBJECT)

target_sources(libexpressions_parsers
    PRIVATE
        ASTConversion.cpp
        s-expression-parser.cpp
        ${BISON_s-expression-parser_OUTPUTS}
        ${FLEX_s-expression-lexer_OUTPUTS}
    PUBLIC
        ASTConversion.hpp
        ast.hpp
        s-expression-parser.hpp)

set_source_files_properties(${FLEX_s-expression-lexer_OUTPUTS} PROPERTIES COMPILE_FLAGS "-Wno-old-style-cast -Wno-sign-conversion")

target_include_directories(libexpressions_parsers PRIVATE ${LIBEXPRESSIONS_INCLUDE_ROOT})
