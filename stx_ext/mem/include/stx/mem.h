#pragma once

#include <atomic>
#include <cinttypes>
#include <cstddef>
#include <new>
#include <string_view>

#include "stx/resource.h"
#include "stx/span.h"

namespace stx {

/// memory resource management
namespace mem {

/// uses polymorphic default-delete manager
template <typename T>
using Rc = stx::Rc<T*>;

namespace pmr {

/// thread-safe
template <typename Object>
struct RefCntHandle final : public stx::pmr::ManagerHandle {
  using object_type = Object;
  Object object;
  alignas(alignof(std::max_align_t) * 2) std::atomic<uint64_t> ref_count;

  template <typename... Args>
  RefCntHandle(uint64_t initial_ref_count, Args&&... args)
      : object{std::forward<Args>(args)...}, ref_count{initial_ref_count} {}

  RefCntHandle(RefCntHandle const&) = delete;
  RefCntHandle(RefCntHandle&&) = delete;

  RefCntHandle& operator=(RefCntHandle const&) = delete;
  RefCntHandle& operator=(RefCntHandle&&) = delete;

  virtual void ref() override final {
    ref_count.fetch_add(1, std::memory_order_relaxed);
  }

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
    if (ref_count.fetch_sub(1, std::memory_order_acquire) == 1) {
      delete this;
    }
  }
};

}  // namespace pmr

/// adopt the object
///
/// reference count for this associated object must be >=1 (if any).
template <typename T>
mem::Rc<T> unsafe_make_rc(T& object, stx::pmr::Manager&& manager) {
  return stx::unsafe_make_rc<T*>(&object, std::move(manager));
}

template <typename T, typename... Args>
mem::Rc<T> make_rc_inplace(Args&&... args) {
  auto* manager_handle =
      new pmr::RefCntHandle<T>{0, std::forward<Args>(args)...};
  stx::pmr::Manager manager{*manager_handle};

  // the polymorphic manager manages itself,
  // unref can be called on a polymorphic manager with a different pointer since
  // it doesn't need the handle, it can delete itself independently
  manager.ref();

  mem::Rc<pmr::RefCntHandle<T>> manager_rc =
      unsafe_make_rc(*manager_handle, std::move(manager));

  return transmute(static_cast<T*>(&manager_handle->object),
                   std::move(manager_rc));
}

/// uses polymorphic default-delete manager
template <typename T>
mem::Rc<T> make_rc(T&& value) {
  return make_rc_inplace<T>(std::move(value));
}

// make_rc_array

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
mem::Rc<T> make_rc_for_static(T& object) {
  stx::pmr::Manager manager{stx::pmr::static_storage_manager_handle};
  manager.ref();
  return unsafe_make_rc(object, std::move(manager));
}

// requires that c_str be non-null.
inline stx::Rc<std::string_view> make_static_string_rc(char const* c_str) {
  return transmute(std::string_view{c_str}, make_rc_for_static(c_str[0]));
}

// make static array rc

}  // namespace mem
}  // namespace stx
