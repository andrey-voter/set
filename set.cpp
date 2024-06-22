#include <iostream>

template<class T>
class Set {
public:
    // region <============= Helper structs =============>
    enum Colour {
        red,
        black
    };

    struct RBTreeNode;

    using NodePointer = RBTreeNode *;

    struct RBTreeNode {
        NodePointer left = Nill;
        NodePointer right = Nill;
        NodePointer parent = Nill;
        Colour colour;

        T key;
    };

    static NodePointer Nill;

    class iterator {
    public:
        iterator(NodePointer node, NodePointer root) : cur_(node), root_(root) {}

        iterator() = default;

        iterator &operator++() {
            cur_ = LowerBoundHelper(cur_);
            return *this;
        }

        iterator operator++(int) {
            iterator it = *this;
            ++(*this);
            return it;
        }

        iterator &operator--() {
            if (cur_ == Nill) {
                cur_ = FindMax(root_);
            } else {
                cur_ = NearestLess(cur_);
            }
            return *this;
        }

        iterator operator--(int) {
            iterator it = *this;
            --(*this);
            return it;
        }

        const T &operator*() const {
            return cur_->key;
        }

        const T *operator->() const {
            return &cur_->key;
        }

        bool operator==(const iterator &other) const {
            return cur_ == other.cur_;
        }

        bool operator!=(const iterator &other) const {
            return cur_ != other.cur_;
        }

    private:
        NodePointer LowerBoundHelper(NodePointer node) {
            if (node->right != Nill) {
                return FindMin(node->right);
            }

            auto key = node->key;

            while (node->parent != Nill) {
                node = node->parent;
                if (!(node->key < key)) {
                    return node;
                }
            }
            return Nill;
        }

        NodePointer NearestLess(NodePointer node) {
            if (node->left != Nill) {
                return FindMax(node->left);
            }
            auto key = node->key;
            while (node->parent != Nill) {
                node = node->parent;
                if (node->key < key) {
                    return node;
                }
            }
            return Nill;
        }

        NodePointer cur_ = Nill;
        NodePointer root_ = Nill;
    };

    // endregion

    Set() = default;

    template<class iterator>
    Set(iterator begin, iterator end) {
        for (auto it = begin; it != end; ++it) {
            insert(*it);
        }
    }

    explicit Set(std::initializer_list<T> elements) {
        for (const auto &element: elements) {
            insert(element);
        }
    }

    Set(const Set &other) {
        for (const auto key: other) {
            insert(key);
        }
    }

    Set operator=(const Set &other) {
        if (this != &other) {
            DeleteTree_(root_);
            root_ = Nill;
            size_ = 0;
            for (const auto &key: other) {
                insert(key);
            }
        }
        return *this;
    }

    ~Set() {
        DeleteTree_(root_);
    }

    size_t size() const {
        return size_;
    }

    bool empty() const {
        return size_ == 0;
    }

    void insert(T key) {
        if (FindKeyNode(key) != Nill) {
            return;
        }

        ++size_;

        if (root_ == Nill) {
            root_ = new RBTreeNode{.colour = black, .key = key};
            return;
        }

        auto proper_node = FindPlaceToKey(key);
        auto leaf = insertNode(proper_node, key);

        if (proper_node->colour == red) {
            FixAfterinsert(leaf);
        }
    }

    void erase(const T key) {
        auto node_to_delete = FindKeyNode(key);

        if (node_to_delete == Nill) {
            return;
        }

        --size_;

        if (CountChildren(node_to_delete) == 2) {
            auto new_node_to_delete = FindMax(node_to_delete->left);
            node_to_delete->key = new_node_to_delete->key;
            node_to_delete = new_node_to_delete;
        }

        // black node, 1 child
        if ((node_to_delete->colour == black) and (CountChildren(node_to_delete) == 1)) {
            if (node_to_delete->left != Nill) {
                node_to_delete->key = node_to_delete->left->key;
                node_to_delete = node_to_delete->left;
                node_to_delete->parent->left = Nill;
            } else {
                node_to_delete->key = node_to_delete->right->key;
                node_to_delete = node_to_delete->right;
                node_to_delete->parent->right = Nill;
            }
            delete node_to_delete;
            return;
        }

        // red node, no children
        if ((node_to_delete->colour == red) and (CountChildren(node_to_delete) == 0)) {
            if (IsLeftChild(node_to_delete)) {
                node_to_delete->parent->left = Nill;
            } else {
                node_to_delete->parent->right = Nill;
            }
            delete node_to_delete;
            return;
        }

        // black node, no children
        if ((node_to_delete->colour == black) and (CountChildren(node_to_delete) == 0)) {
            if (node_to_delete == root_) {
                root_ = Nill;
                delete node_to_delete;
                return;
            }

            bool deleted_node_was_right_child = true;

            if (IsLeftChild(node_to_delete)) {
                node_to_delete->parent->left = Nill;
                deleted_node_was_right_child = false;
            } else {
                node_to_delete->parent->right = Nill;
            }

            auto parent = node_to_delete->parent;
            delete node_to_delete;
            FixAftererase(parent, deleted_node_was_right_child);
            return;
        }
    }

