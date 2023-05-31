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

struct DrawCommand
{
  u32   nvertices = 0;
  u32   nindices  = 0;
  rect  clip_rect;
  mat4  transform = mat4::identity();
  image texture   = WHITE_IMAGE;
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

namespace polygons
{

inline void rect(vec2 position, vec2 extent, mat4 const &transform, vec4 color, ash::rect texture_area, stx::Span<vertex> polygon)
{
  vec2 p2 = vec2{extent.x, 0};
  vec2 p3 = extent;
  vec2 p4 = vec2{0, extent.y};

  vec2 st0 = texture_area.offset;
  vec2 st1 = texture_area.offset + p2 / extent * texture_area.extent;
  vec2 st2 = texture_area.offset + p3 / extent * texture_area.extent;
  vec2 st3 = texture_area.offset + p4 / extent * texture_area.extent;

  vertex vertices[] = {{.position = position, .st = st0, .color = color},
                       {.position = position + ash::transform(transform, p2), .st = st1, .color = color},
                       {.position = position + ash::transform(transform, p3), .st = st2, .color = color},
                       {.position = position + ash::transform(transform, p4), .st = st3, .color = color}};

  polygon.copy(vertices);
}

inline void circle(vec2 position, f32 radius, u32 nsegments, mat4 const &transform, vec4 color, ash::rect texture_area, stx::Span<vertex> polygon)
{
  if (nsegments == 0 || radius <= 0)
  {
    return;
  }

  f32 step = (2 * PI) / nsegments;

  for (u32 i = 0; i < nsegments; i++)
  {
    vec2 p                       = radius + radius *vec2{std::cos(i * step), std::sin(i * step)};
    vec2                      st = texture_area.offset + p / (radius * 2) * texture_area.extent;

    polygon[i] = vertex{.position = position + ash::transform(transform, p), .st = st, .color = color};
  }
}

inline void ellipse(vec2 position, vec2 radii, u32 nsegments, mat4 const &transform, vec4 color, ash::rect texture_area, stx::Span<vertex> polygon)
{
  if (nsegments == 0 || radii.x <= 0 || radii.y <= 0)
  {
    return;
  }

  f32 step = (2 * PI) / nsegments;

  for (u32 i = 0; i < nsegments; i++)
  {
    vec2 p                     = radii + radii *vec2{std::cos(i * step), std::sin(i * step)};
    vec2                    st = texture_area.offset + p / (2 * radii) * texture_area.extent;
    polygon[i]                 = vertex{.position = position + ash::transform(transform, p), .st = st, .color = color};
  }
}

// TODO(lamarrr): clamp border radius from going berserk
/// outputs nsegments*4 vertices
inline void round_rect(vec2 position, vec2 extent, vec4 radii, u32 nsegments, mat4 const &transform, vec4 color, ash::rect texture_area, stx::Span<vertex> polygon)
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

    vec2 st = texture_area.offset + p / extent * texture_area.extent;

    polygon[i] = vertex{.position = position + ash::transform(transform, p), .st = st, .color = color};
  }

  for (u32 segment = 0; segment < nsegments; segment++, i++)
  {
    vec2 p = vec2{radii.w, extent.y - radii.w} + radii.w *vec2{std::cos(AS(f32, PI / 2) + segment * step), std::sin(AS(f32, PI / 2) + segment * step)};

    vec2 st = texture_area.offset + p / extent * texture_area.extent;

    polygon[i] = vertex{.position = position + ash::transform(transform, p), .st = st, .color = color};
  }

  for (u32 segment = 0; segment < nsegments; segment++, i++)
  {
    vec2 p = radii.x + radii.x *vec2{std::cos(AS(f32, PI) + segment * step), std::sin(AS(f32, PI) + segment * step)};

    vec2 st = texture_area.offset + p / extent * texture_area.extent;

    polygon[i] = vertex{.position = position + ash::transform(transform, p), .st = st, .color = color};
  }

  for (u32 segment = 0; segment < nsegments; segment++, i++)
  {
    vec2 p = vec2{extent.x - radii.y, radii.y} + radii.y *vec2{std::cos(AS(f32, PI * 3) / 2 + segment * step), std::sin(AS(f32, PI * 3) / 2 + segment * step)};

    vec2 st = texture_area.offset + p / extent * texture_area.extent;

    polygon[i] = vertex{.position = position + ash::transform(transform, p), .st = st, .color = color};
  }
}

}        // namespace polygons

