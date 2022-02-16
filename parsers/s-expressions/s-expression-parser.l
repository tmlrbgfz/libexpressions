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

#include <string>
#include <stdexcept>

#include "libexpressions/parsers/ast.hpp"
#include "libexpressions/parsers/ASTConversion.hpp"
#include "libexpressions/parsers/s-expressions/s-expression-parser.hpp"
#include "s-expression-parser.tab.hpp"
extern libexpressions::parsers::ExpressionList<std::string> result;
static std::string errorString;

#define YY_DECL int yylex(void)
extern "C" YY_DECL;

%}

/* Options */
%option nomain
%option noyywrap
%option nounput
%option noinput

/* Regexes */

IDENTIFIER [_a-zA-Z!%&/=?+*<>@.,0-9'\-]+
WHITESPACE [ \t\n]+

%%

{IDENTIFIER} { yylval.atom = new libexpressions::parsers::AtomicProposition<std::string>(std::string(yytext, yyleng)); return IDENTIFIER; }
{WHITESPACE} { }
. { return static_cast<unsigned char>(*yytext); }

%%

void yyerror(char const *str) {
		errorString = str;
}

namespace libexpressions::parsers {

libexpressions::parsers::ExpressionList<std::string> parseSExpressions(std::string const &input) {
		auto buffer = yy_scan_string(input.c_str());
		auto parseResult = yyparse();
		yy_delete_buffer(buffer);
		yylex_destroy();
		if(parseResult != 0) {
				throw std::runtime_error(std::string("Could not parse input as S-Expression: ") + errorString);
		}
    return result;
}

}

