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

constexpr vec2 operator*(f32 a, vec2 b) { return b * a; }

constexpr vec2 operator*(vec2 a, vec2 b) { return vec2{a.x * b.x, a.y * b.y}; }

constexpr vec2 operator/(vec2 a, vec2 b) { return vec2{a.x / b.x, a.y / b.y}; }

constexpr vec2 operator+(vec2 a, vec2 b) { return vec2{a.x + b.x, a.y + b.y}; }

constexpr vec2 operator+(vec2 a, f32 b) { return vec2{a.x + b, a.y + b}; }

constexpr vec2 operator+(f32 a, vec2 b) { return b + a; }

constexpr vec2 operator-(vec2 a, vec2 b) { return vec2{a.x - b.x, a.y - b.y}; }

constexpr vec2 operator-(vec2 a, f32 b) { return vec2{a.x - b, a.y - b}; }

constexpr vec2 operator-(f32 a, vec2 b) { return vec2{a - b.x, a - b.y}; }

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

struct rect {
  vec2 offset;
  vec2 extent;

  constexpr auto bounds() const {
    return std::make_tuple(offset.x, offset.x + extent.x, offset.y,
                           offset.y + extent.y);
  }

  constexpr bool overlaps(rect other) const {
    auto [x1_min, x1_max, y1_min, y1_max] = bounds();
    auto [x2_min, x2_max, y2_min, y2_max] = other.bounds();

    return x1_min < x2_max && x1_max > x2_min && y2_max > y1_min &&
           y2_min < y1_max;
  }
};

struct vec3 {
  f32 x = 0.0f, y = 0.0f, z = 0.0f;
};

constexpr vec3 operator-(vec3 a, f32 b) {
  return vec3{a.x - b, a.y - b, a.z - b};
}

constexpr vec3 operator-(f32 a, vec3 b) {
  return vec3{a - b.x, a - b.y, a - b.z};
}

constexpr vec3 operator*(vec3 a, f32 b) {
  return vec3{a.x * b, a.y * b, a.z * b};
}

constexpr vec3 operator*(f32 a, vec3 b) {
  return vec3{a * b.x, a * b.y, a * b.z};
}

constexpr vec3 operator/(vec3 a, vec3 b) {
  return vec3{a.x / b.x, a.y / b.y, a.z / b.z};
}

struct vec4 {
  f32 x = 0.0f, y = 0.0f, z = 0.0f, w = 0.0f;
};

constexpr bool operator==(vec4 a, vec4 b) {
  return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
}

