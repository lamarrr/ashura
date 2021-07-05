#pragma once

#include <algorithm>
#include <cstdint>
#include <tuple>

#include "stx/option.h"
#include "vlk/utils/utils.h"

namespace vlk {

using ZIndex = int64_t;

constexpr bool fits_u32(int64_t value) {
  return value <= u32_max && value >= 0;
}

constexpr bool fits_u32(int32_t value) { return value >= 0; }

constexpr bool fits_i32(int64_t value) {
  return value <= i32_max && value >= i32_min;
}

constexpr bool fits_i32(uint32_t value) {
  return value <= static_cast<uint32_t>(i32_max);
}

// TODO(lamarrr): implement
constexpr uint32_t u32_clamp(int64_t value) {
  return static_cast<uint32_t>(std::clamp<int64_t>(value, 0, u32_max));
}

constexpr uint32_t u32_clamp(int32_t value) {
  return static_cast<uint32_t>(std::clamp<int32_t>(value, 0, i32_max));
}

constexpr int32_t i32_clamp(int64_t value) {
  return static_cast<int32_t>(std::clamp<int64_t>(value, i32_min, i32_max));
}

constexpr int32_t i32_clamp(uint32_t value) {
  return static_cast<int32_t>(std::clamp<uint32_t>(value, 0, i32_max));
}

struct IOffset {
  int64_t x = 0;
  int64_t y = 0;
};

constexpr IOffset operator+(IOffset const &a, IOffset const &b) {
  return IOffset{a.x + b.x, a.y + b.y};
}

constexpr IOffset operator-(IOffset const &a, IOffset const &b) {
  return IOffset{a.x - b.x, a.y - b.y};
}

constexpr bool operator==(IOffset const &a, IOffset const &b) {
  return a.x == b.x && a.y == b.y;
}

constexpr bool operator!=(IOffset const &a, IOffset const &b) {
  return !(a == b);
}

constexpr bool fits_u32(IOffset const &offset) {
  return fits_u32(offset.x) && fits_u32(offset.y);
}

constexpr bool fits_i32(IOffset const &offset) {
  return fits_i32(offset.x) && fits_i32(offset.y);
}

struct Offset {
  uint32_t x = 0;
  uint32_t y = 0;

  explicit constexpr operator IOffset() const { return IOffset{x, y}; }
};

constexpr Offset operator+(Offset const &a, Offset const &b) {
  return Offset{a.x + b.x, a.y + b.y};
}

constexpr bool operator==(Offset const &a, Offset const &b) {
  return a.x == b.x && a.y == b.y;
}

constexpr bool operator!=(Offset const &a, Offset const &b) {
  return !(a == b);
}

constexpr bool fits_i32(Offset const &offset) {
  return fits_i32(offset.x) && fits_i32(offset.y);
}

constexpr std::pair<int32_t, int32_t> i32_clamp(Offset const &offset) {
  return std::make_pair(i32_clamp(offset.x), i32_clamp(offset.y));
}

struct Extent {
  uint32_t width = 0;
  uint32_t height = 0;

  constexpr bool visible() const { return width != 0 && height != 0; }

  constexpr Extent constrain(Extent const &other) const {
    return Extent{std::min(width, other.width), std::min(height, other.height)};
  }
};

constexpr Extent operator+(Extent const &a, Extent const &b) {
  return Extent{a.width + b.width, a.height + b.height};
}

constexpr bool operator==(Extent const &a, Extent const &b) {
  return a.width == b.width && a.height == b.height;
}

constexpr bool operator!=(Extent const &a, Extent const &b) {
  return !(a == b);
}

constexpr bool fits_i32(Extent const &extent) {
  return fits_i32(extent.width) && fits_i32(extent.height);
}

constexpr std::pair<int32_t, int32_t> i32_clamp(Extent const &extent) {
  return std::make_pair(i32_clamp(extent.width), i32_clamp(extent.height));
}

struct IRect {
  IOffset offset;
  Extent extent;

  constexpr auto bounds() const {
    return std::make_tuple(
        offset.x, offset.x + static_cast<int64_t>(extent.width), offset.y,
        offset.y + static_cast<int64_t>(extent.height));
  }

  constexpr bool overlaps(IRect const &other) const {
    auto const [x1_min, x1_max, y1_min, y1_max] = bounds();
    auto const [x2_min, x2_max, y2_min, y2_max] = other.bounds();

    return x1_min < x2_max && x1_max > x2_min && y2_max > y1_min &&
           y2_min < y1_max;
  }

