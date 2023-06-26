#pragma once

#include <algorithm>
#include <chrono>
#include <cstdint>

#include "ashura/utils.h"
#include "fmt/format.h"
#include "stx/limits.h"
#include "stx/option.h"

#define ASH_TO_RADIANS(...) AS(f32, ::ash::PI *(__VA_ARGS__) / 180)

namespace ash
{

using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using i8  = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using f32 = float;
using f64 = double;

using usize = size_t;
using isize = ptrdiff_t;

using uchar = unsigned char;
using uint  = unsigned int;

using Clock        = std::chrono::steady_clock;        // monotonic system clock
using timepoint    = Clock::time_point;
using nanoseconds  = std::chrono::nanoseconds;
using milliseconds = std::chrono::milliseconds;
using seconds      = std::chrono::seconds;

constexpr f32 PI = 3.14159265358979323846f;

constexpr f32 abs(f32 x)
{
  return x >= 0 ? x : -x;
}

constexpr bool epsilon_equal(f32 a, f32 b)
{
  return abs(b - a) <= stx::F32_EPSILON;
}

constexpr f32 epsilon_clamp(f32 x)
{
  return abs(x) > stx::F32_EPSILON ? x : stx::F32_EPSILON;
}

// WARNING: the only non-floating-point integral type you should be using this for is i64.
template <typename T>
constexpr T lerp(T const &a, T const &b, f32 t)
{
  return AS(T, a + (b - a) * t);
}

struct vec2
{
  f32 x = 0, y = 0;

  static constexpr vec2 splat(f32 v)
  {
    return vec2{.x = v, .y = v};
  }
};

constexpr vec2 epsilon_clamp(vec2 a)
{
  return vec2{.x = epsilon_clamp(a.x), .y = epsilon_clamp(a.y)};
}

constexpr bool operator==(vec2 a, vec2 b)
{
  return a.x == b.x && a.y == b.y;
}

constexpr bool operator!=(vec2 a, vec2 b)
{
  return a.x != b.x || a.y != b.y;
}

constexpr vec2 operator+(vec2 a, vec2 b)
{
  return vec2{a.x + b.x, a.y + b.y};
}

constexpr vec2 operator+(vec2 a, f32 b)
{
  return vec2{a.x + b, a.y + b};
}

constexpr vec2 operator+(f32 a, vec2 b)
{
  return b + a;
}

constexpr vec2 operator-(vec2 a, vec2 b)
{
  return vec2{a.x - b.x, a.y - b.y};
}

constexpr vec2 operator-(vec2 a, f32 b)
{
  return vec2{a.x - b, a.y - b};
}

constexpr vec2 operator-(f32 a, vec2 b)
{
  return vec2{a - b.x, a - b.y};
}

constexpr vec2 operator*(vec2 a, vec2 b)
{
  return vec2{a.x * b.x, a.y * b.y};
}

constexpr vec2 operator*(vec2 a, f32 b)
{
  return vec2{a.x * b, a.y * b};
}

constexpr vec2 operator*(f32 a, vec2 b)
{
  return b * a;
}

constexpr vec2 operator/(vec2 a, vec2 b)
{
  return vec2{a.x / b.x, a.y / b.y};
}

constexpr vec2 operator/(vec2 a, f32 b)
{
  return vec2{a.x / b, a.y / b};
}

constexpr vec2 operator/(f32 a, vec2 b)
{
  return vec2{a / b.x, a / b.y};
}

constexpr vec2 &operator+=(vec2 &a, vec2 b)
{
  a = a + b;
  return a;
}

constexpr vec2 &operator+=(vec2 &a, f32 b)
{
  a = a + b;
  return a;
}

constexpr vec2 &operator-=(vec2 &a, vec2 b)
{
  a = a - b;
  return a;
}

constexpr vec2 &operator-=(vec2 &a, f32 b)
{
  a = a - b;
  return a;
}

constexpr vec2 &operator*=(vec2 &a, vec2 b)
{
  a = a * b;
  return a;
}

constexpr vec2 &operator*=(vec2 &a, f32 b)
{
  a = a * b;
  return a;
}

constexpr vec2 &operator/=(vec2 &a, vec2 b)
{
  a = a / b;
  return a;
}

constexpr vec2 &operator/=(vec2 &a, f32 b)
{
  a = a / b;
  return a;
}

constexpr f32 dot(vec2 a, vec2 b)
{
  return a.x * b.x + a.y * b.y;
}

constexpr f32 cross(vec2 a, vec2 b)
{
  return a.x * b.y - b.x * a.y;
}

struct tri
{
  vec2 p0, p1, p2;

