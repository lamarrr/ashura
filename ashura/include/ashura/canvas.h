#pragma once

#include <algorithm>
#include <cinttypes>
#include <cmath>
#include <utility>

#include "ashura/font.h"
#include "ashura/image.h"
#include "ashura/primitives.h"
#include "stx/text.h"
#include "stx/vec.h"

namespace ash {
namespace gfx {

struct Brush {
  ash::color color = colors::BLACK;
  bool fill = true;
  f32 line_thickness = 1;
  image texture = 0;
};

struct DrawCommand {
  u32 indices_offset = 0;
  u32 nindices = 0;
  rect clip_rect;
  mat4 transform = mat4::identity();
  image texture = 0;
};

struct DrawList {
  stx::Vec<vertex> vertices{stx::os_allocator};
  stx::Vec<u32> indices{stx::os_allocator};
  stx::Vec<DrawCommand> cmds{stx::os_allocator};

  void clear() {
    vertices.clear();
    indices.clear();
    cmds.clear();
  }
};

namespace polygons {

inline void rect(vec2 position, vec2 extent, mat4 const& transform, vec4 color,
                 ash::rect texture_area, stx::Span<vertex> polygon) {
  vec2 p2 = vec2{extent.x, 0};
  vec2 p3 = extent;
  vec2 p4 = vec2{0, extent.y};

  vec2 st0 = texture_area.offset;
  vec2 st1 = texture_area.offset + p2 / extent * texture_area.extent;
  vec2 st2 = texture_area.offset + p3 / extent * texture_area.extent;
  vec2 st3 = texture_area.offset + p4 / extent * texture_area.extent;

  vertex vertices[] = {{.position = position, .st = st0, .color = color},
                       {.position = position + ash::transform(transform, p2),
                        .st = st1,
                        .color = color},
                       {.position = position + ash::transform(transform, p3),
                        .st = st2,
                        .color = color},
                       {.position = position + ash::transform(transform, p4),
                        .st = st3,
                        .color = color}};

  polygon.copy(vertices);
}

inline void circle(vec2 position, f32 radius, usize nsegments,
                   mat4 const& transform, vec4 color, ash::rect texture_area,
                   stx::Span<vertex> polygon) {
  if (nsegments == 0 || radius <= 0) return;

  f32 step = AS(f32, (2 * pi) / nsegments);

  for (usize i = 0; i < nsegments; i++) {
    vec2 p = radius + radius* vec2{std::cos(i * step), std::sin(i * step)};
    vec2 st = texture_area.offset + p / (radius * 2) * texture_area.extent;

    polygon[i] = vertex{.position = position + ash::transform(transform, p),
                        .st = st,
                        .color = color};
  }
}

inline void ellipse(vec2 position, vec2 radii, usize nsegments,
                    mat4 const& transform, vec4 color, ash::rect texture_area,
                    stx::Span<vertex> polygon) {
  if (nsegments == 0 || radii.x <= 0 || radii.y <= 0) return;

  f32 step = AS(f32, (2 * pi) / nsegments);

  for (usize i = 0; i < nsegments; i++) {
    vec2 p = radii + radii* vec2{std::cos(i * step), std::sin(i * step)};
    vec2 st = texture_area.offset + p / (2 * radii) * texture_area.extent;
    polygon[i] = vertex{.position = position + ash::transform(transform, p),
                        .st = st,
                        .color = color};
  }
}

/// {polygon.size() == nsegments * 4}
// TODO(lamarrr): clamp border radius from going berserk
inline void round_rect(vec2 position, vec2 extent, vec4 radii, usize nsegments,
                       mat4 const& transform, vec4 color,
                       ash::rect texture_area, stx::Span<vertex> polygon) {
  if (nsegments == 0) return;

  radii.x = std::max(0.0f, std::min(radii.x, std::min(extent.x, extent.y)));
  radii.y = std::max(0.0f, std::min(radii.y, std::min(extent.x, extent.y)));
  radii.z = std::max(0.0f, std::min(radii.z, std::min(extent.x, extent.y)));
  radii.w = std::max(0.0f, std::min(radii.w, std::min(extent.x, extent.y)));

  f32 step = AS(f32, (pi / 2) / nsegments);

  usize i = 0;

  for (usize segment = 0; segment < nsegments; segment++, i++) {
    vec2 p = (extent - radii.z) +
             radii.z* vec2{std::cos(segment * step), std::sin(segment * step)};

    vec2 st = texture_area.offset + p / extent * texture_area.extent;

    polygon[i] = vertex{.position = position + ash::transform(transform, p),
                        .st = st,
                        .color = color};
  }

  for (usize segment = 0; segment < nsegments; segment++, i++) {
    vec2 p = vec2{radii.w, extent.y - radii.w} +
             radii.w* vec2{std::cos(AS(f32, pi / 2) + segment * step),
                           std::sin(AS(f32, pi / 2) + segment * step)};

    vec2 st = texture_area.offset + p / extent * texture_area.extent;

    polygon[i] = vertex{.position = position + ash::transform(transform, p),
                        .st = st,
                        .color = color};
  }

  for (usize segment = 0; segment < nsegments; segment++, i++) {
    vec2 p = radii.x + radii.x* vec2{std::cos(AS(f32, pi) + segment * step),
                                     std::sin(AS(f32, pi) + segment * step)};

    vec2 st = texture_area.offset + p / extent * texture_area.extent;

    polygon[i] = vertex{.position = position + ash::transform(transform, p),
                        .st = st,
                        .color = color};
  }

  for (usize segment = 0; segment < nsegments; segment++, i++) {
    vec2 p = vec2{extent.x - radii.y, radii.y} +
             radii.y* vec2{std::cos(AS(f32, pi * 3) / 2 + segment * step),
                           std::sin(AS(f32, pi * 3) / 2 + segment * step)};

    vec2 st = texture_area.offset + p / extent * texture_area.extent;

    polygon[i] = vertex{.position = position + ash::transform(transform, p),
                        .st = st,
                        .color = color};
  }
}

}  // namespace polygons

constexpr void normalize_for_viewport(stx::Span<vertex> vertices,
                                      vec2 viewport_extent) {
  for (vertex& v : vertices) {
    // transform to -1 to +1 range from x pointing right and y pointing upwards
    v.position = (2 * v.position / viewport_extent) - 1;
  }
}

inline void triangulate_convex_polygon(stx::Vec<u32>& indices,
                                       u32 first_vertex_index, u32 nvertices) {
  ASH_CHECK(nvertices >= 3, "polygon must have 3 or more points");

  for (u32 i = 2; i < nvertices; i++) {
    indices.push_inplace(first_vertex_index).unwrap();
    indices.push_inplace(first_vertex_index + (i - 1)).unwrap();
    indices.push_inplace(first_vertex_index + i).unwrap();
  }
}

inline void triangulate_line(vec2 position, stx::Span<vertex const> in_vertices,
                             vec2 extent, mat4 const& transform,
                             f32 line_thickness, ash::rect texture_area,
                             u32 first_vertex_index,
                             stx::Vec<vertex>& out_vertices,
                             stx::Vec<u32>& out_indices) {
  if (in_vertices.size() < 2) return;

  bool has_previous_line = false;

  u32 vertex_index = first_vertex_index;

  for (usize i = 1; i < in_vertices.size(); i++) {
    vec4 color = in_vertices[i].color;
    vec2 p0 = in_vertices[i - 1].position;
    vec2 p1 = in_vertices[i].position;

    // the angles are specified in clockwise direction to be compatible with the
    // vulkan coordinate system
    //
    // get the angle of inclination of p2 to p1
    vec2 d = p1 - p0;
    f32 grad = std::abs(d.y / std::max(stx::F32_EPSILON, d.x));
    f32 alpha = std::atan(grad);

    // use direction of the points to get the actual overall angle of
    // inclination of p2 to p1
    if (d.x < 0 && d.y > 0) {
      alpha = AS(f32, pi - alpha);
    } else if (d.x < 0 && d.y < 0) {
      alpha = AS(f32, pi + alpha);
    } else if (d.x > 0 && d.y < 0) {
      alpha = AS(f32, 2 * pi - alpha);
    } else {
      // d.x >=0 && d.y >= 0
    }

    // line will be at a parallel angle
    alpha = AS(f32, alpha + pi / 2);

    vec2 f = line_thickness / 2 * vec2{std::cos(alpha), std::sin(alpha)};
    vec2 g = line_thickness / 2 *
             vec2{std::cos(AS(f32, pi + alpha)), std::sin(AS(f32, pi + alpha))};

    vec2 m0 = p0 + f;
    vec2 m1 = p0 + g;

    vec2 n0 = p1 + f;
    vec2 n1 = p1 + g;

    vec2 st0 = texture_area.offset + m0 / extent * texture_area.extent;
    vec2 st1 = texture_area.offset + m1 / extent * texture_area.extent;
    vec2 st2 = texture_area.offset + n0 / extent * texture_area.extent;
    vec2 st3 = texture_area.offset + n1 / extent * texture_area.extent;

    vertex vertices[] = {{.position = position + ash::transform(transform, m0),
                          .st = st0,
                          .color = color},
                         {.position = position + ash::transform(transform, m1),
                          .st = st1,
                          .color = color},
                         {.position = position + ash::transform(transform, n0),
                          .st = st2,
                          .color = color},
                         {.position = position + ash::transform(transform, n1),
                          .st = st3,
                          .color = color}};

    u32 indices[] = {vertex_index, vertex_index + 1, vertex_index + 3,
                     vertex_index, vertex_index + 2, vertex_index + 3};

    out_vertices.extend(vertices).unwrap();
    out_indices.extend(indices).unwrap();

    if (has_previous_line) {
      u32 prev_line_vertex_index = vertex_index - 4;

      u32 indices[] = {prev_line_vertex_index + 2,
                       prev_line_vertex_index + 3,
                       vertex_index,
                       prev_line_vertex_index + 2,
                       prev_line_vertex_index + 3,
                       vertex_index + 1};

      out_indices.extend(indices).unwrap();
    }

    has_previous_line = true;

    vertex_index += 4;
  }
}

struct CanvasState {
  mat4 transform = mat4::identity();
  mat4 global_transform = mat4::identity();
  rect clip_rect;
  Brush brush;
};

/// coordinates are specified in top-left origin space with x pointing to the
/// right and y pointing downwards (Vulkan Coordinate System).
///
///
/// NOTE: the canvas doesn't manage the lifetime of the handed over resources or
/// images
///
// In order to implement command culling we might need to sync with or remove
// global transform
//
struct Canvas {
  vec2 viewport_extent;
  Brush brush;
  mat4 transform = mat4::identity();
  mat4 global_transform = mat4::identity();
  rect clip_rect;

