#pragma once

#include <iterator>
#include <memory>
#include <vector>

template <class T>
class Vector {
public:
    Vector() : root_(CreateNodePartial(kStartingBit - kNumOfBits, 0)), size_(0) {
    }

    Vector(const Vector& other) : root_(other.root_), size_(other.size_) {
    }

    Vector& operator=(const Vector&) = delete;

    explicit Vector(size_t count, const T& value = {})
        : root_(CreateNodePartialWithValue(value, kStartingBit - kNumOfBits, count)), size_(count) {
    }

    template <class Iterator>
    Vector(Iterator first, Iterator last) {
        size_ = 0;
        auto copy = first;
        while (copy != last) {
            ++copy;
            ++size_;
        }
        root_ = CreateNodePartial(kStartingBit - kNumOfBits, size_);
        for (size_t i = 0; i < size_; ++i) {
            AddValue(root_, kStartingBit - kNumOfBits, i, *first);
            ++first;
        }
    }

    Vector(std::initializer_list<T> l)
        : root_(CreateNodePartial(kStartingBit - kNumOfBits, l.size())), size_(l.size()) {
        auto it = l.begin();
        for (size_t i = 0; i < size_; ++i) {
            AddValue(root_, kStartingBit - kNumOfBits, i, *it);
            ++it;
        }
    }

    Vector Set(size_t index, const T& value) const {
        return Vector(Change(root_, kStartingBit - kNumOfBits, index, value), size_);
    }

    const T& Get(size_t index) const {
        std::shared_ptr<Node> result = root_;
        for (size_t i = kStartingBit - kNumOfBits; i > 0; i -= kNumOfBits) {
            size_t count_bits = CountBits(index, i, i + kNumOfBits);
            result = dynamic_cast<PtrNode*>(result.get())->children[count_bits];
        }
        size_t count_bits = CountBits(index, 0, kNumOfBits);
        return dynamic_cast<ValueNode*>(result.get())->value[count_bits];
    }

    Vector PushBack(const T& value) const {
        return Vector(AddNewValue(root_, kStartingBit - kNumOfBits, size_, value), size_ + 1);
    }

    Vector PopBack() const {
        return Vector(PopValue(root_, kStartingBit - kNumOfBits, size_ - 1), size_ - 1);
    }

    size_t Size() const {
        return size_;
    }

private:
    static const size_t kWidth = 32;
    static const size_t kNumOfBits = 5;
    static const size_t kStartingBit = 30;

    struct Node {
        virtual ~Node() = default;
    };

    struct ValueNode : Node {
        std::vector<T> value;
    };

    struct PtrNode : Node {
        std::vector<std::shared_ptr<Node>> children;
    };

    std::shared_ptr<Node> root_;
    size_t size_;

    static size_t CountBits(size_t number, size_t left, size_t right) {
        size_t result = 0;
        for (; left < right; --right) {
            size_t current = ((number >> (right - 1)) & 1);
            result = result * 2 + current;
        }
        return result;
    }

    static std::shared_ptr<Node> CreateNodeWithValue(const T& value, size_t left) {
        if (left == 0) {
            ValueNode* result = new ValueNode;
            result->value = std::vector<T>(kWidth, value);
            return std::shared_ptr<Node>(result);
        }
        PtrNode* node = new PtrNode;
        for (size_t i = 0; i < kWidth; ++i) {
            node->children.push_back(CreateNodeWithValue(value, left - kNumOfBits));
        }
        return std::shared_ptr<Node>(node);
    }

    static std::shared_ptr<Node> CreateNodePartialWithValue(const T& value, size_t left,
                                                            size_t number) {
        size_t count_bits = CountBits(number, left, left + kNumOfBits);
        if (left == 0) {
            ValueNode* result = new ValueNode;
            result->value = std::vector<T>(count_bits, value);
            return std::shared_ptr<Node>(result);
        }

        PtrNode* node = new PtrNode;
        for (size_t i = 0; i < count_bits; ++i) {
            node->children.push_back(CreateNodeWithValue(value, left - kNumOfBits));
        }

        node->children.push_back(CreateNodePartialWithValue(value, left - kNumOfBits, number));
        return std::shared_ptr<Node>(node);
    }
    static std::shared_ptr<Node> CreateNode(size_t left) {
        if (left == 0) {
            auto node = std::shared_ptr<Node>(new ValueNode);
            dynamic_cast<ValueNode*>(node.get())->value.reserve(kWidth);
            return node;
        }
        PtrNode* node = new PtrNode;
        for (size_t i = 0; i < kWidth; ++i) {
            node->children.push_back(CreateNode(left - kNumOfBits));
        }
        return std::shared_ptr<Node>(node);
    }

