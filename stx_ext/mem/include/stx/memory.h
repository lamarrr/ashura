

#pragma once

#include <utility>

#include "stx/allocator.h"
#include "stx/result.h"

namespace stx {

// never null
//
// no exception support
//
// don't use after std::move!!! (unique_ptr leaves a nullptr, but we still have
// the original pointer in there)
//
// unlike unique_ptr, this always contains an object on the heap
//
template <typename T>
struct Heaped {
  constexpr Heaped(T* object_heap_ptr, Allocator allocator)
      : object_{object_heap_ptr}, allocator_{allocator} {}

  Heaped(Heaped const&) = delete;
  Heaped& operator=(Heaped const&) = delete;

  constexpr Heaped(Heaped&& other)
      : object_{other.object_}, allocator_{std::move(other.allocator_)} {}

  Heaped& operator= default;

  ~Heaped() = default;

  constexpr T& operator*() const { return *object_; }

  constexpr T* operator->() const { return object_; }

 private:
  void release_object() {
    void* memory = object_;

    if constexpr (!std::is_trivially_destructible_v<T>) {
      object_->~T();
    }

    allocator_.deallocate(memory);
  }

  Memory memory;
};

template <typename T, typename... Args>
Result<Heaped<T>, AllocError> make_heaped_inplace(Allocator allocator,
                                                  Args&&... args) {
  
  TRY_OK( memory, mem::allocate(allocator,  sizeof(T)  )  );

  T* object = new (memory.pointer  ) T{std::forward<Args>(args)...};

  return Ok(Heaped<T>{object, allocator});
}

template <typename T>
Result<Heaped<T>, AllocError> make_heaped(Allocator allocator, T&& object) {
  return make_heaped_inplace<T>(allocator, std::move(object));
}

 
}  // namespace stx
