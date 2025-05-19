// BPlusTree.tpp
// Implementation for a production-ready, page-size-sensitive B+Tree.

#pragma once
#include "BPlusTree.h"
#include <stdexcept>

template<typename Key, typename Value>
BPlusTree<Key, Value>::BPlusTree(std::size_t nodeOrder)
    : root_(std::make_unique<LeafNode>(nodeOrder)),
      size_(0), height_(1), nodeOrder_(nodeOrder),
      leftmostLeaf_(static_cast<LeafNode*>(root_.get())),
      rightmostLeaf_(static_cast<LeafNode*>(root_.get()))
{}

template<typename Key, typename Value>
BPlusTree<Key, Value>::~BPlusTree() {
    clear();
}

template<typename Key, typename Value>
void BPlusTree<Key, Value>::clear() {
    clearRecursive(root_);
    root_ = std::make_unique<LeafNode>(nodeOrder_);
    leftmostLeaf_ = static_cast<LeafNode*>(root_.get());
    rightmostLeaf_ = static_cast<LeafNode*>(root_.get());
    size_ = 0;
    height_ = 1;
}

template<typename Key, typename Value>
void BPlusTree<Key, Value>::insert(const Key& key, const Value& value) {
    if constexpr (!std::is_copy_constructible_v<Value>) {
        throw std::invalid_argument("Attempted to insert a non-copyable value into BPlusTree.");
    }
    Key upKey;
    std::unique_ptr<Node> newChild;
    if (insertRecursive(root_.get(), key, value, upKey, newChild, height_)) {
        // Node split reached root
        auto newRoot = std::make_unique<InternalNode>(nodeOrder_);
        newRoot->keys.push_back(upKey);
        newRoot->children.push_back(std::move(root_));
        newRoot->children.push_back(std::move(newChild));
        root_ = std::move(newRoot);
        height_++;
    }
    size_++;
}

template<typename Key, typename Value>
bool BPlusTree<Key, Value>::insertRecursive(Node* node, const Key& key, const Value& value, Key& upKey, std::unique_ptr<Node>& newChild, std::size_t curHeight) {
    if (node->isLeaf) {
        auto* leaf = static_cast<LeafNode*>(node);
        auto it = std::lower_bound(leaf->keys.begin(), leaf->keys.end(), key);
        auto idx = std::distance(leaf->keys.begin(), it);
        if (it != leaf->keys.end() && *it == key) {
            leaf->values[idx] = value;
            return false;
        }
        leaf->keys.insert(it, key);
        leaf->values.insert(leaf->values.begin() + idx, value);
        leaf->count++;
        if (leaf->keys.size() > nodeOrder_) {
            std::unique_ptr<LeafNode> newLeaf = std::make_unique<LeafNode>(nodeOrder_);
            splitLeaf(leaf, newLeaf, upKey);
            // Patch doubly-linked leaf list
            newLeaf->next = leaf->next;
            newLeaf->prev = leaf;
            if (leaf->next) leaf->next->prev = newLeaf.get();
            leaf->next = newLeaf.get();
            if (rightmostLeaf_ == leaf) rightmostLeaf_ = newLeaf.get();
            newChild = std::move(newLeaf);
            return true;
        }
        return false;
    } else {
        auto* internal = static_cast<InternalNode*>(node);
        auto idx = findChildIndex(internal->keys, key);
        Key childUpKey;
        std::unique_ptr<Node> childNewChild;
        if (insertRecursive(internal->children[idx].get(), key, value, childUpKey, childNewChild, curHeight - 1)) {
            internal->keys.insert(internal->keys.begin() + idx, childUpKey);
            internal->children.insert(internal->children.begin() + idx + 1, std::move(childNewChild));
            internal->count++;
            if (internal->keys.size() > nodeOrder_) {
                std::unique_ptr<InternalNode> newNode = std::make_unique<InternalNode>(nodeOrder_);
                splitInternal(internal, newNode, upKey);
                newChild = std::move(newNode);
                return true;
            }
        }
        return false;
    }
}

