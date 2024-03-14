#pragma once
#include "ashura/renderer/camera.h"
#include "ashura/renderer/light.h"
#include "ashura/renderer/renderer.h"
#include "ashura/renderer/shader.h"

namespace ash
{

/// SEE: https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos
/// SEE:
/// https://github.com/KhronosGroup/glTF-Sample-Viewer/blob/main/source/Renderer/shaders/textures.glsl
BEGIN_SHADER_PARAMETER(PBRShaderParameter)
SHADER_SAMPLER(sampler, 1)
SHADER_SAMPLED_IMAGE(base_color, 1)
SHADER_SAMPLED_IMAGE(metallic, 1)
SHADER_SAMPLED_IMAGE(roughness, 1)
SHADER_SAMPLED_IMAGE(normal, 1)
SHADER_SAMPLED_IMAGE(occlusion, 1)
SHADER_SAMPLED_IMAGE(emissive, 1)
END_SHADER_PARAMETER(PBRShaderParameter)

struct PBRLightsUniform
{
  AmbientLight     ambient_light         = {};
  DirectionalLight directional_lights[2] = {};
  PointLight       point_lights[4]       = {};
  SpotLight        spot_lights[4]        = {};
};

struct PBRObjectUniform
{
  MVPTransform transform          = {};
  Vec4         base_color_factor  = {1, 1, 1, 1};
  f32          metallic_factor    = 1;
  f32          roughness_factor   = 1;
  f32          normal_scale       = 1;
  f32          occlusion_strength = 1;
  Vec4         emissive_factor    = {1, 1, 1, 1};
  f32          emissive_strength  = 1;
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
  u64            index_buffer_offset  = 0;
  i32            vertex_offset        = 0;
  u32            first_index          = 0;
  u32            num_indices          = 0;
  gfx::IndexType index_type           = gfx::IndexType::Uint16;
};

struct PBRObject
{
  PBRMesh            mesh       = {};
  gfx::DescriptorSet descriptor = {};
  PBRObjectUniform   uniform    = {};
  bool               wireframe  = false;
};

struct PBRParams
{
  RenderTarget          render_target = {};
  PBRLightsUniform      lights        = {};
  Span<PBRObject const> objects       = {};
};

struct PBRPass
{
  gfx::RenderPass          render_pass           = nullptr;
  gfx::GraphicsPipeline    pipeline              = nullptr;
  gfx::GraphicsPipeline    wireframe_pipeline    = nullptr;
  gfx::DescriptorSetLayout descriptor_set_layout = nullptr;

  void init(Renderer &renderer);
  void uninit(Renderer &renderer);
  void add_pass(Renderer &renderer, PBRParams const &params);
};

}        // namespace ash
