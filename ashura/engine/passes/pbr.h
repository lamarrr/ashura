#pragma once
#include "ashura/engine/renderer.h"

namespace ash
{

typedef struct PBRMaterial PBRMaterial;
typedef struct PBRVertex   PBRVertex;
typedef struct PBRMesh     PBRMesh;
typedef struct PBRObject   PBRObject;
typedef struct PBRPass     PBRPass;

/// SEE: https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos
/// SEE:
/// https://github.com/KhronosGroup/glTF-Sample-Viewer/blob/main/source/Renderer/shaders/textures.glsl
struct PBRMaterial
{
  Texture base_color_texture = {};
  Texture metallic_texture   = {};
  Texture roughness_texture  = {};
  Texture normal_texture     = {};
  Texture occlusion_texture  = {};
  Texture emissive_texture   = {};
  Vec4    base_color_factor  = {1, 1, 1, 1};
  f32     metallic_factor    = 1;
  f32     roughness_factor   = 1;
  f32     normal_scale       = 1;
  f32     occlusion_strength = 1;
  Vec3    emissive_factor    = {1, 1, 1};
  f32     emissive_strength  = 1;
  bool    unlit              = false;
};

struct PBRVertex
{
  f32 x = 0, y = 0, z = 0;
  f32 u = 0, v = 0;
};

struct PBRMesh
{
  uid32          vertex_buffer = 0;
  uid32          index_buffer  = 0;
  u32            first_index   = 0;
  u32            num_indices   = 0;
  gfx::IndexType index_type    = gfx::IndexType::Uint16;
};

struct PBRObject
{
  PBRMaterial material              = {};
  PBRMesh     mesh                  = {};
  uid32       scene_id              = UID32_INVALID;
  uid32       scene_object_id       = UID32_INVALID;
  u32         descriptor_heap_index = 0;
};

struct PBRPass
{
  Vec<PBRObject>           objects               = {};
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
  static void sort(Pass self, RenderServer *server, PassSortInfo const *info);
  static void begin(Pass self, RenderServer *server,
                    gfx::CommandEncoderImpl const *encoder);
  static void encode(Pass self, RenderServer *server,
                     gfx::CommandEncoderImpl const *encoder);
  static void end(Pass self, RenderServer *server,
                  gfx::CommandEncoderImpl const *encoder);

  static constexpr PassInterface const interface{.init          = init,
                                                 .deinit        = deinit,
                                                 .acquire_scene = acquire_scene,
                                                 .release_scene = release_scene,
                                                 .acquire_view  = acquire_view,
                                                 .release_view  = release_view,
                                                 .release_object =
                                                     release_object,
                                                 .sort   = sort,
                                                 .begin  = begin,
                                                 .encode = encode,
                                                 .end    = end};
};

}        // namespace ash
