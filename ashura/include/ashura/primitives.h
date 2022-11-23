#pragma once

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <tuple>
#include <utility>

#include "ashura/utils.h"
#include "stx/limits.h"
#include "stx/option.h"

namespace asr {

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using f32 = float;
using f64 = double;

using usize = size_t;
using isize = ptrdiff_t;

struct vec2 {
  f32 x = 0.0f, y = 0.0f;
};

constexpr vec2 operator*(vec2 a, f32 b) { return vec2{a.x * b, a.y * b}; }

constexpr vec2 operator*(f32 a, vec2 b) { return vec2{b.x * a, b.y * a}; }

constexpr vec2 operator+(vec2 a, vec2 b) { return vec2{a.x + b.x, a.y + b.y}; }

constexpr vec2 operator-(vec2 a, vec2 b) { return vec2{a.x - b.x, a.y - b.y}; }

constexpr bool operator==(vec2 a, vec2 b) { return a.x == b.x && a.y == b.y; }

constexpr bool operator!=(vec2 a, vec2 b) { return !(a == b); }

constexpr f32 dot(vec2 a, vec2 b) { return a.x * b.x + a.y * b.y; }

constexpr f32 cross(vec2 a, vec2 b) { return a.x * b.y - b.x * a.y; }

// check if a point is on the LEFT side of an edge
constexpr bool is_inside(vec2 a, vec2 b, vec2 point) {
  return (cross(a - b, point) + cross(b, a)) < 0.0f;
}

// calculate intersection point of two lines
constexpr vec2 intersection(vec2 a1, vec2 a2, vec2 b1, vec2 b2) {
  return ((b1 - b2) * cross(a1, a2) - (a1 - a2) * cross(b1, b2)) *
         (1.0f / cross(a1 - a2, b1 - b2));
}

constexpr bool is_inside_triangle(vec2 p1, vec2 p2, vec2 p3, vec2 point) {
  vec2 a = p3 - p2;
  vec2 b = p1 - p3;
  vec2 c = p2 - p1;
  vec2 ap = point - p1;
  vec2 bp = point - p2;
  vec2 cp = point - p3;

  return cross(a, bp) >= 0.0f && cross(c, ap) >= 0.0f && cross(b, cp) >= 0.0f;
}

struct vec3 {
  f32 x = 0.0f, y = 0.0f, z = 0.0f;
};

struct vec4 {
  f32 x = 0.0f, y = 0.0f, z = 0.0f, w = 0.0f;
};

struct mat4x4 {
  vec4 data[4]{};

  constexpr mat4x4 operator*(mat4x4 const &other) const;

  static constexpr mat4x4 identity() {
    return mat4x4{vec4{.x = 1.0f, .y = 0.0f, .z = 0.0f, .w = 0.0f},
                  vec4{.x = 0.0f, .y = 1.0f, .z = 0.0f, .w = 0.0f},
                  vec4{.x = 0.0f, .y = 0.0f, .z = 1.0f, .w = 0.0f},
                  vec4{.x = 0.0f, .y = 0.0f, .z = 0.0f, .w = 1.0f}};
  }
};

struct Offset {
  u32 x = 0, y = 0;
};

constexpr Offset operator+(Offset const &a, Offset const &b) {
  return Offset{.x = a.x + b.x, .y = a.y + b.y};
}

constexpr bool operator==(Offset const &a, Offset const &b) {
  return a.x == b.x && a.y == b.y;
}

constexpr bool operator!=(Offset const &a, Offset const &b) {
  return !(a == b);
}

struct OffsetF {
  f32 x = 0.0f, y = 0.0f;
};

constexpr OffsetF operator+(OffsetF const &a, OffsetF const &b) {
  return OffsetF{.x = a.x + b.x, .y = a.y + b.y};
}

constexpr OffsetF operator-(OffsetF const &a, OffsetF const &b) {
  return OffsetF{.x = a.x - b.x, .y = a.y - b.y};
}

constexpr bool operator==(OffsetF const &a, OffsetF const &b) {
  return a.x == b.x && a.y == b.y;
}

constexpr bool operator!=(OffsetF const &a, OffsetF const &b) {
  return !(a == b);
}

struct OffsetI {
  i64 x = 0, y = 0;
};

constexpr OffsetI operator+(OffsetI const &a, OffsetI const &b) {
  return OffsetI{.x = a.x + b.x, .y = a.y + b.y};
}

constexpr OffsetI operator-(OffsetI const &a, OffsetI const &b) {
  return OffsetI{.x = a.x - b.x, .y = a.y - b.y};
}

constexpr bool operator==(OffsetI const &a, OffsetI const &b) {
  return a.x == b.x && a.y == b.y;
}

constexpr bool operator!=(OffsetI const &a, OffsetI const &b) {
  return !(a == b);
}

struct Extent {
  u32 w = 0, h = 0;

  constexpr bool is_visible() const { return w != 0 && h != 0; }

