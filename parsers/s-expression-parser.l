/* MIT License
 * 
 * Copyright (c) 2022 Niklas Krafczyk
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* s-expression lexer */

%{

#include "libexpressions/parsers/ast.hpp"
#include "libexpressions/parsers/ASTConversion.hpp"
#include "libexpressions/parsers/s-expression-parser.hpp"
#include "s-expression-parser.tab.hpp"
extern std::vector<libexpressions::parsers::Operator<std::string>> result;

#define YY_DECL int yylex(void)
extern "C" YY_DECL;

%}

/* Options */
%option nomain
%option noyywrap
%option nounput

/* Regexes */

IDENTIFIER [_a-zA-Z!%&/=?+*<>@.,0-9'\-]+
WHITESPACE [ \t\n]+

%%

{IDENTIFIER} { yylval.atom = new libexpressions::parsers::AtomicProposition<std::string>(std::string(yytext, yyleng)); return IDENTIFIER; }
{WHITESPACE} { }
. { return static_cast<unsigned char>(*yytext); }

%%

namespace libexpressions::parsers {

libexpressions::ExpressionNodePtr parseSExpression(libexpressions::ExpressionFactory *factory, std::string const &input) {
		auto buffer = yy_scan_string(input.c_str());
		yyparse();
		yy_delete_buffer(buffer);
		yylex_destroy();
    return generateExpressionFromAST(factory, result.front());
}

}

