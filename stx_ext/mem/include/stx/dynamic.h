

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
struct Dynamic {
  explicit Dynamic(Memory&& object) : memory{std::move(object)} {}

  Dynamic(Dynamic const&) = delete;
  Dynamic& operator=(Dynamic const&) = delete;

  Dynamic(Dynamic&& other) : memory{std::move(other.memory)} {
    should_destruct = other.should_destruct;
    other.should_destruct = false;
  }

  Dynamic& operator=(Dynamic&& other) {
    std::swap(memory, other.memory);
    std::swap(should_destruct, other.should_destruct);

    return *this;
  }

  ~Dynamic() {
    if (should_destruct) {
      T* object = static_cast<T*>(memory.handle);
      object->~T();
    }
  }

  constexpr T& operator*() const { return *static_cast<T*>(memory.handle); }

  constexpr T* operator->() const { return static_cast<T*>(memory.handle); }

  Memory memory;
  // this encodes the semantics of the C++ object model,
  // destructors need to be run for heap-stored non-trivial objects.
  // we are avoiding using nullptr or any sentinel values.
  //
  // moved-from memory doesn't need to run the destructor as its
  // representing object would have been moved.
  // trivially destructible types too don't need it.
  //
  bool should_destruct = !std::is_trivially_destructible_v<T>;
};

namespace dyn {

template <typename T, typename... Args>
Result<Dynamic<T>, AllocError> make_inplace(Allocator allocator,
                                            Args&&... args) {
  TRY_OK(memory, mem::allocate(allocator, sizeof(T)));

  new (memory.handle) T{std::forward<Args>(args)...};

  return Ok(Dynamic<T>{std::move(memory)});
}

template <typename T>
Result<Dynamic<T>, AllocError> make(Allocator allocator, T object) {
  return make_inplace<T>(allocator, std::move(object));
}

}  // namespace dyn
}  // namespace stx
