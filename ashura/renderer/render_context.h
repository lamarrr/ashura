#pragma once
#include "ashura/gfx/gfx.h"
#include "ashura/renderer/scene.h"
#include "ashura/renderer/shader.h"
#include "ashura/renderer/view.h"
#include "ashura/std/dict.h"
#include "ashura/std/error.h"
#include "ashura/std/option.h"
#include "ashura/std/result.h"
#include "ashura/std/types.h"

namespace ash
{

struct Mesh
{
  gfx::Buffer    vertex_buffer        = nullptr;
  u64            vertex_buffer_offset = 0;
  gfx::Buffer    index_buffer         = nullptr;
  u64            index_buffer_offset  = 0;
  gfx::IndexType index_type           = gfx::IndexType::Uint16;
};

/// color_images and depth_stencil_image format must be same as render context's
struct RenderTarget
{
  Span<gfx::ImageView const> color_images          = {};
  gfx::ImageView             depth_stencil_image   = nullptr;
  gfx::ImageAspects          depth_stencil_aspects = gfx::ImageAspects::None;
  Vec2U                      extent                = {};
  Vec2U                      render_offset         = {};
  Vec2U                      render_extent         = {};
};

/// created with sampled, storage, color attachment, and transfer flags
struct Scratch
{
  gfx::ImageDesc     color_image_desc              = {};
  gfx::ImageDesc     depth_stencil_image_desc      = {};
  gfx::ImageViewDesc color_image_view_desc         = {};
  gfx::ImageViewDesc depth_stencil_image_view_desc = {};
  gfx::Image         color_image                   = nullptr;
  gfx::Image         depth_stencil_image           = nullptr;
  gfx::ImageView     color_image_view              = nullptr;
  gfx::ImageView     depth_stencil_image_view      = nullptr;
};

/// @color_format: hdr if hdr supported and required
/// scratch images resized when swapchain extents changes
struct RenderContext
{
  gfx::DeviceImpl          device               = {};
  gfx::PipelineCache       pipeline_cache       = nullptr;
  gfx::FrameContext        frame_context        = nullptr;
  gfx::FrameInfo           frame_info           = {};
  gfx::Format              color_format         = gfx::Format::Undefined;
  gfx::Format              depth_stencil_format = gfx::Format::Undefined;
  Scratch                  scatch               = {};
  Vec<UniformHeap>         uniform_heaps        = {};
  gfx::DescriptorSetLayout uniform_layout       = nullptr;
  Vec<Tuple<gfx::FrameId, gfx::Framebuffer>> released_framebuffers = {};
  Vec<Tuple<gfx::FrameId, gfx::Image>>       released_images       = {};
  Vec<Tuple<gfx::FrameId, gfx::ImageView>>   released_image_views  = {};
  // Dict shader_map;

  void init();

  gfx::CommandEncoderImpl encoder() const
  {
    return frame_info.command_encoders[ring_index()];
  }

  u8 ring_index() const
  {
    return (u8) frame_info.current_command_encoder;
  }

  u8 max_frames_in_flight() const
  {
    return (u8) frame_info.command_encoders.size();
  }

  template <typename T>
  Uniform push_uniform(T const &uniform)
  {
    return uniform_heaps[ring_index()].push(uniform);
  }

  template <typename T>
  Uniform push_uniform_range(Span<T const> uniform)
  {
    return uniform_heaps[ring_index()].push_range(uniform);
  }

  Option<gfx::Shader> get_shader(Span<char const> name);

  void release(gfx::Framebuffer framebuffer)
  {
    CHECK(released_framebuffers.push(frame_info.current, framebuffer));
  }

  void release(gfx::Image image)
  {
    CHECK(released_images.push(frame_info.current, image));
  }

  void release(gfx::ImageView view)
  {
    CHECK(released_image_views.push(frame_info.current, view));
  }
};

}        // namespace ash
