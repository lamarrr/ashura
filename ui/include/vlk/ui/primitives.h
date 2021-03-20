#pragma once

#include <cstdint>
#include <tuple>

#include "stx/option.h"
#include "vlk/utils/utils.h"

namespace vlk {
namespace ui {

using ZIndex = int64_t;

// type marker
template <typename T>
using Normalized = T;  // normalized range [0.0f, 1.0f], i.e. for depth buffer
                       // where we are not exposing the depth bit

struct IOffset {
  int64_t x;
  int64_t y;
};

inline constexpr IOffset operator+(IOffset const &a, IOffset const &b) {
  return IOffset{a.x + b.x, a.y + b.y};
}

inline constexpr bool operator==(IOffset const &a, IOffset const &b) {
  return a.x == b.x && a.y == b.y;
}

inline constexpr bool operator!=(IOffset const &a, IOffset const &b) {
  return !(a == b);
}

struct Offset {
  uint32_t x;
  uint32_t y;

  explicit constexpr operator IOffset() const { return IOffset{x, y}; }
};

inline constexpr Offset operator+(Offset const &a, Offset const &b) {
  return Offset{a.x + b.x, a.y + b.y};
}

inline constexpr bool operator==(Offset const &a, Offset const &b) {
  return a.x == b.x && a.y == b.y;
}

inline constexpr bool operator!=(Offset const &a, Offset const &b) {
  return !(a == b);
}

struct RelativeOffset {
  float x;
  float y;
};

struct Extent {
  uint32_t width;
  uint32_t height;

  constexpr bool is_visible() const { return width != 0 && height != 0; }
};

inline constexpr bool operator==(Extent const &a, Extent const &b) {
  return a.width == b.width && a.height == b.height;
}

inline constexpr bool operator!=(Extent const &a, Extent const &b) {
  return !(a == b);
}

struct RelativeExtent {
  float width;
  float height;
};

struct IRect {
  IOffset offset;
  Extent extent;

  constexpr auto bounds() const {
    return std::make_tuple(
        offset.x, offset.x + static_cast<int64_t>(extent.width), offset.y,
        offset.y + static_cast<int64_t>(extent.height));
  }

  // TEST
  constexpr bool overlaps(IRect const &other) const {
    auto [x1_min, x1_max, y1_min, y1_max] = bounds();
    auto [x2_min, x2_max, y2_min, y2_max] = other.bounds();

    if (y2_max > y1_min || y1_max > y2_min || x2_max > x1_min ||
        x1_max > x2_min)
      return false;

    return true;
  }

  // TEST
  constexpr bool contains(IRect const &other) const {
    auto [x1_min, x1_max, y1_min, y1_max] = bounds();
    auto [x2_min, x2_max, y2_min, y2_max] = other.bounds();

    return x1_min < x2_min && x1_max > x2_max && y1_min < y2_min &&
           y1_max > y2_max;
  }

  //  TEST
  constexpr IRect intersect(IRect const &other) const {
    auto [x1_min, x1_max, y1_min, y1_max] = bounds();
    auto [x2_min, x2_max, y2_min, y2_max] = other.bounds();

    IOffset offset = {};

    offset.x = std::max(x1_min, x2_min);
    offset.y = std::max(y1_min, y2_min);

    Extent extent = {};
    extent.width = static_cast<uint32_t>(std::min(x1_max, x2_max) - x1_min);
    extent.height = static_cast<uint32_t>(std::min(y1_max, y2_max) - y1_min);

    return IRect{offset, extent};
  }

  IRect checked_intersect(IRect const &other) const {
    VLK_ENSURE(overlaps(other));
    return intersect(other);
  }
};

inline constexpr bool operator==(IRect const &a, IRect const &b) {
  return a.offset == b.offset && a.extent == b.extent;
}

inline constexpr bool operator!=(IRect const &a, IRect const &b) {
  return !(a == b);
}

struct Rect {
  Offset offset;
  Extent extent;

  constexpr auto bounds() const {
    return std::make_tuple(offset.x, offset.x + extent.width, offset.y,
                           offset.y + extent.height);
  }

  constexpr bool overlaps(Rect const &other) const {
    auto [x1_min, x1_max, y1_min, y1_max] = bounds();
    auto [x2_min, x2_max, y2_min, y2_max] = other.bounds();

    if (y2_max > y1_min || y1_max > y2_min || x2_max > x1_min ||
        x1_max > x2_min)
      return false;

    return true;
  }

  constexpr bool contains(Rect const &other) const {
    auto [x1_min, x1_max, y1_min, y1_max] = bounds();
    auto [x2_min, x2_max, y2_min, y2_max] = other.bounds();

    return x1_min < x2_min && x1_max > x2_max && y1_min < y2_min &&
           y1_max > y2_max;
  }

