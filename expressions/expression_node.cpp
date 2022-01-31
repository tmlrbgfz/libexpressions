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
#include "libexpressions/expressions/expression_node.hpp"

#include "libexpressions/expressions/expression_visit_helper.hpp"


namespace libexpressions {
    void ExpressionNode::accept(libexpressions::ExpressionVisitor *visitor) const {
        visitor->visit(*this);
        libexpressions::visit(this, [visitor](auto thisNode) {
            visitor->visit(*thisNode);
        });
    }

    std::string ExpressionNode::toString() const {
        return libexpressions::visit(this, [](auto thisNode) {
            return thisNode->toString();
        });
    }

    IHT::hash_type ExpressionNode::hash() const {
        return libexpressions::visit(this, [](auto thisNode) {
            return thisNode->hash();
        });
    }

    bool ExpressionNode::equal_to(ExpressionNode const *node) const {
        return libexpressions::visit(this, [node](auto thisNode) {
            return thisNode->equal_to(node);
        });
    }
}

