
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

#include <unordered_map>
#include <map>
#include <vector>
#include <memory>

#include "libexpressions/utils/tree-visit.hpp"
#include "libexpressions/matchers/matchers.hpp"
#include "libexpressions/matchers/matcherConnectives.hpp"
#include "libexpressions/matchers/hierarchyMatchers.hpp"

namespace libexpressions {

inline ExpressionNodePtr replaceSubexpressionsByEquality(ExpressionFactory *factory, ExpressionNodePtr const &expressionToReplaceIn, std::unordered_map<ExpressionNodePtr, ExpressionNodePtr> const &expressionReplacementMap) {
    std::vector<Matcher> expressionMatchers;
    std::transform(expressionReplacementMap.cbegin(),
                   expressionReplacementMap.cend(),
                   std::back_inserter(expressionMatchers),
                   [](auto const &expression) {
        return Matchers::IsEqual(std::get<0>(expression));
    });
    Matcher matcher = Matchers::Or(expressionMatchers);
    std::map<Operator::Path, ExpressionNodePtr> locationReplacement;
    invokeUsingMatcher<TreeTraversalOrder::POSTFIX_TRAVERSAL>(expressionToReplaceIn, matcher, [&locationReplacement,&expressionReplacementMap](auto const &node, auto const &path) {
        locationReplacement[path] = expressionReplacementMap.at(node);
    });
    return factory->modifyExpression(expressionToReplaceIn, locationReplacement);
}

}

