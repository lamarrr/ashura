#pragma once

#include <atomic>
#include <cinttypes>
#include <cstddef>
#include <string_view>

#include "stx/resource.h"
#include "stx/span.h"

namespace stx {

/// memory resource management
namespace mem {

/// uses polymorphic default-delete manager
template <typename T>
using Rc = stx::Rc<T*>;

/// thread-safe
///
///
/// TODO(lamarrr): RefCnt objects should be created in batches to avoid false
/// sharing issues
///
///
template <typename Object>
struct RefCnt final : public pmr::ManagerHandle {
  using object_type = Object;

  Object object;
  std::atomic<uint64_t> ref_count;
  // TODO(lamarrr): needs allocator

  template <typename... Args>
  RefCnt(uint64_t initial_ref_count, Args&&... args)
      : object{std::forward<Args>(args)...}, ref_count{initial_ref_count} {}

  RefCnt(RefCnt const&) = delete;
  RefCnt& operator=(RefCnt const&) = delete;
  RefCnt(RefCnt&&) = delete;
  RefCnt& operator=(RefCnt&&) = delete;

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

/// adopt the object
///
/// reference count for this associated object must be >=1 (if any).
template <typename T>
mem::Rc<T> unsafe_make_rc(T& object, pmr::Manager&& manager) {
  return stx::unsafe_make_rc<T*>(&object, std::move(manager));
}

template <typename T, typename... Args>
mem::Rc<T> make_rc_inplace(Args&&... args) {
  // TODO(lamarrr): accept allocator
  auto* manager_handle = new RefCnt<T>{0, std::forward<Args>(args)...};
  pmr::Manager manager{*manager_handle};

  // the polymorphic manager manages itself,
  // unref can be called on a polymorphic manager with a different pointer since
  // it doesn't need the handle, it can delete itself independently
  manager.ref();

  mem::Rc<RefCnt<T>> manager_rc =
      unsafe_make_rc(*manager_handle, std::move(manager));

  return transmute(static_cast<T*>(&manager_handle->object),
                   std::move(manager_rc));
}

/// uses polymorphic default-delete manager
template <typename T>
mem::Rc<T> make_rc(T&& value) {
  // TODO(lamarrr): check for l-value references?
  return make_rc_inplace<T>(std::move(value));
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
mem::Rc<T> make_rc_for_static(T& object) {
  pmr::Manager manager{pmr::static_storage_manager_handle};
  manager.ref();
  return unsafe_make_rc(object, std::move(manager));
}

// requires that c_str be non-null.
inline stx::Rc<std::string_view> make_static_string_rc(
    std::string_view string) {
  stx::pmr::Manager manager{stx::pmr::static_storage_manager_handle};
  manager.ref();
  return stx::unsafe_make_rc<std::string_view>(std::move(string),
                                               std::move(manager));
}

// TODO(lamarrr): make static array rc

// TODO(lamarrr): make chunk

//
template <typename Target, typename Source>
Rc<Target> cast(Rc<Source>&& source) {
  Target* target = static_cast<Target*>(source.get());
  return transmute(static_cast<Target*>(target), std::move(source));
}

template <typename Target, typename Source>
Rc<Target> cast(Rc<Source> const& source) {
  return transmute(static_cast<Target*>(source.get()), source);
}

}  // namespace mem
}  // namespace stx
