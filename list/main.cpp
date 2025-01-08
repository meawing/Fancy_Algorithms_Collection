#pragma once

#include <cstddef>
#include <iterator>
#include <utility>

template <class T>
class List {

    struct Node;

    struct Links {
        Links* next_;
        Links* prev_;

        Links() = default;

        Links(Links* next, Links* prev) : next_(next), prev_(prev) {}

        virtual ~Links() = default;

        Node* CastToNode() {
            return dynamic_cast<Node*>(this);
        }

        const Node* CastToNode() const {
            return dynamic_cast<const Node*>(this);
        }
    };

    struct Node : public Links {
        T value_;
        template<typename ... Args>
        Node(Links* next, Links* prev, Args&& ...args) : Links(next, prev), value_(std::forward<Args>(args)...) {
        }
    };

public:
    class Iterator {
        friend class List;
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;

        Iterator() : ptr_(nullptr) {
        };

        Iterator& operator++() {
            ptr_ = ptr_->next_;
            return *this;
        }

        Iterator operator++(int) {
            Links* tmp = ptr_;
            ptr_ = ptr_->next_;
            return Iterator(tmp);
        }

        Iterator& operator--() {
            ptr_ = ptr_->prev_;
            return *this;
        }

        Iterator operator--(int) {
            Links* tmp = ptr_;
            ptr_ = ptr_->prev_;
            return Iterator(tmp);
        }

        T& operator*() const {
            return ptr_->CastToNode()->value_;
        }
        T* operator->() const {
            return &(ptr_->CastToNode()->value_);
        }

        bool operator==(const Iterator& rhs) const {
            return ptr_ == rhs.ptr_;
        }
        bool operator!=(const Iterator& rhs) const {
            return ptr_ != rhs.ptr_;

        }

    private:
        explicit Iterator(Links* ptr) : ptr_(ptr) {
        };

        Links* ptr_;
    };


    List() : size_(0) {
        end_.next_ = &end_;
        end_.prev_ = &end_;
    }

    template <typename Iter>
    List(Iter first, Iter last) : List() {
        while (first != last) {
            PushBack(*first);
            ++first;
        }
    }

    List(const List& other) : List(other.Begin(), other.End()) {}

    List& operator=(const List& other) {
        if (this == &other) {
            return *this;
        }
        Clear();
        for (const auto& elem : other) {
            PushBack(elem);
        }
        return *this;
    }

    List(List&& other) : size_(other.size_) {
        if (other.end_.next_ != &other.end_) {
            end_.prev_ = other.end_.prev_;
            end_.next_ = other.end_.next_;
            other.end_.next_->prev_ = &end_;
            other.end_.prev_->next_ = &end_;
            other.end_.prev_ = &other.end_;
            other.end_.next_ = &other.end_;
        }
        else {
            end_.prev_ = &end_;
            end_.next_ = &end_;
        }
        size_ = other.size_;
        other.size_ = 0;
    }

    List& operator=(List&& other) noexcept {
        if (this == &other) {
            return *this;
        }
        Clear();
        if (other.end_.next_ != &other.end_) {
            end_.prev_ = other.end_.prev_;
            end_.next_ = other.end_.next_;
            other.end_.next_->prev_ = &end_;
            other.end_.prev_->next_ = &end_;
            other.end_.prev_ = &other.end_;
            other.end_.next_ = &other.end_;
        }
        size_ = other.size_;
        other.size_ = 0;
        return *this;
    }

    ~List() {
        Clear();
    }

    bool IsEmpty() const {
        return size_ == 0;
    }
    size_t Size() const {
        return size_;
    }

    void PushBack(const T& value) {
        Insert(end_.prev_, value);
    }

    void PushBack(T&& value) {
        Insert(end_.prev_, std::move(value));
    }

    void PushFront(const T& value) {
        Insert(&end_, value);
    }

    void PushFront(T&& value) {
        Insert(&end_, std::move(value));
    }

    T& Front() {
        return end_.next_->CastToNode()->value_;
    }
    const T& Front() const {
        return end_.next_->CastToNode()->value_;
    }

    T& Back() {
        return end_.prev_->CastToNode()->value_;
    }
    const T& Back() const {
        return end_.prev_->CastToNode()->value_;
    }

    void PopBack() {
        Delete(end_.prev_);
    }
    void PopFront() {
        Delete(end_.next_);
    }

    void Delete(Links* links) {
        links->prev_->next_ = links->next_;
        links->next_->prev_ = links->prev_;
        delete links;
        --size_;
    }

    void Erase(Iterator it) {
        Delete(it.ptr_);
    }    

    Iterator Begin() {
        return Iterator(end_.next_);
    }

    Iterator Begin() const {
        return Iterator(end_.next_);
    }

    Iterator End() {
        return Iterator(&end_);
    }

    Iterator End() const {
        return Iterator(&end_);
    }

    Iterator begin() const {
        return Iterator(end_.next_);
    }

    Iterator end() const {
        return Iterator(&end_);
    }

private:
    mutable Links end_;
    size_t size_;

    void Clear() {
        while (size_ != 0) {
            PopBack();
        }
    }

    template<typename ... Args>
    void Insert(Links* where, Args&& ... args) {

        Node* x = new Node(where->next_, where, std::forward<Args>(args)...);
        where->next_ = x;
        x->next_->prev_ = x;
        ++size_;
    }
};

template <class T>
typename List<T>::Iterator begin(List<T>& list) {
    return list.Begin();
}

template <class T>
typename List<T>::Iterator end(List<T>& list) {
    return list.End();
}