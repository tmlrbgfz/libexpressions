add_library(libexpressions_evaluators INTERFACE)
target_sources(libexpressions_evaluators
    INTERFACE
        evaluator.hpp
    )

target_include_directories(libexpressions_evaluators INTERFACE ${LIBEXPRESSIONS_INCLUDE_ROOT})
