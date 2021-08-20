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

// template <typename T>
// using RcObj = Rc<T*>;

/// thread-safe
///
///
/// RefCnt objects should be created in batches to avoid false
/// sharing issues
///
///
struct AtomicRefCnt {
  STX_MAKE_PINNED(AtomicRefCnt)

  std::atomic<uint64_t> ref_count;

  explicit AtomicRefCnt(uint64_t initial_ref_count)
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
// ObjectManager{ Object; AtomicRefCnt;   void unref() {  delete Object;   } }
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
//

/// thread-safe in ref-count and deallocation only
///
/// an independently managed object/memory.
///
/// can be used for bulk object sharing.
///
///
template <typename Object>
struct RefCntObject final : public ManagerHandle {
  using object_type = Object;

  AtomicRefCnt ref_cnt;
  Allocator allocator;
  Object object;

  STX_MAKE_PINNED(RefCntObject)

  template <typename... Args>
  RefCntObject(uint64_t initial_ref_count, Allocator iallocator, Args&&... args)
      : ref_cnt{initial_ref_count},
        allocator{std::move(iallocator)},
        object{std::forward<Args>(args)...} {}

  virtual void ref() override final { ref_cnt.ref(); }

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
    if (ref_cnt.unref() == 1) {
      if constexpr (!std::is_trivially_destructible_v<Object>) {
        this->~RefCntObject();
      }
      allocator.deallocate(this);
    }
  }
};

/// memory resource management
namespace mem {

/// adopt the object
///
/// reference count for this associated object must be >=1 (if any).
template <typename T>
Rc<T*> unsafe_make_rc(T& object, Manager manager) {
  return stx::unsafe_make_rc<T*>(&object, manager);
}

template <typename T, typename... Args>
Result<Rc<T*>, AllocError> make_rc_inplace(Allocator allocator,
                                           Args&&... args) {
  void* mem = nullptr;
  AllocError error = allocator.allocate(mem, sizeof(RefCntObject<T>));

  if (error != AllocError::None) {
    return Err(AllocError{error});
  }

  RefCntObject<T>* obj_ptr =
      new (mem) RefCntObject<T>{0, allocator, std::forward<Args>(args)...};

  Manager manager{*static_cast<ManagerHandle*>(obj_ptr)};

  // the polymorphic manager manages itself,
  // unref can be called on a polymorphic manager with a different pointer since
  // it doesn't need the handle, it can delete itself independently
  manager.ref();

  Rc<RefCntObject<T>*> obj_rc = unsafe_make_rc(*obj_ptr, manager);

  return Ok(transmute(static_cast<T*>(&obj_ptr->object), std::move(obj_rc)));
}

/// uses polymorphic default-delete manager
template <typename T>
auto make_rc(Allocator allocator, T&& value) {
  // TODO(lamarrr): check for l-value references?
  return make_rc_inplace<T>(allocator, std::move(value));
}

// make_rc_array
//
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
Rc<T*> make_rc_for_static(T& object) {
  Manager manager = static_storage_manager;
  manager.ref();
  return unsafe_make_rc(object, std::move(manager));
}

}  // namespace mem
}  // namespace stx
