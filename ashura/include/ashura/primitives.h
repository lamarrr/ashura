#pragma once

#include <algorithm>
#include <chrono>
#include <cstdint>

#include "ashura/utils.h"
#include "fmt/format.h"
#include "stx/limits.h"
#include "stx/option.h"

#define ASH_TO_RADIANS(...) AS(f32, ::ash::PI *(__VA_ARGS__) / 180.0f)

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

  constexpr bool overlaps(quad const &quad) const
  {
    return contains(quad.p0) || contains(quad.p1) || contains(quad.p2) || contains(quad.p3);
  }

  /// @brief NOTE: returns 0-extent rect if there's no intersection
  /// @param other
  /// @return
  constexpr rect intersect(rect other) const
  {
    auto const [x1_min, x1_max, y1_min, y1_max] = bounds();
    auto const [x2_min, x2_max, y2_min, y2_max] = other.bounds();

    if (!overlaps(other))
    {
      return rect{.offset = offset, .extent = vec2{0, 0}};
    }

    vec2 offset{std::max(x1_min, x2_min), std::max(y1_min, y2_min)};
    vec2 extent{std::min(x1_max, x2_max) - offset.x, std::min(y1_max, y2_max) - offset.y};

    return rect{.offset = offset, .extent = extent};
  }

  constexpr bool contains(vec2 point) const
  {
    return offset.x <= point.x && offset.y <= point.y && (offset.x + extent.x) >= point.x && (offset.y + extent.y) >= point.y;
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

  constexpr rect centered() const
  {
    return with_center(offset);
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

struct mat2
{
  vec2 rows[2];

  static constexpr mat2 identity()
  {
    return mat2{.rows = {{.x = 1, .y = 0},
                         {.x = 0, .y = 1}}};
  }

  constexpr mat2 transpose() const
  {
    return mat2{.rows = {{rows[0].x, rows[1].x},
                         {rows[0].y, rows[1].y}}};
  }

  constexpr vec2 &operator[](usize i)
  {
    return rows[i];
  }

  constexpr vec2 const &operator[](usize i) const
  {
    return rows[i];
  }
};

constexpr mat2 operator*(mat2 a, f32 b)
{
  return mat2{.rows = {a[0] * b,
                       a[1] * b}};
}

constexpr mat2 operator*(f32 a, mat2 b)
{
  return mat2{.rows = {a * b[0],
                       a * b[1]}};
}

constexpr f32 determinant(mat2 a)
{
  return a[0].x * a[1].y - a[1].x * a[0].y;
}

constexpr mat2 adjoint(mat2 a)
{
  return mat2{.rows = {{a[1].y, -a[0].y},
                       {-a[1].x, a[0].x}}};
}

constexpr mat2 inverse(mat2 a)
{
  return 1 / determinant(a) * adjoint(a);
}

/// row-major
struct mat3
{
  vec3 rows[3];

  static constexpr mat3 identity()
  {
    return mat3{.rows = {{.x = 1, .y = 0, .z = 0},
                         {.x = 0, .y = 1, .z = 0},
                         {.x = 0, .y = 0, .z = 1}}};
  }

  constexpr mat3 transpose() const
  {
    return mat3{.rows = {{rows[0].x, rows[1].x, rows[2].x},
                         {rows[0].y, rows[1].y, rows[2].y},
                         {rows[0].z, rows[1].z, rows[2].z}}};
  }

  constexpr vec3 &operator[](usize i)
  {
    return rows[i];
  }

  constexpr vec3 const &operator[](usize i) const
  {
    return rows[i];
  }
};

constexpr mat3 operator*(mat3 a, f32 b)
{
  return mat3{.rows = {a[0] * b,
                       a[1] * b,
                       a[2] * b}};
}

constexpr mat3 operator*(f32 a, mat3 b)
{
  return mat3{.rows = {a * b[0],
                       a * b[1],
                       a * b[2]}};
}

constexpr vec3 operator*(mat3 const &a, vec3 const &b)
{
  return vec3{.x = dot(a[0], b), .y = dot(a[1], b), .z = dot(a[2], b)};
}

constexpr mat3 operator*(mat3 const &a, mat3 const &b)
{
  return mat3{.rows = {{dot(a[0], {b[0].x, b[1].x, b[2].x}),
                        dot(a[0], {b[0].y, b[1].y, b[2].y}),
                        dot(a[0], {b[0].z, b[1].z, b[2].z})},
                       {dot(a[1], {b[0].x, b[1].x, b[2].x}),
                        dot(a[1], {b[0].y, b[1].y, b[2].y}),
                        dot(a[1], {b[0].z, b[1].z, b[2].z})},
                       {dot(a[2], {b[0].x, b[1].x, b[2].x}),
                        dot(a[2], {b[0].y, b[1].y, b[2].y}),
                        dot(a[2], {b[0].z, b[1].z, b[2].z})}}};
}

constexpr f32 determinant(mat3 const &a)
{
  return a[0].x * a[1].y * a[2].z -
         a[0].x * a[1].z * a[2].y -
         a[0].y * a[1].x * a[2].z +
         a[0].y * a[1].z * a[2].x +
         a[0].z * a[1].x * a[2].y -
         a[0].z * a[1].y * a[2].x;
}

constexpr mat3 adjoint(mat3 const &a)
{
  return mat3{.rows = {{a[1].y * a[2].z - a[1].z * a[2].y,
                        a[0].z * a[2].y - a[0].y * a[2].z,
                        a[0].y * a[1].z - a[0].z * a[1].y},
                       {a[1].z * a[2].x - a[1].x * a[2].z,
                        a[0].x * a[2].z - a[0].z * a[2].x,
                        a[0].z * a[1].x - a[0].x * a[1].z},
                       {a[1].x * a[2].y - a[1].y * a[2].x,
                        a[0].y * a[2].x - a[0].x * a[2].y,
                        a[0].x * a[1].y - a[0].y * a[1].x}}};
}

constexpr mat3 inverse(mat3 const &a)
{
  return 1 / determinant(a) * adjoint(a);
}

/// row-major
struct mat4
{
  vec4 rows[4];

  static constexpr mat4 identity()
  {
    return mat4{.rows = {{.x = 1, .y = 0, .z = 0, .w = 0},
                         {.x = 0, .y = 1, .z = 0, .w = 0},
                         {.x = 0, .y = 0, .z = 1, .w = 0},
                         {.x = 0, .y = 0, .z = 0, .w = 1}}};
  }

  constexpr mat4 transpose() const
  {
    return mat4{.rows = {{rows[0].x, rows[1].x, rows[2].x, rows[3].x},
                         {rows[0].y, rows[1].y, rows[2].y, rows[3].y},
                         {rows[0].z, rows[1].z, rows[2].z, rows[3].z},
                         {rows[0].w, rows[1].w, rows[2].w, rows[3].w}}};
  }

  constexpr vec4 &operator[](usize i)
  {
    return rows[i];
  }

  constexpr vec4 const &operator[](usize i) const
  {
    return rows[i];
  }
};

constexpr mat4 operator*(mat4 a, f32 b)
{
  return mat4{.rows = {a[0] * b,
                       a[1] * b,
                       a[2] * b,
                       a[3] * b}};
}

constexpr mat4 operator*(f32 a, mat4 b)
{
  return mat4{.rows = {a * b[0],
                       a * b[1],
                       a * b[2],
                       a * b[3]}};
}

constexpr bool operator==(mat4 const &a, mat4 const &b)
{
  return a[0] == b[0] && a[1] == b[1] && a[2] == b[2] && a[3] == b[3];
}

constexpr bool operator!=(mat4 const &a, mat4 const &b)
{
  return a[0] != b[0] || a[1] != b[1] || a[2] != b[2] || a[3] != b[3];
}

constexpr mat4 operator*(mat4 const &a, mat4 const &b)
{
  return mat4{.rows = {{dot(a[0], {b[0].x, b[1].x, b[2].x, b[3].x}),
                        dot(a[0], {b[0].y, b[1].y, b[2].y, b[3].y}),
                        dot(a[0], {b[0].z, b[1].z, b[2].z, b[3].z}),
                        dot(a[0], {b[0].w, b[1].w, b[2].w, b[3].w})},
                       {dot(a[1], {b[0].x, b[1].x, b[2].x, b[3].x}),
                        dot(a[1], {b[0].y, b[1].y, b[2].y, b[3].y}),
                        dot(a[1], {b[0].z, b[1].z, b[2].z, b[3].z}),
                        dot(a[1], {b[0].w, b[1].w, b[2].w, b[3].w})},
                       {dot(a[2], {b[0].x, b[1].x, b[2].x, b[3].x}),
                        dot(a[2], {b[0].y, b[1].y, b[2].y, b[3].y}),
                        dot(a[2], {b[0].z, b[1].z, b[2].z, b[3].z}),
                        dot(a[2], {b[0].w, b[1].w, b[2].w, b[3].w})},
                       {dot(a[3], {b[0].x, b[1].x, b[2].x, b[3].x}),
                        dot(a[3], {b[0].y, b[1].y, b[2].y, b[3].y}),
                        dot(a[3], {b[0].z, b[1].z, b[2].z, b[3].z}),
                        dot(a[3], {b[0].w, b[1].w, b[2].w, b[3].w})}}};
}

constexpr vec4 operator*(mat4 const &a, vec4 const &b)
{
  return vec4{.x = dot(a[0], b), .y = dot(a[1], b), .z = dot(a[2], b), .w = dot(a[3], b)};
}

constexpr f32 determinant(mat4 const &a)
{
  return a[0].x * (a[1].y * a[2].z * a[3].w +
                   a[1].z * a[2].w * a[3].y +
                   a[1].w * a[2].y * a[3].z -
                   a[1].w * a[2].z * a[3].y -
                   a[1].z * a[2].y * a[3].w -
                   a[1].y * a[2].w * a[3].z) -
         a[1].x * (a[0].y * a[2].z * a[3].w +
                   a[0].z * a[2].w * a[3].y +
                   a[0].w * a[2].y * a[3].z -
                   a[0].w * a[2].z * a[3].y -
                   a[0].z * a[2].y * a[3].w -
                   a[0].y * a[2].w * a[3].z) +
         a[2].x * (a[0].y * a[1].z * a[3].w +
                   a[0].z * a[1].w * a[3].y +
                   a[0].w * a[1].y * a[3].z -
                   a[0].w * a[1].z * a[3].y -
                   a[0].z * a[1].y * a[3].w -
                   a[0].y * a[1].w * a[3].z) -
         a[3].x * (a[0].y * a[1].z * a[2].w +
                   a[0].z * a[1].w * a[2].y +
                   a[0].w * a[1].y * a[2].z -
                   a[0].w * a[1].z * a[2].y -
                   a[0].z * a[1].y * a[2].w -
                   a[0].y * a[1].w * a[2].z);
}

// constexpr mat4 adjoint(mat4 const &a)
// {
//   // TODO(lamarrr): complete from https://semath.info/src/inverse-cofactor-ex4.html
//   return mat4{.rows = {{a[1].y * a[2].z * a[3].w +
//                             a[1].z * a[2].w * a[3].y +
//                             a[1].w * a[2].y * a[3].z -
//                             a[1].w * a[2].z * a[3].y -
//                             a[1].w * a[2].y * a[3].w -
//                             a[1].y * a[2].w * a[3].z,
//                         -a[0].y * a[2].z * a[3].w -
//                             a[0].z * a[2].w * a[3].y -
//                             a[0].w * a[2].y * a[3].z +
//                             a[0].w * a[2].z * a[3].y +
//                             a[0].z * a[2].y * a[3].w +
//                             a[0].y * a[2].w * a[3].z,
//                         a[0].y * a[1].z * a[3].w +
//                             a[0].z * a[1].w * a[3].y +
//                             a[0].w * a[1].y * a[3].z -
//                             a[0].w * a[1].z * a[3].y -
//                             a[0].z * a[1].y * a[3].w -
//                             a[0].y * a[1].w * a[3].z,
//                         -a[0].y * a[1].z * a[2].w -
//                             a[0].z * a[1].w * a[2].y -
//                             a[0].w * a[1].y * a[2].z +
//                             a[0].w * a[1].z * a[2].y +
//                             a[0].z * a[1].y * a[2].w +
//                             a[0].y * a[1].w * a[2].z},
//                        {},
//                        {},
//                        {}}};
//}

// constexpr mat4 inverse(mat4 const &a)
// {
//   return 1 / determinant(a) * adjoint(a);
// }

constexpr vec2 transform3d(mat4 const &a, vec2 const &b)
{
  vec4 prod = a *vec4{b.x, b.y, 0, 1};
  return vec2{.x = prod.x, .y = prod.y};
}

constexpr vec2 transform3d(mat4 const &a, vec3 const &b)
{
  vec4 prod = a *vec4{b.x, b.y, b.z, 1};
  return vec2{.x = prod.x, .y = prod.y};
}

constexpr vec2 transform2d(mat3 const &a, vec2 const &b)
{
  vec3 prod = a *vec3{b.x, b.y, 0};
  return vec2{prod.x, prod.y};
}

constexpr quad transform2d(mat3 const &a, rect const &b)
{
  return quad{
      .p0 = transform2d(a, b.top_left()),
      .p1 = transform2d(a, b.top_right()),
      .p2 = transform2d(a, b.bottom_right()),
      .p3 = transform2d(a, b.bottom_left())};
}

constexpr mat3 translate2d(vec2 t)
{
  return mat3{.rows = {{1, 0, t.x},
                       {0, 1, t.y},
                       {0, 0, 1}}};
}

constexpr mat3 translate2d(f32 tx, f32 ty)
{
  return translate2d(vec2{tx, ty});
}

constexpr mat4 translate3d(vec3 t)
{
  return mat4{.rows = {{1, 0, 0, t.x},
                       {0, 1, 0, t.y},
                       {0, 0, 1, t.z},
                       {0, 0, 0, 1}}};
}

constexpr mat3 scale2d(vec2 s)
{
  return mat3{.rows = {{s.x, 0, 0},
                       {0, s.y, 0},
                       {0, 0, 1}}};
}

constexpr mat3 scale2d(f32 sx, f32 sy)
{
  return scale2d(vec2{sx, sy});
}

constexpr mat4 scale3d(vec3 s)
{
  return mat4{.rows = {{s.x, 0, 0, 0},
                       {0, s.y, 0, 0},
                       {0, 0, s.z, 0},
                       {0, 0, 0, 1}}};
}

inline mat3 rotate2d(f32 degree_radians)
{
  return mat3{.rows = {{1, 0, 0},
                       {0, std::cos(degree_radians), -std::sin(degree_radians)},
                       {0, std::sin(degree_radians), std::cos(degree_radians)}}};
}

inline mat4 rotate3d_x(f32 degree_radians)
{
  return mat4{.rows = {{1, 0, 0, 0},
                       {0, std::cos(degree_radians), -std::sin(degree_radians), 0},
                       {0, std::sin(degree_radians), std::cos(degree_radians), 0},
                       {0, 0, 0, 1}}};
}

inline mat4 rotate3d_y(f32 degree_radians)
{
  return mat4{.rows = {{std::cos(degree_radians), 0, std::sin(degree_radians), 0},
                       {0, 1, 0, 0},
                       {-std::sin(degree_radians), 0, std::cos(degree_radians), 0},
                       {0, 0, 0, 1}}};
}

inline mat4 rotate3d_z(f32 degree_radians)
{
  return mat4{.rows = {{std::cos(degree_radians), -std::sin(degree_radians), 0, 0},
                       {std::sin(degree_radians), std::cos(degree_radians), 0, 0},
                       {0, 0, 1, 0},
                       {0, 0, 0, 1}}};
}

constexpr mat3 shear2d_x(f32 x_shear)
{
  return mat3{.rows = {{1, 0, 0},
                       {x_shear, 1, 0},
                       {0, 0, 1}}};
}

constexpr mat3 shear2d_y(f32 y_shear)
{
  return mat3{.rows = {{1, y_shear, 0},
                       {0, 1, 0},
                       {0, 0, 1}}};
}

constexpr mat4 shear3d_x(f32 y_shear, f32 z_shear)
{
  return mat4{.rows = {{1, y_shear, z_shear, 0},
                       {0, 1, 0, 0},
                       {0, 0, 1, 0},
                       {0, 0, 0, 1}}};
}

constexpr mat4 shear3d_y(f32 x_shear, f32 z_shear)
{
  return mat4{.rows = {{1, 0, 0, 0},
                       {x_shear, 1, z_shear, 0},
                       {0, 0, 1, 0},
                       {0, 0, 0, 1}}};
}

constexpr mat4 shear3d_z(f32 x_shear, f32 y_shear)
{
  return mat4{.rows = {{1, 0, 0, 0},
                       {0, 1, 0, 0},
                       {x_shear, y_shear, 1, 0},
                       {0, 0, 0, 1}}};
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

struct ioffset
{
  i32 x = 0, y = 0;

  constexpr vec2 as_vec() const
  {
    return vec2{AS(f32, x), AS(f32, y)};
  }
};

constexpr bool operator==(ioffset a, ioffset b)
{
  return a.x == b.x && a.y == b.y;
}

constexpr bool operator!=(ioffset a, ioffset b)
{
  return !(a == b);
}

constexpr ioffset operator+(ioffset a, ioffset b)
{
  return ioffset{.x = a.x + b.x, .y = a.y + b.y};
}

constexpr ioffset operator-(ioffset a, ioffset b)
{
  return ioffset{.x = a.x - b.x, .y = a.y - b.y};
}

struct extent
{
  u32 width = 0, height = 0;

  static constexpr extent from(vec2 wh)
  {
    return extent{AS(u32, wh.x), AS(u32, wh.y)};
  }

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

struct irect
{
  ioffset offset;
  extent  extent;
};

/// Simple Layout Constraint Model
struct constraint
{
  f32 bias  = 0;                   /// adding or subtracting from the source size, i.e. value should be source size - 20px
  f32 scale = 0;                   /// scales the source size, i.e. value should be 0.5 of source size
  f32 min   = stx::F32_MIN;        /// clamps the source size, i.e. value should be at least 20px
  f32 max   = stx::F32_MAX;        /// clamps the source size, i.e. value should be at most 100px
  f32 minr  = 0;                   /// clamps the source size relatively. i.e. value should be at least 0.5 of source size
  f32 maxr  = 1;                   /// clamps the source size relatively. i.e. value should be at most 0.5 of source size

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
    return constraint{.bias = bias, .scale = scale, .min = v, .max = max, .minr = minr, .maxr = maxr};
  }

  constexpr constraint with_max(f32 v) const
  {
    return constraint{.bias = bias, .scale = scale, .min = min, .max = v, .minr = minr, .maxr = maxr};
  }

  constexpr f32 resolve(f32 value) const
  {
    return std::clamp(std::clamp(bias + value * scale, min, max), minr * value, maxr * value);
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
