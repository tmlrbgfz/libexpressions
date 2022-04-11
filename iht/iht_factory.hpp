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
#include <optional>

namespace IHT {
    template<typename NodeType>
    class IHTFactory {
    private: //private typedefs
        typedef std::weak_ptr<typename IHT::IHTNodePtr<NodeType>::element_type> IHTWeakNodePtr;
        typedef std::vector<typename IHT::IHTFactory<NodeType>::IHTWeakNodePtr> IHTWeakNodePtrContainer;
    private: //private member functions
        //Not thread safe
        template<typename SpecialisedType, typename Deleter>
        std::optional<IHT::IHTNodePtr<NodeType>> insertNode(std::unique_ptr<SpecialisedType, Deleter> &&node, Deleter &&deleter = Deleter{});
        template<typename SpecialisedType, typename Deleter>
        IHT::IHTNodePtr<NodeType> findOrInsertNode(std::unique_ptr<SpecialisedType, Deleter> &&node, Deleter &&deleter = Deleter{});
    private: //private members
        static std::unique_ptr<IHT::IHTFactory<NodeType>> singletonInstance;

        std::unordered_map<IHT::hash_type, std::tuple<std::mutex, IHTWeakNodePtrContainer>> nodes;
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
            static_assert(std::is_base_of<IHT::IHTNode<NodeType>, SpecialisedType>::value == true, "IHTFactory cannot create objects of types not derived from IHTNode.");
            //Create a node with the given arguments
            return this->insertNode(std::unique_ptr<SpecialisedType>(new SpecialisedType(std::forward<Args>(args)...)));
        }

        //Constructors and destructors of NodeType should not have side effects
        //as temporary objects are created and might be destroyed.
        template<typename SpecialisedType, class ... Args>
        IHT::IHTNodePtr<NodeType> createNode(Args&& ...args) {
            static_assert(std::is_base_of<IHT::IHTNode<NodeType>, SpecialisedType>::value == true, "IHTFactory cannot create objects of types not derived from IHTNode.");
            //Create a node with the given arguments
            return this->findOrInsertNode(std::unique_ptr<SpecialisedType>(new SpecialisedType(std::forward<Args>(args)...)));
        }

        //Constructors and destructors of NodeType should not have side effects
        //as temporary objects are created and might be destroyed.
        template<typename SpecialisedType, typename Deleter, class ... Args>
        std::optional<IHT::IHTNodePtr<NodeType>> tryCreateNewNodeWithDeleter(Deleter &&deleter, Args&& ...args) {
            static_assert(std::is_base_of<IHT::IHTNode<NodeType>, SpecialisedType>::value == true, "IHTFactory cannot create objects of types not derived from IHTNode.");
            //Create a node with the given arguments
            return this->insertNode(std::unique_ptr<SpecialisedType, Deleter>(new SpecialisedType(std::forward<Args>(args)...)), std::forward<Deleter>(deleter));
        }

        //Constructors and destructors of NodeType should not have side effects
        //as temporary objects are created and might be destroyed.
        template<typename SpecialisedType, typename Deleter, class ... Args>
        IHT::IHTNodePtr<NodeType> createNodeWithDeleter(Deleter &&deleter, Args&& ...args) {
            static_assert(std::is_base_of<IHT::IHTNode<NodeType>, SpecialisedType>::value == true, "IHTFactory cannot create objects of types not derived from IHTNode.");
            //Create a node with the given arguments
            return this->findOrInsertNode(std::unique_ptr<SpecialisedType, Deleter>(new SpecialisedType(std::forward<Args>(args)...)), std::forward<Deleter>(deleter));
        }

        bool hasEquivalentNode(IHT::IHTNodePtr<NodeType> const &node) {
            if(node == nullptr) {
                return false;
            }
            auto &[mutex, nodesWithThisHash] = nodes.at(node->hash());
            std::unique_lock<std::mutex> lockForNodeContainer(mutex);
            return std::any_of(nodesWithThisHash.cbegin(),
                               nodesWithThisHash.cend(),
                               [&node](auto const &weakPtrNode) {
                return node->equal_to(weakPtrNode.lock().get());
            });
        }

