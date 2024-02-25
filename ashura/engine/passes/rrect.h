#pragma once
#include "ashura/engine/camera.h"
#include "ashura/engine/light.h"
#include "ashura/engine/render_graph.h"

namespace ash
{
typedef struct RRectTexture  RRectTexture;
typedef struct RRect         RRect;
typedef struct RRectMaterial RRectMaterial;
typedef struct RRectDesc     RRectDesc;
typedef struct RRectObject   RRectObject;
typedef struct RRectPass     RRectPass;

struct RRectTexture
{
  gfx::ImageView view = nullptr;
  Vec2           uv0  = {};
  Vec2           uv1  = {};
};

struct RRect
{
  Vec3 center           = {};
  Vec3 half_extent      = {};
  f32  border_thickness = 0;
  Vec4 border_radii     = {};
};

struct RRectMaterial
{
  RRectTexture base_color_texture    = {};
  Vec4         base_color_factors[4] = {};
  Vec4         border_colors[4]      = {};
};

struct RRectObject
{
  RRect         rrect                 = {};
  RRectMaterial material              = {};
  u32           descriptor_heap_group = 0;
};

struct RRectParams
{
  rdg::RenderTarget        render_target;
  Span<RRectObject const>  objects;
  gfx::DescriptorSetLayout descriptor_set_layout = nullptr;
  gfx::DescriptorHeapImpl  descriptor_heap       = {};
};

struct RRectPass
{
  static void create_rgb_textures(RenderGraph *graph);
  static void add_pass(RenderGraph *graph, RRectParams const *params);
};

}        // namespace ash
