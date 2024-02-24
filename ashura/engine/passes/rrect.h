#pragma once
#include "ashura/engine/renderer.h"

namespace ash
{

typedef struct RRect         RRect;
typedef struct RRectMaterial RRectMaterial;
typedef struct RRectDesc     RRectDesc;
typedef struct RRectObject   RRectObject;
typedef struct RRectPass     RRectPass;

struct RRect
{
  Vec3 center           = {};
  Vec3 half_extent      = {};
  f32  border_thickness = 0;
  Vec4 border_radii     = {};
};

struct RRectMaterial
{
  Texture base_color_texture    = {};
  Vec4    base_color_factors[4] = {};
  Vec4    border_colors[4]      = {};
};

struct RRectObject
{
  RRect         rrect                 = {};
  RRectMaterial material              = {};
  u32           descriptor_heap_group = 0;
};

struct RRectParams
{
  Span<RRectObject const>  objects;
  gfx::DescriptorSetLayout descriptor_set_layout = nullptr;
  gfx::DescriptorHeapImpl  descriptor_heap       = {};
  gfx::Sampler             sampler               = nullptr;
  gfx::GraphicsPipeline    pipeline              = nullptr;
};

struct RRectPass
{
  static void init(Pass self, RenderServer *server, uid32 id);
  static void deinit(Pass self, RenderServer *server);

  static constexpr PassInterface const interface{.init   = init,
                                                 .deinit = deinit};

  void execute(rdg::RenderGraph *graph, RRectParams const *params);
};

}        // namespace ash
