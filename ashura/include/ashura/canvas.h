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

namespace paths
{

inline stx::Span<vertex> rect(vec2 extent, vec4 color, vec2 offset, stx::Span<vertex> polygon)
{
  vertex vertices[] = {{.position = offset, .uv = {}, .color = color},
                       {.position = offset + vec2{extent.x, 0}, .uv = {}, .color = color},
                       {.position = offset + extent, .uv = {}, .color = color},
                       {.position = offset + vec2{0, extent.y}, .uv = {}, .color = color}};

  polygon.copy(vertices);

  return polygon;
}

inline stx::Span<vertex> arc(vec2 offset, f32 radius, f32 begin, f32 end, u32 nsegments, vec4 color, stx::Span<vertex> polygon)
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
    vec2 p     = radius + radius *vec2{std::cos(angle), std::sin(angle)};
    polygon[i] = vertex{.position = offset + p, .uv = {}, .color = color};
  }

  return polygon;
}
{
  if (nsegments == 0 || radius <= 0)
  {
    return {};
  }

  f32 step = (2 * PI) / nsegments;

  for (u32 i = 0; i < nsegments; i++)
  {
    vec2 p     = radius + radius *vec2{std::cos(i * step), std::sin(i * step)};
    polygon[i] = vertex{.position = offset + p, .uv = {}, .color = color};
  }

  return polygon;
}

inline stx::Span<vertex> ellipse(vec2 radii, u32 nsegments, vec4 color, vec2 offset, stx::Span<vertex> polygon)
{
  if (nsegments == 0 || radii.x <= 0 || radii.y <= 0)
  {
    return {};
  }

  f32 step = (2 * PI) / nsegments;

  for (u32 i = 0; i < nsegments; i++)
  {
    vec2 p     = radii + radii *vec2{std::cos(i * step), std::sin(i * step)};
    polygon[i] = vertex{.position = offset + p, .uv = {}, .color = color};
  }

  return polygon;
}

// outputs 8 + nsegments * 4 vertices
inline stx::Span<vertex> round_rect(vec2 extent, vec4 radii, u32 nsegments, vec4 color, vec2 offset, stx::Span<vertex> polygon)
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

  polygon[i] = vertex{.position = offset + extent - vec2{0, radii.z}, .uv = {}, .color = color};
  i++;

  for (u32 segment = 0; segment < nsegments; segment++, i++)
  {
    vec2 p     = (extent - radii.z) + radii.z *vec2{std::cos(segment * step), std::sin(segment * step)};
    polygon[i] = vertex{.position = offset + p, .uv = {}, .color = color};
  }

  polygon[i] = vertex{.position = offset + extent - vec2{radii.z, 0}, .uv = {}, .color = color};
  i++;

  polygon[i] = vertex{.position = offset + vec2{radii.w, extent.y}, .uv = {}, .color = color};
  i++;

  for (u32 segment = 0; segment < nsegments; segment++, i++)
  {
    vec2 p     = vec2{radii.w, extent.y - radii.w} + radii.w *vec2{std::cos(PI / 2 + segment * step), std::sin(PI / 2 + segment * step)};
    polygon[i] = vertex{.position = offset + p, .uv = {}, .color = color};
  }

  polygon[i] = vertex{.position = offset + vec2{0, extent.y - radii.w}, .uv = {}, .color = color};
  i++;

  polygon[i] = vertex{.position = offset + vec2{0, radii.x}, .uv = {}, .color = color};
  i++;

  for (u32 segment = 0; segment < nsegments; segment++, i++)
  {
    vec2 p     = radii.x + radii.x *vec2{std::cos(PI + segment * step), std::sin(PI + segment * step)};
    polygon[i] = vertex{.position = offset + p, .uv = {}, .color = color};
  }

  polygon[i] = vertex{.position = offset + vec2{radii.x, 0}, .uv = {}, .color = color};
  i++;

  polygon[i] = vertex{.position = offset + vec2{extent.x - radii.y, 0}, .uv = {}, .color = color};
  i++;

  for (u32 segment = 0; segment < nsegments; segment++, i++)
  {
    vec2 p     = vec2{extent.x - radii.y, radii.y} + radii.y *vec2{std::cos(PI * 3.0f / 2.0f + segment * step), std::sin(PI * 3.0f / 2.0f + segment * step)};
    polygon[i] = vertex{.position = offset + p, .uv = {}, .color = color};
  }

  polygon[i] = vertex{.position = offset + vec2{extent.x, radii.y}, .uv = {}, .color = color};

  return polygon;
}