    void InOrderTraverse() {
        InOrderTraverse_(root_);
        std::cout << std::endl;
    }

    iterator find(T key) const {
        auto node = FindKeyNode(key);
        return iterator(node, root_);
    }

    iterator lower_bound(T key) const {
        auto current = root_;
        auto pretendent = Nill;

        while (current != Nill) {
            if (key < current->key) {
                pretendent = current;
                current = current->left;
            } else if (current->key < key) {
                current = current->right;
            } else {
                return iterator(current, root_);
            }
        }
        return iterator(pretendent, root_);
    }

    iterator begin() const {
        auto min_node = FindMin(root_);
        return iterator(min_node, root_);
    }

    iterator end() const {
        return iterator(Nill, root_);
    }

private:
    // region <============= Helper methods =============>

    int CountChildren(NodePointer node) const {
        if (node == Nill) {
            return 0;
        }
        int res = 0;
        if (node->left != Nill) {
            ++res;
        }
        if (node->right != Nill) {
            ++res;
        }
        return res;
    }

    void FixAftererase(NodePointer node, bool deleted_node_was_right_child) {
        if (node == Nill) {
            return;
        }

        // deleted node was right child
        if (deleted_node_was_right_child) {
            auto left_son = node->left;

            if (left_son->colour == red) {
                node->colour = red;
                left_son->colour = black;
                RightRotate(node);
                left_son = node->left;
            }

            if (left_son->colour == black) {
                if ((left_son->right->colour == red) and (left_son->left->colour == black)) {
                    left_son->right->colour = black;
                    left_son->colour = red;
                    LeftRotate(left_son);
                    left_son = node->left;
                }

                if (left_son->left->colour == red) {
                    left_son->colour = node->colour;
                    node->colour = black;
                    left_son->left->colour = black;
                    RightRotate(node);
                    return;
                }
            }

            if ((left_son->colour == black) and (left_son->left->colour == black) and
                (left_son->right->colour == black)) {
                left_son->colour = red;
                if (node->colour == red) {
                    node->colour = black;
                    return;
                } else {
                    FixAftererase(node->parent, !IsLeftChild(node));
                    return;
                }
            }

        } else {
            auto right_son = node->right;

            if (right_son->colour == red) {
                node->colour = red;
                right_son->colour = black;
                LeftRotate(node);
                right_son = node->right;
            }

            if (right_son->colour == black) {
                if ((right_son->left->colour == red) and (right_son->right->colour == black)) {
                    right_son->left->colour = black;
                    right_son->colour = red;
                    RightRotate(right_son);
                    right_son = node->right;
                }

                if (right_son->right->colour == red) {
                    right_son->colour = node->colour;
                    node->colour = black;
                    right_son->right->colour = black;
                    LeftRotate(node);
                    return;
                }
            }

            if ((right_son->colour == black) and (right_son->right->colour == black) and
                (right_son->left->colour == black)) {
                right_son->colour = red;
                if (node->colour == red) {
                    node->colour = black;
                    return;
                } else {
                    FixAftererase(node->parent, !IsLeftChild(node));
                    return;
                }
            }
        }
    }

    static NodePointer FindMax(NodePointer node) {
        if (node == Nill) {
            return Nill;
        }
        auto current = node;
        while (current->right != Nill) {
            current = current->right;
        }
        return current;
    }

    static NodePointer FindMin(NodePointer node) {
        if (node == Nill) {
            return Nill;
        }
        auto current = node;
        while (current->left != Nill) {
            current = current->left;
        }
        return current;
    }

    NodePointer FindKeyNode(T key) const {
        auto current = root_;
        while (current != Nill) {
            if (!(current->key < key) and !(key < current->key)) {
                return current;
            }
            if (key < current->key) {
                current = current->left;
            } else {
                current = current->right;
            }
        }
        return current;
    }

