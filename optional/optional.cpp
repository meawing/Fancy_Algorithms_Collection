#pragma once

#include <array>
#include <cstdint>
#include <cstddef>

constexpr static size_t TwoPow(size_t i) {
    size_t two_power = 1;
    while (two_power < i) {
        two_power *= 2;
    }
    return two_power;
}

template <class T>
class alignas(TwoPow(sizeof(T))) Optional {
public:
    Optional() : exist_(false) {
    }

    Optional(const T& value) : exist_(true) {
        new (reinterpret_cast<T*>(data_.data())) T(value);
    }

    Optional(T&& value) : exist_(true) {
        new (reinterpret_cast<T*>(data_.data())) T(std::move(value));
    }

    Optional(const Optional& other) {
        exist_ = other.exist_;
        if (exist_) {
            new (reinterpret_cast<T*>(data_.data())) T(*reinterpret_cast<T*>(other.data_.data()));
        }
    }

    // This constructor must not change other.HasValue()
    // Just move value (if present)
    Optional(Optional&& other) {
        exist_ = other.exist_;
        if (exist_) {
            new (reinterpret_cast<T*>(data_.data()))
                T(std::move(*reinterpret_cast<T*>(other.data_.data())));
        }
    }

    ~Optional() {
        if (exist_) {
            // explicitly destruct object of type T inside array
            reinterpret_cast<T*>(data_.data())->~T();
        }
    }

    Optional& operator=(const Optional& other) {
        if (this == &other) {
            return *this;
        }
        if (exist_ && other.exist_) {
            *reinterpret_cast<T*>(data_.data()) = *reinterpret_cast<const T*>(other.data_.data());
        } else if (!exist_ && other.exist_) {
            new (reinterpret_cast<T*>(data_.data()))
                T(*reinterpret_cast<const T*>(other.data_.data()));
            exist_ = true;
        } else if (exist_ && !other.exist_) {
            reinterpret_cast<T*>(data_.data())->~T();
            exist_ = false;
        }
        return *this;
    }

    // This method must not change other.HasValue()
    // Just move value (if present)
    Optional& operator=(Optional&& other) {
        if (this == &other) {
            return *this;
        }
        if (exist_ && other.exist_) {
            *reinterpret_cast<T*>(data_.data()) =
                std::move(*reinterpret_cast<T*>(other.data_.data()));

        } else if (!exist_ && other.exist_) {
            new (reinterpret_cast<T*>(data_.data()))
                T(std::move(*reinterpret_cast<T*>(other.data_.data())));
            exist_ = true;
        } else if (exist_ && !other.exist_) {
            reinterpret_cast<T*>(data_.data())->~T();
            exist_ = false;
        }
        return *this;
    }

    T& operator*() & {
        return *reinterpret_cast<T*>(data_.data());
    }

    const T& operator*() const& {
        return *reinterpret_cast<const T*>(data_.data());
    }

    T&& operator*() && {
        return std::move(*reinterpret_cast<T*>(data_.data()));
    }

    T* operator->() {
        return reinterpret_cast<T*>(data_.data());
    }

    const T* operator->() const {
        return reinterpret_cast<const T*>(data_.data());
    }

    T& Value() & {
        if (!exist_) {
            throw std::exception();
        }
        return *reinterpret_cast<T*>(data_.data());
    }

    const T& Value() const& {
        if (!exist_) {
            throw std::exception();
        }
        return *reinterpret_cast<const T*>(data_.data());
    }

    T&& Value() && {
        if (!exist_) {
            throw std::exception();
        }
        return std::move(*reinterpret_cast<T*>(data_.data()));
    }

    bool HasValue() const {
        return exist_;
    }

    void Reset() {
        if (exist_) {
            reinterpret_cast<T*>(data_.data())->~T();
            exist_ = false;
        }
    }

private:
    mutable std::array<uint8_t, sizeof(T)> data_;
    bool exist_;
};
