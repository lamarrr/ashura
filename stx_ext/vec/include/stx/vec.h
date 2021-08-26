#pragma once

#include <algorithm>
#include <utility>

#include "stx/allocator.h"
#include "stx/result.h"
#include "stx/span.h"
#include "stx/void.h"

namespace stx {

// there is not enough memory for the insertion operation
enum class VecError : uint8_t { InsufficientMemory };

namespace impl {
template <typename T>
constexpr void destroy_range(T* start, size_t size) {
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

constexpr size_t grow_to_target(size_t present_capacity, size_t target) {
  return std::max(present_capacity << 1, target);
}

constexpr size_t grow_vec(size_t capacity, size_t new_target_size) {
  return capacity >= new_target_size
             ? capacity
             : grow_to_target(capacity, new_target_size);
}

}  // namespace impl

template <typename T>
struct VecBase {
  static_assert(!std::is_reference_v<T>);

  static constexpr size_t alignment = alignof(T);
  static constexpr size_t element_size = sizeof(T);

  explicit VecBase(Memory memory, size_t capacity)
      : memory_{std::move(memory)}, size_{0}, capacity_{capacity} {}

  VecBase(VecBase&& other)
      : memory_{std::move(other.memory_)},
        size_{other.size_},
        capacity_{other.capacity_} {
    other.size_ = 0;
    other.capacity_ = 0;
  }

  VecBase& operator=(VecBase&& other) {
    std::swap(memory_, other.memory_);
    std::swap(size_, other.size_);
    std::swap(capacity_, other.capacity_);

    return *this;
  }

  VecBase(VecBase const&) = delete;
  VecBase& operator=(VecBase const&) = delete;

  ~VecBase() { impl::destroy_range(elements(), size_); }

  T& operator[](size_t index) { return elements()[index]; }
  T const& operator[](size_t index) const { return elements()[index]; }

  Option<Ref<T>> at(size_t index) {
    if (index >= size_) {
      return None;
    } else {
      return some_ref(elements()[index]);
    }
  }

  Option<Ref<T const>> at(size_t index) const {
    if (index >= size_) {
      return None;
    } else {
      return some_ref<T const>(elements()[index]);
    }
  }

  Span<T> span() { return *this; }
  Span<T const> span() const { return *this; }

  size_t size() const { return size_; }

  size_t capacity() const { return capacity_; }

  T* data() { return static_cast<T*>(memory_.handle); }
  T const* data() const { return static_cast<T const*>(memory_.handle); }

  T* elements() { return data(); }
  T const* elements() const { return data(); }

  bool empty() const { return size_ == 0; }

  T* begin() { return data(); }
  T const* begin() const { return data(); }

  T* end() { return data() + size_; }
  T const* end() const { return data() + size_; }

  // capacity is unchanged
  void clear() {
    impl::destroy_range(begin(), size_);
    size_ = 0;
  }

  // `capacity` is unchanged
  //
  // `first` must be a valid pointer to an element in the range or the `end` of
  // the vector.
  //
  // `last` must be greater than `end`.
  //
  size_t erase(T* first, T* last) {
    if (first >= end()) {
      return 0;
    } else {
      size_t destroy_size = std::min(static_cast<size_t>(last - first), size_);

      T* erase_start = first;
      T* erase_end = first + destroy_size;

      impl::destroy_range(erase_start, destroy_size);

      size_t num_trailing = end() - erase_end;

      // move trailing elements to the front
      impl::move_construct_range(erase_end, num_trailing, erase_start);

      size_ -= destroy_size;

      return destroy_size;
    }
  }

  auto& unsafe_memory_ref() { return memory_; }
  auto& unsafe_size_ref() { return size_; }
  auto& unsafe_capacity_ref() { return capacity_; }

 protected:
  Memory memory_;
  size_t size_ = 0;
  size_t capacity_ = 0;
};

// Vec is an adapter to an allocator.
//
// Vec maintains a contiguous sequence of elements, and insertions or removal of
// elements causes the elements to be moved if necessary.
//
// the allocator must be alive for the lifetime of the Vec.
//
// Just like std::vector, Vec armortizes insertion to O(1) as allocation can be
// slow. but doesn't select its own allocator nor hide errors behind exceptions.
//
// Vecs are growable adapters abstractions for memory resources.
//
// Vec is also a memory allocation deferrer. i.e. it tries to minimize the costs
// of memory allocation and deallocation.
//
template <typename T>
struct Vec : public VecBase<T> {
  using base = VecBase<T>;

  explicit Vec(Allocator allocator) : base{Memory{allocator, nullptr}, 0} {}

  Vec(Vec&&) = default;
  Vec& operator=(Vec&&) = default;

  Vec(Vec const&) = delete;
  Vec& operator=(Vec const&) = delete;

  // test move, test move-assign

  // invalidates references
  //
  // value is not moved if an allocation error occurs
  Result<Void, AllocError> push(T&& value) {
    return push_inplace(std::move(value));
  }