constexpr f32 dot(vec4 a, vec4 b) {
  return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

struct mat4 {
  vec4 data[4];

  static constexpr mat4 identity() {
    return mat4{vec4{.x = 1.0f, .y = 0.0f, .z = 0.0f, .w = 0.0f},
                vec4{.x = 0.0f, .y = 1.0f, .z = 0.0f, .w = 0.0f},
                vec4{.x = 0.0f, .y = 0.0f, .z = 1.0f, .w = 0.0f},
                vec4{.x = 0.0f, .y = 0.0f, .z = 0.0f, .w = 1.0f}};
  }

  constexpr mat4 transpose() const {
    return mat4{vec4{data[0].x, data[1].x, data[2].x, data[3].x},
                vec4{data[0].y, data[1].y, data[2].y, data[3].y},
                vec4{data[0].z, data[1].z, data[2].z, data[3].z},
                vec4{data[0].w, data[1].w, data[2].w, data[3].w}};
  }
};

constexpr mat4 operator*(mat4 const &a, mat4 const &b) {
  return mat4{
      .data = {
          vec4{dot(a.data[0],
                   vec4{b.data[0].x, b.data[1].x, b.data[2].x, b.data[3].x}),
               dot(a.data[0],
                   vec4{b.data[0].y, b.data[1].y, b.data[2].y, b.data[3].y}),
               dot(a.data[0],
                   vec4{b.data[0].z, b.data[1].z, b.data[2].z, b.data[3].x}),
               dot(a.data[0],
                   vec4{b.data[0].w, b.data[1].w, b.data[2].w, b.data[3].w})},
          vec4{dot(a.data[1],
                   vec4{b.data[0].x, b.data[1].x, b.data[2].x, b.data[3].x}),
               dot(a.data[1],
                   vec4{b.data[0].y, b.data[1].y, b.data[2].y, b.data[3].y}),
               dot(a.data[1],
                   vec4{b.data[0].z, b.data[1].z, b.data[2].z, b.data[3].x}),
               dot(a.data[1],
                   vec4{b.data[0].w, b.data[1].w, b.data[2].w, b.data[3].w})},
          vec4{dot(a.data[2],
                   vec4{b.data[0].x, b.data[1].x, b.data[2].x, b.data[3].x}),
               dot(a.data[2],
                   vec4{b.data[0].y, b.data[1].y, b.data[2].y, b.data[3].y}),
               dot(a.data[2],
                   vec4{b.data[0].z, b.data[1].z, b.data[2].z, b.data[3].x}),
               dot(a.data[2],
                   vec4{b.data[0].w, b.data[1].w, b.data[2].w, b.data[3].w})},
          vec4{dot(a.data[3],
                   vec4{b.data[0].x, b.data[1].x, b.data[2].x, b.data[3].x}),
               dot(a.data[3],
                   vec4{b.data[0].y, b.data[1].y, b.data[2].y, b.data[3].y}),
               dot(a.data[3],
                   vec4{b.data[0].z, b.data[1].z, b.data[2].z, b.data[3].x}),
               dot(a.data[3],
                   vec4{b.data[0].w, b.data[1].w, b.data[2].w, b.data[3].w})}}};
}

constexpr bool operator==(mat4 const &a, mat4 const &b) {
  return a.data[0] == b.data[0] && a.data[1] == b.data[1] &&
         a.data[2] == b.data[2] && a.data[3] == b.data[3];
}

struct Offset {
  u32 x = 0, y = 0;
};

constexpr Offset operator+(Offset a, Offset b) {
  return Offset{.x = a.x + b.x, .y = a.y + b.y};
}

constexpr bool operator==(Offset a, Offset b) {
  return a.x == b.x && a.y == b.y;
}

constexpr bool operator!=(Offset a, Offset b) { return !(a == b); }

struct OffsetF {
  f32 x = 0.0f, y = 0.0f;
};

constexpr OffsetF operator+(OffsetF a, OffsetF b) {
  return OffsetF{.x = a.x + b.x, .y = a.y + b.y};
}

constexpr OffsetF operator-(OffsetF a, OffsetF b) {
  return OffsetF{.x = a.x - b.x, .y = a.y - b.y};
}

constexpr bool operator==(OffsetF a, OffsetF b) {
  return a.x == b.x && a.y == b.y;
}

constexpr bool operator!=(OffsetF a, OffsetF b) { return !(a == b); }

struct OffsetI {
  i64 x = 0, y = 0;
};

constexpr OffsetI operator+(OffsetI a, OffsetI b) {
  return OffsetI{.x = a.x + b.x, .y = a.y + b.y};
}

constexpr OffsetI operator-(OffsetI a, OffsetI b) {
  return OffsetI{.x = a.x - b.x, .y = a.y - b.y};
}

constexpr bool operator==(OffsetI a, OffsetI b) {
  return a.x == b.x && a.y == b.y;
}

constexpr bool operator!=(OffsetI a, OffsetI b) { return !(a == b); }

struct Extent {
  u32 w = 0, h = 0;

  constexpr bool is_visible() const { return w != 0 && h != 0; }

  constexpr Extent constrain(Extent other) const {
    return Extent{.w = std::min(w, other.w), .h = std::min(w, other.w)};
  }
};

constexpr Extent operator+(Extent a, Extent b) {
  return Extent{.w = a.w + b.w, .h = a.h + b.h};
}

constexpr bool operator==(Extent a, Extent b) {
  return a.w == b.w && a.h == b.h;
}

constexpr bool operator!=(Extent a, Extent b) { return !(a == b); }

struct ExtentF {
  f32 w = 0.0f, h = 0.0f;
};

constexpr ExtentF operator+(ExtentF a, ExtentF b) {
  return ExtentF{.w = a.w + b.w, .h = a.h + b.h};
}

constexpr ExtentF operator-(ExtentF a, ExtentF b) {
  return ExtentF{.w = a.w - b.w, .h = a.h - b.h};
}

constexpr bool operator==(ExtentF a, ExtentF b) {
  return a.w == b.w && a.h == b.h;
}

constexpr bool operator!=(ExtentF a, ExtentF b) { return !(a == b); }

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
    ASR_CHECK(overlaps(other));
    return intersect(other);
  }

  constexpr u32 x() const { return offset.x; }
  constexpr u32 y() const { return offset.y; }

  constexpr u32 w() const { return extent.w; }
  constexpr u32 h() const { return extent.h; }

  constexpr bool is_visible() const { return extent.is_visible(); }

  constexpr Rect with_offset(Offset new_offset) const {
    return Rect{.offset = new_offset, .extent = extent};
  }

  constexpr Rect with_extent(Extent new_extent) const {
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

  constexpr RectF with_offset(OffsetF new_offset) const {
    return RectF{.offset = new_offset, .extent = extent};
  }

  constexpr RectF with_extent(ExtentF new_extent) const {
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
    return std::make_tuple(offset.x, offset.x + AS_I64(extent.w), offset.y,
                           offset.y + AS_I64(extent.h));
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
        .extent = Extent{.w = AS_U32(std::min(x1_max, x2_max) - offset.x),
                         .h = AS_U32(std::min(y1_max, y2_max) - offset.y)}};
  }

  RectI checked_intersect(RectI const &other) const {
    ASR_CHECK(overlaps(other));
    return intersect(other);
  }

  constexpr i64 x() const { return offset.x; }
  constexpr i64 y() const { return offset.y; }

  constexpr u32 w() const { return extent.w; }
  constexpr u32 h() const { return extent.h; }

  constexpr bool is_visible() const { return extent.is_visible(); }

  constexpr RectI with_offset(OffsetI new_offset) const {
    return RectI{.offset = new_offset, .extent = extent};
  }

  constexpr RectI with_extent(Extent new_extent) const {
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
constexpr Color WHITE = Color::from_rgb(0xFF, 0xFF, 0xFF);
constexpr Color BLACK = Color::from_rgb(0x00, 0x00, 0x00);
constexpr Color RED = Color::from_rgb(0xFF, 0x00, 0x00);
constexpr Color BLUE = Color::from_rgb(0x00, 0x00, 0xFF);
constexpr Color GREEN = Color::from_rgb(0x00, 0xFF, 0x00);
constexpr Color CYAN = Color::from_rgb(0x00, 0xFF, 0xFF);
constexpr Color MAGENTA = Color::from_rgb(0xFF, 0x00, 0xFF);

}  // namespace colors

