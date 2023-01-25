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
  f32 x = 0, y = 0;
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

struct rect {
  vec2 offset, extent;

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

  constexpr bool is_visible() const { return extent.x != 0 && extent.y != 0; }
};

struct vec3 {
  f32 x = 0, y = 0, z = 0, ____pad = 0;
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

// column vector
struct vec4 {
  f32 x = 0, y = 0, z = 0, w = 0;
};

constexpr bool operator==(vec4 a, vec4 b) {
  return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
}

constexpr f32 dot(vec4 a, vec4 b) {
  return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

// row-major
struct mat4 {
  vec4 data[4];

  static constexpr mat4 identity() {
    return mat4{vec4{.x = 1, .y = 0, .z = 0, .w = 0},
                vec4{.x = 0, .y = 1, .z = 0, .w = 0},
                vec4{.x = 0, .y = 0, .z = 1, .w = 0},
                vec4{.x = 0, .y = 0, .z = 0, .w = 1}};
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
          {dot(a.data[0], {b.data[0].x, b.data[1].x, b.data[2].x, b.data[3].x}),
           dot(a.data[0], {b.data[0].y, b.data[1].y, b.data[2].y, b.data[3].y}),
           dot(a.data[0], {b.data[0].z, b.data[1].z, b.data[2].z, b.data[3].z}),
           dot(a.data[0],
               {b.data[0].w, b.data[1].w, b.data[2].w, b.data[3].w})},
          {dot(a.data[1], {b.data[0].x, b.data[1].x, b.data[2].x, b.data[3].x}),
           dot(a.data[1], {b.data[0].y, b.data[1].y, b.data[2].y, b.data[3].y}),
           dot(a.data[1], {b.data[0].z, b.data[1].z, b.data[2].z, b.data[3].z}),
           dot(a.data[1],
               {b.data[0].w, b.data[1].w, b.data[2].w, b.data[3].w})},
          {dot(a.data[2], {b.data[0].x, b.data[1].x, b.data[2].x, b.data[3].x}),
           dot(a.data[2], {b.data[0].y, b.data[1].y, b.data[2].y, b.data[3].y}),
           dot(a.data[2], {b.data[0].z, b.data[1].z, b.data[2].z, b.data[3].z}),
           dot(a.data[2],
               {b.data[0].w, b.data[1].w, b.data[2].w, b.data[3].w})},
          {dot(a.data[3], {b.data[0].x, b.data[1].x, b.data[2].x, b.data[3].x}),
           dot(a.data[3], {b.data[0].y, b.data[1].y, b.data[2].y, b.data[3].y}),
           dot(a.data[3], {b.data[0].z, b.data[1].z, b.data[2].z, b.data[3].z}),
           dot(a.data[3],
               {b.data[0].w, b.data[1].w, b.data[2].w, b.data[3].w})}}};
}

constexpr vec4 operator*(mat4 const &a, vec4 const &b) {
  return vec4{.x = dot(a.data[0], b),
              .y = dot(a.data[1], b),
              .z = dot(a.data[2], b),
              .w = dot(a.data[3], b)};
}

constexpr vec4 operator*(vec4 const &a, mat4 const &b) {
  return vec4{.x = dot(a, b.data[0]),
              .y = dot(a, b.data[1]),
              .z = dot(a, b.data[2]),
              .w = dot(a, b.data[3])};
}

constexpr vec2 transform(mat4 const &a, vec2 const &b) {
  vec4 prod = a * vec4{b.x, b.y, 0, 1};
  return vec2{.x = prod.x, .y = prod.y};
}

constexpr vec2 transform(vec2 const &a, mat4 const &b) {
  vec4 prod = b * vec4{a.x, a.y, 0, 1};
  return vec2{.x = prod.x, .y = prod.y};
}

constexpr vec2 transform(mat4 const &a, vec3 const &b) {
  vec4 prod = a * vec4{b.x, b.y, b.z, 1};
  return vec2{.x = prod.x, .y = prod.y};
}

constexpr vec2 transform(vec3 const &a, mat4 const &b) {
  vec4 prod = b * vec4{a.x, a.y, a.z, 1};
  return vec2{.x = prod.x, .y = prod.y};
}

constexpr bool operator==(mat4 const &a, mat4 const &b) {
  return a.data[0] == b.data[0] && a.data[1] == b.data[1] &&
         a.data[2] == b.data[2] && a.data[3] == b.data[3];
}

namespace transforms {

constexpr mat4 translate(vec3 t) {
  return {
      vec4{1, 0, 0, t.x},
      vec4{0, 1, 0, t.y},
      vec4{0, 0, 1, t.z},
      vec4{0, 0, 0, 1},
  };
}

constexpr mat4 scale(vec3 s) {
  return {
      vec4{s.x, 0, 0, 0},
      vec4{0, s.y, 0, 0},
      vec4{0, 0, s.z, 0},
      vec4{0, 0, 0, 1},
  };
}

inline mat4 rotate_x(f32 degree_radians) {
  return {
      vec4{1, 0, 0, 0},
      vec4{0, std::cos(degree_radians), -std::sin(degree_radians), 0},
      vec4{0, std::sin(degree_radians), std::cos(degree_radians), 0},
      vec4{0, 0, 0, 1},
  };
}

inline mat4 rotate_y(f32 degree_radians) {
  return {
      vec4{std::cos(degree_radians), 0, std::sin(degree_radians), 0},
      vec4{0, 1, 0, 0},
      vec4{-std::sin(degree_radians), 0, std::cos(degree_radians), 0},
      vec4{0, 0, 0, 1},
  };
}

inline mat4 rotate_z(f32 degree_radians) {
  return {
      vec4{std::cos(degree_radians), -std::sin(degree_radians), 0, 0},
      vec4{std::sin(degree_radians), std::cos(degree_radians), 0, 0},
      vec4{0, 0, 1, 0},
      vec4{0, 0, 0, 1},
  };
}

}  // namespace transforms

struct offset {
  u32 x = 0, y = 0;
};

constexpr offset operator+(offset a, offset b) {
  return offset{.x = a.x + b.x, .y = a.y + b.y};
}

constexpr bool operator==(offset a, offset b) {
  return a.x == b.x && a.y == b.y;
}

constexpr bool operator!=(offset a, offset b) { return !(a == b); }

struct offseti {
  i64 x = 0, y = 0;
};

constexpr offseti operator+(offseti a, offseti b) {
  return offseti{.x = a.x + b.x, .y = a.y + b.y};
}

constexpr offseti operator-(offseti a, offseti b) {
  return offseti{.x = a.x - b.x, .y = a.y - b.y};
}

constexpr bool operator==(offseti a, offseti b) {
  return a.x == b.x && a.y == b.y;
}

constexpr bool operator!=(offseti a, offseti b) { return !(a == b); }

struct extent {
  u32 width = 0, height = 0;

