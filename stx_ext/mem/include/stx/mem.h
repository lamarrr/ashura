#pragma once

#include <atomic>
#include <cinttypes>
#include <cstddef>
#include <utility>

#include "stx/allocator.h"
#include "stx/rc.h"
#include "stx/result.h"
#include "stx/struct.h"

namespace stx {
/// thread-safe
///
///
/// RefCnt objects should be created in batches to avoid false
/// sharing issues
///
///
/// we assume the user is sharing data/instructions and their side effects via a
/// shared object shared across threads, so we perform acquire when performing
/// unref.
///
struct AtomicRefCount {
  STX_MAKE_PINNED(AtomicRefCount)

  std::atomic<uint64_t> ref_count;

  explicit AtomicRefCount(uint64_t initial_ref_count)
      : ref_count{initial_ref_count} {}

  uint64_t ref() { return ref_count.fetch_add(1, std::memory_order_relaxed); }

  [[nodiscard]] uint64_t unref() {
    return ref_count.fetch_sub(1, std::memory_order_acquire);
  }
};

// if for a single type, we can use a pool allocator depending on whether it is
// trivially destructible or not. and not have to store destructors. but that
// would rarely happen in our use case.
//
//
//
//
// we need to store manager too if non-trivial.
//
//
//
// needs lock to hold pool when allocating memory
//
// non-trivially destructible?
// allocate
// ObjectManager{ Object; AtomicRefCount;   void unref() {  delete Object;   } }
// allocate storage for pool
//
// trivially destructible blocks with trivial chunks, needs no destructor
// non-trivially destructible blocks with trivial chunks, needs destructor
//
//
// Memory Chunk - uninitialized memory, needs to be destroyed once the last
// object is destroyed, i.e. refcount reaches zero
//
// Memory Chunk  Rc{refcount, memory}. will need destructor function pointer to
// run once unref is called?
//
// create object = Rc<Memory>::share() => new(memory) {object} => Rc<Object*>
// destructor????
//
//
template <typename Object>
struct DeallocateObject {
  STX_MAKE_PINNED(DeallocateObject)

  using object_type = Object;

  // placed in union to ensure the object's destructor is not called
  union {
    Object object;
  };

  Allocator allocator;

  template <typename... Args>
  explicit DeallocateObject(Allocator iallocator, Args&&... args)
      : object{std::forward<Args>(args)...}, allocator{std::move(iallocator)} {}

  constexpr void operator()(void* memory) {
    object.~Object();
    allocator.handle->deallocate(memory);
  }

  ~DeallocateObject() {}
};

/// thread-safe in ref-count and deallocation only
///
/// an independently managed object/memory.
///
/// can be used for bulk object sharing.
///
template <typename Functor>
struct RefCntOperation : public ManagerHandle {
  static_assert(std::is_invocable_v<Functor, void*>);

  STX_MAKE_PINNED(RefCntOperation)

  AtomicRefCount ref_count;

  // operation to be performed once. i.e.
  // once the ref count reaches zero.
  Functor operation;

  template <typename... Args>
  RefCntOperation(uint64_t initial_ref_count, Args&&... args)
      : ref_count{initial_ref_count}, operation{std::forward<Args>(args)...} {}

  virtual void ref() override final { ref_count.ref(); }

  virtual void unref() override final {
    /// NOTE: the last user of the object might have made modifications to the
    /// object's address just before unref is called, this means we need to
    /// ensure correct ordering of the operations/instructions relative to the
    /// unref call (instruction re-ordering).
    ///
    /// we don't need to ensure correct ordering of instructions after this
    /// atomic operation since it is non-observable anyway, since the handle
    /// will be deleted afterwards
    ///
    if (ref_count.unref() == 1) {
      operation(this);
    }
  }
};

namespace dyn {
namespace rc {

template <typename T, typename... Args>
Result<Rc<T*>, AllocError> make_inplace(Allocator allocator, Args&&... args) {
  TRY_OK(memory, mem::allocate(allocator,
                               sizeof(RefCntOperation<DeallocateObject<T>>)));

  void* mem = memory.handle;

  // release ownership of memory
  memory.allocator = allocator_stub;

  using destroy_operation_type = RefCntOperation<DeallocateObject<T>>;

  destroy_operation_type* destroy_operation_handle =
      new (mem) RefCntOperation<DeallocateObject<T>>{
          0, std::move(allocator), std::forward<Args>(args)...};

  // this polymorphic manager manages itself.
  // unref can be called on a polymorphic manager with a different pointer since
  // it doesn't need the handle, it can delete itself independently
  Manager manager{*destroy_operation_handle};

  // we perform a reference count increment for debugging/runtime hooking
  // purpose. I know we can start with a 1 ref-count but no.
  manager.ref();

  Rc<destroy_operation_type*> destroy_operation_rc{
      static_cast<destroy_operation_type*>(destroy_operation_handle), manager};

  return Ok(transmute(&destroy_operation_handle->operation.object,
                      std::move(destroy_operation_rc)));
}

template <typename T>
auto make(Allocator allocator, T value) {
  return make_inplace<T>(allocator, std::move(value));
}

/// adopt an object memory handle that is guaranteed to be valid for the
/// lifetime of this mem::Rc struct and any mem::Rc structs constructed or
/// assigned from it. typically used for static storage lifetimes.
///
/// it is advised that this should not be used for scope-local storage as it
/// would be difficult to guarantee that a called function does not retain a
/// copy or move an mem::Rc constructed using this method. However, static
/// storage objects live for the whole duration of the program so this is safe.
///
template <typename T>
Rc<T*> make_static(T& object) {
  Manager manager = static_storage_manager;
  manager.ref();
  return Rc<T*>{&object, std::move(manager)};
}

}  // namespace rc
}  // namespace dyn
}  // namespace stx