    static std::shared_ptr<Node> CreateNodePartial(size_t left, size_t number) {
        size_t count_bits = CountBits(number, left, left + kNumOfBits);
        if (left == 0) {
            auto node = std::shared_ptr<Node>(new ValueNode);
            dynamic_cast<ValueNode*>(node.get())->value.reserve(count_bits);
            return node;
        }

        PtrNode* node = new PtrNode;
        for (size_t i = 0; i < count_bits; ++i) {
            node->children.push_back(CreateNode(left - kNumOfBits));
        }
        node->children.push_back(CreateNodePartial(left - kNumOfBits, number));
        return std::shared_ptr<Node>(node);
    }

    Vector(std::shared_ptr<Node> node, size_t size) : root_(node), size_(size) {
    }

    static std::shared_ptr<Node> Change(std::shared_ptr<Node> node, size_t left, size_t number,
                                        const T& value) {
        size_t count_bits = CountBits(number, left, left + kNumOfBits);
        if (left == 0) {
            ValueNode* new_node = new ValueNode;
            ValueNode* value_node = dynamic_cast<ValueNode*>(node.get());
            new_node->value.reserve(value_node->value.size());
            for (size_t i = 0; i < value_node->value.size(); ++i) {
                if (i != count_bits) {
                    new_node->value.push_back(value_node->value[i]);
                } else {
                    new_node->value.push_back(value);
                }
            }
            return std::shared_ptr<Node>(new_node);
        }
        PtrNode* new_node = new PtrNode;
        PtrNode* ptr_node = dynamic_cast<PtrNode*>(node.get());
        for (size_t i = 0; i < ptr_node->children.size(); ++i) {
            if (i != count_bits) {
                new_node->children.push_back(ptr_node->children[i]);
            } else {
                new_node->children.push_back(
                    Change(ptr_node->children[i], left - kNumOfBits, number, value));
            }
        }
        return std::shared_ptr<Node>(new_node);
    }

    static void AddValue(std::shared_ptr<Node> node, size_t left, size_t pos, const T& value) {
        if (left == 0) {
            dynamic_cast<ValueNode*>(node.get())->value.push_back(value);
            return;
        }
        size_t count_bits = CountBits(pos, left, left + kNumOfBits);
        PtrNode* ptr_node = dynamic_cast<PtrNode*>(node.get());
        if (ptr_node->children.size() == count_bits) {
            if (left == kNumOfBits) {
                ptr_node->children.emplace_back(new ValueNode);
            } else {
                ptr_node->children.emplace_back(new PtrNode);
            }
            AddValue(ptr_node->children[count_bits], left - kNumOfBits, pos, value);
            return;
        }
        AddValue(dynamic_cast<PtrNode*>(node.get())->children[count_bits], left - kNumOfBits, pos,
                 value);
    }
    static std::shared_ptr<Node> AddNewValue(std::shared_ptr<Node> node, size_t left, size_t pos,
                                             const T& value) {
        if (left == 0) {
            ValueNode* value_node = new ValueNode;
            value_node->value = dynamic_cast<ValueNode*>(node.get())->value;
            value_node->value.push_back(value);
            return std::shared_ptr<Node>(value_node);
        }
        size_t current_bits = CountBits(pos, left, left + kNumOfBits);
        PtrNode* current_node = dynamic_cast<PtrNode*>(node.get());
        PtrNode* ptr_node = new PtrNode;
        ptr_node->children = current_node->children;
        if (current_node->children.size() == current_bits) {
            if (left == kNumOfBits) {
                ptr_node->children.emplace_back(new ValueNode);
            } else {
                ptr_node->children.emplace_back(new PtrNode);
            }
            AddValue(ptr_node->children[current_bits], left - kNumOfBits, pos, value);
            return std::shared_ptr<Node>(ptr_node);
        }
        ptr_node->children.back() =
            AddNewValue(ptr_node->children.back(), left - kNumOfBits, pos, value);
        return std::shared_ptr<Node>(ptr_node);
    }
    static std::shared_ptr<Node> PopValue(std::shared_ptr<Node> node, size_t left, size_t pos) {
        if (left == 0) {
            ValueNode* value_node = dynamic_cast<ValueNode*>(node.get());
            if (value_node->value.size() == 1 && pos != 0) {
                return nullptr;
            }
            ValueNode* new_node = new ValueNode;
            for (size_t i = 0; i + 1 < value_node->value.size(); ++i) {
                new_node->value.push_back(value_node->value[i]);
            }
            return std::shared_ptr<Node>(new_node);
        }
        PtrNode* ptr_node = dynamic_cast<PtrNode*>(node.get());
        PtrNode* new_node = new PtrNode;
        new_node->children = ptr_node->children;
        new_node->children.back() = PopValue(new_node->children.back(), left - kNumOfBits, pos);
        if (new_node->children.back() == nullptr) {
            if (new_node->children.size() == 1) {
                delete new_node;
                return nullptr;
            }
            new_node->children.pop_back();
        }
        return std::shared_ptr<Node>(new_node);
    }
};

template <class Iterator>
Vector(Iterator, Iterator) -> Vector<std::iter_value_t<Iterator>>;
