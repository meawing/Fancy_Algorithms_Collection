#include <algorithm>
#include <initializer_list>
#include <vector>

enum class Color {
    RED, BLACK
}; 

template <typename ValueType>
class Set {
    struct Node {
        explicit Node(ValueType value): Node(value, Color::RED) {
        }

        Node(const ValueType& value, Color color)
            : Node(value, nullptr, nullptr, nullptr, color) {  
        }

        Node(const ValueType& value, Node* left,
            Node* right, Node* parent, Color color)
            : value_{value}, left_{left}, right_{right},
              parent_{parent}, color_{color} {
        }

        ValueType value_;
        Node* left_;
        Node* right_;
        Node* parent_;        
        Color color_;              
    };

public:
    class iterator {
    public:
        using iterator_category = std::bidirectional_iterator_tag;

        iterator(): set_{nullptr}, node_{nullptr} {
        }

        iterator(const Set* set, Node* node): set_{set}, node_{node} {
        }

        iterator& operator++() {
            node_ = set_->Successor(node_);
            return *this;            
        }

        iterator operator++(int) {
            iterator old(set_, node_);
            this->operator++();
            return old;            
        }

        iterator& operator--() {
            if (node_ != set_->nil_) {
                node_ = set_->Predecessor(node_);            
            } else {
                node_ = set_->MaxValueNode(set_->root_);
            }
            return *this;
        }

        iterator operator--(int) {
            iterator old(set_, node_);
            this->operator--();
            return old;            
        }

        bool operator==(const iterator& rhs) const {
            return node_ == rhs.node_;            
        }

        bool operator!=(const iterator& rhs) const {
            return node_ != rhs.node_;            
        }

        const ValueType& operator*() {
            return node_->value_;            
        }

        const ValueType* operator->() {
            return &(node_->value_);            
        }      

    private:
        const Set* set_;
        Node* node_;        
    };  

    Set(): nil_{new Node(ValueType(), Color::BLACK)}, size_{0} {
        root_ = nil_;
        nil_->left_ = nil_->right_ = nil_->parent_ = nil_;
    }        

    template <typename Iterator>
    Set(Iterator first, Iterator last): Set() {
        while (first != last) {
            insert(*first);
            ++first;
        }
    }

    explicit Set(std::initializer_list<ValueType> init_list)
                : Set(init_list.begin(), init_list.end()) {
    }

    Set(const Set<ValueType>& other): Set(other.begin(), other.end()) {
        size_ = other.size_;
    }

    Set& operator=(const Set<ValueType>& rhs) {
        if (this == &rhs) {
            return *this;
        }

        ClearAll(root_);
        root_ = nil_;
        iterator itr = rhs.begin();
        while (itr != rhs.end()) {
            insert(*itr);
            ++itr;
        }

        size_ = rhs.size_;
        return *this;
    }

    ~Set() {
        if (root_ != nil_) {
            ClearAll(root_);
        }
        delete nil_;
    }

    iterator begin() const {
        return iterator(this, MinValueNode(root_));        
    }

    iterator end() const {
        return iterator(this, nil_);        
    } 

    void insert(const ValueType& value) {
        if (FindByValue(value) != nil_) {
            return;
        }
        Node* node = new Node(value);
        return RBInsert(node);
    }

    void erase(const ValueType& value) {
        Node* node = FindByValue(value);
        if (node != nil_) {
            return RBDelete(node);
        }
        return;
    }

    iterator find(const ValueType& value) const {
        return iterator(this, FindByValue(value));
    }   

    iterator lower_bound(const ValueType& value) const {
        Node* current_node = root_;
        Node* current_parent = current_node;
        
        while (current_node != nil_ && !Equal(current_node->value_, value)) {
            current_parent = current_node;           
            if (value < current_node->value_) {
                current_node = current_node->left_;
            } else {
                current_node = current_node->right_;
            }
        }

        if (current_node != nil_) {
            return iterator(this, current_node);
        } else if (current_parent->value_ < value) {
            return iterator(this, Successor(current_parent));
        } else {
            return iterator(this, current_parent);
        }
    }    

    size_t size() const {
        return size_;
    }
    bool empty() const {
        return size_ == 0;
    }

private:
    void ClearAll(Node*& root);
    bool Equal(const ValueType& lhs, const ValueType& rhs) const;
    Node* FindByValue(const ValueType& value) const;

    Node* MinValueNode(Node* root) const;
    Node* MaxValueNode(Node* root) const;
    Node* Successor(Node* node) const;
    Node* Predecessor(Node* node) const;

    bool RedFlag(Node* node) const;
    bool BlackFlag(Node* node) const;
    void PaintRed(Node* node);
    void PaintBlack(Node* node);

    void LeftRotate(Node* x_node);
    void RightRotate(Node* x_node);
    void RBInsert(Node*& z_node);
    void RBInsertFixup(Node*& z_node);
    void RBTransplant(Node*& x_node, Node*& y_node);
    void RBDelete(Node*& z_node);
    void RBDeleteFixup(Node*& z_node);

