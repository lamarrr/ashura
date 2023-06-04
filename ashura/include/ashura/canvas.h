#pragma once

#include <algorithm>
#include <cinttypes>
#include <cmath>
#include <utility>

#include "ashura/font.h"
#include "ashura/image.h"
#include "ashura/primitives.h"
#include "ashura/text.h"
#include "stx/text.h"
#include "stx/vec.h"

namespace ash
{
namespace gfx
{

namespace polygons
{

inline void rect(vec2 extent, vec4 color, rect_uv texture_region, stx::Span<vertex> out_polygon)
{
  vec2 p2 = vec2{extent.x, 0};
  vec2 p3 = extent;
  vec2 p4 = vec2{0, extent.y};

  vertex vertices[] = {{.position = {}, .uv = texture_region.uv0, .color = color},
                       {.position = p2, .uv = vec2{texture_region.uv1.x, texture_region.uv0.y}, .color = color},
                       {.position = p3, .uv = texture_region.uv1, .color = color},
                       {.position = p4, .uv = vec2{texture_region.uv0.x, texture_region.uv1.y}, .color = color}};

  out_polygon.copy(vertices);
}

inline void circle(f32 radius, u32 nsegments, vec4 color, rect_uv texture_region, stx::Span<vertex> out_polygon)
{
  if (nsegments == 0 || radius <= 0)
  {
    return;
  }

  f32 step = (2 * PI) / nsegments;

  for (u32 i = 0; i < nsegments; i++)
  {
    vec2 p                       = radius + radius *vec2{std::cos(i * step), std::sin(i * step)};
    vec2                      uv = texture_region.uv0 + p / (radius * 2) * (texture_region.uv1 - texture_region.uv0);

    out_polygon[i] = vertex{.position = p, .uv = uv, .color = color};
  }
}

inline void ellipse(vec2 radii, u32 nsegments, vec4 color, rect_uv texture_region, stx::Span<vertex> polygon)
{
  if (nsegments == 0 || radii.x <= 0 || radii.y <= 0)
  {
    return;
  }

  f32 step = (2 * PI) / nsegments;

  for (u32 i = 0; i < nsegments; i++)
  {
    vec2 p                     = radii + radii *vec2{std::cos(i * step), std::sin(i * step)};
    vec2                    uv = texture_region.uv0 + p / (2 * radii) * (texture_region.uv1 - texture_region.uv0);
    polygon[i]                 = vertex{.position = p, .uv = uv, .color = color};
  }
}

// TODO(lamarrr): clamp border radius from going berserk
/// outputs nsegments*4 vertices
inline void round_rect(vec2 extent, vec4 radii, u32 nsegments, vec4 color, rect_uv texture_region, stx::Span<vertex> polygon)
{
  if (nsegments == 0)
  {
    return;
  }

  radii.x = std::max(0.0f, std::min(radii.x, std::min(extent.x, extent.y)));
  radii.y = std::max(0.0f, std::min(radii.y, std::min(extent.x, extent.y)));
  radii.z = std::max(0.0f, std::min(radii.z, std::min(extent.x, extent.y)));
  radii.w = std::max(0.0f, std::min(radii.w, std::min(extent.x, extent.y)));

  f32 step = (PI / 2) / nsegments;

  usize i = 0;

  for (u32 segment = 0; segment < nsegments; segment++, i++)
  {
    vec2 p = (extent - radii.z) + radii.z *vec2{std::cos(segment * step), std::sin(segment * step)};

    vec2 uv = texture_region.uv0 + p / extent * (texture_region.uv1 - texture_region.uv0);

    polygon[i] = vertex{.position = p, .uv = uv, .color = color};
  }

  for (u32 segment = 0; segment < nsegments; segment++, i++)
  {
    vec2 p = vec2{radii.w, extent.y - radii.w} + radii.w *vec2{std::cos(PI / 2 + segment * step), std::sin(PI / 2 + segment * step)};

    vec2 uv = texture_region.uv0 + p / extent * (texture_region.uv1 - texture_region.uv0);

    polygon[i] = vertex{.position = p, .uv = uv, .color = color};
  }

  for (u32 segment = 0; segment < nsegments; segment++, i++)
  {
    vec2 p = radii.x + radii.x *vec2{std::cos(PI + segment * step), std::sin(PI + segment * step)};

    vec2 uv = texture_region.uv0 + p / extent * (texture_region.uv1 - texture_region.uv0);

    polygon[i] = vertex{.position = p, .uv = uv, .color = color};
  }

  for (u32 segment = 0; segment < nsegments; segment++, i++)
  {
    vec2 p = vec2{extent.x - radii.y, radii.y} + radii.y *vec2{std::cos(PI * 3.0f / 2.0f + segment * step), std::sin(PI * 3.0f / 2.0f + segment * step)};

    vec2 uv = texture_region.uv0 + p / extent * (texture_region.uv1 - texture_region.uv0);

    polygon[i] = vertex{.position = p, .uv = uv, .color = color};
  }
}

}        // namespace polygons

// outputs (n-2)*3 indices
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

// line joint is a bevel joint
inline void add_line_stroke(vec2 p0, vec2 p1, f32 thickness, vec2 extent, vec4 color, rect_uv texture_region, stx::Vec<vertex> &out)
{
  // the angles are specified in clockwise direction to be compatible with the
  // vulkan coordinate system
  //
  // get the angle of inclination of p2 to p1
  vec2 d     = p1 - p0;
  f32  grad  = std::abs(d.y / std::max(stx::F32_EPSILON, d.x));
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

  vec2 p0_0_uv = texture_region.uv0 + p0_0 / extent * (texture_region.uv1 - texture_region.uv0);
  vec2 p0_1_uv = texture_region.uv0 + p0_1 / extent * (texture_region.uv1 - texture_region.uv0);
  vec2 p1_0_uv = texture_region.uv0 + p1_0 / extent * (texture_region.uv1 - texture_region.uv0);
  vec2 p1_1_uv = texture_region.uv0 + p1_1 / extent * (texture_region.uv1 - texture_region.uv0);

  vertex vertices[] = {{.position = p0_0, .uv = p0_0_uv, .color = color},
                       {.position = p0_1, .uv = p0_1_uv, .color = color},
                       {.position = p1_0, .uv = p1_0_uv, .color = color},
                       {.position = p1_1, .uv = p1_1_uv, .color = color}};

  out.extend(vertices).unwrap();
}

// line joint is a bevel joint, it is the most efficient since it re-uses existing vertices and doesn't require generating new vertices
inline void triangulate_line(stx::Span<vertex const> in_points, vec2 extent, f32 thickness, rect_uv texture_region, stx::Vec<vertex> &out_vertices, stx::Vec<u32> &out_indices, bool should_close)
{
  if (in_points.size() < 2)
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

    add_line_stroke(p0, p1, thickness, extent, color, texture_region, out_vertices);

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

    add_line_stroke(p0, p1, thickness, extent, color, texture_region, out_vertices);

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
  u32   nvertices = 0;
  u32   nindices  = 0;
  rect  clip_rect;
  mat4  transform;
  image texture = WHITE_IMAGE;
};

struct DrawList
{
  stx::Vec<vertex>      vertices;
  stx::Vec<u32>         indices;
  stx::Vec<DrawCommand> cmds;