    void InOrderTraverse_(NodePointer root) const {
        if (root == Nill) {
            return;
        }
        if (root->left != Nill) {
            InOrderTraverse_(root->left);
        }
        std::cout << root->key << " ";
        if (root->right != Nill) {
            InOrderTraverse_(root->right);
        }
    }

    void DeleteTree_(NodePointer root) {
        if (root == Nill) {
            return;
        }
        DeleteTree_(root->left);
        DeleteTree_(root->right);
        delete root;
    }

    NodePointer FindPlaceToKey(T key) const {
        auto current = root_;
        while (current->left != Nill or current->right != Nill) {
            if ((current->left == Nill and key < current->key) or
                (current->right == Nill and !(key < current->key))) {
                break;
            }
            if (key < current->key) {
                current = current->left;
            } else {
                current = current->right;
            }
        }
        return current;
    }

    static NodePointer insertNode(NodePointer parent, T key) {
        auto new_node = new RBTreeNode{.parent = parent, .colour = red, .key = key};
        if (key < parent->key) {
            parent->left = new_node;
        } else {
            parent->right = new_node;
        }
        return new_node;
    }

    bool IsLeftChild(NodePointer node) const {
        if (node == root_) {
            return false;
        }
        if (node->parent->left == node) {
            return true;
        }
        return false;
    }

    NodePointer GetUncle(NodePointer node) const {
        if (node == root_) {
            return Nill;
        }

        if (IsLeftChild(node->parent)) {
            return node->parent->parent->right;
        } else {
            return node->parent->parent->left;
        }
    }

    void LeftRotate(NodePointer node) {
        auto parent = node->parent;
        auto right_child = node->right;

        if (node == root_) {
            root_ = right_child;
            right_child->parent = Nill;

            node->right = right_child->left;
            if (right_child->left != Nill) {
                right_child->left->parent = node;
            }
            right_child->left = node;
            node->parent = right_child;
            return;
        }

        if (IsLeftChild(node)) {
            parent->left = right_child;
        } else {
            parent->right = right_child;
        }

        right_child->parent = parent;

        node->right = right_child->left;

        if (right_child->left != Nill) {
            right_child->left->parent = node;
        }

        right_child->left = node;
        node->parent = right_child;
    }

    void RightRotate(NodePointer node) {
        auto parent = node->parent;
        auto left_child = node->left;

        if (node == root_) {
            root_ = left_child;
            left_child->parent = Nill;

            node->left = left_child->right;
            if (left_child->right != Nill) {
                left_child->right->parent = node;
            }

            left_child->right = node;
            node->parent = left_child;
            return;
        }

        if (IsLeftChild(node)) {
            parent->left = left_child;
        } else {
            parent->right = left_child;
        }

        left_child->parent = parent;

        node->left = left_child->right;
        if (left_child->right != Nill) {
            left_child->right->parent = node;
        }

        left_child->right = node;
        node->parent = left_child;
    }

    void FixAfterinsert(NodePointer leaf) {
        while (leaf->parent->colour == red) {
            if (IsLeftChild(leaf->parent)) {
                auto uncle = GetUncle(leaf);
                if (uncle->colour == red) {
                    leaf->parent->colour = black;
                    uncle->colour = black;
                    leaf->parent->parent->colour = red;
                    leaf = leaf->parent->parent;
                } else {
                    if (!IsLeftChild(leaf)) {
                        leaf = leaf->parent;
                        LeftRotate(leaf);
                    }
                    leaf->parent->colour = black;
                    leaf->parent->parent->colour = red;
                    RightRotate(leaf->parent->parent);
                }
            } else {
                auto uncle = GetUncle(leaf);
                if (uncle->colour == red) {
                    leaf->parent->colour = black;
                    uncle->colour = black;
                    leaf->parent->parent->colour = red;
                    leaf = leaf->parent->parent;
                } else {
                    if (IsLeftChild(leaf)) {
                        leaf = leaf->parent;
                        RightRotate(leaf);
                    }
                    leaf->parent->colour = black;
                    leaf->parent->parent->colour = red;
                    LeftRotate(leaf->parent->parent);
                }
            }
        }
        root_->colour = black;
    }
    // endregion

    NodePointer root_ = Nill;
    size_t size_ = 0;
};

template<class T>
typename Set<T>::NodePointer Set<T>::Nill = new typename Set<T>::RBTreeNode{.colour = black};