  constexpr bool is_visible() const { return width != 0 && height != 0; }

  constexpr extent constrain(extent other) const {
    return extent{.width = std::min(width, other.width),
                  .height = std::min(height, other.height)};
  }

  constexpr u64 area() const { return AS_U64(width) * height; }
};

constexpr extent operator+(extent a, extent b) {
  return extent{.width = a.width + b.width, .height = a.height + b.height};
}

constexpr bool operator==(extent a, extent b) {
  return a.width == b.width && a.height == b.height;
}

constexpr bool operator!=(extent a, extent b) { return !(a == b); }

struct range {
  u32 min = 0;
  u32 max = 0;

  constexpr bool contains(u32 value) const {
    return value >= min && value <= max;
  }
};

constexpr bool operator==(range a, range b) {
  return a.min == b.min && a.max == b.max;
}

constexpr bool operator!=(range a, range b) { return !(a == b); }

struct color {
  u8 r = 0, g = 0, b = 0, a = 255;

  static constexpr color from_rgb(u8 r, u8 g, u8 b) {
    return color{.r = r, .g = g, .b = b, .a = 0xff};
  }

  constexpr color with_red(u8 nr) const {
    return color{.r = nr, .g = g, .b = b, .a = a};
  }

  constexpr color with_green(u8 ng) const {
    return color{.r = r, .g = ng, .b = b, .a = a};
  }

  constexpr color with_blue(u8 nb) const {
    return color{.r = r, .g = g, .b = nb, .a = a};
  }

  constexpr color with_alpha(u8 na) const {
    return color{.r = r, .g = g, .b = b, .a = na};
  }

  constexpr bool is_transparent() const { return a == 0; }

  constexpr bool is_visible() const { return !is_transparent(); }

  constexpr vec4 as_vec() const {
    return vec4{
        .x = r / 255.0f, .y = g / 255.0f, .z = b / 255.0f, .w = a / 255.0f};
  }
};

constexpr bool operator==(color a, color b) {
  return a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a;
}

constexpr bool operator!=(color a, color b) { return !(a == b); }

namespace colors {

constexpr color TRANSPARENT{.r = 0x00, .g = 0x00, .b = 0x00, .a = 0x00};
constexpr color WHITE = color::from_rgb(0xff, 0xff, 0xff);
constexpr color BLACK = color::from_rgb(0x00, 0x00, 0x00);
constexpr color RED = color::from_rgb(0xff, 0x00, 0x00);
constexpr color BLUE = color::from_rgb(0x00, 0x00, 0xff);
constexpr color GREEN = color::from_rgb(0x00, 0xff, 0x00);
constexpr color CYAN = color::from_rgb(0x00, 0xff, 0xff);
constexpr color MAGENTA = color::from_rgb(0xff, 0x00, 0xff);

}  // namespace colors
}  // namespace asr
