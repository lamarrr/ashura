#pragma once
#include "ashura/gfx/gfx.h"
#include "ashura/std/error.h"
#include "ashura/std/hash_map.h"
#include "ashura/std/option.h"
#include "ashura/std/result.h"
#include "ashura/std/types.h"
#include "ashura/std/vec.h"

namespace ash
{

constexpr u32 TEXTURE_WHITE        = 0;
constexpr u32 TEXTURE_BLACK        = 1;
constexpr u32 TEXTURE_TRANSPARENT  = 2;
constexpr u32 TEXTURE_RED          = 3;
constexpr u32 TEXTURE_GREEN        = 4;
constexpr u32 TEXTURE_BLUE         = 5;
constexpr u32 NUM_DEFAULT_TEXTURES = 6;

constexpr u32 SAMPLER_LINEAR       = 0;
constexpr u32 SAMPLER_NEAREST      = 1;
constexpr u32 NUM_DEFAULT_SAMPLERS = 2;

struct FramebufferAttachment
{
  gfx::ImageDesc     desc      = {};
  gfx::ImageViewDesc view_desc = {};
  gfx::Image         image     = nullptr;
  gfx::ImageView     view      = nullptr;
};

/// created with sampled, storage, color attachment, and transfer flags
struct Framebuffer
{
  FramebufferAttachment color         = {};
  FramebufferAttachment depth_stencil = {};
  gfx::DescriptorSet    color_texture = nullptr;
  gfx::Extent           extent        = {};
};

struct SamplerHasher
{
  constexpr Hash operator()(gfx::SamplerDesc const &desc) const
  {
    return hash_combine_n(
        (Hash) desc.mag_filter, (Hash) desc.min_filter,
        (Hash) desc.mip_map_mode, (Hash) desc.address_mode_u,
        (Hash) desc.address_mode_v, (Hash) desc.address_mode_w,
        (Hash) desc.mip_lod_bias, (Hash) desc.anisotropy_enable,
        (Hash) desc.max_anisotropy, (Hash) desc.compare_enable,
        (Hash) desc.compare_op, (Hash) desc.min_lod, (Hash) desc.max_lod,
        (Hash) desc.border_color, (Hash) desc.unnormalized_coordinates);
  }
};

struct SamplerEq
{
  constexpr Hash operator()(gfx::SamplerDesc const &a,
                            gfx::SamplerDesc const &b) const
  {
    return a.mag_filter == b.mag_filter && a.mip_map_mode == b.mip_map_mode &&
           a.address_mode_u == b.address_mode_u &&
           a.address_mode_v == b.address_mode_v &&
           a.address_mode_w == b.address_mode_w &&
           a.mip_lod_bias == b.mip_lod_bias &&
           a.anisotropy_enable == b.anisotropy_enable &&
           a.max_anisotropy == b.max_anisotropy &&
           a.compare_enable == b.compare_enable &&
           a.compare_op == b.compare_op && a.min_lod == b.min_lod &&
           a.max_lod == b.max_lod && a.border_color == b.border_color &&
           a.unnormalized_coordinates == b.unnormalized_coordinates;
  }
};

struct CachedSampler
{
  gfx::Sampler sampler = nullptr;
  u32          slot    = 0;
};

typedef HashMap<gfx::SamplerDesc, CachedSampler, SamplerHasher, SamplerEq, u32>
    SamplerCache;

/// @param color_format hdr if hdr supported and required.
///
/// scratch images resized when swapchain extents changes
///
struct RenderContext
{
  static constexpr gfx::FormatFeatures COLOR_FEATURES =
      gfx::FormatFeatures::ColorAttachment |
      gfx::FormatFeatures::ColorAttachmentBlend |
      gfx::FormatFeatures::StorageImage | gfx::FormatFeatures::SampledImage;
  static constexpr gfx::FormatFeatures DEPTH_STENCIL_FEATURES =
      gfx::FormatFeatures::DepthStencilAttachment |
      gfx::FormatFeatures::SampledImage;
  static constexpr gfx::BufferUsage SSBO_USAGE =
      gfx::BufferUsage::UniformBuffer | gfx::BufferUsage::StorageBuffer |
      gfx::BufferUsage::UniformTexelBuffer |
      gfx::BufferUsage::StorageTexelBuffer | gfx::BufferUsage::IndirectBuffer |
      gfx::BufferUsage::TransferSrc | gfx::BufferUsage::TransferDst;
  static constexpr u16 NUM_TEXTURE_SLOTS = 2048;
  static constexpr u16 NUM_SAMPLER_SLOTS = 128;

  static_assert(NUM_TEXTURE_SLOTS % 64 == 0);
  static_assert(NUM_SAMPLER_SLOTS % 64 == 0);

  u64                      texture_slots[NUM_TEXTURE_SLOTS / 64] = {};
  u64                      sampler_slots[NUM_SAMPLER_SLOTS / 64] = {};
  gfx::DeviceImpl          device                                = {};
  gfx::PipelineCache       pipeline_cache                        = nullptr;
  u32                      buffering                             = 0;
  StrHashMap<gfx::Shader>  shader_map                            = {};
  gfx::Format              color_format         = gfx::Format::Undefined;
  gfx::Format              depth_stencil_format = gfx::Format::Undefined;
  gfx::DescriptorSetLayout ubo_layout           = nullptr;
  gfx::DescriptorSetLayout ssbo_layout          = nullptr;
  gfx::DescriptorSetLayout textures_layout      = nullptr;
  gfx::DescriptorSetLayout samplers_layout      = nullptr;
  gfx::DescriptorSet       texture_views        = nullptr;
  gfx::DescriptorSet       samplers             = nullptr;
  Vec<gfx::Object>         released_objects[gfx::MAX_FRAME_BUFFERING] = {};
  SamplerCache             sampler_cache                              = {};
  Framebuffer              screen_fb                                  = {};
  Framebuffer              scratch_fbs[2]                             = {};
  gfx::Image               default_image                              = nullptr;
  gfx::ImageView           default_image_views[NUM_DEFAULT_TEXTURES]  = {};

  void init(gfx::DeviceImpl device, bool use_hdr, u32 buffering,
            gfx::Extent initial_extent, StrHashMap<gfx::Shader> shader_map);
  void uninit();

  void recreate_framebuffers(gfx::Extent new_extent);

  gfx::CommandEncoderImpl encoder();
  u32                     ring_index();
  gfx::FrameId            frame_id();
  gfx::FrameId            tail_frame_id();

  Option<gfx::Shader> get_shader(Span<char const> name);
  CachedSampler       create_sampler(gfx::SamplerDesc const &desc);

  u32  alloc_texture_slot();
  void release_texture_slot(u32 slot);
  u32  alloc_sampler_slot();
  void release_sampler_slot(u32 slot);

  void release(gfx::Image image);
  void release(gfx::ImageView view);
  void release(gfx::Buffer view);
  void release(gfx::BufferView view);
  void release(gfx::DescriptorSetLayout layout);
  void release(gfx::DescriptorSet set);
  void release(gfx::Sampler sampler);
  void release(FramebufferAttachment fb)
  {
    release(fb.image);
    release(fb.view);
  }
  void release(Framebuffer fb)
  {
    release(fb.color);
    release(fb.depth_stencil);
    release(fb.color_texture);
  }

  void idle_reclaim();

  void begin_frame(gfx::Swapchain swapchain);
  void end_frame(gfx::Swapchain swapchain);
};

}        // namespace ash
