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
#ifndef __LIBEXPRESSIONS_PREDICATEMATCHER__
#define __LIBEXPRESSIONS_PREDICATEMATCHER__

#include <functional>

#include "libexpressions/matchers/matchers.hpp"

namespace libexpressions::Matchers {
    class PredicateMatcher : public libexpressions::MatcherImpl {
    public:
        typedef std::function<bool(libexpressions::ExpressionNodePtr)> PredicateType;
    protected:
        PredicateType predicate;
        std::unique_ptr<MatcherImpl> construct() const override {
            return std::unique_ptr<MatcherImpl>(new PredicateMatcher);
        }
        std::unique_ptr<MatcherImpl> construct(PredicateType paramPredicate) const {
            return std::unique_ptr<MatcherImpl>(new PredicateMatcher(paramPredicate));
        }
        using libexpressions::MatcherImpl::construct;
        PredicateMatcher(PredicateType paramPredicate) : predicate(paramPredicate) { }
    public:
        PredicateMatcher() = default;
        virtual bool operator()(libexpressions::ExpressionNodePtr const &ptr) const override {
            return predicate and predicate(ptr);
        }

        std::unique_ptr<libexpressions::MatcherImpl> operator()(PredicateType paramPredicate) {
            return this->construct(paramPredicate);
        }

        std::unique_ptr<MatcherImpl> clone() const override {
            if(predicate) {
                return this->construct(predicate);
            } else {
                return this->construct();
            }
        }
    };

    template<typename... Args>
    Matcher SatisfiesPredicate(Args&&...args) {
        PredicateMatcher matcher;
        return matcher(std::forward<Args>(args)...);
    }
}

#endif //__LIBEXPRESSIONS_PREDICATEMATCHER__
