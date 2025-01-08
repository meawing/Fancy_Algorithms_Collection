#include <iostream>
#include <string>

template <class T>
concept NotAny = !std::same_as<std::remove_cvref_t<T>, class Any>;

class BaseClass {
public:
    virtual ~BaseClass() = default;
    virtual BaseClass* MakeCopy() = 0;
    virtual const std::type_info& GetTypeInfo() = 0;
};

template <typename P>
class Value : public BaseClass {
public:
    explicit Value(P value) : value_(std::move(value)) {
    }
    ~Value() override = default;
    Value<P>* MakeCopy() override {
        return new Value<P>(value_);
    };
    const std::type_info& GetTypeInfo() override {
        return typeid(value_);
    }
    P value_;
};

class Any {

public:
    template <class T>
    Any(const T& val) : bc_(new Value<T>(val)) {
        // std::cout << "Any(const T& val)" << "\n";
    }

    template <NotAny T>
    Any(T&& value) : bc_(new Value<std::remove_cvref_t<T>>(std::forward<T>(value))) {
        // std::cout << "Any(T&& value)" << "\n";
    }

    Any(Any&& other) noexcept : bc_(other.bc_) {
        // std::cout << "Any(Any&& other)" << "\n";
        other.bc_ = nullptr;
    }

    Any() : bc_(nullptr) {
        // std::cout << "Any()" << "\n";
    }

    ~Any() {
        delete bc_;
    }

    Any(const Any& other) {
        // std::cout << "Any(const Any &other)" << "\n";
        if (other.bc_ == nullptr) {
            bc_ = nullptr;
        } else {
            bc_ = other.bc_->MakeCopy();
        }
    }

    Any& operator=(const Any& other) {
        // std::cout << "Any& operator = (const Any& other)" << "\n";
        if (std::addressof(other) == this) {
            return *this;
        }
        Any tmp_bc = other;
        delete bc_;
        if (tmp_bc.bc_ == nullptr) {
            bc_ = nullptr;
        } else {
            bc_ = tmp_bc.bc_->MakeCopy();
        }
        // delete tmp_bc.bc_;
        tmp_bc.Clear();
        return *this;
    }

    Any& operator=(Any&& other) noexcept {
        // std::cout << "Any& operator = (Any&& other)" << "\n";
        if (std::addressof(other) == this) {
            return *this;
        }
        delete bc_;
        bc_ = other.bc_;
        other.bc_ = nullptr;
        return *this;
    }

    BaseClass* GetBC() const {
        return bc_;
    }

    Any& Swap(Any& rhs) {
        std::swap(bc_, rhs.bc_);
        return *this;
    }

    template <typename T>
    const T& GetValue() const {
        if ((bc_ == nullptr && typeid(T) != typeid(nullptr)) || typeid(T) != bc_->GetTypeInfo()) {
            throw std::bad_cast();
        }
        return dynamic_cast<Value<T>*>(bc_)->value_;
    }

    bool Empty() const {
        return bc_ == nullptr;
    }
    const std::type_info& Type() {
        return bc_->GetTypeInfo();
    }

    void Clear() {
        Any().Swap(*this);
    }

private:
    BaseClass* bc_;
};