        bool hasNode(IHT::IHTNodePtr<NodeType> const &node) {
            if(node == nullptr) {
                return false;
            }
            auto const &[mutex, nodesWithThisHash] = nodes.at(node->hash());
            std::unique_lock<std::mutex> lockForNodeContainer(mutex);
            return std::any_of(nodesWithThisHash.cbegin(),
                               nodesWithThisHash.cend(),
                               [&node](auto const &weakPtrNode) {
                return node == weakPtrNode.lock();
            });
        }
    };
    template<typename NodeType> std::unique_ptr<IHT::IHTFactory<NodeType>> IHTFactory<NodeType>::singletonInstance;

    // Implementation of long methods from class template
    template<typename NodeType>
    template<typename SpecialisedType, typename Deleter>
    IHT::IHTNodePtr<NodeType> IHTFactory<NodeType>::findOrInsertNode(std::unique_ptr<SpecialisedType, Deleter> &&node, Deleter &&deleter) {
        if(node == nullptr) {
            return nullptr;
        }
        //Determine the created nodes hash and search for the hash in the
        //node map
        auto const hash = node->hash();
        IHT::IHTNodePtr<NodeType> result;
        auto &[mutex, ptrs] = nodes[hash];
        std::vector<typename IHT::IHTFactory<NodeType>::IHTWeakNodePtrContainer::iterator> toDelete;
        //Iterate all node pointers in the node map with the given hash
        std::unique_lock<std::mutex> lockForNodeContainer(mutex);
        for(typename IHT::IHTFactory<NodeType>::IHTWeakNodePtrContainer::iterator iter = ptrs.begin();
            iter != ptrs.end(); ++iter) {
            //Determine the shared pointer of the node
            auto existingPtr = iter->lock();
            //If the node was not destroyed yet
            if(existingPtr) {
                //and if the node pointed to is equal to the created
                //node
                if(existingPtr->equal_to(node.get())) {
                    //Return the found node pointer
                    result = existingPtr;
                    break;
                }
            } else {
                //If the node was destroyed, mark the pointer for
                //deletion
                toDelete.push_back(iter);
            }
        }
        //Erase all encountered weak pointers to destroyed nodes
        for(auto &iter : toDelete) {
            ptrs.erase(iter);
        }
        //If no node was found, insert the created node into the node map
        //and return the pointer
        if(result == nullptr) {
            result.reset(node.release(), std::forward<Deleter>(deleter));
            ptrs.push_back(result);
        }
        return result;
    }
    // Implementation of long methods from class template
    template<typename NodeType>
    template<typename SpecialisedType, typename Deleter>
    std::optional<IHT::IHTNodePtr<NodeType>> IHTFactory<NodeType>::insertNode(std::unique_ptr<SpecialisedType, Deleter> &&node, Deleter &&deleter) {
        if(node == nullptr) {
            return nullptr;
        }
        //Determine the created nodes hash and search for the hash in the
        //node map
        auto const hash = node->hash();
        IHT::IHTNodePtr<NodeType> result;
        auto &[mutex, ptrs] = nodes[hash];
        std::vector<typename IHT::IHTFactory<NodeType>::IHTWeakNodePtrContainer::iterator> toDelete;
        //Iterate all node pointers in the node map with the given hash
        std::unique_lock<std::mutex> lockForNodeContainer(mutex);
        for(typename IHT::IHTFactory<NodeType>::IHTWeakNodePtrContainer::iterator iter = ptrs.begin();
            iter != ptrs.end(); ++iter) {
            //Determine the shared pointer of the node
            auto existingPtr = iter->lock();
            //If the node was not destroyed yet
            if(existingPtr) {
                //and if the node pointed to is equal to the created
                //node
                if(existingPtr->equal_to(node.get())) {
                    //Return the found node pointer
                    result = existingPtr;
                    break;
                }
            } else {
                //If the node was destroyed, mark the pointer for
                //deletion
                toDelete.push_back(iter);
            }
        }
        //Erase all encountered weak pointers to destroyed nodes
        for(auto &iter : toDelete) {
            ptrs.erase(iter);
        }
        //If no node was found, insert the created node into the node map
        //and return the pointer
        if(result == nullptr) {
            result.reset(node.release(), std::forward<Deleter>(deleter));
            ptrs.push_back(result);
            return result;
        } else {
            return std::nullopt;
        }
    }
}

