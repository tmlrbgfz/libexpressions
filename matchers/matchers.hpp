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
#ifndef __LIBEXPRESSIONS_MATCHERS__
#define __LIBEXPRESSIONS_MATCHERS__

#include <functional>
#include <memory>
#include <deque>
#include <vector>
#include <type_traits>
#include <gsl/gsl_assert>
#include <tuple>

#include "libexpressions/expressions/expression_node.hpp"
#include "libexpressions/expressions/operator.hpp"
#include "libexpressions/utils/expression-tree-visit.hpp"

namespace libexpressions {
    class MatcherImpl {
    protected:
        std::unique_ptr<MatcherImpl> nested;
        virtual std::unique_ptr<MatcherImpl> construct() const = 0;
        virtual std::unique_ptr<MatcherImpl> construct(std::unique_ptr<MatcherImpl> &&matcher) const {
            auto val = this->construct();
            val->nested = std::move(matcher);
            return val;
        }
    public:
        virtual ~MatcherImpl() = default;
        virtual std::unique_ptr<MatcherImpl> operator()(std::unique_ptr<MatcherImpl> &&matcher) const final {
            return this->construct(std::move(matcher));
        }

        virtual bool operator()(ExpressionNodePtr const &ptr) const = 0;
        /*template<typename ...Args>
        std::unique_ptr<MatcherImpl> operator()(Args&&... args) const {
            return this->construct(std::forward<Args>(args)...);
        }*/

        virtual std::unique_ptr<MatcherImpl> clone() const {
            if(nested != nullptr) {
                return this->construct(nested->clone());
            } else {
                return this->construct(std::unique_ptr<MatcherImpl>{});
            }
        }
    };
    class Matcher final {
    private:
        std::unique_ptr<MatcherImpl> matcher;
    public:
        Matcher(std::unique_ptr<MatcherImpl> &&paramMatcher) : matcher(std::move(paramMatcher)) {}
        Matcher(Matcher const &other) : matcher(other.matcher->clone()) {}

        bool operator()(ExpressionNodePtr const &ptr) const {
            return matcher->operator()(ptr);
        }
        Matcher operator()(Matcher const &other) const {
            return { matcher->operator()(other.matcher->clone()) };
        }
    };

    // Invoke function `fn` on all expression nodes where a given matcher matches
    template<TreeTraversalOrder direction, typename Fn,
                                    typename std::enable_if<
                                        std::is_same_v<
                                            std::decay_t<
                                                std::invoke_result_t<Fn, ExpressionNodePtr const, Operator::Path const>
                                            >,
                                            bool
                                        >
                                    >::type* = nullptr>
    void invokeUsingMatcher(ExpressionNodePtr const &expression, Matcher m, Fn &&fn) {
        auto getSuccessors = [](ExpressionNodePtr const &nodePtr) {
            if( llvm::isa<Operator>(nodePtr.get()) ) {
                Operator const *ptr = llvm::dyn_cast<Operator const>(nodePtr.get());
                return std::make_tuple(ptr->begin(), ptr->end());
            }
            return std::make_tuple(libexpressions::Operator::Iterator(), libexpressions::Operator::Iterator());
        };
        auto functionToCall = [&m,&fn](ExpressionNodePtr const &nodePtr, Operator::Path const &path) -> bool {
            if(m(nodePtr)) {
                return fn(nodePtr, path);
            }
            return true;
        };
        traverseTree<direction>(getSuccessors, functionToCall, expression);
    }

    template<TreeTraversalOrder direction, typename Fn,
                                    typename std::enable_if<
                                        std::negation_v<
                                            std::is_same<
                                                std::decay_t<
                                                    std::invoke_result_t<Fn, ExpressionNodePtr const, Operator::Path const>
                                                >,
                                                bool
                                            >
                                        >
                                    >::type* = nullptr>
    void invokeUsingMatcher(ExpressionNodePtr const &expression, Matcher m, Fn &&fn) {
        invokeUsingMatcher<direction>(expression, m, [&fn](ExpressionNodePtr const &node, Operator::Path const &path) mutable -> bool {
            fn(node, path);
            return true;
        });
    }
}

#endif //__LIBEXPRESSIONS_MATCHERS__
