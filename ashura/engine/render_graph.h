#pragma once
#include "ashura/gfx/gfx.h"
#include "ashura/std/option.h"
#include "ashura/std/result.h"
#include "ashura/std/types.h"

namespace ash
{
namespace rdg
{

struct Attachment
{
  gfx::Image     image = nullptr;
  gfx::ImageView view  = nullptr;
  gfx::ImageDesc desc  = {};
};

struct RenderTarget
{
  Attachment  color;
  Attachment  depth_stencil;
  gfx::Offset offset;
  gfx::Extent extent;
};

struct FramebufferDesc
{
};

struct RenderPassDesc
{
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

struct RenderGraph
{
  // images resized when swapchain extents changes
  Option<rdg::Attachment>
       request_scratch_attachment(gfx::ImageDesc const &desc);
  void release_scratch_attachment(rdg::Attachment const &attachment);
  Option<gfx::RenderPass>      get_render_pass(gfx::RenderPassDesc const &desc);
  Option<gfx::ComputePipeline> get_compute_pipeline();
  Option<gfx::GraphicsPipeline> get_graphics_pipeline();
  Option<gfx::Shader>           get_shader(Span<char const> name);
  void                          queue_delete(u64 last_use_tick);
  template <typename T, typename... Args>
  T *allocate_param(Args &&...args);

  template <typename Binding, typename Reg, typename Exe>
  void add_pass(Span<char const> name, rdg::PassFlags flags, Reg &&registration,
                Exe &&execution);
};

}        // namespace ash
