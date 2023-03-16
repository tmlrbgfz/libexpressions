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
#pragma once

#include "libexpressions/matchers/matchers.hpp"
#include "libexpressions/matchers/matcherConnectives.hpp"

namespace libexpressions::Matchers {
    class EqualityProperty : public MatcherImpl {
    private:
        libexpressions::ExpressionNodePtr ref;
    protected:
        virtual std::unique_ptr<MatcherImpl> construct() const override {
            return std::unique_ptr<MatcherImpl>(new EqualityProperty(ref));
        }
        EqualityProperty() = default;
    public:
        EqualityProperty(libexpressions::ExpressionNodePtr const &ptr) : ref(ptr) {}
        virtual bool operator()(libexpressions::ExpressionNodePtr const &ptr) const override {
            bool result = ptr->equal_to(this->ref.get());
            return result;
        }
    };
    class HasChildProperty : public MatcherImpl {
    protected:
        virtual std::unique_ptr<MatcherImpl> construct() const override {
            return std::unique_ptr<MatcherImpl>(new HasChildProperty);
        }
    public:
        virtual bool operator()(libexpressions::ExpressionNodePtr const &ptr) const override {
            if(dynamic_cast<libexpressions::Operator const*>(ptr.get()) != nullptr) {
                libexpressions::Operator const *op = dynamic_cast<libexpressions::Operator const*>(ptr.get());
                return std::any_of(op->begin(), op->end(), [this](libexpressions::ExpressionNodePtr const &child)->bool {
                            return this->nested->operator()(child);
                });
            }
            return false;
        }
    };
    class AllChildrenProperty : public MatcherImpl {
    protected:
        virtual std::unique_ptr<MatcherImpl> construct() const override {
            return std::unique_ptr<MatcherImpl>(new AllChildrenProperty);
        }
    public:
        virtual bool operator()(libexpressions::ExpressionNodePtr const &ptr) const override {
            if(dynamic_cast<libexpressions::Operator const*>(ptr.get()) != nullptr) {
                libexpressions::Operator const *op = dynamic_cast<libexpressions::Operator const*>(ptr.get());
                return std::all_of(op->begin(), op->end(), [this](libexpressions::ExpressionNodePtr const &child)->bool {
                            return this->nested->operator()(child);
                });
            }
            return false;
        }
    };
    class NthChildProperty : public MatcherImpl {
    private:
        size_t n;
    protected:
        std::unique_ptr<MatcherImpl> construct() const override {
            return std::unique_ptr<MatcherImpl>(new NthChildProperty(n));
        }
    public:
        NthChildProperty(size_t param) : n(param) {}
        bool operator()(libexpressions::ExpressionNodePtr const &ptr) const override {
            if(dynamic_cast<libexpressions::Operator const*>(ptr.get()) != nullptr) {
                libexpressions::Operator const *op = dynamic_cast<libexpressions::Operator const*>(ptr.get());
                auto iter = op->begin();
                std::advance(iter, static_cast<ssize_t>(n));
                return this->nested->operator()(*iter);
            }
            return false;
        }
    };
    class AllExceptNthChildProperty : public MatcherImpl {
    private:
        size_t n;
    protected:
        std::unique_ptr<MatcherImpl> construct() const override {
            return std::unique_ptr<MatcherImpl>(new AllExceptNthChildProperty(n));
        }
    public:
        AllExceptNthChildProperty(size_t param) : n(param) {}

        bool operator()(libexpressions::ExpressionNodePtr const &ptr) const override {
            if(dynamic_cast<libexpressions::Operator const*>(ptr.get()) != nullptr) {
                libexpressions::Operator const *op = dynamic_cast<libexpressions::Operator const*>(ptr.get());
                size_t idx = 0;
                for(auto iter = op->begin(); iter != op->end(); ++iter, ++idx) {
                    if(idx != n and not this->nested->operator()(*iter)) {
                        return false;
                    }
                }
                return true;
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
                if(dynamic_cast<libexpressions::Operator const*>(node.get()) != nullptr) {
                    libexpressions::Operator const *op = dynamic_cast<libexpressions::Operator const*>(node.get());
                    worklist.insert(worklist.end(), op->begin(), op->end());
                }
                worklist.pop_front();
            }
            return false;
        }
    };
    class AllChildrenRecurseProperty : public MatcherImpl {
    private:
        std::unique_ptr<MatcherImpl> invariantMatcher;
    protected:
        virtual std::unique_ptr<MatcherImpl> construct() const override {
            return std::unique_ptr<MatcherImpl>(new AllChildrenRecurseProperty(invariantMatcher->clone()));
        }
    public:
        AllChildrenRecurseProperty(std::unique_ptr<MatcherImpl> &&invariant) : invariantMatcher(std::move(invariant)) { }
        virtual bool operator()(libexpressions::ExpressionNodePtr const &ptr) const override {
            if(this->nested->operator()(ptr)) {
                return true;
            } else if(auto op = dynamic_cast<libexpressions::Operator const*>(ptr.get());
              op != nullptr ) {
                if (this->invariantMatcher->operator()(ptr)) {
                    for (auto const &child: *op) {
                        if (not this->operator()(child)) {
                            return false;
                        }
                    }
                    return true;
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
                if(dynamic_cast<libexpressions::Operator const*>(node.get()) != nullptr) {
                    libexpressions::Operator const *op = dynamic_cast<libexpressions::Operator const*>(node.get());
                    worklist.insert(worklist.end(), op->begin(), op->end());
                }
                worklist.pop_front();
            }
            return false;
        }
    };

    Matcher IsEqual(libexpressions::ExpressionNodePtr const &ptr) {
        EqualityProperty x(ptr);
        return x.clone();
    }
    template<typename... Args>
    Matcher HasChild(Args&& ...args) {
        HasChildProperty prop;
        return prop(args...);
    }
    template<typename... Args>
    Matcher AllChildren(Args&& ...args) {
        AllChildrenProperty prop;
        return prop(args...);
    }
    Matcher HasNthChild(size_t n) {
        return NthChildProperty(n).clone();
    }
    Matcher AllChildrenExceptNth(size_t n) {
        return AllExceptNthChildProperty(n).clone();
    }
    template<typename... Args>
    Matcher ThisOrAnyChild(Args&& ...args) {
        KleeneProperty prop;
        return prop(args...);
    }
    template<typename... Args>
    Matcher HasDescendant(Args&& ...args) {
        DescendantProperty prop;
        return prop(args...);
    }
    Matcher RecursiveUntilProperty(Matcher const &invariantMatcher) {
        return AllChildrenRecurseProperty(invariantMatcher.getImpl()->clone()).clone();
    }
}

