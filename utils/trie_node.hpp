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
#include <memory>
#include <optional>

namespace libexpressions {

template<typename Key, typename Data>
class TrieNode {
public:
    typedef TrieNode<Key, Data> TrieNodeType;
private:
    typedef std::unordered_map<Key, std::unique_ptr<TrieNodeType>> InternalContainerType;
public:
    typedef typename InternalContainerType::iterator iterator;
    typedef typename InternalContainerType::const_iterator const_iterator;
private:
    std::optional<Data> data;
    mutable InternalContainerType descendants;
public:
    operator Data() const {
        return data.value();
    }
    Data const& value() const {
        return data.value();
    }
    Data& value() {
        return const_cast<Data&>(static_cast<TrieNodeType const*>(this)->value());
    }
    TrieNodeType &operator=(Data const &d) {
        this->data = d;
        return *this;
    }
    bool hasValue() const {
        return this->data.has_value();
    }
    TrieNodeType const &operator[](Key const &index) const {
        if(this->descendants[index] == nullptr) {
            this->descendants[index].reset(new TrieNodeType);
        }
        return *this->descendants[index];
    }
    TrieNodeType &operator[](Key const &index) {
        return const_cast<TrieNodeType&>(static_cast<TrieNodeType const*>(this)->operator[](index));
    }
    TrieNodeType const &at(Key const &index) const {
        return *this->descendants.at(index);
    }
    TrieNodeType &at(Key const &index) {
        return const_cast<TrieNodeType&>(static_cast<TrieNodeType const*>(this->at(index)));
    }
    TrieNodeType const &operator[](std::vector<Key> const &index) const {
        TrieNodeType const *ptr = this;
        for(auto const idx : index) {
            ptr = &ptr->operator[](idx);
        }
        return *ptr;
    }
    TrieNodeType &operator[](std::vector<Key> const &index) {
        return const_cast<TrieNodeType&>(static_cast<TrieNodeType const*>(this)->operator[](index));
    }
    TrieNodeType const &at(std::vector<Key> const &index) const {
        TrieNodeType const *ptr = this;
        for(auto const idx : index) {
            ptr = ptr->descendants.at(idx).get();
        }
        return *ptr;
    }
    TrieNodeType &at(std::vector<Key> const &index) {
        return const_cast<TrieNodeType&>(static_cast<TrieNodeType const*>(this)->at(index));
    }

    size_t size() const {
        return this->descendants.size();
    }
    
    const_iterator begin() const {
        return this->descendants.begin();
    }
    iterator begin() {
        return this->descendants.begin();
    }
    const_iterator cbegin() const {
        return this->descendants.cbegin();
    }
    const_iterator end() const {
        return this->descendants.end();
    }
    iterator end() {
        return this->descendants.end();
    }
    const_iterator cend() const {
        return this->descendants.cend();
    }

    bool contains(Key const &index) const {
        return this->descendants.count(index) > 0;
    }
    bool contains(std::vector<Key> const &index) const {
        TrieNodeType const *ptr = this;
        for(auto const idx : index) {
            if(ptr->contains(idx)) {
                ptr = &ptr->at(idx);
            } else {
                return false;
            }
        }
        return true;
    }
    bool containsData(Key const &index) const {
        return this->contains(index) and this->at(index).hasValue();
    }
    bool containsData(std::vector<Key> const &index) const {
        return this->contains(index) and this->at(index).hasValue();
    }
    bool prefixContainsData(std::vector<Key> index) const {
        while(not index.empty()) {
            if(this->containsData(index)) {
                return true;
            }
            index.pop_back();
        }
        return false;
    }
    std::vector<Key> getLongestPrefixContainingData(std::vector<Key> index) const {
        while(not index.empty()) {
            if(this->containsData(index)) {
                return index;
            }
            index.pop_back();
        }
        return index;
    }
};

}

