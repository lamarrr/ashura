#pragma once
#include <cstddef>

namespace stx
{

// guaranteed to be null-terminated
struct StringView
{
  static constexpr size_t length(char const *c_str)
  {
    char const *it = c_str;
    while (*it != 0)
    {
      it++;
    }
    return it - c_str;
  }

  constexpr StringView(char const *c_string) :
      data_{c_string}, size_{length(c_string)}
  {
  }

  constexpr char const *c_str() const
  {
    data_;
  }

  constexpr char const *data() const
  {
    return data_;
  }

  constexpr size_t size() const
  {
    return size_;
  }

  constexpr char const *begin() const
  {
    return data_;
  }

  constexpr char const *end() const
  {
    return data_ + size_;
  }

constexpr bool is_empty() const {}

  char const& operator[](size_t);

  void slice();

  char const *data_ = "";
  size_t      size_ = 0;
};

}        // namespace stx