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

#include <algorithm>
#include "matchers/matchers.hpp"

namespace libexpressions::Matchers {
    class MultiMatcher : public libexpressions::MatcherImpl {
    protected:
        std::vector<std::unique_ptr<MatcherImpl>> matchers;
    public:
        template<typename ...Args>
        std::unique_ptr<MatcherImpl> operator()(Args&&... matchers) const {
            auto obj = this->construct();
            auto matcher = dynamic_cast<MultiMatcher*>(obj.get());
            Expects(matcher != nullptr);
            ((matcher->matchers.push_back(matchers->clone())), ...);
            return obj;
        }
        virtual std::unique_ptr<MatcherImpl> clone() const override {
            auto val = libexpressions::MatcherImpl::clone();
            MultiMatcher *casted = static_cast<MultiMatcher*>(val.get());
            if(!matchers.empty()) {
                for(auto const &m : this->matchers) {
                    casted->matchers.push_back(m->clone());
                }
            }
            return val;
        }
    };
    class MatcherConjunction : public MultiMatcher {
    protected:
        virtual std::unique_ptr<MatcherImpl> construct() const override {
            return std::unique_ptr<MatcherImpl>(new MatcherConjunction);
        }
    public:
        virtual bool operator()(libexpressions::ExpressionNodePtr const &ptr) const override {
            for(auto &matcher : matchers) {
                if(not matcher->operator()(ptr)) {
                    return false;
                }
            }
            return true;
        }

        using MultiMatcher::operator();
    };
    class MatcherDisjunction : public MultiMatcher {
    protected:
        virtual std::unique_ptr<MatcherImpl> construct() const override {
            return std::unique_ptr<MatcherImpl>(new MatcherDisjunction);
        }
    public:
        virtual bool operator()(libexpressions::ExpressionNodePtr const &ptr) const override {
            for(auto &matcher : matchers) {
                if(matcher->operator()(ptr)) {
                    return true;
                }
            }
            return false;
        }
        using MultiMatcher::operator();
    };
    class MatcherNegation : public libexpressions::MatcherImpl {
    protected:
        virtual std::unique_ptr<MatcherImpl> construct() const override {
            return std::unique_ptr<MatcherImpl>(new MatcherNegation);
        }
    public:
        virtual bool operator()(libexpressions::ExpressionNodePtr const &ptr) const override {
            return nested->operator()(ptr) == false;
        }
    };
    
    MatcherConjunction And;
    MatcherDisjunction Or;
    MatcherNegation Not;
}

