#pragma once

#include <algorithm>
#include <utility>

#include "stx/allocator.h"
#include "stx/result.h"
#include "stx/span.h"
#include "stx/struct.h"
#include "stx/try_ok.h"
#include "stx/void.h"

#define STX_DISABLE_BOUNDS_CHECK 0

#if STX_DISABLE_BOUNDS_CHECK
#define STX_CHECK_IN_BOUND(cmp_first, cmp_last)
#define STX_CHECK_IN_RANGE(first, last, cmp_first, cmp_last)
#else
#define STX_CHECK_IN_BOUND(cmp_first, cmp_last)
#define STX_CHECK_IN_RANGE(first, last, cmp_first, cmp_last)
#endif

// TODO
#define STX_ENSURE(condition, error_message)

namespace stx {

// TODO(lamarrr): we should find another name other than Flex
//
// TODO(lamarrr): consider adding checks to the flex[]
//

// there is not enough memory for the insertion operation
enum class FlexError : uint8_t { InsufficientMemory };

template <typename T>
constexpr void destruct_range(T* start, size_t size) {
  if constexpr (std::is_trivially_destructible_v<T>) {
  } else {
    for (T& element : stx::Span<T>{start, size}) {
      element.~T();
    }
  }
}

template <typename T>
constexpr void move_construct_range(T* start, size_t size, T* output) {
  for (T* iter = start; iter < (start + size); iter++, output++) {
    new (output) T{std::move(*iter)};
  }
}

constexpr size_t grow_flex_to_target(size_t present_capacity, size_t target) {
  return std::max(present_capacity << 1, target);
}

constexpr size_t grow_flex(size_t capacity, size_t new_target_size) {
  return capacity >= new_target_size
             ? capacity
             : grow_flex_to_target(capacity, new_target_size);
}

//
// ONLY NON-CONST METHODS INVALIDATE ITERATORS
//
//
//
// TODO(lamarrr): clear and all state-mutating ones should use std::move and a
// separate method.
//
//
//
template <typename T>
struct FlexBase {
  static_assert(!std::is_reference_v<T>);

  static constexpr size_t alignment = alignof(T);
  static constexpr size_t element_size = sizeof(T);

  // TODO(lamarrr): this should also accept size
  // we also need one that will be an empty flex
  explicit FlexBase(Memory memory, size_t size, size_t capacity)
      : memory_{std::move(memory)}, size_{size}, capacity_{capacity} {}

  FlexBase()
      : memory_{Memory{noop_allocator, nullptr}}, size_{0}, capacity_{0} {}

  explicit FlexBase(Allocator allocator)
      : memory_{Memory{allocator, nullptr}}, size_{0}, capacity_{0} {}

  FlexBase(FlexBase&& other)
      : memory_{std::move(other.memory_)},
        size_{other.size_},
        capacity_{other.capacity_} {
    other.memory_.allocator = noop_allocator;
    other.memory_.handle = nullptr;
    other.size_ = 0;
    other.capacity_ = 0;
  }

  FlexBase& operator=(FlexBase&& other) {
    std::swap(memory_, other.memory_);
    std::swap(size_, other.size_);
    std::swap(capacity_, other.capacity_);

    return *this;
  }

  STX_DISABLE_COPY(FlexBase)

  ~FlexBase() { destruct_range(iterator____elements(), size_); }

  Span<T> span() const& { return Span<T>{iterator____begin(), size_}; }

  Span<T> span() const&& = delete;

  size_t size() const { return size_; }

  size_t capacity() const { return capacity_; }

  bool empty() const { return size_ == 0; }

  T* iterator____data() const { return static_cast<T*>(memory_.handle); }

  T* iterator____elements() const { return iterator____data(); }

  T* iterator____begin() const { return iterator____data(); }

  T* iterator____end() const { return iterator____data() + size_; }

  T const* iterator____cbegin() const { return iterator____begin(); }

  T const* iterator____cend() const { return iterator____end(); }

  Memory memory_;
  size_t size_ = 0;
  size_t capacity_ = 0;
};

// Flex is an adapter to an allocator.
//
// Flex maintains a contiguous sequence of elements, and insertions or removal
// of elements causes the elements to be moved if necessary.
//
// the allocator must be alive for the lifetime of the Flex.
//
// Just like std::vector, Flex armortizes insertion to O(1) as allocation can be
// slow. but doesn't select its own allocator nor hide errors behind exceptions.
//
// Flexs are growable adapters abstractions for memory resources.
//
// Flex is also a memory allocation deferrer. i.e. it tries to minimize the
// costs of memory allocation and deallocation.
//
template <typename T>
struct Flex : public FlexBase<T> {
  using base = FlexBase<T>;

