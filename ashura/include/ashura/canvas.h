#pragma once

#include <algorithm>
#include <cinttypes>
#include <cmath>
#include <utility>

#include "ashura/font.h"
#include "ashura/image.h"
#include "ashura/pipeline.h"
#include "ashura/primitives.h"
#include "ashura/text.h"
#include "stx/text.h"
#include "stx/vec.h"

namespace ash
{

constexpr u32 NIMAGES_PER_DRAWCALL = 8;
constexpr u32 PUSH_CONSTANT_SIZE   = 128;

static_assert(PUSH_CONSTANT_SIZE % 4 == 0);

namespace gfx
{

enum class TextRenderStyleWrap : u8
{
  None,
  Letter,
  Word,
  Line
};
// https://fossheim.io/writing/posts/css-text-gradient/
struct TextRenderStyle
{
  LinearColorGradient color_gradient;
  TextRenderStyleWrap wrap = TextRenderStyleWrap::None;
};

namespace paths
{

inline stx::Span<Vertex2d> Rect(Vec2 offset, Vec2 extent, Vec4 color, stx::Span<Vertex2d> polygon)
{
  Vertex2d const vertices[] = {{.position = offset, .uv = {}, .color = color},
                               {.position = offset + Vec2{extent.x, 0}, .uv = {}, .color = color},
                               {.position = offset + extent, .uv = {}, .color = color},
                               {.position = offset + Vec2{0, extent.y}, .uv = {}, .color = color}};

  return polygon.copy(vertices);
}

inline stx::Span<Vertex2d> arc(Vec2 offset, f32 radius, f32 begin, f32 end, u32 nsegments, Vec4 color, stx::Span<Vertex2d> polygon)
{
  begin = ASH_TO_RADIANS(begin);
  end   = ASH_TO_RADIANS(end);

  if (nsegments < 1 || radius <= 0)
  {
    return {};
  }

  for (u32 i = 0; i < nsegments; i++)
  {
    f32  angle = lerp(begin, end, AS(f32, i / (nsegments - 1)));
    Vec2 p     = radius + radius * Vec2{std::cos(angle), std::sin(angle)};
    polygon[i] = Vertex2d{.position = offset + p, .uv = {}, .color = color};
  }

  return polygon;
}

inline stx::Span<Vertex2d> circle(Vec2 offset, f32 radius, u32 nsegments, Vec4 color, stx::Span<Vertex2d> polygon)
{
  if (nsegments == 0 || radius <= 0)
  {
    return {};
  }

  f32 step = (2 * PI) / nsegments;

  for (u32 i = 0; i < nsegments; i++)
  {
    Vec2 p     = radius + radius * Vec2{std::cos(i * step), std::sin(i * step)};
    polygon[i] = Vertex2d{.position = offset + p, .uv = {}, .color = color};
  }

  return polygon;
}

inline stx::Span<Vertex2d> ellipse(Vec2 offset, Vec2 radii, u32 nsegments, Vec4 color, stx::Span<Vertex2d> polygon)
{
  if (nsegments == 0 || radii.x <= 0 || radii.y <= 0)
  {
    return {};
  }

  f32 step = (2 * PI) / nsegments;

  for (u32 i = 0; i < nsegments; i++)
  {
    Vec2 p     = radii + radii * Vec2{std::cos(i * step), std::sin(i * step)};
    polygon[i] = Vertex2d{.position = offset + p, .uv = {}, .color = color};
  }

  return polygon;
}

// outputs 8 + nsegments * 4 vertices
inline stx::Span<Vertex2d> round_rect(Vec2 offset, Vec2 extent, Vec4 radii, u32 nsegments, Vec4 color, stx::Span<Vertex2d> polygon)
{
  f32 max_radius   = std::min(extent.x, extent.y);
  radii.x          = std::min(radii.x, max_radius);
  radii.y          = std::min(radii.y, max_radius - radii.x);
  f32 max_radius_z = std::min(max_radius - radii.x, max_radius - radii.y);
  radii.z          = std::min(radii.z, max_radius_z);
  f32 max_radius_w = std::min(max_radius_z, max_radius - radii.z);
  radii.w          = std::min(radii.w, max_radius_w);

  f32 step = nsegments == 0 ? 0.0f : (PI / 2) / nsegments;

  u32 i = 0;

  polygon[i] = Vertex2d{.position = offset + extent - Vec2{0, radii.z}, .uv = {}, .color = color};
  i++;

  for (u32 segment = 0; segment < nsegments; segment++, i++)
  {
    Vec2 p     = (extent - radii.z) + radii.z * Vec2{std::cos(segment * step), std::sin(segment * step)};
    polygon[i] = Vertex2d{.position = offset + p, .uv = {}, .color = color};
  }

  polygon[i] = Vertex2d{.position = offset + extent - Vec2{radii.z, 0}, .uv = {}, .color = color};
  i++;

  polygon[i] = Vertex2d{.position = offset + Vec2{radii.w, extent.y}, .uv = {}, .color = color};
  i++;

  for (u32 segment = 0; segment < nsegments; segment++, i++)
  {
    Vec2 p     = Vec2{radii.w, extent.y - radii.w} + radii.w * Vec2{std::cos(PI / 2 + segment * step), std::sin(PI / 2 + segment * step)};
    polygon[i] = Vertex2d{.position = offset + p, .uv = {}, .color = color};
  }

  polygon[i] = Vertex2d{.position = offset + Vec2{0, extent.y - radii.w}, .uv = {}, .color = color};
  i++;

  polygon[i] = Vertex2d{.position = offset + Vec2{0, radii.x}, .uv = {}, .color = color};
  i++;

  for (u32 segment = 0; segment < nsegments; segment++, i++)
  {
    Vec2 p     = radii.x + radii.x * Vec2{std::cos(PI + segment * step), std::sin(PI + segment * step)};
    polygon[i] = Vertex2d{.position = offset + p, .uv = {}, .color = color};
  }

  polygon[i] = Vertex2d{.position = offset + Vec2{radii.x, 0}, .uv = {}, .color = color};
  i++;

  polygon[i] = Vertex2d{.position = offset + Vec2{extent.x - radii.y, 0}, .uv = {}, .color = color};
  i++;

  for (u32 segment = 0; segment < nsegments; segment++, i++)
  {
    Vec2 p     = Vec2{extent.x - radii.y, radii.y} + radii.y * Vec2{std::cos(PI * 3.0f / 2.0f + segment * step), std::sin(PI * 3.0f / 2.0f + segment * step)};
    polygon[i] = Vertex2d{.position = offset + p, .uv = {}, .color = color};
  }

  polygon[i] = Vertex2d{.position = offset + Vec2{extent.x, radii.y}, .uv = {}, .color = color};

  return polygon;
}

inline stx::Span<Vertex2d> bevel_rect(Vec2 offset, Vec2 extent, Vec4 radii, Vec4 color, stx::Span<Vertex2d> polygon)
{
  f32 max_radius   = std::min(extent.x, extent.y);
  radii.x          = std::min(radii.x, max_radius);
  radii.y          = std::min(radii.y, max_radius - radii.x);
  f32 max_radius_z = std::min(max_radius - radii.x, max_radius - radii.y);
  radii.z          = std::min(radii.z, max_radius_z);
  f32 max_radius_w = std::min(max_radius_z, max_radius - radii.z);
  radii.w          = std::min(radii.w, max_radius_w);

  Vertex2d const vertices[] = {{.position = offset + Vec2{radii.x, 0}, .uv = {}, .color = color},
                               {.position = offset + Vec2{extent.x - radii.y, 0}, .uv = {}, .color = color},
                               {.position = offset + Vec2{extent.x, radii.y}, .uv = {}, .color = color},
                               {.position = offset + Vec2{extent.x, extent.y - radii.z}, .uv = {}, .color = color},
                               {.position = offset + Vec2{extent.x - radii.z, extent.y}, .uv = {}, .color = color},
                               {.position = offset + Vec2{radii.w, extent.y}, .uv = {}, .color = color},
                               {.position = offset + Vec2{0, extent.y - radii.w}, .uv = {}, .color = color},
                               {.position = offset + Vec2{0, radii.x}, .uv = {}, .color = color}};

  return polygon.copy(vertices);
}

inline stx::Span<Vertex2d> lerp_uvs(stx::Span<Vertex2d> path, Vec2 extent, TextureRect texture_region)
{
  for (Vertex2d &v : path)
  {
    Vec2 t = v.position / epsilon_clamp(extent);
    v.uv.x = lerp(texture_region.uv0.x, texture_region.uv1.x, t.x);
    v.uv.y = lerp(texture_region.uv0.y, texture_region.uv1.y, t.y);
  }

  return path;
}

inline stx::Span<Vertex2d> lerp_color_gradient(stx::Span<Vertex2d> path, Vec2 extent, LinearColorGradient gradient)
{
  if (gradient.is_uniform())
  {
    return path;
  }

  f32 const x = std::cos(ASH_TO_RADIANS(gradient.angle));
  f32 const y = std::sin(ASH_TO_RADIANS(gradient.angle));

  for (Vertex2d &v : path)
  {
    Vec2 const p = v.position / extent;
    f32 const  t = p.x * x + p.y * y;
    v.color      = lerp(gradient.begin, gradient.end, t).to_normalized_vec();
  }

  return path;
}

}        // namespace paths

/// outputs (n-2)*3 indices
inline void triangulate_convex_polygon(stx::Vec<u32> &indices, u32 nvertices)
{
  if (nvertices < 3)
  {
    return;
  }

  for (u32 i = 2; i < nvertices; i++)
  {
    indices.push(0).unwrap();
    indices.push_inplace((i - 1)).unwrap();
    indices.push_inplace(i).unwrap();
  }
}

/// line joint is a bevel joint
inline void add_line_stroke(Vec2 p0, Vec2 p1, f32 thickness, Vec4 color, stx::Vec<Vertex2d> &out)
{
  // the angles are specified in clockwise direction to be compatible with the
  // vulkan coordinate system
  //
  // get the angle of inclination of p2 to p1
  Vec2 d     = p1 - p0;
  f32  grad  = abs(d.y / epsilon_clamp(d.x));
  f32  alpha = std::atan(grad);

  // use direction of the points to get the actual overall angle of
  // inclination of p2 to p1
  if (d.x < 0 && d.y > 0)
  {
    alpha = PI - alpha;
  }
  else if (d.x < 0 && d.y < 0)
  {
    alpha = PI + alpha;
  }
  else if (d.x > 0 && d.y < 0)
  {
    alpha = 2 * PI - alpha;
  }
  else
  {
    // d.x >=0 && d.y >= 0
  }

  // line will be at a parallel angle
  alpha = alpha + PI / 2;

  Vec2 f = thickness / 2 * Vec2{std::cos(alpha), std::sin(alpha)};
  Vec2 g = thickness / 2 * Vec2{std::cos(PI + alpha), std::sin(PI + alpha)};

  Vec2 p0_0 = p0 + f;
  Vec2 p0_1 = p0 + g;

  Vec2 p1_0 = p1 + f;
  Vec2 p1_1 = p1 + g;

  Vertex2d vertices[] = {{.position = p0_0, .uv = {}, .color = color},
                         {.position = p0_1, .uv = {}, .color = color},
                         {.position = p1_0, .uv = {}, .color = color},
                         {.position = p1_1, .uv = {}, .color = color}};

  out.extend(vertices).unwrap();
}

// line joint is a bevel joint, it is the most efficient since it re-uses existing vertices and doesn't require generating new vertices
inline void triangulate_line(stx::Span<Vertex2d const> in_points, f32 thickness, stx::Vec<Vertex2d> &out_vertices, stx::Vec<u32> &out_indices, bool should_close)
{
  if (in_points.size() < 2 || thickness == 0)
  {
    return;
  }

  bool has_previous_line = false;

  u32 Vertex_index = 0;

  for (u32 i = 1; i < AS(u32, in_points.size()); i++)
  {
    Vec4 color = in_points[i - 1].color;
    Vec2 p0    = in_points[i - 1].position;
    Vec2 p1    = in_points[i].position;

    add_line_stroke(p0, p1, thickness, color, out_vertices);

    // weave the line triangles
    u32 indices[] = {Vertex_index, Vertex_index + 1, Vertex_index + 3, Vertex_index, Vertex_index + 2, Vertex_index + 3};

    out_indices.extend(indices).unwrap();

    // weave the previous line's end to the beginning of this line
    if (has_previous_line)
    {
      u32 prev_line_Vertex_index = Vertex_index - 4;

      u32 indices[] = {prev_line_Vertex_index + 2, prev_line_Vertex_index + 3, Vertex_index,
                       prev_line_Vertex_index + 2, prev_line_Vertex_index + 3, Vertex_index + 1};

      out_indices.extend(indices).unwrap();
    }

    has_previous_line = true;

    Vertex_index += 4;
  }

  // requires at least 3 points to be closable
  if (should_close && in_points.size() > 2)
  {
    Vec4 color = in_points[in_points.size() - 1].color;
    Vec2 p0    = in_points[in_points.size() - 1].position;
    Vec2 p1    = in_points[0].position;

    add_line_stroke(p0, p1, thickness, color, out_vertices);

    // weave the line triangles
    u32 indices[] = {Vertex_index, Vertex_index + 1, Vertex_index + 3, Vertex_index, Vertex_index + 2, Vertex_index + 3};

    out_indices.extend(indices).unwrap();

    {
      u32 prev_line_Vertex_index  = Vertex_index - 4;
      u32 first_line_Vertex_index = 0;

      u32 indices[] = {
          // weave the previous line's end to the beginning of this line
          prev_line_Vertex_index + 2, prev_line_Vertex_index + 3, Vertex_index,
          prev_line_Vertex_index + 2, prev_line_Vertex_index + 3, Vertex_index + 1,
          // weave this line's end to the beginning of the first line
          Vertex_index + 2, Vertex_index + 3, first_line_Vertex_index,
          Vertex_index + 2, Vertex_index + 3, first_line_Vertex_index + 1};

      out_indices.extend(indices).unwrap();
    }
  }
}

struct DrawCommand
{
  std::string_view pipeline;                                      /// ID of pipeline to use for rendering
  u32              nvertices      = 0;                            /// number of vertices for this draw call. offset is automatically determined
  u32              nindices       = 0;                            /// number of indices for this draw call. offset is automatically determined
  u32              first_instance = 0;                            /// first instance used for instanced rendering
  u32              ninstances     = 1;                            /// number of instances used for instanced rendering
  Rect             scissor;                                       /// determines visible area of the rendering operation, in framebuffer coordinates (0, 0) -> viewport_extent
  image            textures[NIMAGES_PER_DRAWCALL]    = {};        /// textures bounded to each descriptor set, 8-max
  u8               push_constant[PUSH_CONSTANT_SIZE] = {};        /// push constant used for draw call. maximum size of PUSH_CONSTANT_SIZE bytes

