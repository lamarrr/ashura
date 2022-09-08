#pragma once

#include <string>
#include <string_view>
#include <utility>

#include "stx/mem.h"
#include "stx/rc.h"
#include "stx/string.h"
#include "stx/struct.h"

namespace vlk {

struct AssetTag {
  explicit AssetTag(stx::Rc<stx::StringView> rc) : tag{std::move(rc)} {}

  stx::StringView as_str() const { return tag.handle; }

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

  stx::Rc<stx::StringView> tag;
};

}  // namespace vlk
