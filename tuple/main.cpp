#include <iostream>
#include <string>

template <typename T, typename U>
struct CompareType {
    static constexpr bool X = false;
};

template <typename T>
struct CompareType<T, T> {
    static constexpr bool X = true;
};

template <typename... Args>
class Tuple;

template<>
struct Tuple <> {

    bool operator < (const Tuple<>& other) const  {
        return false;
    }
    bool operator > (const Tuple<>& other) const  {
        return false;
    }

    bool operator == (const Tuple<>& other) const {
        return true;
    }

    bool operator >= (const Tuple<>& other) const {
        return true;
    }
    bool operator <= (const Tuple<>& other) const {
        return true;
    }

    static constexpr int size_ = 0;

    friend std::ostream& operator << (std::ostream& out, const Tuple<>& t) {
        return out;
    }    
};

template <typename Head, typename... Tail>
class Tuple<Head, Tail...> {
  public:  
     Tuple(const Head& head, const Tail&... tail) : head_(head), tail_(tail...) {
     }

    Tuple <Head, Tail...>& operator = (const Tuple <Head, Tail...>& other) {
      if (this != std::addressof(other)) {
          std::cout << "copying" << std::endl;
          head_ = other.head_;
          tail_ = other.tail_;
      }
      return *this;
    }     

    bool operator < (const Tuple<Head, Tail...>& other) const {
        return head_ < other.head_ || tail_ < other.tail_;
    }

    bool operator > (const Tuple<Head, Tail...>& other) const {
        return head_ > other.head_ || tail_ > other.tail_;
    }    

    bool operator == (const Tuple<Head, Tail...>& other) const  {
        return !(*this < other) && !(*this > other);
    }

    bool operator >= (const Tuple<Head, Tail...>& other) const {
        return !(*this < other);
    }

    bool operator <= (const Tuple<Head, Tail...>& other) const {
        return !(*this > other);
    }

    friend std::ostream& operator << (std::ostream& out, const Tuple<Head, Tail...>& t) {
        out << t.head_ << " " << t.tail_;
        return out;
    }

  Head head_;
  Tuple<Tail...> tail_;
  typedef Head Type;
  static constexpr int size_ = sizeof...(Tail) + 1;
};

template <int index, typename... Args>
auto& get(Tuple<Args...>& tuple) {
  if constexpr (index == 0) {
    return tuple.head_;
  } else {
    return get<index - 1>(tuple.tail_);
  }
}

template <typename T, typename... Args>
auto& get(Tuple<Args...>& tuple) {
  if constexpr (CompareType<T, typename Tuple<Args...>::Type>::X) {
    return tuple.head_;
  } else {
    return get<T>(tuple.tail_);
  }
}

template <typename Head, typename... Tail>
Tuple<Head, Tail...> makeTuple(const Head& head, const Tail&... tail) {
    return Tuple<Head, Tail...> (head, tail...);
}

namespace Helper {
    
template <typename Tuple1, int... Indexes1>
struct Concat {
  
  template <typename Tuple2, int... Indexes2>
  static auto unpack(Tuple1 t1, Tuple2 t2) {
    if constexpr (sizeof...(Indexes1) == Tuple1::size_ && sizeof...(Indexes2) == Tuple2::size_) {
      return  makeTuple(get<Indexes1>(t1)..., get<Indexes2>(t2)...);
    } else if constexpr (sizeof...(Indexes1) < Tuple1::size_) {
      return Concat<Tuple1, Indexes1..., sizeof...(Indexes1)>::template unpack<Tuple2>(t1, t2);
    } else {
      return  Concat<Tuple1, Indexes1...>::template unpack<Tuple2, Indexes2..., sizeof...(Indexes2)>(t1, t2);
    }
  }
};
}

template <typename Tuple1, typename Tuple2>
auto Concat(Tuple1 t1, Tuple2 t2) {
  return Helper::Concat<Tuple1>::template unpack<Tuple2>(t1, t2);
}

template <typename HeadTuple, typename... TailTuples>
auto TupleCat(HeadTuple head, TailTuples... tails) {
    if constexpr (sizeof...(TailTuples) == 0) {
      return head;
    } else {
      return Concat(head, TupleCat(tails...));
    }
}

template <typename Head, typename... Tail>
void print(const Head& head, const Tail&... tail) {
  std::cout << head << " " << std::endl;
  if constexpr (sizeof...(tail) != 0) {
     print(tail...);
  }
}

int main() {
  Tuple<int, int, int> t(1,2,3);
  Tuple<int, int, int> const t1(1, 5, 5);
  Tuple<int, int, double> t2(2, 4, 3.1);
  Tuple<double> t3(10.5);

  auto x = TupleCat(t1, t2, t3);

  std::cout << x << std::endl;

  return 0;
}