  explicit Flex(Memory memory, size_t size, size_t capacity)
      : base{std::move(memory), size, capacity} {}

  Flex() : base{} {}

  explicit Flex(Allocator allocator) : base{allocator} {}

  STX_DEFAULT_MOVE(Flex)
  STX_DISABLE_COPY(Flex)

  // test move, test move-assign
};

// a fixed capacity flex
template <typename T>
struct FixedFlex : public FlexBase<T> {
  using base = FlexBase<T>;

  explicit FixedFlex(Memory memory, size_t size, size_t capacity)
      : base{std::move(memory), size, capacity} {}

  FixedFlex() : base{} {}

  explicit FixedFlex(Allocator allocator) : base{allocator} {}

  STX_DEFAULT_MOVE(FixedFlex)
  STX_DISABLE_COPY(FixedFlex)
};

namespace flex {

// capacity = 0
// make()

template <typename T>
Result<Flex<T>, AllocError> make(Allocator allocator, size_t capacity = 0) {
  TRY_OK(memory, mem::allocate(allocator, capacity * sizeof(T)));

  return Ok(Flex<T>{std::move(memory), 0, capacity});
}

template <typename T>
Result<FixedFlex<T>, AllocError> make_fixed(Allocator allocator,
                                            size_t capacity = 0) {
  TRY_OK(memory, mem::allocate(allocator, capacity * sizeof(T)));

  return Ok(FixedFlex<T>{std::move(memory), 0, capacity});
}

// reserve enough memory to contain at least n elements
//
// does not release excess memory.
//
// returns the error if memory allocation fails
//
// invalidates references
//
template <typename T>
Result<Void, AllocError> flex____reserve(FlexBase<T>& base, size_t cap) {
  size_t new_capacity = base.capacity_ > cap ? base.capacity_ : cap;
  size_t new_capacity_bytes = new_capacity * base.element_size;

  if (new_capacity != base.capacity_) {
    if constexpr (std::is_trivially_move_constructible_v<T> &&
                  std::is_trivially_destructible_v<T>) {
      TRY_OK(ok, mem::reallocate(base.memory_, new_capacity_bytes));

      (void)ok;

      base.capacity_ = new_capacity;
    } else {
      TRY_OK(new_memory,
             mem::allocate(base.memory_.allocator, new_capacity_bytes));

      T* new_location = static_cast<T*>(new_memory.handle);

      T* iter = new_location;

      for (T& element : base.span()) {
        new (iter) T{std::move(element)};
        iter++;
      }

      destruct_range(base.iterator____begin(), base.size_);

      base.memory_ = std::move(new_memory);
      base.capacity_ = new_capacity;
    }

    return Ok(Void{});

  } else {
    return Ok(Void{});
  }
}

template <typename T>
Result<Flex<T>, AllocError> reserve(Flex<T>&& flex, size_t capacity) {
  TRY_OK(ok, flex____reserve(flex, capacity));

  (void)ok;

  return Ok(std::move(flex));
}

template <typename T>
Result<FixedFlex<T>, AllocError> reserve(FixedFlex<T>&& flex, size_t capacity) {
  TRY_OK(ok, flex____reserve(flex, capacity));

  return Ok(std::move(flex));
}

// invalidates references
//
//
// typically needed for non-movable types
template <typename T, typename... Args>
Result<Flex<T>, AllocError> push_inplace(Flex<T>&& flex, Args&&... args) {
  static_assert(std::is_constructible_v<T, Args&&...>);

  size_t const target_size = flex.size_ + 1;
  size_t const new_capacity = grow_flex(flex.capacity_, target_size);

  TRY_OK(new_flex, reserve(std::move(flex), new_capacity));

  T* inplace_construct_pos = new_flex.iterator____begin() + new_flex.size_;

  new (inplace_construct_pos) T{std::forward<Args>(args)...};

  new_flex.size_ = target_size;

  return Ok(std::move(new_flex));
}

// invalidates references
//
// value is not moved if an allocation error occurs
template <typename T>
Result<Flex<T>, AllocError> push(Flex<T>&& flex, T&& value) {
  return push_inplace(std::move(flex), std::move(value));
}

template <typename T>
Result<Flex<T>, AllocError> push(Flex<T>&& flex, T& value) = delete;

template <typename T, typename... Args>
Result<FixedFlex<T>, FlexError> push_inplace(FixedFlex<T>&& flex,
                                             Args&&... args) {
  static_assert(std::is_constructible_v<T, Args&&...>);
  size_t const target_size = flex.size_ + 1;

  if (flex.capacity_ < target_size) {
    return Err(FlexError::InsufficientMemory);
  } else {
    new (flex.iterator____begin() + flex.size_) T{std::forward<Args>(args)...};

    flex.size_ = target_size;

    return Ok(std::move(flex));
  }
}

template <typename T>
Result<FixedFlex<T>, FlexError> push(FixedFlex<T>&& flex, T&& value) {
  return push_inplace(std::move(flex), std::move(value));
}

template <typename T>
Result<FixedFlex<T>, FlexError> push(FixedFlex<T>&& flex, T& value) = delete;

// copy_with_allocator();
// copy()

// smaller size or zero?
//
///
//
//
//
//
// TODO(lamarrr): resize and move should use move semantics
//
//
//
//
//
//
//

template <typename T>
Result<Flex<T>, AllocError> resize(Flex<T>&& flex, size_t target_size,
                                   T const& to_copy = {}) {
  size_t const previous_size = flex.size();

  if (target_size > previous_size) {
    size_t const new_capacity = grow_flex(flex.capacity(), target_size);

    TRY_OK(new_flex, reserve(std::move(flex), new_capacity));

    T* copy_construct_begin = new_flex.iterator____begin() + previous_size;
    T* copy_construct_end = new_flex.iterator____begin() + target_size;

    for (T* iter = copy_construct_begin; iter < copy_construct_end; iter++) {
      new (iter) T{to_copy};
    }

    new_flex.size_ = target_size;

    return Ok(std::move(new_flex));

  } else {
    // target_size <= previous_size
    T* destruct_begin = flex.iterator____begin() + target_size;
    destruct_range(destruct_begin, previous_size - target_size);

    flex.size_ = target_size;

    return Ok(std::move(flex));
  }
}

// smaller size or zero?
template <typename T>
Result<Void, FlexError> resize(FixedFlex<T>&& flex, size_t target_size,
                               T const& to_copy = {}) {
  size_t const previous_size = flex.size();

  if (target_size > previous_size) {
    if (target_size > flex.capacity()) {
      return Err(FlexError::InsufficientMemory);
    }

    T* copy_construct_begin = flex.iterator____begin() + previous_size;
    T* copy_construct_end = flex.iterator____begin() + target_size;
    for (T* iter = copy_construct_begin; iter < copy_construct_end; iter++) {
      new (iter) T{to_copy};
    }
  } else if (target_size < previous_size) {
    T* destruct_begin = flex.iterator____begin() + target_size;
    destruct_range(destruct_begin, previous_size - target_size);
  } else {
    // equal
  }

  flex.size_ = target_size;

  return Ok(std::move(flex));
}

// capacity is unchanged
template <typename T>
void flex____clear(FlexBase<T>& base) {
  destruct_range(base.iterator____begin(), base.iterator____end());
  base.size_ = 0;
}

template <typename T>
void clear(Flex<T>&& flex) {
  flex____clear(flex);
}

template <typename T>
void clear(FixedFlex<T>&& flex) {
  flex____clear(flex);
}

// `capacity` is unchanged
//
// `first` must be a valid pointer to an element in the range or the `end` of
// the flex.
//
// `last` must be greater than `end`.
//
// TODO(lamarrr): this should take a span
//
//
template <typename T>
void flex____erase(FlexBase<T>& base, Span<T> range) {
  STX_ENSURE(base.iterator____begin() <= range.begin() &&
                 base.iterator____end() >= range.end(),
             "erase operation out of Flex range");

  size_t destruct_size = range.size();

  T* erase_start = range.begin();
  T* erase_end = range.end();

  destruct_range(erase_start, destruct_size);

  size_t num_trailing = base.iterator____end() - erase_end;

  // move trailing elements to the front
  move_construct_range(erase_end, num_trailing, erase_start);

  base.size_ -= destruct_size;
}

template <typename T>
Flex<T> erase(Flex<T>&& flex, Span<T> range) {
  flex____erase(flex, range);
  return std::move(flex);
}

template <typename T>
Flex<T> erase(FixedFlex<T>&& flex, Span<T> range) {
  flex____erase(flex, range);
  return std::move(flex);
}

}  // namespace flex
}  // namespace stx

// TODO(lamarrr): this should be handled by span.
// all element accessing content should be handled by span
//
//
//
// T& operator[](size_t index) const {
// STX_ENSURE(index < size_, "Index out of bounds");
// return iterator____elements()[index];
//}
//
// Option<Ref<T>> at(size_t index) const {
//  if (index >= size_) {
//    return None;
//  } else {
//   return some_ref<T>(iterator____elements()[index]);
// }
// }
