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

#include "libexpressions/matchers/matchers.hpp"

namespace libexpressions {
    std::unique_ptr<MatcherImpl> MatcherImpl::construct(std::unique_ptr<MatcherImpl> &&matcher) const {
        auto val = this->construct();
        val->nested = std::move(matcher);
        return val;
    }
    std::unique_ptr<MatcherImpl> MatcherImpl::operator()(std::unique_ptr<MatcherImpl> &&matcher) const {
        return this->construct(std::move(matcher));
    }
    std::unique_ptr<MatcherImpl> MatcherImpl::operator()(Matcher const &matcher) const {
        return this->operator()(matcher.getImpl()->clone());
    }

    std::unique_ptr<MatcherImpl> MatcherImpl::clone() const {
        if(nested != nullptr) {
            return this->construct(nested->clone());
        } else {
            return this->construct(std::unique_ptr<MatcherImpl>{});
        }
    }

    Matcher::Matcher(std::unique_ptr<MatcherImpl> &&paramMatcher) : matcher(std::move(paramMatcher)) {}
    Matcher::Matcher(Matcher const &other) : matcher(other.matcher->clone()) {}

    bool Matcher::operator()(ExpressionNodePtr const &ptr) const {
        return matcher->operator()(ptr);
    }
    Matcher Matcher::operator()(Matcher const &other) const {
        return { matcher->operator()(other.matcher->clone()) };
    }

    std::unique_ptr<MatcherImpl> const &Matcher::getImpl() const {
        return this->matcher;
    }
}

