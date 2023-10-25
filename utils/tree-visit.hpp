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

#include <tuple>
#include <vector>
#include <deque>
#include <type_traits>
#include <cstdlib>
#include <cassert>

namespace libexpressions {

enum class TreeTraversalOrder {
    PREFIX_TRAVERSAL,
    INFIX_TRAVERSAL,
    POSTFIX_TRAVERSAL,
    BREADTH_FIRST
};

template<typename Fn, typename NodeType, typename Stack, typename StackPathCalculator>
auto treeTraversalFunctionAdaptor(Fn &&fn, NodeType &&node, Stack &&/*stack*/, StackPathCalculator &&/*pathCalculator*/)
->  std::enable_if_t<std::is_invocable_r_v<bool, Fn, NodeType>, bool>
{
    return fn(node);
}
template<typename Fn, typename NodeType, typename Stack, typename StackPathCalculator>
auto treeTraversalFunctionAdaptor(Fn &&fn, NodeType &&node, Stack &&stack, StackPathCalculator &&pathCalculator)
->  std::enable_if_t<std::is_invocable_r_v<bool, Fn, NodeType, std::vector<size_t> const &>, bool>
{
    return fn(node, pathCalculator(stack));
}
template<typename Fn, typename NodeType, typename Stack, typename StackPathCalculator>
auto treeTraversalFunctionAdaptor(Fn &&fn, NodeType &&node, Stack &&/*stack*/, StackPathCalculator &&/*pathCalculator*/)
->  std::enable_if_t<std::conjunction_v<std::is_invocable<Fn, NodeType>,
                                        std::negation<std::is_invocable_r<bool, Fn, NodeType>>>, bool>
{
    fn(node);
    return true;
}
template<typename Fn, typename NodeType, typename Stack, typename StackPathCalculator>
auto treeTraversalFunctionAdaptor(Fn &&fn, NodeType &&node, Stack &&stack, StackPathCalculator &&pathCalculator)
->  std::enable_if_t<std::conjunction_v<std::is_invocable<Fn, NodeType, std::vector<size_t> const &>,
                                        std::negation<std::is_invocable_r<bool, Fn, NodeType, std::vector<size_t> const &>>>, bool>
{
    fn(node, pathCalculator(stack));
    return true;
}

template<typename NodePointer>
size_t getChildNodeIndex(NodePointer parent, NodePointer child);

template<typename NodePointer, typename IIterator1, typename IIterator2>
std::vector<size_t> calculatePathFromStack(std::vector<std::tuple<NodePointer, IIterator1, IIterator2, size_t>> const &stack) {
    std::vector<size_t> result;
    if(not stack.empty()) {
        result.reserve(stack.size()-1);
        NodePointer previousNode = std::get<0>(stack.at(0));
        for(auto &[ptr, _ignore0, _ignore1, index] : stack) {
            result.push_back(index - 1);
            previousNode = ptr;
        }
    }
    result.pop_back();
    return result;
}

template<TreeTraversalOrder order,
         typename ChildIteratorGetterFunction,
         typename NodeFunction,
         typename NodeType,
         std::enable_if_t<std::disjunction_v<std::is_invocable<NodeFunction, NodeType const &>,
                                             std::is_invocable<NodeFunction, NodeType const &, std::vector<size_t>>>, bool> = true>
void traverseTree(ChildIteratorGetterFunction &&childGetter,
                  NodeFunction &&functionToCall,
                  NodeType const &paramTopNode) {
    typedef typename std::add_pointer<NodeType const>::type NodePointer;
    if constexpr ( order != TreeTraversalOrder::BREADTH_FIRST ) {

        auto [childrenBeginOfRoot, childrenEndOfRoot] = childGetter(paramTopNode);
        typedef std::decay_t<decltype(childrenBeginOfRoot)> IIterator1;
        typedef std::decay_t<decltype(childrenEndOfRoot)> IIterator2;
        // Last is index of next node to visit in the frames child nodes (IIterator1 + size_t)
        typedef std::tuple<NodePointer, IIterator1, IIterator2, size_t> StackFrame;

        std::vector<StackFrame> stack;
        stack.emplace_back(&paramTopNode, childrenBeginOfRoot, childrenEndOfRoot, 0);

        if constexpr ( order == TreeTraversalOrder::PREFIX_TRAVERSAL 
                    or (order == TreeTraversalOrder::INFIX_TRAVERSAL
                       and childrenBeginOfRoot == childrenEndOfRoot) ) {
            if( not treeTraversalFunctionAdaptor(functionToCall, paramTopNode, stack, calculatePathFromStack<NodePointer, IIterator1, IIterator2>)) {
                return;
            }
        }

        while(not stack.empty()) {
            auto &[stackTopNode, stackChildrenBegin, stackChildrenEnd, indexTop] = stack.back();
            
            if( auto nextChildIter = stackChildrenBegin + indexTop; nextChildIter != stackChildrenEnd ) {
                auto nextChild = &*nextChildIter;
                ++indexTop;
                auto [nextChildrenBegin, nextChildrenEnd] = childGetter(*nextChild);
                stack.emplace_back(nextChild, nextChildrenBegin, nextChildrenEnd, 0);

                if constexpr ( order == TreeTraversalOrder::PREFIX_TRAVERSAL 
                            or (order == TreeTraversalOrder::INFIX_TRAVERSAL
                               and nextChildrenBegin == nextChildrenEnd) ) {
                    if( not treeTraversalFunctionAdaptor(functionToCall, *nextChild, stack, calculatePathFromStack<NodePointer, IIterator1, IIterator2>)) {
                        return;
                    }
                }
            } else {
                if constexpr ( order == TreeTraversalOrder::POSTFIX_TRAVERSAL ) {
                    if( not treeTraversalFunctionAdaptor(functionToCall, *stackTopNode, stack, calculatePathFromStack<NodePointer, IIterator1, IIterator2>)) {
                        return;
                    }
                }
                stack.pop_back();
                if constexpr ( order == TreeTraversalOrder::INFIX_TRAVERSAL ) {
                    auto &[topNode, childrenBegin, childrenEnd, index] = stack.back();
                    if( not treeTraversalFunctionAdaptor(functionToCall, *topNode, stack, calculatePathFromStack<NodePointer, IIterator1, IIterator2>)) {
                        return;
                    }
                }
            }
        }
    } else {
        std::deque<NodePointer, std::vector<NodePointer>> workQueue;
        workQueue.emplace_back(&paramTopNode, {});

        while( not workQueue.empty() ) {
            auto [node, path] = workQueue.front();

            if( not treeTraversalFunctionAdaptor(functionToCall, *node, path, [](auto const &x) { return x; }) ) {
                return;
            }

            auto [childrenBegin, childrenEnd] = childGetter(node);
            path.push_back(node);
            for(auto iter = childrenBegin; iter != childrenEnd; ++iter) {
                workQueue.emplace_back(&*iter, path);
            }
        }
    }
}

}

