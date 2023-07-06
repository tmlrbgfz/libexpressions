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

#include "libexpressions/iht/iht_node.hpp"
#include <unordered_map>
#include <memory>
#include <type_traits>
#include <vector>
#include <algorithm>
#include <mutex>
#include <shared_mutex>
#include <optional>
#include <cassert>
#include <iostream>

namespace IHT {
    template<typename NodeType>
    class IHTFactory {
    private: //private typedefs
        typedef typename IHT::IHTNodePtr<NodeType>::weak_type IHTWeakNodePtr;
        typedef typename IHT::IHTNode<NodeType> const * IHTNodePtr;
        typedef std::vector<std::tuple<IHTNodePtr, typename IHT::IHTFactory<NodeType>::IHTWeakNodePtr>> IHTWeakNodePtrContainer;
    private: //private member functions
        //Not thread safe
        template<typename SpecialisedType, typename Deleter>
        IHT::IHTNodePtr<NodeType> findOrInsertNode(std::unique_ptr<SpecialisedType, Deleter> &&node);

        template<typename Deleter>
        decltype(auto) getNodeDeleter(Deleter &&deleter) {
            return [deleter,this](IHT::IHTNode<NodeType> *node) {
                       if(node != nullptr) {
                           this->unregisterNode(node);
                           deleter(static_cast<NodeType*>(node));
                       }
                   };
        }

        template<typename Deleter>
        using deleter_type = typename std::invoke_result<decltype(&IHT::IHTFactory<NodeType>::getNodeDeleter<Deleter>), IHT::IHTFactory<NodeType>*, Deleter>::type;

        void unregisterNode(IHT::IHTNode<NodeType> *node) {
            auto const hash = node->hash();
            std::shared_lock<std::shared_mutex> nodesLock(this->nodesMutex);
            auto &[ptrs, hashMutex] = nodes[hash];
            std::unique_lock<std::shared_mutex> hashLock(hashMutex);
            for(typename IHT::IHTFactory<NodeType>::IHTWeakNodePtrContainer::iterator iter = ptrs.begin();
                iter != ptrs.end(); ++iter) {
                auto &[ptr, weakPtr] = *iter;
                //If the node is in here or if there is a nullptr
                //  Note: The order for weak_ptr expiration and deleter call is undefined by the current standards. (https://cplusplus.github.io/LWG/issue2751)
                //        Therefore, the weak_ptr we are looking for can either still be locked and return the pointer to the node or will be expired.
                //        Thus, we erase the correct weak_ptr if we find it or any nullptr's we find on the way.
                if(ptr == node or weakPtr.expired()) {
                    ptrs.erase(iter);
                    break;
                }
            }
            hashLock.unlock();
            if(ptrs.empty()) {
                nodesLock.unlock(); //Unlocking to lock unique to modify node hash map
                // Here, we assume anything can happen and someone else might
                // insert another node with this hash or delete this element of
                // the node hash map.
                std::unique_lock<std::shared_mutex> exclusiveNodesLock(this->nodesMutex);
                // We need to check everything again and assume, the node hash
                // map has been modified.
                if(nodes.count(hash) > 0) {
                    auto &[ptrsExcl, hashExcl] = nodes.at(hash);
                    std::unique_lock<std::shared_mutex> exclusiveHashLock(hashExcl);
                    if(ptrsExcl.empty()) {
                        nodes.erase(hash);
                        // Hash mutex has been erased, just release it
                        exclusiveHashLock.release();
                    }
                }
            }
        }

        std::optional<IHT::IHTNodePtr<NodeType>> findEquivalentNodeAssumeLocked(IHT::IHTNode<NodeType> const *node) const {
            if(node == nullptr) {
                return std::nullopt;
            }
            auto const hash = node->hash();
            if(nodes.count(hash) > 0) {
                auto &[nodesWithThisHash, hashMutex] = nodes.at(hash);
                auto found = std::find_if(nodesWithThisHash.cbegin(),
                                   nodesWithThisHash.cend(),
                                   [&node](auto const &ptrNode) {
                    auto &[ptr, weakPtr] = ptrNode;
                    return not weakPtr.expired() and node->equal_to(ptr);
                });
                if(found != nodesWithThisHash.cend()) {
                    auto &[_, weakPtr] = *found;
                    return weakPtr.lock();
                }
            }
            return std::nullopt;
        }

        // Doing the same as `findEquivalentNodeAssumeLocked` but with mutex
        // locking. Not just locking in this function and then calling
        // `findEquivalentNodeAssumeLocked` could be considered a performance
        // optimisation but is at most a minor one. Doing it anyways and
        // leaving this comment.
        std::optional<IHT::IHTNodePtr<NodeType>> findEquivalentNode(IHT::IHTNode<NodeType> const *node) {
            if(node == nullptr) {
                return std::nullopt;
            }
            auto const hash = node->hash();
            std::shared_lock<std::shared_mutex> nodesLock(this->nodesMutex);
            if(nodes.count(hash) > 0) {
                auto &[nodesWithThisHash, hashMutex] = nodes.at(hash);
                std::shared_lock<std::shared_mutex> hashLock(hashMutex);
                auto found = std::find_if(nodesWithThisHash.cbegin(),
                                   nodesWithThisHash.cend(),
                                   [&node](auto const &ptrNode) {
                    auto &[ptr, weakPtr] = ptrNode;
                    return not weakPtr.expired() and node->equal_to(ptr);
                });
                if(found != nodesWithThisHash.cend()) {
                    auto &[_, weakPtr] = *found;
                    return weakPtr.lock();
                }
            }
            return std::nullopt;
        }
    private: //private members
        static std::unique_ptr<IHT::IHTFactory<NodeType>> singletonInstance;