constexpr bool fits_u32(i64 value) {
  return value <= stx::u32_max && value >= 0;
}

constexpr bool fits_u32(i32 value) { return value >= 0; }

constexpr bool fits_i32(i64 value) {
  return value <= stx::i32_max && value >= stx::i32_min;
}

constexpr bool fits_i32(u32 value) { return value <= AS_U32(stx::i32_max); }

constexpr u32 u32_clamp(i64 value) {
  return AS_U32(std::clamp<i64>(value, 0, stx::u32_max));
}

constexpr u32 u32_clamp(i32 value) { return AS_U32(std::max<i32>(value, 0)); }

constexpr i32 i32_clamp(i64 value) {
  return AS_I32(std::clamp<i64>(value, stx::i32_min, stx::i32_max));
}

constexpr i32 i32_clamp(u32 value) {
  return AS_I32(std::min<u32>(value, stx::i32_max));
}

constexpr bool fits_u32(OffsetI offset) {
  return fits_u32(offset.x) && fits_u32(offset.y);
}

constexpr bool fits_i32(OffsetI offset) {
  return fits_i32(offset.x) && fits_i32(offset.y);
}

constexpr bool fits_i32(Offset offset) {
  return fits_i32(offset.x) && fits_i32(offset.y);
}

constexpr std::pair<i32, i32> i32_clamp(Offset offset) {
  return std::make_pair(i32_clamp(offset.x), i32_clamp(offset.y));
}

constexpr bool fits_i32(Extent extent) {
  return fits_i32(extent.w) && fits_i32(extent.h);
}

constexpr std::pair<i32, i32> i32_clamp(Extent extent) {
  return std::make_pair(i32_clamp(extent.w), i32_clamp(extent.h));
}

struct ImageDims {
  u32 width = 0;
  u32 height = 0;
  u32 nchannels = 4;

  constexpr usize size() const {
    usize result = width;
    result *= height;
    result *= nchannels;

    return result;
  }
};

}  // namespace asr
