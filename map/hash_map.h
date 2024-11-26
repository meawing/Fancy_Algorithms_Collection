#include <stdexcept>
#include <vector>
#include <list>
#include <algorithm>
#include <utility>

constexpr size_t table_size = 64;

template<typename KeyType, typename ValueType, typename Hash = std::hash<KeyType>>
class HashMap {
    using KeyValueType = std::pair<const KeyType, ValueType>;
    using VectorOfLists = std::vector<std::list<KeyValueType>>;

public:
    explicit HashMap(Hash hash = Hash())
                    : table_(VectorOfLists(table_size)), hash_(hash), size_(0) {
    }

    HashMap(const HashMap& other)
            : table_(other.table_), hash_(other.hash_), size_(other.size_) {
    }

    HashMap(HashMap&& other) noexcept 
            : table_(std::move(other.table_)), hash_(std::move(other.hash_)), 
            size_(std::move(other.size_)) {
    }

    template <typename Iterator>
    HashMap(Iterator first, Iterator last, Hash hash = Hash()) : HashMap(hash) {
        while (first != last) {
            insert(*first);
            ++first;
        }
    }

    HashMap(std::initializer_list<KeyValueType> init_list, Hash hash = Hash()) 
            : HashMap(init_list.begin(), init_list.end(), hash) {
    }

    HashMap& operator=(HashMap rhs) {
        swap(rhs);
        return *this;
    };

    void swap(HashMap &other) {
        if (&other == this) {
            return;
        }
        size_t size_tmp = size_;
        size_ = other.size_;
        other.size_ = size_tmp;        
        std::swap(table_, other.table_);
        std::swap(hash_, other.hash_);
    }

    class iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = KeyValueType;
        using difference_type = ptrdiff_t;
        using pointer = KeyValueType*;
        using reference = KeyValueType&;

        using IteratorVector = typename VectorOfLists::iterator;
        using IteratorList = typename std::list<KeyValueType>::iterator;

        iterator() = default;

        explicit iterator(HashMap<KeyType, ValueType, Hash>* hash_map)
                        : hash_map_(hash_map), table_iter_(hash_map_->table_.begin()), 
                        list_iter_(table_iter_->begin()) {
            // search for non empty bucket
            while (table_iter_ != hash_map_->table_.end() && table_iter_->empty()) {
                ++table_iter_;
            }
            if (table_iter_ != hash_map_->table_.end()) {
                list_iter_ = table_iter_->begin();
            } else {
                list_iter_ = hash_map_->table_.front().end();            
            }
        }

        iterator(HashMap<KeyType, ValueType, Hash>* hash_map,
                IteratorVector table_iter, IteratorList list_iter)
                : hash_map_(hash_map), table_iter_(table_iter),
                 list_iter_(list_iter) {                    
        }                                

        KeyValueType& operator*() {
            return list_iter_.operator*();
        }

        KeyValueType* operator->() {
            return list_iter_.operator->();
        }

        bool operator==(const iterator &rhs) const {
            return hash_map_ == rhs.hash_map_ && table_iter_ == rhs.table_iter_ &&
                    list_iter_ == rhs.list_iter_;
        }

        bool operator!=(const iterator &rhs) const {
            return hash_map_ != rhs.hash_map_ || table_iter_ != rhs.table_iter_ ||
                    list_iter_ != rhs.list_iter_;
        }

        iterator& operator++() {
            if (list_iter_ != table_iter_->end()) {
                ++list_iter_;
            }
            if (list_iter_ == table_iter_->end()) {
                ++table_iter_;
                //  look for next non empty
                while (table_iter_ != hash_map_->table_.end() && table_iter_->empty()) {
                    ++table_iter_;
                }
                if (table_iter_ != hash_map_->table_.end()) {
                    list_iter_ = table_iter_->begin();
                } else {
                    //  when all next are empty
                    list_iter_ = hash_map_->table_.front().end(); 
                }
            }
            return *this;
        }

        iterator operator++(int) {
            iterator old(*this);
            this->operator++();
            return old;
        }