  template <typename T>
  DrawCommand with_push_constant(T const &constant)
  {
    static_assert(sizeof(T) <= PUSH_CONSTANT_SIZE);
    DrawCommand copy{*this};
    stx::Span{copy.push_constant}.as_u8().copy(stx::Span{&constant, 1}.as_u8());
    return copy;
  }
};

struct DrawList
{
  stx::Vec<Vertex2d>    vertices;
  stx::Vec<u32>         indices;
  stx::Vec<DrawCommand> commands;

  void clear()
  {
    vertices.clear();
    indices.clear();
    commands.clear();
  }
};

struct CanvasState
{
  Mat3 local_transform;         // local object transform, applies to local coordinates of the objects
  Mat3 global_transform;        // global scene transform, applies to the global coordinate of the objects
  Rect scissor;
};

/// Coordinates are specified in top-left origin absolute pixel coordinates with x pointing to the
/// right and y pointing downwards (i.e. {0, 0} being top left and {x, y} being bottom right),
/// the transform matrix transforms the vertices to a Vulkan Coordinate System (i.e. {-1, -1} top left and {1, 1} bottom right).
///
/// LIMITATIONS:
/// - each draw call must not have more than 2^32 vertices and indices, otherwise split them up
/// - the canvas must not have more than 2^32 draw calls
///
/// NOTES:
/// - some graphics frameworks, i.e. vulkan only allow u32 indices so we have to split up the draw calls across multiple draw list batches
/// - the canvas doesn't manage the lifetime of the handed over resources or images
///
struct Canvas
{
  Vec2                  viewport_extent;
  CanvasState           state;
  stx::Vec<CanvasState> state_stack;
  DrawList              draw_list;
  stx::Vec<Vertex2d>    scratch;        // scratch/temporary buffer for storing generating vertices before storing in the draw list

