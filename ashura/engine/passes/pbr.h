#pragma once
#include "ashura/engine/renderer.h"

namespace ash
{

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
  gfx::Buffer    vertex_buffer        = 0;
  gfx::Buffer    index_buffer         = 0;
  u64            vertex_buffer_offset = 0;
  u32            first_index          = 0;
  u32            num_indices          = 0;
  gfx::IndexType index_type           = gfx::IndexType::Uint16;
};

struct PBRObject
{
  PBRMaterial material              = {};
  PBRMesh     mesh                  = {};
  u32         descriptor_heap_group = 0;
};

struct PBRParams
{
  AmbientLight                 ambient_light         = {};
  Span<DirectionalLight const> directional_lights    = {};
  Span<PointLight const>       point_lights          = {};
  Span<SpotLight const>        spot_lights           = {};
  Span<AreaLight const>        area_lights           = {};
  Span<PBRObject const>        objects               = {};
  gfx::DescriptorSetLayout     descriptor_set_layout = nullptr;
  gfx::DescriptorHeapImpl      descriptor_heap       = {};
  gfx::Sampler                 sampler               = nullptr;
};

struct PBRPass
{
  static void init(Pass self, RenderServer *server, uid32 id);
  static void deinit(Pass self, RenderServer *server);

  static constexpr PassInterface const interface{.init   = init,
                                                 .deinit = deinit};

  void execute(rdg::RenderGraph *graph, PBRParams const *params);
};

}        // namespace ash
