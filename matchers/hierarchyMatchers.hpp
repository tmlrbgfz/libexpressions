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
#ifndef __LIBEXPRESSIONS_HIERARCHYMATCHERS__
#define __LIBEXPRESSIONS_HIERARCHYMATCHERS__

#include "matchers/matchers.hpp"
#include "matchers/matcherConnectives.hpp"

namespace libexpressions::Matchers {
    class EqualityProperty : public MatcherImpl {
    private:
        libexpressions::ExpressionNodePtr ref;
    protected:
        virtual std::unique_ptr<MatcherImpl> construct() const override {
            return std::unique_ptr<MatcherImpl>(new EqualityProperty);
        }
        EqualityProperty() = default;
    public:
        EqualityProperty(libexpressions::ExpressionNodePtr const &ptr) : ref(ptr) {}
        virtual bool operator()(libexpressions::ExpressionNodePtr const &ptr) const override {
            return ptr->equal_to(this->ref.get());
        }
    };
    class HasChildProperty : public MatcherImpl {
    protected:
        virtual std::unique_ptr<MatcherImpl> construct() const override {
            return std::unique_ptr<MatcherImpl>(new ChildProperty);
        }
    public:
        virtual bool operator()(libexpressions::ExpressionNodePtr const &ptr) const override {
            if(llvm::isa<libexpressions::Operator>(ptr.get())) {
                libexpressions::Operator const *op = llvm::dyn_cast<libexpressions::Operator const>(ptr.get());
                return std::any_of(op->begin(), op->end(), [this](libexpressions::ExpressionNodePtr const &child)->bool {
                            return this->nested->operator()(child);
                });
            }
            return false;
        }
    };
    class KleeneProperty : public MatcherImpl {
    protected:
        virtual std::unique_ptr<MatcherImpl> construct() const override {
            return std::unique_ptr<MatcherImpl>(new KleeneProperty);
        }
    public:
        virtual bool operator()(libexpressions::ExpressionNodePtr const &ptr) const override {
            std::deque<libexpressions::ExpressionNodePtr> worklist;
            worklist.push_back(ptr);
            while(!worklist.empty()) {
                libexpressions::ExpressionNodePtr node = worklist.front();
                if(this->nested->operator()(node)) {
                    return true;
                }
                if(llvm::isa<libexpressions::Operator>(node.get())) {
                    libexpressions::Operator const *op = llvm::dyn_cast<libexpressions::Operator const>(node.get());
                    worklist.insert(worklist.end(), op->begin(), op->end());
                }
            }
            return false;
        }
    };
    class DescendantProperty : public MatcherImpl {
    protected:
        virtual std::unique_ptr<MatcherImpl> construct() const override {
            return std::unique_ptr<MatcherImpl>(new DescendantProperty);
        }
    public:
        virtual bool operator()(libexpressions::ExpressionNodePtr const &ptr) const override {
            std::deque<libexpressions::ExpressionNodePtr> worklist;
            worklist.push_back(ptr);
            while(!worklist.empty()) {
                libexpressions::ExpressionNodePtr node = worklist.front();
                if(node != ptr && this->nested->operator()(node)) {
                    return true;
                }
                if(llvm::isa<libexpressions::Operator>(node.get())) {
                    libexpressions::Operator const *op = llvm::dyn_cast<libexpressions::Operator const>(node.get());
                    worklist.insert(worklist.end(), op->begin(), op->end());
                }
            }
            return false;
        }
    };

    std::unique_ptr<MatcherImpl> IsEqual(libexpressions::ExpressionNodePtr const &ptr) {
        EqualityProperty x(ptr);
        return x.clone();
    }
    ChildProperty HasChild;
    KleeneProperty ThisOrAnyChild;
    DescendantProperty HasDescendant;
}

#endif //__LIBEXPRESSIONS_HIERARCHYMATCHERS__
