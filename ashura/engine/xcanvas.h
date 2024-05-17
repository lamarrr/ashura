#pragma once

#include "ashura/constraint.h"
#include "ashura/engine/font.h"
#include "ashura/engine/text.h"
#include "ashura/gfx/gfx.h"
#include "ashura/gfx/image.h"
#include "ashura/std/math.h"
#include "ashura/std/types.h"

namespace ash
{

struct Vertex2d
{
  Vec2 position;
  Vec2 uv;
  Vec4 tint;
};

/// TODO(lamarrr): use convex path abstraction
namespace paths
{

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
inline void add_line_stroke(Vec2 p0, Vec2 p1, f32 thickness, Vec4 color,
                            stx::Vec<Vertex2d> &out)
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

// line joint is a bevel joint, it is the most efficient since it re-uses
// existing vertices and doesn't require generating new vertices
inline void triangulate_line(stx::Span<Vertex2d const> in_points, f32 thickness,
                             stx::Vec<Vertex2d> &out_vertices,
                             stx::Vec<u32> &out_indices, bool should_close)
{
  if (in_points.size() < 2 || thickness == 0)
  {
    return;
  }

  bool has_previous_line = false;

  u32 Vertex_index = 0;

  for (u32 i = 1; i < static_cast<u32>(in_points.size()); i++)
  {
    Vec4 color = in_points[i - 1].color;
    Vec2 p0    = in_points[i - 1].position;
    Vec2 p1    = in_points[i].position;

    add_line_stroke(p0, p1, thickness, color, out_vertices);

    // weave the line triangles
    u32 indices[] = {Vertex_index, Vertex_index + 1, Vertex_index + 3,
                     Vertex_index, Vertex_index + 2, Vertex_index + 3};

    out_indices.extend(indices).unwrap();

    // weave the previous line's end to the beginning of this line
    if (has_previous_line)
    {
      u32 prev_line_Vertex_index = Vertex_index - 4;

      u32 indices[] = {prev_line_Vertex_index + 2,
                       prev_line_Vertex_index + 3,
                       Vertex_index,
                       prev_line_Vertex_index + 2,
                       prev_line_Vertex_index + 3,
                       Vertex_index + 1};

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
    u32 indices[] = {Vertex_index, Vertex_index + 1, Vertex_index + 3,
                     Vertex_index, Vertex_index + 2, Vertex_index + 3};

    out_indices.extend(indices).unwrap();

    {
      u32 prev_line_Vertex_index  = Vertex_index - 4;
      u32 first_line_Vertex_index = 0;

      u32 indices[] = {
          // weave the previous line's end to the beginning of this line
          prev_line_Vertex_index + 2, prev_line_Vertex_index + 3, Vertex_index,
          prev_line_Vertex_index + 2, prev_line_Vertex_index + 3,
          Vertex_index + 1,
          // weave this line's end to the beginning of the first line
          Vertex_index + 2, Vertex_index + 3, first_line_Vertex_index,
          Vertex_index + 2, Vertex_index + 3, first_line_Vertex_index + 1};

      out_indices.extend(indices).unwrap();
    }
  }
}

/// Coordinates are specified in top-left origin absolute pixel coordinates with
/// x pointing to the right and y pointing downwards (i.e. {0, 0} being top left
/// and {x, y} being bottom right), the transform matrix transforms the vertices
/// to a Vulkan Coordinate System (i.e. {-1, -1} top left and {1, 1} bottom
/// right).
///
/// LIMITATIONS:
/// - each draw call must not have more than 2^32 vertices and indices,
/// otherwise split them up
/// - the canvas must not have more than 2^32 draw calls
///
/// NOTES:
/// - some graphics frameworks, i.e. vulkan only allow u32 indices so we have to
/// split up the draw calls across multiple draw list batches
/// - the canvas doesn't manage the lifetime of the handed over resources or
/// images
///
struct Canvas
{
  Vec2     viewport_extent;
  DrawList draw_list;

  Mat3 make_transform(Vec2 position) const
  {
    Vec2 viewport_extent_clamped = Vec2{epsilon_clamp(viewport_extent.x),
                                        epsilon_clamp(viewport_extent.y)};

    Mat3 t = state.local_transform;        /// apply local coordinate transform
    t      = translate2d(position) * t;        /// apply positioning
    t = state.global_transform * t;        /// apply global coordinate transform
    t = scale2d(2 / viewport_extent_clamped) *
        t;        /// normalize to 0 to 2 coordinate range
    t = translate2d({-1, -1}) * t;        /// normalize from [0, 2] to vulkan
                                          /// viewport coordinate range [-1, -1]
    return t;
  }

  // TODO(lamarrr): draw quad

  Canvas &draw_path(stx::Span<Vertex2d const> points, Vec2 position,
                    Vec2 uv_stretch, f32 thickness, bool should_close,
                    image texture = WHITE_IMAGE, Vec2 uv0 = Vec2{0, 0},
                    Vec2 uv1 = Vec2{1, 1})
  {
    if (points.size() < 2 || thickness == 0)
    {
      return *this;
    }

    usize prev_nvertices = draw_list.vertices.size();
    usize prev_nindices  = draw_list.indices.size();

    triangulate_line(points, thickness, draw_list.vertices, draw_list.indices,
                     should_close);
    paths::lerp_uvs(draw_list.vertices.span().slice(prev_nvertices), uv_stretch,
                    uv0, uv1);

    usize curr_nvertices = draw_list.vertices.size();
    usize curr_nindices  = draw_list.indices.size();

    u32 nvertices = static_cast<u32>(curr_nvertices - prev_nvertices);
    u32 nindices  = static_cast<u32>(curr_nindices - prev_nindices);

    draw_list.commands
        .push(DrawCommand{.pipeline       = DEFAULT_SHAPE_PIPELINE,
                          .nvertices      = nvertices,
                          .nindices       = nindices,
                          .first_instance = 0,
                          .ninstances     = 1,
                          .scissor_offset = state.scissor_offset,
                          .scissor_extent = state.scissor_extent,
                          .textures       = {texture}}
                  .with_push_constant(transpose(make_transform(position))))
        .unwrap();

    return *this;
  }

  stx::Span<Vertex2d> reserve_convex_polygon(u32 npoints, Vec2 position,
                                             image texture)
  {
    ASH_CHECK(npoints >= 3, "A polygon consists of at least 3 points");

    usize prev_nvertices = draw_list.vertices.size();
    usize prev_nindices  = draw_list.indices.size();

    triangulate_convex_polygon(draw_list.indices, npoints);

    stx::Span polygon =
        draw_list.vertices
            .unsafe_resize_uninitialized(draw_list.vertices.size() + npoints)
            .unwrap();

    usize curr_nvertices = draw_list.vertices.size();
    usize curr_nindices  = draw_list.indices.size();

    u32 nvertices = static_cast<u32>(curr_nvertices - prev_nvertices);
    u32 nindices  = static_cast<u32>(curr_nindices - prev_nindices);

    draw_list.commands
        .push(DrawCommand{.pipeline       = DEFAULT_SHAPE_PIPELINE,
                          .nvertices      = nvertices,
                          .nindices       = nindices,
                          .first_instance = 0,
                          .ninstances     = 1,
                          .scissor_offset = state.scissor_offset,
                          .scissor_extent = state.scissor_extent,
                          .textures       = {texture}}
                  .with_push_constant(transpose(make_transform(position))))
        .unwrap();

    return polygon;
  }

  // texture coordinates are assumed to already be filled and area of viewport
  // known
  Canvas &draw_convex_polygon_filled(stx::Span<Vertex2d const> polygon,
                                     Vec2 position, image texture)
  {
    if (polygon.size() < 3)
    {
      return *this;
    }

    reserve_convex_polygon(static_cast<u32>(polygon.size()), position, texture)
        .copy(polygon);

    return *this;
  }

  Canvas &draw_rect_filled(Vec2 offset, Vec2 extent, Vec4 color,
                           LinearColorGradient gradient = {},
                           image texture = WHITE_IMAGE, Vec2 uv0 = Vec2{0, 0},
                           Vec2 uv1 = Vec2{1, 1})
  {
    if (!viewport_contains(offset, extent))
    {
      return *this;
    }

    paths::lerp_color_gradient(
        paths::lerp_uvs(paths::rect(Vec2{0, 0}, extent, color,
                                    reserve_convex_polygon(4, offset, texture)),
                        extent, uv0, uv1),
        extent, gradient);

    return *this;
  }

  Canvas &draw_rect_stroke(Vec2 offset, Vec2 extent, Vec4 color, f32 thickness,
                           image texture = WHITE_IMAGE, Vec2 uv0 = Vec2{0, 0},
                           Vec2 uv1 = Vec2{1, 1})
  {
    if (!viewport_contains(offset, extent) || thickness == 0)
    {
      return *this;
    }

    Vertex2d line[4];

    paths::rect(Vec2{thickness / 2, thickness / 2}, extent - thickness, color,
                line);

    return draw_path(line, offset, extent, thickness, true, texture, uv0, uv1);
  }

  Canvas &draw_circle_filled(Vec2 center, f32 radius, u32 nsegments, Vec4 color,
                             LinearColorGradient gradient = {},
                             image texture = WHITE_IMAGE, Vec2 uv0 = Vec2{0, 0},
                             Vec2 uv1 = Vec2{1, 1})
  {
    Vec2 position = center - radius;
    Vec2 offset   = position;
    Vec2 extent{2 * radius, 2 * radius};

    if (!viewport_contains(offset, extent))
    {
      return *this;
    }

    paths::lerp_color_gradient(
        paths::lerp_uvs(
            paths::circle(Vec2{0, 0}, radius, nsegments, color,
                          reserve_convex_polygon(nsegments, position, texture)),
            extent, uv0, uv1),
        extent, gradient);

    return *this;
  }

  Canvas &draw_circle_stroke(Vec2 center, f32 radius, u32 nsegments, Vec4 color,
                             f32 thickness, image texture = WHITE_IMAGE,
                             Vec2 uv0 = Vec2{0, 0}, Vec2 uv1 = Vec2{1, 1})
  {
    Vec2 position = center - radius - thickness / 2;
    Vec2 offset   = position;
    f32  diameter = 2 * radius + thickness;
    Vec2 extent   = Vec2{diameter, diameter};

    if (!viewport_contains(offset, extent) || thickness == 0)
    {
      return *this;
    }

    scratch.unsafe_resize_uninitialized(nsegments).unwrap();

    paths::circle(Vec2{thickness / 2, thickness / 2}, radius, nsegments, color,
                  scratch);

    return draw_path(scratch, offset, extent, thickness, true, texture, uv0,
                     uv1);
  }

  Canvas &draw_arc_stroke(Vec2 center, f32 radius, f32 begin, f32 end,
                          u32 nsegments, Vec4 color, f32 thickness,
                          image texture = WHITE_IMAGE, Vec2 uv0 = {0, 0},
                          Vec2 uv1 = {1, 1})
  {
    Vec2 position = center - radius - thickness / 2;
    Vec2 offset   = position;
    f32  diameter = 2 * radius + thickness;
    Vec2 extent   = Vec2{diameter, diameter};

    if (!viewport_contains(offset, extent))
    {
      return *this;
    }

    paths::lerp_uvs(
        paths::arc(Vec2{thickness / 2, thickness / 2}, radius, begin, end,
                   nsegments, color,
                   reserve_convex_polygon(nsegments, position, texture)),
        extent, uv0, uv1);

    return *this;
  }

  Canvas &draw_ellipse_filled(Vec2 center, Vec2 radii, u32 nsegments,
                              Vec4 color, LinearColorGradient gradient = {},
                              image texture = WHITE_IMAGE,
                              Vec2 uv0 = Vec2{0, 0}, Vec2 uv1 = Vec2{1, 1})
  {
    Vec2 position = center - radii;
    Vec2 offset   = position;
    Vec2 extent   = 2 * radii;

    if (!viewport_contains(offset, extent))
    {
      return *this;
    }

    paths::lerp_color_gradient(
        paths::lerp_uvs(
            paths::ellipse(Vec2{0, 0}, radii, nsegments, color,
                           reserve_convex_polygon(nsegments, offset, texture)),
            extent, uv0, uv1),
        extent, gradient);

    return *this;
  }

  Canvas &draw_ellipse_stroke(Vec2 center, Vec2 radii, u32 nsegments,
                              Vec4 color, f32 thickness,
                              image texture = WHITE_IMAGE,
                              Vec2 uv0 = Vec2{0, 0}, Vec2 uv1 = Vec2{1, 1})
  {
    Vec2 position = center - radii;
    Vec2 offset   = position;
    Vec2 extent   = 2 * radii;

    if (!viewport_contains(offset, extent) || thickness == 0)
    {
      return *this;
    }

    scratch.unsafe_resize_uninitialized(nsegments).unwrap();

    paths::ellipse(Vec2{thickness / 2, thickness / 2}, radii - thickness,
                   nsegments, color, scratch);

    return draw_path(scratch, offset, extent, thickness, true, texture, uv0,
                     uv1);
  }

  Canvas &draw_round_rect_filled(Vec2 offset, Vec2 extent, Vec4 radii,
                                 u32 nsegments, Vec4 color,
                                 LinearColorGradient gradient = {},
                                 image               texture  = WHITE_IMAGE,
                                 Vec2 uv0 = Vec2{0, 0}, Vec2 uv1 = Vec2{1, 1})
  {
    if (!viewport_contains(offset, extent))
    {
      return *this;
    }

    paths::lerp_color_gradient(
        paths::lerp_uvs(
            paths::round_rect(
                Vec2{0, 0}, extent, radii, nsegments, color,
                reserve_convex_polygon(nsegments * 4 + 8, offset, texture)),
            extent, uv0, uv1),
        extent, gradient);

    return *this;
  }

  Canvas &draw_round_rect_stroke(Vec2 offset, Vec2 extent, Vec4 radii,
                                 u32 nsegments, Vec4 color, f32 thickness,
                                 image texture = WHITE_IMAGE, Vec2 uv0 = {0, 0},
                                 Vec2 uv1 = {1, 1})
  {
    if (!viewport_contains(offset, extent) || thickness == 0)
    {
      return *this;
    }

    scratch.unsafe_resize_uninitialized(nsegments * 4 + 8).unwrap();

    paths::round_rect(Vec2{thickness / 2, thickness / 2}, extent - thickness,
                      radii, nsegments, color, scratch);

    return draw_path(scratch, offset, extent, thickness, true, texture, uv0,
                     uv1);
  }

  Canvas &draw_bevel_rect_filled(Vec2 offset, Vec2 extent, Vec4 radii,
                                 Vec4 color, LinearColorGradient gradient = {},
                                 image texture = WHITE_IMAGE,
                                 Vec2 uv0 = Vec2{0, 0}, Vec2 uv1 = Vec2{1, 1})
  {
    if (!viewport_contains(offset, extent))
    {
      return *this;
    }

    paths::lerp_color_gradient(
        paths::lerp_uvs(
            paths::bevel_rect(Vec2{0, 0}, extent, radii, color,
                              reserve_convex_polygon(8, offset, texture)),
            extent, uv0, uv1),
        extent, gradient);

    return *this;
  }

  Canvas &draw_bevel_rect_stroke(Vec2 offset, Vec2 extent, Vec4 radii,
                                 Vec4 color, f32 thickness,
                                 image texture = WHITE_IMAGE,
                                 Vec2 uv0 = Vec2{0, 0}, Vec2 uv1 = Vec2{1, 1})
  {
    if (!viewport_contains(offset, extent) || thickness == 0)
    {
      return *this;
    }

    scratch.unsafe_resize_uninitialized(8).unwrap();

    paths::bevel_rect(Vec2{thickness / 2, thickness / 2}, extent - thickness,
                      radii, color, scratch);

    return draw_path(scratch, offset, extent, thickness, true, texture, uv0,
                     uv1);
  }

  Canvas &draw_image(image img, Vec2 offset, Vec2 extent, Vec4 tint,
                     Vec2 uv0 = Vec2{0, 0}, Vec2 uv1 = Vec2{1, 1})
  {
    if (!viewport_contains(offset, extent))
    {
      return *this;
    }

    paths::lerp_uvs(paths::rect(Vec2{0, 0}, extent, tint,
                                reserve_convex_polygon(4, offset, img)),
                    extent, uv0, uv1);

    return *this;
  }

  Canvas &draw_rounded_image(image img, Vec2 offset, Vec2 extent,
                             Vec4 border_radii, u32 nsegments, Vec4 tint,
                             Vec2 uv0 = Vec2{0, 0}, Vec2 uv1 = Vec2{1, 1})
  {
    if (!viewport_contains(offset, extent))
    {
      return *this;
    }

    paths::lerp_uvs(paths::round_rect(
                        Vec2{0, 0}, extent, border_radii, nsegments, tint,
                        reserve_convex_polygon(nsegments * 4 + 8, offset, img)),
                    extent, uv0, uv1);

    return *this;
  }

  Canvas &draw_rounded_image(image img, Vec2 offset, Vec2 extent,
                             Vec4 border_radii, u32 nsegments, Vec4 tint)
  {
    return draw_rounded_image(img, offset, extent, border_radii, nsegments,
                              tint, {0, 0}, {1, 1});
  }

  Canvas &draw_glyph(Vec2 block_position, Vec2 baseline, f32 text_scale_factor,
                     Glyph const &glyph, GlyphShaping const &shaping,
                     TextStyle const &style, image atlas)
  {
    save();
    state.local_transform = state.local_transform * translate2d(baseline);

    Vec2 offset = Vec2{glyph.metrics.bearing.x, -glyph.metrics.bearing.y} *
                      style.font_height * text_scale_factor +
                  shaping.offset;
    Vec2 extent = glyph.metrics.extent * style.font_height * text_scale_factor;
    Mat3 transform = state.global_transform * translate2d(block_position) *
                     state.local_transform;

    if (!overlaps({0, 0}, viewport_extent, ash::transform(transform, offset),
                  ash::transform(transform, offset + extent)))
    {
      restore();
      return *this;
    }

    Vertex2d const vertices[] = {
        {.position = offset, .uv = glyph.uv0, .color = style.foreground_color},
        {.position = {offset.x + extent.x, offset.y},
         .uv       = {glyph.uv1.x, glyph.uv0.y},
         .color    = style.foreground_color},
        {.position = offset + extent,
         .uv       = glyph.uv1,
         .color    = style.foreground_color},
        {.position = {offset.x, offset.y + extent.y},
         .uv       = {glyph.uv0.x, glyph.uv1.y},
         .color    = style.foreground_color}};

    draw_list.vertices.extend(vertices).unwrap();

    triangulate_convex_polygon(draw_list.indices, 4);

    draw_list.commands
        .push(
            DrawCommand{.pipeline       = DEFAULT_GLYPH_PIPELINE,
                        .nvertices      = 4,
                        .nindices       = 6,
                        .first_instance = 0,
                        .ninstances     = 1,
                        .scissor_offset = state.scissor_offset,
                        .scissor_extent = state.scissor_extent,
                        .textures       = {atlas}}
                .with_push_constant(transpose(make_transform(block_position))))
        .unwrap();

    restore();
    return *this;
  }

  Canvas &draw_glyph_shadow(Vec2 block_position, Vec2 baseline,
                            f32 text_scale_factor, Glyph const &glyph,
                            GlyphShaping const &shaping, TextStyle const &style,
                            image atlas)
  {
    save();
    state.local_transform = state.local_transform * translate2d(baseline);

    // TODO(lamarrr): add offset to shadow scale? and let offset be from
    // midpoint??
    Vec2 offset = Vec2{glyph.metrics.bearing.x, -glyph.metrics.bearing.y} *
                      style.font_height * text_scale_factor +
                  shaping.offset;
    Vec2 extent = glyph.metrics.extent * style.font_height * text_scale_factor;
    Mat3 transform = state.global_transform *
                     (translate2d(block_position) * state.local_transform);

    Vec2 shadow_offset = offset + style.shadow_offset;
    Vec2 shadow_extent = extent * style.shadow_scale;

    if (!overlaps({0, 0}, viewport_extent,
                  ash::transform(transform, shadow_offset),
                  ash::transform(transform, shadow_offset + shadow_extent)))
    {
      restore();
      return *this;
    }

    Vertex2d const vertices[] = {
        {.position = shadow_offset,
         .uv       = glyph.uv0,
         .color    = style.shadow_color},
        {.position = {shadow_offset.x + shadow_extent.x, shadow_offset.y},
         .uv       = {glyph.uv1.x, glyph.uv0.y},
         .color    = style.shadow_color},
        {.position = shadow_offset + shadow_extent,
         .uv       = glyph.uv1,
         .color    = style.shadow_color},
        {.position = {shadow_offset.x, shadow_offset.y + shadow_extent.y},
         .uv       = {glyph.uv0.x, glyph.uv1.y},
         .color    = style.shadow_color}};

    draw_list.vertices.extend(vertices).unwrap();

    triangulate_convex_polygon(draw_list.indices, 4);

    draw_list.commands
        .push(
            DrawCommand{.pipeline       = DEFAULT_GLYPH_PIPELINE,
                        .nvertices      = 4,
                        .nindices       = 6,
                        .first_instance = 0,
                        .ninstances     = 1,
                        .scissor_offset = state.scissor_offset,
                        .scissor_extent = state.scissor_extent,
                        .textures       = {atlas}}
                .with_push_constant(transpose(make_transform(block_position))))
        .unwrap();

    restore();
    return *this;
  }

  Canvas &draw_text_segment_lines(Vec2 block_position, Vec2 baseline,
                                  f32 line_height, f32 segment_width,
                                  TextStyle const &style)
  {
    save();
    translate(block_position);

    if (style.strikethrough_color.w > 0 && style.strikethrough_thickness > 0)
    {
      Vertex2d const strikethrough_path[] = {
          {.position = baseline - Vec2{0, line_height / 2},
           .uv       = {},
           .color    = style.strikethrough_color},
          {.position = baseline - Vec2{-segment_width, line_height / 2},
           .uv       = {},
           .color    = style.strikethrough_color}};

      draw_path(strikethrough_path, Vec2{0, 0}, Vec2{0, 0},
                style.strikethrough_thickness, false);
    }

    if (style.underline_color.w > 0 && style.underline_thickness > 0)
    {
      Vertex2d const underline_path[] = {
          {.position = baseline, .uv = {}, .color = style.underline_color},
          {.position = baseline + Vec2{segment_width, 0},
           .uv       = {},
           .color    = style.underline_color}};

      draw_path(underline_path, Vec2{0, 0}, Vec2{0, 0},
                style.underline_thickness, false);
    }

    restore();

    return *this;
  }

  Canvas &draw_text_segment_background(Vec2 block_position, Vec2 line_top,
                                       Vec2 extent, TextStyle const &style)
  {
    save();
    translate(block_position);
    draw_rect_filled(line_top, extent, style.background_color);
    restore();
    return *this;
  }

  // TODO(lamarrr): text gradient, reset on each line or continue???? how does
  // css do it?
  Canvas &draw_text(TextBlock const &block, TextLayout const &layout,
                    stx::Span<BundledFont const> font_bundle,
                    Vec2 const                   position)
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

        for (TextRunSegment const &segment : layout.run_segments.span().slice(
                 line.run_segments_offset, line.nrun_segments))
        {
          TextStyle const &segment_style =
              segment.style >= block.styles.size() ?
                  block.default_style :
                  block.styles[segment.style];
          if (segment_style.background_color.w > 0)
          {
            draw_text_segment_background(position, Vec2{x_cursor, line_top},
                                         Vec2{segment.width, line.line_height},
                                         segment_style);
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
        f32 const line_gap =
            max(line.line_height - (line.ascent + line.descent), 0.0f) / 2;
        f32 const baseline =
            line_top + line.line_height - line_gap - line.descent;

        for (TextRunSegment const &segment : layout.run_segments.span().slice(
                 line.run_segments_offset, line.nrun_segments))
        {
          TextStyle const &segment_style =
              segment.style >= block.styles.size() ?
                  block.default_style :
                  block.styles[segment.style];

          if (segment_style.shadow_color.w == 0 ||
              segment_style.shadow_scale <= 0)
          {
            continue;
          }

          FontAtlas const &atlas    = font_bundle[segment.font].atlas;
          f32              x_cursor = x_segment_cursor;

          for (GlyphShaping const &shaping : layout.glyph_shapings.span().slice(
                   segment.glyph_shapings_offset, segment.nglyph_shapings))
          {
            draw_glyph_shadow(
                position, Vec2{x_cursor, baseline}, layout.text_scale_factor,
                atlas.glyphs[shaping.glyph], shaping, segment_style,
                atlas.bins[atlas.glyphs[shaping.glyph].bin].texture);
            x_cursor += shaping.advance +
                        layout.text_scale_factor * segment_style.letter_spacing;
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
        f32 const line_gap =
            max(line.line_height - (line.ascent + line.descent), 0.0f) / 2;
        f32 const baseline =
            line_top + line.line_height - line_gap - line.descent;

        for (TextRunSegment const &segment : layout.run_segments.span().slice(
                 line.run_segments_offset, line.nrun_segments))
        {
          TextStyle const &segment_style =
              segment.style >= block.styles.size() ?
                  block.default_style :
                  block.styles[segment.style];
          FontAtlas const &atlas    = font_bundle[segment.font].atlas;
          f32              x_cursor = x_segment_cursor;

          for (GlyphShaping const &shaping : layout.glyph_shapings.span().slice(
                   segment.glyph_shapings_offset, segment.nglyph_shapings))
          {
            draw_glyph(position, Vec2{x_cursor, baseline},
                       layout.text_scale_factor, atlas.glyphs[shaping.glyph],
                       shaping, segment_style,
                       atlas.bins[atlas.glyphs[shaping.glyph].bin].texture);
            x_cursor += shaping.advance +
                        layout.text_scale_factor * segment_style.letter_spacing;
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
        f32 const line_gap =
            max(line.line_height - (line.ascent + line.descent), 0.0f) / 2;
        f32 const baseline =
            line_top + line.line_height - line_gap - line.descent;

        for (TextRunSegment const &segment : layout.run_segments.span().slice(
                 line.run_segments_offset, line.nrun_segments))
        {
          TextStyle const &segment_style =
              segment.style >= block.styles.size() ?
                  block.default_style :
                  block.styles[segment.style];

          if ((segment_style.underline_color.w > 0 &&
               segment_style.underline_thickness > 0) ||
              (segment_style.strikethrough_color.w > 0 &&
               segment_style.strikethrough_thickness > 0)) [[unlikely]]
          {
            draw_text_segment_lines(position, Vec2{x_cursor, baseline},
                                    line.line_height, segment.width,
                                    segment_style);
          }

          x_cursor += segment.width;
        }

        line_top += line.line_height;
      }
    }

    return *this;
  }
};

}        // namespace ash
