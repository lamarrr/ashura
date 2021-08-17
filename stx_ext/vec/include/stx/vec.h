#pragma once

#include <algorithm>
#include <new>
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

  explicit VecBase(StaticAllocator allocator, T* elements, size_t capacity)
      : allocator_{std::move(allocator)},
        elements_{elements},
        size_{0},
        capacity_{capacity} {}

  VecBase(VecBase&& other)
      : allocator_{std::move(other.allocator_)},
        elements_{other.elements_},
        size_{other.size_},
        capacity_{other.capacity_} {
    other.elements_ = nullptr;
    other.size_ = 0;
    other.capacity_ = 0;
  }

  VecBase& operator=(VecBase&& other) {
    impl::destroy_range(elements_, size_);
    unsafe_deallocate();

    allocator_ = std::move(other.allocator_);
    elements_ = other.elements_;
    size_ = other.size_;
    capacity_ = other.capacity_;

    other.elements_ = nullptr;
    other.size_ = 0;
    other.capacity_ = 0;
  }

  VecBase(VecBase const&) = delete;
  VecBase& operator=(VecBase const&) = delete;

  ~VecBase() {
    impl::destroy_range(elements_, size_);
    unsafe_deallocate();
  }

  T& operator[](size_t index) { return elements_[index]; }
  T const& operator[](size_t index) const { return elements_[index]; }

  Option<Ref<T>> at(size_t index) {
    if (index >= size_) {
      return None;
    } else {
      return some_ref(elements_[index]);
    }
  }

  Option<Ref<T const>> at(size_t index) const {
    if (index >= size_) {
      return None;
    } else {
      return some_ref<T const>(elements_[index]);
    }
  }

  Span<T> span() { return *this; }
  Span<T const> span() const { return *this; }

  size_t size() const { return size_; }

  size_t capacity() const { return capacity_; }

  T* data() { return elements_; }
  T const* data() const { return elements_; }

  bool empty() const { return size_ == 0; }

  T* begin() { return elements_; }
  T const* begin() const { return cbegin(); }
  T const* cbegin() const { return elements_; }

  T* end() { return elements_ + size_; }
  T const* end() const { return cend(); }
  T const* cend() const { return elements_ + size_; }

  // capacity is unchanged
  void clear() {
    impl::destroy_range(elements_, size_);
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

  auto& unsafe_allocator_ref() { return allocator_; }
  auto& unsafe_elements_ref() { return elements_; }
  auto& unsafe_size_ref() { return size_; }
  auto& unsafe_capacity_ref() { return capacity_; }
  void unsafe_deallocate() { allocator_.deallocate(elements_); }

 protected:
  StaticAllocator allocator_;
  T* elements_ = nullptr;
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

  explicit Vec(StaticAllocator allocator)
      : base{std::move(allocator), nullptr, 0} {}

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

    T* inplace_construct_pos = base::elements_ + base::size_;

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

    if (new_capacity != base::capacity_) {
      if constexpr (std::is_trivially_move_constructible_v<T> &&
                    std::is_trivially_destructible_v<T>) {
        void* ptr = base::elements_;

        AllocError err =
            base::allocator_.reallocate(ptr, new_capacity * base::element_size);

        if (err != AllocError::None) {
          return Err(AllocError{err});
        }

        base::elements_ = std::launder(reinterpret_cast<T*>(ptr));
        base::capacity_ = new_capacity;

      } else {
        void* ptr = nullptr;

        AllocError err =
            base::allocator_.allocate(ptr, new_capacity * base::element_size);

        if (err != AllocError::None) {
          return Err(AllocError{err});
        }

        T* new_elements = std::launder(reinterpret_cast<T*>(ptr));

        T* out_it = new_elements;

        for (T& element : base::span()) {
          new (out_it) T{std::move(element)};
          out_it++;
        }

        impl::destroy_range(base::elements_, base::size_);
        base::unsafe_deallocate();

        base::elements_ = new_elements;
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
  FixedVec(StaticAllocator allocator, T* memory, size_t capacity)
      : base{std::move(allocator), memory, capacity} {}

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
      new (base::elements_ + base::size_) T{std::forward<Args>(args)...};

      base::size_ = target_size;

      return Ok(Void{});
    }
  }
};

namespace vec {

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