  constexpr Extent constrain(Extent const &other) const {
    return Extent{.w = std::min(w, other.w), .h = std::min(w, other.w)};
  }
};

constexpr Extent operator+(Extent const &a, Extent const &b) {
  return Extent{.w = a.w + b.w, .h = a.h + b.h};
}

constexpr bool operator==(Extent const &a, Extent const &b) {
  return a.w == b.w && a.h == b.h;
}

constexpr bool operator!=(Extent const &a, Extent const &b) {
  return !(a == b);
}

struct ExtentF {
  f32 w = 0.0f, h = 0.0f;
};

constexpr ExtentF operator+(ExtentF const &a, ExtentF const &b) {
  return ExtentF{.w = a.w + b.w, .h = a.h + b.h};
}

constexpr ExtentF operator-(ExtentF const &a, ExtentF const &b) {
  return ExtentF{.w = a.w - b.w, .h = a.h - b.h};
}

constexpr bool operator==(ExtentF const &a, ExtentF const &b) {
  return a.w == b.w && a.h == b.h;
}

constexpr bool operator!=(ExtentF const &a, ExtentF const &b) {
  return !(a == b);
}

struct Rect {
  Offset offset;
  Extent extent;

  constexpr auto bounds() const {
    return std::make_tuple(offset.x, offset.x + extent.w, offset.y,
                           offset.y + extent.h);
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

    return Rect{.offset = Offset{.x = std::max(x1_min, x2_min),
                                 .y = std::max(y1_min, y2_min)},
                .extent = Extent{.w = std::min(x1_max, x2_max) - offset.x,
                                 .h = std::min(y1_max, y2_max) - offset.y}};
  }

  Rect checked_intersect(Rect const &other) const {
    ASR_ENSURE(overlaps(other));
    return intersect(other);
  }

  constexpr u32 x() const { return offset.x; }
  constexpr u32 y() const { return offset.y; }

  constexpr u32 w() const { return extent.w; }
  constexpr u32 h() const { return extent.h; }

  constexpr bool is_visible() const { return extent.is_visible(); }

  constexpr Rect with_offset(Offset const &new_offset) const {
    return Rect{.offset = new_offset, .extent = extent};
  }

  constexpr Rect with_extent(Extent const &new_extent) const {
    return Rect{.offset = offset, .extent = new_extent};
  }
};

constexpr bool operator==(Rect const &a, Rect const &b) {
  return a.offset == b.offset && a.extent == b.extent;
}

constexpr bool operator!=(Rect const &a, Rect const &b) { return !(a == b); }

struct RectF {
  OffsetF offset;
  ExtentF extent;

  constexpr auto bounds() const {
    return std::make_tuple(offset.x, offset.x + extent.w, offset.y,
                           offset.y + extent.h);
  }

  constexpr bool overlaps(RectF const &other) const {
    auto [x1_min, x1_max, y1_min, y1_max] = bounds();
    auto [x2_min, x2_max, y2_min, y2_max] = other.bounds();

    return x1_min < x2_max && x1_max > x2_min && y2_max > y1_min &&
           y2_min < y1_max;
  }

  constexpr bool contains(RectF const &other) const {
    auto const [x1_min, x1_max, y1_min, y1_max] = bounds();
    auto const [x2_min, x2_max, y2_min, y2_max] = other.bounds();

    return x1_min <= x2_min && x1_max >= x2_max && y1_min <= y2_min &&
           y1_max >= y2_max;
  }

  constexpr f32 x() const { return offset.x; }
  constexpr f32 y() const { return offset.y; }

  constexpr f32 w() const { return extent.w; }
  constexpr f32 h() const { return extent.h; }

  constexpr RectF with_offset(OffsetF const &new_offset) const {
    return RectF{.offset = new_offset, .extent = extent};
  }

  constexpr RectF with_extent(ExtentF const &new_extent) const {
    return RectF{.offset = offset, .extent = new_extent};
  }
};

constexpr bool operator==(RectF const &a, RectF const &b) {
  return a.offset == b.offset && a.extent == b.extent;
}

constexpr bool operator!=(RectF const &a, RectF const &b) { return !(a == b); }

struct RectI {
  OffsetI offset;
  Extent extent;

  constexpr auto bounds() const {
    return std::make_tuple(offset.x, offset.x + static_cast<i64>(extent.w),
                           offset.y, offset.y + static_cast<i64>(extent.h));
  }

  constexpr bool overlaps(RectI const &other) const {
    auto const [x1_min, x1_max, y1_min, y1_max] = bounds();
    auto const [x2_min, x2_max, y2_min, y2_max] = other.bounds();

    return x1_min < x2_max && x1_max > x2_min && y2_max > y1_min &&
           y2_min < y1_max;
  }

  constexpr bool contains(RectI const &other) const {
    auto const [x1_min, x1_max, y1_min, y1_max] = bounds();
    auto const [x2_min, x2_max, y2_min, y2_max] = other.bounds();

    return x1_min <= x2_min && x1_max >= x2_max && y1_min <= y2_min &&
           y1_max >= y2_max;
  }