    private:
        HashMap<KeyType, ValueType, Hash>* hash_map_;
        IteratorVector table_iter_;
        IteratorList list_iter_;
    };

    class const_iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = KeyValueType;
        using difference_type = ptrdiff_t;
        using pointer = KeyValueType*;
        using reference = KeyValueType&;

        using IteratorVectorConst = typename VectorOfLists::const_iterator;
        using IteratorListConst = typename std::list<KeyValueType>::const_iterator;        

        const_iterator() = default;

        explicit const_iterator(const HashMap<KeyType, ValueType, Hash>* hash_map)
                                        : hash_map_(hash_map), 
                                         table_iter_(hash_map->table_.begin()),
                                         list_iter_(table_iter_->begin()) {
            // search for non empty bucket
            while (table_iter_ != hash_map_->table_.end() && table_iter_->empty()) {
                ++table_iter_;
            }

            if (table_iter_ != hash_map_->table_.end()) {
                list_iter_ = table_iter_->begin();
            } else {
                list_iter_ = hash_map_->table_.front().end();
            }
        }
        
        const_iterator(const HashMap<KeyType, ValueType, Hash>* hash_map,
                         IteratorVectorConst bit, IteratorListConst lit)
                        : hash_map_{hash_map}, table_iter_{bit},
                          list_iter_{lit} {                            
        }

        const KeyValueType& operator*() {
            return list_iter_.operator*();
        }

        const KeyValueType* operator->() {
            return list_iter_.operator->();
        }

        bool operator==(const const_iterator &rhs) const {
            return hash_map_ == rhs.hash_map_ && table_iter_ == rhs.table_iter_ &&
                     list_iter_ == rhs.list_iter_;
        }

        bool operator!=(const const_iterator &rhs) const {
            return hash_map_ != rhs.hash_map_ || table_iter_ != rhs.table_iter_ ||
                     list_iter_ != rhs.list_iter_;
        }

        const_iterator& operator++() {
            if (list_iter_ != table_iter_->end()) {
                ++list_iter_;
            }
            if (list_iter_ == table_iter_->end()) {
                ++table_iter_;
                while (table_iter_ != hash_map_->table_.end() && table_iter_->empty()) {
                    ++table_iter_;
                }
                if (table_iter_ != hash_map_->table_.end()) {
                    list_iter_ = table_iter_->begin();
                } else {
                    list_iter_ = hash_map_->table_.front().end();
                }
            }

            return *this;
        };

        const_iterator operator++(int) {
            const_iterator old(*this);
            this->operator++();
            return old;
        }

    private:
        const HashMap<KeyType, ValueType, Hash>* hash_map_;
        IteratorVectorConst table_iter_;
        IteratorListConst list_iter_;
    };

    iterator begin() {
        return iterator(this);
    }
    iterator end() {
        return iterator(this, this->table_.end(), this->table_.front().end());
    }

    const_iterator begin() const {
        return const_iterator(this);
    }

    const_iterator end() const {
        return const_iterator(this, this->table_.end(), this->table_.front().end());
    }

    size_t size() const;
    bool empty() const;
    Hash hash_function() const;
    template <typename T>
    void insert(T&&);
    void erase(const KeyType&);
    iterator find(const KeyType&);
    const_iterator find(const KeyType&) const;
    ValueType& operator[](const KeyType&);
    const ValueType& at(const KeyType&) const;
    void clear();    

private:
    VectorOfLists table_;
    Hash hash_;
    size_t size_;

    void rehash(size_t new_size);
};

template <typename KeyType, typename ValueType, typename Hash>
size_t HashMap<KeyType, ValueType, Hash>::size() const {
    return size_;
}

template <typename KeyType, typename ValueType, typename Hash>
bool HashMap<KeyType, ValueType, Hash>::empty() const {
    return size() == 0;
}

template <typename KeyType, typename ValueType, typename Hash>
Hash HashMap<KeyType, ValueType, Hash>::hash_function() const {
    return hash_;
}

template <typename KeyType, typename ValueType, typename Hash>
template <typename T>
void HashMap<KeyType, ValueType, Hash>::insert(T&& pair) {
    if (find(pair.first) == end()) { // if not such element inserted
        table_[hash_(pair.first) % table_.size()].push_front(std::forward<T>(pair));
        ++size_;
        if (size_ > 0.5 * table_.size()) {
            rehash(2 * table_.size());
        }
    } else {
        return;
    }
}

template <typename KeyType, typename ValueType, typename Hash>
void HashMap<KeyType, ValueType, Hash>::erase(const KeyType& key) {
    size_t hash_idx = hash_(key) % table_.size();
    for (auto itr = table_[hash_idx].begin(); itr != table_[hash_idx].end(); ++itr) {
        if (itr->first == key) {
            itr = table_[hash_idx].erase(itr);
            --size_;
            break;
        }
    }
}

template <typename KeyType, typename ValueType, typename Hash>
typename HashMap<KeyType, ValueType, Hash>::iterator
HashMap<KeyType, ValueType, Hash>::find(const KeyType& key) {
    size_t table_idx = hash_(key) % table_.size();
    auto itr = table_[table_idx].begin();
    while (itr != table_[table_idx].end()) {
        if (itr->first == key) {
            return HashMap<KeyType, ValueType, Hash>::iterator(this,
                    table_idx + this->table_.begin(), itr);
        }
        ++itr;
    }
    return end();
}

template <typename KeyType, typename ValueType, typename Hash>
typename HashMap<KeyType, ValueType, Hash>::const_iterator
HashMap<KeyType, ValueType, Hash>::find(const KeyType& key) const {
    size_t table_idx = hash_(key) % table_.size();
    auto itr = table_[table_idx].begin();
    while (itr != table_[table_idx].end()) {
        if (itr->first == key) {
            return HashMap<KeyType, ValueType, Hash>::const_iterator(this,
                    table_idx + this->table_.begin(), itr);
        }
        ++itr;
    }
    return end();
}

template <typename KeyType, typename ValueType, typename Hash>
ValueType& HashMap<KeyType, ValueType, Hash>::operator[](const KeyType& key) {
    auto itr = find(key);
    if (itr != end()) {
        return itr->second;
    }
    table_[hash_(key) % table_.size()].emplace_front(key, ValueType());
    ++size_;
    if (size_  >= 0.5 * table_.size()) {
        rehash(2 * table_.size());
    }
    return find(key)->second;
}

template <typename KeyType, typename ValueType, typename Hash>
const ValueType& HashMap<KeyType, ValueType, Hash>::at(const KeyType& key) const {
    auto itr = find(key);
    if (itr == end()) {
        throw std::out_of_range("not found");
    }
    return itr->second;
}

template <typename KeyType, typename ValueType, typename Hash>
void HashMap<KeyType, ValueType, Hash>::clear() {
    size_ = 0;
    table_ = VectorOfLists(table_size);
}

template <typename KeyType, typename ValueType, typename Hash>
void HashMap<KeyType, ValueType, Hash>::rehash(size_t size_rehashed) {
    VectorOfLists table_rehashed(size_rehashed);
    for (auto& list: table_) {
        for (auto& item: list) {
            size_t list_rehashed_idx = hash_(item.first) % table_rehashed.size();
            table_rehashed[list_rehashed_idx].push_front(std::move(item));
        }
    }
    table_ = std::move(table_rehashed);
}