  void clear()
  {
    vertices.clear();
    indices.clear();
    cmds.clear();
  }
};

struct CanvasState
{
  mat4 transform        = mat4::identity();        // local object transform, applies to local coordinates of the objects
  mat4 global_transform = mat4::identity();        // global scene transform, applies to the global coordinate of the objects
  rect clip_rect;                                  // determines visible area of the rendering operation
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

  Canvas &restart(vec2 viewport_extent)
  {
    this->viewport_extent = viewport_extent;
    state                 = CanvasState{.clip_rect = rect{.offset = {0, 0}, .extent = viewport_extent}};
    state_stack.clear();
    draw_list.clear();
    return *this;
  }

  mat4 make_transform(vec2 position) const
  {
    return ash::translate(vec3{-1, -1, 0})                                            /// normalize to vulkan viewport coordinate range -1 to 1
           * ash::scale(vec3{2 / viewport_extent.x, 2 / viewport_extent.y, 0})        /// normalize to 0 to 2 coordinate range
           * state.global_transform                                                   /// apply global coordinate transform
           * ash::translate(vec3{position.x, position.y, 0})                          /// apply viewport positioning
           * state.transform;                                                         /// apply local coordinate transform
  }

  /// push state (transform and clips) on state stack
  Canvas &save()
  {
    state_stack.push_inplace(state).unwrap();
    return *this;
  }