  constexpr bool contains(IRect const &other) const {
    auto const [x1_min, x1_max, y1_min, y1_max] = bounds();
    auto const [x2_min, x2_max, y2_min, y2_max] = other.bounds();

    return x1_min <= x2_min && x1_max >= x2_max && y1_min <= y2_min &&
           y1_max >= y2_max;
  }

  constexpr IRect intersect(IRect const &other) const {
    auto const [x1_min, x1_max, y1_min, y1_max] = bounds();
    auto const [x2_min, x2_max, y2_min, y2_max] = other.bounds();

    IOffset offset{};

    offset.x = std::max(x1_min, x2_min);
    offset.y = std::max(y1_min, y2_min);

    Extent extent{};
    extent.width = static_cast<uint32_t>(std::min(x1_max, x2_max) - offset.x);
    extent.height = static_cast<uint32_t>(std::min(y1_max, y2_max) - offset.y);

    return IRect{offset, extent};
  }

  IRect checked_intersect(IRect const &other) const {
    VLK_ENSURE(overlaps(other));
    return intersect(other);
  }

  constexpr int64_t x() const { return offset.x; }
  constexpr int64_t y() const { return offset.y; }

  constexpr uint32_t width() const { return extent.width; }
  constexpr uint32_t height() const { return extent.height; }

  constexpr bool visible() const { return extent.visible(); }

  constexpr IRect with_offset(IOffset const &new_offset) const {
    return IRect{new_offset, extent};
  }

  constexpr IRect with_extent(Extent const &new_extent) const {
    return IRect{offset, new_extent};
  }
};

constexpr bool operator==(IRect const &a, IRect const &b) {
  return a.offset == b.offset && a.extent == b.extent;
}

constexpr bool operator!=(IRect const &a, IRect const &b) { return !(a == b); }

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

    return x1_min < x2_max && x1_max > x2_min && y2_max > y1_min &&
           y2_min < y1_max;
  }

  constexpr bool contains(Rect const &other) const {
    auto const [x1_min, x1_max, y1_min, y1_max] = bounds();
    auto const [x2_min, x2_max, y2_min, y2_max] = other.bounds();

    return x1_min <= x2_min && x1_max >= x2_max && y1_min <= y2_min &&
           y1_max >= y2_max;
  }

  constexpr Rect intersect(Rect const &other) const {
    auto const [x1_min, x1_max, y1_min, y1_max] = bounds();
    auto const [x2_min, x2_max, y2_min, y2_max] = other.bounds();

    Offset offset;

    offset.x = std::max(x1_min, x2_min);
    offset.y = std::max(y1_min, y2_min);

    Extent extent;
    extent.width = std::min(x1_max, x2_max) - offset.x;
    extent.height = std::min(y1_max, y2_max) - offset.y;

    return Rect{offset, extent};
  }

  Rect checked_intersect(Rect const &other) const {
    VLK_ENSURE(overlaps(other));
    return intersect(other);
  }

  constexpr uint32_t x() const { return offset.x; }
  constexpr uint32_t y() const { return offset.y; }

  constexpr uint32_t width() const { return extent.width; }
  constexpr uint32_t height() const { return extent.height; }

  constexpr bool visible() const { return extent.visible(); }

  constexpr Rect with_offset(Offset const &new_offset) const {
    return Rect{new_offset, extent};
  }

  constexpr Rect with_extent(Extent const &new_extent) const {
    return Rect{offset, new_extent};
  }

  explicit constexpr operator IRect() const {
    return IRect{IOffset(offset), extent};
  }
};

constexpr bool operator==(Rect const &a, Rect const &b) {
  return a.offset == b.offset && a.extent == b.extent;
}

constexpr bool operator!=(Rect const &a, Rect const &b) { return !(a == b); }

//! Virtual Rects
//!

//! unit of time within the whole API.
//! NOTE: wall or system clocks are unreliable and not easily reproducible.
struct Ticks {
  uint64_t value = 0;

  constexpr uint64_t count() const { return value; }

  constexpr Ticks &operator++(int) {
    value++;
    return *this;
  }

  constexpr void reset() { value = 0; }
};

constexpr bool operator>(Ticks const &a, Ticks const &b) {
  return a.count() > b.count();
}

constexpr bool operator>=(Ticks const &a, Ticks const &b) {
  return a.count() >= b.count();
}

constexpr bool operator<(Ticks const &a, Ticks const &b) {
  return a.count() < b.count();
}

constexpr bool operator<=(Ticks const &a, Ticks const &b) {
  return a.count() <= b.count();
}

constexpr bool operator==(Ticks const &a, Ticks const &b) {
  return a.count() == b.count();
}

