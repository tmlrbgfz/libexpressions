#!/usr/bin/env bash

flex --header-file=s-expression-parser.lex.hpp --outfile=s-expression-parser.lex.cpp s-expression-parser.l
bison --defines=s-expression-parser.tab.hpp -o s-expression-parser.tab.cpp s-expression-parser.y