  /// pop state (transform and clips) stack and restore state
  Canvas &restore()
  {
    state = state_stack.pop().unwrap_or(CanvasState{.clip_rect = rect{.offset = {0, 0}, .extent = viewport_extent}});
    return *this;
  }

  /// reset the rendering context to its default state (transform and clips)
  Canvas &reset()
  {
    state = CanvasState{.clip_rect = rect{.offset = {0, 0}, .extent = viewport_extent}};
    state_stack.clear();
    return *this;
  }

  Canvas &translate(f32 x, f32 y)
  {
    state.transform = ash::translate(vec3{x, y, 1}) * state.transform;
    return *this;
  }

  Canvas &global_translate(f32 x, f32 y)
  {
    state.global_transform = ash::translate(vec3{x, y, 1}) * state.global_transform;
    return *this;
  }

  Canvas &rotate(f32 x, f32 y, f32 z)
  {
    state.transform = ash::rotate_z(ASH_RADIANS(z)) * ash::rotate_y(ASH_RADIANS(y)) * ash::rotate_x(ASH_RADIANS(x)) * state.transform;
    return *this;
  }

  Canvas &global_rotate(f32 x, f32 y, f32 z)
  {
    state.global_transform = ash::rotate_z(ASH_RADIANS(z)) * ash::rotate_y(ASH_RADIANS(y)) * ash::rotate_x(ASH_RADIANS(x)) * state.global_transform;
    return *this;
  }

  Canvas &scale(f32 x, f32 y)
  {
    state.transform = ash::scale(vec3{x, y, 1}) * state.transform;
    return *this;
  }

  Canvas &global_scale(f32 x, f32 y)
  {
    state.global_transform = ash::scale(vec3{x, y, 1}) * state.global_transform;
    return *this;
  }

  Canvas &transform(mat4 const &t)
  {
    state.transform = t * state.transform;
    return *this;
  }

  Canvas &global_transform(mat4 const &t)
  {
    state.global_transform = t * state.global_transform;
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

    draw_list.cmds
        .push(DrawCommand{.nvertices = AS(u32, std::size(vertices)),
                          .nindices  = AS(u32, std::size(indices)),
                          .clip_rect = rect{.offset = {0, 0}, .extent = viewport_extent},
                          .transform = mat4::identity(),
                          .texture   = texture})
        .unwrap();

    return *this;
  }

  Canvas &draw_path(stx::Span<vertex const> points, rect area, rect_uv texture_region, image texture, f32 thickness, bool should_close)
  {
    if (points.size() < 2 || !area.is_visible())
    {
      return *this;
    }

    usize prev_nvertices = draw_list.vertices.size();
    usize prev_nindices  = draw_list.indices.size();

    // the input texture coordinates are not used since we need to regenerate
    // them for the line thickness
    triangulate_line(points, area.extent, thickness, texture_region, draw_list.vertices, draw_list.indices, should_close);

    usize curr_nvertices = draw_list.vertices.size();
    usize curr_nindices  = draw_list.indices.size();

    u32 nvertices = AS(u32, curr_nvertices - prev_nvertices);
    u32 nindices  = AS(u32, curr_nindices - prev_nindices);

    draw_list.cmds.push(DrawCommand{.nvertices = nvertices,
                                    .nindices  = nindices,
                                    .clip_rect = state.clip_rect,
                                    .transform = make_transform(area.offset),
                                    .texture   = texture})
        .unwrap();

    return *this;
  }

  stx::Span<vertex> __reserve_convex_polygon(u32 npoints, rect area, image texture)
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

    draw_list.cmds
        .push(DrawCommand{.nvertices = nvertices,
                          .nindices  = nindices,
                          .clip_rect = state.clip_rect,
                          .transform = make_transform(area.offset),
                          .texture   = texture})
        .unwrap();

    return polygon;
  }

