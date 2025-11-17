add_library(libexpressions_representation OBJECT)

target_sources(libexpressions_representation
    PRIVATE
        ASTConversion.cpp
    PUBLIC
        ASTConversion.hpp
        ast.hpp
        ExpressionRepresentationInterface.hpp)

target_include_directories(libexpressions_representation PRIVATE ${LIBEXPRESSIONS_INCLUDE_ROOT})

add_subdirectory(s-expressions)
add_subdirectory(infix-expressions)
