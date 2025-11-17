#!/usr/bin/env bash

flex --header-file=s-expression-parser.lex.hpp --outfile=s-expression-parser.lex.cpp s-expression-parser.l
bison -Hs-expression-parser.tab.hpp -o s-expression-parser.tab.cpp s-expression-parser.y
