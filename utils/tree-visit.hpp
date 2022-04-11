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

namespace libexpressions {

enum class TreeTraversalOrder {
    PREFIX_TRAVERSAL,
    INFIX_TRAVERSAL,
    POSTFIX_TRAVERSAL,
    BREADTH_FIRST
};

template<typename Fn, typename NodeType>
auto treeTraversalFunctionAdaptor(Fn &&fn, NodeType &&node, std::vector<std::size_t> const &/*path*/)
->  std::enable_if_t<std::is_same_v<bool, std::decay_t<std::invoke_result_t<Fn(NodeType const&)>>>, bool>
{
    return fn(node);
}
template<typename Fn, typename NodeType>
auto treeTraversalFunctionAdaptor(Fn &&fn, NodeType &&node, std::vector<size_t> const &path)
->  std::enable_if_t<std::is_same_v<bool, std::decay_t<std::invoke_result_t<Fn(NodeType const&, std::vector<size_t> const&)>>>, bool>
{
    return fn(node, path);
}
template<typename Fn, typename NodeType>
auto treeTraversalFunctionAdaptor(Fn &&fn, NodeType &&node, std::vector<size_t> const &/*path*/)
->  std::enable_if_t<std::negation_v<std::is_same<bool, std::decay_t<std::invoke_result_t<Fn(NodeType)>>>>, bool>
{
    return fn(node);
}
template<typename Fn, typename NodeType>
bool treeTraversalFunctionAdaptor(Fn &&fn, NodeType &&node, std::vector<size_t> const &path)
/*->  std::enable_if_t<std::negation_v<std::is_same<bool, std::decay_t<std::invoke_result_t<Fn(NodeType, 
                                                        std::vector<size_t>)>>>>, bool>*/
{
    fn(node, path);
    return true;
}

template<typename NodePointer>
size_t getChildNodeIndex(NodePointer parent, NodePointer child);

template<typename NodePointer, typename IIterator1, typename IIterator2>
std::vector<size_t> calculatePathFromStack(std::vector<std::tuple<NodePointer, IIterator1, IIterator2>> const &stack) {
    std::vector<size_t> result;
    if(not stack.empty()) {
        result.reserve(stack.size()-1);
        NodePointer previousNode = std::get<0>(stack.at(0));
        for(auto &[ptr, _ignore0, _ignore1] : stack) {
            if(previousNode != ptr) { // root is on stack, we want to ignore it
                result.push_back(getChildNodeIndex(previousNode, ptr));
            }
            previousNode = ptr;
        }
    }
    return result;
}

template<TreeTraversalOrder order,
         typename ChildIteratorGetterFunction,
         typename NodeFunction,
         typename NodeType>
void traverseTree(ChildIteratorGetterFunction &&childGetter,
                  NodeFunction &&functionToCall,
                  NodeType const &paramTopNode) {
    typedef typename std::add_pointer<NodeType const>::type NodePointer;
    if constexpr ( order != TreeTraversalOrder::BREADTH_FIRST ) {

        auto [childrenBeginOfRoot, childrenEndOfRoot] = childGetter(paramTopNode);
        typedef std::tuple<NodePointer, std::decay_t<decltype(childrenBeginOfRoot)>, std::decay_t<decltype(childrenEndOfRoot)>> StackFrame;

        std::vector<StackFrame> stack;
        stack.emplace_back(&paramTopNode, childrenBeginOfRoot, childrenEndOfRoot);

        if constexpr ( order == TreeTraversalOrder::PREFIX_TRAVERSAL 
                    or (order == TreeTraversalOrder::INFIX_TRAVERSAL
                       and childrenBeginOfRoot == childrenEndOfRoot) ) {
            if( not treeTraversalFunctionAdaptor(functionToCall, paramTopNode, calculatePathFromStack(stack))) {
                return;
            }
        }

        while(not stack.empty()) {
            auto &[stackTopNode, stackChildrenBegin, stackChildrenEnd] = stack.back();
            
            if( stackChildrenBegin != stackChildrenEnd ) {
                auto nextTop = &*stackChildrenBegin;
                ++stackChildrenBegin;
                auto [nextChildrenBegin, nextChildrenEnd] = childGetter(*nextTop);
                stack.emplace_back(nextTop, nextChildrenBegin, nextChildrenEnd);

                if constexpr ( order == TreeTraversalOrder::PREFIX_TRAVERSAL 
                            or (order == TreeTraversalOrder::INFIX_TRAVERSAL
                               and nextChildrenBegin == nextChildrenEnd) ) {
                    if( not treeTraversalFunctionAdaptor(functionToCall, *nextTop, calculatePathFromStack(stack))) {
                        return;
                    }
                }
            } else {
                if constexpr ( order == TreeTraversalOrder::POSTFIX_TRAVERSAL ) {
                    if( not treeTraversalFunctionAdaptor(functionToCall, *stackTopNode, calculatePathFromStack(stack))) {
                        return;
                    }
                }
                stack.pop_back();
                if constexpr ( order == TreeTraversalOrder::INFIX_TRAVERSAL ) {
                    auto &[topNode, childrenBegin, childrenEnd] = stack.back();
                    if( not treeTraversalFunctionAdaptor(functionToCall, *topNode, calculatePathFromStack(stack))) {
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

            if( not treeTraversalFunctionAdaptor(functionToCall(*node, path)) ) {
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

