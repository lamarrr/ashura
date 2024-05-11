#pragma once
#include "ashura/std/cfg.h"
#include "ashura/std/range.h"
#include "ashura/std/types.h"
#include <bit>
#include <math.h>

#if ASH_CFG(COMPILER, MSVC)
#  include <intrin.h>
#endif

namespace ash
{

template <typename SignedType>
constexpr SignedType abs(SignedType x)
{
  return x > SignedType{} ? x : -x;
}

constexpr bool approx_equal(f32 a, f32 b)
{
  return abs(b - a) <= F32_EPSILON;
}

constexpr bool approx_equal(f64 a, f64 b)
{
  return abs(b - a) <= F64_EPSILON;
}

constexpr f32 epsilon_clamp(f32 x)
{
  return abs(x) > F32_EPSILON ? x : F32_EPSILON;
}

constexpr f32 to_radians(f32 degree)
{
  return PI * degree / 180.0f;
}

constexpr f64 to_radians(f64 degree)
{
  return PI * degree / 180.0;
}

// find intepolated value v, given points a and b, and interpolator t
template <typename T, typename Interpolator>
constexpr T lerp(T const &low, T const &high, Interpolator const &t)
{
  return static_cast<T>(low + (high - low) * t);
}

// find interpolator t, given points a and b, and interpolated value v
template <typename T>
constexpr T unlerp(T const &low, T const &high, T const &value)
{
  return (value - low) / (high - low);
}

template <typename T>
constexpr void relerp(T const &input_low, T const &input_high,
                      T const &output_low, T const &output_high, T &value);

// Undefined behaviour if value is 0
inline u32 u32log2(u32 value)
{
#if ASH_CFG(COMPILER, MSVC)
  unsigned long index;
  _BitScanReverse(&index, value);
  return 31U - index;
#else
#  if defined(__has_builtin) && __has_builtin(__builtin_clz)
  return 31U - __builtin_clz(value);
#  else
  return 31U - std::count_lzero(value);
#  endif
#endif
}

constexpr u32 mip_down(u32 a, u32 level)
{
  return max(a >> level, 1U);
}

constexpr Vec2U mip_down(Vec2U a, u32 level)
{
  return Vec2U{max(a.x >> level, 1U), max(a.y >> level, 1U)};
}

constexpr Vec3U mip_down(Vec3U a, u32 level)
{
  return Vec3U{max(a.x >> level, 1U), max(a.y >> level, 1U),
               max(a.z >> level, 1U)};
}

constexpr Vec4U mip_down(Vec4U a, u32 level)
{
  return Vec4U{max(a.x >> level, 1U), max(a.y >> level, 1U),
               max(a.z >> level, 1U), max(a.w >> level, 1U)};
}

inline u32 num_mip_levels(u32 a)
{
  return a == 0 ? 0 : u32log2(a);
}

inline u32 num_mip_levels(Vec2U a)
{
  u32 max = ash::max(a.x, a.y);
  return max == 0 ? 0 : (u32log2(max) + 1);
}

inline u32 num_mip_levels(Vec3U a)
{
  u32 max = ash::max(ash::max(a.x, a.y), a.z);
  return max == 0 ? 0 : (u32log2(max) + 1);
}

inline u32 num_mip_levels(Vec4U a)
{
  u32 max = ash::max(ash::max(ash::max(a.x, a.y), a.z), a.w);
  return max == 0 ? 0 : (u32log2(max) + 1);
}

constexpr Mat2 transpose(Mat2 const &a)
{
  return Mat2{.rows = {a.x(), a.y()}};
}

constexpr Mat3 transpose(Mat3 const &a)
{
  return Mat3{.rows = {a.x(), a.y(), a.z()}};
}

constexpr Mat4 transpose(Mat4 const &a)
{
  return Mat4{.rows = {a.x(), a.y(), a.z(), a.w()}};
}

constexpr f32 determinant(Mat2 const &a)
{
  return a[0].x * a[1].y - a[1].x * a[0].y;
}

constexpr f32 determinant(Mat3 const &a)
{
  return a[0].x * a[1].y * a[2].z - a[0].x * a[1].z * a[2].y -
         a[0].y * a[1].x * a[2].z + a[0].y * a[1].z * a[2].x +
         a[0].z * a[1].x * a[2].y - a[0].z * a[1].y * a[2].x;
}

constexpr f32 determinant(Mat4 const &a)
{
  return a[0].x * (a[1].y * a[2].z * a[3].w + a[1].z * a[2].w * a[3].y +
                   a[1].w * a[2].y * a[3].z - a[1].w * a[2].z * a[3].y -
                   a[1].z * a[2].y * a[3].w - a[1].y * a[2].w * a[3].z) -
         a[1].x * (a[0].y * a[2].z * a[3].w + a[0].z * a[2].w * a[3].y +
                   a[0].w * a[2].y * a[3].z - a[0].w * a[2].z * a[3].y -
                   a[0].z * a[2].y * a[3].w - a[0].y * a[2].w * a[3].z) +
         a[2].x * (a[0].y * a[1].z * a[3].w + a[0].z * a[1].w * a[3].y +
                   a[0].w * a[1].y * a[3].z - a[0].w * a[1].z * a[3].y -
                   a[0].z * a[1].y * a[3].w - a[0].y * a[1].w * a[3].z) -
         a[3].x * (a[0].y * a[1].z * a[2].w + a[0].z * a[1].w * a[2].y +
                   a[0].w * a[1].y * a[2].z - a[0].w * a[1].z * a[2].y -
                   a[0].z * a[1].y * a[2].w - a[0].y * a[1].w * a[2].z);
}

constexpr Mat2 adjoint(Mat2 const &a)
{
  return Mat2{.rows = {{a[1].y, -a[0].y}, {-a[1].x, a[0].x}}};
}

constexpr Mat3 adjoint(Mat3 const &a)
{
  return Mat3{
      .rows = {
          {a[1].y * a[2].z - a[1].z * a[2].y, a[0].z * a[2].y - a[0].y * a[2].z,
           a[0].y * a[1].z - a[0].z * a[1].y},
          {a[1].z * a[2].x - a[1].x * a[2].z, a[0].x * a[2].z - a[0].z * a[2].x,
           a[0].z * a[1].x - a[0].x * a[1].z},
          {a[1].x * a[2].y - a[1].y * a[2].x, a[0].y * a[2].x - a[0].x * a[2].y,
           a[0].x * a[1].y - a[0].y * a[1].x}}};
}

constexpr Mat4 adjoint(Mat4 const &a)
{
  Mat4 r;
  r[0].x = a[1].y * a[2].z * a[3].w + a[1].z * a[2].w * a[3].y +
           a[1].w * a[2].y * a[3].z - a[1].w * a[2].z * a[3].y -
           a[1].z * a[2].y * a[3].w - a[1].y * a[2].w * a[3].z;
  r[0].y = -a[0].y * a[2].z * a[3].w - a[0].z * a[2].w * a[3].y -
           a[0].w * a[2].y * a[3].z + a[0].w * a[2].z * a[3].y +
           a[0].z * a[2].y * a[3].w + a[0].y * a[2].w * a[3].z;
  r[0].z = a[0].y * a[1].z * a[3].w + a[0].z * a[1].w * a[3].y +
           a[0].w * a[1].y * a[3].z - a[0].w * a[1].z * a[3].y -
           a[0].z * a[1].y * a[3].w - a[0].y * a[1].w * a[3].z;
  r[0].w = -a[0].y * a[1].z * a[2].w - a[0].z * a[1].w * a[2].y -
           a[0].w * a[1].y * a[2].z + a[0].w * a[1].z * a[2].y +
           a[0].z * a[1].y * a[2].w + a[0].y * a[1].w * a[2].z;
  r[1].x = -a[1].x * a[2].z * a[3].w - a[1].z * a[2].w * a[3].x -
           a[1].w * a[2].x * a[3].z + a[1].w * a[2].z * a[3].x +
           a[1].z * a[2].x * a[3].w + a[1].x * a[2].w * a[3].z;
  r[1].y = a[0].x * a[2].z * a[3].w + a[0].z * a[2].w * a[3].x +
           a[0].w * a[2].x * a[3].z - a[0].w * a[2].z * a[3].x -
           a[0].z * a[2].x * a[3].w - a[0].x * a[2].w * a[3].z;
  r[1].z = -a[0].x * a[1].z * a[3].w - a[0].z * a[1].w * a[3].x -
           a[0].w * a[1].x * a[3].z + a[0].w * a[1].z * a[3].x +
           a[0].z * a[1].x * a[3].w + a[0].x * a[1].w * a[3].z;
  r[1].w = a[0].x * a[1].z * a[2].w + a[0].z * a[1].w * a[2].x +
           a[0].w * a[1].x * a[2].z - a[0].w * a[1].z * a[2].x -
           a[0].z * a[1].x * a[2].w - a[0].x * a[1].w * a[2].z;
  r[2].x = a[1].x * a[2].y * a[3].w + a[1].y * a[2].w * a[3].x +
           a[1].w * a[2].x * a[3].y - a[1].w * a[2].y * a[3].x -
           a[1].y * a[2].y * a[3].w - a[1].x * a[2].w * a[3].y;
  r[2].y = -a[0].x * a[2].y * a[3].w - a[0].y * a[2].w * a[3].x -
           a[0].w * a[2].x * a[3].y + a[0].w * a[2].y * a[3].x +
           a[0].y * a[2].x * a[3].w + a[0].x * a[2].w * a[3].y;
  r[2].z = a[0].x * a[1].y * a[3].w + a[0].y * a[1].w * a[3].x +
           a[0].w * a[1].x * a[3].y - a[0].w * a[1].y * a[3].x -
           a[0].y * a[1].x * a[3].w - a[0].x * a[1].w * a[3].y;
  r[2].w = -a[0].x * a[1].y * a[2].w - a[0].y * a[1].w * a[2].x -
           a[0].w * a[1].x * a[2].y + a[0].w * a[1].y * a[2].x +
           a[0].y * a[1].x * a[2].w + a[0].x * a[1].w * a[2].y;
  r[3].x = -a[1].x * a[2].y * a[3].z - a[1].y * a[2].z * a[3].x -
           a[1].z * a[2].x * a[3].y + a[1].z * a[2].y * a[3].x +
           a[1].y * a[2].x * a[3].z + a[1].x * a[2].z * a[3].y;
  r[3].y = a[0].x * a[2].y * a[3].z + a[0].y * a[2].z * a[3].x +
           a[0].z * a[2].x * a[3].y - a[0].z * a[2].y * a[3].x -
           a[0].y * a[2].x * a[3].z - a[0].x * a[2].z * a[3].y;
  r[3].z = -a[0].x * a[1].y * a[3].z - a[0].y * a[1].z * a[3].x -
           a[0].z * a[1].x * a[3].y + a[0].z * a[1].y * a[3].x +
           a[0].y * a[1].x * a[3].z + a[0].x * a[1].z * a[3].y;
  r[3].w = a[0].x * a[1].y * a[2].z + a[0].y * a[1].z * a[2].y +
           a[0].z * a[1].x * a[2].y - a[0].z * a[1].y * a[2].x -
           a[0].y * a[1].x * a[2].z - a[0].x * a[1].z * a[2].y;
  return r;
}

constexpr Mat2 inverse(Mat2 a)
{
  return Mat2::uniform(1.0F / determinant(a)) * adjoint(a);
}

constexpr Mat3 inverse(Mat3 const &a)
{
  return Mat3::uniform(1.0F / determinant(a)) * adjoint(a);
}

constexpr Mat4 inverse(Mat4 const &a)
{
  return Mat4::uniform(1.0F / determinant(a)) * adjoint(a);
}

constexpr Mat3 translate2d(Vec2 t)
{
  return Mat3{.rows = {{1, 0, t.x}, {0, 1, t.y}, Mat3Affine::trailing_row}};
}

constexpr Mat3Affine affine_translate2d(Vec2 t)
{
  return Mat3Affine{.rows = {{1, 0, t.x}, {0, 1, t.y}}};
}

constexpr Mat4 translate3d(Vec3 t)
{
  return Mat4{.rows = {{1, 0, 0, t.x},
                       {0, 1, 0, t.y},
                       {0, 0, 1, t.z},
                       Mat4Affine::trailing_row}};
}

constexpr Mat4Affine affine_translate3d(Vec3 t)
{
  return Mat4Affine{.rows = {{1, 0, 0, t.x}, {0, 1, 0, t.y}, {0, 0, 1, t.z}}};
}

constexpr Mat3 scale2d(Vec2 s)
{
  return Mat3{.rows = {{s.x, 0, 0}, {0, s.y, 0}, Mat3Affine::trailing_row}};
}

constexpr Mat4 scale3d(Vec3 s)
{
  return Mat4{.rows = {{s.x, 0, 0, 0},
                       {0, s.y, 0, 0},
                       {0, 0, s.z, 0},
                       Mat4Affine::trailing_row}};
}

constexpr Mat3Affine affine_scale2d(Vec2 s)
{
  return Mat3Affine{.rows = {{s.x, 0, 0}, {0, s.y, 0}}};
}

constexpr Mat4Affine affine_scale3d(Vec3 s)
{
  return Mat4Affine{.rows = {{s.x, 0, 0, 0}, {0, s.y, 0, 0}, {0, 0, s.z, 0}}};
}

inline Mat3 rotate2d(f32 radians)
{
  return Mat3{.rows = {{cosf(radians), -sinf(radians), 0},
                       {sinf(radians), cosf(radians), 0},
                       Mat3Affine::trailing_row}};
}

inline Mat3Affine affine_rotate2d(f32 radians)
{
  return Mat3Affine{.rows = {{cosf(radians), -sinf(radians), 0},
                             {sinf(radians), cosf(radians), 0}}};
}

inline Mat4 rotate3d_x(f32 radians)
{
  return Mat4{.rows = {{1, 0, 0, 0},
                       {0, cosf(radians), -sinf(radians), 0},
                       {0, sinf(radians), cosf(radians), 0},
                       Mat4Affine::trailing_row}};
}

inline Mat4Affine affine_rotate3d_x(f32 radians)
{
  return Mat4Affine{.rows = {{1, 0, 0, 0},
                             {0, cosf(radians), -sinf(radians), 0},
                             {0, sinf(radians), cosf(radians), 0}}};
}

inline Mat4 rotate3d_y(f32 radians)
{
  return Mat4{.rows = {{cosf(radians), 0, sinf(radians), 0},
                       {0, 1, 0, 0},
                       {-sinf(radians), 0, cosf(radians), 0},
                       Mat4Affine::trailing_row}};
}

inline Mat4Affine affine_rotate3d_y(f32 radians)
{
  return Mat4Affine{.rows = {{cosf(radians), 0, sinf(radians), 0},
                             {0, 1, 0, 0},
                             {-sinf(radians), 0, cosf(radians), 0}}};
}

inline Mat4 rotate3d_z(f32 radians)
{
  return Mat4{.rows = {{cosf(radians), -sinf(radians), 0, 0},
                       {sinf(radians), cosf(radians), 0, 0},
                       {0, 0, 1, 0},
                       Mat4Affine::trailing_row}};
}

inline Mat4Affine affine_rotate3d_z(f32 radians)
{
  return Mat4Affine{.rows = {{cosf(radians), -sinf(radians), 0, 0},
                             {sinf(radians), cosf(radians), 0, 0},
                             {0, 0, 1, 0}}};
}

constexpr Vec2 transform(Mat3 const &t, Vec2 value)
{
  Vec3 v = t * Vec3{value.x, value.y, 1};
  return Vec2{v.x, v.y};
}

constexpr Vec2 transform(Mat3Affine const &t, Vec2 value)
{
  Vec3 v = t * Vec3{value.x, value.y, 1};
  return Vec2{v.x, v.y};
}

constexpr Vec3 transform(Mat4 const &t, Vec3 value)
{
  Vec4 v = t * Vec4{value.x, value.y, value.z, 1};
  return Vec3{v.x, v.y, v.z};
}

constexpr Vec3 transform(Mat4Affine const &t, Vec3 value)
{
  Vec4 v = t * Vec4{value.x, value.y, 1};
  return Vec3{v.x, v.y, v.z};
}

// TODO(lamarrr): is this correct? for transformed vertices
constexpr bool overlaps(Vec2 a_begin, Vec2 a_end, Vec2 b_begin, Vec2 b_end)
{
  return a_begin.x <= b_end.x && a_end.x >= b_begin.x && a_begin.y <= b_end.y &&
         a_end.y >= b_begin.y;
}

// quad overlaps

constexpr bool rect_contains_point(Vec2 begin, Vec2 end, Vec2 point)
{
  return begin.x <= point.x && begin.y <= point.y && end.x >= point.x &&
         end.y >= point.y;
}

constexpr void rect_intersect(Vec2 a_begin, Vec2 a_end, Vec2 &b_begin,
                              Vec2 &b_end)
{
  if (!overlaps(a_begin, a_end, b_begin, b_end))
  {
    b_begin = {};
    b_end   = {};
    return;
  }

  Vec2 intersect_begin{max(a_begin.x, b_begin.x), max(a_begin.y, b_begin.y)};
  Vec2 intersect_end{min(a_end.x, b_end.x), min(a_end.y, b_end.y)};

  b_begin = intersect_begin;
  b_end   = intersect_end;
}

struct Rect
{
  Vec2 offset;
  Vec2 extent;

