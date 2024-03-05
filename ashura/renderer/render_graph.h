#pragma once
#include "ashura/gfx/gfx.h"
#include "ashura/std/option.h"
#include "ashura/std/result.h"
#include "ashura/std/types.h"

namespace ash
{
namespace rdg
{

struct RenderPassDesc;
struct FramebufferDesc;
struct ComputePipelineDesc;
struct GraphicsPipelineDesc;
struct ImageAttachmentDesc;
struct RenderTargetDesc;

struct RenderPass;
struct Framebuffer;
struct ComputePipeline;
struct GraphicsPipeline;
struct ImageAttachment;
struct RenderTarget;

struct ImageAttachment
{
  gfx::Image     image = nullptr;
  gfx::ImageView view  = nullptr;
  gfx::ImageDesc desc  = {};
};

struct RenderTarget
{
  Span<ImageAttachment const> colors;
  ImageAttachment             depth_stencil;
  gfx::Offset                 scissor_offset;
  gfx::Extent                 scissor_extent;
};

struct Uniform
{
  gfx::DescriptorSet set;
  u64                buffer_offset = 0;
};

enum class PassFlags : u8
{
  None     = 0x00,
  Render   = 0x01,
  Compute  = 0x02,
  Transfer = 0x04,
  Mesh     = 0x08
};

ASH_DEFINE_ENUM_BIT_OPS(PassFlags)

}        // namespace rdg

struct RenderGraphCtx;

// per-frame uniform buffer
// allocate multiple large uniform buffers along with descriptor sets
// since we are buffering, once we get to this frame's next cycle, we would be
// able to write directly to the memory-mapped gpu memory then use dynamic
// offsets to point to the intended uniform. must be aligned to our
// requirements. what size of buffers to choose?
// 1024 bytes max uniform size, max alignment of 256-bytes, uniform buffers of
// size 8192 Bytes
//
struct RenderGraph
{
  // images resized when swapchain extents changes
  Option<rdg::ImageAttachment> create_attachment(gfx::ImageDesc const &desc);
  void release_attachment(rdg::ImageAttachment const &attachment);
  Option<rdg::RenderPass> create_render_pass(gfx::RenderPassDesc const &desc);
  void                    create_render_targets(int attachments);
  Option<rdg::ComputePipeline>  create_compute_pipeline();
  Option<rdg::GraphicsPipeline> create_graphics_pipeline();
  Option<gfx::Shader>           get_shader(Span<char const> name);
  void                          queue_delete();
  template <typename T, typename... Args>
  T *allocate_param(Args &&...args);

  template <typename Binding, typename Reg, typename Exe>
  void add_pass(Span<char const> name, rdg::PassFlags flags, Reg &&registration,
                Exe &&execution);

  void begin_frame();
  void end_frame();

  rdg::Uniform push_uniform();
};

}        // namespace ash