  stx::Vec<CanvasState> state_stack{stx::os_allocator};

  DrawList draw_list;

  void restart(vec2 new_viewport_extent) {
    viewport_extent = new_viewport_extent;
    brush = Brush{};
    transform = mat4::identity();
    global_transform = mat4::identity();
    clip_rect = rect{.offset = {0, 0}, .extent = viewport_extent};
    state_stack.clear();
    draw_list.clear();
  }

  /// push state (transform and clips) on state stack
  Canvas& save() {
    state_stack
        .push(CanvasState{.transform = transform,
                          .global_transform = global_transform,
                          .clip_rect = clip_rect,
                          .brush = brush})
        .unwrap();
    return *this;
  }

  /// save current transform and clip state
  /// pop state (transform and clips) stack and restore state
  Canvas& restore() {
    CanvasState state = state_stack.pop().unwrap_or(CanvasState{});
    transform = state.transform;
    global_transform = state.global_transform;
    clip_rect = state.clip_rect;
    brush = state.brush;
    return *this;
  }

  /// reset the rendering context to its default state (transform
  /// and clips)
  Canvas& reset() {
    transform = mat4::identity();
    global_transform = mat4::identity();
    clip_rect = rect{.offset = {0, 0}, .extent = viewport_extent};
    state_stack.clear();
    return *this;
  }

