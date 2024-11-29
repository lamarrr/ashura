/// SPDX-License-Identifier: MIT
#include "ashura/engine/canvas.h"
#include "ashura/engine/font_atlas.h"
#include "ashura/std/math.h"

namespace ash
{

void path::rect(Vec<Vec2> &vtx)
{
  vtx.extend_copy(span<Vec2>({{-1, -1}, {1, -1}, {1, 1}, {-1, 1}})).unwrap();
}

void path::arc(Vec<Vec2> &vtx, f32 start, f32 stop, u32 segments)
{
  if (segments < 2)
  {
    return;
  }

  u32 const first = vtx.size32();

  vtx.extend_uninit(segments).unwrap();

  f32 const step = (stop - start) / (segments - 1);

  for (u32 i = 0; i < segments; i++)
  {
    vtx[first + i] = rotor(i * step) * 2 - 1;
  }
}

void path::circle(Vec<Vec2> &vtx, u32 segments)
{
  if (segments < 4)
  {
    return;
  }

  u32 const first = vtx.size32();

  vtx.extend_uninit(segments).unwrap();

  f32 const step = (2 * PI) / (segments - 1);

  for (u32 i = 0; i < segments; i++)
  {
    vtx[first + i] = rotor(i * step);
  }
}

void path::squircle(Vec<Vec2> &vtx, f32 elasticity, u32 segments)
{
  if (segments < 128)
  {
    return;
  }

  elasticity = clamp(elasticity, 0.0F, 1.0F);

  path::cubic_bezier(vtx, {0, -1}, {elasticity, -1}, {1, -1}, {1, 0},
                     segments >> 2);
  path::cubic_bezier(vtx, {1, 0}, {1, elasticity}, {1, 1}, {0, 1},
                     segments >> 2);
  path::cubic_bezier(vtx, {0, 1}, {-elasticity, 1}, {-1, 1}, {-1, 0},
                     segments >> 2);
  path::cubic_bezier(vtx, {-1, 0}, {-1, -elasticity}, {-1, -1}, {0, -1},
                     segments >> 2);
}

void path::rrect(Vec<Vec2> &vtx, Vec4 radii, u32 segments)
{
  if (segments < 8)
  {
    return;
  }

  radii   = radii * 2;
  radii.x = min(radii.x, 2.0F);
  radii.y = min(radii.y, 2.0F);
  radii.z = min(radii.z, 2.0F);
  radii.w = min(radii.w, 2.0F);

  /// clipping
  radii.y          = min(radii.y, 2.0F - radii.x);
  f32 max_radius_z = min(2.0F - radii.x, 1.0F - radii.y);
  radii.z          = min(radii.z, max_radius_z);
  f32 max_radius_w = min(max_radius_z, 1.0F - radii.z);
  radii.w          = min(radii.w, max_radius_w);

  u32 const curve_segments = (segments - 8) / 4;
  f32 const step  = (curve_segments == 0) ? 0.0F : ((PI / 2) / curve_segments);
  u32 const first = vtx.size32();

  vtx.extend_uninit(segments).unwrap();

  u32 i = 0;

  vtx[first + i++] = Vec2{1, 1 - radii.z};

  for (u32 s = 0; s < curve_segments; s++)
  {
    vtx[first + i++] = (1 - radii.z) + radii.z * rotor(s * step);
  }

  vtx[first + i++] = Vec2{1 - radii.z, 1};

  vtx[first + i++] = Vec2{-1 + radii.w, 1};

  for (u32 s = 0; s < curve_segments; s++)
  {
    vtx[first + i++] =
        Vec2{-1 + radii.w, 1 - radii.w} + radii.w * rotor(PI / 2 + s * step);
  }

  vtx[first + i++] = Vec2{-1, 1 - radii.w};

  vtx[first + i++] = Vec2{-1, -1 + radii.x};

  for (u32 s = 0; s < curve_segments; s++)
  {
    vtx[first + i++] = (-1 + radii.x) + radii.x * rotor(PI + s * step);
  }

  vtx[first + i++] = Vec2{-1 + radii.x, -1};

  vtx[first + i++] = Vec2{1 - radii.y, -1};

  for (u32 s = 0; s < curve_segments; s++)
  {
    vtx[first + i++] = Vec2{1 - radii.y, (-1 + radii.y)} +
                       radii.y * rotor(PI * 3.0F / 2.0F + s * step);
  }

  vtx[first + i++] = Vec2{1, -1 + radii.y};
}

void path::brect(Vec<Vec2> &vtx, Vec4 slant)
{
  slant   = slant * 2.0F;
  slant.x = min(slant.x, 2.0F);
  slant.y = min(slant.y, 2.0F);
  slant.z = min(slant.z, 2.0F);
  slant.w = min(slant.w, 2.0F);

  slant.y          = min(slant.y, 2.0F - slant.x);
  f32 max_radius_z = min(2.0F - slant.x, 2.0F - slant.y);
  slant.z          = min(slant.z, max_radius_z);
  f32 max_radius_w = min(max_radius_z, 2.0F - slant.z);
  slant.w          = min(slant.w, max_radius_w);

  Vec2 const vertices[] = {{-1 + slant.x, -1}, {1 - slant.y, -1},
                           {1, -1 + slant.y},  {1, 1 - slant.z},
                           {1 - slant.z, 1},   {-1 + slant.w, 1},
                           {-1, 1 - slant.w},  {-1, -1 + slant.x}};

  vtx.extend_copy(vertices).unwrap();
}

void path::bezier(Vec<Vec2> &vtx, Vec2 cp0, Vec2 cp1, Vec2 cp2, u32 segments)
{
  if (segments < 3)
  {
    return;
  }

  u32 const first = vtx.size32();

  vtx.extend_uninit(segments).unwrap();

  f32 const step = 1.0F / (segments - 1);

  for (u32 i = 0; i < segments; i++)
  {
    vtx[first + i] = Vec2{ash::bezier(cp0.x, cp1.x, cp2.x, step * i),
                          ash::bezier(cp0.y, cp1.y, cp2.y, step * i)};
  }
}

void path::cubic_bezier(Vec<Vec2> &vtx, Vec2 cp0, Vec2 cp1, Vec2 cp2, Vec2 cp3,
                        u32 segments)
{
  if (segments < 4)
  {
    return;
  }

  u32 const first = vtx.size32();

  vtx.extend_uninit(segments).unwrap();

  f32 const step = 1.0F / (segments - 1);

  for (u32 i = 0; i < segments; i++)
  {
    vtx[first + i] =
        Vec2{ash::cubic_bezier(cp0.x, cp1.x, cp2.x, cp3.x, step * i),
             ash::cubic_bezier(cp0.y, cp1.y, cp2.y, cp3.y, step * i)};
  }
}

void path::catmull_rom(Vec<Vec2> &vtx, Vec2 cp0, Vec2 cp1, Vec2 cp2, Vec2 cp3,
                       u32 segments)
{
  if (segments < 4)
  {
    return;
  }

  u32 const beg = vtx.size32();

  vtx.extend_uninit(segments).unwrap();

  f32 const step = 1.0F / (segments - 1);

  for (u32 i = 0; i < segments; i++)
  {
    vtx[beg + i] =
        Vec2{ash::cubic_bezier(cp0.x, cp1.x, cp2.x, cp3.x, step * i),
             ash::cubic_bezier(cp0.y, cp1.y, cp2.y, cp3.y, step * i)};
  }
}

void path::triangulate_stroke(Span<Vec2 const> points, Vec<Vec2> &vertices,
                              Vec<u32> &indices, f32 thickness)
{
  if (points.size() < 2)
  {
    return;
  }

  u32 const first_vtx    = vertices.size32();
  u32 const first_idx    = indices.size32();
  u32 const num_points   = points.size32();
  u32 const num_vertices = (num_points - 1) * 4;
  u32 const num_indices  = (num_points - 1) * 6 + (num_points - 2) * 6;
  vertices.extend_uninit(num_vertices).unwrap();
  indices.extend_uninit(num_indices).unwrap();

  Vec2 *vtx  = vertices.data() + first_vtx;
  u32  *idx  = indices.data() + first_idx;
  u32   ivtx = 0;

  for (u32 i = 0; i < num_points - 1; i++)
  {
    Vec2 const p0    = points[i];
    Vec2 const p1    = points[i + 1];
    Vec2 const d     = p1 - p0;
    f32 const  alpha = atanf(d.y / d.x);

    // parallel angle
    Vec2 const down = (thickness / 2) * rotor(alpha + PI / 2);

    // perpendicular angle
    Vec2 const up = -down;

    vtx[0] = p0 + up;
    vtx[1] = p0 + down;
    vtx[2] = p1 + up;
    vtx[3] = p1 + down;
    idx[0] = ivtx;
    idx[1] = ivtx + 1;
    idx[2] = ivtx + 3;
    idx[3] = ivtx;
    idx[4] = ivtx + 3;
    idx[5] = ivtx + 2;

    if (i != 0)
    {
      u32 prev = ivtx - 2;
      idx[6]   = prev;
      idx[7]   = prev + 1;
      idx[8]   = ivtx + 1;
      idx[9]   = prev;
      idx[10]  = prev + 1;
      idx[11]  = ivtx;
      idx += 6;
    }

    idx += 6;
    vtx += 4;
    ivtx += 4;
  }
}

void path::triangles(u32 first_vertex, u32 num_vertices, Vec<u32> &indices)
{
  CHECK(num_vertices > 3);
  u32 const num_triangles = num_vertices / 3;
  u32 const first_idx     = indices.size32();
  indices.extend_uninit(num_triangles * 3).unwrap();

  u32 *idx = indices.data() + first_idx;
  for (u32 i = 0; i < num_triangles * 3; i += 3)
  {
    idx[i]     = first_vertex + i;
    idx[i + 1] = first_vertex + i + 1;
    idx[i + 2] = first_vertex + i + 2;
  }
}

void path::triangulate_convex(Vec<u32> &idx, u32 first_vertex, u32 num_vertices)
{
  if (num_vertices < 3)
  {
    return;
  }

  u32 const num_indices = (num_vertices - 2) * 3;
  u32 const first_index = idx.size32();

  idx.extend_uninit(num_indices).unwrap();

  for (u32 i = 0, v = 1; i < num_indices; i += 3, v++)
  {
    idx[first_index + i]     = first_vertex;
    idx[first_index + i + 1] = first_vertex + v;
    idx[first_index + i + 2] = first_vertex + v + 1;
  }
}

Canvas &Canvas::reset()
{
  frame_arena.reclaim();
  passes.clear();
  rrect_params.clear();
  ngon_params.clear();
  ngon_vertices.clear();
  ngon_indices.clear();
  ngon_index_counts.clear();
  passes.clear();
  batch = {};

  return *this;
}

Canvas &Canvas::begin_recording(Vec2  new_viewport_extent,
                                Vec2U new_surface_extent)
{
  reset();

  viewport_extent = new_viewport_extent;
  surface_extent  = new_surface_extent;

  if (new_viewport_extent.y == 0 || new_viewport_extent.x == 0)
  {
    viewport_aspect_ratio = 1;
  }
  else
  {
    viewport_aspect_ratio = viewport_extent.x / viewport_extent.y;
  }

  world_to_view =
      translate3d(Vec3{-1, -1, 0}) * scale3d(vec3(2 / viewport_extent, 1));

  return *this;
}

constexpr RectU clip_to_scissor(gpu::Viewport const &viewport,
                                CRect const         &clip)
{
  // clips only apply translations. no scaling
  Rect s{viewport.offset + clip.begin(), clip.extent};

  s.offset.x = clamp(s.offset.x, 0.0F, viewport.extent.x);

  s.offset.y = clamp(s.offset.y, 0.0F, viewport.extent.y);

  s.extent.x =
      clamp(s.offset.x + s.extent.x, 0.0F, viewport.extent.x) - s.offset.x;

  s.extent.y =
      clamp(s.offset.y + s.extent.y, 0.0F, viewport.extent.y) - s.offset.y;

  return RectU{.offset{(u32) s.offset.x, (u32) s.offset.y},
               .extent{(u32) s.extent.x, (u32) s.extent.y}};
}

static inline void flush_batch(Canvas &c)
{
  Canvas::Batch batch = c.batch;
  c.batch             = Canvas::Batch{.type = Canvas::BatchType::None};

  switch (batch.type)
  {
    case Canvas::BatchType::RRect:
      c.add_pass("RRect"_str, [batch, world_to_view = c.world_to_view](
                                  Canvas::RenderContext const &ctx) {
        RRectPassParams params{.rendering_info = ctx.rt.info,
                               .scissor =
                                   clip_to_scissor(ctx.rt.viewport, batch.clip),
                               .viewport       = ctx.rt.viewport,
                               .world_to_view  = world_to_view,
                               .params_ssbo    = ctx.rrects.descriptor,
                               .textures       = ctx.gpu.texture_views,
                               .first_instance = batch.objects.offset,
                               .num_instances  = batch.objects.span};

        ctx.passes.rrect->encode(ctx.gpu, ctx.enc, params);
      });
      return;

    case Canvas::BatchType::Ngon:
      c.add_pass("Ngon"_str, [batch, world_to_view = c.world_to_view](
                                 Canvas::RenderContext const &ctx) {
        NgonPassParams params{
            .rendering_info = ctx.rt.info,
            .scissor        = clip_to_scissor(ctx.rt.viewport, batch.clip),
            .viewport       = ctx.rt.viewport,
            .world_to_view  = world_to_view,
            .vertices_ssbo  = ctx.ngon_vertices.descriptor,
            .indices_ssbo   = ctx.ngon_indices.descriptor,
            .params_ssbo    = ctx.ngons.descriptor,
            .textures       = ctx.gpu.texture_views,
            .index_counts =
                span(ctx.canvas.ngon_index_counts).slice(batch.objects)};
        ctx.passes.ngon->encode(ctx.gpu, ctx.enc, params);
      });
      return;

    default:
      return;
  }
}

static inline void add_rrect(Canvas &c, RRectParam const &param,
                             CRect const &clip)
{
  u32 const index = c.rrect_params.size32();
  c.rrect_params.push(param).unwrap();

  if (c.batch.type != Canvas::BatchType::RRect || c.batch.clip != clip)
      [[unlikely]]
  {
    flush_batch(c);
    c.batch = Canvas::Batch{.type = Canvas::BatchType::RRect,
                            .clip = clip,
                            .objects{.offset = index, .span = 1}};
    return;
  }

  c.batch.objects.span++;
}

static inline void add_ngon(Canvas &c, NgonParam const &param,
                            CRect const &clip, u32 num_indices)
{
  u32 const index = c.ngon_params.size32();
  c.ngon_index_counts.push(num_indices).unwrap();
  c.ngon_params.push(param).unwrap();

  if (c.batch.type != Canvas::BatchType::Ngon || c.batch.clip != clip)
      [[unlikely]]
  {
    flush_batch(c);
    c.batch = Canvas::Batch{.type = Canvas::BatchType::Ngon,
                            .clip = clip,
                            .objects{.offset = index, .span = 1}};
    return;
  }

  c.batch.objects.span++;
}

Canvas &Canvas::end_recording()
{
  flush_batch(*this);
  return *this;
}

Canvas &Canvas::clip(CRect const &c)
{
  current_clip = c;
  return *this;
}

constexpr Mat4 object_to_world(Mat4 const &transform, Vec2 center, Vec2 extent)
{
  return transform * translate3d(vec3(center, 0)) *
         scale3d(vec3(extent * 0.5F, 1));
}

Canvas &Canvas::circle(ShapeInfo const &info)
{
  add_rrect(*this,
            RRectParam{.transform = object_to_world(info.transform, info.center,
                                                    info.extent),
                       .tint      = {info.tint[0], info.tint[1], info.tint[2],
                                     info.tint[3]},
                       .radii     = {1, 1, 1, 1},
                       .uv        = {info.uv[0], info.uv[1]},
                       .tiling    = info.tiling,
                       .aspect_ratio    = info.extent.x / info.extent.y,
                       .stroke          = info.stroke,
                       .thickness       = info.thickness / info.extent.y,
                       .edge_smoothness = info.edge_smoothness,
                       .sampler         = info.sampler,
                       .albedo          = info.texture},
            current_clip);

  return *this;
}

Canvas &Canvas::rect(ShapeInfo const &info)
{
  add_rrect(*this,
            RRectParam{.transform = object_to_world(info.transform, info.center,
                                                    info.extent),
                       .tint      = {info.tint[0], info.tint[1], info.tint[2],
                                     info.tint[3]},
                       .radii     = {0, 0, 0, 0},
                       .uv        = {info.uv[0], info.uv[1]},
                       .tiling    = info.tiling,
                       .aspect_ratio    = info.extent.x / info.extent.y,
                       .stroke          = info.stroke,
                       .thickness       = info.thickness / info.extent.y,
                       .edge_smoothness = info.edge_smoothness,
                       .sampler         = info.sampler,
                       .albedo          = info.texture},
            current_clip);
  return *this;
}

Canvas &Canvas::rrect(ShapeInfo const &info)
{
  add_rrect(*this,
            RRectParam{.transform = object_to_world(info.transform, info.center,
                                                    info.extent),
                       .tint      = {info.tint[0], info.tint[1], info.tint[2],
                                     info.tint[3]},
                       .radii     = info.corner_radii / info.extent.y,
                       .uv        = {info.uv[0], info.uv[1]},
                       .tiling    = info.tiling,
                       .aspect_ratio    = info.extent.x / info.extent.y,
                       .stroke          = info.stroke,
                       .thickness       = info.thickness / info.extent.y,
                       .edge_smoothness = info.edge_smoothness,
                       .sampler         = info.sampler,
                       .albedo          = info.texture},
            current_clip);
  return *this;
}

Canvas &Canvas::brect(ShapeInfo const &info)
{
  u32 const first_vertex = ngon_vertices.size32();
  u32 const first_index  = ngon_indices.size32();

  path::brect(ngon_vertices, info.corner_radii);

  u32 const num_vertices = ngon_vertices.size32() - first_vertex;

  path::triangulate_convex(ngon_indices, first_vertex, num_vertices);

  u32 const num_indices = ngon_indices.size32() - first_index;

  add_ngon(*this,
           NgonParam{
               .transform =
                   object_to_world(info.transform, info.center, info.extent),
               .tint = {info.tint[0], info.tint[1], info.tint[2], info.tint[3]},
               .uv   = {info.uv[0], info.uv[1]},
               .tiling       = info.tiling,
               .sampler      = info.sampler,
               .albedo       = info.texture,
               .first_index  = first_index,
               .first_vertex = first_vertex},
           current_clip, num_indices);

  return *this;
}

Canvas &Canvas::squircle(ShapeInfo const &info, f32 elasticity, u32 segments)
{
  u32 const first_vertex = ngon_vertices.size32();
  u32 const first_index  = ngon_indices.size32();

  path::squircle(ngon_vertices, elasticity, segments);

  u32 const num_vertices = ngon_vertices.size32() - first_vertex;

  path::triangulate_convex(ngon_indices, first_vertex, num_vertices);

  u32 const num_indices = ngon_indices.size32() - first_index;

  add_ngon(*this,
           NgonParam{
               .transform =
                   object_to_world(info.transform, info.center, info.extent),
               .tint = {info.tint[0], info.tint[1], info.tint[2], info.tint[3]},
               .uv   = {info.uv[0], info.uv[1]},
               .tiling       = info.tiling,
               .sampler      = info.sampler,
               .albedo       = info.texture,
               .first_index  = first_index,
               .first_vertex = first_vertex},
           current_clip, num_indices);

  return *this;
}

Canvas &Canvas::text(ShapeInfo const &info, TextBlock const &block,
                     TextLayout const &layout, TextBlockStyle const &style,
                     CRect const &clip)
{
  CHECK(style.runs.size() == block.runs.size());
  CHECK(style.runs.size() == block.fonts.size());

  f32 const  block_width = max(layout.extent.x, style.align_width);
  Vec2 const block_extent{block_width, layout.extent.y};

  constexpr u8 PASS_BACKGROUND    = 0;
  constexpr u8 PASS_GLYPH_SHADOWS = 1;
  constexpr u8 PASS_GLYPHS        = 2;
  constexpr u8 PASS_UNDERLINE     = 3;
  constexpr u8 PASS_STRIKETHROUGH = 4;
  constexpr u8 NUM_PASSES         = 5;

  for (u8 pass = 0; pass < NUM_PASSES; pass++)
  {
    f32 line_y = -block_extent.y * 0.5F;
    for (Line const &ln : layout.lines)
    {
      if (!overlaps(clip, CRect{.center = info.center + line_y,
                                .extent = {block_width, ln.metrics.height}}))
      {
        continue;
      }
      line_y += ln.metrics.height;
      f32 const           baseline  = line_y - ln.metrics.descent;
      TextDirection const direction = level_to_direction(ln.metrics.level);
      // flip the alignment axis direction if it is an RTL line
      f32 const alignment =
          style.alignment *
          ((direction == TextDirection::LeftToRight) ? 1 : -1);
      f32 cursor = space_align(block_width, ln.metrics.width, alignment) -
                   ln.metrics.width * 0.5F;
      for (TextRun const &run :
           span(layout.runs).slice(ln.first_run, ln.num_runs))
      {
        FontStyle const    &font_style = block.fonts[run.style];
        TextStyle const    &run_style  = style.runs[run.style];
        FontInfo const      font       = font_style.font->info();
        GpuFontAtlas const *atlas      = font.gpu_atlas.value();
        f32 const run_width = au_to_px(run.metrics.advance, run.font_height);

        if (pass == PASS_BACKGROUND && !run_style.background.is_transparent())
        {
          Vec2 extent{run_width,
                      au_to_px(run.metrics.height(), run.font_height)};
          Vec2 center =
              Vec2{cursor + extent.x * 0.5F,
                   baseline - au_to_px(run.metrics.ascent, run.font_height) +
                       extent.y * 0.5F};
          rect({.center    = info.center,
                .extent    = extent,
                .transform = info.transform * translate3d(vec3(center, 0)),
                .tint      = run_style.background});
        }

        f32 glyph_cursor = cursor;
        for (u32 g = 0; g < run.num_glyphs; g++)
        {
          GlyphShape const &sh  = layout.glyphs[run.first_glyph + g];
          Glyph const      &gl  = font.glyphs[sh.glyph];
          AtlasGlyph const &agl = atlas->glyphs[sh.glyph];
          Vec2 const extent     = au_to_px(gl.metrics.extent, run.font_height);
          Vec2 const center     = Vec2{glyph_cursor, baseline} +
                              au_to_px(gl.metrics.bearing, run.font_height) +
                              au_to_px(sh.offset, run.font_height) +
                              extent * 0.5F;

          if (pass == PASS_GLYPH_SHADOWS && run_style.shadow_scale != 0 &&
              !run_style.shadow.is_transparent())
          {
            Vec2 shadow_extent = extent * run_style.shadow_scale;
            Vec2 shadow_center = center + run_style.shadow_offset;
            rect({.center = info.center,
                  .extent = shadow_extent,
                  .transform =
                      info.transform * translate3d(vec3(shadow_center, 0)),
                  .tint            = run_style.shadow,
                  .sampler         = info.sampler,
                  .texture         = atlas->textures[agl.layer],
                  .uv              = {agl.uv[0], agl.uv[1]},
                  .tiling          = info.tiling,
                  .edge_smoothness = info.edge_smoothness});
          }

          if (pass == PASS_GLYPHS && !run_style.foreground.is_transparent())
          {
            rect({.center    = info.center,
                  .extent    = extent,
                  .transform = info.transform * translate3d(vec3(center, 0)),
                  .tint      = run_style.foreground,
                  .sampler   = info.sampler,
                  .texture   = atlas->textures[agl.layer],
                  .uv        = {agl.uv[0], agl.uv[1]},
                  .tiling    = info.tiling,
                  .edge_smoothness = info.edge_smoothness});
          }

          glyph_cursor += au_to_px(sh.advance, run.font_height);
        }

        if (pass == PASS_STRIKETHROUGH &&
            run_style.strikethrough_thickness != 0)
        {
          Vec2 extent{run_width, run_style.strikethrough_thickness};
          Vec2 center = Vec2{cursor, baseline - run.metrics.ascent * 0.5F} +
                        extent * 0.5F;
          rect({.center    = info.center,
                .extent    = extent,
                .transform = info.transform * translate3d(vec3(center, 0)),
                .tint      = run_style.strikethrough,
                .sampler   = info.sampler,
                .texture   = 0,
                .uv        = {},
                .tiling    = info.tiling,
                .edge_smoothness = info.edge_smoothness});
        }

        if (pass == PASS_UNDERLINE && run_style.underline_thickness != 0)
        {
          Vec2 extent{run_width, run_style.underline_thickness};
          Vec2 center = Vec2{cursor, baseline + 2} + extent * 0.5F;
          rect({.center    = info.center,
                .extent    = extent,
                .transform = info.transform * translate3d(vec3(center, 0)),
                .tint      = run_style.underline,
                .sampler   = info.sampler,
                .texture   = 0,
                .uv        = {},
                .tiling    = info.tiling,
                .edge_smoothness = info.edge_smoothness});
        }
        cursor += run_width;
      }
    }
  }

  return *this;
}

Canvas &Canvas::triangles(ShapeInfo const &info, Span<Vec2 const> points)
{
  if (points.size() < 3)
  {
    return *this;
  }

  u32 const first_index  = ngon_indices.size32();
  u32 const first_vertex = ngon_vertices.size32();

  ngon_vertices.extend_copy(points).unwrap();
  path::triangles(first_vertex, points.size32(), ngon_indices);

  u32 const num_indices = ngon_vertices.size32() - first_vertex;

  add_ngon(*this,
           NgonParam{
               .transform =
                   object_to_world(info.transform, info.center, info.extent),
               .tint = {info.tint[0], info.tint[1], info.tint[2], info.tint[3]},
               .uv   = {info.uv[0], info.uv[1]},
               .tiling       = info.tiling,
               .sampler      = info.sampler,
               .albedo       = info.texture,
               .first_index  = first_index,
               .first_vertex = first_vertex},
           current_clip, num_indices);

  return *this;
}

Canvas &Canvas::triangles(ShapeInfo const &info, Span<Vec2 const> points,
                          Span<u32 const> idx)
{
  if (points.size() < 3)
  {
    return *this;
  }

  u32 const first_index  = ngon_indices.size32();
  u32 const first_vertex = ngon_vertices.size32();

  ngon_vertices.extend_copy(points).unwrap();
  ngon_indices.extend_copy(idx).unwrap();

  for (u32 &v : span(ngon_indices).slice(first_index))
  {
    v += first_vertex;
  }

  add_ngon(*this,
           NgonParam{
               .transform =
                   object_to_world(info.transform, info.center, info.extent),
               .tint = {info.tint[0], info.tint[1], info.tint[2], info.tint[3]},
               .uv   = {info.uv[0], info.uv[1]},
               .tiling       = info.tiling,
               .sampler      = info.sampler,
               .albedo       = info.texture,
               .first_index  = first_index,
               .first_vertex = first_vertex},
           current_clip, idx.size32());

  return *this;
}

Canvas &Canvas::line(ShapeInfo const &info, Span<Vec2 const> points)
{
  if (points.size() < 2)
  {
    return *this;
  }

  u32 const first_index  = ngon_indices.size32();
  u32 const first_vertex = ngon_vertices.size32();
  path::triangulate_stroke(points, ngon_vertices, ngon_indices,
                           info.thickness / info.extent.y);

  u32 const num_indices = ngon_indices.size32() - first_index;

  add_ngon(*this,
           NgonParam{
               .transform =
                   object_to_world(info.transform, info.center, info.extent),
               .tint = {info.tint[0], info.tint[1], info.tint[2], info.tint[3]},
               .uv   = {info.uv[0], info.uv[1]},
               .tiling       = info.tiling,
               .sampler      = info.sampler,
               .albedo       = info.texture,
               .first_index  = first_index,
               .first_vertex = first_vertex},
           current_clip, num_indices);

  return *this;
}

Canvas &Canvas::blur(CRect const &area, u32 num_passes)
{
  flush_batch(*this);

  add_pass("Blur"_str, [num_passes, area](Canvas::RenderContext const &ctx) {
    BlurPassParams params{.image_view   = ctx.rt.info.color_attachments[0].view,
                          .extent       = ctx.rt.extent,
                          .texture_view = ctx.rt.color_descriptor,
                          .texture      = 0,
                          .passes       = num_passes,
                          .area = clip_to_scissor(ctx.rt.viewport, area)};
    ctx.passes.blur->encode(ctx.gpu, ctx.enc, params);
  });

  return *this;
}

Canvas &Canvas::add_pass(Pass &&pass)
{
  flush_batch(*this);
  passes.push(std::move(pass)).unwrap();
  return *this;
}

}        // namespace ash