    Node* root_;
    Node* nil_;
    size_t size_;
};


template <typename ValueType>
void Set<ValueType>::ClearAll(Node*& root) {
    if (root->left_ != nil_) {
        ClearAll(root->left_);
    }
    if (root->right_ != nil_) {
        ClearAll(root->right_);
    }
    delete root;
}

template <typename ValueType>
typename Set<ValueType>::Node* Set<ValueType>::FindByValue(const ValueType& value) const {
    Node* current_node = root_;

    while (current_node != nil_) {
        if (Equal(current_node->value_, value)) {
            return current_node;
        }
        if (value < current_node->value_) {
            current_node = current_node->left_;
        } else {
            current_node = current_node->right_;
        }
    }

    return current_node;
}

template <typename ValueType>
typename Set<ValueType>::Node* Set<ValueType>::MinValueNode(Node* root) const {
    Node* min_val_node = root;

    while (min_val_node->left_ != nil_) {
        min_val_node = min_val_node->left_;
    }

    return min_val_node;
}

template <typename ValueType>
typename Set<ValueType>::Node* Set<ValueType>::MaxValueNode(Node* root) const {
    Node* max_val_node = root;

    while (max_val_node->right_ != nil_) {
        max_val_node = max_val_node->right_;
    }

    return max_val_node;
}

template <typename ValueType>
typename Set<ValueType>::Node* Set<ValueType>::Successor(Node* node) const {
    if (node->right_ != nil_) {
        return MinValueNode(node->right_);
    }

    Node* y_node = node->parent_;
    while (y_node != nil_ && node == y_node->right_) {
        node = y_node;
        y_node = y_node->parent_;
    }

    return y_node;
}

template <typename ValueType>
typename Set<ValueType>::Node* Set<ValueType>::Predecessor(Node* node) const {
    if (node->left_ != nil_) {
        return MaxValueNode(node->left_);
    }

    Node* y_node = node->parent_;
    while (y_node != nil_ && node == y_node->left_) {
        node = y_node;
        y_node = y_node->parent_;
    }

    return y_node;
}

template <typename ValueType>
bool Set<ValueType>::Equal(const ValueType& lhs, const ValueType& rhs) const {
    return !(lhs < rhs) && !(rhs < lhs);
}

template <typename ValueType>
bool Set<ValueType>::RedFlag(Node* node) const {
    return node->color_ == Color::RED;
}

template <typename ValueType>
bool Set<ValueType>::BlackFlag(Node* node) const {
    return node->color_ == Color::BLACK;
}

template <typename ValueType>
void Set<ValueType>::PaintRed(Node* node) {
    node->color_ = Color::RED;
}

template <typename ValueType>
void Set<ValueType>::PaintBlack(Node* node) {
    node->color_ = Color::BLACK;
}

template <typename ValueType>
void Set<ValueType>::LeftRotate(Node* x_node) {
    Node* y_node = x_node->right_;
    x_node->right_ = y_node->left_;

    if (y_node->left_ != nil_) {
        y_node->left_->parent_ = x_node;
    }

    y_node->parent_ = x_node->parent_;
    if (x_node->parent_ == nil_) {
        root_ = y_node;

    } else if (x_node == x_node->parent_->left_) {
        x_node->parent_->left_ = y_node;

    } else {
        x_node->parent_->right_ = y_node;
    }

    y_node->left_ = x_node;
    x_node->parent_ = y_node;
}

template <typename ValueType>
void Set<ValueType>::RightRotate(Node* x_node) {
    auto y_node = x_node->left_;
    x_node->left_ = y_node->right_;

    if (y_node->right_ != nil_) {
        y_node->right_->parent_ = x_node;
    }

    y_node->parent_ = x_node->parent_;
    if (x_node->parent_ == nil_) {
        root_ = y_node;

    } else if (x_node == x_node->parent_->left_) {
        x_node->parent_->left_ = y_node;
    } else {
        x_node->parent_->right_ = y_node;
    }

    y_node->right_ = x_node;
    x_node->parent_ = y_node;
}

template <typename ValueType>
void Set<ValueType>::RBInsert(Node*& z_node) {
    Node* y_node = nil_;
    Node* x_node = root_;

    while (x_node != nil_) {
        y_node = x_node;
        if (z_node->value_ < x_node->value_) {
            x_node = x_node->left_;
        } else {
            x_node = x_node->right_;
        }
    }

    z_node->parent_ = y_node;
    if (y_node == nil_) {
        root_ = z_node;
    } else if (z_node->value_ < y_node->value_) {
        y_node->left_ = z_node;
    } else {
        y_node->right_ = z_node;
    }

    z_node->left_ = nil_;
    z_node->right_ = nil_;
    PaintRed(z_node);
    RBInsertFixup(z_node);
    ++size_;
}

