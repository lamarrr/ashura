#pragma once
#include "ashura/renderer/camera.h"
#include "ashura/renderer/light.h"
#include "ashura/renderer/render_context.h"

namespace ash
{

/// SEE: https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos
/// SEE:
/// https://github.com/KhronosGroup/glTF-Sample-Viewer/blob/main/source/Renderer/shaders/textures.glsl

struct PBRParam
{
  ViewTransform transform               = {};
  Vec4          view_position           = {0, 0, 0, 0};
  Vec4          albedo                  = {1, 1, 1, 1};
  f32           metallic                = 1;
  f32           roughness               = 1;
  f32           normal                  = 1;
  f32           occlusion               = 1;
  Vec4          emissive                = {1, 1, 1, 1};
  f32           ior                     = 1.5;
  f32           clearcoat               = 1;
  f32           clearcoat_roughness     = 1;
  f32           clearcoat_normal        = 1;
  u32           albedo_map              = 0;
  u32           metallic_map            = 0;
  u32           roughness_map           = 0;
  u32           normal_map              = 0;
  u32           occlusion_map           = 0;
  u32           emissive_map            = 0;
  u32           clearcoat_map           = 0;
  u32           clearcoat_roughness_map = 0;
  u32           clearcoat_normal_map    = 0;
  u32           first_light             = 0;
  u32           num_lights              = 0;
};

struct PBRVertex
{
  f32 x = 0;
  f32 y = 0;
  f32 z = 0;
  f32 u = 0;
  f32 v = 0;
};

struct PBRPassParams
{
  RenderTarget       render_target       = {};
  gfx::DescriptorSet vertex_buffer_ssbos = nullptr;
  gfx::DescriptorSet index_buffer_ssbos  = nullptr;
  gfx::DescriptorSet params_ssbo         = nullptr;
  gfx::DescriptorSet lights_ssbo         = nullptr;
  gfx::DescriptorSet textures            = nullptr;
  bool               wireframe           = false;
  u32                first_instance      = 0;
  u32                num_instances       = 0;
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