  bool viewport_contains(Rect area) const
  {
    // TODO(lamarrr): check for scissor
    return Rect{.offset = {}, .extent = viewport_extent}
        .overlaps(transform2d(state.global_transform * state.local_transform, area));
  }

  Canvas &restart(Vec2 viewport_extent)
  {
    this->viewport_extent = viewport_extent;
    state                 = CanvasState{.local_transform  = Mat3::identity(),
                                        .global_transform = Mat3::identity(),
                                        .scissor          = Rect{.offset = {0, 0}, .extent = viewport_extent}};
    state_stack.clear();
    draw_list.clear();
    return *this;
  }

  Mat3 make_transform(Vec2 position) const
  {
    Vec2 viewport_extent_clamped = epsilon_clamp(viewport_extent);

    Mat3 t = state.local_transform;                           /// apply local coordinate transform
    t      = translate2d(position) * t;                       /// apply positioning
    t      = state.global_transform * t;                      /// apply global coordinate transform
    t      = scale2d(2 / viewport_extent_clamped) * t;        /// normalize to 0 to 2 coordinate range
    t      = translate2d(-1, -1) * t;                         /// normalize from [0, 2] to vulkan viewport coordinate range [-1, 1]
    return t;
  }

  /// push state (transform and scissor) on state stack
  Canvas &save()
  {
    state_stack.push_inplace(state).unwrap();
    return *this;
  }

