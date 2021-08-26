
#pragma once

#include <cstddef>
#include <cstring>
#include <string_view>
#include <utility>

#include "stx/allocator.h"
#include "stx/mem.h"
#include "stx/option.h"
#include "stx/rc.h"

namespace stx {

using std::string_view;

// meaning, i want to share this, but I don't care about it's source or any
// allocation operations.
// I just want to be able to read the string as long as I have this Rc.
//
// Can be copied and shared across threads.
//
//
using RcStr = Rc<string_view>;

//
// An owning byte string.
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
  Str() : allocator_{noop_allocator}, data_{nullptr}, size_{0} {}

  Str(Allocator allocator, char const* data, size_t size)
      : allocator_{allocator}, data_{data}, size_{size} {}

  Str(Str const&) = delete;
  Str& operator=(Str const&) = delete;

  Str(Str&& other)
      : allocator_{std::move(other.allocator_)},
        data_{other.data_},
        size_{other.size_} {
    other.data_ = nullptr;
    other.size_ = 0;
  }

  Str& operator=(Str&& other) {
    std::swap(allocator_, other.allocator_);
    std::swap(data_, other.data_);
    std::swap(size_, other.size_);

    return *this;
  }

  ~Str() {
    (void)size_;
    allocator_.handle->deallocate(
        const_cast<void*>(static_cast<void const*>(data_)));
  }

  char const* data() const { return data_; }
  size_t size() const { return size_; }

  char const* begin() const { return data_; }
  char const* end() const { return begin() + size(); }

  bool empty() const { return size_ == 0; }

  char operator[](size_t index) const { return data_[index]; }

  Option<char> at(size_t index) const {
    if (index >= size_) return None;

    return Some(static_cast<char>(data_[index]));
  }

  bool operator==(std::string_view other) const {
    if (size_ != other.size()) return false;

    return strncmp(data_, other.data(), size_) == 0;
  }

  bool operator!=(std::string_view other) const { return !(*this == other); }

  bool starts_with(std::string_view other) const {
    if (other.size() > size_) return false;

    return strncmp(data_, other.data(), other.size()) == 0;
  }

  bool starts_with(char c) const { return size_ > 0 && data_[0] == c; }

  bool ends_with(std::string_view other) const {
    if (other.size() > size_) return false;

    return strncmp(data_ + (size_ - other.size()), other.data(),
                   other.size()) == 0;
  }

  bool ends_with(char c) const { return size_ > 0 && data_[size_ - 1] == c; }

  operator std::string_view() const { return std::string_view{data_, size_}; }

 private:
  Allocator allocator_;
  char const* data_ = 0;
  size_t size_ = 0;
};

namespace str {

inline Result<Str, AllocError> make(Allocator allocator, std::string_view str) {
  TRY_OK(memory, mem::allocate(allocator, str.size()));

  // release ownership of memory
  memory.allocator = allocator_stub;

  return Ok(
      Str{allocator, static_cast<char const*>(memory.handle), str.size()});
}

inline Str make_static(std::string_view str) {
  return Str{static_storage_allocator, str.begin(), str.size()};
}

namespace rc {

inline RcStr make_static(std::string_view str) {
  Manager manager = static_storage_manager;
  manager.ref();
  return Rc<string_view>{std::move(str), std::move(manager)};
}

}  // namespace rc
}  // namespace str
}  // namespace stx
