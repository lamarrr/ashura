/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/gpu_system.h"
#include "ashura/gpu/gpu.h"
#include "ashura/std/types.h"

namespace ash
{

struct ColorTextureResult
{
  ColorTexture color{};
  RectU        rect{};
};

/// @brief Passes are re-usable and stateless compute and graphics pipeline
/// components. They set up static resources: pipelines, shaders, and render
/// data needed for executing rendering operations. Passes dispatch
/// compute/graphics shaders using their specified arguments. They are mostly
/// used by renderers.
struct Pass
{
  virtual Str  label()   = 0;
  virtual void acquire() = 0;
  virtual void release() = 0;
  virtual ~Pass()        = default;
};

struct BloomPassParams
{
  Vec2U          offset = {};
  Vec2U          extent = {};
  gpu::Image     image  = nullptr;
  gpu::ImageView view   = nullptr;
};

struct BloomPass : Pass
{
  BloomPass() = default;

  virtual ~BloomPass() override = default;

  virtual Str label() override
  {
    return "Bloom"_str;
  }

  virtual void acquire() override;

  virtual void release() override;

  void encode(gpu::CommandEncoder & encoder, BloomPassParams const & params);
};

struct BlurParam
{
  alignas(16) Vec2 uv[2] = {};
  alignas(8) Vec2 radius = {};
  SamplerId sampler      = SamplerId::Linear;
  TextureId texture      = TextureId::White;
};

struct BlurPassParams
{
  Framebuffer framebuffer = {};
  RectU       area        = {};
  Vec2U       radius      = {1, 1};
};

struct BlurPass : Pass
{
  static constexpr u32 DOWNSCALE_FACTOR = 8;

  static constexpr u32 MAX_PASSES = 8;

  // 1 pass for every BLUR_PERIOD radius
  static constexpr f32 BLUR_PERIOD = 1;

  gpu::GraphicsPipeline downsample_pipeline = nullptr;
  gpu::GraphicsPipeline upsample_pipeline   = nullptr;

  BlurPass() = default;

  virtual ~BlurPass() override = default;

  virtual Str label() override
  {
    return "Blur"_str;
  }

  virtual void acquire() override;

  virtual void release() override;

  Option<ColorTextureResult> encode(gpu::CommandEncoder &  encoder,
                                    BlurPassParams const & params);
};

struct NgonParam
{
  alignas(16) Mat4 transform = {};
  alignas(16) Vec4 tint[4]   = {};
  alignas(8) Vec2 uv[2]      = {};
  f32       tiling           = 1;
  SamplerId sampler          = SamplerId::Linear;
  TextureId albedo           = TextureId::White;
  u32       first_index      = 0;
  u32       first_vertex     = 0;
};

struct NgonPassParams
{
  Framebuffer        framebuffer          = {};
  RectU              scissor              = {};
  gpu::Viewport      viewport             = {};
  Mat4               world_to_view        = {};
  gpu::DescriptorSet vertices_ssbo        = nullptr;
  u32                vertices_ssbo_offset = 0;
  gpu::DescriptorSet indices_ssbo         = nullptr;
  u32                indices_ssbo_offset  = 0;
  gpu::DescriptorSet params_ssbo          = nullptr;
  u32                params_ssbo_offset   = 0;
  gpu::DescriptorSet textures             = nullptr;
  u32                first_instance       = 0;
  Span<u32 const>    index_counts         = {};
};

struct NgonPass : Pass
{
  gpu::GraphicsPipeline pipeline = nullptr;

  NgonPass() = default;

  virtual ~NgonPass() override = default;

  virtual Str label() override
  {
    return "Ngon"_str;
  }

  virtual void acquire() override;

  virtual void release() override;

