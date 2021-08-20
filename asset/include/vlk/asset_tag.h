#pragma once

#include <string>
#include <string_view>
#include <utility>

#include "stx/mem.h"
#include "stx/struct.h"

namespace vlk {
using std::string_view;
using stx::Rc;

namespace impl {

struct AssetTag {
  explicit AssetTag(Rc<string_view> &&rc) : tag{std::move(rc)} {}

  string_view as_str() const { return tag.get(); }

  bool operator==(AssetTag const &other) const {
    return as_str() == other.as_str();
  }

  bool operator!=(AssetTag const &other) const { return !(*this == other); }

  bool operator<(AssetTag const &other) const {
    return as_str() < other.as_str();
  }

  bool operator>(AssetTag const &other) const {
    return as_str() > other.as_str();
  }

  bool operator<=(AssetTag const &other) const {
    return as_str() <= other.as_str();
  }

  bool operator>=(AssetTag const &other) const {
    return as_str() >= other.as_str();
  }

  Rc<string_view> tag;
};

}  // namespace impl
