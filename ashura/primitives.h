#pragma once

#include <algorithm>
#include <chrono>
#include <cmath>
#include <limits>
#include <type_traits>

#include "ashura/types.h"
#include "stx/limits.h"
#include "stx/option.h"

#define ASH_TO_RADIANS(...) (f32)(::ash::PI * (__VA_ARGS__) / 180.0f)

namespace ash
{

constexpr bool is_pointer_aligned(void *pointer, uintptr_t alignment)
{
  return (((uintptr_t) pointer) % alignment) == 0;
}

constexpr usize align_offset(usize offset, usize alignment)
{
  return (offset + (alignment - 1)) / alignment;
}

template <typename T, typename Base>
concept Impl = std::is_base_of_v<Base, T>;

template <typename T>
constexpr bool has_bits(T src, T cmp)
{
  return (src & cmp) == cmp;
}

template <typename T>
constexpr bool has_any_bit(T src, T cmp)
{
  return (src & cmp) != (T) 0;
}

using Clock        = std::chrono::steady_clock;        // monotonic system clock
using timepoint    = Clock::time_point;
using nanoseconds  = std::chrono::nanoseconds;
using milliseconds = std::chrono::milliseconds;
using seconds      = std::chrono::seconds;

constexpr f32 PI  = 3.14159265358979323846f;
constexpr f32 INF = std::numeric_limits<f32>::infinity();

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

static constexpr u32 log2_floor_u32(u32 x)
{
  u32 result = 0;
  for (u32 i = 1; i < 32; i++)
  {
    result += (u32) (bool) (x >> i);
  }
  return result;
}

// WARNING: the only non-floating-point integral type you should be using this
// for is i64.
template <typename T>
constexpr T lerp(T const &a, T const &b, f32 t)
{
  return static_cast<T>(a + (b - a) * t);
}

// template<typename T>
// constexpr T grid_snap(T const& a, T const& unit){

// (a / unit);

// }

// 	/** Snaps a value to the nearest grid multiple */
// 	template< class T >
// 	UE_NODISCARD static constexpr FORCEINLINE T GridSnap(T Location, T Grid)
// 	{
// 		return (Grid == T{}) ? Location : (Floor((Location + (Grid/(T)2)) /
// Grid) * Grid);
// 	}

/*
 *	Cubic Catmull-Rom Spline interpolation. Based on
 *http://www.cemyuksel.com/research/catmullrom_param/catmullrom.pdf Curves are
 *guaranteed to pass through the control points and are easily chained together.
 *Equation supports abitrary parameterization. eg. Uniform=0,1,2,3 ; chordal=
 *|Pn - Pn-1| ; centripetal = |Pn - Pn-1|^0.5 P0 - The control point preceding
 *the interpolation range. P1 - The control point starting the interpolation
 *range. P2 - The control point ending the interpolation range. P3 - The control
 *point following the interpolation range. T0-3 - The interpolation parameters
 *for the corresponding control points. T - The interpolation factor in the
 *range 0 to 1. 0 returns P1. 1 returns P2.
 */
// template< class U >
// UE_NODISCARD static constexpr FORCEINLINE_DEBUGGABLE U
// CubicCRSplineInterp(const U& P0, const U& P1, const U& P2, const U& P3, const
// float T0, const float T1, const float T2, const float T3, const float T)
// {
// 	//Based on http://www.cemyuksel.com/research/catmullrom_param/catmullrom.pdf
// 	float InvT1MinusT0 = 1.0f / (T1 - T0);
// 	U L01 = ( P0 * ((T1 - T) * InvT1MinusT0) ) + ( P1 * ((T - T0) *
// InvT1MinusT0) ); 	float InvT2MinusT1 = 1.0f / (T2 - T1); 	U L12 = ( P1 *
// ((T2 - T) * InvT2MinusT1) ) + ( P2 * ((T - T1) * InvT2MinusT1) ); 	float
// InvT3MinusT2 = 1.0f / (T3 - T2); 	U L23 = ( P2 * ((T3 - T) * InvT3MinusT2)
// ) + ( P3 * ((T - T2) * InvT3MinusT2) );

// 	float InvT2MinusT0 = 1.0f / (T2 - T0);
// 	U L012 = ( L01 * ((T2 - T) * InvT2MinusT0) ) + ( L12 * ((T - T0) *
// InvT2MinusT0) ); 	float InvT3MinusT1 = 1.0f / (T3 - T1); 	U L123 = ( L12 *
// ((T3
// - T) * InvT3MinusT1) ) + ( L23 * ((T - T1) * InvT3MinusT1) );

// 	return  ( ( L012 * ((T2 - T) * InvT2MinusT1) ) + ( L123 * ((T - T1) *
// InvT2MinusT1) ) );
// }

template <typename T>
constexpr f32 inverse_lerp(T const &a, T const &b, T const &v)
{
  return (f32) (v - a) / (f32) (b - a);
}

struct Vec2
{
  f32 x = 0, y = 0;

  static constexpr Vec2 splat(f32 v)
  {
    return Vec2{.x = v, .y = v};
  }

  constexpr f32 &operator[](usize i)
  {
    return (&x)[i];
  }

  constexpr f32 const &operator[](usize i) const
  {
    return (&x)[i];
  }
};

constexpr Vec2 epsilon_clamp(Vec2 a)
{
  return Vec2{.x = epsilon_clamp(a.x), .y = epsilon_clamp(a.y)};
}

constexpr bool operator==(Vec2 a, Vec2 b)
{
  return a.x == b.x && a.y == b.y;
}

constexpr bool operator!=(Vec2 a, Vec2 b)
{
  return a.x != b.x || a.y != b.y;
}

constexpr Vec2 operator+(Vec2 a, Vec2 b)
{
  return Vec2{a.x + b.x, a.y + b.y};
}

constexpr Vec2 operator+(Vec2 a, f32 b)
{
  return Vec2{a.x + b, a.y + b};
}

constexpr Vec2 operator+(f32 a, Vec2 b)
{
  return b + a;
}

constexpr Vec2 operator-(Vec2 a, Vec2 b)
{
  return Vec2{a.x - b.x, a.y - b.y};
}

constexpr Vec2 operator-(Vec2 a, f32 b)
{
  return Vec2{a.x - b, a.y - b};
}

constexpr Vec2 operator-(f32 a, Vec2 b)
{
  return Vec2{a - b.x, a - b.y};
}

constexpr Vec2 operator*(Vec2 a, Vec2 b)
{
  return Vec2{a.x * b.x, a.y * b.y};
}

constexpr Vec2 operator*(Vec2 a, f32 b)
{
  return Vec2{a.x * b, a.y * b};
}

constexpr Vec2 operator*(f32 a, Vec2 b)
{
  return b * a;
}

constexpr Vec2 operator/(Vec2 a, Vec2 b)
{
  return Vec2{a.x / b.x, a.y / b.y};
}

constexpr Vec2 operator/(Vec2 a, f32 b)
{
  return Vec2{a.x / b, a.y / b};
}

constexpr Vec2 operator/(f32 a, Vec2 b)
{
  return Vec2{a / b.x, a / b.y};
}

constexpr Vec2 &operator+=(Vec2 &a, Vec2 b)
{
  a = a + b;
  return a;
}

constexpr Vec2 &operator+=(Vec2 &a, f32 b)
{
  a = a + b;
  return a;
}

constexpr Vec2 &operator-=(Vec2 &a, Vec2 b)
{
  a = a - b;
  return a;
}

constexpr Vec2 &operator-=(Vec2 &a, f32 b)
{
  a = a - b;
  return a;
}

constexpr Vec2 &operator*=(Vec2 &a, Vec2 b)
{
  a = a * b;
  return a;
}

constexpr Vec2 &operator*=(Vec2 &a, f32 b)
{
  a = a * b;
  return a;
}

constexpr Vec2 &operator/=(Vec2 &a, Vec2 b)
{
  a = a / b;
  return a;
}

constexpr Vec2 &operator/=(Vec2 &a, f32 b)
{
  a = a / b;
  return a;
}

constexpr f32 dot(Vec2 a, Vec2 b)
{
  return a.x * b.x + a.y * b.y;
}

constexpr f32 cross(Vec2 a, Vec2 b)
{
  return a.x * b.y - b.x * a.y;
}

struct tri
{
  Vec2 p0, p1, p2;