  Canvas& translate(f32 x, f32 y) {
    vec3 translation =
        2 * vec3{x, y, 1} / vec3{viewport_extent.x, viewport_extent.y, 1};
    transform = ash::translate(translation) * transform;
    return *this;
  }

  Canvas& global_translate(f32 x, f32 y) {
    vec3 translation =
        2 * vec3{x, y, 1} / vec3{viewport_extent.x, viewport_extent.y, 1};
    global_transform = ash::translate(translation) * global_transform;
    return *this;
  }

  Canvas& rotate(f32 x, f32 y, f32 z) {
    transform = ash::rotate_z(RADIANS(z)) * ash::rotate_y(RADIANS(y)) *
                ash::rotate_x(RADIANS(x)) * transform;
    return *this;
  }

  Canvas& global_rotate(f32 x, f32 y, f32 z) {
    global_transform = ash::rotate_z(RADIANS(z)) * ash::rotate_y(RADIANS(y)) *
                       ash::rotate_x(RADIANS(x)) * global_transform;
    return *this;
  }

  Canvas& scale(f32 x, f32 y) {
    transform = ash::scale(vec3{x, y, 1}) * transform;
    return *this;
  }

  Canvas& global_scale(f32 x, f32 y) {
    global_transform = ash::scale(vec3{x, y, 1}) * global_transform;
    return *this;
  }

