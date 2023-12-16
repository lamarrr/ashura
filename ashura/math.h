#pragma once

#include "ashura/algorithms.h"
#include "ashura/cfg.h"
#include "ashura/types.h"
#include <bit>
#include <cmath>

#if ASH_CFG(COMPILER, MSVC)
#  include <intrin.h>
#endif

namespace ash
{
namespace math
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
constexpr T lerp(T const &a, T const &b, Interpolator const &t)
{
  return static_cast<T>(a + (b - a) * t);
}

// find interpolator t, given points a and b, and interpolated value v
template <typename T>
constexpr T unlerp(T const &a, T const &b, T const &v)
{
  return (v - a) / (b - a);
}

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
  return op::max(a >> level, 1U);
}

constexpr Vec2U mip_down(Vec2U a, u32 level)
{
  return Vec2U{op::max(a.x >> level, 1U), op::max(a.y >> level, 1U)};
}

constexpr Vec3U mip_down(Vec3U a, u32 level)
{
  return Vec3U{op::max(a.x >> level, 1U), op::max(a.y >> level, 1U),
               op::max(a.z >> level, 1U)};
}

constexpr Vec4U mip_down(Vec4U a, u32 level)
{
  return Vec4U{op::max(a.x >> level, 1U), op::max(a.y >> level, 1U),
               op::max(a.z >> level, 1U), op::max(a.w >> level, 1U)};
}

inline u32 num_mip_levels(u32 a)
{
  return a == 0 ? 0 : u32log2(a);
}

inline u32 num_mip_levels(Vec2U a)
{
  u32 max = op::max(a.x, a.y);
  return max == 0 ? 0 : (u32log2(max) + 1);
}

inline u32 num_mip_levels(Vec3U a)
{
  u32 max = op::max(op::max(a.x, a.y), a.z);
  return max == 0 ? 0 : (u32log2(max) + 1);
}

inline u32 num_mip_levels(Vec4U a)
{
  u32 max = op::max(op::max(op::max(a.x, a.y), a.z), a.w);
  return max == 0 ? 0 : (u32log2(max) + 1);
}

constexpr f32 dot(Vec2 a, Vec2 b)
{
  return a.x * b.x + a.y * b.y;
}

constexpr i32 dot(Vec2I a, Vec2I b)
{
  return a.x * b.x + a.y * b.y;
}

