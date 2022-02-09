add_library(libexpressions_parsers_sexpressions OBJECT)

find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

bison_target(s-expression-parser "s-expression-parser.y" "${CMAKE_CURRENT_BINARY_DIR}/s-expression-parser.tab.cpp")
flex_target(s-expression-lexer "s-expression-parser.l" "${CMAKE_CURRENT_BINARY_DIR}/s-expression-parser.lex.cpp")
add_flex_bison_dependency(s-expression-lexer s-expression-parser)

target_sources(libexpressions_parsers_sexpressions
    PRIVATE
        ${BISON_s-expression-parser_OUTPUTS}
        ${FLEX_s-expression-lexer_OUTPUTS}
        interface.cpp
    PUBLIC
        s-expression-parser.hpp
        interface.hpp)

set_source_files_properties(${FLEX_s-expression-lexer_OUTPUTS} PROPERTIES COMPILE_FLAGS "-Wno-old-style-cast -Wno-sign-conversion")

target_include_directories(libexpressions_parsers_sexpressions PRIVATE ${LIBEXPRESSIONS_INCLUDE_ROOT})
