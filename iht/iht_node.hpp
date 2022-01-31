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
#ifndef __IHT_IHT_NODE__
#define __IHT_IHT_NODE__

#include <cstddef>

namespace std {
    template<class T> class shared_ptr;
}

// namespace for IHT (Immutable Hashed Tree)
namespace IHT {
    typedef std::size_t hash_type;
    template<typename NodeType>
    class IHTFactory;

    template<typename NodeType>
    class IHTNode {
        friend class IHTFactory<NodeType>;
    public:
        typedef NodeType node_type;
    protected:
        IHTNode() = default;
    public:
        ~IHTNode() = default;
        IHT::hash_type hash() const {
            return static_cast<node_type const*>(this)->hash();
        }
        bool equal_to(IHT::IHTNode<node_type> const *other) const {
            return static_cast<node_type const*>(this)->equal_to(static_cast<node_type const*>(other));
        }
    };

    template<typename NodeType>
    using IHTNodePtr = std::shared_ptr<IHT::IHTNode<NodeType> const>;

    template<typename NodeType>
    IHT::hash_type hashof(IHT::IHTNode<NodeType> const &node) {
        return node.hash();
    }
    template<typename NodeType>
    IHT::hash_type hashof(IHT::IHTNodePtr<NodeType> const &node) {
        return node->hash();
    }
}

namespace std {
    template<class T> struct hash;
    template<class T> struct equal_to;

    template<typename NodeType> struct hash<IHT::IHTNode<NodeType>> {
        IHT::hash_type operator()(IHT::IHTNode<NodeType> const &node) const {
            return IHT::hashof(node);
        }
    };
    template<typename NodeType> struct hash<IHT::IHTNodePtr<NodeType>> {
        IHT::hash_type operator()(IHT::IHTNodePtr<NodeType> const &node) const {
            return IHT::hashof(node);
        }
    };
    template<typename NodeType> struct equal_to<IHT::IHTNode<NodeType>> {
        bool operator()(IHT::IHTNode<NodeType> const &node1, IHT::IHTNode<NodeType> const &node2) const {
            return node1.equal_to(&node2)
            and node2.equal_to(&node1);
        }
    };
    template<typename NodeType> struct equal_to<IHT::IHTNodePtr<NodeType>> {
        bool operator()(IHT::IHTNodePtr<NodeType> const &node1, IHT::IHTNodePtr<NodeType> const &node2) const {
            return node1->equal_to(node2.get())
            and node2->equal_to(node1.get());
        }
    };
}

#endif //__IHT_IHT_NODE__

