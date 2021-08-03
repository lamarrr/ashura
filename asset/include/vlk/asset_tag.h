#pragma once

#include <string>
#include <string_view>
#include <utility>

#include "stx/mem.h"
#include "stx/struct.h"

namespace vlk {

namespace impl {

inline stx::Rc<std::string_view> make_rc_string_view(std::string &&str) {
  auto rc = stx::mem::make_rc(std::move(str));
  auto view = std::string_view{*rc.get()};
  return stx::transmute(std::move(view), std::move(rc));
}

}  // namespace impl

struct AssetTag {
  explicit AssetTag(std::string tag_string)
      : tag_{impl::make_rc_string_view(std::move(tag_string))} {}

  explicit AssetTag(stx::Rc<std::string_view> &&rc) : tag_{std::move(rc)} {}

  static AssetTag from_static(std::string_view ref) {
    return AssetTag{stx::mem::make_static_string_rc(ref)};
  }

  std::string_view as_str() const { return tag_.get(); }

  auto get_data() const { return tag_; }

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

 private:
  stx::Rc<std::string_view> tag_;
};

}  // namespace vlk