inline stx::Span<vertex> lerp_uvs(stx::Span<vertex> path, vec2 extent, texture_rect texture_region)
{
  for (vertex &v : path)
  {
    v.uv = texture_region.uv0 + v.position / epsilon_clamp(extent) * (texture_region.uv1 - texture_region.uv0);
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
inline void add_line_stroke(vec2 p0, vec2 p1, f32 thickness, vec4 color, stx::Vec<vertex> &out)
{
  // the angles are specified in clockwise direction to be compatible with the
  // vulkan coordinate system
  //
  // get the angle of inclination of p2 to p1
  vec2 d     = p1 - p0;
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

  vec2 f = thickness / 2 * vec2{std::cos(alpha), std::sin(alpha)};
  vec2 g = thickness / 2 * vec2{std::cos(PI + alpha), std::sin(PI + alpha)};

  vec2 p0_0 = p0 + f;
  vec2 p0_1 = p0 + g;

  vec2 p1_0 = p1 + f;
  vec2 p1_1 = p1 + g;

  vertex vertices[] = {{.position = p0_0, .uv = {}, .color = color},
                       {.position = p0_1, .uv = {}, .color = color},
                       {.position = p1_0, .uv = {}, .color = color},
                       {.position = p1_1, .uv = {}, .color = color}};

  out.extend(vertices).unwrap();
}

// line joint is a bevel joint, it is the most efficient since it re-uses existing vertices and doesn't require generating new vertices
inline void triangulate_line(stx::Span<vertex const> in_points, f32 thickness, stx::Vec<vertex> &out_vertices, stx::Vec<u32> &out_indices, bool should_close)
{
  if (in_points.size() < 2 || thickness == 0)
  {
    return;
  }

  bool has_previous_line = false;

  u32 vertex_index = 0;

  for (u32 i = 1; i < AS(u32, in_points.size()); i++)
  {
    vec4 color = in_points[i - 1].color;
    vec2 p0    = in_points[i - 1].position;
    vec2 p1    = in_points[i].position;

    add_line_stroke(p0, p1, thickness, color, out_vertices);

    // weave the line triangles
    u32 indices[] = {vertex_index, vertex_index + 1, vertex_index + 3, vertex_index, vertex_index + 2, vertex_index + 3};

    out_indices.extend(indices).unwrap();

    // weave the previous line's end to the beginning of this line
    if (has_previous_line)
    {
      u32 prev_line_vertex_index = vertex_index - 4;

      u32 indices[] = {prev_line_vertex_index + 2, prev_line_vertex_index + 3, vertex_index,
                       prev_line_vertex_index + 2, prev_line_vertex_index + 3, vertex_index + 1};

      out_indices.extend(indices).unwrap();
    }

    has_previous_line = true;

    vertex_index += 4;
  }

  // requires at least 3 points to be closable
  if (should_close && in_points.size() > 2)
  {
    vec4 color = in_points[in_points.size() - 1].color;
    vec2 p0    = in_points[in_points.size() - 1].position;
    vec2 p1    = in_points[0].position;

    add_line_stroke(p0, p1, thickness, color, out_vertices);

    // weave the line triangles
    u32 indices[] = {vertex_index, vertex_index + 1, vertex_index + 3, vertex_index, vertex_index + 2, vertex_index + 3};

    out_indices.extend(indices).unwrap();

    {
      u32 prev_line_vertex_index  = vertex_index - 4;
      u32 first_line_vertex_index = 0;

      u32 indices[] = {
          // weave the previous line's end to the beginning of this line
          prev_line_vertex_index + 2, prev_line_vertex_index + 3, vertex_index,
          prev_line_vertex_index + 2, prev_line_vertex_index + 3, vertex_index + 1,
          // weave this line's end to the beginning of the first line
          vertex_index + 2, vertex_index + 3, first_line_vertex_index,
          vertex_index + 2, vertex_index + 3, first_line_vertex_index + 1};

      out_indices.extend(indices).unwrap();
    }
  }
}

struct DrawCommand
{
  std::string_view pipeline;                                      /// ID of pipeline to use for rendering
  u32              nvertices  = 0;                                /// number of vertices for this draw call. offset is automatically determined
  u32              nindices   = 0;                                /// number of indices for this draw call. offset is automatically determined
  u32              ninstances = 1;                                /// number of instances used for instanced rendering
  irect            scissor;                                       /// determines visible area of the rendering operation, in framebuffer coordinates (0, 0) -> viewport_extent
  image            textures[NIMAGES_PER_DRAWCALL]    = {};        /// textures bounded to each descriptor set, 8-max
  u8               push_constant[PUSH_CONSTANT_SIZE] = {};        /// push constant used for draw call. maximum size of PUSH_CONSTANT_SIZE bytes

  template <typename T>
  DrawCommand with_push_constant(T const &constant) const
  {
    static_assert(sizeof(T) <= PUSH_CONSTANT_SIZE);
    stx::Span{push_constant}.as_u8().copy(stx::Span{&constant, 1}.as_u8());
    return *this;
  }
};

struct DrawList
{
  stx::Vec<vertex>      vertices;
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
  mat3  transform;               // local object transform, applies to local coordinates of the objects
  mat3  global_transform;        // global scene transform, applies to the global coordinate of the objects
  irect scissor;
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
  vec2                  viewport_extent;
  CanvasState           state;
  stx::Vec<CanvasState> state_stack;
  DrawList              draw_list;
  stx::Vec<vertex>      scratch;        // scratch/temporary buffer for storing generating vertices before storing in the draw list

  bool viewport_contains(rect area) const
  {
    return rect{.offset = {}, .extent = viewport_extent}
        .overlaps(transform2d(state.global_transform * state.transform, area));
  }

  Canvas &restart(vec2 viewport_extent)
  {
    this->viewport_extent = viewport_extent;
    state                 = CanvasState{.transform        = mat3::identity(),
                                        .global_transform = mat3::identity(),
                                        .scissor          = irect{.offset = {0, 0}, .extent = extent::from(viewport_extent)}};
    state_stack.clear();
    draw_list.clear();
    return *this;
  }

  mat3 make_transform(vec2 position) const
  {
    vec2 viewport_extent_clamped = epsilon_clamp(viewport_extent);
    return translate2d(-1, -1)                                                            /// normalize to vulkan viewport coordinate range -1 to 1
           * scale2d(2 / viewport_extent_clamped.x, 2 / viewport_extent_clamped.y)        /// normalize to 0 to 2 coordinate range
           * state.global_transform                                                       /// apply global coordinate transform
           * translate2d(position.x, position.y)                                          /// apply viewport positioning
           * state.transform;                                                             /// apply local coordinate transform
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
    state = state_stack.pop().unwrap_or(CanvasState{.transform = mat4::identity(), .global_transform = mat4::identity(), .scissor = rect{.offset = {0, 0}, .extent = viewport_extent}});
    return *this;
  }

  /// reset the rendering context to its default state (transform and scissor)
  Canvas &reset()
  {
    state = CanvasState{.transform = mat4::identity(), .global_transform = mat4::identity(), .scissor = rect{.offset = {0, 0}, .extent = viewport_extent}};
    state_stack.clear();
    return *this;
  }

  Canvas &translate(f32 tx, f32 ty)
  {
    state.transform = translate2d(tx, ty) * state.transform;
    return *this;
  }

  Canvas &global_translate(f32 tx, f32 ty)
  {
    state.global_transform = translate2d(tx, ty) * state.global_transform;
    return *this;
  }

  Canvas &rotate(f32 angle)
  {
    state.transform = rotate2d(ASH_TO_RADIANS(angle)) * state.transform;
    return *this;
  }

  Canvas &global_rotate(f32 angle)
  {
    state.global_transform = rotate2d(ASH_TO_RADIANS(angle)) * state.global_transform;
    return *this;
  }

  Canvas &scale(f32 x, f32 y)
  {
    state.transform = scale2d(x, y) * state.transform;
    return *this;
  }

  Canvas &global_scale(f32 x, f32 y)
  {
    state.global_transform = scale2d(x, y) * state.global_transform;
    return *this;
  }

  Canvas &shear_x(f32 shear)
  {
    state.transform = shear2d_x(shear) * state.transform;
    return *this;
  }

  Canvas &global_shear_x(f32 shear)
  {
    state.global_transform = shear2d_x(shear) * state.global_transform;
    return *this;
  }

  Canvas &shear_y(f32 shear)
  {
    state.transform = shear2d_y(shear) * state.transform;
    return *this;
  }

  Canvas &global_shear_y(f32 shear)
  {
    state.global_transform = shear2d_y(shear) * state.global_transform;
    return *this;
  }

  Canvas &transform(mat3 const &t)
  {
    state.transform = t * state.transform;
    return *this;
  }

  Canvas &global_transform(mat3 const &t)
  {
    state.global_transform = t * state.global_transform;
    return *this;
  }

  /// Not affected by transforms
  Canvas &scissor(irect scissor)
  {
    state.scissor = scissor;
    return *this;
  }

  Canvas &clear(color clear_color, image texture = WHITE_IMAGE)
  {
    draw_list.clear();

    vec4 color = clear_color.to_vec();

    vertex vertices[] = {{.position = {0, 0}, .uv = {0, 0}, .color = color},
                         {.position = {viewport_extent.x, 0}, .uv = {1, 0}, .color = color},
                         {.position = viewport_extent, .uv = {1, 1}, .color = color},
                         {.position = {0, viewport_extent.y}, .uv = {0, 1}, .color = color}};

    draw_list.vertices.extend(vertices).unwrap();

    u32 indices[] = {0, 1, 2, 0, 2, 3};

    draw_list.indices.extend(indices).unwrap();

    draw_list.commands.push(DrawCommand{
                                .pipeline   = DEFAULT_SHAPE_PIPELINE,
                                .nvertices  = AS(u32, std::size(vertices)),
                                .nindices   = AS(u32, std::size(indices)),
                                .ninstances = 1,
                                .scissor    = irect{.offset = {0, 0}, .extent = extent::from(viewport_extent)},
                                .textures   = {texture}}
                                .with_push_constant(make_transform(vec2{0, 0}).transpose()))
        .unwrap();

    return *this;
  }

  Canvas &draw_path(stx::Span<vertex const> points, rect area, f32 thickness, bool should_close, image texture = WHITE_IMAGE, texture_rect texture_region = texture_rect{.uv0 = vec2{0, 0}, .uv1 = vec2{1, 1}})
  {
    if (points.size() < 2 || thickness == 0)
    {
      return *this;
    }

    usize prev_nvertices = draw_list.vertices.size();
    usize prev_nindices  = draw_list.indices.size();

    triangulate_line(points, thickness, draw_list.vertices, draw_list.indices, should_close);
    paths::lerp_uvs(draw_list.vertices.span().slice(prev_nvertices), area.extent, texture_region);

    usize curr_nvertices = draw_list.vertices.size();
    usize curr_nindices  = draw_list.indices.size();

    u32 nvertices = AS(u32, curr_nvertices - prev_nvertices);
    u32 nindices  = AS(u32, curr_nindices - prev_nindices);

    draw_list.commands.push(DrawCommand{
                                .pipeline   = DEFAULT_SHAPE_PIPELINE,
                                .nvertices  = nvertices,
                                .nindices   = nindices,
                                .ninstances = 1,
                                .scissor    = state.scissor,
                                .textures   = {texture}}
                                .with_push_constant(make_transform(area.offset).transpose()))
        .unwrap();

    return *this;
  }

  stx::Span<vertex> reserve_convex_polygon(u32 npoints, vec2 position, image texture)
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
                                .pipeline   = DEFAULT_SHAPE_PIPELINE,
                                .nvertices  = nvertices,
                                .nindices   = nindices,
                                .ninstances = 1,
                                .scissor    = state.scissor,
                                .textures   = {texture}}
                                .with_push_constant(make_transform(position).transpose()))
        .unwrap();

    return polygon;
  }

  // texture coordinates are assumed to already be filled and area of viewport known
  Canvas &draw_convex_polygon_filled(stx::Span<vertex const> polygon, vec2 position, image texture)
  {
    if (polygon.size() < 3)
    {
      return *this;
    }

    reserve_convex_polygon(AS(u32, polygon.size()), position, texture).copy(polygon);

    return *this;
  }

  Canvas &draw_rect_filled(rect area, color color, image texture = WHITE_IMAGE, texture_rect texture_region = texture_rect{.uv0 = {0, 0}, .uv1 = {1, 1}})
  {
    if (!viewport_contains(area))
    {
      return *this;
    }

    paths::interpolate_uvs(paths::rect(area.extent, color.to_vec(), vec2{0, 0}, reserve_convex_polygon(4, area.offset, texture)),
                           area.extent,
                           texture_region);

    return *this;
  }

  Canvas &draw_rect_stroke(rect area, color color, f32 thickness, image texture = WHITE_IMAGE, texture_rect texture_region = texture_rect{.uv0 = {0, 0}, .uv1 = {1, 1}})
  {
    if (!viewport_contains(area) || thickness == 0)
    {
      return *this;
    }

    vertex line[4];

    paths::rect(vec2::splat(thickness / 2), area.extent - thickness, color.to_vec(), line);

    return draw_path(line, area, thickness, true, texture, texture_region);
  }

  Canvas &draw_circle_filled(vec2 center, f32 radius, u32 nsegments, color color, image texture = WHITE_IMAGE, texture_rect texture_region = texture_rect{.uv0 = {0, 0}, .uv1 = {1, 1}})
  {
    vec2 position = center - radius;
    rect area{.offset = position, .extent = vec2::splat(2 * radius)};

    if (!viewport_contains(area))
    {
      return *this;
    }

    paths::interpolate_uvs(paths::circle(radius, nsegments, color.to_vec(), vec2{0, 0}, reserve_convex_polygon(nsegments, position, texture)),
                           area.extent,
                           texture_region);

    return *this;
  }

  Canvas &draw_circle_stroke(vec2 center, f32 radius, u32 nsegments, color color, f32 thickness, image texture = WHITE_IMAGE, texture_rect texture_region = texture_rect{.uv0 = {0, 0}, .uv1 = {1, 1}})
  {
    vec2 position = center - radius - thickness / 2;
    rect area{.offset = position, .extent = vec2::splat(2 * radius + thickness)};

    if (!viewport_contains(area) || thickness == 0)
    {
      return *this;
    }

    scratch.unsafe_resize_uninitialized(nsegments).unwrap();

    paths::circle(vec2::splat(thickness / 2), radius - thickness, nsegments, color.to_vec(), scratch);

    return draw_path(scratch, area, thickness, true, texture, texture_region);
  }

  Canvas &draw_arc_stroke(vec2 center, f32 radius, f32 begin, f32 end, u32 nsegments, color color, f32 thickness, image texture = WHITE_IMAGE, texture_rect texture_region = texture_rect{.uv0 = {0, 0}, .uv1 = {1, 1}})
  {
    vec2 position = center - radius - thickness / 2;
    rect area{.offset = position, .extent = vec2::splat(2 * radius + thickness)};

    if (!viewport_contains(area))
    {
      return *this;
    }

    paths::lerp_uvs(paths::arc(vec2::splat(thickness / 2), radius, begin, end, nsegments, color.to_vec(),
                               reserve_convex_polygon(nsegments, position, texture)),
                    area.extent, texture_region);

    return *this;
  }

  Canvas &draw_ellipse_filled(vec2 center, vec2 radii, u32 nsegments, color color, image texture = WHITE_IMAGE, texture_rect texture_region = texture_rect{.uv0 = {0, 0}, .uv1 = {1, 1}})
  {
    vec2 position = center - radii;
    rect area{.offset = position, .extent = 2 * radii};

    if (!viewport_contains(area))
    {
      return *this;
    }

    paths::lerp_uvs(paths::ellipse(vec2{0, 0}, radii, nsegments, color.to_vec(), reserve_convex_polygon(nsegments, area.offset, texture)),
                    area.extent, texture_region);

    return *this;
  }

  Canvas &draw_ellipse_stroke(vec2 center, vec2 radii, u32 nsegments, color color, f32 thickness, image texture = WHITE_IMAGE, texture_rect texture_region = texture_rect{.uv0 = {0, 0}, .uv1 = {1, 1}})
  {
    vec2 position = center - radii;
    rect area{.offset = position, .extent = 2 * radii};

    if (!viewport_contains(area) || thickness == 0)
    {
      return *this;
    }

    scratch.unsafe_resize_uninitialized(nsegments).unwrap();

    paths::ellipse(vec2::splat(thickness / 2), radii - thickness, nsegments, color.to_vec(), scratch);

    return draw_path(scratch, area, thickness, true, texture, texture_region);
  }

  Canvas &draw_round_rect_filled(rect area, vec4 radii, u32 nsegments, color color, image texture = WHITE_IMAGE, texture_rect texture_region = texture_rect{.uv0 = {0, 0}, .uv1 = {1, 1}})
  {
    if (!viewport_contains(area))
    {
      return *this;
    }

    paths::lerp_uvs(paths::round_rect(vec2{0, 0}, area.extent, radii, nsegments, color.to_vec(), reserve_convex_polygon(nsegments * 4 + 8, area.offset, texture)),
                    area.extent, texture_region);

    return *this;
  }

  Canvas &draw_round_rect_stroke(rect area, vec4 radii, u32 nsegments, color color, f32 thickness, image texture = WHITE_IMAGE, texture_rect texture_region = texture_rect{.uv0 = {0, 0}, .uv1 = {1, 1}})
  {
    if (!viewport_contains(area) || thickness == 0)
    {
      return *this;
    }

    scratch.unsafe_resize_uninitialized(nsegments * 4 + 8).unwrap();

    paths::round_rect(vec2::splat(thickness / 2), area.extent - thickness, radii, nsegments, color.to_vec(), scratch);

    return draw_path(scratch, area, thickness, true, texture, texture_region);
  }

  Canvas &draw_image(image img, rect area, texture_rect texture_region, color tint = colors::WHITE)
  {
    if (!viewport_contains(area))
    {
      return *this;
    }

    paths::interpolate_uvs(paths::rect(area.extent, tint.to_vec(), vec2{0, 0}, reserve_convex_polygon(4, area.offset, img)),
                           area.extent,
                           texture_region);

    return *this;
  }

  Canvas &draw_image(image img, rect area, color tint = colors::WHITE)
  {
    return draw_image(img, area, texture_rect{.uv0 = {0, 0}, .uv1 = {1, 1}}, tint);
  }

  Canvas &draw_rounded_image(image img, rect area, vec4 border_radii, u32 nsegments, texture_rect texture_region, color tint = colors::WHITE)
  {
    if (!viewport_contains(area))
    {
      return *this;
    }

    paths::interpolate_uvs(paths::round_rect(area.extent, border_radii, nsegments, tint.to_vec(), vec2{0, 0}, reserve_convex_polygon(nsegments * 4 + 8, area.offset, img)),
                           area.extent,
                           texture_region);

    return *this;
  }

  Canvas &draw_rounded_image(image img, rect area, vec4 border_radii, u32 nsegments, color tint = colors::WHITE)
  {
    return draw_rounded_image(img, area, border_radii, nsegments, texture_rect{.uv0 = {0, 0}, .uv1 = {1, 1}}, tint);
  }

  Canvas &draw_text(Paragraph const &paragraph, TextLayout const &layout, stx::Span<BundledFont const> font_bundle, vec2 const position)
  {
    if (!viewport_contains(rect{.offset = position, .extent = layout.span}))
    {
      return *this;
    }

    for (TextRunSubWord const &subword : layout.subwords)
    {
      TextProps const &props = paragraph.runs[subword.run].props.as_cref().unwrap_or(paragraph.props);

      if (props.background_color.is_visible())
      {
        draw_rect_filled(rect{.offset = position + subword.area.offset, .extent = subword.area.extent}, props.background_color);
      }
    }

    for (GlyphLayout const &glyph_layout : layout.glyph_layouts)
    {
      TextProps const &props = paragraph.runs[glyph_layout.run].props.as_cref().unwrap_or(paragraph.props);
      FontAtlas const &atlas = font_bundle[glyph_layout.font].atlas;

      if (props.stroke_color.is_visible() && props.font_height > 0 && font_bundle[glyph_layout.font].stroke_atlas.is_some())
      {
        FontStrokeAtlas const &stroke_atlas  = font_bundle[glyph_layout.font].stroke_atlas.value();
        f32                    glyph_scale   = props.font_height / atlas.font_height;
        vec2                   extent        = glyph_scale * stroke_atlas.strokes[glyph_layout.glyph].extent.to_vec();
        vec2                   stroke_offset = (atlas.glyphs[glyph_layout.glyph].extent.to_vec() - stroke_atlas.strokes[glyph_layout.glyph].extent.to_vec()) / 2;
        stroke_offset                        = glyph_scale * stroke_offset + props.stroke_offset;

        draw_image(stroke_atlas.texture,
                   rect{.offset = position + glyph_layout.offset + stroke_offset, .extent = extent},
                   stroke_atlas.strokes[glyph_layout.glyph].texture_region,
                   props.stroke_color);
      }
    }

    for (GlyphLayout const &glyph_layout : layout.glyph_layouts)
    {
      TextProps const &props = paragraph.runs[glyph_layout.run].props.as_cref().unwrap_or(paragraph.props);
      Font const      &font  = *font_bundle[glyph_layout.font].font;
      FontAtlas const &atlas = font_bundle[glyph_layout.font].atlas;

      if (props.font_height > 0)
      {
        if (!font.has_color)
        {
          if (props.foreground_color.is_visible())
          {
            draw_image(atlas.texture,
                       rect{.offset = position + glyph_layout.offset, .extent = glyph_layout.extent},
                       atlas.glyphs[glyph_layout.glyph].texture_region,
                       props.foreground_color);
          }
        }
        else
        {
          draw_image(atlas.texture,
                     rect{.offset = position + glyph_layout.offset, .extent = glyph_layout.extent},
                     atlas.glyphs[glyph_layout.glyph].texture_region,
                     colors::WHITE);
        }
      }
    }

    for (TextRunSubWord const &subword : layout.subwords)
    {
      TextProps const &props = paragraph.runs[subword.run].props.as_cref().unwrap_or(paragraph.props);

      if (props.strikethrough_color.is_visible() && props.strikethrough_thickness > 0)
      {
        vec2 pos = position + subword.area.line_top;
        pos.y += (subword.area.baseline.y - subword.area.line_top.y) / 2;
        pos.y -= props.strikethrough_thickness / 2;
        draw_rect_filled(rect{.offset = pos, .extent = vec2{subword.area.extent.x, props.strikethrough_thickness}}, props.strikethrough_color);
      }

      if (props.underline_color.is_visible() && props.underline_thickness > 0)
      {
        draw_rect_filled(rect{.offset = position + subword.area.baseline, .extent = vec2{subword.area.extent.x, props.underline_thickness}}, props.underline_color);
      }
    }

    return *this;
  }
};

}        // namespace gfx
}        // namespace ash
