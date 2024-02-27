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
};

}        // namespace ash