constexpr bool operator!=(Ticks const &a, Ticks const &b) {
  return a.count() != b.count();
}

struct Color {
  static constexpr uint32_t kRedMask = 0xFF000000U;
  static constexpr uint32_t kGreenMask = kRedMask >> 8;
  static constexpr uint32_t kBlueMask = kGreenMask >> 8;
  static constexpr uint32_t kAlphaMask = kBlueMask >> 8;

  uint32_t rgba = 0x000000FFU;

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

  constexpr bool transparent() const { return (rgba & kAlphaMask) == 0u; }

  constexpr bool visible() const { return !transparent(); }
};

constexpr bool operator==(Color const &a, Color const &b) {
  return a.rgba == b.rgba;
}

constexpr bool operator!=(Color const &a, Color const &b) { return !(a == b); }

constexpr Color operator|(Color const &a, Color const &b) {
  return Color{a.rgba | b.rgba};
}

constexpr Color operator&(Color const &a, Color const &b) {
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

struct Edges {
  uint32_t top = 0, right = 0, bottom = 0, left = 0;

  static constexpr Edges all(uint32_t value) {
    return Edges{value, value, value, value};
  }

  static constexpr Edges symmetric(uint32_t x, uint32_t y) {
    return Edges{y, x, y, x};
  }

  static constexpr Edges trbl(uint32_t t, uint32_t r, uint32_t b, uint32_t l) {
    return Edges{t, r, b, l};
  }
};

constexpr bool operator==(Edges const &a, Edges const &b) {
  return a.top == b.top && a.right == b.right && a.bottom == b.bottom &&
         a.left == b.left;
}

constexpr bool operator!=(Edges const &a, Edges const &b) { return !(a == b); }

struct Corners {
  uint32_t top_left = 0, top_right = 0, bottom_right = 0, bottom_left = 0;

  static constexpr Corners all(uint32_t value) {
    return Corners{value, value, value, value};
  }

  static constexpr Corners across(uint32_t tl_br, uint32_t tr_bl) {
    return Corners{tl_br, tr_bl, tl_br, tr_bl};
  }

  static constexpr Corners spec(uint32_t tl, uint32_t tr, uint32_t br,
                                uint32_t bl) {
    return Corners{tl, tr, br, bl};
  }
};

constexpr bool operator==(Corners const &a, Corners const &b) {
  return a.top_left == b.top_left && a.top_right == b.top_right &&
         a.bottom_right == b.bottom_right && a.bottom_left == b.bottom_left;
}

constexpr bool operator!=(Corners const &a, Corners const &b) {
  return !(a == b);
}

struct Border {
  Color color;
  Edges edges;

  static constexpr Border all(Color color, uint32_t value) {
    return Border{color, Edges::all(value)};
  }

  static constexpr Border symmetric(Color color, uint32_t x, uint32_t y) {
    return Border{color, Edges::symmetric(x, y)};
  }

  static constexpr Border trbl(Color color, uint32_t t, uint32_t r, uint32_t b,
                               uint32_t l) {
    return Border{color, Edges::trbl(t, r, b, l)};
  }
};

constexpr bool operator==(Border const &a, Border const &b) {
  return a.color == b.color && a.edges == b.edges;
}

constexpr bool operator!=(Border const &a, Border const &b) {
  return !(a == b);
}

using BorderRadius = Corners;

struct Blur {
  Blur(float x, float y) : sigma_x_{x}, sigma_y_{y} {
    VLK_ENSURE(x > 0.0f, "Gaussian Blur Sigma X must be greater than 0.0f");
    VLK_ENSURE(y > 0.0f, "Gaussian Blur Sigma Y must be greater than 0.0f");
  }

  explicit Blur(float value) : Blur{value, value} {}

  float x() const { return sigma_x_; }

  float y() const { return sigma_y_; }

  bool is_valid() const { return sigma_x_ > 0.0f && sigma_y_ > 0.0f; }

  bool operator==(Blur const &other) const {
    return f32_eq(sigma_x_, other.sigma_x_) && f32_eq(sigma_y_, other.sigma_y_);
  }

  bool operator!=(Blur const &other) const { return !(*this == other); }

 private:
  float sigma_x_;
  float sigma_y_;
};

constexpr Extent aspect_ratio_trim(Extent aspect_ratio, Extent extent) {
  float ratio = aspect_ratio.width / static_cast<float>(aspect_ratio.height);

  uint32_t width = std::min<uint32_t>(extent.height * ratio, extent.width);
  uint32_t height = std::min<uint32_t>(extent.width / ratio, extent.height);

  return Extent{width, height};
}

}  // namespace vlk