  constexpr Vec2 center() const
  {
    return offset + (extent / 2);
  }

  constexpr Vec2 end() const
  {
    return offset + extent;
  }

  constexpr f32 area() const
  {
    return extent.x * extent.y;
  }

  constexpr bool contains(Vec2 point) const
  {
    return offset.x <= point.x && offset.y <= point.y &&
           (offset.x + extent.x) >= point.x && (offset.y + extent.y) >= point.y;
  }

  constexpr bool overlaps(Rect const &other) const
  {
    Vec2 a_begin = offset;
    Vec2 a_end   = offset + extent;
    Vec2 b_begin = other.offset;
    Vec2 b_end   = other.offset + other.extent;
    return a_begin.x <= b_end.x && a_end.x >= b_begin.x &&
           a_begin.y <= b_end.y && a_end.y >= b_begin.y;
  }

  constexpr Rect intersect(Rect const &other) const
  {
    if (!overlaps(other))
    {
      return Rect{offset, {0, 0}};
    }

    Vec2 a_begin = offset;
    Vec2 a_end   = offset + extent;
    Vec2 b_begin = other.offset;
    Vec2 b_end   = other.offset + other.extent;
    Vec2 int_begin{max(a_begin.x, b_begin.x), max(a_begin.y, b_begin.y)};
    Vec2 int_end{min(a_end.x, b_end.x), min(a_end.y, b_end.y)};

    return Rect{int_begin, int_end - int_begin};
  }
};

struct Box
{
  Vec3 offset;
  Vec3 extent;

