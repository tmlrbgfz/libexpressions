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
#ifndef __LIBEXPRESSIONS_EXPRESSION_NODE__
#define __LIBEXPRESSIONS_EXPRESSION_NODE__

#include "libexpressions/iht/iht_node.hpp"
#include "libexpressions/expressions/expression_node_kind.hpp"
#include "libexpressions/expressions/expression_visitor.hpp"
#include <string>
#include <vector>
#include <memory>

namespace libexpressions {
    class ExpressionNode : public IHT::IHTNode<ExpressionNode> {
    public:
        typedef ExpressionNodeKind kind_type;
    private:
        kind_type nodeKind;
    protected:
        ExpressionNode(kind_type kind) : nodeKind(kind) { }
    public:
        ~ExpressionNode() = default;

        void accept(ExpressionVisitor *visitor) const;

        std::string toString() const;

        constexpr kind_type getKind() const {
            return this->nodeKind;
        }

        IHT::hash_type hash() const;

        bool equal_to(ExpressionNode const *node) const;

        static constexpr bool classof(ExpressionNode const *node) {
            return node->getKind() > ExpressionNodeKind::EXPRESSION_NODE && node->getKind() < ExpressionNodeKind::LAST_EXPRESSION_NODE;
        }
    };

    typedef std::shared_ptr<ExpressionNode const> ExpressionNodePtr;

    typedef std::vector<ExpressionNodePtr> ExpressionNodePtrContainer;
}

#endif //__LIBEXPRESSIONS_EXPRESSION_NODE__

