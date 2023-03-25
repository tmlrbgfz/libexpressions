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

#include "libexpressions/matchers/hierarchyMatchers.hpp"

namespace libexpressions::Matchers {
    /// EqualityProperty
    std::unique_ptr<MatcherImpl> EqualityProperty::construct() const {
        return std::unique_ptr<MatcherImpl>(new EqualityProperty(ref));
    }
    EqualityProperty::EqualityProperty(libexpressions::ExpressionNodePtr const &ptr) : ref(ptr) {}
    bool EqualityProperty::operator()(libexpressions::ExpressionNodePtr const &ptr) const {
        bool result = ptr->equal_to(this->ref.get());
        return result;
    }

    /// HasChildProperty
    std::unique_ptr<MatcherImpl> HasChildProperty::construct() const {
        return std::unique_ptr<MatcherImpl>(new HasChildProperty);
    }
    bool HasChildProperty::operator()(libexpressions::ExpressionNodePtr const &ptr) const {
        if(dynamic_cast<libexpressions::Operator const*>(ptr.get()) != nullptr) {
            libexpressions::Operator const *op = dynamic_cast<libexpressions::Operator const*>(ptr.get());
            return std::any_of(op->begin(), op->end(), [this](libexpressions::ExpressionNodePtr const &child)->bool {
                        return this->nested->operator()(child);
            });
        }
        return false;
    }
    /// AllChildrenProperty
    std::unique_ptr<MatcherImpl> AllChildrenProperty::construct() const {
        return std::unique_ptr<MatcherImpl>(new AllChildrenProperty);
    }
    bool AllChildrenProperty::operator()(libexpressions::ExpressionNodePtr const &ptr) const {
        if(dynamic_cast<libexpressions::Operator const*>(ptr.get()) != nullptr) {
            libexpressions::Operator const *op = dynamic_cast<libexpressions::Operator const*>(ptr.get());
            return std::all_of(op->begin(), op->end(), [this](libexpressions::ExpressionNodePtr const &child)->bool {
                        return this->nested->operator()(child);
            });
        }
        return false;
    }
    /// NthChildProperty
    std::unique_ptr<MatcherImpl> NthChildProperty::construct() const {
        return std::unique_ptr<MatcherImpl>(new NthChildProperty(n));
    }
    NthChildProperty::NthChildProperty(size_t param) : n(param) {}
    bool NthChildProperty::operator()(libexpressions::ExpressionNodePtr const &ptr) const {
        if(dynamic_cast<libexpressions::Operator const*>(ptr.get()) != nullptr) {
            libexpressions::Operator const *op = dynamic_cast<libexpressions::Operator const*>(ptr.get());
            auto iter = op->begin();
            std::advance(iter, static_cast<ssize_t>(n));
            return this->nested->operator()(*iter);
        }
        return false;
    }
    /// AllExceptNthChildProperty
    std::unique_ptr<MatcherImpl> AllExceptNthChildProperty::construct() const {
        return std::unique_ptr<MatcherImpl>(new AllExceptNthChildProperty(n));
    }
    AllExceptNthChildProperty::AllExceptNthChildProperty(size_t param) : n(param) {}

    bool AllExceptNthChildProperty::operator()(libexpressions::ExpressionNodePtr const &ptr) const {
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

    /// KleeneProperty
    std::unique_ptr<MatcherImpl> KleeneProperty::construct() const {
        return std::unique_ptr<MatcherImpl>(new KleeneProperty);
    }
    bool KleeneProperty::operator()(libexpressions::ExpressionNodePtr const &ptr) const {
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
    
    /// AllChildrenRecurseProperty
    std::unique_ptr<MatcherImpl> AllChildrenRecurseProperty::construct() const {
        return std::unique_ptr<MatcherImpl>(new AllChildrenRecurseProperty(invariantMatcher->clone()));
    }
    AllChildrenRecurseProperty::AllChildrenRecurseProperty(std::unique_ptr<MatcherImpl> &&invariant) : invariantMatcher(std::move(invariant)) { }
    bool AllChildrenRecurseProperty::operator()(libexpressions::ExpressionNodePtr const &ptr) const {
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

    /// DescendantProperty
    std::unique_ptr<MatcherImpl> DescendantProperty::construct() const {
        return std::unique_ptr<MatcherImpl>(new DescendantProperty);
    }
    bool DescendantProperty::operator()(libexpressions::ExpressionNodePtr const &ptr) const {
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

    Matcher IsEqual(libexpressions::ExpressionNodePtr const &ptr) {
        EqualityProperty x(ptr);
        return x.clone();
    }
    Matcher HasNthChild(size_t n) {
        return NthChildProperty(n).clone();
    }
    Matcher AllChildrenExceptNth(size_t n) {
        return AllExceptNthChildProperty(n).clone();
    }
    Matcher RecursiveUntilProperty(Matcher const &invariantMatcher) {
        return AllChildrenRecurseProperty(invariantMatcher.getImpl()->clone()).clone();
    }
}