  constexpr f32 sign() const
  {
    return (p0.x - p2.x) * (p1.y - p2.y) - (p1.x - p2.x) * (p0.y - p2.y);
  }

  constexpr bool contains(vec2 point) const
  {
    f32 sign0 = tri{point, p0, p1}.sign();
    f32 sign1 = tri{point, p1, p2}.sign();
    f32 sign2 = tri{point, p2, p0}.sign();

    bool has_neg = (sign0 < 0) || (sign1 < 0) || (sign2 < 0);
    bool has_pos = (sign0 > 0) || (sign1 > 0) || (sign2 > 0);

    return !(has_neg && has_pos);
  }
};

// each coordinate is an edge of the quad
struct quad
{
  vec2 p0, p1, p2, p3;

  constexpr bool contains(vec2 point) const
  {
    return tri{.p0 = p0, .p1 = p1, .p2 = p2}.contains(point) || tri{.p0 = p0, .p1 = p2, .p2 = p3}.contains(point);
  }
};

struct rect
{
  vec2 offset, extent;

  constexpr auto bounds() const
  {
    return std::make_tuple(offset.x, offset.x + extent.x, offset.y, offset.y + extent.y);
  }

  constexpr bool overlaps(rect other) const
  {
    auto [x0_min, x0_max, y0_min, y0_max] = bounds();
    auto [x1_min, x1_max, y1_min, y1_max] = other.bounds();

    return x0_min < x1_max && x0_max > x1_min && y1_max > y0_min && y1_min < y0_max;
  }

  constexpr bool contains(vec2 point) const
  {
    return offset.x <= point.x && offset.y <= point.y && (offset.x + extent.x) >= point.x && (offset.y + extent.y) >= point.y;
  }

  constexpr bool contains(quad const &quad) const
  {
    return contains(quad.p0) || contains(quad.p1) || contains(quad.p2) || contains(quad.p3);
  }

  constexpr bool is_visible() const
  {
    return extent.x != 0 && extent.y != 0;
  }

  constexpr vec2 top_left() const
  {
    return offset;
  }

  constexpr vec2 top_right() const
  {
    return offset + vec2{extent.x, 0};
  }

  constexpr vec2 bottom_left() const
  {
    return offset + vec2{0, extent.y};
  }

  constexpr vec2 bottom_right() const
  {
    return offset + extent;
  }

  constexpr quad to_quad() const
  {
    return quad{
        .p0 = top_left(),
        .p1 = top_right(),
        .p2 = bottom_right(),
        .p3 = bottom_left()};
  }

  constexpr rect with_offset(vec2 new_offset) const
  {
    return rect{.offset = new_offset, .extent = extent};
  }

  constexpr rect with_offset(f32 x, f32 y) const
  {
    return rect{.offset = vec2{x, y}, .extent = extent};
  }

  constexpr rect with_extent(vec2 new_extent) const
  {
    return rect{.offset = offset, .extent = new_extent};
  }

  constexpr rect with_extent(f32 w, f32 h) const
  {
    return rect{.offset = offset, .extent = vec2{w, h}};
  }

  constexpr rect with_center(vec2 center) const
  {
    return rect{.offset = center - extent / 2, .extent = extent};
  }

  constexpr rect with_center(f32 cx, f32 cy) const
  {
    return with_center(vec2{cx, cy});
  }
};

struct vec3
{
  f32 x = 0, y = 0, z = 0, __padding__;