  /// pop state (transform and scissor) stack and restore state
  Canvas &restore()
  {
    state = state_stack.pop().unwrap_or(CanvasState{.local_transform = Mat3::identity(), .global_transform = Mat3::identity(), .scissor = Rect{.offset = {0, 0}, .extent = viewport_extent}});
    return *this;
  }

  /// reset the rendering context to its default state (transform and scissor)
  Canvas &reset()
  {
    state = CanvasState{.local_transform = Mat3::identity(), .global_transform = Mat3::identity(), .scissor = Rect{.offset = {0, 0}, .extent = viewport_extent}};
    state_stack.clear();
    return *this;
  }

  Canvas &translate(f32 tx, f32 ty)
  {
    state.local_transform = translate2d(tx, ty) * state.local_transform;
    return *this;
  }

  Canvas &translate(Vec2 t)
  {
    return translate(t.x, t.y);
  }

  Canvas &global_translate(f32 tx, f32 ty)
  {
    state.global_transform = translate2d(tx, ty) * state.global_transform;
    return *this;
  }

  Canvas &global_translate(Vec2 t)
  {
    return global_translate(t.x, t.y);
  }

  Canvas &rotate(f32 angle)
  {
    state.local_transform = rotate2d(ASH_TO_RADIANS(angle)) * state.local_transform;
    return *this;
  }

  Canvas &global_rotate(f32 angle)
  {
    state.global_transform = rotate2d(ASH_TO_RADIANS(angle)) * state.global_transform;
    return *this;
  }

  Canvas &scale(f32 sx, f32 sy)
  {
    state.local_transform = scale2d(sx, sy) * state.local_transform;
    return *this;
  }

  Canvas &scale(Vec2 s)
  {
    return scale(s.x, s.y);
  }

  Canvas &global_scale(f32 sx, f32 sy)
  {
    state.global_transform = scale2d(sx, sy) * state.global_transform;
    return *this;
  }

  Canvas &global_scale(Vec2 s)
  {
    return global_scale(s.x, s.y);
  }

  Canvas &shear_x(f32 shear)
  {
    state.local_transform = shear2d_x(shear) * state.local_transform;
    return *this;
  }

  Canvas &global_shear_x(f32 shear)
  {
    state.global_transform = shear2d_x(shear) * state.global_transform;
    return *this;
  }

  Canvas &shear_y(f32 shear)
  {
    state.local_transform = shear2d_y(shear) * state.local_transform;
    return *this;
  }

  Canvas &global_shear_y(f32 shear)
  {
    state.global_transform = shear2d_y(shear) * state.global_transform;
    return *this;
  }
  // TODO(lamarrr): transform_origin
  Canvas &transform(Mat3 const &t)
  {
    state.local_transform = t * state.local_transform;
    return *this;
  }

  Canvas &global_transform(Mat3 const &t)
  {
    state.global_transform = t * state.global_transform;
    return *this;
  }

  /// Not affected by transforms
  Canvas &scissor(Rect scissor)
  {
    state.scissor = scissor;
    return *this;
  }

  Canvas &clear(Color clear_color, image texture = WHITE_IMAGE)
  {
    draw_list.clear();

    Vec4 color = clear_color.to_normalized_vec();

    Vertex2d vertices[] = {{.position = {0, 0}, .uv = {0, 0}, .color = color},
                           {.position = {viewport_extent.x, 0}, .uv = {1, 0}, .color = color},
                           {.position = viewport_extent, .uv = {1, 1}, .color = color},
                           {.position = {0, viewport_extent.y}, .uv = {0, 1}, .color = color}};

    draw_list.vertices.extend(vertices).unwrap();

    u32 indices[] = {0, 1, 2, 0, 2, 3};

    draw_list.indices.extend(indices).unwrap();

    draw_list.commands.push(DrawCommand{
                                .pipeline       = DEFAULT_SHAPE_PIPELINE,
                                .nvertices      = AS(u32, std::size(vertices)),
                                .nindices       = AS(u32, std::size(indices)),
                                .first_instance = 0,
                                .ninstances     = 1,
                                .scissor        = Rect{.offset = {0, 0}, .extent = viewport_extent},
                                .textures       = {texture}}
                                .with_push_constant(make_transform(Vec2{0, 0}).transpose()))
        .unwrap();

    return *this;
  }

  Canvas &draw_path(stx::Span<Vertex2d const> points, Vec2 position, Vec2 uv_stretch, f32 thickness, bool should_close, image texture = WHITE_IMAGE, TextureRect texture_region = TextureRect{.uv0 = Vec2{0, 0}, .uv1 = Vec2{1, 1}})
  {
    if (points.size() < 2 || thickness == 0)
    {
      return *this;
    }

    usize prev_nvertices = draw_list.vertices.size();
    usize prev_nindices  = draw_list.indices.size();

    triangulate_line(points, thickness, draw_list.vertices, draw_list.indices, should_close);
    paths::lerp_uvs(draw_list.vertices.span().slice(prev_nvertices), uv_stretch, texture_region);

    usize curr_nvertices = draw_list.vertices.size();
    usize curr_nindices  = draw_list.indices.size();

    u32 nvertices = AS(u32, curr_nvertices - prev_nvertices);
    u32 nindices  = AS(u32, curr_nindices - prev_nindices);

    draw_list.commands.push(DrawCommand{
                                .pipeline       = DEFAULT_SHAPE_PIPELINE,
                                .nvertices      = nvertices,
                                .nindices       = nindices,
                                .first_instance = 0,
                                .ninstances     = 1,
                                .scissor        = state.scissor,
                                .textures       = {texture}}
                                .with_push_constant(make_transform(position).transpose()))
        .unwrap();

    return *this;
  }

