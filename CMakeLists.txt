cmake_minimum_required(VERSION 3.20)

set(LIBEXPRESSIONS_VERSION_MAJOR 0)
set(LIBEXPRESSIONS_VERSION_MINOR 1)

project(libexpressions VERSION "${LIBEXPRESSIONS_VERSION_MAJOR}.${LIBEXPRESSIONS_VERSION_MINOR}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_library(expressions STATIC)
target_include_directories(expressions PUBLIC ${LIBEXPRESSIONS_INCLUDE_ROOT})

target_compile_definitions(expressions PRIVATE LIBEXPRESSIONS_VERSION_MAJOR=${LIBEXPRESSIONS_VERSION_MAJOR}
                                               LIBEXPRESSIONS_VERSION_MINOR=${LIBEXPRESSIONS_VERSION_MINOR})
if (MSVC)
    add_compile_options(/WX /W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -pedantic -Wsign-conversion -Wconversion -Wnarrowing -Wuninitialized -Wshadow -Wfatal-errors
                        -Wold-style-cast -Wswitch-enum)
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
      add_compile_options(-Wdocumentation)
    endif()
endif()

add_subdirectory(expressions)
add_subdirectory(iht)
add_subdirectory(matchers)
add_subdirectory(parsers)
add_subdirectory(utils)

target_link_libraries(expressions
    PUBLIC
        libexpressions_expressionNodes
        libexpressions_iht
        libexpressions_matchers
        libexpressions_parsers
        libexpressions_parsers_sexpressions
        libexpressions_utils)