  Canvas& clear() {
    draw_list.clear();

    vec4 color = brush.color.as_vec();

    vertex vertices[] = {
        {.position = {0, 0}, .st = {0, 0}, .color = color},
        {.position = {viewport_extent.x, 0}, .st = {1, 0}, .color = color},
        {.position = viewport_extent, .st = {1, 1}, .color = color},
        {.position = {0, viewport_extent.y}, .st = {0, 1}, .color = color}};

    normalize_for_viewport(vertices, viewport_extent);

    draw_list.vertices.extend(vertices).unwrap();

    u32 indices[] = {0, 1, 2, 0, 2, 3};

    draw_list.indices.extend(indices).unwrap();

    draw_list.cmds
        .push(DrawCommand{
            .indices_offset = 0,
            .nindices = AS(u32, std::size(indices)),
            .clip_rect = rect{.offset = {0, 0}, .extent = viewport_extent},
            .transform = mat4::identity(),
            .texture = brush.texture})
        .unwrap();

    return *this;
  }

  Canvas& draw_lines(stx::Span<vertex const> points, rect area,
                     rect texture_area, image background_image) {
    if (points.size() < 2 || !area.is_visible() || !clip_rect.overlaps(area)) {
      return *this;
    }

    u32 indices_offset = AS(u32, draw_list.indices.size());

    u32 vertices_offset = AS(u32, draw_list.vertices.size());

    // the input texture coordinates are not used since we need to regenerate
    // them for the line thickness
    triangulate_line(area.offset, points, area.extent, transform,
                     brush.line_thickness, texture_area, vertices_offset,
                     draw_list.vertices, draw_list.indices);

    u32 nindices = AS(u32, draw_list.indices.size() - indices_offset);

    normalize_for_viewport(draw_list.vertices.span().slice(vertices_offset),
                           viewport_extent);

    draw_list.cmds
        .push(DrawCommand{.indices_offset = indices_offset,
                          .nindices = nindices,
                          .clip_rect = clip_rect,
                          .transform = global_transform,
                          .texture = background_image})
        .unwrap();

    return *this;
  }

  Canvas& draw_convex_polygon_filled(stx::Span<vertex const> polygon, rect area,
                                     image background_image) {
    if (polygon.size() < 3 || !area.is_visible() || !clip_rect.overlaps(area)) {
      return *this;
    }

    u32 indices_offset = AS(u32, draw_list.indices.size());

    u32 vertices_offset = AS(u32, draw_list.vertices.size());

    triangulate_convex_polygon(draw_list.indices, vertices_offset,
                               AS(u32, polygon.size()));

    draw_list.vertices.extend(polygon).unwrap();

    normalize_for_viewport(draw_list.vertices.span().slice(vertices_offset),
                           viewport_extent);

    u32 nindices = AS(u32, draw_list.indices.size() - indices_offset);

    draw_list.cmds
        .push(DrawCommand{.indices_offset = indices_offset,
                          .nindices = nindices,
                          .clip_rect = clip_rect,
                          .transform = global_transform,
                          .texture = background_image})
        .unwrap();

    return *this;
  }

  Canvas& draw_line(vec2 p1, vec2 p2) {
    vec4 color = brush.color.as_vec();
    vertex points[] = {
        vertex{.position = p1, .st = {}, .color = color},
        vertex{.position = p1 + ash::transform(transform, p2 - p1),
               .st = {},
               .color = color}};

    rect area{.offset = p1, .extent = p2 - p1};

    return draw_lines(points, area, rect{.offset = {0, 0}, .extent = {1, 1}},
                      brush.texture);
  }

  Canvas& draw_rect(rect area) {
    vertex vertices[4];

    rect texture_area{.offset = {0, 0}, .extent = {1, 1}};

    polygons::rect(area.offset, area.extent, transform, brush.color.as_vec(),
                   texture_area, vertices);

    if (brush.fill) {
      return draw_convex_polygon_filled(vertices, area, brush.texture);
    } else {
      area.offset = area.offset - brush.line_thickness / 2;
      area.extent = area.extent + brush.line_thickness;
      vertex opoints[] = {vertices[0], vertices[1], vertices[2],
                          vertices[3], vertices[0], vertices[1]};
      return draw_lines(opoints, area, texture_area, brush.texture);
    }
  }

  Canvas& draw_circle(vec2 position, f32 radius, usize nsegments) {
    stx::Vec<vertex> vertices{stx::os_allocator};
    vertices.resize(nsegments).unwrap();

    rect texture_area{.offset = {0, 0}, .extent = {1, 1}};
    polygons::circle(position, radius, nsegments, transform,
                     brush.color.as_vec(), texture_area, vertices);

    rect area{.offset = position, .extent = vec2::splat(2 * radius)};

    if (brush.fill) {
      return draw_convex_polygon_filled(vertices, area, brush.texture);
    } else {
      if (vertices.size() > 0) {
        vertices.push_inplace(vertices[0]).unwrap();
      }
      if (vertices.size() > 1) {
        vertices.push_inplace(vertices[1]).unwrap();
      }
      area.offset = area.offset -
                    vec2{brush.line_thickness / 2, brush.line_thickness / 2};
      area.extent =
          area.extent + vec2{brush.line_thickness, brush.line_thickness};
      return draw_lines(vertices, area, texture_area, brush.texture);
    }
  }

