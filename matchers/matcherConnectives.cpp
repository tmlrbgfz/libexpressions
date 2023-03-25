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

#include "libexpressions/matchers/matcherConnectives.hpp"

namespace libexpressions::Matchers {
    /// MultiMatcher
    void MultiMatcher::addMatcher(MultiMatcher *obj, std::unique_ptr<libexpressions::MatcherImpl> &&impl) const {
        assert(obj != nullptr);
        obj->matchers.emplace_back(std::move(impl));
    }
    void MultiMatcher::addMatcher(MultiMatcher *obj, std::unique_ptr<libexpressions::MatcherImpl> const &impl) const {
        this->addMatcher(obj, impl->clone());
    }
    void MultiMatcher::addMatcher(MultiMatcher *obj, Matcher const &matcher) const {
        this->addMatcher(obj, matcher.getImpl()->clone());
    }

    std::unique_ptr<MatcherImpl> MultiMatcher::clone() const {
        auto val = libexpressions::MatcherImpl::clone();
        MultiMatcher *casted = static_cast<MultiMatcher*>(val.get());
        if(!matchers.empty()) {
            for(auto const &m : this->matchers) {
                casted->matchers.push_back(m->clone());
            }
        }
        return val;
    }

    /// MatcherConjunction
    std::unique_ptr<MatcherImpl> MatcherConjunction::construct() const {
        return std::unique_ptr<MatcherImpl>(new MatcherConjunction);
    }
    bool MatcherConjunction::operator()(libexpressions::ExpressionNodePtr const &ptr) const {
        for(auto &matcher : matchers) {
            if(not matcher->operator()(ptr)) {
                return false;
            }
        }
        return true;
    }

    /// MatcherDisjunction
    std::unique_ptr<MatcherImpl> MatcherDisjunction::construct() const {
        return std::unique_ptr<MatcherImpl>(new MatcherDisjunction);
    }
    bool MatcherDisjunction::operator()(libexpressions::ExpressionNodePtr const &ptr) const {
        for(auto &matcher : matchers) {
            if(matcher->operator()(ptr)) {
                return true;
            }
        }
        return false;
    }

    /// MatcherNegation
    std::unique_ptr<MatcherImpl> MatcherNegation::construct() const {
        return std::unique_ptr<MatcherImpl>(new MatcherNegation);
    }
    bool MatcherNegation::operator()(libexpressions::ExpressionNodePtr const &ptr) const {
        return nested->operator()(ptr) == false;
    }
}

