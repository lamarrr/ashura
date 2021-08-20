
#pragma once

#include <cstddef>
#include <string_view>
#include <utility>

#include "stx/allocator.h"
#include "stx/mem.h"
#include "stx/option.h"
#include "stx/rc.h"

namespace stx {

using std::string_view;

//
// An owning string.
//
// PROPERTIES:
// - No small string optimization (SSO)
// - always read-only
// - never null-terminated
// - doesn't support copying from copy constructors
// - it's just a plain dumb sequence of characters/bytes
//
// with these attributes, we can avoid heap allocation of static strings.
// we can be move the strings across threads.
// the string can be accessed from multiple threads with no data race.
// the string is always valid as long as lifetime of `Str` is valid.
//
struct Str {
  Str() : data_{nullptr}, size_{0}, allocator_{noop_allocator} {}

  Str(char const* data, size_t size, Allocator allocator)
      : data_{data}, size_{size}, allocator_{allocator} {}

  Str(Str const&) = delete;
  Str& operator=(Str const&) = delete;

  // TODO(lamarrr): the allocator is dis-armed but the data will still reside
  // there
  Str(Str&&) = default;
  Str& operator=(Str&&) = default;

  ~Str() {
    (void)size_;
    allocator_.deallocate(const_cast<void*>(static_cast<void const*>(data_)));
  }

  // compare
  // operator[]
  // at()
  // data
  // begin
  // end
  // cend
  // cbegin
  // empty
  // size
  // operator ==

 private:
  char const* data_ = 0;
  size_t size_ = 0;
  Allocator allocator_;
};

// meaning, i want to share this, but I don't care about it's source or any
// allocation operations.
// I just want to be able to read the string as long as I have this Rc.
//
// Can be copied and shared across threads.
//
//
using RcStr = Rc<string_view>;

namespace str {

// starts_with
// ends_with
// contains
// find

inline Str make_static(std::string_view str) {
  return Str{str.begin(), str.size(), static_storage_allocator};
}

inline RcStr make_static_rc(std::string_view str) {
  Manager manager = static_storage_manager;
  manager.ref();
  return unsafe_make_rc<std::string_view>(std::move(str), std::move(manager));
}

template <typename... Strs>
Result<Str, AllocError> join(Str const& first, Strs const&...,
                             Allocator allocator);

}  // namespace str
}  // namespace stx