  Canvas &draw_convex_polygon_filled(stx::Span<vertex const> polygon, rect area, image texture)
  {
    if (polygon.size() < 3 || !area.is_visible())
    {
      return *this;
    }

    __reserve_convex_polygon(AS(u32, polygon.size()), area, texture).copy(polygon);

    return *this;
  }

  Canvas &draw_line(vec2 begin, vec2 end, color color, f32 thickness, image texture = WHITE_IMAGE, rect_uv texture_region = rect_uv{.uv0 = {0, 0}, .uv1 = {1, 1}})
  {
    vec4   color_v  = color.to_vec();
    vertex points[] = {{.position = {}, .uv = {}, .color = color_v},
                       {.position = end - begin, .uv = {}, .color = color_v}};

    rect area{.offset = begin, .extent = end - begin};

    return draw_path(points, area, texture_region, texture, thickness, false);
  }

  Canvas &draw_rect_filled(rect area, color color, image texture = WHITE_IMAGE, rect_uv texture_region = rect_uv{.uv0 = {0, 0}, .uv1 = {1, 1}})
  {
    polygons::rect(area.extent, color.to_vec(), texture_region, __reserve_convex_polygon(4, area, texture));
    return *this;
  }

  Canvas &draw_rect_stroke(rect area, color color, f32 thickness, image texture = WHITE_IMAGE, rect_uv texture_region = rect_uv{.uv0 = {0, 0}, .uv1 = {1, 1}})
  {
    vertex line[4];

    polygons::rect(area.extent, color.to_vec(), texture_region, line);

    area.offset = area.offset - thickness / 2;
    area.extent = area.extent + thickness;

    return draw_path(line, area, texture_region, texture, thickness, true);
  }

  Canvas &draw_circle_filled(vec2 position, f32 radius, u32 nsegments, color color, image texture = WHITE_IMAGE, rect_uv texture_region = rect_uv{.uv0 = {0, 0}, .uv1 = {1, 1}})
  {
    rect area{.offset = position, .extent = vec2::splat(2 * radius)};
    polygons::circle(radius, nsegments, color.to_vec(), texture_region, __reserve_convex_polygon(nsegments, area, texture));
    return *this;
  }

  Canvas &draw_circle_stroke(vec2 position, f32 radius, u32 nsegments, color color, f32 thickness, image texture = WHITE_IMAGE, rect_uv texture_region = rect_uv{.uv0 = {0, 0}, .uv1 = {1, 1}})
  {
    scratch.unsafe_resize_uninitialized(nsegments).unwrap();

    polygons::circle(radius, nsegments, color.to_vec(), texture_region, scratch);

    rect area{.offset = position - vec2{thickness / 2, thickness / 2}, .extent = vec2::splat(2 * radius) + vec2{thickness, thickness}};

    return draw_path(scratch, area, texture_region, texture, thickness, true);
  }

  Canvas &draw_ellipse_filled(vec2 position, vec2 radii, u32 nsegments, color color, image texture = WHITE_IMAGE, rect_uv texture_region = rect_uv{.uv0 = {0, 0}, .uv1 = {1, 1}})
  {
    rect area{.offset = position, .extent = 2 * radii};
    polygons::ellipse(radii, nsegments, color.to_vec(), texture_region, __reserve_convex_polygon(nsegments, area, texture));
    return *this;
  }

  Canvas &draw_ellipse_stroke(vec2 position, vec2 radii, u32 nsegments, color color, f32 thickness, image texture = WHITE_IMAGE, rect_uv texture_region = rect_uv{.uv0 = {0, 0}, .uv1 = {1, 1}})
  {
    scratch.unsafe_resize_uninitialized(nsegments).unwrap();

    polygons::ellipse(radii, nsegments, color.to_vec(), texture_region, scratch);

    rect area{.offset = position - vec2::splat(thickness / 2), .extent = (2 * radii) + vec2::splat(thickness)};

    return draw_path(scratch, area, texture_region, texture, thickness, true);
  }

