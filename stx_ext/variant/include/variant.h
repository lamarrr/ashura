#include <cstddef>
#include <optional>
#include <type_traits>

template <typename H, typename... T>
struct all_trivially_move_constructible_impl {
  static constexpr bool value =
      std::is_trivially_move_constructible_v<H> &&
      all_trivially_move_constructible_impl<T...>::value;
};

template <typename H>
struct all_trivially_move_constructible_impl<H> {
  static constexpr bool value = std::is_trivially_move_constructible_v<H>;
};

template <typename H, typename... T>
constexpr bool all_trivially_move_constructible =
    all_trivially_move_constructible_impl<H, T...>::value;

template <typename H, typename... T>
struct all_trivially_move_assignable_impl {
  static constexpr bool value = std::is_trivially_move_assignable_v<H> &&
                                all_trivially_move_assignable_impl<T...>::value;
};

template <typename H>
struct all_trivially_move_assignable_impl<H> {
  static constexpr bool value = std::is_trivially_move_assignable_v<H>;
};

template <typename H, typename... T>
constexpr bool all_trivially_move_assignable =
    all_trivially_move_assignable_impl<H, T...>::value;

template <size_t Index, size_t HIndex, typename H, typename... T>
struct type_at_index_impl : public type_at_index_impl<Index, HIndex + 1, T...> {
};

template <size_t Index, typename H, typename... T>
struct type_at_index_impl<Index, Index, H, T...> {
  using type = H;
};

template <size_t Index, typename H, typename... T>
using type_at_index = typename type_at_index_impl<Index, 0, H, T...>::type;

template <typename Type, size_t HIndex, typename H, typename... T>
struct index_of_type_impl : public index_of_type_impl<Type, HIndex + 1, T...> {
};

template <typename Type, size_t HIndex, typename... T>
struct index_of_type_impl<Type, HIndex, Type, T...> {
  static constexpr size_t index = HIndex;
};

template <typename Type, typename H, typename... T>
constexpr size_t index_of_type = index_of_type_impl<Type, 0, H, T...>::index;

static_assert(index_of_type<int, int, float> == 0);
static_assert(index_of_type<int, float, int> == 1);

static_assert(std::is_same_v<type_at_index<0, int, float>, int>);
static_assert(
    std::is_same_v<type_at_index<1, int, float, unsigned int>, float>);

template <size_t Index, size_t HIndex, typename H, typename... T>
struct VariantStorage {
  using Tail = VariantStorage<Index + 1, HIndex + 1, T...>;
  union {
    H head;
    Tail tail;
  };
};

template <size_t Index, size_t HIndex, typename H>
struct VariantStorage<Index, HIndex, H> {
  union {
    H head;
  };
};












template <size_t Index, size_t HIndex, typename H, typename... T>
struct VariantStorage {
  using Tail = VariantStorage<Index + 1, HIndex + 1, T...>;
  union {
    H head;
    Tail tail;
  };

  template <typename Value>
  explicit constexpr VariantStorage(Value&& value) {
    if constexpr (std::is_same_v<Value, H>) {
      if constexpr (std::is_trivially_move_constructible_v<Value>) {
        head = value;
      } else {
        new (&head) H{std::move(value)};
      }
    } else {
      new (&tail) Tail(std::move(value));
    }
  }
};

template <size_t Index, size_t HIndex, typename H>
struct VariantStorage<Index, HIndex, H> {
  union {
    H head;
  };

  explicit constexpr VariantStorage(H&& value) {
    if constexpr (std::is_trivially_move_constructible_v<H>) {
      head = value;
    } else {
      new (&head) H{std::move(value)};
    }
  }
};

template <typename H, typename... T>
struct check_variant_types {};

template <typename H, typename... T>
struct Variant : check_variant_types<H, T...> {
  template <typename Q>
  static constexpr size_t index_of = index_of_type<Q, H, T...>;

  using storage = VariantStorage<0, 1 + sizeof...(T), H, T...>;

  template <typename U>
  constexpr std::optional<std::reference_wrapper<U>> get() {}

  template <typename Q>
  explicit constexpr Variant(Q&& value) {
    pack.move_construct(std::move(value));
  }

  template <typename... Invocables>
  void match();

  template <typename U>
  constexpr bool is() const {
    return init_index == index_of<U>;
  }

 private:
  // check not repeating
  storage pack;
  size_t init_index;
};