  constexpr Rect intersect(Rect const &other) const {
    auto [x1_min, x1_max, y1_min, y1_max] = bounds();
    auto [x2_min, x2_max, y2_min, y2_max] = other.bounds();

    Offset offset = {};

    offset.x = std::max(x1_min, x2_min);
    offset.y = std::max(y1_min, y2_min);

    Extent extent = {};
    extent.width = std::min(x1_max, x2_max) - x1_min;
    extent.height = std::min(y1_max, y2_max) - y1_min;

    return Rect{offset, extent};
  }

  Rect checked_intersect(Rect const &other) const {
    VLK_ENSURE(overlaps(other));
    return intersect(other);
  }

  explicit constexpr operator IRect() const {
    return IRect{IOffset(offset), extent};
  }
};

inline constexpr bool operator==(Rect const &a, Rect const &b) {
  return a.offset == b.offset && a.extent == b.extent;
}

inline constexpr bool operator!=(Rect const &a, Rect const &b) {
  return !(a == b);
}

struct Ticks {
  Ticks(uint64_t value = 0) : value_{value} {}

  constexpr uint64_t count() const { return value_; }

  constexpr Ticks &operator++(int) {
    value_++;
    return *this;
  }

  constexpr void reset() { value_ = 0; }

  constexpr bool operator>(Ticks const &other) const {
    return value_ > other.value_;
  }

  constexpr bool operator>=(Ticks const &other) const {
    return value_ >= other.value_;
  }

  constexpr bool operator<(Ticks const &other) const {
    return value_ < other.value_;
  }

  constexpr bool operator<=(Ticks const &other) const {
    return value_ <= other.value_;
  }

  constexpr bool operator==(Ticks const &other) const {
    return value_ == other.value_;
  }

  constexpr bool operator!=(Ticks const &other) const {
    return value_ != other.value_;
  }

 private:
  uint64_t value_ = 0;
};

struct RelativeRect {
  RelativeOffset offset;
  RelativeExtent extent;
};

// TODO(lamarrr): fix id casing here
struct Color {
  static constexpr uint32_t kRedMask = 0xFF000000U;
  static constexpr uint32_t kGreenMask = kRedMask >> 8;
  static constexpr uint32_t kBlueMask = kGreenMask >> 8;
  static constexpr uint32_t kAlphaMask = kBlueMask >> 8;

  uint32_t rgba;

  static constexpr Color from_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    return Color{static_cast<uint32_t>(r) << 24 |
                 static_cast<uint32_t>(g) << 16 |
                 static_cast<uint32_t>(b) << 8 | a};
  }

  static constexpr Color from_argb(uint32_t argb) {
    return Color{(argb << 8) | (argb >> 24)};
  }

  static constexpr Color from_rgb(uint8_t r, uint8_t g, uint8_t b) {
    return Color::from_rgba(r, g, b, 0xFF);
  }

  constexpr uint32_t to_argb() const { return (rgba >> 8) | (rgba << 24); }

  constexpr Color with_red(uint8_t r) const {
    return Color{(rgba & ~kRedMask) | r};
  }

  constexpr Color with_green(uint8_t g) const {
    return Color{(rgba & ~kGreenMask) | g};
  }

  constexpr Color with_blue(uint8_t b) const {
    return Color{(rgba & ~kBlueMask) | b};
  }

  constexpr Color with_alpha(uint8_t a) const {
    return Color{(rgba & ~kAlphaMask) | a};
  }
};

inline constexpr bool operator==(Color const &a, Color const &b) {
  return a.rgba == b.rgba;
}

inline constexpr bool operator!=(Color const &a, Color const &b) {
  return !(a == b);
}

inline constexpr Color operator|(Color const &a, Color const &b) {
  return Color{a.rgba | b.rgba};
}

inline constexpr Color operator&(Color const &a, Color const &b) {
  return Color{a.rgba & b.rgba};
}

namespace colors {
constexpr auto Transparent = Color::from_rgba(0x00, 0x00, 0x00, 0x00);
constexpr auto White = Color::from_rgb(0xFF, 0xFF, 0xFF);
constexpr auto Black = Color::from_rgb(0x00, 0x00, 0x00);
constexpr auto Red = Color::from_rgb(0xFF, 0x00, 0x00);
constexpr auto Blue = Color::from_rgb(0x00, 0x00, 0xFF);
constexpr auto Green = Color::from_rgb(0x00, 0xFF, 0x00);
constexpr auto Cyan = Color::from_rgb(0x00, 0xFF, 0xFF);
constexpr auto Magenta = Color::from_rgb(0xFF, 0x00, 0xFF);
}  // namespace colors

struct Edge {
  uint32_t top = 0, right = 0, bottom = 0, left = 0;