  Canvas &draw_round_rect_filled(rect area, vec4 radii, u32 nsegments, color color, image texture = WHITE_IMAGE, rect_uv texture_region = rect_uv{.uv0 = {0, 0}, .uv1 = {1, 1}})
  {
    polygons::round_rect(area.extent, radii, nsegments, color.to_vec(), texture_region, __reserve_convex_polygon(nsegments * 4, area, texture));
    return *this;
  }

  Canvas &draw_round_rect_stroke(rect area, vec4 radii, color color, f32 thickness, u32 nsegments, image texture = WHITE_IMAGE, rect_uv texture_region = rect_uv{.uv0 = {0, 0}, .uv1 = {1, 1}})
  {
    scratch.unsafe_resize_uninitialized(nsegments * 4).unwrap();

    polygons::round_rect(area.extent, radii, nsegments, color.to_vec(), texture_region, scratch);

    area.offset = area.offset - vec2::splat(thickness / 2);
    area.extent = area.extent + vec2::splat(thickness);

    return draw_path(scratch, area, texture_region, texture, thickness, true);
  }

  Canvas &draw_image(image img, rect area, rect_uv texture_region, color tint = colors::WHITE)
  {
    polygons::rect(area.extent, tint.to_vec(), texture_region, __reserve_convex_polygon(4, area, img));
    return *this;
  }

  Canvas &draw_image(image img, rect area, color tint = colors::WHITE)
  {
    return draw_image(img, area, rect_uv{.uv0 = {0, 0}, .uv1 = {1, 1}}, tint);
  }

  Canvas &draw_rounded_image(image img, rect area, vec4 border_radii, u32 nsegments, rect_uv texture_region, color tint = colors::WHITE)
  {
    polygons::round_rect(area.extent, border_radii, nsegments, tint.to_vec(), texture_region, __reserve_convex_polygon(nsegments * 4, area, img));
    return *this;
  }

  Canvas &draw_rounded_image(image img, rect area, vec4 border_radii, u32 nsegments, color tint = colors::WHITE)
  {
    return draw_rounded_image(img, area, border_radii, nsegments, rect_uv{.uv0 = {0, 0}, .uv1 = {1, 1}}, tint);
  }

  Canvas &draw_sine_wave(f32 thickness, rect area, color color);

