
#pragma once
#include <cinttypes>
#include <cstddef>
#include <cstdlib>
#include <utility>

#include "stx/result.h"
#include "stx/void.h"

namespace stx {

enum class [[nodiscard]] AllocError : uint8_t{None, NoMemory};

// a static allocator is always available for the lifetime of the program.
//
// a static allocator *should* be thread-safe (preferably lock-free).
// (single-threaded programs don't need them to be thread-safe).
//
// allocators MUST never throw
//
struct AllocatorHandle {
  // returns `AllocError::NoMemory` if allocation fails
  //
  // returns `nullptr` output if `size` is 0.
  //
  // alignment must be greater than 0.
  //
  virtual AllocError allocate(void *&out_mem, size_t size) = 0;

  // if there is not enough memory, the old memory block is not freed and
  // `AllocError::NoMemory` is returned without modifying the output pointer.
  //
  // if `ptr` is nullptr, it should behave as if `allocate(size, alignment)` was
  // called.
  //
  // if `new_size` is 0, the implementation must behave as if
  // `deallocate(nullptr)` is called.
  //
  // `new_size` must not be equal to the present size of the pointer memory.
  //
  // if `ptr` is not `nullptr`, it must have been previously returned by
  // `allocate`.
  //
  // if successful, the bytes in the previous pointer must be copied into the
  // new pointer.
  //
  virtual AllocError reallocate(void *&out_mem, size_t new_size) = 0;

  // if `ptr` is `nullptr`, nothing is done.
  // if `ptr` is not `nullptr`, it must have been previously allocated by
  // calling `allocate`, or `reallocate`
  //
  virtual void deallocate(void *mem) = 0;
};

struct NoopAllocatorHandle final : public AllocatorHandle {
  virtual AllocError allocate(void *&, size_t) override {
    return AllocError::NoMemory;
  }

  virtual AllocError reallocate(void *&, size_t) override {
    return AllocError::NoMemory;
  }

  virtual void deallocate(void *) override {
    // no-op
  }
};

struct AllocatorStubHandle final : public AllocatorHandle {
  virtual AllocError allocate(void *&, size_t) override {
    return AllocError::NoMemory;
  }

  virtual AllocError reallocate(void *&, size_t) override {
    return AllocError::NoMemory;
  }

  virtual void deallocate(void *) override {
    // no-op
  }
};

// it has no memory once program is initialized
struct StaticStorageAllocatorHandle final : public AllocatorHandle {
  virtual AllocError allocate(void *&, size_t) override {
    return AllocError::NoMemory;
  }

  virtual AllocError reallocate(void *&, size_t) override {
    return AllocError::NoMemory;
  }

  virtual void deallocate(void *) override {
    // no-op
  }
};

struct OsAllocatorHandle final : public AllocatorHandle {
  virtual AllocError allocate(void *&out_mem, size_t size) override {
    if (size == 0) {
      out_mem = nullptr;
      return AllocError::None;
    }

    void *mem = malloc(size);
    if (mem == nullptr) {
      return AllocError::NoMemory;
    } else {
      out_mem = mem;
      return AllocError::None;
    }
  }

  virtual AllocError reallocate(void *&out_mem, size_t new_size) override {
    if (out_mem == nullptr) {
      return allocate(out_mem, new_size);
    }

    if (new_size == 0) {
      deallocate(out_mem);
      out_mem = nullptr;
      return AllocError::None;
    }

    void *mem = std::realloc(out_mem, new_size);

    if (mem == nullptr) {
      return AllocError::NoMemory;
    } else {
      out_mem = mem;
      return AllocError::None;
    }
  }

  virtual void deallocate(void *mem) override { free(mem); }
};

constexpr const inline NoopAllocatorHandle noop_allocator_handle;
constexpr const inline StaticStorageAllocatorHandle
    static_storage_allocator_handle;
constexpr const inline OsAllocatorHandle os_allocator_handle;
constexpr const inline AllocatorStubHandle allocator_stub_handle;

struct Allocator {
  explicit constexpr Allocator(AllocatorHandle &allocator_handle)
      : handle{&allocator_handle} {}

  constexpr Allocator(Allocator &&other) : handle{other.handle} {
    other.handle = const_cast<AllocatorStubHandle *>(&allocator_stub_handle);
  }

  constexpr Allocator &operator=(Allocator &&other) {
    std::swap(handle, other.handle);
    return *this;
  }

  constexpr Allocator(Allocator const &) = default;
  constexpr Allocator &operator=(Allocator const &other) = default;

  AllocatorHandle *handle;
};

// an always-valid memory
struct Memory {
  constexpr Memory(Allocator iallocator, void *imemory)
      : allocator{iallocator}, handle{imemory} {}

  Memory(Memory const &) = delete;
  Memory &operator=(Memory const &) = delete;

  Memory(Memory &&other)
      : allocator{std::move(other.allocator)}, handle{other.handle} {}

  Memory &operator=(Memory &&other) {
    std::swap(allocator, other.allocator);
    std::swap(handle, other.handle);

    return *this;
  }

  ~Memory() { allocator.handle->deallocate(handle); }

  Allocator allocator;
  void *handle;
};

namespace mem {

inline Result<Memory, AllocError> allocate(Allocator allocator, size_t size) {
  void *memory = nullptr;

  AllocError error = allocator.handle->allocate(memory, size);

  if (error != AllocError::None) {
    return Err(AllocError{error});
  } else {
    return Ok(Memory{allocator, memory});
  }
}

inline Result<Void, AllocError> reallocate(Memory &memory, size_t new_size) {
  void *new_memory = memory.handle;

  AllocError error = memory.allocator.handle->reallocate(new_memory, new_size);

  if (error != AllocError::None) {
    return Err(AllocError{error});
  } else {
    return Ok(Void{});
  }
}

}  // namespace mem

// const-cast necessary to prove to the compiler that the contents of the
// addresses will not be changed even if the non-const functions are called.
// otherwise, when the compiler sees the allocators, it'd assume the function
// pointer's addresses changed whilst we in fact know they don't change.
//
inline constexpr const Allocator noop_allocator{
    const_cast<NoopAllocatorHandle &>(noop_allocator_handle)};

inline constexpr const Allocator os_allocator{
    const_cast<OsAllocatorHandle &>(os_allocator_handle)};

inline constexpr const Allocator static_storage_allocator{
    const_cast<StaticStorageAllocatorHandle &>(
        static_storage_allocator_handle)};

inline constexpr const Allocator allocator_stub{
    const_cast<AllocatorStubHandle &>(allocator_stub_handle)};

}  // namespace stx