        std::unordered_map<IHT::hash_type, std::tuple<IHTWeakNodePtrContainer, std::shared_mutex>> nodes;
        std::shared_mutex nodesMutex;
    public:
        static IHTFactory<NodeType> *get() {
            if(singletonInstance == nullptr) {
                singletonInstance.reset(new IHTFactory());
            }
            return singletonInstance.get();
        }

        //Constructors and destructors of NodeType should not have side effects
        //as temporary objects are created and might be destroyed.
        template<typename SpecialisedType, class ... Args>
        std::optional<IHT::IHTNodePtr<NodeType>> tryCreateNewNode(Args&& ...args) {
            return this->tryCreateNewNodeWithDeleter<SpecialisedType>(std::default_delete<NodeType>{}, std::forward<Args>(args)...);
        }

        //Constructors and destructors of NodeType should not have side effects
        //as temporary objects are created and might be destroyed.
        template<typename SpecialisedType, class ... Args>
        IHT::IHTNodePtr<NodeType> createNode(Args&& ...args) {
            return this->createNodeWithDeleter<SpecialisedType>(std::default_delete<NodeType>{}, std::forward<Args>(args)...);
        }

        //Constructors and destructors of NodeType should not have side effects
        //as temporary objects are created and might be destroyed.
        //The deleter is called on a pointer to an object of type
        //NodeType and shall take care of freeing memory.
        //Note: If there already is an equivalent node produced by this factory
        //but with a different or no deleter, that note will be returned and
        //the deleter given here will be invoked immediately.
        template<typename SpecialisedType, typename Deleter, class ... Args>
        std::optional<IHT::IHTNodePtr<NodeType>> tryCreateNewNodeWithDeleter(Deleter &&deleter, Args&& ...args) {
            static_assert(std::is_base_of<IHT::IHTNode<NodeType>, NodeType>::value == true, "IHTFactory cannot create objects of types not derived from IHTNode.");
            static_assert(std::is_base_of<NodeType, SpecialisedType>::value == true, "IHTFactory cannot create objects of types not derived from IHTNode.");

            //Create a node with the given arguments
            auto newNodePtr = new SpecialisedType(std::forward<Args>(args)...);
            //Create a node with the given arguments
            auto inserted = this->findOrInsertNode(std::unique_ptr<SpecialisedType, deleter_type<Deleter>>(newNodePtr,
                                    this->getNodeDeleter(std::forward<Deleter>(deleter))));
            if(newNodePtr == inserted.get()) {
                return inserted;
            } else {
                return std::nullopt;
            }
        }

        //Constructors and destructors of NodeType should not have side effects
        //as temporary objects are created and might be destroyed.
        //The deleter is called on a pointer to an object of type
        //NodeType and shall take care of freeing memory.
        //Note: If there already is an equivalent node produced by this factory
        //but with a different or no deleter, that note will be returned and
        //the deleter given here will be invoked immediately.
        template<typename SpecialisedType, typename Deleter, class ... Args>
        IHT::IHTNodePtr<NodeType> createNodeWithDeleter(Deleter &&deleter, Args&& ...args) {
            static_assert(std::is_base_of<IHT::IHTNode<NodeType>, NodeType>::value == true, "IHTFactory cannot create objects of types not derived from IHTNode.");
            static_assert(std::is_base_of<NodeType, SpecialisedType>::value == true, "IHTFactory cannot create objects of types not derived from IHTNode.");
            //Create a node with the given arguments
            return this->findOrInsertNode(std::unique_ptr<SpecialisedType, deleter_type<Deleter>>(new SpecialisedType(std::forward<Args>(args)...),
                                          this->getNodeDeleter(std::forward<Deleter>(deleter))));
        }

        bool hasEquivalentNode(IHT::IHTNode<NodeType> const *node) {
            return this->findEquivalentNode(node).has_value();
        }
        bool hasEquivalentNode(IHT::IHTNodePtr<NodeType> const &node) {
            return this->hasEquivalentNode(node.get());
        }
    };

    template<typename NodeType> std::unique_ptr<IHT::IHTFactory<NodeType>> IHTFactory<NodeType>::singletonInstance;

    // Implementation of long methods from class template
    template<typename NodeType>
    template<typename SpecialisedType, typename Deleter>
    IHT::IHTNodePtr<NodeType> IHTFactory<NodeType>::findOrInsertNode(std::unique_ptr<SpecialisedType, Deleter> &&node) {
        if(node == nullptr) {
            return nullptr;
        }
        //If no node was found, insert the created node into the node map
        //and return the pointer
        if(auto eqNode = this->findEquivalentNode(node.get());
            not eqNode.has_value()) {
            IHT::IHTNodePtr<NodeType> toInsert(node.get(), node.get_deleter());
            node.release();
            auto const hash = toInsert->hash();
            std::unique_lock<std::shared_mutex> nodesLock(this->nodesMutex);
            auto &[ptrs, hashMutex] = nodes[hash];
            {
                std::unique_lock<std::shared_mutex> hashLock(hashMutex);
                if(not this->findEquivalentNodeAssumeLocked(node.get()).has_value()) {
                    ptrs.emplace_back(toInsert.get(), toInsert);
                }
            }
            assert(this->findEquivalentNodeAssumeLocked(toInsert.get()).has_value());
            return toInsert;
        } else {
            return eqNode.value();
        }
    }
}