template<typename Key, typename Value>
void BPlusTree<Key, Value>::splitLeaf(LeafNode* node, std::unique_ptr<LeafNode>& newLeaf, Key& upKey) {
    std::size_t mid = node->keys.size() / 2;
    newLeaf->keys.assign(node->keys.begin() + mid, node->keys.end());
    newLeaf->values.assign(node->values.begin() + mid, node->values.end());
    newLeaf->count = newLeaf->keys.size();
    node->keys.resize(mid);
    node->values.resize(mid);
    node->count = node->keys.size();
    upKey = newLeaf->keys.front();
}

template<typename Key, typename Value>
void BPlusTree<Key, Value>::splitInternal(InternalNode* node, std::unique_ptr<InternalNode>& newNode, Key& upKey) {
    std::size_t mid = node->keys.size() / 2;
    upKey = node->keys[mid];
    newNode->keys.assign(node->keys.begin() + mid + 1, node->keys.end());
    newNode->children.reserve(node->children.size() - (mid + 1));
    for (auto it = node->children.begin() + mid + 1; it != node->children.end(); ++it) {
        newNode->children.push_back(std::move(*it));
    }
    newNode->count = newNode->keys.size();
    node->keys.resize(mid);
    node->children.resize(mid + 1);
    node->count = node->keys.size();
}

template<typename Key, typename Value>
std::optional<Value> BPlusTree<Key, Value>::find(const Key& key) const {
    if (!root_) throw std::runtime_error("Cannot find in an empty BPlusTree.");
    return findRecursive(root_.get(), key, height_);
}

template<typename Key, typename Value>
std::optional<Value> BPlusTree<Key, Value>::findRecursive(const Node* node, const Key& key, std::size_t curHeight) const {
    if (node->isLeaf) {
        auto* leaf = static_cast<const LeafNode*>(node);
        auto it = std::lower_bound(leaf->keys.begin(), leaf->keys.end(), key);
        if (it != leaf->keys.end() && *it == key) {
            return leaf->values[std::distance(leaf->keys.begin(), it)];
        }
        return std::nullopt;
    } else {
        auto* internal = static_cast<const InternalNode*>(node);
        auto idx = findChildIndex(internal->keys, key);
        return findRecursive(internal->children[idx].get(), key, curHeight - 1);
    }
}

template<typename Key, typename Value>
std::size_t BPlusTree<Key, Value>::size() const {
    return size_;
}

template<typename Key, typename Value>
std::size_t BPlusTree<Key, Value>::height() const {
    return height_;
}

template<typename Key, typename Value>
std::size_t BPlusTree<Key, Value>::findChildIndex(const std::vector<Key>& keys, const Key& key) const {
    // Find the first key > 'key'
    if (keys.empty()) throw std::runtime_error("Invalid operation: children index on empty keys vector in BPlusTree.");
    return std::distance(keys.begin(), std::upper_bound(keys.begin(), keys.end(), key));
}

template<typename Key, typename Value>
std::vector<std::pair<Key, Value>> BPlusTree<Key, Value>::range(const std::optional<Key>& lower,
                                                                const std::optional<Key>& upper) const
{
    std::vector<std::pair<Key, Value>> out;
    LeafNode* leaf = leftmostLeaf_;
    // Advance to lower bound:
    while (leaf) {
        auto it = lower ? std::lower_bound(leaf->keys.begin(), leaf->keys.end(), *lower) : leaf->keys.begin();
        for (; it != leaf->keys.end(); ++it) {
            auto idx = std::distance(leaf->keys.begin(), it);
            if (upper && *it >= *upper)
                return out;
            out.emplace_back(*it, leaf->values[idx]);
        }
        leaf = leaf->next;
    }
    return out;
}

template<typename Key, typename Value>
void BPlusTree<Key, Value>::clearRecursive(std::unique_ptr<Node>& node) {
    if (!node) return;
    if (!node->isLeaf) {
        auto* internal = static_cast<InternalNode*>(node.get());
        for (auto& child : internal->children)
            clearRecursive(child);
    }
    node.reset();
}

