
/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/render_context.h"
#include "ashura/gpu/gpu.h"
#include "ashura/std/types.h"

namespace ash
{

struct BloomPassParams
{
  Vec2U          offset = {};
  Vec2U          extent = {};
  gpu::Image     image  = nullptr;
  gpu::ImageView view   = nullptr;
};

struct BloomPass
{
  void init(RenderContext &ctx);
  void uninit(RenderContext &ctx);
  void add_pass(RenderContext &ctx, BloomPassParams const &params);
};

struct BlurParam
{
  Vec2 uv[2]   = {};
  Vec2 radius  = {};
  u32  sampler = 0;
  u32  texture = 0;
};

struct BlurPassParams
{
  gpu::ImageView     image_view   = nullptr;
  Vec2U              extent       = {};
  gpu::DescriptorSet texture_view = nullptr;
  u32                texture      = 0;
  u32                passes       = 1;
  gpu::Rect          area         = {};
};

struct BlurPass
{
  gpu::GraphicsPipeline downsample_pipeline = nullptr;
  gpu::GraphicsPipeline upsample_pipeline   = nullptr;

  void init(RenderContext &ctx);
  void uninit(RenderContext &ctx);
  void add_pass(RenderContext &ctx, BlurPassParams const &params);
};

/// @param transform needs to transform from [-1, +1] to clip space
struct NgonParam
{
  Mat4 transform    = {};
  Vec4 tint[4]      = {};
  Vec2 uv[2]        = {};
  f32  tiling       = 1;
  u32  sampler      = 0;
  u32  albedo       = 0;
  u32  first_index  = 0;
  u32  first_vertex = 0;
};

struct NgonPassParams
{
  gpu::RenderingInfo rendering_info = {};
  gpu::Rect          scissor        = {};
  gpu::Viewport      viewport       = {};
  gpu::DescriptorSet vertices_ssbo  = nullptr;
  gpu::DescriptorSet indices_ssbo   = nullptr;
  gpu::DescriptorSet params_ssbo    = nullptr;
  gpu::DescriptorSet textures       = nullptr;
  Span<u32 const>    index_counts   = {};
};

struct NgonPass
{
  gpu::GraphicsPipeline pipeline = nullptr;

  void init(RenderContext &ctx);
  void uninit(RenderContext &ctx);
  void add_pass(RenderContext &ctx, NgonPassParams const &params);
};

/// @see https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos
/// @see
/// https://github.com/KhronosGroup/glTF-Sample-Viewer/blob/main/source/Renderer/shaders/textures.glsl
struct PBRParam
{
  Mat4 model                   = {};
  Mat4 view                    = {};
  Mat4 projection              = {};
  Vec4 eye_position            = {0, 0, 0, 0};
  Vec4 albedo                  = {1, 1, 1, 1};
  f32  metallic                = 0;
  f32  roughness               = 0;
  f32  normal                  = 0;
  f32  occlusion               = 0;
  Vec4 emissive                = {0, 0, 0, 0};
  f32  ior                     = 1.5F;
  f32  clearcoat               = 0;
  f32  clearcoat_roughness     = 0;
  f32  clearcoat_normal        = 0;
  u32  sampler                 = 0;
  u32  albedo_map              = 0;
  u32  metallic_map            = 0;
  u32  roughness_map           = 0;
  u32  normal_map              = 0;
  u32  occlusion_map           = 0;
  u32  emissive_map            = 0;
  u32  clearcoat_map           = 0;
  u32  clearcoat_roughness_map = 0;
  u32  clearcoat_normal_map    = 0;
  u32  first_vertex            = 0;
  u32  first_light             = 0;
};

struct PBRVertex
{
  Vec4 pos = {};
  Vec2 uv  = {};
};

struct PBRPassParams
{
  gpu::RenderingInfo rendering_info = {};
  gpu::Rect          scissor        = {};
  gpu::Viewport      viewport       = {};
  bool               wireframe      = false;
  gpu::DescriptorSet vertices_ssbo  = nullptr;
  gpu::DescriptorSet indices_ssbo   = nullptr;
  gpu::DescriptorSet params_ssbo    = nullptr;
  gpu::DescriptorSet lights_ssbo    = nullptr;
  gpu::DescriptorSet textures       = nullptr;
  u32                instance       = 0;
  u32                num_indices    = 0;
};

struct PBRPass
{
  gpu::GraphicsPipeline pipeline           = nullptr;
  gpu::GraphicsPipeline wireframe_pipeline = nullptr;

  void init(RenderContext &ctx);
  void uninit(RenderContext &ctx);
  void add_pass(RenderContext &ctx, PBRPassParams const &params);
};

enum class FillRule : u8
{
  EvenOdd = 0,
  NonZero = 1
};

/// @param transform needs to transform from [-1, +1] to clip space
struct PolyParam
{
  Mat4 transform    = {};
  Vec4 tint[4]      = {};
  Vec2 uv[2]        = {};
  f32  tiling       = 1;
  u32  sampler      = 0;
  u32  albedo       = 0;
  u32  first_index  = 0;
  u32  first_vertex = 0;
};

struct PolyPassParams
{
  gpu::RenderingInfo   rendering_info = {};
  gpu::Rect            scissor        = {};
  gpu::Viewport        viewport       = {};
  gpu::DescriptorSet   vertices_ssbo  = nullptr;
  gpu::DescriptorSet   indices_ssbo   = nullptr;
  gpu::DescriptorSet   params_ssbo    = nullptr;
  gpu::DescriptorSet   textures       = nullptr;
  Span<u32 const>      index_counts   = {};
  Span<FillRule const> fill_rules     = {};
};

struct PolyPass
{
  gpu::GraphicsPipeline pipeline = nullptr;

  void init(RenderContext &ctx);
  void uninit(RenderContext &ctx);
  void add_pass(RenderContext &ctx, PolyPassParams const &params);
};

/// @param transform needs to transform from [-1, +1] to clip space
struct RRectParam
{
  Mat4 transform       = {};
  Vec4 tint[4]         = {};
  Vec4 radii           = {};
  Vec2 uv[2]           = {};
  f32  tiling          = 1;
  f32  aspect_ratio    = 1;
  f32  stroke          = 0;
  f32  thickness       = 0;
  f32  edge_smoothness = 0.0015F;
  u32  sampler         = 0;
  u32  albedo          = 0;
};

struct RRectPassParams
{
  gpu::RenderingInfo rendering_info = {};
  gpu::Rect          scissor        = {};
  gpu::Viewport      viewport       = {};
  gpu::DescriptorSet params_ssbo    = nullptr;
  gpu::DescriptorSet textures       = nullptr;
  u32                first_instance = 0;
  u32                num_instances  = 0;
};

struct RRectPass
{
  gpu::GraphicsPipeline pipeline = nullptr;

  void init(RenderContext &ctx);
  void uninit(RenderContext &ctx);
  void add_pass(RenderContext &ctx, RRectPassParams const &params);
};

}        // namespace ash