  stx::Span<Vertex2d> reserve_convex_polygon(u32 npoints, Vec2 position, image texture)
  {
    ASH_CHECK(npoints >= 3, "A polygon consists of at least 3 points");

    usize prev_nvertices = draw_list.vertices.size();
    usize prev_nindices  = draw_list.indices.size();

    triangulate_convex_polygon(draw_list.indices, npoints);

    stx::Span polygon = draw_list.vertices.unsafe_resize_uninitialized(draw_list.vertices.size() + npoints).unwrap();

    usize curr_nvertices = draw_list.vertices.size();
    usize curr_nindices  = draw_list.indices.size();

    u32 nvertices = AS(u32, curr_nvertices - prev_nvertices);
    u32 nindices  = AS(u32, curr_nindices - prev_nindices);

    draw_list.commands.push(DrawCommand{
                                .pipeline       = DEFAULT_SHAPE_PIPELINE,
                                .nvertices      = nvertices,
                                .nindices       = nindices,
                                .first_instance = 0,
                                .ninstances     = 1,
                                .scissor        = state.scissor,
                                .textures       = {texture}}
                                .with_push_constant(make_transform(position).transpose()))
        .unwrap();

    return polygon;
  }

  // texture coordinates are assumed to already be filled and area of viewport known
  Canvas &draw_convex_polygon_filled(stx::Span<Vertex2d const> polygon, Vec2 position, image texture)
  {
    if (polygon.size() < 3)
    {
      return *this;
    }

    reserve_convex_polygon(AS(u32, polygon.size()), position, texture).copy(polygon);

    return *this;
  }

  Canvas &draw_rect_filled(Rect area, Color color, LinearColorGradient gradient = {}, image texture = WHITE_IMAGE, TextureRect texture_region = TextureRect{.uv0 = {0, 0}, .uv1 = {1, 1}})
  {
    if (!viewport_contains(area))
    {
      return *this;
    }

    paths::lerp_color_gradient(paths::lerp_uvs(paths::Rect(Vec2{0, 0},
                                                           area.extent,
                                                           color.to_normalized_vec(),
                                                           reserve_convex_polygon(4, area.offset, texture)),
                                               area.extent,
                                               texture_region),
                               area.extent,
                               gradient);

    return *this;
  }

  Canvas &draw_rect_stroke(Rect area, Color color, f32 thickness, image texture = WHITE_IMAGE, TextureRect texture_region = TextureRect{.uv0 = {0, 0}, .uv1 = {1, 1}})
  {
    if (!viewport_contains(area) || thickness == 0)
    {
      return *this;
    }

    Vertex2d line[4];

    paths::Rect(Vec2::splat(thickness / 2), area.extent - thickness, color.to_normalized_vec(), line);

    return draw_path(line, area.offset, area.extent, thickness, true, texture, texture_region);
  }

  Canvas &draw_circle_filled(Vec2 center, f32 radius, u32 nsegments, Color color, LinearColorGradient gradient = {}, image texture = WHITE_IMAGE, TextureRect texture_region = TextureRect{.uv0 = {0, 0}, .uv1 = {1, 1}})
  {
    Vec2 position = center - radius;
    Rect area{.offset = position, .extent = Vec2::splat(2 * radius)};

    if (!viewport_contains(area))
    {
      return *this;
    }

    paths::lerp_color_gradient(paths::lerp_uvs(paths::circle(Vec2{0, 0}, radius, nsegments, color.to_normalized_vec(), reserve_convex_polygon(nsegments, position, texture)),
                                               area.extent, texture_region),
                               area.extent, gradient);

    return *this;
  }

  Canvas &draw_circle_stroke(Vec2 center, f32 radius, u32 nsegments, Color color, f32 thickness, image texture = WHITE_IMAGE, TextureRect texture_region = TextureRect{.uv0 = {0, 0}, .uv1 = {1, 1}})
  {
    Vec2 position = center - radius - thickness / 2;
    Rect area{.offset = position, .extent = Vec2::splat(2 * radius + thickness)};

    if (!viewport_contains(area) || thickness == 0)
    {
      return *this;
    }

    scratch.unsafe_resize_uninitialized(nsegments).unwrap();

    paths::circle(Vec2::splat(thickness / 2), radius, nsegments, color.to_normalized_vec(), scratch);

    return draw_path(scratch, area.offset, area.extent, thickness, true, texture, texture_region);
  }

  Canvas &draw_arc_stroke(Vec2 center, f32 radius, f32 begin, f32 end, u32 nsegments, Color color, f32 thickness, image texture = WHITE_IMAGE, TextureRect texture_region = TextureRect{.uv0 = {0, 0}, .uv1 = {1, 1}})
  {
    Vec2 position = center - radius - thickness / 2;
    Rect area{.offset = position, .extent = Vec2::splat(2 * radius + thickness)};

    if (!viewport_contains(area))
    {
      return *this;
    }

    paths::lerp_uvs(paths::arc(Vec2::splat(thickness / 2), radius, begin, end, nsegments, color.to_normalized_vec(),
                               reserve_convex_polygon(nsegments, position, texture)),
                    area.extent, texture_region);

    return *this;
  }

  Canvas &draw_ellipse_filled(Vec2 center, Vec2 radii, u32 nsegments, Color color, LinearColorGradient gradient = {}, image texture = WHITE_IMAGE, TextureRect texture_region = TextureRect{.uv0 = {0, 0}, .uv1 = {1, 1}})
  {
    Vec2 position = center - radii;
    Rect area{.offset = position, .extent = 2 * radii};

    if (!viewport_contains(area))
    {
      return *this;
    }

    paths::lerp_color_gradient(paths::lerp_uvs(paths::ellipse(Vec2{0, 0}, radii, nsegments, color.to_normalized_vec(), reserve_convex_polygon(nsegments, area.offset, texture)),
                                               area.extent, texture_region),
                               area.extent, gradient);

    return *this;
  }

