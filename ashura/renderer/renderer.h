#pragma once
#include "ashura/renderer/passes/bloom.h"
#include "ashura/renderer/passes/blur.h"
#include "ashura/renderer/passes/custom.h"
#include "ashura/renderer/passes/fxaa.h"
#include "ashura/renderer/passes/msaa.h"
#include "ashura/renderer/passes/pbr.h"
#include "ashura/renderer/passes/rrect.h"
#include "ashura/renderer/render_context.h"

namespace ash
{

// sky render pass
// render 3d scene pass + custom shaders (pipeline + fragment + vertex shader)
// perform bloom, blur, msaa on 3d scene
// render UI pass + custom shaders, blur ???
// copy and composite 3d and 2d scenes
struct RenderPasses
{
  BloomPass        bloom;
  BlurPass         blur;
  FXAAPass         fxaa;
  MSAAPass         msaa;
  PBRPass          pbr;
  CustomShaderPass custom;
  RRectPass        rrect;
};

struct RendererConfig
{
  bool        use_hdr              = false;
  u8          max_frames_in_flight = 2;
  gfx::Extent initial_extent       = {1920, 1080};
};

struct Renderer
{
  RenderPasses  passes;
  RenderContext ctx;

  void init(gfx::DeviceImpl device, gfx::PipelineCache pipeline_cache,
            StrHashMap<gfx::Shader> shader_map, RendererConfig const &config)
  {
    CHECK(config.max_frames_in_flight <= 4 && config.max_frames_in_flight > 0);
    ctx.device         = device;
    ctx.shader_map     = shader_map;
    ctx.pipeline_cache = pipeline_cache;

    gfx::Format                   color_format         = gfx::Format::Undefined;
    gfx::Format                   depth_stencil_format = gfx::Format::Undefined;
    constexpr gfx::FormatFeatures COLOR_FEATURES =
        gfx::FormatFeatures::ColorAttachment |
        gfx::FormatFeatures::ColorAttachmentBlend;
    constexpr gfx::FormatFeatures DEPTH_STENCIL_FEATURES =
        gfx::FormatFeatures::DepthStencilAttachment;

    if (config.use_hdr)
    {
      gfx::FormatProperties properties =
          device
              ->get_format_properties(device.self,
                                      gfx::Format::R16G16B16A16_SFLOAT)
              .unwrap();
      if (has_bits(properties.optimal_tiling_features, COLOR_FEATURES))
      {
        color_format = gfx::Format::R16G16B16A16_SFLOAT;
      }
      else
      {
        default_logger->warn("HDR mode requested but Device does not support "
                             "HDR render target, trying UNORM color");
      }
    }

    if (color_format == gfx::Format::Undefined)
    {
      gfx::FormatProperties properties =
          device
              ->get_format_properties(device.self, gfx::Format::B8G8R8A8_UNORM)
              .unwrap();
      if (has_bits(properties.optimal_tiling_features, COLOR_FEATURES))
      {
        color_format = gfx::Format::B8G8R8A8_UNORM;
      }
    }

    if (color_format == gfx::Format::Undefined)
    {
      gfx::FormatProperties properties =
          device
              ->get_format_properties(device.self, gfx::Format::R8G8B8A8_UNORM)
              .unwrap();
      if (has_bits(properties.optimal_tiling_features, COLOR_FEATURES))
      {
        color_format = gfx::Format::R8G8B8A8_UNORM;
      }
    }

    {
      gfx::FormatProperties properties =
          device
              ->get_format_properties(device.self,
                                      gfx::Format::D16_UNORM_S8_UINT)
              .unwrap();
      if (has_bits(properties.optimal_tiling_features, DEPTH_STENCIL_FEATURES))
      {
        depth_stencil_format = gfx::Format::D16_UNORM_S8_UINT;
      }
    }

    if (depth_stencil_format == gfx::Format::Undefined)
    {
      gfx::FormatProperties properties =
          device
              ->get_format_properties(device.self,
                                      gfx::Format::D24_UNORM_S8_UINT)
              .unwrap();
      if (has_bits(properties.optimal_tiling_features, DEPTH_STENCIL_FEATURES))
      {
        depth_stencil_format = gfx::Format::D24_UNORM_S8_UINT;
      }
    }

    CHECK_EX("Device doesn't support any known color format",
             color_format != gfx::Format::Undefined);
    CHECK_EX("Device doesn't support any depth stencil format",
             depth_stencil_format != gfx::Format::Undefined);

    ctx.color_format         = color_format;
    ctx.depth_stencil_format = depth_stencil_format;

    CHECK(ctx.uniform_heaps.resize_defaulted(config.max_frames_in_flight));

    for (u8 i = 0; i < config.max_frames_in_flight; i++)
    {
      ctx.uniform_heaps[i].init(device);
    }

    constexpr Array uniform_bindings_desc =
        UniformShaderParameter::GET_BINDINGS_DESC();
    ctx.uniform_layout =
        device
            ->create_descriptor_set_layout(
                device.self,
                gfx::DescriptorSetLayoutDesc{
                    .label    = "Uniform Set Layout",
                    .bindings = to_span(uniform_bindings_desc)})
            .unwrap();

    AllocatorImpl allocators[4] = {default_allocator, default_allocator,
                                   default_allocator, default_allocator};

    ctx.frame_context = device
                            ->create_frame_context(
                                device.self, config.max_frames_in_flight,
                                Span{allocators, config.max_frames_in_flight})
                            .unwrap();
    ctx.frame_info = device->get_frame_info(device.self, ctx.frame_context);

    // TODO(lamarrrr): main color render targets???
    update_extent(config.initial_extent);
    init_passes();
  }

