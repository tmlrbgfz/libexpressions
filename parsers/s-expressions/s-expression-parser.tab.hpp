/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_LIBEXPRESSIONS_S_EXPRESSION_S_EXPRESSION_PARSER_TAB_HPP_INCLUDED
# define YY_LIBEXPRESSIONS_S_EXPRESSION_S_EXPRESSION_PARSER_TAB_HPP_INCLUDED
/* Debug traces.  */
#ifndef LIBEXPRESSIONS_S_EXPRESSIONDEBUG
# if defined YYDEBUG
#if YYDEBUG
#   define LIBEXPRESSIONS_S_EXPRESSIONDEBUG 1
#  else
#   define LIBEXPRESSIONS_S_EXPRESSIONDEBUG 0
#  endif
# else /* ! defined YYDEBUG */
#  define LIBEXPRESSIONS_S_EXPRESSIONDEBUG 0
# endif /* ! defined YYDEBUG */
#endif  /* ! defined LIBEXPRESSIONS_S_EXPRESSIONDEBUG */
#if LIBEXPRESSIONS_S_EXPRESSIONDEBUG
extern int libexpressions_s_expressiondebug;
#endif

/* Token kinds.  */
#ifndef LIBEXPRESSIONS_S_EXPRESSIONTOKENTYPE
# define LIBEXPRESSIONS_S_EXPRESSIONTOKENTYPE
  enum libexpressions_s_expressiontokentype
  {
    LIBEXPRESSIONS_S_EXPRESSIONEMPTY = -2,
    LIBEXPRESSIONS_S_EXPRESSIONEOF = 0, /* "end of file"  */
    LIBEXPRESSIONS_S_EXPRESSIONerror = 256, /* error  */
    LIBEXPRESSIONS_S_EXPRESSIONUNDEF = 257, /* "invalid token"  */
    IDENTIFIER = 258,              /* IDENTIFIER  */
    WHITESPACE = 259               /* WHITESPACE  */
  };
  typedef enum libexpressions_s_expressiontokentype libexpressions_s_expressiontoken_kind_t;
#endif

/* Value type.  */
#if ! defined LIBEXPRESSIONS_S_EXPRESSIONSTYPE && ! defined LIBEXPRESSIONS_S_EXPRESSIONSTYPE_IS_DECLARED
union LIBEXPRESSIONS_S_EXPRESSIONSTYPE
{
#line 41 "s-expression-parser.y"

	libexpressions::parsers::AtomicProposition<std::string> *atom;
	libexpressions::parsers::Operand<std::string> *operand;
	std::vector<libexpressions::parsers::Operand<std::string>> *operandList;
	libexpressions::parsers::Operator<std::string> *oprtr;
	std::vector<libexpressions::parsers::Operator<std::string>> *operatorList;

#line 84 "s-expression-parser.tab.hpp"

};
typedef union LIBEXPRESSIONS_S_EXPRESSIONSTYPE LIBEXPRESSIONS_S_EXPRESSIONSTYPE;
# define LIBEXPRESSIONS_S_EXPRESSIONSTYPE_IS_TRIVIAL 1
# define LIBEXPRESSIONS_S_EXPRESSIONSTYPE_IS_DECLARED 1
#endif




int libexpressions_s_expressionparse (libexpressions::parsers::ExpressionList<std::string> &result, std::string &errorString, yyscan_t yyscanner);


#endif /* !YY_LIBEXPRESSIONS_S_EXPRESSION_S_EXPRESSION_PARSER_TAB_HPP_INCLUDED  */