constexpr void normalize_for_viewport(stx::Span<vertex> vertices, vec2 viewport_extent)
{
  for (vertex &v : vertices)
  {
    // transform to -1 to +1 range from x pointing right and y pointing downwards
    v.position = (2 * v.position / viewport_extent) - 1;
  }
}

// outputs (n-2)*3 vertices
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

inline void triangulate_line(vec2 position, stx::Span<vertex const> in_vertices, vec2 extent, mat4 const &transform, f32 thickness, ash::rect texture_area, stx::Vec<vertex> &out_vertices, stx::Vec<u32> &out_indices)
{
  if (in_vertices.size() < 2)
  {
    return;
  }

  bool has_previous_line = false;

  u32 vertex_index = 0;

  for (u32 i = 1; i < AS(u32, in_vertices.size()); i++)
  {
    vec4 color = in_vertices[i].color;
    vec2 p0    = in_vertices[i - 1].position;
    vec2 p1    = in_vertices[i].position;

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
    vec2 g = thickness / 2 * vec2{std::cos(AS(f32, PI + alpha)), std::sin(AS(f32, PI + alpha))};

    vec2 m0 = p0 + f;
    vec2 m1 = p0 + g;

    vec2 n0 = p1 + f;
    vec2 n1 = p1 + g;

    vec2 st0 = texture_area.offset + m0 / extent * texture_area.extent;
    vec2 st1 = texture_area.offset + m1 / extent * texture_area.extent;
    vec2 st2 = texture_area.offset + n0 / extent * texture_area.extent;
    vec2 st3 = texture_area.offset + n1 / extent * texture_area.extent;

    vertex vertices[] = {{.position = position + ash::transform(transform, m0), .st = st0, .color = color},
                         {.position = position + ash::transform(transform, m1), .st = st1, .color = color},
                         {.position = position + ash::transform(transform, n0), .st = st2, .color = color},
                         {.position = position + ash::transform(transform, n1), .st = st3, .color = color}};

    u32 indices[] = {vertex_index, vertex_index + 1, vertex_index + 3, vertex_index, vertex_index + 2, vertex_index + 3};

    out_vertices.extend(vertices).unwrap();
    out_indices.extend(indices).unwrap();

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
}

struct CanvasState
{
  mat4 transform        = mat4::identity();
  mat4 global_transform = mat4::identity();
  rect clip_rect;
};

// In order to implement command culling we might need to sync with or remove
// global transform
//
//
/// coordinates are specified in top-left origin space with x pointing to the
/// right and y pointing downwards (Vulkan Coordinate System).
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
  mat4                  transform        = mat4::identity();        // local object transform, applies to individual objects
  mat4                  global_transform = mat4::identity();        // global scene transform, applies to all objects in the scene, this is the first transform applied to the objects
  rect                  clip_rect;
  stx::Vec<CanvasState> state_stack;
  DrawList              draw_list;

  Canvas &begin(vec2 new_viewport_extent)
  {
    viewport_extent  = new_viewport_extent;
    transform        = mat4::identity();
    global_transform = mat4::identity();
    clip_rect        = rect{.offset = {0, 0}, .extent = viewport_extent};
    state_stack.clear();
    draw_list.clear();
    return *this;
  }

  /// push state (transform and clips) on state stack
  Canvas &save()
  {
    state_stack.push(CanvasState{.transform = transform, .global_transform = global_transform, .clip_rect = clip_rect}).unwrap();
    return *this;
  }

  /// save current transform and clip state
  /// pop state (transform and clips) stack and restore state
  Canvas &restore()
  {
    CanvasState state = state_stack.pop().unwrap_or(CanvasState{});
    transform         = state.transform;
    global_transform  = state.global_transform;
    clip_rect         = state.clip_rect;
    return *this;
  }

  /// reset the rendering context to its default state (transform
  /// and clips)
  Canvas &reset()
  {
    transform        = mat4::identity();
    global_transform = mat4::identity();
    clip_rect        = rect{.offset = {0, 0}, .extent = viewport_extent};
    state_stack.clear();
    return *this;
  }

  Canvas &translate(f32 x, f32 y)
  {
    vec3 translation = 2 * vec3{x, y, 1} / vec3{viewport_extent.x, viewport_extent.y, 1};
    transform        = ash::translate(translation) * transform;
    return *this;
  }

  Canvas &global_translate(f32 x, f32 y)
  {
    vec3 translation = 2 * vec3{x, y, 1} / vec3{viewport_extent.x, viewport_extent.y, 1};
    global_transform = ash::translate(translation) * global_transform;
    return *this;
  }

  Canvas &rotate(f32 x, f32 y, f32 z)
  {
    transform = ash::rotate_z(ASH_RADIANS(z)) * ash::rotate_y(ASH_RADIANS(y)) * ash::rotate_x(ASH_RADIANS(x)) * transform;
    return *this;
  }

  Canvas &global_rotate(f32 x, f32 y, f32 z)
  {
    global_transform = ash::rotate_z(ASH_RADIANS(z)) * ash::rotate_y(ASH_RADIANS(y)) * ash::rotate_x(ASH_RADIANS(x)) * global_transform;
    return *this;
  }

  Canvas &scale(f32 x, f32 y)
  {
    transform = ash::scale(vec3{x, y, 1}) * transform;
    return *this;
  }

  Canvas &global_scale(f32 x, f32 y)
  {
    global_transform = ash::scale(vec3{x, y, 1}) * global_transform;
    return *this;
  }

  Canvas &clear(color clear_color, image texture = WHITE_IMAGE)
  {
    draw_list.clear();

    vec4 color = clear_color.as_vec();

    vertex vertices[] = {{.position = {0, 0}, .st = {0, 0}, .color = color},
                         {.position = {viewport_extent.x, 0}, .st = {1, 0}, .color = color},
                         {.position = viewport_extent, .st = {1, 1}, .color = color},
                         {.position = {0, viewport_extent.y}, .st = {0, 1}, .color = color}};

    normalize_for_viewport(vertices, viewport_extent);

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

  Canvas &draw_lines(stx::Span<vertex const> points, rect area, rect texture_area, image texture, f32 thickness)
  {
    if (points.size() < 2 || !area.is_visible())
    {
      return *this;
    }

    usize prev_nvertices = draw_list.vertices.size();
    usize prev_nindices  = draw_list.indices.size();

    // the input texture coordinates are not used since we need to regenerate
    // them for the line thickness
    triangulate_line(area.offset, points, area.extent, transform, thickness, texture_area, draw_list.vertices, draw_list.indices);

    usize curr_nvertices = draw_list.vertices.size();
    usize curr_nindices  = draw_list.indices.size();

    u32 nvertices = AS(u32, curr_nvertices - prev_nvertices);
    u32 nindices  = AS(u32, curr_nindices - prev_nindices);

    normalize_for_viewport(draw_list.vertices.span().slice(prev_nvertices), viewport_extent);

    draw_list.cmds.push(DrawCommand{.nvertices = nvertices,
                                    .nindices  = nindices,
                                    .clip_rect = clip_rect,
                                    .transform = global_transform,
                                    .texture   = texture})
        .unwrap();

    return *this;
  }

  Canvas &draw_convex_polygon_filled(stx::Span<vertex const> polygon, rect area, image texture)
  {
    if (polygon.size() < 3 || !area.is_visible())
    {
      return *this;
    }

    usize prev_nvertices = draw_list.vertices.size();
    usize prev_nindices  = draw_list.indices.size();

    triangulate_convex_polygon(draw_list.indices, AS(u32, polygon.size()));

    draw_list.vertices.extend(polygon).unwrap();

    usize curr_nvertices = draw_list.vertices.size();
    usize curr_nindices  = draw_list.indices.size();

    normalize_for_viewport(draw_list.vertices.span().slice(prev_nvertices), viewport_extent);

    u32 nvertices = AS(u32, curr_nvertices - prev_nvertices);
    u32 nindices  = AS(u32, curr_nindices - prev_nindices);

    draw_list.cmds
        .push(DrawCommand{.nvertices = nvertices,
                          .nindices  = nindices,
                          .clip_rect = clip_rect,
                          .transform = global_transform,
                          .texture   = texture})
        .unwrap();

    return *this;
  }

  Canvas &draw_line(vec2 p1, vec2 p2, color color, f32 thickness, image texture = WHITE_IMAGE)
  {
    vec4   color_v  = color.as_vec();
    vertex points[] = {{.position = p1, .st = {}, .color = color_v},
                       {.position = p1 + ash::transform(transform, p2 - p1), .st = {}, .color = color_v}};

    rect area{.offset = p1, .extent = p2 - p1};

    return draw_lines(points, area, rect{.offset = {0, 0}, .extent = {1, 1}}, texture, thickness);
  }

  Canvas &draw_rect_filled(rect area, color color, image texture = WHITE_IMAGE)
  {
    vertex vertices[4];

    rect texture_area{.offset = {0, 0}, .extent = {1, 1}};

    polygons::rect(area.offset, area.extent, transform, color.as_vec(), texture_area, vertices);

    return draw_convex_polygon_filled(vertices, area, texture);
  }

  Canvas &draw_rect_stroke(rect area, color color, f32 thickness, image texture = WHITE_IMAGE)
  {
    vertex vertices[4];

    rect texture_area{.offset = {0, 0}, .extent = {1, 1}};

    polygons::rect(area.offset, area.extent, transform, color.as_vec(), texture_area, vertices);

    area.offset = area.offset - thickness / 2;
    area.extent = area.extent + thickness;

    vertex lines[] = {vertices[0], vertices[1], vertices[2], vertices[3], vertices[0], vertices[1]};

    return draw_lines(lines, area, texture_area, texture, thickness);
  }

  Canvas &draw_circle_filled(vec2 position, f32 radius, u32 nsegments, color color, image texture = WHITE_IMAGE)
  {
    stx::Vec<vertex> vertices;
    vertices.unsafe_resize_uninitialized(nsegments).unwrap();

    rect texture_area{.offset = {0, 0}, .extent = {1, 1}};
    polygons::circle(position, radius, nsegments, transform, color.as_vec(), texture_area, vertices);

    rect area{.offset = position, .extent = vec2::splat(2 * radius)};

    return draw_convex_polygon_filled(vertices, area, texture);
  }

  Canvas &draw_circle_stroke(vec2 position, f32 radius, u32 nsegments, color color, f32 thickness, image texture = WHITE_IMAGE)
  {
    stx::Vec<vertex> vertices;
    vertices.unsafe_resize_uninitialized(nsegments).unwrap();

    rect texture_area{.offset = {0, 0}, .extent = {1, 1}};
    polygons::circle(position, radius, nsegments, transform, color.as_vec(), texture_area, vertices);

    rect area{.offset = position - vec2{thickness / 2, thickness / 2}, .extent = vec2::splat(2 * radius) + vec2{thickness, thickness}};

    if (vertices.size() > 0)
    {
      vertices.push_inplace(vertices[0]).unwrap();
    }
    if (vertices.size() > 1)
    {
      vertices.push_inplace(vertices[1]).unwrap();
    }

    return draw_lines(vertices, area, texture_area, texture, thickness);
  }

  Canvas &draw_ellipse_filled(vec2 position, vec2 radii, u32 nsegments, color color, image texture = WHITE_IMAGE)
  {
    stx::Vec<vertex> vertices;
    vertices.unsafe_resize_uninitialized(nsegments).unwrap();

    rect texture_area{.offset = {0, 0}, .extent = {1, 1}};

    polygons::ellipse(position, radii, nsegments, transform, color.as_vec(), texture_area, vertices);

    rect area{.offset = position, .extent = 2 * radii};

    return draw_convex_polygon_filled(vertices, area, texture);
  }

  Canvas &draw_ellipse_filled(vec2 position, vec2 radii, u32 nsegments, color color, f32 thickness, image texture = WHITE_IMAGE)
  {
    stx::Vec<vertex> vertices;
    vertices.unsafe_resize_uninitialized(nsegments).unwrap();

    rect texture_area{.offset = {0, 0}, .extent = {1, 1}};

    polygons::ellipse(position, radii, nsegments, transform, color.as_vec(), texture_area, vertices);

    rect area{.offset = position - vec2::splat(thickness / 2), .extent = (2 * radii) + vec2::splat(thickness)};

    if (vertices.size() > 0)
    {
      vertices.push_inplace(vertices[0]).unwrap();
    }
    if (vertices.size() > 1)
    {
      vertices.push_inplace(vertices[1]).unwrap();
    }

    return draw_lines(vertices, area, texture_area, texture, thickness);
  }

  Canvas &draw_round_rect_filled(rect area, vec4 radii, u32 nsegments, color color, image texture = WHITE_IMAGE)
  {
    stx::Vec<vertex> vertices;
    vertices.unsafe_resize_uninitialized(nsegments * 4).unwrap();

    rect texture_area{.offset = {0, 0}, .extent = {1, 1}};

    polygons::round_rect(area.offset, area.extent, radii, nsegments, transform, color.as_vec(), texture_area, vertices);

    return draw_convex_polygon_filled(vertices, area, texture);
  }

  Canvas &draw_round_rect_stroke(rect area, vec4 radii, u32 nsegments, color color, f32 thickness, image texture = WHITE_IMAGE)
  {
    stx::Vec<vertex> vertices;
    vertices.unsafe_resize_uninitialized(nsegments * 4).unwrap();

    rect texture_area{.offset = {0, 0}, .extent = {1, 1}};

    polygons::round_rect(area.offset, area.extent, radii, nsegments, transform, color.as_vec(), texture_area, vertices);

    if (vertices.size() > 0)
    {
      vertices.push_inplace(vertices[0]).unwrap();
    }
    if (vertices.size() > 1)
    {
      vertices.push_inplace(vertices[1]).unwrap();
    }

    area.offset = area.offset - vec2::splat(thickness / 2);
    area.extent = area.extent + vec2::splat(thickness);

    return draw_lines(vertices, area, texture_area, texture, thickness);
  }

  Canvas &draw_image(image img, rect area, f32 s0, f32 t0, f32 s1, f32 t1, color tint)
  {
    vertex vertices[4];
    rect   texture_area{.offset = {s0, t0}, .extent = {s1 - s0, t1 - t0}};

    polygons::rect(area.offset, area.extent, transform, tint.as_vec(), texture_area, vertices);

    return draw_convex_polygon_filled(vertices, area, img);
  }

  Canvas &draw_image(image img, rect area, color tint)
  {
    return draw_image(img, area, 0, 0, 1, 1, tint);
  }

  Canvas &draw_rounded_image(image img, rect area, rect image_portion, vec4 border_radii, u32 nsegments, color tint)
  {
    stx::Vec<vertex> vertices;
    vertices.unsafe_resize_uninitialized(nsegments * 4).unwrap();

    polygons::round_rect(area.offset, area.extent, border_radii, nsegments, transform, tint.as_vec(), image_portion, vertices);

    return draw_convex_polygon_filled(vertices, area, img);
  }

  Canvas &draw_rounded_image(image img, rect area, vec4 border_radii, u32 nsegments, color tint)
  {
    rect texture_area{.offset = {0, 0}, .extent = {1, 1}};

    return draw_rounded_image(img, area, texture_area, border_radii, nsegments, tint);
  }

  Canvas &draw_glyph(Glyph const &glyph, TextRun const &run, FontAtlas const &atlas, vec2 baseline, f32 line_height, f32 vert_spacing)
  {
    f32  font_scale = run.style.font_height / atlas.font_height;
    f32  ascent     = font_scale * glyph.ascent;
    vec2 advance    = font_scale * glyph.advance;
    vec2 extent{font_scale * glyph.extent.width, font_scale * glyph.extent.height};

    if (run.style.background_color.is_visible())
    {
      draw_rect_filled(rect{.offset = baseline - vec2{0, line_height}, .extent = vec2{advance.x + run.style.letter_spacing, line_height}}, run.style.background_color);
    }

    if (run.style.foreground_color.is_visible())
    {
      draw_image(atlas.texture, rect{.offset = baseline - vec2{0, vert_spacing + ascent}, .extent = extent}, glyph.s0, glyph.t0, glyph.s1, glyph.t1, atlas.has_color ? colors::WHITE : run.style.foreground_color);
    }

    if (run.style.underline_color.is_visible() && run.style.underline_thickness != 0)
    {
      draw_rect_filled(rect{.offset = baseline, .extent = vec2{advance.x + run.style.letter_spacing, run.style.underline_thickness}}, run.style.underline_color);
    }

    if (run.style.strikethrough_color.is_visible() && run.style.strikethrough_thickness != 0)
    {
      draw_rect_filled(rect{.offset = baseline - vec2{0, line_height / 2 + run.style.strikethrough_thickness / 2}, .extent = vec2{advance.x + run.style.letter_spacing, run.style.strikethrough_thickness}}, run.style.strikethrough_color);
    }

    return *this;
  }

  Canvas &draw_glyph(Glyph const &glyph, GlyphStroke const &stroke, TextRun const &run, FontAtlas const &atlas, FontStrokeAtlas const &stroke_atlas, vec2 baseline, f32 line_height, f32 vert_spacing)
  {
    f32  font_scale = run.style.font_height / atlas.font_height;
    f32  ascent     = font_scale * glyph.ascent;
    vec2 advance    = font_scale * glyph.advance;
    vec2 glyph_extent{font_scale * glyph.extent.width, font_scale * glyph.extent.height};
    vec2 stroke_extent{font_scale * stroke.extent.width, font_scale * stroke.extent.height};

    if (run.style.background_color.is_visible())
    {
      draw_rect_filled(rect{.offset = baseline - vec2{0, line_height}, .extent = vec2{advance.x + run.style.letter_spacing, line_height}}, run.style.background_color);
    }

    if (run.style.stroke_color.is_visible())
    {
      // position stroke center on the center of the glyph by default
      vec2 stroke_alignment = (stroke_extent - glyph_extent) / 2;
      draw_image(stroke_atlas.texture, rect{.offset = (baseline - vec2{0, vert_spacing + ascent}) - stroke_alignment + run.style.stroke_offset, .extent = stroke_extent}, stroke.s0, stroke.t0, stroke.s1, stroke.t1, run.style.stroke_color);
    }

    if (run.style.foreground_color.is_visible())
    {
      draw_image(atlas.texture, rect{.offset = baseline - vec2{0, vert_spacing + ascent}, .extent = glyph_extent}, glyph.s0, glyph.t0, glyph.s1, glyph.t1, run.style.foreground_color);
    }

    if (run.style.underline_color.is_visible() && run.style.underline_thickness != 0)
    {
      draw_rect_filled(rect{.offset = baseline, .extent = vec2{advance.x + run.style.letter_spacing, run.style.underline_thickness}}, run.style.underline_color);
    }

    if (run.style.strikethrough_color.is_visible() && run.style.strikethrough_thickness != 0)
    {
      draw_rect_filled(rect{.offset = baseline - vec2{0, line_height / 2 + run.style.strikethrough_thickness / 2}, .extent = vec2{advance.x + run.style.letter_spacing, run.style.strikethrough_thickness}}, run.style.strikethrough_color);
    }

    return *this;
  }

  Canvas &draw_space(TextRun const &run, vec2 baseline, f32 line_height, f32 width)
  {
    if (run.style.background_color.is_visible())
    {
      draw_rect_filled(rect{.offset = baseline - vec2{0, line_height}, .extent = vec2{width, line_height}}, run.style.background_color);
    }

    if (run.style.underline_color.is_visible() && run.style.underline_thickness != 0)
    {
      draw_rect_filled(rect{.offset = baseline, .extent = vec2{width, run.style.underline_thickness}}, run.style.underline_color);
    }

    if (run.style.strikethrough_color.is_visible() && run.style.strikethrough_thickness != 0)
    {
      draw_rect_filled(rect{.offset = baseline - vec2{0, line_height / 2 + run.style.strikethrough_thickness / 2}, .extent = vec2{width, run.style.strikethrough_thickness}}, run.style.strikethrough_color);
    }

    return *this;
  }

  Canvas &draw_text(Paragraph const &paragraph, TextLayout const &layout, stx::Span<BundledFont const> font_bundle, vec2 position)
  {
    for (SpaceLayout const &space_layout : layout.space_layouts)
    {
      draw_space(paragraph.runs[space_layout.run], position + space_layout.baseline, space_layout.line_height, space_layout.width);
    }

    for (GlyphLayout const &glyph_layout : layout.glyph_layouts)
    {
      if (paragraph.runs[glyph_layout.run].style.stroke_color.is_visible() && font_bundle[glyph_layout.font].stroke_atlas.is_some())
      {
        draw_glyph(font_bundle[glyph_layout.font].atlas.glyphs[glyph_layout.glyph],
                   font_bundle[glyph_layout.font].stroke_atlas.value().strokes[glyph_layout.glyph],
                   paragraph.runs[glyph_layout.run],
                   font_bundle[glyph_layout.font].atlas,
                   font_bundle[glyph_layout.font].stroke_atlas.value(),
                   position + glyph_layout.baseline,
                   glyph_layout.line_height,
                   glyph_layout.vert_spacing);
      }
      else
      {
        draw_glyph(font_bundle[glyph_layout.font].atlas.glyphs[glyph_layout.glyph],
                   paragraph.runs[glyph_layout.run],
                   font_bundle[glyph_layout.font].atlas,
                   position + glyph_layout.baseline, glyph_layout.line_height,
                   glyph_layout.vert_spacing);
      }
    }

    return *this;
  }
};

struct CanvasPushConstants
{
  mat4 transform = mat4::identity();
};

}        // namespace gfx
}        // namespace ash