  Canvas& draw_ellipse(vec2 position, vec2 radii, usize nsegments) {
    stx::Vec<vertex> vertices{stx::os_allocator};
    vertices.resize(nsegments).unwrap();

    rect texture_area{.offset = {0, 0}, .extent = {1, 1}};

    polygons::ellipse(position, radii, nsegments, transform,
                      brush.color.as_vec(), texture_area, vertices);

    rect area{.offset = position, .extent = 2 * radii};

    if (brush.fill) {
      return draw_convex_polygon_filled(vertices, area, brush.texture);
    } else {
      if (vertices.size() > 0) {
        vertices.push_inplace(vertices[0]).unwrap();
      }
      if (vertices.size() > 1) {
        vertices.push_inplace(vertices[1]).unwrap();
      }
      area.offset = area.offset -
                    vec2{brush.line_thickness / 2, brush.line_thickness / 2};
      area.extent =
          area.extent + vec2{brush.line_thickness, brush.line_thickness};
      return draw_lines(vertices, area, texture_area, brush.texture);
    }
  }

  Canvas& draw_round_rect(rect area, vec4 radii, usize nsegments) {
    stx::Vec<vertex> vertices{stx::os_allocator};
    vertices.resize(nsegments * 4).unwrap();

    rect texture_area{.offset = {0, 0}, .extent = {1, 1}};

    polygons::round_rect(area.offset, area.extent, radii, nsegments, transform,
                         brush.color.as_vec(), texture_area, vertices);

    if (brush.fill) {
      return draw_convex_polygon_filled(vertices, area, brush.texture);
    } else {
      if (vertices.size() > 0) {
        vertices.push_inplace(vertices[0]).unwrap();
      }
      if (vertices.size() > 1) {
        vertices.push_inplace(vertices[1]).unwrap();
      }
      area.offset = area.offset -
                    vec2{brush.line_thickness / 2, brush.line_thickness / 2};
      area.extent =
          area.extent + vec2{brush.line_thickness, brush.line_thickness};
      return draw_lines(vertices, area, texture_area, brush.texture);
    }
  }

  Canvas& draw_image(image img, rect area, f32 s0, f32 t0, f32 s1, f32 t1,
                     color color = colors::WHITE) {
    vertex vertices[4];
    rect texture_area{.offset = {s0, t0}, .extent = {s1 - s0, t1 - t0}};

    polygons::rect(area.offset, area.extent, transform, color.as_vec(),
                   texture_area, vertices);

    return draw_convex_polygon_filled(vertices, area, img);
  }

  Canvas& draw_image(image img, rect area, color color = colors::WHITE) {
    return draw_image(img, area, 0, 0, 1, 1, color);
  }

  Canvas& draw_rounded_image(image img, rect area, rect image_portion,
                             vec4 border_radii, usize nsegments) {
    stx::Vec<vertex> vertices{stx::os_allocator};
    vertices.resize(nsegments * 4).unwrap();

    polygons::round_rect(area.offset, area.extent, border_radii, nsegments,
                         transform, colors::WHITE.as_vec(), image_portion,
                         vertices);

    return draw_convex_polygon_filled(vertices, area, img);
  }

  Canvas& draw_rounded_image(image img, rect area, vec4 border_radii,
                             usize nsegments) {
    rect texture_area{.offset = {0, 0}, .extent = {1, 1}};
    return draw_rounded_image(img, area, texture_area, border_radii, nsegments);
  }

  Canvas& draw_glyph(Glyph const& glyph, TextRun const& run, image atlas,
                     vec2 baseline, f32 font_scale, f32 line_height,
                     f32 vert_spacing) {
    f32 ascent = font_scale * glyph.ascent;
    vec2 advance = font_scale * glyph.advance;
    vec2 extent{font_scale * glyph.extent.width,
                font_scale * glyph.extent.height};

    if (run.style.background_color.is_visible()) {
      save();
      brush.color = run.style.background_color;
      brush.fill = true;
      draw_rect(rect{
          .offset = baseline - vec2{0, line_height},
          .extent = vec2{advance.x + run.style.letter_spacing, line_height}});
      restore();
    }

    if (run.style.foreground_color.is_visible()) {
      save();
      brush.color = run.style.foreground_color;
      draw_image(atlas,
                 rect{.offset = baseline - vec2{0, vert_spacing + ascent},
                      .extent = extent},
                 glyph.s0, glyph.t0, glyph.s1, glyph.t1, brush.color);
      restore();
    }

    return *this;
  }

