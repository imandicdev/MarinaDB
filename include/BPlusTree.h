// BPlusTree.h
// Production-grade, page-size-sensitive, extensible B+Tree implementation for indexing
// No STL/Boost B+Tree exists as of C++20; this is a full, modern implementation.

#pragma once
#include <vector>
#include <memory>
#include <optional>
#include <stdexcept>
#include <algorithm>
#include <cstddef>
#include <limits>

// -- PAGE-SIZE SENSITIVITY AND NODE ORDER --
// User should specify node order (max number of keys) based on hardware or page size at tree construction.
// For real-world use, page size can be 4KB/8KB/etc, and order is set so a node fits one page.

template<typename Key, typename Value>
class BPlusTree {
public:
    // Node order: how many keys per node/page; should fit in page for on-disk B+Tree. Default is "safe" for 4k page and small keys.
    explicit BPlusTree(std::size_t nodeOrder = 128);
    ~BPlusTree();

    BPlusTree(const BPlusTree&) = delete;
    BPlusTree& operator=(const BPlusTree&) = delete;

    void insert(const Key& key, const Value& value);
    std::optional<Value> find(const Key& key) const;
    void erase(const Key& key);
    std::vector<std::pair<Key, Value>> range(const std::optional<Key>& lower = std::nullopt, const std::optional<Key>& upper = std::nullopt) const;
    void clear();

    // For diagnostics/monitoring:
    std::size_t size() const;
    std::size_t height() const;
    // Expose order/page size for admin
    std::size_t order() const { return nodeOrder_; }

private:
    struct Node;
    struct InternalNode;
    struct LeafNode;

    std::unique_ptr<Node> root_;
    std::size_t size_;
    std::size_t height_;
    std::size_t nodeOrder_;

    LeafNode* leftmostLeaf_;    // For range scan
    LeafNode* rightmostLeaf_;   // For reverse scan

    // -- MAIN NODE HIERARCHY --
    struct Node {
        bool isLeaf;
        std::size_t count;
        explicit Node(bool lf) : isLeaf(lf), count(0) {}
        virtual ~Node() = default;
    };

    struct InternalNode : Node {
        std::vector<Key> keys;
        std::vector<std::unique_ptr<Node>> children;
        InternalNode(std::size_t order)
            : Node(false)
        {
            keys.reserve(order);
            children.reserve(order + 1);
        }
    };

    struct LeafNode : Node {
        std::vector<Key> keys;
        std::vector<Value> values;
        LeafNode* next;
        LeafNode* prev;
        LeafNode(std::size_t order)
            : Node(true), next(nullptr), prev(nullptr)
        {
            keys.reserve(order);
            values.reserve(order);
        }
    };

    // -- INSERT, ERASE, & SPLIT/MERGE LOGIC --
    // All insert/erase are recursive and handle split/merge up to root.
    bool insertRecursive(Node* node, const Key& key, const Value& value, Key& upKey, std::unique_ptr<Node>& newChild, std::size_t curHeight);
    bool eraseRecursive(Node* node, const Key& key, std::size_t curHeight, bool& needsMerge);

    void splitLeaf(LeafNode* node, std::unique_ptr<LeafNode>& newLeaf, Key& upKey);
    void splitInternal(InternalNode* node, std::unique_ptr<InternalNode>& newNode, Key& upKey);

    void mergeLeaves(LeafNode* leftLeaf, LeafNode* rightLeaf);
    void mergeInternals(InternalNode* leftNode, InternalNode* rightNode, Key& separator);

    std::optional<Value> findRecursive(const Node* node, const Key& key, std::size_t curHeight) const;
    void rangeRecursive(const Node* node,
                        std::vector<std::pair<Key, Value>>& out,
                        const std::optional<Key>& lower,
                        const std::optional<Key>& upper,
                        std::size_t curHeight) const;
    void clearRecursive(std::unique_ptr<Node>& node);

    // Used by erase for rebalancing
    bool borrowFromLeft(Node* parent, std::size_t childIdx, std::size_t curHeight);
    bool borrowFromRight(Node* parent, std::size_t childIdx, std::size_t curHeight);

    // -- UTILITIES --
    std::size_t findChildIndex(const std::vector<Key>& keys, const Key& key) const;

    // (Optional: support for persistent serialization)
};
#include "BPlusTree.tpp"