  Canvas &draw_ellipse_stroke(Vec2 center, Vec2 radii, u32 nsegments, Color color, f32 thickness, image texture = WHITE_IMAGE, TextureRect texture_region = TextureRect{.uv0 = {0, 0}, .uv1 = {1, 1}})
  {
    Vec2 position = center - radii;
    Rect area{.offset = position, .extent = 2 * radii};

    if (!viewport_contains(area) || thickness == 0)
    {
      return *this;
    }

    scratch.unsafe_resize_uninitialized(nsegments).unwrap();

    paths::ellipse(Vec2::splat(thickness / 2), radii - thickness, nsegments, color.to_normalized_vec(), scratch);

    return draw_path(scratch, area.offset, area.extent, thickness, true, texture, texture_region);
  }

  Canvas &draw_round_rect_filled(Rect area, Vec4 radii, u32 nsegments, Color color, LinearColorGradient gradient = {}, image texture = WHITE_IMAGE, TextureRect texture_region = TextureRect{.uv0 = {0, 0}, .uv1 = {1, 1}})
  {
    if (!viewport_contains(area))
    {
      return *this;
    }

    paths::lerp_color_gradient(paths::lerp_uvs(paths::round_rect(Vec2{0, 0}, area.extent, radii, nsegments, color.to_normalized_vec(), reserve_convex_polygon(nsegments * 4 + 8, area.offset, texture)),
                                               area.extent, texture_region),
                               area.extent, gradient);

    return *this;
  }

  Canvas &draw_round_rect_stroke(Rect area, Vec4 radii, u32 nsegments, Color color, f32 thickness, image texture = WHITE_IMAGE, TextureRect texture_region = TextureRect{.uv0 = {0, 0}, .uv1 = {1, 1}})
  {
    if (!viewport_contains(area) || thickness == 0)
    {
      return *this;
    }

    scratch.unsafe_resize_uninitialized(nsegments * 4 + 8).unwrap();

    paths::round_rect(Vec2::splat(thickness / 2), area.extent - thickness, radii, nsegments, color.to_normalized_vec(), scratch);

    return draw_path(scratch, area.offset, area.extent, thickness, true, texture, texture_region);
  }

  Canvas &draw_bevel_rect_filled(Rect area, Vec4 radii, Color color, LinearColorGradient gradient = {}, image texture = WHITE_IMAGE, TextureRect texture_region = TextureRect{.uv0 = {0, 0}, .uv1 = {1, 1}})
  {
    if (!viewport_contains(area))
    {
      return *this;
    }

    paths::lerp_color_gradient(paths::lerp_uvs(paths::bevel_rect(Vec2{0, 0}, area.extent, radii, color.to_normalized_vec(), reserve_convex_polygon(8, area.offset, texture)),
                                               area.extent, texture_region),
                               area.extent, gradient);

    return *this;
  }

  Canvas &draw_bevel_rect_stroke(Rect area, Vec4 radii, Color color, f32 thickness, image texture = WHITE_IMAGE, TextureRect texture_region = TextureRect{.uv0 = {0, 0}, .uv1 = {1, 1}})
  {
    if (!viewport_contains(area) || thickness == 0)
    {
      return *this;
    }

    scratch.unsafe_resize_uninitialized(8).unwrap();

    paths::bevel_rect(Vec2::splat(thickness / 2), area.extent - thickness, radii, color.to_normalized_vec(), scratch);

    return draw_path(scratch, area.offset, area.extent, thickness, true, texture, texture_region);
  }

  Canvas &draw_image(image img, Rect area, TextureRect texture_region, Color tint = colors::WHITE)
  {
    if (!viewport_contains(area))
    {
      return *this;
    }

    paths::lerp_uvs(paths::Rect(Vec2{0, 0}, area.extent, tint.to_normalized_vec(), reserve_convex_polygon(4, area.offset, img)),
                    area.extent, texture_region);

    return *this;
  }

  Canvas &draw_image(image img, Rect area, Color tint = colors::WHITE)
  {
    return draw_image(img, area, TextureRect{.uv0 = {0, 0}, .uv1 = {1, 1}}, tint);
  }

  Canvas &draw_rounded_image(image img, Rect area, Vec4 border_radii, u32 nsegments, TextureRect texture_region, Color tint = colors::WHITE)
  {
    if (!viewport_contains(area))
    {
      return *this;
    }

    paths::lerp_uvs(paths::round_rect(Vec2{0, 0}, area.extent, border_radii, nsegments, tint.to_normalized_vec(), reserve_convex_polygon(nsegments * 4 + 8, area.offset, img)),
                    area.extent, texture_region);

    return *this;
  }

  Canvas &draw_rounded_image(image img, Rect area, Vec4 border_radii, u32 nsegments, Color tint = colors::WHITE)
  {
    return draw_rounded_image(img, area, border_radii, nsegments, TextureRect{.uv0 = {0, 0}, .uv1 = {1, 1}}, tint);
  }

