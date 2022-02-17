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

#include "libexpressions/expressions/expression_visitor.hpp"
#include "libexpressions/expressions/expression_visit_helper.hpp"
#include "libexpressions/expressions/expression_factory.hpp"
#include "libexpressions/expressions/operator.hpp"
#include "libexpressions/expressions/atom.hpp"
#include "libexpressions/parsers/ast.hpp"
#include "libexpressions/parsers/ExpressionRepresentationInterface.hpp"
#include <gsl/gsl_assert>
#include <stack>
#include <memory>
#include <iostream>
#include <llvm/Support/Casting.h>

namespace libexpressions::parsers {

libexpressions::ExpressionNodePtr generateExpressionFromAST(libexpressions::ExpressionFactory *factory, libexpressions::parsers::Expression<std::string>   const &exp) {
    std::stack<Operand<std::string>> stOperands;
    std::stack<Operator<std::string>> stOperators;
    std::stack<AtomicProposition<std::string>> stAtoms;
    std::stack<std::stack<libexpressions::ExpressionNodePtr>> buildingStack;
    enum State_t {
        EXPRESSION,
        OPERAND,
        OPERATOR,
        ATOM
    };
    std::stack<State_t> state;
    state.push(EXPRESSION);
    class OperandVisitor {
    private:
        std::stack<decltype(stOperators)::value_type> &operatorStack;
        std::stack<decltype(stAtoms)::value_type> &apStack;
        std::stack<decltype(state)::value_type> &stateStack;
    public:
        OperandVisitor(std::stack<Operator<std::string>> &opStack,
                       std::stack<AtomicProposition<std::string>> &paramApStack,
                       std::stack<State_t> &stStack) :
                       operatorStack(opStack),
                       apStack(paramApStack),
                       stateStack(stStack) {}
        void operator()(AtomicProposition<std::string> const &ap) {
            apStack.push(ap);
            stateStack.push(ATOM);
        }
        void operator()(Operator<std::string> const &op) {
            operatorStack.push(op);
            stateStack.push(OPERATOR);
        }
    } operandVisitor(stOperators, stAtoms, state);
    do {
        if(state.top() == EXPRESSION) {
            if(buildingStack.size() > 0) {
                state.pop(); //Pop EXPRESSION
            } else {
                buildingStack.emplace();
                state.push(OPERAND);
                stOperands.push(exp);
            }
        } else if(state.top() == OPERAND) {
            state.pop(); //Pop OPERAND
            //boost::apply_visitor(operandVisitor, stOperands.top());
            std::visit(operandVisitor, stOperands.top());
            if(state.top() == OPERATOR) {
                buildingStack.emplace();
            }
            stOperands.pop();
        } else if(state.top() == OPERATOR) {
            if(not buildingStack.top().empty()) {
                std::vector<libexpressions::ExpressionNodePtr> operands;
                for(size_t num = stOperators.top().operands.size(); num > 0; --num) {
                    operands.push_back(std::move(buildingStack.top().top()));
                    buildingStack.top().pop();
                }
                buildingStack.pop();
                buildingStack.top().push(factory->makeExpression(operands.begin(), operands.end()));
                stOperators.pop();
                state.pop(); //Pop OPERATOR
            } else {
                if(stOperators.top().operands.empty()) {
                    buildingStack.pop();
                    buildingStack.top().push(factory->makeExpression());
                    stOperators.pop();
                    state.pop(); //Pop OPERATOR
                } else {
                    for(auto iter = stOperators.top().operands.begin();
                        iter != stOperators.top().operands.end();
                        ++iter) {
                        state.push(OPERAND);
                        stOperands.push(*iter);
                    }
                }
            }
        } else if(state.top() == ATOM) {
            buildingStack.top().push(factory->makeIdentifier(std::move(stAtoms.top())));
            stAtoms.pop();
            state.pop(); //Pop ATOM
        }
    } while(!state.empty());
    return buildingStack.top().top();
}

Expression<std::string>   generateASTFromExpression(libexpressions::ExpressionNodePtr const &exp) {
    Expects(llvm::isa<libexpressions::Operator const>(exp.get()));
    std::stack<std::stack<Operand<std::string>>> stOperands;
    std::stack<Operator<std::string>> stOperators;
    std::stack<AtomicProposition<std::string>> stAtoms;
    std::stack<libexpressions::ExpressionNodePtr> decompositionStack;
    enum State_t {
        EXPRESSION_DOWN,
        EXPRESSION_UP,
        OPERAND_DOWN,
        OPERAND_UP,
        OPERATOR_DOWN,
        OPERATOR_UP,
        ATOM
    };
    std::stack<State_t> state;
    state.push(EXPRESSION_DOWN);
    class OperandVisitor : public libexpressions::ExpressionVisitor {
    private:
        std::stack<State_t> &stateStack;
    public:
        OperandVisitor(std::stack<State_t> &stStack) :
            stateStack(stStack) {}
        void operator()(libexpressions::ExpressionNode const* /*node*/) {}
        void operator()(libexpressions::Atom const */*ap*/) {
            stateStack.push(ATOM);
        }
        void operator()(libexpressions::Operator const */*op*/) {
            stateStack.push(OPERATOR_DOWN);
        }
    } operandVisitor(state);
    do {
        if(state.top() == EXPRESSION_DOWN) {
            state.pop();
            state.push(EXPRESSION_UP);

            decompositionStack.push(exp);
            state.push(OPERATOR_DOWN);
        } else if(state.top() == EXPRESSION_UP) {
            state.pop();
        } else if(state.top() == OPERAND_DOWN) {
            state.pop();
            state.push(OPERAND_UP);

            libexpressions::visit(decompositionStack.top().get(), operandVisitor);
        } else if(state.top() == OPERAND_UP) {
            state.pop();
        } else if(state.top() == OPERATOR_DOWN) {
            state.pop();
            state.push(OPERATOR_UP);

            Expects(llvm::isa<libexpressions::Operator const>(decompositionStack.top().get()));
            libexpressions::Operator const &op = *std::static_pointer_cast<libexpressions::Operator const>(decompositionStack.top());

            for(auto const &operand : op) {
                state.push(OPERAND_DOWN);
                decompositionStack.push(operand);
            }
            stOperands.emplace();
        } else if(state.top() == OPERATOR_UP) {
            stOperators.emplace(Operator<std::string>());
            while(not stOperands.top().empty()) {
                stOperators.top().operands.push_back(stOperands.top().top());
                stOperands.top().pop();
            }
            stOperands.pop();
            decompositionStack.pop();
            state.pop();
            if(state.top() == OPERAND_UP) {
                stOperands.top().emplace(std::move(stOperators.top()));
                stOperators.pop();
            }
        } else if(state.top() == ATOM) {
            Expects(llvm::isa<libexpressions::Atom const>(decompositionStack.top().get()));
            libexpressions::Atom const &atom = *std::static_pointer_cast<libexpressions::Atom const>(decompositionStack.top());

            stAtoms.push(AtomicProposition<std::string>(atom.getSymbol()));
            decompositionStack.pop();
            state.pop();
            if(state.top() == OPERAND_UP) {
                stOperands.top().emplace(std::move(stAtoms.top()));
                stAtoms.pop();
            }
        }
    } while(!state.empty());
    return stOperators.top();
}

std::vector<libexpressions::ExpressionNodePtr> generateExpressionsFromString(libexpressions::ExpressionFactory *factory, libexpressions::parsers::ExpressionRepresentationInterface const* eri, std::string const &string) {
    auto eriRep = eri->stringToExpressionList(string);
    std::vector<libexpressions::ExpressionNodePtr> result;
    std::transform(eriRep.cbegin(), eriRep.cend(), std::back_inserter(result), [factory](auto const &expression) {
        return generateExpressionFromAST(factory, expression);
    });
    return result;
}


}