  constexpr RectI intersect(RectI const &other) const {
    auto const [x1_min, x1_max, y1_min, y1_max] = bounds();
    auto const [x2_min, x2_max, y2_min, y2_max] = other.bounds();

    return RectI{
        .offset = OffsetI{.x = std::max(x1_min, x2_min),
                          .y = std::max(y1_min, y2_min)},
        .extent =
            Extent{.w = static_cast<u32>(std::min(x1_max, x2_max) - offset.x),
                   .h = static_cast<u32>(std::min(y1_max, y2_max) - offset.y)}};
  }

  RectI checked_intersect(RectI const &other) const {
    ASR_ENSURE(overlaps(other));
    return intersect(other);
  }

  constexpr i64 x() const { return offset.x; }
  constexpr i64 y() const { return offset.y; }

  constexpr u32 w() const { return extent.w; }
  constexpr u32 h() const { return extent.h; }

  constexpr bool is_visible() const { return extent.is_visible(); }

  constexpr RectI with_offset(OffsetI const &new_offset) const {
    return RectI{.offset = new_offset, .extent = extent};
  }

  constexpr RectI with_extent(Extent const &new_extent) const {
    return RectI{.offset = offset, .extent = new_extent};
  }
};

constexpr bool operator==(RectI const &a, RectI const &b) {
  return a.offset == b.offset && a.extent == b.extent;
}

constexpr bool operator!=(RectI const &a, RectI const &b) { return !(a == b); }

struct Color {
  u8 r = 0, g = 0, b = 0, a = 255;

  static constexpr Color from_rgb(u8 r, u8 g, u8 b) {
    return Color{.r = r, .g = g, .b = b, .a = 0xFF};
  }

  constexpr Color with_red(u8 nr) const {
    return Color{.r = nr, .g = g, .b = b, .a = a};
  }

  constexpr Color with_green(u8 ng) const {
    return Color{.r = r, .g = ng, .b = b, .a = a};
  }

  constexpr Color with_blue(u8 nb) const {
    return Color{.r = r, .g = g, .b = nb, .a = a};
  }

  constexpr Color with_alpha(u8 na) const {
    return Color{.r = r, .g = g, .b = b, .a = na};
  }

  constexpr bool is_transparent() const { return a == 0; }

  constexpr bool is_visible() const { return !is_transparent(); }
};

constexpr bool operator==(Color const &a, Color const &b) {
  return a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a;
}

constexpr bool operator!=(Color const &a, Color const &b) { return !(a == b); }

namespace colors {

constexpr Color TRANSPARENT{.r = 0x00, .g = 0x00, .b = 0x00, .a = 0x00};
constexpr auto WHITE = Color::from_rgb(0xFF, 0xFF, 0xFF);
constexpr auto BLACK = Color::from_rgb(0x00, 0x00, 0x00);
constexpr auto RED = Color::from_rgb(0xFF, 0x00, 0x00);
constexpr auto BLUE = Color::from_rgb(0x00, 0x00, 0xFF);
constexpr auto GREEN = Color::from_rgb(0x00, 0xFF, 0x00);
constexpr auto CYAN = Color::from_rgb(0x00, 0xFF, 0xFF);
constexpr auto MAGENTA = Color::from_rgb(0xFF, 0x00, 0xFF);

}  // namespace colors

constexpr bool fits_u32(i64 value) {
  return value <= stx::u32_max && value >= 0;
}

constexpr bool fits_u32(i32 value) { return value >= 0; }

constexpr bool fits_i32(i64 value) {
  return value <= stx::i32_max && value >= stx::i32_min;
}

constexpr bool fits_i32(u32 value) {
  return value <= static_cast<u32>(stx::i32_max);
}

constexpr u32 u32_clamp(i64 value) {
  return static_cast<u32>(std::clamp<i64>(value, 0, stx::u32_max));
}

constexpr u32 u32_clamp(i32 value) {
  return static_cast<u32>(std::max<i32>(value, 0));
}

constexpr i32 i32_clamp(i64 value) {
  return static_cast<i32>(std::clamp<i64>(value, stx::i32_min, stx::i32_max));
}

constexpr i32 i32_clamp(u32 value) {
  return static_cast<i32>(std::min<u32>(value, stx::i32_max));
}

constexpr bool fits_u32(OffsetI const &offset) {
  return fits_u32(offset.x) && fits_u32(offset.y);
}

constexpr bool fits_i32(OffsetI const &offset) {
  return fits_i32(offset.x) && fits_i32(offset.y);
}

constexpr bool fits_i32(Offset const &offset) {
  return fits_i32(offset.x) && fits_i32(offset.y);
}

constexpr std::pair<i32, i32> i32_clamp(Offset const &offset) {
  return std::make_pair(i32_clamp(offset.x), i32_clamp(offset.y));
}

constexpr bool fits_i32(Extent const &extent) {
  return fits_i32(extent.w) && fits_i32(extent.h);
}

constexpr std::pair<i32, i32> i32_clamp(Extent const &extent) {
  return std::make_pair(i32_clamp(extent.w), i32_clamp(extent.h));
}

}  // namespace asr
