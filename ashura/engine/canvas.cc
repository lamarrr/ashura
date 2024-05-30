#include "ashura/engine/canvas.h"
#include "ashura/engine/renderer.h"

namespace ash
{

void Canvas::begin()
{
  // Canvas params for transform from px to -1 +1 relative viewport space
  // for rrect, transform needs to transform from -1 +1 to px space
  // rrect_params.clear();
  // blur_params.clear();
}

void Canvas::end()
{
  // offload to gpu, set up passes, render
}

void Canvas::circle(CanvasStyle const &style, Vec2 center, Vec2 radii)
{
  if (style.stroke)
  {
  }
  else
  {
    // TODO(lamarrr):radii
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

void Canvas::rect(CanvasStyle const &style, Vec2 center, Vec2 extent)
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

void Canvas::rrect(CanvasStyle const &style, Vec2 center, Vec2 extent,
                   Vec4 radii)
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

void Canvas::arc(CanvasStyle const &style, Vec2 center, f32 radius,
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

void Canvas::simple_text(CanvasStyle const &style, Vec2 baseline,
                         Span<char const> text)
{
  // rrect
}

void Canvas::text(CanvasStyle const &style, Vec2 center, TextBlock const &block,
                  TextLayout const &layout)
{
  // rrect
}

void Canvas::convex_polygon(CanvasStyle const &style,
                            Span<Vertex const> vertices)
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