  void update_extent(Vec2U new_extent)
  {
    ctx.release(ctx.scatch.color_image);
    ctx.release(ctx.scatch.color_image_view);
    ctx.release(ctx.scatch.depth_stencil_image);
    ctx.release(ctx.scatch.depth_stencil_image_view);

    ctx.scatch.color_image_desc = gfx::ImageDesc{
        .label  = "Scratch Image",
        .type   = gfx::ImageType::Type2D,
        .format = ctx.color_format,
        .usage  = gfx::ImageUsage::ColorAttachment | gfx::ImageUsage::Sampled |
                 gfx::ImageUsage::Storage | gfx::ImageUsage::TransferDst |
                 gfx::ImageUsage::TransferSrc,
        .aspects      = gfx::ImageAspects::Color,
        .extent       = gfx::Extent3D{new_extent.x, new_extent.y, 1},
        .mip_levels   = 1,
        .array_layers = 1,
        .sample_count = gfx::SampleCount::Count1};
    ctx.scatch.color_image =
        ctx.device->create_image(ctx.device.self, ctx.scatch.color_image_desc)
            .unwrap();

    ctx.scatch.color_image_view_desc =
        gfx::ImageViewDesc{.label       = "Scratch Color Image View",
                           .image       = ctx.scatch.color_image,
                           .view_type   = gfx::ImageViewType::Type2D,
                           .view_format = ctx.scatch.color_image_desc.format,
                           .mapping     = {},
                           .aspects     = gfx::ImageAspects::Color,
                           .first_mip_level   = 0,
                           .num_mip_levels    = 1,
                           .first_array_layer = 0,
                           .num_array_layers  = 1};
    ctx.scatch.color_image_view =
        ctx.device
            ->create_image_view(ctx.device.self,
                                ctx.scatch.color_image_view_desc)
            .unwrap();

    ctx.scatch.depth_stencil_image_desc = gfx::ImageDesc{
        .label  = "Depth Stencil Image",
        .type   = gfx::ImageType::Type2D,
        .format = ctx.depth_stencil_format,
        .usage  = gfx::ImageUsage::DepthStencilAttachment |
                 gfx::ImageUsage::Sampled | gfx::ImageUsage::Storage |
                 gfx::ImageUsage::TransferDst | gfx::ImageUsage::TransferSrc,
        .aspects      = gfx::ImageAspects::Depth | gfx::ImageAspects::Stencil,
        .extent       = gfx::Extent3D{new_extent.x, new_extent.y, 1},
        .mip_levels   = 1,
        .array_layers = 1,
        .sample_count = gfx::SampleCount::Count1};
    ctx.scatch.depth_stencil_image =
        ctx.device
            ->create_image(ctx.device.self, ctx.scatch.depth_stencil_image_desc)
            .unwrap();
    ctx.scatch.depth_stencil_image_view_desc = gfx::ImageViewDesc{
        .label       = "Scratch Depth Stencil Image View",
        .image       = ctx.scatch.depth_stencil_image,
        .view_type   = gfx::ImageViewType::Type2D,
        .view_format = ctx.scatch.depth_stencil_image_desc.format,
        .mapping     = {},
        .aspects     = gfx::ImageAspects::Depth | gfx::ImageAspects::Stencil,
        .first_mip_level   = 0,
        .num_mip_levels    = 1,
        .first_array_layer = 0,
        .num_array_layers  = 1};
    ctx.scatch.depth_stencil_image_view =
        ctx.device
            ->create_image_view(ctx.device.self,
                                ctx.scatch.depth_stencil_image_view_desc)
            .unwrap();
  }

  void init_passes()
  {
    passes.bloom.init(ctx);
    passes.blur.init(ctx);
    passes.fxaa.init(ctx);
    passes.msaa.init(ctx);
    passes.pbr.init(ctx);
    passes.custom.init(ctx);
    passes.rrect.init(ctx);
  }

  void uninit()
  {
    passes.bloom.uninit(ctx);
    passes.blur.uninit(ctx);
    passes.fxaa.uninit(ctx);
    passes.msaa.uninit(ctx);
    passes.pbr.uninit(ctx);
    passes.custom.uninit(ctx);
    passes.rrect.uninit(ctx);
  }

  void begin_frame(gfx::Swapchain swapchain);
  void record_frame(gfx::Swapchain swapchain);
  void end_frame(gfx::Swapchain swapchain);
};

}        // namespace ash
