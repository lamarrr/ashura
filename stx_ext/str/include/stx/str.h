
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

constexpr char const empty_string[] = "";

#define STX_ENSURE(condition, error_message)

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
//
// TODO(lamarrr): this should use the Memory abstraction?
//
//
// ITERATORS are never invalidate THROUHGOUT ITS LIFETIME
//
//
struct Str {
  Str() : memory_{static_storage_allocator, empty_string}, size_{0} {}

  Str(ReadOnlyMemory memory, size_t size)
      : memory_{std::move(memory)}, size_{size} {}

  Str(Str const&) = delete;
  Str& operator=(Str const&) = delete;

  Str(Str&& other) : memory_{std::move(other.memory_)}, size_{other.size_} {
    other.memory_.allocator = static_storage_allocator;
    other.memory_.handle = empty_string;
    other.size_ = 0;
  }

  Str& operator=(Str&& other) {
    std::swap(memory_, other.memory_);
    std::swap(size_, other.size_);

    return *this;
  }

  [[nodiscard]] char const* data() const {
    return static_cast<char const*>(memory_.handle);
  }
  [[nodiscard]] size_t size() const { return size_; }

  [[nodiscard]] char const* begin() const { return data(); }
  [[nodiscard]] char const* end() const { return begin() + size(); }

  [[nodiscard]] bool empty() const { return size_ == 0; }

  char const operator[](size_t index) const {
    STX_ENSURE(index < size_, "Index Out of Bounds");
    return data()[index];
  }

  Option<Ref<char const> > at(size_t index) const {
    if (index >= size_) return None;

    return some_ref(data()[index]);
  }

  [[nodiscard]] bool operator==(std::string_view other) const {
    if (size_ != other.size()) return false;

    return memcmp(data(), other.data(), size_) == 0;
  }

  [[nodiscard]] bool operator!=(std::string_view other) const {
    return !(*this == other);
  }

  [[nodiscard]] bool starts_with(std::string_view other) const {
    if (other.size() > size_) return false;

    return memcmp(data(), other.data(), other.size()) == 0;
  }

  [[nodiscard]] bool starts_with(char c) const {
    return size_ > 0 && data()[0] == c;
  }

  [[nodiscard]] bool ends_with(std::string_view other) const {
    if (other.size() > size_) return false;

    return memcmp(data() + (size_ - other.size()), other.data(),
                  other.size()) == 0;
  }

  [[nodiscard]] bool ends_with(char c) const {
    return size_ > 0 && data()[size_ - 1] == c;
  }

  operator std::string_view() const { return std::string_view{data(), size_}; }

  ReadOnlyMemory memory_;
  size_t size_ = 0;
};

namespace str {

inline Result<Str, AllocError> make(Allocator allocator, std::string_view str) {
  TRY_OK(memory, mem::allocate(allocator, str.size()));

  memcpy(memory.handle, str.data(), str.size());

  ReadOnlyMemory str_memory{std::move(memory)};

  return Ok(Str{std::move(str_memory), str.size()});
}

inline Str make_static(std::string_view str) {
  return Str{ReadOnlyMemory{static_storage_allocator, str.data()}, str.size()};
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
