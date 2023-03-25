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
        std::unique_ptr<MatcherImpl> construct() const override;
        EqualityProperty() = default;
    public:
        EqualityProperty(libexpressions::ExpressionNodePtr const &ptr);
        bool operator()(libexpressions::ExpressionNodePtr const &ptr) const override;
    };
    class HasChildProperty : public MatcherImpl {
    protected:
        std::unique_ptr<MatcherImpl> construct() const override;
    public:
        bool operator()(libexpressions::ExpressionNodePtr const &ptr) const override;
    };
    class AllChildrenProperty : public MatcherImpl {
    protected:
        std::unique_ptr<MatcherImpl> construct() const override;
    public:
        bool operator()(libexpressions::ExpressionNodePtr const &ptr) const override;
    };
    class NthChildProperty : public MatcherImpl {
    private:
        size_t n;
    protected:
        std::unique_ptr<MatcherImpl> construct() const override;
    public:
        NthChildProperty(size_t param);
        bool operator()(libexpressions::ExpressionNodePtr const &ptr) const override;
    };
    class AllExceptNthChildProperty : public MatcherImpl {
    private:
        size_t n;
    protected:
        std::unique_ptr<MatcherImpl> construct() const override;
    public:
        AllExceptNthChildProperty(size_t param);

        bool operator()(libexpressions::ExpressionNodePtr const &ptr) const override;
    };

    class KleeneProperty : public MatcherImpl {
    protected:
        std::unique_ptr<MatcherImpl> construct() const override;
    public:
        bool operator()(libexpressions::ExpressionNodePtr const &ptr) const override;
    };
    class AllChildrenRecurseProperty : public MatcherImpl {
    private:
        std::unique_ptr<MatcherImpl> invariantMatcher;
    protected:
        std::unique_ptr<MatcherImpl> construct() const override;
    public:
        AllChildrenRecurseProperty(std::unique_ptr<MatcherImpl> &&invariant);
        bool operator()(libexpressions::ExpressionNodePtr const &ptr) const override;
    };
    class DescendantProperty : public MatcherImpl {
    protected:
        std::unique_ptr<MatcherImpl> construct() const override;
    public:
        bool operator()(libexpressions::ExpressionNodePtr const &ptr) const override;
    };

    Matcher IsEqual(libexpressions::ExpressionNodePtr const &ptr);
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
    Matcher HasNthChild(size_t n);
    Matcher AllChildrenExceptNth(size_t n);
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
    Matcher RecursiveUntilProperty(Matcher const &invariantMatcher);
}