  constexpr Vec3 center() const
  {
    return offset + (extent / 2);
  }

  constexpr Vec3 end() const
  {
    return offset + extent;
  }

  constexpr f32 volume() const
  {
    return extent.x * extent.y * extent.z;
  }

  constexpr bool contains(Vec3 point) const
  {
    return offset.x <= point.x && offset.y <= point.y && offset.z <= point.z &&
           (offset.x + extent.x) >= point.x &&
           (offset.y + extent.y) >= point.y && (offset.z + extent.z) >= point.z;
    return true;
  }

  constexpr bool overlaps(Box const &other) const
  {
    Vec3 a_begin = offset;
    Vec3 a_end   = offset + extent;
    Vec3 b_begin = other.offset;
    Vec3 b_end   = other.offset + other.extent;
    return a_begin.x <= b_end.x && a_end.x >= b_begin.x &&
           a_begin.y <= b_end.y && a_end.y >= b_begin.y &&
           a_begin.z <= b_end.z && a_end.z >= b_begin.z;
  }
};

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
 *guaranteed to pass through the control points and are easily chained
 *together. Equation supports abitrary parameterization. eg. Uniform=0,1,2,3 ;
 *chordal= |Pn - Pn-1| ; centripetal = |Pn - Pn-1|^0.5 P0 - The control point
 *preceding the interpolation range. P1 - The control point starting the
 *interpolation range. P2 - The control point ending the interpolation range.
 *P3 - The control point following the interpolation range. T0-3 - The
 *interpolation parameters for the corresponding control points. T - The
 *interpolation factor in the range 0 to 1. 0 returns P1. 1 returns P2.
 */
// template< class U >
// UE_NODISCARD static constexpr FORCEINLINE_DEBUGGABLE U
// CubicCRSplineInterp(const U& P0, const U& P1, const U& P2, const U& P3,
// const float T0, const float T1, const float T2, const float T3, const float
// T)
// {
// 	//Based on
// http://www.cemyuksel.com/research/catmullrom_param/catmullrom.pdf 	float
// InvT1MinusT0 = 1.0f / (T1 - T0); 	U L01 = ( P0 * ((T1 - T) * InvT1MinusT0)
// )
// + ( P1 * ((T - T0) * InvT1MinusT0) ); 	float InvT2MinusT1 = 1.0f / (T2 -
// T1); 	U L12 = ( P1 *
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
}        // namespace ash
