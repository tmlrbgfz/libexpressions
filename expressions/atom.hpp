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
#ifndef __LIBEXPRESSIONS_ATOM__
#define __LIBEXPRESSIONS_ATOM__

#include "libexpressions/expressions/expression_node.hpp"

#include <string>
#include <memory>

namespace libexpressions {
    template<typename T>
    class EvaluateableTheory;

    class Atom final : public ExpressionNode {
        friend class IHT::IHTFactory<ExpressionNode>;
    private:
        std::string const symbol;
    protected:
        Atom(std::string paramSymbol)
            : ExpressionNode(ExpressionNodeKind::EXPRESSION_ATOM),
              symbol(paramSymbol) {}
    public:
        ~Atom() = default;

        std::string const &getSymbol() const {
            return symbol;
        }

        IHT::hash_type hash() const {
            auto symHash = std::hash<std::string>{}(symbol);
            symHash ^= (((symHash & 0xFFu) << (sizeof(decltype(symHash))-1u)*8u) & ~0xFFu);
            return symHash;
        }

        std::string toString() const {
            return symbol;
        }

        bool equal_to(ExpressionNode const *other) const {
            if(Atom::classof(other)) {
                Atom const *atom = static_cast<Atom const *>(other);
                return atom->symbol == this->symbol;
            }
            return false;
        }
        static constexpr bool classof(ExpressionNode const *node) {
            return node->getKind() == ExpressionNodeKind::EXPRESSION_ATOM;
        }
    };

    typedef std::shared_ptr<Atom> ExpressionAtomPtr;
}

#endif //__LIBEXPRESSIONS_ATOM__

