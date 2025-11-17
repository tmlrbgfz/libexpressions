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

/* s-expression parser */

%{
#include <vector>
#include <stdexcept>
#include "libexpressions/representation_conversion/ast.hpp"

typedef void* yyscan_t;
#include "libexpressions/representation_conversion/s-expressions/s-expression-parser.tab.hpp"
#include "libexpressions/representation_conversion/s-expressions/s-expression-parser.lex.hpp"

void libexpressions_s_expressionerror(libexpressions::representation::ExpressionList<std::string>&, std::string&, yyscan_t, char const*);

%}

/* Declarations */

%union {
	libexpressions::representation::AtomicProposition<std::string> *atom;
	libexpressions::representation::Operand<std::string> *operand;
	std::vector<libexpressions::representation::Operand<std::string>> *operandList;
	libexpressions::representation::Operator<std::string> *oprtr;
	std::vector<libexpressions::representation::Operator<std::string>> *operatorList;
}

%token <atom> IDENTIFIER
%token WHITESPACE
%nterm <operand> OPERAND
%nterm <operandList> OPERANDLIST
%nterm <oprtr> OPERATOR
%nterm <operatorList> RESULT

%start RESULT

%define parse.lac full
%define parse.error detailed
%define api.prefix {libexpressions_s_expression}
%define api.pure full
%parse-param {libexpressions::representation::ExpressionList<std::string> &result} {std::string &errorString}
%param {yyscan_t yyscanner}

%%

/* Grammar */
OPERAND: IDENTIFIER { $OPERAND = new libexpressions::representation::Operand<std::string>(*$IDENTIFIER); delete $IDENTIFIER; }
			 | OPERATOR { $OPERAND = new libexpressions::representation::Operand<std::string>(*$OPERATOR); delete $OPERATOR; }
OPERANDLIST[result]: OPERANDLIST[list] OPERAND { $result = $list; $result->push_back(std::move(*$OPERAND)); delete $OPERAND; }
									 | %empty { $result = new std::vector<libexpressions::representation::Operand<std::string>>(); }
OPERATOR: '(' OPERANDLIST ')' { $OPERATOR = new libexpressions::representation::Operator<std::string>(); $OPERATOR->operands = std::move(*$OPERANDLIST); delete $OPERANDLIST; }
RESULT: OPERANDLIST { result = std::move(*$OPERANDLIST); delete $OPERANDLIST; }

%%

/* Epilogue */