template <typename ValueType>
void Set<ValueType>::RBInsertFixup(Node*& z_node) {
    while (RedFlag(z_node->parent_)) {
        if (z_node->parent_ == z_node->parent_->parent_->left_) {
            auto y_node = z_node->parent_->parent_->right_;
            if (RedFlag(y_node)) {
                PaintBlack(z_node->parent_);
                PaintBlack(y_node);
                PaintRed(z_node->parent_->parent_);
                z_node = z_node->parent_->parent_;

            } else {
                if (z_node == z_node->parent_->right_) {
                    z_node = z_node->parent_;
                    LeftRotate(z_node);
                }
                PaintBlack(z_node->parent_);
                PaintRed(z_node->parent_->parent_);
                RightRotate(z_node->parent_->parent_);
            }

        } else {
            auto y_node = z_node->parent_->parent_->left_;
            if (RedFlag(y_node)) {
                PaintBlack(z_node->parent_);
                PaintBlack(y_node);
                PaintRed(z_node->parent_->parent_);
                z_node = z_node->parent_->parent_;

            } else {
                if (z_node == z_node->parent_->left_) {
                    z_node = z_node->parent_;
                    RightRotate(z_node);
                }
                PaintBlack(z_node->parent_);
                PaintRed(z_node->parent_->parent_);
                LeftRotate(z_node->parent_->parent_);
            }
        }
    }
    PaintBlack(root_);
}

template <typename ValueType>
void Set<ValueType>::RBTransplant(Node*& x_node, Node*& y_node) {
    if (x_node->parent_ == nil_) {
        root_ = y_node;

    } else if (x_node == x_node->parent_->left_) {
        x_node->parent_->left_ = y_node;

    } else {
        x_node->parent_->right_ = y_node;
    }

    y_node->parent_ = x_node->parent_;
}

template <typename ValueType>
void Set<ValueType>::RBDelete(Node*& z_node) {
    Node* x_node = nil_;
    Node* y_node = z_node;
    Color y_original_color = y_node->color_;

    if (z_node->left_ == nil_) {
        x_node = z_node->right_;
        RBTransplant(z_node, z_node->right_);

    } else if (z_node->right_ == nil_) {
        x_node = z_node->left_;
        RBTransplant(z_node, z_node->left_);

    } else {
        y_node = MinValueNode(z_node->right_);
        y_original_color = y_node->color_;
        x_node = y_node->right_;
        if (y_node->parent_ == z_node) {
            x_node->parent_ = y_node;

        } else {
            RBTransplant(y_node, y_node->right_);
            y_node->right_ = z_node->right_;
            y_node->right_->parent_ = y_node;
        }

        RBTransplant(z_node, y_node);
        y_node->left_ = z_node->left_;
        y_node->left_->parent_ = y_node;
        y_node->color_ = z_node->color_;
    }

    if (y_original_color == Color::BLACK) {
        RBDeleteFixup(x_node);
    }

    --size_;
    delete z_node;
}

template <typename ValueType>
void Set<ValueType>::RBDeleteFixup(Node*& x_node) {
    while (x_node != root_ && BlackFlag(x_node)) {
        if (x_node == x_node->parent_->left_) {
            auto w_node = x_node->parent_->right_;
            if (RedFlag(w_node)) {
                PaintBlack(w_node);
                PaintRed(x_node->parent_);
                LeftRotate(x_node->parent_);
                w_node = x_node->parent_->right_;
            }

            if (BlackFlag(w_node->left_) && BlackFlag(w_node->right_)) {
                PaintRed(w_node);
                x_node = x_node->parent_;

            } else {
                if (BlackFlag(w_node->right_)) {
                    PaintBlack(w_node->left_);
                    PaintRed(w_node);
                    RightRotate(w_node);
                    w_node = x_node->parent_->right_;
                }

                w_node->color_ = x_node->parent_->color_;
                PaintBlack(x_node->parent_);
                PaintBlack(w_node->right_);
                LeftRotate(x_node->parent_);
                x_node = root_;
            }

        } else {
            auto w_node = x_node->parent_->left_;
            if (RedFlag(w_node)) {
                PaintBlack(w_node);
                PaintRed(x_node->parent_);
                RightRotate(x_node->parent_);
                w_node = x_node->parent_->left_;
            }

            if (BlackFlag(w_node->left_) && BlackFlag(w_node->right_)) {
                PaintRed(w_node);
                x_node = x_node->parent_;

            } else {
                if (BlackFlag(w_node->left_)) {
                    PaintBlack(w_node->right_);
                    PaintRed(w_node);
                    LeftRotate(w_node);
                    w_node = x_node->parent_->left_;
                }
                w_node->color_ = x_node->parent_->color_;
                PaintBlack(x_node->parent_);
                PaintBlack(w_node->left_);
                RightRotate(x_node->parent_);
                x_node = root_;
            }
        }
    }

    PaintBlack(x_node);
}
