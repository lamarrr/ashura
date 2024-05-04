#pragma once
#include "ashura/renderer/camera.h"
#include "ashura/renderer/light.h"
#include "ashura/renderer/render_context.h"
#include "ashura/renderer/shader.h"

namespace ash
{

/// SEE: https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos
/// SEE:
/// https://github.com/KhronosGroup/glTF-Sample-Viewer/blob/main/source/Renderer/shaders/textures.glsl
BEGIN_SHADER_PARAMETER(PBRShaderParameter)
COMBINED_IMAGE_SAMPLER(albedo, 1)
COMBINED_IMAGE_SAMPLER(metallic, 1)
COMBINED_IMAGE_SAMPLER(roughness, 1)
COMBINED_IMAGE_SAMPLER(normal, 1)
COMBINED_IMAGE_SAMPLER(occlusion, 1)
COMBINED_IMAGE_SAMPLER(emissive, 1)
END_SHADER_PARAMETER(PBRShaderParameter)

constexpr u8 MAX_PBR_AMBIENT_LIGHTS     = 1;
constexpr u8 MAX_PBR_DIRECTIONAL_LIGHTS = 2;
constexpr u8 MAX_PBR_POINT_LIGHTS       = 2;
constexpr u8 MAX_PBR_SPOT_LIGHTS        = 2;

struct PBRUniform
{
  ViewTransform transform          = {};
  Vec4          base_color_factor  = {1, 1, 1, 1};
  f32           metallic_factor    = 1;
  f32           roughness_factor   = 1;
  f32           normal_scale       = 1;
  f32           occlusion_strength = 1;
  Vec4          emissive_factor    = {1, 1, 1, 1};
  f32           emissive_strength  = 1;
};

struct PBRLightUniform
{
  AmbientLight     ambient_light                                  = {};
  DirectionalLight directional_lights[MAX_PBR_DIRECTIONAL_LIGHTS] = {};
  PointLight       point_lights[MAX_PBR_POINT_LIGHTS]             = {};
  SpotLight        spot_lights[MAX_PBR_SPOT_LIGHTS]               = {};
};

struct PBRVertex
{
  f32 x = 0, y = 0, z = 0;
  f32 u = 0, v = 0;
};

struct PBRObject
{
  Mesh                    mesh          = {};
  gfx::DescriptorSet      descriptor    = {};
  Uniform                 uniform       = {};
  Uniform                 light_uniform = {};
  gfx::DrawIndexedCommand command       = {};
  bool                    wireframe     = false;
};

struct PBRPassParams
{
  RenderTarget          render_target = {};
  Span<PBRObject const> objects       = {};
};

struct PBRPass
{
  gfx::RenderPass          render_pass           = nullptr;
  gfx::GraphicsPipeline    pipeline              = nullptr;
  gfx::GraphicsPipeline    wireframe_pipeline    = nullptr;
  gfx::DescriptorSetLayout descriptor_set_layout = nullptr;

  void init(RenderContext &ctx);
  void uninit(RenderContext &ctx);
  void add_pass(RenderContext &ctx, PBRPassParams const &params);
};

}        // namespace ash
