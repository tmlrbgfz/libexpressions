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
#include <system_error>

#include "libexpressions/representation_conversion/ast.hpp"
#include "libexpressions/representation_conversion/ASTConversion.hpp"
#include "libexpressions/representation_conversion/s-expressions/s-expression-parser.hpp"
#include "s-expression-parser.tab.hpp"
#define YYSTYPE LIBEXPRESSIONS_S_EXPRESSIONSTYPE

%}

/* Options */
%option nomain
%option noyywrap
%option nounput
%option noinput
%option prefix="libexpressions_s_expression"
%option reentrant
%option bison-bridge
%option warn
%option nodefault

/* Regexes */

IDENTIFIER [^)( \t\n]+
WHITESPACE [ \t\n]+

%%

{IDENTIFIER} { yylval->atom = new libexpressions::representation::AtomicProposition<std::string>(std::string(yytext, yyleng)); return IDENTIFIER; }
{WHITESPACE} { }
. { return static_cast<unsigned char>(*yytext); }

%%

void libexpressions_s_expressionerror(libexpressions::representation::ExpressionList<std::string> &/*result*/, std::string &errorString, yyscan_t /*scanner*/, char const *str) {
		errorString = str;
}

namespace libexpressions::representation {

libexpressions::representation::ExpressionList<std::string> parseSExpressions(std::string const &input) {
		libexpressions::representation::ExpressionList<std::string> result;
		std::string errorString;
		yyscan_t scanner;
		if(yylex_init(&scanner)) {
			throw std::system_error(errno, std::system_category());
		}
		auto buffer = yy_scan_string(input.c_str(), scanner);
		auto parseResult = libexpressions_s_expressionparse(result, errorString, scanner);
		yy_delete_buffer(buffer, scanner);
		yylex_destroy(scanner);
		if(parseResult != 0) {
				throw std::runtime_error(std::string("Could not parse input as S-Expression: ") + errorString);
		}
    return result;
}

}