  constexpr f32 sign() const
  {
    return (p0.x - p2.x) * (p1.y - p2.y) - (p1.x - p2.x) * (p0.y - p2.y);
  }

  constexpr bool contains(Vec2 point) const
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
struct Quad
{
  Vec2 p0, p1, p2, p3;

  constexpr bool contains(Vec2 point) const
  {
    return tri{.p0 = p0, .p1 = p1, .p2 = p2}.contains(point) ||
           tri{.p0 = p0, .p1 = p2, .p2 = p3}.contains(point);
  }
};

struct Rect
{
  Vec2 offset, extent;

  constexpr auto bounds() const
  {
    return std::make_tuple(offset.x, offset.x + extent.x, offset.y,
                           offset.y + extent.y);
  }

  constexpr Vec2 min() const
  {
    return offset;
  }

  constexpr Vec2 max() const
  {
    return offset + extent;
  }

  constexpr bool overlaps(Rect other) const
  {
    auto [x0_min, x0_max, y0_min, y0_max] = bounds();
    auto [x1_min, x1_max, y1_min, y1_max] = other.bounds();

    return x0_min < x1_max && x0_max > x1_min && y1_max > y0_min &&
           y1_min < y0_max;
  }

  constexpr bool overlaps(Quad const &quad) const
  {
    return contains(quad.p0) || contains(quad.p1) || contains(quad.p2) ||
           contains(quad.p3);
  }

  /// @brief NOTE: returns 0-extent rect if there's no intersection
  /// @param other
  /// @return
  constexpr Rect intersect(Rect other) const
  {
    auto const [x1_min, x1_max, y1_min, y1_max] = bounds();
    auto const [x2_min, x2_max, y2_min, y2_max] = other.bounds();

    if (!overlaps(other))
    {
      return Rect{.offset = offset, .extent = Vec2{0, 0}};
    }

    Vec2 intersect_offset{std::max(x1_min, x2_min), std::max(y1_min, y2_min)};
    Vec2 intersect_extent{std::min(x1_max, x2_max) - offset.x,
                          std::min(y1_max, y2_max) - offset.y};

    return Rect{.offset = intersect_offset, .extent = intersect_extent};
  }

  constexpr bool contains(Vec2 point) const
  {
    return offset.x <= point.x && offset.y <= point.y &&
           (offset.x + extent.x) >= point.x && (offset.y + extent.y) >= point.y;
  }

  constexpr bool is_visible() const
  {
    return extent.x != 0 && extent.y != 0;
  }

  constexpr Vec2 top_left() const
  {
    return offset;
  }

  constexpr Vec2 top_right() const
  {
    return offset + Vec2{extent.x, 0};
  }

  constexpr Vec2 bottom_left() const
  {
    return offset + Vec2{0, extent.y};
  }

  constexpr Vec2 bottom_right() const
  {
    return offset + extent;
  }

  constexpr Quad to_quad() const
  {
    return Quad{.p0 = top_left(),
                .p1 = top_right(),
                .p2 = bottom_right(),
                .p3 = bottom_left()};
  }

  constexpr Rect with_offset(Vec2 new_offset) const
  {
    return Rect{.offset = new_offset, .extent = extent};
  }

  constexpr Rect with_offset(f32 x, f32 y) const
  {
    return Rect{.offset = Vec2{x, y}, .extent = extent};
  }

  constexpr Rect with_extent(Vec2 new_extent) const
  {
    return Rect{.offset = offset, .extent = new_extent};
  }

  constexpr Rect with_extent(f32 w, f32 h) const
  {
    return Rect{.offset = offset, .extent = Vec2{w, h}};
  }

  constexpr Rect with_center(Vec2 center) const
  {
    return Rect{.offset = center - extent / 2, .extent = extent};
  }

  constexpr Rect with_center(f32 cx, f32 cy) const
  {
    return with_center(Vec2{cx, cy});
  }

  constexpr Rect centered() const
  {
    return with_center(offset);
  }
};

struct Vec3
{
  f32 x = 0, y = 0, z = 0, __padding__ = 0;

  static constexpr Vec3 splat(f32 v)
  {
    return Vec3{.x = v, .y = v, .z = v};
  }

  constexpr f32 &operator[](usize i)
  {
    return (&x)[i];
  }