constexpr f32 dot(Vec3 a, Vec3 b)
{
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

constexpr i32 dot(Vec3I a, Vec3I b)
{
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

constexpr f32 dot(Vec4 a, Vec4 b)
{
  return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

constexpr i32 dot(Vec4I a, Vec4I b)
{
  return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

constexpr f32 cross(Vec2 a, Vec2 b)
{
  return a.x * b.y - b.x * a.y;
}

constexpr i32 cross(Vec2I a, Vec2I b)
{
  return a.x * b.y - b.x * a.y;
}

constexpr Vec3 cross(Vec3 a, Vec3 b)
{
  return Vec3{a.y * b.z - a.z * b.y, -(a.x * b.z - a.z * b.x),
              a.x * b.y - a.y * b.x};
}

constexpr Vec3I cross(Vec3I a, Vec3I b)
{
  return Vec3I{a.y * b.z - a.z * b.y, -(a.x * b.z - a.z * b.x),
               a.x * b.y - a.y * b.x};
}

constexpr Vec2 uniform_vec2(f32 value)
{
  return Vec2{value, value};
}

constexpr Vec3 uniform_vec3(f32 value)
{
  return Vec3{value, value, value};
}

constexpr Vec4 uniform_vec4(f32 value)
{
  return Vec4{value, value, value, value};
}

constexpr Mat2 uniform_mat2(f32 value)
{
  return Mat2{.rows = {{value, value}, {value, value}}};
}

constexpr Mat3 uniform_mat3(f32 value)
{
  return Mat3{.rows = {{value, value, value},
                       {value, value, value},
                       {value, value, value}}};
}

constexpr Mat4 uniform_mat4(f32 value)
{
  return Mat4{.rows = {{value, value, value, value},
                       {value, value, value, value},
                       {value, value, value, value},
                       {value, value, value, value}}};
}

constexpr Mat2 diagonal_mat2(f32 value)
{
  return Mat2{.rows = {{value, 0}, {0, value}}};
}

constexpr Mat3 diagonal_mat3(f32 value)
{
  return Mat3{.rows = {{value, 0, 0}, {0, value, 0}, {0, 0, value}}};
}

constexpr Mat4 diagonal_mat4(f32 value)
{
  return Mat4{.rows = {{value, 0, 0, 0},
                       {0, value, 0, 0},
                       {0, 0, value, 0},
                       {0, 0, 0, value}}};
}

constexpr Mat2 identity_mat2()
{
  return diagonal_mat2(1);
}

constexpr Mat3 identity_mat3()
{
  return diagonal_mat3(1);
}

constexpr Mat4 identity_mat4()
{
  return diagonal_mat4(1);
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

constexpr Vec2 matmul(Mat2 const &a, Vec2 const &b)
{
  return Vec2{dot(a[0], b), dot(a[1], b)};
}

constexpr Mat2 matmul(Mat2 const &a, Mat2 const &b)
{
  return Mat2{.rows = {{dot(a[0], b.x()), dot(a[0], b.y())},
                       {dot(a[1], b.x()), dot(a[1], b.y())}}};
}

constexpr Vec3 matmul(Mat3 const &a, Vec3 const &b)
{
  return Vec3{dot(a[0], b), dot(a[1], b), dot(a[2], b)};
}

constexpr Mat3 matmul(Mat3 const &a, Mat3 const &b)
{
  return Mat3{.rows = {
                  {dot(a[0], b.x()), dot(a[0], b.y()), dot(a[0], b.z())},
                  {dot(a[1], b.x()), dot(a[1], b.y()), dot(a[1], b.z())},
                  {dot(a[2], b.x()), dot(a[2], b.y()), dot(a[2], b.z())},
              }};
}

constexpr Vec3 matmul(Mat3Affine const &a, Vec3 const &b)
{
  return Vec3{dot(a[0], b), dot(a[1], b), dot(Mat3Affine::trailing_row, b)};
}

constexpr Mat3 matmul(Mat3Affine const &a, Mat3 const &b)
{
  return Mat3{.rows = {
                  {dot(a[0], b.x()), dot(a[0], b.y()), dot(a[0], b.z())},
                  {dot(a[1], b.x()), dot(a[1], b.y()), dot(a[1], b.z())},
                  {dot(Mat3Affine::trailing_row, b.x()),
                   dot(Mat3Affine::trailing_row, b.y()),
                   dot(Mat3Affine::trailing_row, b.z())},
              }};
}

constexpr Mat3 matmul(Mat3 const &a, Mat3Affine const &b)
{
  return Mat3{.rows = {{dot(a[0], b.x()), dot(a[0], b.y()), dot(a[0], b.z())},
                       {dot(a[1], b.x()), dot(a[1], b.y()), dot(a[1], b.z())},
                       {dot(a[2], b.x()), dot(a[2], b.y()), dot(a[2], b.z())}}};
}

constexpr Mat3Affine matmul(Mat3Affine const &a, Mat3Affine const &b)
{
  return Mat3Affine{
      .rows = {{dot(a[0], b.x()), dot(a[0], b.y()), dot(a[0], b.z())},
               {dot(a[1], b.x()), dot(a[1], b.y()), dot(a[1], b.z())}}};
}

constexpr Vec4 matmul(Mat4 const &a, Vec4 const &b)
{
  return Vec4{dot(a[0], b), dot(a[1], b), dot(a[2], b), dot(a[3], b)};
}

constexpr Mat4 matmul(Mat4 const &a, Mat4 const &b)
{
  return Mat4{.rows = {{dot(a[0], b.x()), dot(a[0], b.y()), dot(a[0], b.z()),
                        dot(a[0], b.w())},
                       {dot(a[1], b.x()), dot(a[1], b.y()), dot(a[1], b.z()),
                        dot(a[1], b.w())},
                       {dot(a[2], b.x()), dot(a[2], b.y()), dot(a[2], b.z()),
                        dot(a[2], b.w())},
                       {dot(a[3], b.x()), dot(a[3], b.y()), dot(a[3], b.z()),
                        dot(a[3], b.w())}}};
}

constexpr Mat4Affine matmul(Mat4Affine const &a, Vec4 const &b)
{
  return Mat4Affine{.rows = {dot(a[0], b), dot(a[1], b), dot(a[2], b),
                             dot(Mat4Affine::trailing_row, b)}};
}

constexpr Mat4 matmul(Mat4Affine const &a, Mat4 const &b)
{
  return Mat4{.rows = {
                  {dot(a[0], b.x()), dot(a[0], b.y()), dot(a[0], b.z()),
                   dot(a[0], b.w())},
                  {dot(a[1], b.x()), dot(a[1], b.y()), dot(a[1], b.z()),
                   dot(a[1], b.w())},
                  {dot(a[2], b.x()), dot(a[2], b.y()), dot(a[2], b.z()),
                   dot(a[2], b.w())},
                  {dot(Mat4Affine::trailing_row, b.x()),
                   dot(Mat4Affine::trailing_row, b.y()),
                   dot(Mat4Affine::trailing_row, b.z()),
                   dot(Mat4Affine::trailing_row, b.w())},
              }};
}

constexpr Mat4 matmul(Mat4 const &a, Mat4Affine const &b)
{
  return Mat4{.rows = {
                  {dot(a[0], b.x()), dot(a[0], b.y()), dot(a[0], b.z()),
                   dot(a[0], b.w())},
                  {dot(a[1], b.x()), dot(a[1], b.y()), dot(a[1], b.z()),
                   dot(a[1], b.w())},
                  {dot(a[2], b.x()), dot(a[2], b.y()), dot(a[2], b.z()),
                   dot(a[2], b.w())},
                  {dot(a[3], b.x()), dot(a[3], b.y()), dot(a[3], b.z()),
                   dot(a[3], b.w())},
              }};
}

constexpr Mat4Affine matmul(Mat4Affine const &a, Mat4Affine const &b)
{
  return Mat4Affine{.rows = {{dot(a[0], b.x()), dot(a[0], b.y()),
                              dot(a[0], b.z()), dot(a[0], b.w())},
                             {dot(a[1], b.x()), dot(a[1], b.y()),
                              dot(a[1], b.z()), dot(a[1], b.w())},
                             {dot(a[2], b.x()), dot(a[2], b.y()),
                              dot(a[2], b.z()), dot(a[2], b.w())}}};
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
  return uniform_mat2(1.0F / determinant(a)) * adjoint(a);
}

constexpr Mat3 inverse(Mat3 const &a)
{
  return uniform_mat3(1.0F / determinant(a)) * adjoint(a);
}

constexpr Mat4 inverse(Mat4 const &a)
{
  return uniform_mat4(1.0F / determinant(a)) * adjoint(a);
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

}        // namespace math
}        // namespace ash