  Canvas &draw_glyph(Vec2 block_position, Vec2 baseline, f32 text_scale_factor, Glyph const &glyph, GlyphShaping const &shaping, TextStyle const &style, gfx::image atlas)
  {
    save();
    state.local_transform = state.local_transform * translate2d(baseline);

    Rect grect;
    grect.offset = Vec2{glyph.metrics.bearing.x, -glyph.metrics.bearing.y} * style.font_height * text_scale_factor + shaping.offset;
    grect.extent = glyph.metrics.extent * style.font_height * text_scale_factor;

    if (!Rect{.offset = {}, .extent = viewport_extent}.overlaps(transform2d(state.global_transform * translate2d(block_position) * state.local_transform, grect)))
    {
      restore();
      return *this;
    }

    Vertex2d const vertices[] = {{.position = grect.top_left(), .uv = glyph.bin_region.top_left(), .color = style.foreground_color.to_normalized_vec()},
                                 {.position = grect.top_right(), .uv = glyph.bin_region.top_right(), .color = style.foreground_color.to_normalized_vec()},
                                 {.position = grect.bottom_right(), .uv = glyph.bin_region.bottom_right(), .color = style.foreground_color.to_normalized_vec()},
                                 {.position = grect.bottom_left(), .uv = glyph.bin_region.bottom_left(), .color = style.foreground_color.to_normalized_vec()}};

    draw_list.vertices.extend(vertices).unwrap();

    triangulate_convex_polygon(draw_list.indices, 4);

    draw_list.commands.push(DrawCommand{
                                .pipeline       = DEFAULT_GLYPH_PIPELINE,
                                .nvertices      = 4,
                                .nindices       = 6,
                                .first_instance = 0,
                                .ninstances     = 1,
                                .scissor        = state.scissor,
                                .textures       = {atlas}}
                                .with_push_constant(make_transform(block_position).transpose()))
        .unwrap();

    restore();
    return *this;
  }

  Canvas &draw_glyph_shadow(Vec2 block_position, Vec2 baseline, f32 text_scale_factor, Glyph const &glyph, GlyphShaping const &shaping, TextStyle const &style, gfx::image atlas)
  {
    save();
    state.local_transform = state.local_transform * translate2d(baseline);

    // TODO(lamarrr): add offset to shadow scale? and let offset be from midpoint??
    Rect grect;
    grect.offset = Vec2{glyph.metrics.bearing.x, -glyph.metrics.bearing.y} * style.font_height * text_scale_factor + shaping.offset;
    grect.extent = glyph.metrics.extent * style.font_height * text_scale_factor;

    Rect srect   = grect;
    srect.extent = srect.extent * style.shadow_scale;
    srect.offset = grect.offset + style.shadow_offset;

    if (!Rect{.offset = {}, .extent = viewport_extent}.overlaps(transform2d(state.global_transform * translate2d(block_position) * state.local_transform, srect)))
    {
      restore();
      return *this;
    }

    Vertex2d const vertices[] = {{.position = srect.top_left(), .uv = glyph.bin_region.top_left(), .color = style.shadow_color.to_normalized_vec()},
                                 {.position = srect.top_right(), .uv = glyph.bin_region.top_right(), .color = style.shadow_color.to_normalized_vec()},
                                 {.position = srect.bottom_right(), .uv = glyph.bin_region.bottom_right(), .color = style.shadow_color.to_normalized_vec()},
                                 {.position = srect.bottom_left(), .uv = glyph.bin_region.bottom_left(), .color = style.shadow_color.to_normalized_vec()}};

    draw_list.vertices.extend(vertices).unwrap();

    triangulate_convex_polygon(draw_list.indices, 4);

    draw_list.commands.push(DrawCommand{
                                .pipeline       = DEFAULT_GLYPH_PIPELINE,
                                .nvertices      = 4,
                                .nindices       = 6,
                                .first_instance = 0,
                                .ninstances     = 1,
                                .scissor        = state.scissor,
                                .textures       = {atlas}}
                                .with_push_constant(make_transform(block_position).transpose()))
        .unwrap();

    restore();
    return *this;
  }

  Canvas &draw_text_segment_lines(Vec2 block_position, Vec2 baseline, f32 line_height, f32 segment_width, TextStyle const &style)
  {
    save();
    translate(block_position);

    if (style.strikethrough_color.is_visible() && style.strikethrough_thickness > 0)
    {
      Vertex2d const strikethrough_path[] = {{.position = baseline - Vec2{0, line_height / 2}, .uv = {}, .color = style.strikethrough_color.to_normalized_vec()},
                                             {.position = baseline - Vec2{-segment_width, line_height / 2}, .uv = {}, .color = style.strikethrough_color.to_normalized_vec()}};

      draw_path(strikethrough_path, Vec2{0, 0}, Vec2{0, 0}, style.strikethrough_thickness, false);
    }

    if (style.underline_color.is_visible() && style.underline_thickness > 0)
    {
      Vertex2d const underline_path[] = {{.position = baseline, .uv = {}, .color = style.underline_color.to_normalized_vec()},
                                         {.position = baseline + Vec2{segment_width, 0}, .uv = {}, .color = style.underline_color.to_normalized_vec()}};

      draw_path(underline_path, Vec2{0, 0}, Vec2{0, 0}, style.underline_thickness, false);
    }

    restore();

    return *this;
  }

  Canvas &draw_text_segment_background(Vec2 block_position, Vec2 line_top, Vec2 extent, TextStyle const &style)
  {
    save();
    translate(block_position);
    draw_rect_filled(Rect{.offset = line_top, .extent = extent}, style.background_color);
    restore();
    return *this;
  }