  static constexpr Edge uniform(uint32_t value) {
    return Edge{value, value, value, value};
  }

  static constexpr Edge xy(uint32_t x, uint32_t y) { return Edge{y, x, y, x}; }

  static constexpr Edge trbl(uint32_t t, uint32_t r, uint32_t b, uint32_t l) {
    return Edge{t, r, b, l};
  }
};

inline constexpr bool operator==(Edge const &a, Edge const &b) {
  return a.top == b.top && a.right == b.right && a.bottom == b.bottom &&
         a.left == b.left;
}

inline constexpr bool operator!=(Edge const &a, Edge const &b) {
  return !(a == b);
}

struct Corner {
  uint32_t top_left = 0, top_right = 0, bottom_right = 0, bottom_left = 0;

  static constexpr Corner uniform(uint32_t value) {
    return Corner{value, value, value, value};
  }

  static constexpr Corner across(uint32_t tl_br, uint32_t tr_bl) {
    return Corner{tl_br, tr_bl, tl_br, tr_bl};
  }

  static constexpr Corner spec(uint32_t tl, uint32_t tr, uint32_t br,
                               uint32_t bl) {
    return Corner{tl, tr, br, bl};
  }
};

inline constexpr bool operator==(Corner const &a, Corner const &b) {
  return a.top_left == b.top_left && a.top_right == b.top_right &&
         a.bottom_right == b.bottom_right && a.bottom_left == b.bottom_left;
}

inline constexpr bool operator!=(Corner const &a, Corner const &b) {
  return !(a == b);
}

enum class Stretch : uint8_t {
  None = 0b00,
  X = 0b01,
  Y = 0b10,
  All = X | Y,
};

inline constexpr Stretch operator|(Stretch a, Stretch b) {
  return vlk::enum_or(a, b);
}

inline constexpr Stretch operator&(Stretch a, Stretch b) {
  return vlk::enum_and(a, b);
}

// TODO(lamarrr): consider using constrain with aspect ratio?

struct Sizing {
  enum class Type : uint8_t {
    // the part of the target used is a portion of the image specified in
    // pixels
    Relative,
    // the part of the target used is a portion of the image specified within
    // the range of 0.0 to 1.0f and scaled to the target's dimensions
    Absolute
  };

  explicit constexpr Sizing(Rect const &rect)
      : type_{Type::Absolute}, rect_data_{rect} {}
  explicit constexpr Sizing(RelativeRect const &rect)
      : type_{Type::Relative}, relative_data_{rect} {}

  constexpr Sizing()
      : type_{Type::Relative},
        relative_data_{RelativeRect{{0.0f, 0.0f}, {1.0f, 1.0f}}} {}

  constexpr Sizing(Sizing const &other) = default;
  constexpr Sizing(Sizing &&other) = default;
  constexpr Sizing &operator=(Sizing const &other) = default;
  constexpr Sizing &operator=(Sizing &&other) = default;
  ~Sizing() = default;

  static constexpr Sizing relative(RelativeRect const &relative) {
    return Sizing{relative};
  }

  static constexpr Sizing relative(float offset_x, float offset_y, float width,
                                   float height) {
    return Sizing{RelativeRect{RelativeOffset{offset_x, offset_y},
                               RelativeExtent{width, height}}};
  }

  static constexpr Sizing relative(float width, float height) {
    return relative(0.0f, 0.0f, width, height);
  }

  static constexpr Sizing relative() { return relative(1.0f, 1.0f); }

  static constexpr Sizing absolute(Rect const &rect) { return Sizing{rect}; }

  static constexpr Sizing absolute(uint32_t offset_x, uint32_t offset_y,
                                   uint32_t width, uint32_t height) {
    return absolute(Rect{Offset{offset_x, offset_y}, Extent{width, height}});
  }

  static constexpr Sizing absolute(uint32_t width, uint32_t height) {
    return absolute(0, 0, width, height);
  }

  constexpr Type type() const { return type_; }

  stx::Option<RelativeRect> get_relative() const {
#if VLK_ENABLE_DEBUG_CHECKS
    if (type_ == Type::Relative) return stx::Some(RelativeRect{relative_data_});
    return stx::None;
#else
    return stx::Some(RelativeRect{relative_data_});
#endif
  }

  stx::Option<Rect> get_absolute() const {
#if VLK_ENABLE_DEBUG_CHECKS
    if (type_ == Type::Absolute) return stx::Some(Rect{rect_data_});
    return stx::None;
#else
    return stx::Some(Rect{rect_data_});
#endif
  }

 private:
  Type type_;
  union {
    RelativeRect relative_data_;
    Rect rect_data_;
  };
};

}  // namespace ui
}  // namespace vlk