  Canvas &draw_text(Paragraph const &paragraph, TextLayout const &layout, stx::Span<BundledFont const> font_bundle, vec2 const position)
  {

// TODO(lamarrr): when rendering glyphs and strokes we need to use the x offset of the glyph as well. this is probably why the arabic text rendering looked weird


    // draw empty spaces background
    for (SpaceLayout const &space_layout : layout.space_layouts)
    {
      TextProps const &props = layout.subwords[space_layout.subword].props;

      if (props.background_color.is_visible())
      {
        vec2 offset = position + space_layout.baseline_position - vec2{0, space_layout.line_height};
        vec2 extent = vec2{space_layout.width, space_layout.line_height};
        draw_rect_filled(rect{.offset = offset, .extent = extent}, props.background_color);
      }
    }

    // draw glyph backgrounds
    for (GlyphLayout const &glyph_layout : layout.glyph_layouts)
    {
      TextProps const &props      = layout.subwords[glyph_layout.subword].props;
      f32              font_scale = props.font_height / font_bundle[glyph_layout.font].atlas.font_height;
      vec2             advance    = font_scale * font_bundle[glyph_layout.font].atlas.glyphs[glyph_layout.glyph].advance;

      if (props.background_color.is_visible())
      {
        vec2 offset = position + glyph_layout.baseline_position - vec2{0, glyph_layout.line_height};
        vec2 extent = vec2{advance.x + props.letter_spacing, glyph_layout.line_height};
        draw_rect_filled(rect{.offset = offset, .extent = extent}, props.background_color);
      }
    }

    // draw glyph strokes
    for (GlyphLayout const &glyph_layout : layout.glyph_layouts)
    {
      TextProps const &props = layout.subwords[glyph_layout.subword].props;

      if (props.stroke_color.is_transparent() || font_bundle[glyph_layout.font].stroke_atlas.is_none())
      {
        continue;
      }

      FontAtlas const       &atlas        = font_bundle[glyph_layout.font].atlas;
      FontStrokeAtlas const &stroke_atlas = font_bundle[glyph_layout.font].stroke_atlas.value();
      f32                    font_scale   = props.font_height / atlas.font_height;
      Glyph const           &glyph        = atlas.glyphs[glyph_layout.glyph];
      GlyphStroke const     &stroke       = stroke_atlas.strokes[glyph_layout.glyph];
      f32                    ascent       = font_scale * glyph.ascent;
      vec2                   glyph_extent{font_scale * glyph.extent.width, font_scale * glyph.extent.height};
      vec2                   extent{font_scale * stroke.extent.width, font_scale * stroke.extent.height};

      // position stroke center on the center of the glyph by default
      vec2 stroke_alignment = (extent - glyph_extent) / 2;
      vec2 offset           = (position + glyph_layout.baseline_position - vec2{0, glyph_layout.vert_spacing + ascent}) - stroke_alignment + props.stroke_offset;
      draw_image(stroke_atlas.texture, rect{.offset = offset, .extent = extent}, stroke.texture_region, props.stroke_color);
    }

    // draw glyphs
    for (GlyphLayout const &glyph_layout : layout.glyph_layouts)
    {
      TextProps const &props = layout.subwords[glyph_layout.subword].props;

      if (props.foreground_color.is_transparent())
      {
        continue;
      }

      FontAtlas const &atlas      = font_bundle[glyph_layout.font].atlas;
      f32              font_scale = props.font_height / atlas.font_height;
      Glyph const     &glyph      = atlas.glyphs[glyph_layout.glyph];
      f32              ascent     = font_scale * glyph.ascent;
      vec2             advance    = font_scale * glyph.advance;
      vec2             offset     = position + glyph_layout.baseline_position - vec2{0, glyph_layout.vert_spacing + ascent};
      vec2             extent{font_scale * glyph.extent.width, font_scale * glyph.extent.height};

      draw_image(atlas.texture, rect{.offset = offset, .extent = extent}, glyph.texture_region, props.foreground_color);
    }

    // draw underline and strike throughs for spaces
    for (SpaceLayout const &space_layout : layout.space_layouts)
    {
      TextProps const &props = layout.subwords[space_layout.subword].props;

      if (props.strikethrough_color.is_visible() && props.strikethrough_thickness > 0)
      {
        vec2 offset = position + space_layout.baseline_position - vec2{0, space_layout.line_height / 2 + props.strikethrough_thickness / 2};
        vec2 extent = vec2{space_layout.width, props.strikethrough_thickness};
        draw_rect_filled(rect{.offset = offset, .extent = extent}, props.strikethrough_color);
      }

      if (props.underline_color.is_visible() && props.underline_thickness > 0)
      {
        vec2 offset = position + space_layout.baseline_position;
        vec2 extent = vec2{space_layout.width, props.underline_thickness};
        draw_rect_filled(rect{.offset = offset, .extent = extent}, props.underline_color);
      }
    }

    // draw underline and strike throughs for glyphs
    for (GlyphLayout const &glyph_layout : layout.glyph_layouts)
    {
      TextProps const &props      = layout.subwords[glyph_layout.subword].props;
      FontAtlas const &atlas      = font_bundle[glyph_layout.font].atlas;
      f32              font_scale = props.font_height / atlas.font_height;
      Glyph const     &glyph      = atlas.glyphs[glyph_layout.glyph];
      vec2             advance    = font_scale * glyph.advance;

      if (props.strikethrough_color.is_visible() && props.strikethrough_thickness > 0)
      {
        vec2 offset = position + glyph_layout.baseline_position - vec2{0, glyph_layout.line_height / 2 + props.strikethrough_thickness / 2};
        vec2 extent = vec2{advance.x + props.letter_spacing, props.strikethrough_thickness};
        draw_rect_filled(rect{.offset = offset, .extent = extent}, props.strikethrough_color);
      }

      if (props.underline_color.is_visible() && props.underline_thickness > 0)
      {
        vec2 offset = position + glyph_layout.baseline_position;
        vec2 extent = vec2{advance.x + props.letter_spacing, props.underline_thickness};
        draw_rect_filled(rect{.offset = offset, .extent = extent}, props.underline_color);
      }
    }

    return *this;
  }
};

}        // namespace gfx
}        // namespace ash
