#pragma once
#include "ashura/engine/light.h"
#include "ashura/engine/render_context.h"

namespace ash
{

/// @see https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos
/// @see
/// https://github.com/KhronosGroup/glTF-Sample-Viewer/blob/main/source/Renderer/shaders/textures.glsl
struct PBRParam
{
  Mat4Affine model                   = {};
  Mat4Affine view                    = {};
  Mat4       projection              = {};
  Vec4       eye_position            = {0, 0, 0, 0};
  Vec4       albedo                  = {1, 1, 1, 1};
  f32        metallic                = 0;
  f32        roughness               = 0;
  f32        normal                  = 0;
  f32        occlusion               = 0;
  Vec4       emissive                = {0, 0, 0, 0};
  f32        ior                     = 1.5F;
  f32        clearcoat               = 0;
  f32        clearcoat_roughness     = 0;
  f32        clearcoat_normal        = 0;
  u32        albedo_map              = 0;
  u32        metallic_map            = 0;
  u32        roughness_map           = 0;
  u32        normal_map              = 0;
  u32        occlusion_map           = 0;
  u32        emissive_map            = 0;
  u32        clearcoat_map           = 0;
  u32        clearcoat_roughness_map = 0;
  u32        clearcoat_normal_map    = 0;
  u32        first_vertex            = 0;
  u32        first_light             = 0;
};

struct PBRVertex
{
  Vec4 pos = {};
  Vec2 uv  = {};
};

struct PBRPassParams
{
  gfx::RenderingInfo rendering_info = {};
  gfx::Rect          scissor        = {};
  gfx::Viewport      viewport       = {};
  bool               wireframe      = false;
  gfx::DescriptorSet vertices_ssbo  = nullptr;
  gfx::DescriptorSet indices_ssbo   = nullptr;
  gfx::DescriptorSet params_ssbo    = nullptr;
  gfx::DescriptorSet lights_ssbo    = nullptr;
  gfx::SamplerDesc   sampler        = {};
  gfx::DescriptorSet textures       = nullptr;
  u32                instance       = 0;
  u32                num_indices    = 0;
};

struct PBRPass
{
  gfx::GraphicsPipeline pipeline           = nullptr;
  gfx::GraphicsPipeline wireframe_pipeline = nullptr;

  void init(RenderContext &ctx);
  void uninit(RenderContext &ctx);
  void add_pass(RenderContext &ctx, PBRPassParams const &params);
};

}        // namespace ash