  // invalidates references
  //
  //
  // typically needed for non-movable types
  template <typename... Args>
  Result<Void, AllocError> push_inplace(Args&&... args) {
    static_assert(std::is_constructible_v<T, Args&&...>);

    size_t const target_size = base::size_ + 1;
    size_t const new_capacity = impl::grow_vec(base::capacity_, target_size);

    TRY_OK(ok, reserve(new_capacity));

    (void)ok;

    T* inplace_construct_pos = base::begin() + base::size_;

    new (inplace_construct_pos) T{std::forward<Args>(args)...};

    base::size_ = target_size;

    return Ok(Void{});
  }

  // reserve enough memory to contain at least n elements
  //
  // does not release excess memory.
  //
  // returns the error if memory allocation fails
  //
  // invalidates references
  //
  Result<Void, AllocError> reserve(size_t cap) {
    size_t new_capacity = base::capacity_ > cap ? base::capacity_ : cap;
    size_t new_capacity_bytes = new_capacity * base::element_size;

    if (new_capacity != base::capacity_) {
      if constexpr (std::is_trivially_move_constructible_v<T> &&
                    std::is_trivially_destructible_v<T>) {
        TRY_OK(ok, mem::reallocate(base::memory_, new_capacity_bytes));

        (void)ok;

        base::capacity_ = new_capacity;
      } else {
        TRY_OK(new_memory,
               mem::allocate(base::memory_.allocator, new_capacity_bytes));

        T* new_location = static_cast<T*>(new_memory.handle);

        T* iter = new_location;

        for (T& element : base::span()) {
          new (iter) T{std::move(element)};
          iter++;
        }

        impl::destroy_range(base::begin(), base::size_);

        base::memory_ = std::move(new_memory);
        base::capacity_ = new_capacity;
      }

      return Ok(Void{});

    } else {
      return Ok(Void{});
    }
  }
};

// a fixed capacity vector
template <typename T>
struct FixedVec : public VecBase<T> {
  using base = VecBase<T>;

  // `memory` must be an uninitialized memory
  //
  FixedVec(Memory memory, size_t capacity)
      : base{std::move(memory), capacity} {}

  FixedVec(FixedVec&&) = default;
  FixedVec& operator=(FixedVec&&) = default;

  FixedVec(FixedVec const&) = delete;
  FixedVec& operator=(FixedVec const&) = delete;

  Result<Void, VecError> push(T&& value) {
    return push_inplace(std::move(value));
  }

  template <typename... Args>
  Result<Void, VecError> push_inplace(Args&&... args) {
    static_assert(std::is_constructible_v<T, Args&&...>);
    size_t const target_size = base::size_ + 1;

    if (base::capacity_ < target_size) {
      return Err(VecError::InsufficientMemory);
    } else {
      new (base::begin() + base::size_) T{std::forward<Args>(args)...};

      base::size_ = target_size;

      return Ok(Void{});
    }
  }
};

namespace vec {

template <typename T>
Result<FixedVec<T>, AllocError> fixed(Allocator allocator, size_t capacity) {
  TRY_OK(memory, mem::allocate(allocator, capacity));

  return Ok(FixedVec<T>{std::move(memory), capacity});
}

// copy_with_allocator();
// copy()
// void copy();
// void push();

// smaller size or zero?
template <typename T>
Result<Void, AllocError> resize(Vec<T>& vec, size_t target_size,
                                T const& to_copy = {}) {
  size_t const previous_size = vec.size();

  if (target_size > previous_size) {
    size_t const new_capacity = impl::grow_vec(vec.capacity(), target_size);

    TRY_OK(ok, vec.reserve(new_capacity));

    (void)ok;

    T* copy_construct_begin = vec.begin() + previous_size;
    T* copy_construct_end = vec.begin() + target_size;

    for (T* iter = copy_construct_begin; iter < copy_construct_end; iter++) {
      new (iter) T{to_copy};
    }
  } else if (target_size < previous_size) {
    T* destroy_begin = vec.begin() + target_size;
    impl::destroy_range(destroy_begin, previous_size - target_size);
  } else {
    // equal
  }

  vec.unsafe_size_ref() = target_size;

  return Ok(Void{});
}

// smaller size or zero?
template <typename T>
Result<Void, VecError> resize(FixedVec<T>& vec, size_t target_size,
                              T const& to_copy = {}) {
  size_t const previous_size = vec.size();

  if (target_size > previous_size) {
    if (target_size > vec.capacity()) {
      return Err(VecError::InsufficientMemory);
    }

    T* copy_construct_begin = vec.begin() + previous_size;
    T* copy_construct_end = vec.begin() + target_size;
    for (T* iter = copy_construct_begin; iter < copy_construct_end; iter++) {
      new (iter) T{to_copy};
    }
  } else if (target_size < previous_size) {
    T* destroy_begin = vec.begin() + target_size;
    impl::destroy_range(destroy_begin, previous_size - target_size);
  } else {
    // equal
  }

  vec.unsafe_size_ref() = target_size;

  return Ok(Void{});
}

}  // namespace vec
}  // namespace stx
