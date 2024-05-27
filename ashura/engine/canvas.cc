#include "ashura/engine/canvas.h"
#include "ashura/renderer/renderer.h"

namespace ash
{

/*
/// outputs (n-2)*3 indices
inline void triangulate_convex_polygon(Vec<u32> &indices, u32 nvertices)
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
                            Vec<Vertex2d> &out)
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
inline void triangulate_line(Span<Vertex2d const> in_points, f32 thickness,
                             Vec<Vertex2d> &out_vertices, Vec<u32> &out_indices,
                             bool should_close)
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
*/

void Canvas::circle(PathStyle const &style, Vec2 center, Vec2 radius)
{
  if (style.stroke)
  {
  }
  else
  {
    CHECK(rrect_params.push(RRectParam{
        .transform = style.transform,
        .radii     = {1, 1, 1, 1},
        .uv        = {style.uv[0], style.uv[1]},
        .tint = {style.tint[0], style.tint[1], style.tint[2], style.tint[3]},
        .aspect_ratio    = {1, 1},
        .edge_smoothness = style.edge_smoothness,
        .albedo          = style.texture}));

    if (pass_runs.is_empty() ||
        pass_runs[pass_runs.size() - 1].pass != CanvasPass::RRect)
    {
      CHECK(pass_runs.push(CanvasPassRun{.pass   = CanvasPass::RRect,
                                         .offset = (u32) rrect_params.size(),
                                         .count  = 1}));
    }
    else
    {
      pass_runs[pass_runs.size() - 1].count++;
    }
  }
}

void Canvas::rect(PathStyle const &style, Vec2 center, Vec2 extent)
{
  if (style.stroke)
  {
  }
  else
  {
    CHECK(rrect_params.push(RRectParam{
        .radii = {0, 0, 0, 0},
        .uv    = {style.uv[0], style.uv[1]},
        .tint  = {style.tint[0], style.tint[1], style.tint[2], style.tint[3]},
        .aspect_ratio    = {1, 1},
        .edge_smoothness = style.edge_smoothness,
        .albedo          = style.texture}));

    if (pass_runs.is_empty() ||
        pass_runs[pass_runs.size() - 1].pass != CanvasPass::RRect)
    {
      CHECK(pass_runs.push(CanvasPassRun{.pass   = CanvasPass::RRect,
                                         .offset = (u32) rrect_params.size(),
                                         .count  = 1}));
    }
    else
    {
      pass_runs[pass_runs.size() - 1].count++;
    }
  }
}

void Canvas::rrect(PathStyle const &style, Vec2 center, Vec2 extent, Vec4 radii)
{
  // todo(lamarrr): scale radii
  if (style.stroke)
  {
  }
  else
  {
    CHECK(rrect_params.push(RRectParam{
        .radii = {0, 0, 0, 0},
        .uv    = {style.uv[0], style.uv[1]},
        .tint  = {style.tint[0], style.tint[1], style.tint[2], style.tint[3]},
        .aspect_ratio    = {1, 1},
        .edge_smoothness = style.edge_smoothness,
        .albedo          = style.texture}));

    if (pass_runs.is_empty() ||
        pass_runs[pass_runs.size() - 1].pass != CanvasPass::RRect)
    {
      CHECK(pass_runs.push(CanvasPassRun{.pass   = CanvasPass::RRect,
                                         .offset = (u32) rrect_params.size(),
                                         .count  = 1}));
    }
    else
    {
      pass_runs[pass_runs.size() - 1].count++;
    }
  }
}

void Canvas::arc(PathStyle const &style, Vec2 center, f32 radius,
                 f32 angle_begin, f32 angle_end)
{
  // path
  if (style.stroke)
  {
  }
  else
  {
  }
}

void Canvas::simple_text(PathStyle const &style, Vec2 baseline,
                         Span<char const> text)
{
  // rrect
}

void Canvas::text(PathStyle const &style, Vec2 center, TextBlock const &block,
                  TextLayout const &layout)
{
  // rrect
}

void Canvas::convex_polygon(PathStyle const     &style,
                            Span<Vertex2d const> vertices)
{
  // convex polygon
}

void Canvas::blur(Vec2U offset, Vec2U extent)
{
  // blur pass
}

void Canvas::custom(CustomCanvasPassInfo const &pass)
{
}

}        // namespace ash
