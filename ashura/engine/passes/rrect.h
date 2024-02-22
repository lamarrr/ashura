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

struct RRectDesc
{
  RRect         rrect    = {};
  RRectMaterial material = {};
};

struct RRectObject
{
  RRectDesc desc                  = {};
  u32       descriptor_heap_group = 0;
};

struct RRectPass
{
  Vec<RRectObject>         objects               = {};
  SparseVec<u32>           id_map                = {};
  gfx::DescriptorSetLayout descriptor_set_layout = nullptr;
  gfx::DescriptorHeapImpl  descriptor_heap       = {};
  gfx::PipelineCache       pipeline_cache        = nullptr;
  gfx::GraphicsPipeline    pipeline              = nullptr;
  gfx::Sampler             sampler               = nullptr;

  static void init(Pass self, RenderServer *server, uid32 id);
  static void deinit(Pass self, RenderServer *server);
  static void acquire_scene(Pass self, RenderServer *server, uid32 scene);
  static void release_scene(Pass self, RenderServer *server, uid32 scene);
  static void acquire_view(Pass self, RenderServer *server, uid32 view);
  static void release_view(Pass self, RenderServer *server, uid32 view);
  static void release_object(Pass self, RenderServer *server, uid32 scene,
                             uid32 object);
  static void begin_frame(Pass self, RenderServer *server,
                          gfx::CommandEncoderImpl const *encoder);
  static void end_frame(Pass self, RenderServer *server,
                        gfx::CommandEncoderImpl const *encoder);
  static void begin(Pass self, RenderServer *server, PassBeginInfo const *info);
  static void encode(Pass self, RenderServer *server,
                     PassEncodeInfo const *info);
  static void end(Pass self, RenderServer *server, PassEndInfo const *info);

  static constexpr PassInterface const interface{.init          = init,
                                                 .deinit        = deinit,
                                                 .acquire_scene = acquire_scene,
                                                 .release_scene = release_scene,
                                                 .acquire_view  = acquire_view,
                                                 .release_view  = release_view,
                                                 .release_object =
                                                     release_object,
                                                 .begin_frame = begin_frame,
                                                 .end_frame   = end_frame,
                                                 .begin       = begin,
                                                 .encode      = encode,
                                                 .end         = end};

  u64  add_object(Scene *scene, RRect const &rrect,
                  RRectMaterial const &material, i64 z_index);
  void remove_object(Scene *scene, u64 object);
};

}        // namespace ash
