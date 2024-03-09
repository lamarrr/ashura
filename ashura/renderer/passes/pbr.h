#pragma once
#include "ashura/renderer/camera.h"
#include "ashura/renderer/light.h"
#include "ashura/renderer/render_graph.h"
#include "ashura/renderer/shader.h"

namespace ash
{

struct PBRTexture
{
  gfx::ImageView view = nullptr;
  Vec2           uv0  = {};
  Vec2           uv1  = {};
};

BEGIN_SHADER_PARAMETER(PBRShaderParameter)
SHADER_SAMPLER(sampler, 1)
SHADER_SAMPLED_IMAGE(base_color, 1)
SHADER_SAMPLED_IMAGE(metallic, 1)
SHADER_SAMPLED_IMAGE(roughness, 1)
SHADER_SAMPLED_IMAGE(normal, 1)
SHADER_SAMPLED_IMAGE(occlusion, 1)
SHADER_SAMPLED_IMAGE(emissive, 1)
END_SHADER_PARAMETER(PBRShaderParameter)

struct PBRShaderUniform
{
  Vec4 base_color_factor  = {1, 1, 1, 1};
  f32  metallic_factor    = 1;
  f32  roughness_factor   = 1;
  f32  normal_scale       = 1;
  f32  occlusion_strength = 1;
  Vec3 emissive_factor    = {1, 1, 1};
  f32  emissive_strength  = 1;
};

/// SEE: https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos
/// SEE:
/// https://github.com/KhronosGroup/glTF-Sample-Viewer/blob/main/source/Renderer/shaders/textures.glsl
struct PBRMaterial
{
  gfx::Sampler sampler            = nullptr;
  PBRTexture   base_color_texture = {};
  PBRTexture   metallic_texture   = {};
  PBRTexture   roughness_texture  = {};
  PBRTexture   normal_texture     = {};
  PBRTexture   occlusion_texture  = {};
  PBRTexture   emissive_texture   = {};
  Vec4         base_color_factor  = {1, 1, 1, 1};
  f32          metallic_factor    = 1;
  f32          roughness_factor   = 1;
  f32          normal_scale       = 1;
  f32          occlusion_strength = 1;
  Vec3         emissive_factor    = {1, 1, 1};
  f32          emissive_strength  = 1;

  // generate descriptor set layout from type declaration
  // generate descriptor set setter from void* of this struct
  // generate binder?
  // uniform buffer size
  // count material types
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
  rdg::RenderTarget            render_target;
  Camera                       camera;
  AmbientLight                 ambient_light         = {};
  Span<DirectionalLight const> directional_lights    = {};
  Span<PointLight const>       point_lights          = {};
  Span<SpotLight const>        spot_lights           = {};
  Span<AreaLight const>        area_lights           = {};
  Span<PBRObject const>        objects               = {};
  gfx::DescriptorSetLayout     descriptor_set_layout = nullptr;
  gfx::DescriptorHeapImpl      descriptor_heap       = {};
};

struct PBRPass
{
  // allocate buffer for camera and transforms
  static void add_pass(rdg::Graph *graph, PBRParams const *params);
};

}        // namespace ash
