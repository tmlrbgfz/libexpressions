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

template<typename Data>
class TrieNode {
private:
    typedef std::unordered_map<size_t, std::unique_ptr<TrieNode<Data>>> InternalContainerType;
public:
    typedef typename InternalContainerType::iterator iterator;
    typedef typename InternalContainerType::const_iterator const_iterator;
private:
    std::optional<Data> data;
    InternalContainerType descendants;
public:
    operator Data() const {
        return data.value();
    }
    Data const& value() const {
        return data.value();
    }
    Data& value() {
        return const_cast<Data&>(static_cast<TrieNode<Data> const*>(this)->value());
    }
    TrieNode<Data> &operator=(Data const &d) {
        this->data = d;
        return *this;
    }
    bool hasValue() const {
        return this->data.has_value();
    }
    TrieNode<Data> &operator[](size_t index) {
        if(this->descendants[index] == nullptr) {
            this->descendants[index].reset(new TrieNode<Data>);
        }
        return *this->descendants[index];
    }
    TrieNode<Data> &at(size_t index) {
        return *this->descendants.at(index);
    }
    TrieNode<Data> &operator[](std::vector<size_t> const &index) {
        TrieNode<Data> *ptr = this;
        for(auto const idx : index) {
            ptr = &ptr->operator[](idx);
        }
        return *ptr;
    }
    TrieNode<Data> &at(std::vector<size_t> const &index) {
        TrieNode<Data> *ptr = this;
        for(auto const idx : index) {
            ptr = ptr->descendants.at(idx).get();
        }
        return *ptr;
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

    bool contains(size_t index) const {
        return this->descendants.count(index) > 0;
    }
    bool contains(std::vector<size_t> const &index) const {
        TrieNode<Data> *ptr = this;
        for(auto const idx : index) {
            if(ptr->contains(idx)) {
                ptr = ptr->at(idx);
            } else {
                return false;
            }
        }
        return true;
    }
    bool containsData(size_t const &index) const {
        return this->contains(index) and this->at(index).has_value();
    }
    bool prefixContainsData(std::vector<size_t> index) const {
        while(not index.empty()) {
            if(this->containsData(index)) {
                return true;
            }
            index.pop_back();
        }
        return false;
    }
};

}