  void encode(gpu::CommandEncoder & encoder, NgonPassParams const & params);
};

/// @see https://github.com/KhronosGroup/glTF/tree/acfcbe65e40c53d6d3aa55a7299982bf2c01c75d/extensions/2.0/Khronos
/// @see
/// https://github.com/KhronosGroup/glTF-Sample-Renderer/blob/63b7c128266cfd86bbd3f25caf8b3db3fe854015/source/Renderer/shaders/textures.glsl#L1
struct PBRParam
{
  alignas(16) Mat4 transform        = {};
  alignas(16) Vec4 eye_position     = {0, 0, 0, 0};
  alignas(16) Vec4 albedo           = {1, 1, 1, 1};
  f32 metallic                      = 0;
  f32 roughness                     = 0;
  f32 normal                        = 0;
  f32 occlusion                     = 0;
  alignas(16) Vec4 emission         = {0, 0, 0, 0};
  f32       ior                     = 1.5F;
  f32       clearcoat               = 0;
  f32       clearcoat_roughness     = 0;
  f32       clearcoat_normal        = 0;
  SamplerId sampler                 = SamplerId::Linear;
  TextureId albedo_map              = TextureId::White;
  TextureId metallic_map            = TextureId::White;
  TextureId roughness_map           = TextureId::White;
  TextureId normal_map              = TextureId::White;
  TextureId occlusion_map           = TextureId::White;
  TextureId emission_map            = TextureId::White;
  TextureId clearcoat_map           = TextureId::White;
  TextureId clearcoat_roughness_map = TextureId::White;
  TextureId clearcoat_normal_map    = TextureId::White;
  u32       first_light             = 0;
  u32       first_vertex            = 0;
};

struct PBRVertex
{
  alignas(16) Vec4 pos = {};
  alignas(8) Vec2 uv   = {};
};

struct PBRPassParams
{
  Framebuffer        framebuffer          = {};
  RectU              scissor              = {};
  gpu::Viewport      viewport             = {};
  Mat4               world_to_view        = {};
  bool               wireframe            = false;
  gpu::DescriptorSet vertices_ssbo        = nullptr;
  u32                vertices_ssbo_offset = 0;
  gpu::DescriptorSet indices_ssbo         = nullptr;
  u32                indices_ssbo_offset  = 0;
  gpu::DescriptorSet params_ssbo          = nullptr;
  u32                params_ssbo_offset   = 0;
  gpu::DescriptorSet lights_ssbo          = nullptr;
  u32                lights_ssbo_offset   = 0;
  gpu::DescriptorSet textures             = nullptr;
  u32                instance             = 0;
  u32                num_indices          = 0;
};

struct PBRPass : Pass
{
  gpu::GraphicsPipeline pipeline           = nullptr;
  gpu::GraphicsPipeline wireframe_pipeline = nullptr;

  PBRPass() = default;

  virtual ~PBRPass() override = default;

  virtual Str label() override
  {
    return "PBR"_str;
  }

  virtual void acquire() override;

  virtual void release() override;

  void encode(gpu::CommandEncoder & encoder, PBRPassParams const & params);
};

struct RRectParam
{
  alignas(16) Mat4 transform = {};
  alignas(16) Vec4 tint[4]   = {};
  alignas(16) Vec4 radii     = {};
  alignas(16) Vec2 uv[2]     = {};
  f32       tiling           = 1;
  f32       aspect_ratio     = 1;
  f32       stroke           = 0;
  f32       thickness        = 0;
  f32       edge_smoothness  = 0;
  SamplerId sampler          = SamplerId::Linear;
  TextureId albedo           = TextureId::White;
};

struct SquircleParam
{
  alignas(16) Mat4 transform = {};
  alignas(16) Vec4 tint[4]   = {};
  alignas(16) Vec2 uv[2]     = {};
  f32       radius           = 0;
  f32       degree           = 5;
  f32       tiling           = 1;
  f32       aspect_ratio     = 1;
  f32       stroke           = 0;
  f32       thickness        = 0;
  f32       edge_smoothness  = 0;
  SamplerId sampler          = SamplerId::Linear;
  TextureId albedo           = TextureId::White;
};

struct RRectPassParams
{
  Framebuffer        framebuffer        = {};
  RectU              scissor            = {};
  gpu::Viewport      viewport           = {};
  Mat4               world_to_view      = {};
  gpu::DescriptorSet params_ssbo        = nullptr;
  u32                params_ssbo_offset = 0;
  gpu::DescriptorSet textures           = nullptr;
  u32                first_instance     = 0;
  u32                num_instances      = 0;
};

struct RRectPass : Pass
{
  gpu::GraphicsPipeline pipeline = nullptr;

  virtual Str label() override
  {
    return "RRect"_str;
  }

  RRectPass() = default;

  virtual ~RRectPass() override = default;

  virtual void acquire() override;

  virtual void release() override;

  void encode(gpu::CommandEncoder & encoder, RRectPassParams const & params);
};

struct SquirclePassParams
{
  Framebuffer        framebuffer        = {};
  RectU              scissor            = {};
  gpu::Viewport      viewport           = {};
  Mat4               world_to_view      = {};
  gpu::DescriptorSet params_ssbo        = nullptr;
  u32                params_ssbo_offset = 0;
  gpu::DescriptorSet textures           = nullptr;
  u32                first_instance     = 0;
  u32                num_instances      = 0;
};

struct SquirclePass : Pass
{
  gpu::GraphicsPipeline pipeline = nullptr;

  virtual Str label() override
  {
    return "Squircle"_str;
  }

  SquirclePass() = default;

  virtual ~SquirclePass() override = default;

  virtual void acquire() override;

  virtual void release() override;

  void encode(gpu::CommandEncoder & encoder, SquirclePassParams const & params);
};

}    // namespace ash