  // TODO(lamarrr): we need separate layout pass so we can perform widget
  // layout, use callbacks to perform certain actions on layout calculation.
  //
  // TODO(lamarrr): [future] add bidi
  Canvas& draw_text(Paragraph paragraph, stx::Span<CachedFont const> fonts,
                    vec2 position, f32 max_line_width,
                    stx::Vec<RunSubWord>& subwords,
                    stx::Vec<SubwordGlyph>& glyphs/*, TextLayout& layout,
                    bool skip_drawing*/) {
    constexpr u32 SPACE = ' ';
    constexpr u32 TAB = '\t';
    constexpr u32 NEWLINE = '\n';
    constexpr u32 RETURN = '\r';

    subwords.clear();
    glyphs.clear();

    // TODO(lamarrr): arabic text is rendering but weird

    for (usize i = 0; i < paragraph.runs.size(); i++) {
      TextRun const& run = paragraph.runs[i];

      for (char const* word_begin = run.text.begin();
           word_begin < run.text.end();) {
        usize nspaces = 0;
        usize nline_breaks = 0;
        char const* seeker = word_begin;
        char const* word_end = seeker;
        u32 codepoint = 0;

        for (; seeker < run.text.end();) {
          codepoint = stx::utf8_next(seeker);

          if (codepoint == RETURN || codepoint == NEWLINE || codepoint == TAB ||
              codepoint == SPACE) {
            break;
          }
        }

        word_end = seeker;

        if (codepoint == RETURN) {
          word_end = seeker - 1;

          if (seeker + 1 < run.text.end()) {
            if (*(seeker + 1) == NEWLINE) {
              seeker++;
              nline_breaks++;
            }
          }
        } else if (codepoint == SPACE) {
          word_end = seeker - 1;
          nspaces++;

          for (char const* iter = seeker; iter < run.text.end();) {
            seeker = iter;
            u32 codepoint = stx::utf8_next(iter);

            if (codepoint == SPACE) {
              nspaces++;
            } else {
              break;
            }
          }
        } else if (codepoint == TAB) {
          word_end = seeker - 1;
          nspaces += run.style.tab_size;

          for (char const* iter = seeker; iter < run.text.end();) {
            seeker = iter;
            u32 codepoint = stx::utf8_next(iter);
            if (codepoint == TAB) {
              nspaces += run.style.tab_size;
            } else {
              break;
            }
          }
        } else if (codepoint == NEWLINE) {
          word_end = seeker - 1;
          nline_breaks++;

          for (char const* iter = seeker; iter < run.text.end();) {
            seeker = iter;
            u32 codepoint = stx::utf8_next(iter);

            if (codepoint == NEWLINE) {
              nline_breaks++;
            } else {
              break;
            }
          }
        }

        subwords
            .push(
                RunSubWord{.text = run.text.slice(word_begin - run.text.begin(),
                                                  word_end - word_begin),
                           .run = i,
                           .nspaces = nspaces,
                           .nline_breaks = nline_breaks})
            .unwrap();

        word_begin = seeker;
      }
    }

    for (RunSubWord& subword : subwords) {
      TextRun const& run = paragraph.runs[subword.run];
      Font const& font = *fonts[run.font].font.handle;
      FontAtlas const& atlas = fonts[run.font].atlas;

      hb_feature_t const shaping_features[] = {
          {Font::KERNING_FEATURE, run.style.use_kerning, 0, stx::U_MAX},
          {Font::LIGATURE_FEATURE, run.style.use_ligatures, 0, stx::U_MAX},
          {Font::CONTEXTUAL_LIGATURE_FEATURE, run.style.use_ligatures, 0,
           stx::U_MAX}};

      fmt::print("language: {}\n", run.language);

      hb_font_set_scale(font.hbfont, 64 * atlas.font_height,
                        64 * atlas.font_height);

      hb_buffer_reset(font.hbscratch_buffer);
      hb_buffer_set_script(font.hbscratch_buffer, AS(hb_script_t, run.script));

      if (run.direction == TextDirection::LeftToRight) {
        hb_buffer_set_direction(font.hbscratch_buffer, HB_DIRECTION_LTR);
      } else {
        hb_buffer_set_direction(font.hbscratch_buffer, HB_DIRECTION_RTL);
      }
      hb_buffer_set_language(
          font.hbscratch_buffer,
          hb_language_from_string(run.language.data(),
                                  AS(int, run.language.size())));
      hb_buffer_add_utf8(font.hbscratch_buffer, subword.text.begin(),
                         AS(int, subword.text.size()), 0,
                         AS(int, subword.text.size()));

      hb_shape(font.hbfont, font.hbscratch_buffer, shaping_features,
               AS(uint, std::size(shaping_features)));

      uint nglyphs;
      hb_glyph_info_t* glyph_info =
          hb_buffer_get_glyph_infos(font.hbscratch_buffer, &nglyphs);

      f32 font_scale = run.style.font_height / atlas.font_height;

      f32 width = 0;

      subword.glyph_start = glyphs.size();
      subword.nglyphs = nglyphs;

      for (usize i = 0; i < nglyphs; i++) {
        u32 glyph_index = glyph_info[i].codepoint;
        stx::Span glyph = atlas.get(glyph_index);

        // TODO(lamarrr): we seem to be getting an extra glyph index 0
        fmt::print("glyph index: {}\n", glyph_index);

        if (!glyph.is_empty()) {
          width += glyph[0].advance.x * font_scale + run.style.letter_spacing;
          glyphs.push(SubwordGlyph{.font = run.font, .glyph = glyph_index})
              .unwrap();
        } else {
          width +=
              atlas.glyphs[0].advance.x * font_scale + run.style.letter_spacing;
          glyphs.push(SubwordGlyph{.font = run.font, .glyph = 0}).unwrap();
        }
      }

      subword.width = width;
    }

    {
      f32 cursor_x = 0;

      for (RunSubWord* iter = subwords.begin(); iter < subwords.end();) {
        f32 word_width = 0;
        RunSubWord* subword = iter;

        for (; subword < subwords.end();) {
          f32 spaced_word_width =
              subword->width +
              subword->nspaces *
                  paragraph.runs[subword->run].style.word_spacing;

          // if end of word
          if (subword->nspaces > 0 || subword->nline_breaks > 0 ||
              subword == subwords.end() - 1) {
            // check if wrapping needed
            if (cursor_x + spaced_word_width > max_line_width) {
              iter->is_wrapped = true;
              cursor_x = spaced_word_width;
              if (subword->nline_breaks > 0) {
                cursor_x = 0;
              }
            } else {
              if (subword->nline_breaks > 0) {
                cursor_x = 0;
              } else {
                cursor_x += spaced_word_width;
              }
            }
            subword++;
            break;
          } else {
            // continue until we reach end of word
            cursor_x += spaced_word_width;
            subword++;
          }
        }

        iter = subword;
      }
    }

    {
      f32 baseline = 0;
      usize nprev_line_breaks = 0;

      for (RunSubWord const* iter = subwords.begin(); iter < subwords.end();) {
        RunSubWord const* line_begin = iter;
        RunSubWord const* line_end = iter + 1;
        usize nline_breaks = 0;

        if (line_begin->is_wrapped && nprev_line_breaks == 0) {
          nprev_line_breaks = 1;
        }

        if (line_begin->nline_breaks == 0) {
          for (; line_end < subwords.end();) {
            if (line_end->nline_breaks > 0) {
              nline_breaks = line_end->nline_breaks;
              line_end++;
              break;
            } else if (line_end->is_wrapped) {
              break;
            } else {
              line_end++;
            }
          }
        }

        f32 line_width = 0;
        f32 line_height = 0;
        f32 max_ascent = 0;

        for (RunSubWord const* subword = line_begin; subword < line_end;
             subword++) {
          line_width += subword->width +
                        subword->nspaces *
                            paragraph.runs[subword->run].style.word_spacing;
          line_height = std::max(
              line_height, paragraph.runs[subword->run].style.line_height *
                               paragraph.runs[subword->run].style.font_height);

          TextRun const& run = paragraph.runs[subword->run];
          Font const& font = *fonts[run.font].font.handle;
          FontAtlas const& atlas = fonts[run.font].atlas;
          f32 font_scale = run.style.font_height / atlas.font_height;

          for (SubwordGlyph const& glyph :
               glyphs.span().slice(subword->glyph_start, subword->nglyphs)) {
            max_ascent = std::max(
                max_ascent, atlas.glyphs[glyph.glyph].ascent * font_scale);
          }
        }

        // TODO(lamarrr): add alignment
        f32 vert_spacing = std::max(line_height - max_ascent, 0.0f) / 2;
        f32 alignment = 0;

        if (paragraph.align == TextAlign::Center) {
          alignment = (std::max(line_width, max_line_width) - line_width) / 2;
        } else if (paragraph.align == TextAlign::Right) {
          alignment = std::max(line_width, max_line_width) - line_width;
        }

        baseline += nprev_line_breaks * line_height;

        f32 cursor_x = 0;

        for (RunSubWord const* subword = line_begin; subword < line_end;) {
          if (paragraph.runs[subword->run].direction ==
              TextDirection::LeftToRight) {
            TextRun const& run = paragraph.runs[subword->run];
            FontAtlas const& atlas = fonts[run.font].atlas;

            f32 font_scale = run.style.font_height / atlas.font_height;
            f32 letter_spacing = run.style.letter_spacing;
            f32 word_spacing = run.style.word_spacing;
            f32 init_cursor_x = cursor_x;

            for (SubwordGlyph const& glyph :
                 glyphs.span().slice(subword->glyph_start, subword->nglyphs)) {
              Glyph const& g = atlas.glyphs[glyph.glyph];
              vec2 advance = g.advance * font_scale;
              draw_glyph(g, run, atlas.texture,
                         position + vec2{cursor_x, baseline}, font_scale,
                         line_height, vert_spacing);
              cursor_x += advance.x + letter_spacing;
            }

            if (run.style.background_color.is_visible() &&
                subword->nspaces > 0) {
              save();
              brush.color = run.style.background_color;
              brush.fill = true;
              draw_rect(rect{
                  .offset = position + vec2{cursor_x, baseline - line_height},
                  .extent =
                      vec2{word_spacing * subword->nspaces, line_height}});
              restore();
            }

            if (run.style.underline_color.is_visible()) {
              save();
              brush.color = run.style.underline_color;
              brush.fill = true;
              draw_rect(rect{.offset = position + vec2{init_cursor_x, baseline},
                             .extent = vec2{subword->width +
                                                subword->nspaces * word_spacing,
                                            run.style.underline_thickness}});
              restore();
            }

            // TODO(lamarrr): implement strikethrough

            cursor_x += subword->nspaces * word_spacing;
            subword++;
          } else {
            f32 rtl_width = 0;
            RunSubWord const* rtl_begin = subword;
            RunSubWord const* rtl_end = subword + 1;

            rtl_width += rtl_begin->width +
                         rtl_begin->nspaces *
                             paragraph.runs[rtl_begin->run].style.word_spacing;

            for (; rtl_end < line_end; rtl_end++) {
              if (paragraph.runs[rtl_end->run].direction ==
                  TextDirection::LeftToRight) {
                break;
              } else {
                rtl_width +=
                    rtl_end->width +
                    rtl_end->nspaces *
                        paragraph.runs[rtl_end->run].style.word_spacing;
              }
            }

            f32 rtl_cursor_x = cursor_x + rtl_width;

            for (RunSubWord const* rtl_iter = rtl_begin; rtl_iter < rtl_end;
                 rtl_iter++) {
              TextRun const& run = paragraph.runs[rtl_iter->run];
              FontAtlas const& atlas = fonts[run.font].atlas;

              f32 font_scale = run.style.font_height / atlas.font_height;
              f32 letter_spacing = run.style.letter_spacing;
              f32 spacing = rtl_iter->nspaces * run.style.word_spacing;
              rtl_cursor_x -= spacing;

              if (run.style.background_color.is_visible() &&
                  rtl_iter->nspaces > 0) {
                save();
                brush.color = run.style.background_color;
                brush.fill = true;
                draw_rect(rect{
                    .offset =
                        position + vec2{rtl_cursor_x, baseline - line_height},
                    .extent = vec2{spacing, line_height}});
                restore();
              }

              if (run.style.background_color.is_visible() &&
                  rtl_iter->nspaces > 0) {
                save();
                brush.color = run.style.background_color;
                brush.fill = true;
                draw_rect(rect{
                    .offset =
                        position + vec2{rtl_cursor_x, baseline - line_height},
                    .extent = vec2{spacing, line_height}});
                restore();
              }

              rtl_cursor_x -= rtl_iter->width;

              f32 glyph_cursor_x = rtl_cursor_x;

              for (SubwordGlyph const& glyph : glyphs.span().slice(
                       rtl_iter->glyph_start, rtl_iter->nglyphs)) {
                Glyph const& g = atlas.glyphs[glyph.glyph];
                vec2 advance = g.advance * font_scale;
                draw_glyph(g, run, atlas.texture,
                           position + vec2{glyph_cursor_x, baseline},
                           font_scale, line_height, vert_spacing);
                glyph_cursor_x += advance.x + letter_spacing;
              }

              if (run.style.underline_color.is_visible()) {
                save();
                brush.color = run.style.underline_color;
                brush.fill = true;
                draw_rect(
                    rect{.offset = position + vec2{rtl_cursor_x, baseline},
                         .extent = vec2{rtl_iter->width + spacing,
                                        run.style.underline_thickness}});
                restore();
              }
            }

            cursor_x += rtl_width;
            subword = rtl_end;
          }
        }

        nprev_line_breaks = nline_breaks;
        iter = line_end;
      }
    }

    return *this;
  }
};

struct CanvasPushConstants {
  mat4 transform = mat4::identity();
};

}  // namespace gfx
}  // namespace ash