  // TODO(lamarrr): text gradient, reset on each line or continue???? how does css do it?
  Canvas &draw_text(TextBlock const &block, TextLayout const &layout, stx::Span<BundledFont const> font_bundle, Vec2 const position)
  {
    /// TEXT BACKGROUNDS ///
    {
      // TODO(lamarrr): merge segment text backgrounds
      f32 line_top = 0;
      for (LineMetrics const &line : layout.lines)
      {
        f32 x_alignment = 0;

        switch (block.align)
        {
          case TextAlign::Start:
          {
            if (line.base_direction == TextDirection::RightToLeft)
            {
              x_alignment = layout.span.x - line.width;
            }
          }
          break;

          case TextAlign::Center:
          {
            x_alignment = (layout.span.x - line.width) / 2;
          }
          break;

          case TextAlign::End:
          {
            if (line.base_direction == TextDirection::LeftToRight)
            {
              x_alignment = layout.span.x - line.width;
            }
          }
          break;

          default:
            break;
        }

        f32 x_cursor = x_alignment;

        for (TextRunSegment const &segment : layout.run_segments.span().slice(line.run_segments_offset, line.nrun_segments))
        {
          TextStyle const &segment_style = segment.style >= block.styles.size() ? block.default_style : block.styles[segment.style];
          if (segment_style.background_color.is_visible())
          {
            draw_text_segment_background(position, Vec2{x_cursor, line_top}, Vec2{segment.width, line.line_height}, segment_style);
          }

          x_cursor += segment.width;
        }

        line_top += line.line_height;
      }
    }

    /// GLYPH SHADOWS ///
    {
      f32 line_top = 0;
      for (LineMetrics const &line : layout.lines)
      {
        f32 x_alignment = 0;

        switch (block.align)
        {
          case TextAlign::Start:
          {
            if (line.base_direction == TextDirection::RightToLeft)
            {
              x_alignment = layout.span.x - line.width;
            }
          }
          break;

          case TextAlign::Center:
          {
            x_alignment = (layout.span.x - line.width) / 2;
          }
          break;

          case TextAlign::End:
          {
            if (line.base_direction == TextDirection::LeftToRight)
            {
              x_alignment = layout.span.x - line.width;
            }
          }
          break;

          default:
            break;
        }

        f32       x_segment_cursor = x_alignment;
        f32 const line_gap         = std::max(line.line_height - (line.ascent + line.descent), 0.0f) / 2;
        f32 const baseline         = line_top + line.line_height - line_gap - line.descent;

        for (TextRunSegment const &segment : layout.run_segments.span().slice(line.run_segments_offset, line.nrun_segments))
        {
          TextStyle const &segment_style = segment.style >= block.styles.size() ? block.default_style : block.styles[segment.style];

          if (segment_style.shadow_color.is_transparent() || segment_style.shadow_scale <= 0)
          {
            continue;
          }

          FontAtlas const &atlas    = font_bundle[segment.font].atlas;
          f32              x_cursor = x_segment_cursor;

          for (GlyphShaping const &shaping : layout.glyph_shapings.span().slice(segment.glyph_shapings_offset, segment.nglyph_shapings))
          {
            draw_glyph_shadow(position, Vec2{x_cursor, baseline}, layout.text_scale_factor, atlas.glyphs[shaping.glyph], shaping, segment_style, atlas.bins[atlas.glyphs[shaping.glyph].bin].texture);
            x_cursor += shaping.advance + layout.text_scale_factor * segment_style.letter_spacing;
          }

          x_segment_cursor += segment.width;
        }

        line_top += line.line_height;
      }
    }

    /// GLYPHS ///
    {
      f32 line_top = 0;
      for (LineMetrics const &line : layout.lines)
      {
        f32 x_alignment = 0;

        switch (block.align)
        {
          case TextAlign::Start:
          {
            if (line.base_direction == TextDirection::RightToLeft)
            {
              x_alignment = layout.span.x - line.width;
            }
          }
          break;

          case TextAlign::Center:
          {
            x_alignment = (layout.span.x - line.width) / 2;
          }
          break;

          case TextAlign::End:
          {
            if (line.base_direction == TextDirection::LeftToRight)
            {
              x_alignment = layout.span.x - line.width;
            }
          }
          break;

          default:
            break;
        }

        f32       x_segment_cursor = x_alignment;
        f32 const line_gap         = std::max(line.line_height - (line.ascent + line.descent), 0.0f) / 2;
        f32 const baseline         = line_top + line.line_height - line_gap - line.descent;

        for (TextRunSegment const &segment : layout.run_segments.span().slice(line.run_segments_offset, line.nrun_segments))
        {
          TextStyle const &segment_style = segment.style >= block.styles.size() ? block.default_style : block.styles[segment.style];
          FontAtlas const &atlas         = font_bundle[segment.font].atlas;
          f32              x_cursor      = x_segment_cursor;

          for (GlyphShaping const &shaping : layout.glyph_shapings.span().slice(segment.glyph_shapings_offset, segment.nglyph_shapings))
          {
            draw_glyph(position, Vec2{x_cursor, baseline}, layout.text_scale_factor, atlas.glyphs[shaping.glyph], shaping, segment_style, atlas.bins[atlas.glyphs[shaping.glyph].bin].texture);
            x_cursor += shaping.advance + layout.text_scale_factor * segment_style.letter_spacing;
          }

          x_segment_cursor += segment.width;
        }

        line_top += line.line_height;
      }
    }

    /// UNDERLINES AND STRIKETHROUGHS ///
    {
      // TODO(lamarrr): merge segment lines and strikethroughs
      f32 line_top = 0;
      for (LineMetrics const &line : layout.lines)
      {
        f32 x_alignment = 0;

        switch (block.align)
        {
          case TextAlign::Start:
          {
            if (line.base_direction == TextDirection::RightToLeft)
            {
              x_alignment = layout.span.x - line.width;
            }
          }
          break;

          case TextAlign::Center:
          {
            x_alignment = (layout.span.x - line.width) / 2;
          }
          break;

          case TextAlign::End:
          {
            if (line.base_direction == TextDirection::LeftToRight)
            {
              x_alignment = layout.span.x - line.width;
            }
          }
          break;

          default:
            break;
        }

        f32       x_cursor = x_alignment;
        f32 const line_gap = std::max(line.line_height - (line.ascent + line.descent), 0.0f) / 2;
        f32 const baseline = line_top + line.line_height - line_gap - line.descent;

        for (TextRunSegment const &segment : layout.run_segments.span().slice(line.run_segments_offset, line.nrun_segments))
        {
          TextStyle const &segment_style = segment.style >= block.styles.size() ? block.default_style : block.styles[segment.style];

          if ((segment_style.underline_color.is_visible() && segment_style.underline_thickness > 0) || (segment_style.strikethrough_color.is_visible() && segment_style.strikethrough_thickness > 0)) [[unlikely]]
          {
            draw_text_segment_lines(position, Vec2{x_cursor, baseline}, line.line_height, segment.width, segment_style);
          }

          x_cursor += segment.width;
        }

        line_top += line.line_height;
      }
    }

    return *this;
  }
};

}        // namespace gfx
}        // namespace ash