  static constexpr vec3 splat(f32 v)
  {
    return vec3{.x = v, .y = v, .z = v};
  }
};

constexpr bool operator==(vec3 a, vec3 b)
{
  return a.x == b.x && a.y == b.y && a.z == b.z;
}

constexpr bool operator!=(vec3 a, vec3 b)
{
  return a.x != b.x || a.y != b.y || a.z != b.z;
}

constexpr vec3 operator+(vec3 a, vec3 b)
{
  return vec3{a.x + b.x, a.y + b.y, a.z + b.z};
}

constexpr vec3 operator+(vec3 a, f32 b)
{
  return vec3{a.x + b, a.y + b, a.z + b};
}

constexpr vec3 operator+(f32 a, vec3 b)
{
  return vec3{a + b.x, a + b.y, a + b.z};
}

constexpr vec3 operator-(vec3 a, vec3 b)
{
  return vec3{a.x - b.x, a.y - b.y, a.z - b.z};
}

constexpr vec3 operator-(vec3 a, f32 b)
{
  return vec3{a.x - b, a.y - b, a.z - b};
}

constexpr vec3 operator-(f32 a, vec3 b)
{
  return vec3{a - b.x, a - b.y, a - b.z};
}

constexpr vec3 operator*(vec3 a, vec3 b)
{
  return vec3{a.x * b.x, a.y * b.y, a.z * b.z};
}

constexpr vec3 operator*(vec3 a, f32 b)
{
  return vec3{a.x * b, a.y * b, a.z * b};
}

constexpr vec3 operator*(f32 a, vec3 b)
{
  return vec3{a * b.x, a * b.y, a * b.z};
}

constexpr vec3 operator/(vec3 a, vec3 b)
{
  return vec3{a.x / b.x, a.y / b.y, a.z / b.z};
}

constexpr vec3 operator/(vec3 a, f32 b)
{
  return vec3{a.x / b, a.y / b, a.z / b};
}

constexpr vec3 operator/(f32 a, vec3 b)
{
  return vec3{a / b.x, a / b.y, a / b.z};
}

constexpr f32 dot(vec3 a, vec3 b)
{
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

/// column vector
struct vec4
{
  f32 x = 0, y = 0, z = 0, w = 0;

  static constexpr vec4 splat(f32 v)
  {
    return vec4{.x = v, .y = v, .z = v, .w = v};
  }
};

constexpr bool operator==(vec4 a, vec4 b)
{
  return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
}

constexpr bool operator!=(vec4 a, vec4 b)
{
  return a.x != b.x || a.y != b.y || a.z != b.z || a.w != b.w;
}

constexpr vec4 operator+(vec4 a, vec4 b)
{
  return vec4{a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w};
}

constexpr vec4 operator+(vec4 a, f32 b)
{
  return vec4{a.x + b, a.y + b, a.z + b, a.w + b};
}

constexpr vec4 operator+(f32 a, vec4 b)
{
  return vec4{a + b.x, a + b.y, a + b.z, a + b.w};
}

constexpr vec4 operator-(vec4 a, vec4 b)
{
  return vec4{a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w};
}

constexpr vec4 operator-(vec4 a, f32 b)
{
  return vec4{a.x - b, a.y - b, a.z - b, a.w - b};
}

constexpr vec4 operator-(f32 a, vec4 b)
{
  return vec4{a - b.x, a - b.y, a - b.z, a - b.w};
}

constexpr vec4 operator*(vec4 a, vec4 b)
{
  return vec4{a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w};
}

constexpr vec4 operator*(vec4 a, f32 b)
{
  return vec4{a.x * b, a.y * b, a.z * b, a.w * b};
}

constexpr vec4 operator*(f32 a, vec4 b)
{
  return vec4{a * b.x, a * b.y, a * b.z, a * b.w};
}

constexpr vec4 operator/(vec4 a, vec4 b)
{
  return vec4{a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w};
}

constexpr vec4 operator/(vec4 a, f32 b)
{
  return vec4{a.x / b, a.y / b, a.z / b, a.w / b};
}

constexpr vec4 operator/(f32 a, vec4 b)
{
  return vec4{a / b.x, a / b.y, a / b.z, a / b.w};
}

constexpr f32 dot(vec4 a, vec4 b)
{
  return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

/// row-major
struct mat3
{
  vec3 rows[3];

  static constexpr mat3 identity()
  {
    return mat3{vec3{.x = 1, .y = 0, .z = 0},
                vec3{.x = 0, .y = 1, .z = 0},
                vec3{.x = 0, .y = 0, .z = 1}};
  }
};

constexpr vec3 operator*(mat3 const &a, vec3 const &b)
{
  return vec3{.x = dot(a.rows[0], b), .y = dot(a.rows[1], b), .z = dot(a.rows[2], b)};
}

/// row-major
struct mat4
{
  vec4 rows[4];

  static constexpr mat4 identity()
  {
    return mat4{vec4{.x = 1, .y = 0, .z = 0, .w = 0}, vec4{.x = 0, .y = 1, .z = 0, .w = 0},
                vec4{.x = 0, .y = 0, .z = 1, .w = 0}, vec4{.x = 0, .y = 0, .z = 0, .w = 1}};
  }

  constexpr mat4 transpose() const
  {
    return mat4{vec4{rows[0].x, rows[1].x, rows[2].x, rows[3].x},
                vec4{rows[0].y, rows[1].y, rows[2].y, rows[3].y},
                vec4{rows[0].z, rows[1].z, rows[2].z, rows[3].z},
                vec4{rows[0].w, rows[1].w, rows[2].w, rows[3].w}};
  }
};

constexpr bool operator==(mat4 const &a, mat4 const &b)
{
  return a.rows[0] == b.rows[0] && a.rows[1] == b.rows[1] && a.rows[2] == b.rows[2] && a.rows[3] == b.rows[3];
}

constexpr bool operator!=(mat4 const &a, mat4 const &b)
{
  return a.rows[0] != b.rows[0] || a.rows[1] != b.rows[1] || a.rows[2] != b.rows[2] || a.rows[3] != b.rows[3];
}

constexpr mat4 operator*(mat4 const &a, mat4 const &b)
{
  return mat4{.rows = {{dot(a.rows[0], {b.rows[0].x, b.rows[1].x, b.rows[2].x, b.rows[3].x}),
                        dot(a.rows[0], {b.rows[0].y, b.rows[1].y, b.rows[2].y, b.rows[3].y}),
                        dot(a.rows[0], {b.rows[0].z, b.rows[1].z, b.rows[2].z, b.rows[3].z}),
                        dot(a.rows[0], {b.rows[0].w, b.rows[1].w, b.rows[2].w, b.rows[3].w})},
                       {dot(a.rows[1], {b.rows[0].x, b.rows[1].x, b.rows[2].x, b.rows[3].x}),
                        dot(a.rows[1], {b.rows[0].y, b.rows[1].y, b.rows[2].y, b.rows[3].y}),
                        dot(a.rows[1], {b.rows[0].z, b.rows[1].z, b.rows[2].z, b.rows[3].z}),
                        dot(a.rows[1], {b.rows[0].w, b.rows[1].w, b.rows[2].w, b.rows[3].w})},
                       {dot(a.rows[2], {b.rows[0].x, b.rows[1].x, b.rows[2].x, b.rows[3].x}),
                        dot(a.rows[2], {b.rows[0].y, b.rows[1].y, b.rows[2].y, b.rows[3].y}),
                        dot(a.rows[2], {b.rows[0].z, b.rows[1].z, b.rows[2].z, b.rows[3].z}),
                        dot(a.rows[2], {b.rows[0].w, b.rows[1].w, b.rows[2].w, b.rows[3].w})},
                       {dot(a.rows[3], {b.rows[0].x, b.rows[1].x, b.rows[2].x, b.rows[3].x}),
                        dot(a.rows[3], {b.rows[0].y, b.rows[1].y, b.rows[2].y, b.rows[3].y}),
                        dot(a.rows[3], {b.rows[0].z, b.rows[1].z, b.rows[2].z, b.rows[3].z}),
                        dot(a.rows[3], {b.rows[0].w, b.rows[1].w, b.rows[2].w, b.rows[3].w})}}};
}

constexpr vec4 operator*(mat4 const &a, vec4 const &b)
{
  return vec4{.x = dot(a.rows[0], b), .y = dot(a.rows[1], b), .z = dot(a.rows[2], b), .w = dot(a.rows[3], b)};
}

constexpr vec4 operator*(vec4 const &a, mat4 const &b)
{
  return vec4{.x = dot(a, b.rows[0]), .y = dot(a, b.rows[1]), .z = dot(a, b.rows[2]), .w = dot(a, b.rows[3])};
}

constexpr vec2 transform(mat4 const &a, vec2 const &b)
{
  vec4 prod = a *vec4{b.x, b.y, 0, 1};
  return vec2{.x = prod.x, .y = prod.y};
}

constexpr vec2 transform(mat4 const &a, vec3 const &b)
{
  vec4 prod = a *vec4{b.x, b.y, b.z, 1};
  return vec2{.x = prod.x, .y = prod.y};
}

constexpr quad transform(mat4 const &a, rect const &b)
{
  return quad{.p0 = transform(a, b.top_left()),
              .p1 = transform(a, b.top_right()),
              .p2 = transform(a, b.bottom_right()),
              .p3 = transform(a, b.bottom_left())};
}

constexpr mat4 translate(vec3 t)
{
  return mat4{
      vec4{1, 0, 0, t.x},
      vec4{0, 1, 0, t.y},
      vec4{0, 0, 1, t.z},
      vec4{0, 0, 0, 1},
  };
}

constexpr mat4 scale(vec3 s)
{
  return mat4{
      vec4{s.x, 0, 0, 0},
      vec4{0, s.y, 0, 0},
      vec4{0, 0, s.z, 0},
      vec4{0, 0, 0, 1},
  };
}

inline mat4 rotate_x(f32 degree_radians)
{
  return mat4{
      vec4{1, 0, 0, 0},
      vec4{0, std::cos(degree_radians), -std::sin(degree_radians), 0},
      vec4{0, std::sin(degree_radians), std::cos(degree_radians), 0},
      vec4{0, 0, 0, 1},
  };
}

inline mat4 rotate_y(f32 degree_radians)
{
  return mat4{
      vec4{std::cos(degree_radians), 0, std::sin(degree_radians), 0},
      vec4{0, 1, 0, 0},
      vec4{-std::sin(degree_radians), 0, std::cos(degree_radians), 0},
      vec4{0, 0, 0, 1},
  };
}

inline mat4 rotate_z(f32 degree_radians)
{
  return mat4{
      vec4{std::cos(degree_radians), -std::sin(degree_radians), 0, 0},
      vec4{std::sin(degree_radians), std::cos(degree_radians), 0, 0},
      vec4{0, 0, 1, 0},
      vec4{0, 0, 0, 1},
  };
}

inline mat4 shear_x(f32 y_shear, f32 z_shear)
{
  return mat4{
      vec4{1, y_shear, z_shear, 0},
      vec4{0, 1, 0, 0},
      vec4{0, 0, 1, 0},
      vec4{0, 0, 0, 1},
  };
}

inline mat4 shear_y(f32 x_shear, f32 z_shear)
{
  return mat4{
      vec4{1, 0, 0, 0},
      vec4{x_shear, 1, z_shear, 0},
      vec4{0, 0, 1, 0},
      vec4{0, 0, 0, 1},
  };
}

inline mat4 shear_z(f32 x_shear, f32 y_shear)
{
  return mat4{
      vec4{1, 0, 0, 0},
      vec4{0, 1, 0, 0},
      vec4{x_shear, y_shear, 1, 0},
      vec4{0, 0, 0, 1},
  };
}

struct quaternion
{
  f32 x = 0, y = 0, z = 0, w = 0;

  // static constexpr quaternion from_euler(vec3 x) { return {}; }

  // constexpr vec3 to_euler() const { return {}; }

  // constexpr quaternion normalized() const { return {}; }
};

constexpr bool operator==(quaternion a, quaternion b)
{
  return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
}

constexpr bool operator!=(quaternion a, quaternion b)
{
  return a.x != b.x || a.y != b.y || a.z != b.z || a.w != b.w;
}

// constexpr quaternion operator+(quaternion a, quaternion b) { return {}; }

// constexpr quaternion operator-(quaternion a, quaternion b) { return {}; }

// constexpr quaternion operator*(quaternion a, f32 b) { return {}; }

// constexpr quaternion operator*(f32 a, quaternion b) { return {}; }

// constexpr quaternion operator*(quaternion a, quaternion b) { return {}; }

// constexpr quaternion dot(quaternion a, quaternion b) { return {}; }

struct offset
{
  u32 x = 0, y = 0;

  constexpr vec2 to_vec() const
  {
    return vec2{.x = AS(f32, x), .y = AS(f32, y)};
  }
};

constexpr bool operator==(offset a, offset b)
{
  return a.x == b.x && a.y == b.y;
}

constexpr bool operator!=(offset a, offset b)
{
  return !(a == b);
}

constexpr offset operator+(offset a, offset b)
{
  return offset{.x = a.x + b.x, .y = a.y + b.y};
}

struct offseti
{
  i32 x = 0, y = 0;

  constexpr vec2 as_vec() const
  {
    return vec2{AS(f32, x), AS(f32, y)};
  }
};

constexpr bool operator==(offseti a, offseti b)
{
  return a.x == b.x && a.y == b.y;
}

constexpr bool operator!=(offseti a, offseti b)
{
  return !(a == b);
}

constexpr offseti operator+(offseti a, offseti b)
{
  return offseti{.x = a.x + b.x, .y = a.y + b.y};
}

constexpr offseti operator-(offseti a, offseti b)
{
  return offseti{.x = a.x - b.x, .y = a.y - b.y};
}

struct extent
{
  u32 width = 0, height = 0;

  constexpr bool is_visible() const
  {
    return width != 0 && height != 0;
  }

  constexpr extent constrain(extent other) const
  {
    return extent{.width = std::min(width, other.width), .height = std::min(height, other.height)};
  }

  constexpr u64 area() const
  {
    return AS(u64, width) * height;
  }

  constexpr vec2 to_vec() const
  {
    return vec2{.x = AS(f32, width), .y = AS(f32, height)};
  }
};

constexpr bool operator==(extent a, extent b)
{
  return a.width == b.width && a.height == b.height;
}

constexpr bool operator!=(extent a, extent b)
{
  return !(a == b);
}

constexpr extent operator+(extent a, extent b)
{
  return extent{.width = a.width + b.width, .height = a.height + b.height};
}

// TODO(lamarrr): remove this???
/// advantages of this constraint model? sizing can be
/// - relative (`scale` = relative size)
/// - absolute (`scale` = 0, `bias` = absolute size) or both
///
/// - absolute min/max (`min`, `max`)
struct constraint
{
  f32 bias  = 0;                   /// removing or deducting from the target size
  f32 scale = 0;                   /// scaling the target size
  f32 min   = stx::F32_MIN;        /// clamps the target size, i.e. value should be at least 20px
  f32 max   = stx::F32_MAX;        /// clamps the target size, i.e. value should be at most 100px

  static constexpr constraint relative(f32 scale)
  {
    return constraint{.bias = 0, .scale = scale, .min = stx::F32_MIN, .max = stx::F32_MAX};
  }

  static constexpr constraint absolute(f32 value)
  {
    return constraint{.bias = value, .scale = 0, .min = stx::F32_MIN, .max = stx::F32_MAX};
  }

  constexpr constraint with_min(f32 v) const
  {
    return constraint{.bias = bias, .scale = scale, .min = v, .max = max};
  }

  constexpr constraint with_max(f32 v) const
  {
    return constraint{.bias = bias, .scale = scale, .min = min, .max = v};
  }

  constexpr f32 resolve(f32 value) const
  {
    return std::clamp(bias + value * scale, min, max);
  }
};

struct SizeConstraint
{
  constraint width, height;

  static constexpr SizeConstraint relative(f32 w, f32 h)
  {
    return SizeConstraint{.width = constraint::relative(w), .height = constraint::relative(h)};
  }

  static constexpr SizeConstraint absolute(f32 w, f32 h)
  {
    return SizeConstraint{.width = constraint::absolute(w), .height = constraint::absolute(h)};
  }

  constexpr SizeConstraint with_min(f32 w, f32 h) const
  {
    return SizeConstraint{.width = width.with_min(w), .height = height.with_min(h)};
  }

  constexpr SizeConstraint with_max(f32 w, f32 h) const
  {
    return SizeConstraint{.width = width.with_max(w), .height = height.with_max(h)};
  }

  constexpr vec2 resolve(f32 w, f32 h) const
  {
    return vec2{width.resolve(w), height.resolve(h)};
  }

  constexpr vec2 resolve(vec2 wh) const
  {
    return resolve(wh.x, wh.y);
  }
};

struct color
{
  u8 r = 0, g = 0, b = 0, a = 0;

  static constexpr color from_rgb(u8 r, u8 g, u8 b)
  {
    return color{.r = r, .g = g, .b = b, .a = 0xff};
  }

  static constexpr color from_rgba(u8 r, u8 g, u8 b, u8 a)
  {
    return color{.r = r, .g = g, .b = b, .a = a};
  }

  constexpr color with_red(u8 nr) const
  {
    return color{.r = nr, .g = g, .b = b, .a = a};
  }

  constexpr color with_green(u8 ng) const
  {
    return color{.r = r, .g = ng, .b = b, .a = a};
  }

  constexpr color with_blue(u8 nb) const
  {
    return color{.r = r, .g = g, .b = nb, .a = a};
  }

  constexpr color with_alpha(u8 na) const
  {
    return color{.r = r, .g = g, .b = b, .a = na};
  }

  constexpr bool is_transparent() const
  {
    return a == 0;
  }

  constexpr bool is_visible() const
  {
    return !is_transparent();
  }

  constexpr vec4 to_vec() const
  {
    return vec4{.x = r / 255.0f, .y = g / 255.0f, .z = b / 255.0f, .w = a / 255.0f};
  }
};

constexpr bool operator==(color a, color b)
{
  return a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a;
}

constexpr bool operator!=(color a, color b)
{
  return a.r != b.r || a.g != b.g || a.b != b.b || a.a != b.a;
}

template <>
constexpr color lerp<color>(color const &a, color const &b, f32 t)
{
  return color{.r = AS(u8, std::clamp<i64>(lerp<i64>(a.r, b.r, t), 0, 255)),
               .g = AS(u8, std::clamp<i64>(lerp<i64>(a.g, b.g, t), 0, 255)),
               .b = AS(u8, std::clamp<i64>(lerp<i64>(a.b, b.b, t), 0, 255)),
               .a = AS(u8, std::clamp<i64>(lerp<i64>(a.a, b.a, t), 0, 255))};
}

namespace colors
{

constexpr color TRANSPARENT = color::from_rgba(0x00, 0x00, 0x00, 0x00);
constexpr color WHITE       = color::from_rgb(0xff, 0xff, 0xff);
constexpr color BLACK       = color::from_rgb(0x00, 0x00, 0x00);
constexpr color RED         = color::from_rgb(0xff, 0x00, 0x00);
constexpr color BLUE        = color::from_rgb(0x00, 0x00, 0xff);
constexpr color GREEN       = color::from_rgb(0x00, 0xff, 0x00);
constexpr color CYAN        = color::from_rgb(0x00, 0xff, 0xff);
constexpr color MAGENTA     = color::from_rgb(0xff, 0x00, 0xff);
constexpr color YELLOW      = color::from_rgb(0xff, 0xff, 0x00);

}        // namespace colors

struct texture_rect
{
  vec2 uv0, uv1;
};

/// a 2d shader vertex
struct vertex
{
  vec2 position;        // point in 2d space
  vec2 uv;              // texture coordinates
  vec4 color;           // color of the vertex encoded in the target's color space
};

struct EdgeInsets
{
  f32 left = 0, top = 0, right = 0, bottom = 0;

  static constexpr EdgeInsets all(f32 v)
  {
    return EdgeInsets{.left = v, .top = v, .right = v, .bottom = v};
  }
};

constexpr bool operator==(EdgeInsets const &a, EdgeInsets const &b)
{
  return a.left == b.left && a.top == b.top && a.right == b.right && a.bottom == b.bottom;
}

constexpr bool operator!=(EdgeInsets const &a, EdgeInsets const &b)
{
  return a.left != b.left || a.top != b.top || a.right != b.right || a.bottom != b.bottom;
}

}        // namespace ash
