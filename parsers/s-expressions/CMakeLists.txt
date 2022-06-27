add_library(libexpressions_parsers_sexpressions OBJECT)

target_sources(libexpressions_parsers_sexpressions
    PRIVATE
        s-expression-parser.tab.cpp
        s-expression-parser.lex.cpp
        interface.cpp
    PUBLIC
        s-expression-parser.hpp
        interface.hpp)

set_source_files_properties(s-expression-parser.lex.cpp PROPERTIES COMPILE_FLAGS "-Wno-old-style-cast -Wno-sign-conversion")

target_include_directories(libexpressions_parsers_sexpressions PRIVATE ${LIBEXPRESSIONS_INCLUDE_ROOT})
