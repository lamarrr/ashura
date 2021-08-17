
#pragma once
#include <cinttypes>
#include <cstddef>
#include <cstdlib>

namespace stx {

enum class [[nodiscard]] AllocError : uint8_t{None, NoMemory};

//
//
/// TODO(lamarrr): move allocator to mem package?
//
//

// Allocator is just a handle.
//
// a static allocator is always available for the lifetime of the program.
//
// a static allocator *should* be thread-safe.
// (should because, single-threaded programs don't need them to be).
//
// allocators must never throw
//
struct StaticAllocatorHandle {
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

struct NoopAllocatorHandle final : public StaticAllocatorHandle {
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

inline NoopAllocatorHandle noop_allocator_handle{};

// it has no memory once program is initialized
struct StaticStorageAllocatorHandle final : public StaticAllocatorHandle {
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

inline StaticStorageAllocatorHandle static_storage_allocator_handle{};

struct OsAllocatorHandle final : public StaticAllocatorHandle {
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

inline OsAllocatorHandle os_allocator_handle{};

struct StaticAllocator {
  explicit constexpr StaticAllocator(StaticAllocatorHandle &allocator_handle)
      : handle{&allocator_handle} {}

  constexpr StaticAllocator(StaticAllocator &&other) : handle{other.handle} {
    other.handle = &noop_allocator_handle;
  }

  constexpr StaticAllocator &operator=(StaticAllocator &&other) {
    handle = other.handle;
    other.handle = &noop_allocator_handle;

    return *this;
  }

  constexpr StaticAllocator(StaticAllocator const &) = default;

  constexpr StaticAllocator &operator=(StaticAllocator const &other) = default;

  AllocError allocate(void *&out_mem, size_t size) const {
    return handle->allocate(out_mem, size);
  }

  AllocError reallocate(void *&out_mem, size_t new_size) const {
    return handle->reallocate(out_mem, new_size);
  }

  void deallocate(void *mem) const { handle->deallocate(mem); }

  constexpr StaticAllocatorHandle *get_handle() const { return handle; }

 private:
  StaticAllocatorHandle *handle;
};

inline StaticAllocator noop_allocator{noop_allocator_handle};
inline StaticAllocator os_allocator{os_allocator_handle};
inline StaticAllocator static_storage_allocator{
    static_storage_allocator_handle};

}  // namespace stx
