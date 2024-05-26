#pragma once
#include "ashura/engine/font.h"
#include "ashura/engine/text.h"
#include "ashura/std/math.h"
#include "ashura/std/types.h"

namespace ash
{

struct Vertex2d
{
  Vec2 position = {0, 0};
  Vec2 uv       = {0, 0};
  Vec4 tint     = {1, 1, 1, 1};
};

struct PathStyle
{
  bool       stroke    = false;
  f32        thickness = 1.0f;
  Vec4       colors[4] = {Vec4{1, 1, 1, 1}, Vec4{1, 1, 1, 1}, Vec4{1, 1, 1, 1},
                          Vec4{1, 1, 1, 1}};
  u32        texture   = 0;
  Font       font      = nullptr;
  u32        font_height    = 16;
  Vec2       uv0            = Vec2{0, 0};
  Vec2       uv1            = Vec2{1, 1};
  Mat4Affine transform      = Mat4Affine::identity();
  Vec2U      scissor_offset = {0, 0};
  Vec2U      scissor_extent = {U32_MAX, U32_MAX};
};

//
// allows joining multiple passes
//
//
// TODO(lamarrr): automatic batching
//
//
// use the passes in the renderer to achieve desired effect
//
// todo(lamarrr): custom shaders?
//
//
struct PathEncoder
{
  virtual void circle(PathStyle const &style, Vec2 center, Vec2 radius) = 0;
  virtual void rect(PathStyle const &style, Vec2 center, Vec2 extent)   = 0;
  virtual void rrect(PathStyle const &style, Vec2 center, Vec2 extent,
                     Vec4 radii)                                        = 0;
  virtual void brect(PathStyle const &style, Vec2 center, Vec2 extent,
                     Vec4 radii)                                        = 0;
  virtual void arc(PathStyle const &style, Vec2 center, f32 radius,
                   f32 angle_begin, f32 angle_end)                      = 0;
  virtual void simple_text(PathStyle const &style, Vec2 baseline,
                           Span<char const> text)                       = 0;
  virtual void text(PathStyle const &style, Vec2 center, TextBlock const &block,
                    TextLayout const &layout)                           = 0;
  virtual void convex_polygon(PathStyle const     &style,
                              Span<Vertex2d const> vertices)            = 0;
  virtual void blur(Vec2U offset, Vec2U extent)                         = 0;
  virtual void flush()                                                  = 0;
};

}        // namespace ash