  constexpr f32 const &operator[](usize i) const
  {
    return (&x)[i];
  }
};

constexpr bool operator==(Vec3 a, Vec3 b)
{
  return a.x == b.x && a.y == b.y && a.z == b.z;
}

constexpr bool operator!=(Vec3 a, Vec3 b)
{
  return a.x != b.x || a.y != b.y || a.z != b.z;
}

constexpr Vec3 operator+(Vec3 a, Vec3 b)
{
  return Vec3{a.x + b.x, a.y + b.y, a.z + b.z};
}

constexpr Vec3 operator+(Vec3 a, f32 b)
{
  return Vec3{a.x + b, a.y + b, a.z + b};
}

constexpr Vec3 operator+(f32 a, Vec3 b)
{
  return Vec3{a + b.x, a + b.y, a + b.z};
}

constexpr Vec3 operator-(Vec3 a, Vec3 b)
{
  return Vec3{a.x - b.x, a.y - b.y, a.z - b.z};
}

constexpr Vec3 operator-(Vec3 a, f32 b)
{
  return Vec3{a.x - b, a.y - b, a.z - b};
}

constexpr Vec3 operator-(f32 a, Vec3 b)
{
  return Vec3{a - b.x, a - b.y, a - b.z};
}

constexpr Vec3 operator*(Vec3 a, Vec3 b)
{
  return Vec3{a.x * b.x, a.y * b.y, a.z * b.z};
}

constexpr Vec3 operator*(Vec3 a, f32 b)
{
  return Vec3{a.x * b, a.y * b, a.z * b};
}

constexpr Vec3 operator*(f32 a, Vec3 b)
{
  return Vec3{a * b.x, a * b.y, a * b.z};
}

constexpr Vec3 operator/(Vec3 a, Vec3 b)
{
  return Vec3{a.x / b.x, a.y / b.y, a.z / b.z};
}

constexpr Vec3 operator/(Vec3 a, f32 b)
{
  return Vec3{a.x / b, a.y / b, a.z / b};
}

constexpr Vec3 operator/(f32 a, Vec3 b)
{
  return Vec3{a / b.x, a / b.y, a / b.z};
}

constexpr f32 dot(Vec3 a, Vec3 b)
{
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

struct Box
{
  Vec3 offset, extent;

  constexpr f32 volume() const
  {
    return extent.x * extent.y * extent.z;
  }

  constexpr Vec3 min() const
  {
    return offset;
  }

  constexpr Vec3 max() const
  {
    return offset + extent;
  }

  constexpr bool contains(Vec3 point) const
  {
    return offset.x <= point.x && offset.y <= point.y && offset.z <= point.z &&
           (offset.x + extent.x) >= point.x &&
           (offset.y + extent.y) >= point.y && (offset.z + extent.z) >= point.z;
    return true;
  }

  constexpr auto bounds() const
  {
    return std::make_tuple(offset.x, offset.x + extent.x, offset.y,
                           offset.y + extent.y, offset.z, offset.z + extent.z);
  }

  constexpr bool overlaps(Box other) const
  {
    auto [x0_min, x0_max, y0_min, y0_max, z0_min, z0_max] = bounds();
    auto [x1_min, x1_max, y1_min, y1_max, z1_min, z1_max] = other.bounds();

    return x0_min < x1_max && x0_max > x1_min && y1_max > y0_min &&
           y1_min < y0_max && z1_max > z0_min && z1_min < z0_max;
  }
};

/// column vector
struct Vec4
{
  f32 x = 0, y = 0, z = 0, w = 0;

  static constexpr Vec4 splat(f32 v)
  {
    return Vec4{.x = v, .y = v, .z = v, .w = v};
  }

  constexpr f32 &operator[](usize i)
  {
    return (&x)[i];
  }

  constexpr f32 const &operator[](usize i) const
  {
    return (&x)[i];
  }
};

constexpr bool operator==(Vec4 a, Vec4 b)
{
  return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
}

constexpr bool operator!=(Vec4 a, Vec4 b)
{
  return a.x != b.x || a.y != b.y || a.z != b.z || a.w != b.w;
}

constexpr Vec4 operator+(Vec4 a, Vec4 b)
{
  return Vec4{a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w};
}

constexpr Vec4 operator+(Vec4 a, f32 b)
{
  return Vec4{a.x + b, a.y + b, a.z + b, a.w + b};
}

constexpr Vec4 operator+(f32 a, Vec4 b)
{
  return Vec4{a + b.x, a + b.y, a + b.z, a + b.w};
}

constexpr Vec4 operator-(Vec4 a, Vec4 b)
{
  return Vec4{a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w};
}

constexpr Vec4 operator-(Vec4 a, f32 b)
{
  return Vec4{a.x - b, a.y - b, a.z - b, a.w - b};
}

constexpr Vec4 operator-(f32 a, Vec4 b)
{
  return Vec4{a - b.x, a - b.y, a - b.z, a - b.w};
}

constexpr Vec4 operator*(Vec4 a, Vec4 b)
{
  return Vec4{a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w};
}

constexpr Vec4 operator*(Vec4 a, f32 b)
{
  return Vec4{a.x * b, a.y * b, a.z * b, a.w * b};
}

constexpr Vec4 operator*(f32 a, Vec4 b)
{
  return Vec4{a * b.x, a * b.y, a * b.z, a * b.w};
}

constexpr Vec4 operator/(Vec4 a, Vec4 b)
{
  return Vec4{a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w};
}

constexpr Vec4 operator/(Vec4 a, f32 b)
{
  return Vec4{a.x / b, a.y / b, a.z / b, a.w / b};
}

constexpr Vec4 operator/(f32 a, Vec4 b)
{
  return Vec4{a / b.x, a / b.y, a / b.z, a / b.w};
}

constexpr f32 dot(Vec4 a, Vec4 b)
{
  return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

struct Mat2
{
  Vec2 rows[2];

  static constexpr Mat2 identity()
  {
    return Mat2{.rows = {{.x = 1, .y = 0}, {.x = 0, .y = 1}}};
  }

  constexpr Mat2 transpose() const
  {
    return Mat2{.rows = {{rows[0][0], rows[1][0]}, {rows[0][1], rows[1][1]}}};
  }

  constexpr Vec2 &operator[](usize i)
  {
    return rows[i];
  }

  constexpr Vec2 const &operator[](usize i) const
  {
    return rows[i];
  }
};

constexpr Mat2 operator*(Mat2 a, f32 b)
{
  return Mat2{.rows = {a[0] * b, a[1] * b}};
}

constexpr Mat2 operator*(f32 a, Mat2 b)
{
  return Mat2{.rows = {a * b[0], a * b[1]}};
}

constexpr f32 determinant(Mat2 a)
{
  return a[0][0] * a[1][1] - a[1][0] * a[0][1];
}

constexpr Mat2 adjoint(Mat2 a)
{
  return Mat2{.rows = {{a[1][1], -a[0][1]}, {-a[1][0], a[0][0]}}};
}

constexpr Mat2 inverse(Mat2 a)
{
  return 1 / determinant(a) * adjoint(a);
}

/// row-major
struct Mat3
{
  Vec3 rows[3];

  static constexpr Mat3 identity()
  {
    return Mat3{.rows = {{.x = 1, .y = 0, .z = 0},
                         {.x = 0, .y = 1, .z = 0},
                         {.x = 0, .y = 0, .z = 1}}};
  }

  constexpr Mat3 transpose() const
  {
    return Mat3{.rows = {{rows[0][0], rows[1][0], rows[2][0]},
                         {rows[0][1], rows[1][1], rows[2][1]},
                         {rows[0][2], rows[1][2], rows[2][2]}}};
  }

  constexpr Vec3 &operator[](usize i)
  {
    return rows[i];
  }

  constexpr Vec3 const &operator[](usize i) const
  {
    return rows[i];
  }
};

constexpr Mat3 operator*(Mat3 a, f32 b)
{
  return Mat3{.rows = {a[0] * b, a[1] * b, a[2] * b}};
}

constexpr Mat3 operator*(f32 a, Mat3 b)
{
  return Mat3{.rows = {a * b[0], a * b[1], a * b[2]}};
}

constexpr Vec3 operator*(Mat3 const &a, Vec3 const &b)
{
  return Vec3{.x = dot(a[0], b), .y = dot(a[1], b), .z = dot(a[2], b)};
}

constexpr Mat3 operator*(Mat3 const &a, Mat3 const &b)
{
  return Mat3{.rows = {{dot(a[0], {b[0][0], b[1][0], b[2][0]}),
                        dot(a[0], {b[0][1], b[1][1], b[2][1]}),
                        dot(a[0], {b[0][2], b[1][2], b[2][2]})},
                       {dot(a[1], {b[0][0], b[1][0], b[2][0]}),
                        dot(a[1], {b[0][1], b[1][1], b[2][1]}),
                        dot(a[1], {b[0][2], b[1][2], b[2][2]})},
                       {dot(a[2], {b[0][0], b[1][0], b[2][0]}),
                        dot(a[2], {b[0][1], b[1][1], b[2][1]}),
                        dot(a[2], {b[0][2], b[1][2], b[2][2]})}}};
}

constexpr f32 determinant(Mat3 const &a)
{
  return a[0][0] * a[1][1] * a[2][2] - a[0][0] * a[1][2] * a[2][1] -
         a[0][1] * a[1][0] * a[2][2] + a[0][1] * a[1][2] * a[2][0] +
         a[0][2] * a[1][0] * a[2][1] - a[0][2] * a[1][1] * a[2][0];
}

constexpr Mat3 adjoint(Mat3 const &a)
{
  return Mat3{.rows = {{a[1][1] * a[2][2] - a[1][2] * a[2][1],
                        a[0][2] * a[2][1] - a[0][1] * a[2][2],
                        a[0][1] * a[1][2] - a[0][2] * a[1][1]},
                       {a[1][2] * a[2][0] - a[1][0] * a[2][2],
                        a[0][0] * a[2][2] - a[0][2] * a[2][0],
                        a[0][2] * a[1][0] - a[0][0] * a[1][2]},
                       {a[1][0] * a[2][1] - a[1][1] * a[2][0],
                        a[0][1] * a[2][0] - a[0][0] * a[2][1],
                        a[0][0] * a[1][1] - a[0][1] * a[1][0]}}};
}

constexpr Mat3 inverse(Mat3 const &a)
{
  return 1 / determinant(a) * adjoint(a);
}

/// row-major
struct Mat4
{
  Vec4 rows[4];

  static constexpr Mat4 identity()
  {
    return Mat4{.rows = {{.x = 1, .y = 0, .z = 0, .w = 0},
                         {.x = 0, .y = 1, .z = 0, .w = 0},
                         {.x = 0, .y = 0, .z = 1, .w = 0},
                         {.x = 0, .y = 0, .z = 0, .w = 1}}};
  }

  constexpr Mat4 transpose() const
  {
    return Mat4{.rows = {{rows[0][0], rows[1][0], rows[2][0], rows[3][0]},
                         {rows[0][1], rows[1][1], rows[2][1], rows[3][1]},
                         {rows[0][2], rows[1][2], rows[2][2], rows[3][2]},
                         {rows[0][3], rows[1][3], rows[2][3], rows[3][3]}}};
  }

  constexpr Vec4 &operator[](usize i)
  {
    return rows[i];
  }

  constexpr Vec4 const &operator[](usize i) const
  {
    return rows[i];
  }
};

constexpr Mat4 operator*(Mat4 a, f32 b)
{
  return Mat4{.rows = {a[0] * b, a[1] * b, a[2] * b, a[3] * b}};
}

constexpr Mat4 operator*(f32 a, Mat4 b)
{
  return Mat4{.rows = {a * b[0], a * b[1], a * b[2], a * b[3]}};
}

constexpr bool operator==(Mat4 const &a, Mat4 const &b)
{
  return a[0] == b[0] && a[1] == b[1] && a[2] == b[2] && a[3] == b[3];
}

constexpr bool operator!=(Mat4 const &a, Mat4 const &b)
{
  return a[0] != b[0] || a[1] != b[1] || a[2] != b[2] || a[3] != b[3];
}

constexpr Mat4 operator*(Mat4 const &a, Mat4 const &b)
{
  return Mat4{.rows = {{dot(a[0], {b[0][0], b[1][0], b[2][0], b[3][0]}),
                        dot(a[0], {b[0][1], b[1][1], b[2][1], b[3][1]}),
                        dot(a[0], {b[0][2], b[1][2], b[2][2], b[3][2]}),
                        dot(a[0], {b[0][3], b[1][3], b[2][3], b[3][3]})},
                       {dot(a[1], {b[0][0], b[1][0], b[2][0], b[3][0]}),
                        dot(a[1], {b[0][1], b[1][1], b[2][1], b[3][1]}),
                        dot(a[1], {b[0][2], b[1][2], b[2][2], b[3][2]}),
                        dot(a[1], {b[0][3], b[1][3], b[2][3], b[3][3]})},
                       {dot(a[2], {b[0][0], b[1][0], b[2][0], b[3][0]}),
                        dot(a[2], {b[0][1], b[1][1], b[2][1], b[3][1]}),
                        dot(a[2], {b[0][2], b[1][2], b[2][2], b[3][2]}),
                        dot(a[2], {b[0][3], b[1][3], b[2][3], b[3][3]})},
                       {dot(a[3], {b[0][0], b[1][0], b[2][0], b[3][0]}),
                        dot(a[3], {b[0][1], b[1][1], b[2][1], b[3][1]}),
                        dot(a[3], {b[0][2], b[1][2], b[2][2], b[3][2]}),
                        dot(a[3], {b[0][3], b[1][3], b[2][3], b[3][3]})}}};
}

constexpr Vec4 operator*(Mat4 const &a, Vec4 const &b)
{
  return Vec4{.x = dot(a[0], b),
              .y = dot(a[1], b),
              .z = dot(a[2], b),
              .w = dot(a[3], b)};
}

constexpr f32 determinant(Mat4 const &a)
{
  return a[0][0] * (a[1][1] * a[2][2] * a[3][3] + a[1][2] * a[2][3] * a[3][1] +
                    a[1][3] * a[2][1] * a[3][2] - a[1][3] * a[2][2] * a[3][1] -
                    a[1][2] * a[2][1] * a[3][3] - a[1][1] * a[2][3] * a[3][2]) -
         a[1][0] * (a[0][1] * a[2][2] * a[3][3] + a[0][2] * a[2][3] * a[3][1] +
                    a[0][3] * a[2][1] * a[3][2] - a[0][3] * a[2][2] * a[3][1] -
                    a[0][2] * a[2][1] * a[3][3] - a[0][1] * a[2][3] * a[3][2]) +
         a[2][0] * (a[0][1] * a[1][2] * a[3][3] + a[0][2] * a[1][3] * a[3][1] +
                    a[0][3] * a[1][1] * a[3][2] - a[0][3] * a[1][2] * a[3][1] -
                    a[0][2] * a[1][1] * a[3][3] - a[0][1] * a[1][3] * a[3][2]) -
         a[3][0] * (a[0][1] * a[1][2] * a[2][3] + a[0][2] * a[1][3] * a[2][1] +
                    a[0][3] * a[1][1] * a[2][2] - a[0][3] * a[1][2] * a[2][1] -
                    a[0][2] * a[1][1] * a[2][3] - a[0][1] * a[1][3] * a[2][2]);
}

constexpr Mat4 adjoint(Mat4 const &a)
{
  Mat4 r;
  r[0][0] = a[1][1] * a[2][2] * a[3][3] + a[1][2] * a[2][3] * a[3][1] +
            a[1][3] * a[2][1] * a[3][2] - a[1][3] * a[2][2] * a[3][1] -
            a[1][2] * a[2][1] * a[3][3] - a[1][1] * a[2][3] * a[3][2];
  r[0][1] = -a[0][1] * a[2][2] * a[3][3] - a[0][2] * a[2][3] * a[3][1] -
            a[0][3] * a[2][1] * a[3][2] + a[0][3] * a[2][2] * a[3][1] +
            a[0][2] * a[2][1] * a[3][3] + a[0][1] * a[2][3] * a[3][2];
  r[0][2] = a[0][1] * a[1][2] * a[3][3] + a[0][2] * a[1][3] * a[3][1] +
            a[0][3] * a[1][1] * a[3][2] - a[0][3] * a[1][2] * a[3][1] -
            a[0][2] * a[1][1] * a[3][3] - a[0][1] * a[1][3] * a[3][2];
  r[0][3] = -a[0][1] * a[1][2] * a[2][3] - a[0][2] * a[1][3] * a[2][1] -
            a[0][3] * a[1][1] * a[2][2] + a[0][3] * a[1][2] * a[2][1] +
            a[0][2] * a[1][1] * a[2][3] + a[0][1] * a[1][3] * a[2][2];
  r[1][0] = -a[1][0] * a[2][2] * a[3][3] - a[1][2] * a[2][3] * a[3][0] -
            a[1][3] * a[2][0] * a[3][2] + a[1][3] * a[2][2] * a[3][0] +
            a[1][2] * a[2][0] * a[3][3] + a[1][0] * a[2][3] * a[3][2];
  r[1][1] = a[0][0] * a[2][2] * a[3][3] + a[0][2] * a[2][3] * a[3][0] +
            a[0][3] * a[2][0] * a[3][2] - a[0][3] * a[2][2] * a[3][0] -
            a[0][2] * a[2][0] * a[3][3] - a[0][0] * a[2][3] * a[3][2];
  r[1][2] = -a[0][0] * a[1][2] * a[3][3] - a[0][2] * a[1][3] * a[3][0] -
            a[0][3] * a[1][0] * a[3][2] + a[0][3] * a[1][2] * a[3][0] +
            a[0][2] * a[1][0] * a[3][3] + a[0][0] * a[1][3] * a[3][2];
  r[1][3] = a[0][0] * a[1][2] * a[2][3] + a[0][2] * a[1][3] * a[2][0] +
            a[0][3] * a[1][0] * a[2][2] - a[0][3] * a[1][2] * a[2][0] -
            a[0][2] * a[1][0] * a[2][3] - a[0][0] * a[1][3] * a[2][2];
  r[2][0] = a[1][0] * a[2][1] * a[3][3] + a[1][1] * a[2][3] * a[3][0] +
            a[1][3] * a[2][0] * a[3][1] - a[1][3] * a[2][1] * a[3][0] -
            a[1][1] * a[2][1] * a[3][3] - a[1][0] * a[2][3] * a[3][1];
  r[2][1] = -a[0][0] * a[2][1] * a[3][3] - a[0][1] * a[2][3] * a[3][0] -
            a[0][3] * a[2][0] * a[3][1] + a[0][3] * a[2][1] * a[3][0] +
            a[0][1] * a[2][0] * a[3][3] + a[0][0] * a[2][3] * a[3][1];
  r[2][2] = a[0][0] * a[1][1] * a[3][3] + a[0][1] * a[1][3] * a[3][0] +
            a[0][3] * a[1][0] * a[3][1] - a[0][3] * a[1][1] * a[3][0] -
            a[0][1] * a[1][0] * a[3][3] - a[0][0] * a[1][3] * a[3][1];
  r[2][3] = -a[0][0] * a[1][1] * a[2][3] - a[0][1] * a[1][3] * a[2][0] -
            a[0][3] * a[1][0] * a[2][1] + a[0][3] * a[1][1] * a[2][0] +
            a[0][1] * a[1][0] * a[2][3] + a[0][0] * a[1][3] * a[2][1];
  r[3][0] = -a[1][0] * a[2][1] * a[3][2] - a[1][1] * a[2][2] * a[3][0] -
            a[1][2] * a[2][0] * a[3][1] + a[1][2] * a[2][1] * a[3][0] +
            a[1][1] * a[2][0] * a[3][2] + a[1][0] * a[2][2] * a[3][1];
  r[3][1] = a[0][0] * a[2][1] * a[3][2] + a[0][1] * a[2][2] * a[3][0] +
            a[0][2] * a[2][0] * a[3][1] - a[0][2] * a[2][1] * a[3][0] -
            a[0][1] * a[2][0] * a[3][2] - a[0][0] * a[2][2] * a[3][1];
  r[3][2] = -a[0][0] * a[1][1] * a[3][2] - a[0][1] * a[1][2] * a[3][0] -
            a[0][2] * a[1][0] * a[3][1] + a[0][2] * a[1][1] * a[3][0] +
            a[0][1] * a[1][0] * a[3][2] + a[0][0] * a[1][2] * a[3][1];
  r[3][3] = a[0][0] * a[1][1] * a[2][2] + a[0][1] * a[1][2] * a[2][1] +
            a[0][2] * a[1][0] * a[2][1] - a[0][2] * a[1][1] * a[2][0] -
            a[0][1] * a[1][0] * a[2][2] - a[0][0] * a[1][2] * a[2][1];
  return r;
}

constexpr Mat4 inverse(Mat4 const &a)
{
  return 1 / determinant(a) * adjoint(a);
}

constexpr Vec2 transform3d(Mat4 const &a, Vec2 const &b)
{
  Vec4 prod = a * Vec4{b.x, b.y, 0, 1};
  return Vec2{.x = prod.x, .y = prod.y};
}

constexpr Vec2 transform3d(Mat4 const &a, Vec3 const &b)
{
  Vec4 prod = a * Vec4{b.x, b.y, b.z, 1};
  return Vec2{.x = prod.x, .y = prod.y};
}

constexpr Vec2 transform2d(Mat3 const &a, Vec2 const &b)
{
  Vec3 prod = a * Vec3{b.x, b.y, 1};
  return Vec2{prod.x, prod.y};
}

constexpr Quad transform2d(Mat3 const &a, Rect const &b)
{
  return Quad{.p0 = transform2d(a, b.top_left()),
              .p1 = transform2d(a, b.top_right()),
              .p2 = transform2d(a, b.bottom_right()),
              .p3 = transform2d(a, b.bottom_left())};
}

constexpr Mat3 translate2d(Vec2 t)
{
  return Mat3{.rows = {{1, 0, t.x}, {0, 1, t.y}, {0, 0, 1}}};
}

constexpr Mat3 translate2d(f32 tx, f32 ty)
{
  return translate2d(Vec2{tx, ty});
}

constexpr Mat4 translate3d(Vec3 t)
{
  return Mat4{
      .rows = {{1, 0, 0, t.x}, {0, 1, 0, t.y}, {0, 0, 1, t.z}, {0, 0, 0, 1}}};
}

constexpr Mat3 scale2d(Vec2 s)
{
  return Mat3{.rows = {{s.x, 0, 0}, {0, s.y, 0}, {0, 0, 1}}};
}

constexpr Mat3 scale2d(f32 sx, f32 sy)
{
  return scale2d(Vec2{sx, sy});
}

constexpr Mat4 scale3d(Vec3 s)
{
  return Mat4{
      .rows = {{s.x, 0, 0, 0}, {0, s.y, 0, 0}, {0, 0, s.z, 0}, {0, 0, 0, 1}}};
}

inline Mat3 rotate2d(f32 degree_radians)
{
  return Mat3{.rows = {{std::cos(degree_radians), -std::sin(degree_radians), 0},
                       {std::sin(degree_radians), std::cos(degree_radians), 0},
                       {0, 0, 1}}};
}

inline Mat4 rotate3d_x(f32 degree_radians)
{
  return Mat4{
      .rows = {{1, 0, 0, 0},
               {0, std::cos(degree_radians), -std::sin(degree_radians), 0},
               {0, std::sin(degree_radians), std::cos(degree_radians), 0},
               {0, 0, 0, 1}}};
}

inline Mat4 rotate3d_y(f32 degree_radians)
{
  return Mat4{
      .rows = {{std::cos(degree_radians), 0, std::sin(degree_radians), 0},
               {0, 1, 0, 0},
               {-std::sin(degree_radians), 0, std::cos(degree_radians), 0},
               {0, 0, 0, 1}}};
}

inline Mat4 rotate3d_z(f32 degree_radians)
{
  return Mat4{
      .rows = {{std::cos(degree_radians), -std::sin(degree_radians), 0, 0},
               {std::sin(degree_radians), std::cos(degree_radians), 0, 0},
               {0, 0, 1, 0},
               {0, 0, 0, 1}}};
}

constexpr Mat3 shear2d_x(f32 x_shear)
{
  return Mat3{.rows = {{1, 0, 0}, {x_shear, 1, 0}, {0, 0, 1}}};
}

constexpr Mat3 shear2d_y(f32 y_shear)
{
  return Mat3{.rows = {{1, y_shear, 0}, {0, 1, 0}, {0, 0, 1}}};
}

constexpr Mat4 shear3d_x(f32 y_shear, f32 z_shear)
{
  return Mat4{
      .rows = {
          {1, y_shear, z_shear, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}}};
}

constexpr Mat4 shear3d_y(f32 x_shear, f32 z_shear)
{
  return Mat4{
      .rows = {
          {1, 0, 0, 0}, {x_shear, 1, z_shear, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}}};
}

constexpr Mat4 shear3d_z(f32 x_shear, f32 y_shear)
{
  return Mat4{
      .rows = {
          {1, 0, 0, 0}, {0, 1, 0, 0}, {x_shear, y_shear, 1, 0}, {0, 0, 0, 1}}};
}

struct Offset
{
  u32 x = 0, y = 0;

  constexpr Vec2 to_vec() const
  {
    return Vec2{.x = static_cast<f32>(x), .y = static_cast<f32>(y)};
  }
};

constexpr bool operator==(Offset a, Offset b)
{
  return a.x == b.x && a.y == b.y;
}

constexpr bool operator!=(Offset a, Offset b)
{
  return !(a == b);
}

constexpr Offset operator+(Offset a, Offset b)
{
  return Offset{.x = a.x + b.x, .y = a.y + b.y};
}

struct IOffset
{
  i32 x = 0, y = 0;

  constexpr Vec2 as_vec() const
  {
    return Vec2{static_cast<f32>(x), static_cast<f32>(y)};
  }
};

constexpr bool operator==(IOffset a, IOffset b)
{
  return a.x == b.x && a.y == b.y;
}

constexpr bool operator!=(IOffset a, IOffset b)
{
  return !(a == b);
}

constexpr IOffset operator+(IOffset a, IOffset b)
{
  return IOffset{.x = a.x + b.x, .y = a.y + b.y};
}

constexpr IOffset operator-(IOffset a, IOffset b)
{
  return IOffset{.x = a.x - b.x, .y = a.y - b.y};
}

struct Extent
{
  u32 width = 0, height = 0;

  static constexpr Extent from(Vec2 wh)
  {
    return Extent{static_cast<u32>(wh.x), static_cast<u32>(wh.y)};
  }

  constexpr bool is_visible() const
  {
    return width != 0 && height != 0;
  }

  constexpr Extent constrain(Extent other) const
  {
    return Extent{.width  = std::min(width, other.width),
                  .height = std::min(height, other.height)};
  }

  constexpr u64 area() const
  {
    return static_cast<u64>(width) * height;
  }

  constexpr Vec2 to_vec() const
  {
    return Vec2{.x = static_cast<f32>(width), .y = static_cast<f32>(height)};
  }

  constexpr u32 max_mip_levels() const
  {
    return log2_floor_u32(std::max(width, height)) + 1;
  }

  constexpr Extent at_mip_level(u32 mip_level) const
  {
    return Extent{.width = width >> mip_level, .height = height >> mip_level};
  }
};

struct Offset3D
{
  u32 x = 0, y = 0, z = 0;
};

struct IOffset3D
{
  i32 x = 0, y = 0, z = 0;
};

struct Extent3D
{
  u32 width = 0, height = 0, depth = 0;

  constexpr bool is_visible() const
  {
    return width != 0 && height != 0 && depth != 0;
  }

  constexpr u64 area() const
  {
    return static_cast<u64>(width) * static_cast<u64>(height) * depth;
  }

  constexpr u32 max_mip_levels() const
  {
    return log2_floor_u32(std::max(std::max(width, height), depth)) + 1;
  }

  constexpr Extent3D at_mip_level(u32 mip_level) const
  {
    return Extent3D{.width  = width >> mip_level,
                    .height = height >> mip_level,
                    .depth  = depth >> mip_level};
  }

  constexpr Offset3D to_offset() const
  {
    return Offset3D{.x = width, .y = height, .z = depth};
  }
};

constexpr bool operator==(Extent a, Extent b)
{
  return a.width == b.width && a.height == b.height;
}

constexpr bool operator!=(Extent a, Extent b)
{
  return !(a == b);
}

constexpr Extent operator+(Extent a, Extent b)
{
  return Extent{.width = a.width + b.width, .height = a.height + b.height};
}

constexpr Offset operator+(Offset a, Extent b)
{
  return Offset{.x = a.x + b.width, .y = a.y + b.height};
}

constexpr Offset operator+(Extent a, Offset b)
{
  return Offset{.x = a.width + b.x, .y = a.height + b.y};
}

struct URect
{
  Offset offset;
  Extent extent;

  constexpr Offset min() const
  {
    return offset;
  }

  constexpr Offset max() const
  {
    return offset + extent;
  }

  constexpr bool contains(URect const &other) const
  {
    return (offset.x <= other.offset.x) &&
           ((offset.x + extent.width) >=
            (other.offset.x + other.extent.width)) &&
           (offset.y <= other.offset.y) &&
           ((offset.y + extent.height) >=
            (other.offset.y + other.extent.height));
  }

  constexpr URect with_offset(Offset new_offset) const
  {
    return URect{.offset = new_offset, .extent = extent};
  }

  constexpr URect with_offset(u32 x, u32 y) const
  {
    return URect{.offset = Offset{x, y}, .extent = extent};
  }

  constexpr URect with_extent(Extent new_extent) const
  {
    return URect{.offset = offset, .extent = new_extent};
  }

  constexpr URect with_extent(u32 w, u32 h) const
  {
    return URect{.offset = offset, .extent = Extent{w, h}};
  }
};

struct URect3D
{
  Offset3D offset;
  Extent3D extent;

  constexpr bool contains(URect3D const &other) const
  {
    return (offset.x <= other.offset.x) &&
           ((offset.x + extent.width) >=
            (other.offset.x + other.extent.width)) &&
           (offset.y <= other.offset.y) &&
           ((offset.y + extent.height) >=
            (other.offset.y + other.extent.height)) &&
           (offset.z <= other.offset.z) &&
           ((offset.z + extent.depth) >= (other.offset.z + other.extent.depth));
  }
};

struct IRect3D
{
  IOffset3D offset;
  Extent3D  extent;
};

struct IRect
{
  IOffset offset;
  Extent  extent;
};

/// Simple Layout Constraint Model
/// @bias: adding or subtracting from the source size, i.e. value should be
/// source size - 20px
/// @scale: scales the source size, i.e. value should be 0.5 of source size
/// @min: clamps the source size, i.e. value should be at least 20px
/// @max: clamps the source size, i.e. value should be at most 100px
/// @minr: clamps the source size relatively. i.e. value should be at least 0.5
/// of source size
/// @maxr: clamps the source size relatively. i.e. value should be at most 0.5
/// of source size
struct Constraint
{
  f32 bias  = 0;
  f32 scale = 0;
  f32 min   = stx::F32_MIN;
  f32 max   = stx::F32_MAX;
  f32 minr  = 0;
  f32 maxr  = 1;

  static constexpr Constraint relative(f32 scale)
  {
    return Constraint{
        .bias = 0, .scale = scale, .min = stx::F32_MIN, .max = stx::F32_MAX};
  }

  static constexpr Constraint absolute(f32 value)
  {
    return Constraint{
        .bias = value, .scale = 0, .min = stx::F32_MIN, .max = stx::F32_MAX};
  }

  constexpr Constraint with_min(f32 v) const
  {
    return Constraint{.bias  = bias,
                      .scale = scale,
                      .min   = v,
                      .max   = max,
                      .minr  = minr,
                      .maxr  = maxr};
  }

  constexpr Constraint with_max(f32 v) const
  {
    return Constraint{.bias  = bias,
                      .scale = scale,
                      .min   = min,
                      .max   = v,
                      .minr  = minr,
                      .maxr  = maxr};
  }

  constexpr Constraint with_minr(f32 v) const
  {
    return Constraint{.bias  = bias,
                      .scale = scale,
                      .min   = min,
                      .max   = max,
                      .minr  = v,
                      .maxr  = maxr};
  }

  constexpr Constraint with_maxr(f32 v) const
  {
    return Constraint{.bias  = bias,
                      .scale = scale,
                      .min   = min,
                      .max   = max,
                      .minr  = minr,
                      .maxr  = v};
  }

  constexpr f32 resolve(f32 value) const
  {
    return std::clamp(std::clamp(bias + value * scale, min, max), minr * value,
                      maxr * value);
  }
};

struct Constraint2D
{
  Constraint x, y;

  static constexpr Constraint2D relative(f32 x, f32 y)
  {
    return Constraint2D{.x = Constraint::relative(x),
                        .y = Constraint::relative(y)};
  }

  static constexpr Constraint2D relative(Vec2 xy)
  {
    return relative(xy.x, xy.y);
  }

  static constexpr Constraint2D absolute(f32 x, f32 y)
  {
    return Constraint2D{.x = Constraint::absolute(x),
                        .y = Constraint::absolute(y)};
  }

  static constexpr Constraint2D absolute(Vec2 xy)
  {
    return absolute(xy.x, xy.y);
  }

  constexpr Constraint2D with_min(f32 nx, f32 ny) const
  {
    return Constraint2D{.x = x.with_min(nx), .y = y.with_min(ny)};
  }

  constexpr Constraint2D with_max(f32 nx, f32 ny) const
  {
    return Constraint2D{.x = x.with_max(nx), .y = y.with_max(ny)};
  }

  constexpr Constraint2D with_minr(f32 nx, f32 ny) const
  {
    return Constraint2D{.x = x.with_minr(nx), .y = y.with_minr(ny)};
  }

  constexpr Constraint2D with_maxr(f32 nx, f32 ny) const
  {
    return Constraint2D{.x = x.with_maxr(nx), .y = y.with_maxr(ny)};
  }

  constexpr Vec2 resolve(f32 xsrc, f32 ysrc) const
  {
    return Vec2{x.resolve(xsrc), y.resolve(ysrc)};
  }

  constexpr Vec2 resolve(Vec2 src) const
  {
    return resolve(src.x, src.y);
  }
};

struct BorderRadius
{
  Constraint top_left, top_right, bottom_right, bottom_left;

  static constexpr BorderRadius relative(f32 tl, f32 tr, f32 br, f32 bl)
  {
    return BorderRadius{.top_left     = Constraint::relative(tl),
                        .top_right    = Constraint::relative(tr),
                        .bottom_right = Constraint::relative(br),
                        .bottom_left  = Constraint::relative(bl)};
  }

  static constexpr BorderRadius relative(Vec4 v)
  {
    return relative(v.x, v.y, v.z, v.w);
  }

  static constexpr BorderRadius relative(f32 v)
  {
    return relative(v, v, v, v);
  }

  static constexpr BorderRadius absolute(f32 tl, f32 tr, f32 br, f32 bl)
  {
    return BorderRadius{.top_left     = Constraint::absolute(tl),
                        .top_right    = Constraint::absolute(tr),
                        .bottom_right = Constraint::absolute(br),
                        .bottom_left  = Constraint::absolute(bl)};
  }

  static constexpr BorderRadius absolute(Vec4 v)
  {
    return absolute(v.x, v.y, v.z, v.w);
  }

  static constexpr BorderRadius absolute(f32 v)
  {
    return absolute(v, v, v, v);
  }

  constexpr Vec4 resolve(f32 w, f32 h) const
  {
    f32 const src = std::min(w, h) / 2;
    return Vec4{.x = top_left.resolve(src),
                .y = top_right.resolve(src),
                .z = bottom_right.resolve(src),
                .w = bottom_left.resolve(src)};
  }

  constexpr Vec4 resolve(Vec2 wh) const
  {
    return resolve(wh.x, wh.y);
  }
};

struct Color
{
  u8 r = 0, g = 0, b = 0, a = 0;

  static constexpr Color from_rgb(u8 r, u8 g, u8 b)
  {
    return Color{.r = r, .g = g, .b = b, .a = 0xff};
  }

  static constexpr Color from_rgba(u8 r, u8 g, u8 b, u8 a)
  {
    return Color{.r = r, .g = g, .b = b, .a = a};
  }

  constexpr Color with_red(u8 nr) const
  {
    return Color{.r = nr, .g = g, .b = b, .a = a};
  }

  constexpr Color with_green(u8 ng) const
  {
    return Color{.r = r, .g = ng, .b = b, .a = a};
  }

  constexpr Color with_blue(u8 nb) const
  {
    return Color{.r = r, .g = g, .b = nb, .a = a};
  }

  constexpr Color with_alpha(u8 na) const
  {
    return Color{.r = r, .g = g, .b = b, .a = na};
  }

  constexpr bool is_transparent() const
  {
    return a == 0;
  }

  constexpr bool is_visible() const
  {
    return !is_transparent();
  }

  constexpr Vec4 to_normalized_vec() const
  {
    return Vec4{
        .x = r / 255.0f, .y = g / 255.0f, .z = b / 255.0f, .w = a / 255.0f};
  }
};

constexpr bool operator==(Color a, Color b)
{
  return a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a;
}

constexpr bool operator!=(Color a, Color b)
{
  return a.r != b.r || a.g != b.g || a.b != b.b || a.a != b.a;
}

template <>
constexpr Color lerp<Color>(Color const &a, Color const &b, f32 t)
{
  return Color{
      .r = static_cast<u8>(std::clamp<f32>(lerp<f32>(a.r, b.r, t), 0, 255)),
      .g = static_cast<u8>(std::clamp<f32>(lerp<f32>(a.g, b.g, t), 0, 255)),
      .b = static_cast<u8>(std::clamp<f32>(lerp<f32>(a.b, b.b, t), 0, 255)),
      .a = static_cast<u8>(std::clamp<f32>(lerp<f32>(a.a, b.a, t), 0, 255))};
}

namespace colors
{

constexpr Color TRANSPARENT = Color::from_rgba(0x00, 0x00, 0x00, 0x00);
constexpr Color WHITE       = Color::from_rgb(0xff, 0xff, 0xff);
constexpr Color BLACK       = Color::from_rgb(0x00, 0x00, 0x00);
constexpr Color RED         = Color::from_rgb(0xff, 0x00, 0x00);
constexpr Color BLUE        = Color::from_rgb(0x00, 0x00, 0xff);
constexpr Color GREEN       = Color::from_rgb(0x00, 0xff, 0x00);
constexpr Color CYAN        = Color::from_rgb(0x00, 0xff, 0xff);
constexpr Color MAGENTA     = Color::from_rgb(0xff, 0x00, 0xff);
constexpr Color YELLOW      = Color::from_rgb(0xff, 0xff, 0x00);

}        // namespace colors

struct TextureRect
{
  Vec2 uv0, uv1;

  constexpr Vec2 top_left() const
  {
    return uv0;
  }

  constexpr Vec2 top_right() const
  {
    return Vec2{uv1.x, uv0.y};
  }

  constexpr Vec2 bottom_right() const
  {
    return uv1;
  }

  constexpr Vec2 bottom_left() const
  {
    return Vec2{uv0.x, uv1.y};
  }
};

/// a 2d shader vertex
struct Vertex2d
{
  Vec2 position;        // point in 2d space
  Vec2 uv;              // texture coordinates
  Vec4 color;        // color of the vertex encoded in the target's color space
};

struct Vertex3d
{
  Vec3 position;        // point in 3d space. NOTE: size is 16 bytes. sames as
                        // Vec4 due to padding
  Vec2 uv;              // texture coordinates
  Vec4 color;        // color of the vertex encoded in the target's color space
};

struct Vertex4d
{
  Vec4 position;        // point in 4d space
  Vec2 uv;              // texture coordinates
  Vec4 color;        // color of the vertex encoded in the target's color space
};

struct EdgeInsets
{
  f32 left = 0, top = 0, right = 0, bottom = 0;

  static constexpr EdgeInsets all(f32 v)
  {
    return EdgeInsets{.left = v, .top = v, .right = v, .bottom = v};
  }

  static constexpr EdgeInsets horizontal(f32 v)
  {
    return EdgeInsets{.left = v, .top = 0, .right = v, .bottom = 0};
  }

  static constexpr EdgeInsets vertical(f32 v)
  {
    return EdgeInsets{.left = 0, .top = v, .right = 0, .bottom = v};
  }

  constexpr f32 y() const
  {
    return top + bottom;
  }

  constexpr f32 x() const
  {
    return left + right;
  }

  constexpr Vec2 xy() const
  {
    return Vec2{x(), y()};
  }

  constexpr Vec2 top_left() const
  {
    return Vec2{left, top};
  }
};

constexpr bool operator==(EdgeInsets const &a, EdgeInsets const &b)
{
  return a.left == b.left && a.top == b.top && a.right == b.right &&
         a.bottom == b.bottom;
}

constexpr bool operator!=(EdgeInsets const &a, EdgeInsets const &b)
{
  return a.left != b.left || a.top != b.top || a.right != b.right ||
         a.bottom != b.bottom;
}

constexpr Vec2 min(Vec2 a, Vec2 b)
{
  return Vec2{std::min(a.x, b.x), std::min(a.y, b.y)};
}

constexpr Vec2 max(Vec2 a, Vec2 b)
{
  return Vec2{std::max(a.x, b.x), std::max(a.y, b.y)};
}

enum class Direction : u8
{
  H = 0,        /// Horizontal
  V = 1         /// Vertical
};

enum class Wrap : u8
{
  None = 0,
  Wrap = 1
};

using Alignment = Vec2;

constexpr Alignment ALIGN_TOP_LEFT      = Vec2{0, 0};
constexpr Alignment ALIGN_TOP_CENTER    = Vec2{0.5f, 0};
constexpr Alignment ALIGN_TOP_RIGHT     = Vec2{1, 0};
constexpr Alignment ALIGN_LEFT_CENTER   = Vec2{0, 0.5f};
constexpr Alignment ALIGN_CENTER        = Vec2{0.5f, 0.5f};
constexpr Alignment ALIGN_RIGHT_CENTER  = Vec2{1, 0.5f};
constexpr Alignment ALIGN_BOTTOM_LEFT   = Vec2{0, 1};
constexpr Alignment ALIGN_BOTTOM_CENTER = Vec2{0.5f, 1};
constexpr Alignment ALIGN_BOTTOM_RIGHT  = Vec2{1, 1};

enum class MainAlign : u8
{
  Start        = 0,
  End          = 1,
  SpaceBetween = 2,
  SpaceAround  = 3,
  SpaceEvenly  = 4
};

enum class CrossAlign : u8
{
  Start  = 0,
  End    = 1,
  Center = 2
};

struct LinearColorGradient
{
  Color begin, end;
  f32   angle = 0;

  constexpr bool is_uniform() const
  {
    return begin == end;
  }

  Color resolve(Vec2 p) const
  {
    f32 const t = p.x * std::cos(ASH_TO_RADIANS(angle)) +
                  p.y * std::sin(ASH_TO_RADIANS(angle));
    return lerp(begin, end, t);
  }
};

struct Version
{
  u8 major = 0, minor = 0, patch = 0;
};

/// @x_mag: The floating-point horizontal magnification of the view. This value
/// MUST NOT be equal to zero. This value SHOULD NOT be negative.
/// @y_mag: The floating-point vertical magnification of the view. This value
/// MUST NOT be equal to zero. This value SHOULD NOT be negative.
/// @z_far: The floating-point distance to the far clipping plane. This value
/// MUST NOT be equal to zero. zfar MUST be greater than znear.
/// @z_near: The floating-point distance to the near clipping plane.
struct OrthographicCamera
{
  f32 x_mag  = 0;
  f32 y_mag  = 0;
  f32 z_far  = 0;
  f32 z_near = 0;
};

/// @aspect_ratio: The floating-point aspect ratio of the field of view.
/// @y_fov: The floating-point vertical field of view in radians. This value
/// SHOULD be less than π.
/// @z_far: The floating-point distance to the far clipping plane.
/// @z_near: The floating-point distance to the near clipping plane.
struct PerspectiveCamera
{
  f32 aspect_ratio = 0;
  f32 y_fov        = 0;
  f32 z_far        = 0;
  f32 z_near       = 0;
};

}        // namespace ash
