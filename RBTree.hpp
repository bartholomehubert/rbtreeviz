#pragma once

#include <iostream>
#include <vector>
#include <random>
#include <algorithm>

template <typename K, typename V>
class RBTree {
public:
    class Node {
    public:
        K key;
        V val;
        Node *parent;
        Node *left;
        Node *right;
        bool isRed;
        int rank;

        Node(Node&& other) = delete;
        Node &operator=(Node&& other) = delete;
        Node(const Node &other) = delete;
        Node &operator=(const Node &other) = delete;

        ~Node() {
            delete left;
            delete right;
        }

        bool isLeaf() const { return left == nullptr && right == nullptr; }

        bool isLeftChild() const {
            if (parent == nullptr)
                return false;
            return this == parent->left;
        }

        bool isRightChild() const {
            if (parent == nullptr)
                return false;
            return this == parent->right;
        }

        Node *sibling() {
            if (parent == nullptr) {
                return nullptr;
            }

            if (parent->left == this) {
                return parent->right;
            }
            return parent->left;
        }

        Node *grandParent() {
            if (parent == nullptr)
                return nullptr;
            return parent->parent;
        }

        Node *uncle() {
            Node *gp = grandParent();
            if (gp == nullptr)
                return nullptr;
            if (gp->left == parent) {
                return gp->right;
            }
            return gp->left;
        }

        friend std::ostream &operator<<(std::ostream &os, const RBTree::Node &node) {
            std::string color = node.isRed ? "red" : "black";
            return os << "Node{key: " << node.key << ", color: " << color << ", rank: " << node.rank << ", parent: " << node.parent << ", left: " << node.left <<", right: " << node.right << "} @" << &node << "\n";
        }
    };

    size_t size = 0;
    Node *root = nullptr;

    RBTree() = default;
    ~RBTree() { delete root; }

    RBTree(const RBTree &other) = delete;
    RBTree(RBTree &&other) noexcept
        : size{other.size}, root{other.root} {
        other.size = 0;
        other.root = nullptr;
    }

    RBTree &operator=(const RBTree &other) = delete;
    RBTree &operator=(RBTree &&other) noexcept {
        if (this != &other) {
            delete root;
            size = other.size;
            root = other.root;
            other.size = 0;
            other.root = nullptr;
        }
        return *this;
    }

    Node *search(K key) const {
        Node *node = root;
        while (node != nullptr) {
            if (key == node->key) {
                return node;
            } else if (key < node->key) {
                node = node->left;
            } else {
                node = node->right;
            }
        }
        return nullptr;
    }

    Node *insert(K key, V val) {
        if (root == nullptr) {
            size = 1;
            root = new Node{key, val, nullptr, nullptr, nullptr, false, 1};
            return root;
        }

        Node *parent = nullptr;
        Node *node = root;
        while (node != nullptr) {
            if (key == node->key) {
                return node;
            }

            parent = node;
            if (key < node->key) {
                node = node->left;
            } else {
                node = node->right;
            }
        }

        node = new Node{key, val, parent, nullptr, nullptr, true, parent->rank};

        if (key < parent->key) {
            parent->left = node;
        } else {
            parent->right = node;
        }

        fix(node);

        size++;
        return node;
    }

    static RBTree createRandom(int n) {
        std::random_device rd;
        std::mt19937 g(rd());
        std::vector<K> vec{};

        K key = 0;
        std::uniform_int_distribution<int> distrib(1, 2);
        for (int i = 0; i < n; ++i) {
            key += distrib(g);
            vec.push_back(key);
        }
        std::shuffle(vec.begin(), vec.end(), g);

        RBTree tree{};
        for (K x : vec) {
            tree.insert(x, V{});
        }
        return tree;
    }

private:
    void fix(Node *node) {

        while (node->grandParent() && node->parent->isRed && node->uncle() && node->uncle()->isRed) {
            node = node->grandParent();
            node->rank++;
            node->isRed = true;
            node->left->isRed = false;
            node->right->isRed = false;
        }

        if (node == root) {
            node->isRed = false;
            return;
        }

        if (node->parent == root && node->sibling() && node->sibling()->isRed) {
            node->parent->rank++;
            node->isRed = false;
            node->sibling()->isRed = false;
            return;
        }

        if (!node->parent->isRed) {
            return;
        }

        if (node->parent->isRed && (node->uncle()  == nullptr || !node->uncle()->isRed)) {
            rotate(node);
            return;
        }
    }

    void rotate(Node *node) {
        Node *alpha = node;
        Node *beta = node->parent;
        Node *gamma = node->grandParent();
        bool isLinear = node->isLeftChild() == node->parent->isLeftChild();

        if (!isLinear) {
            std::swap(alpha, beta);
        }

        if (gamma->parent) {
            if (gamma->isLeftChild()) {
                gamma->parent->left = beta;
            } else {
                gamma->parent->right = beta;
            }
        } else {
            root = beta;
        }
        beta->parent = gamma->parent;

        Node *c;
        Node *orphan; // either B or D
        if (node->key < gamma->key) {
            c = beta->right;
            // orphan is B
            orphan = isLinear ? alpha->right : beta->left;
            beta->left = alpha;
            beta->right = gamma;

            alpha->right = orphan;
            gamma->left = c;
        } else {
            c = beta->left;
            // orphan is D
            orphan = isLinear ? alpha->left : beta->right;
            beta->left = gamma;
            beta->right = alpha;

            alpha->left = orphan;
            gamma->right = c;
        }
        alpha->parent = beta;
        gamma->parent = beta;

        if (c)
            c->parent = gamma;
        if (orphan)
            orphan->parent = alpha;

        beta->isRed = false;
        gamma->isRed = true;
    }
};
