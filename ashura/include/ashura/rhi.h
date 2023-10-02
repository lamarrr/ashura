#pragma once

#include "ashura/gfx.h"
#include "stx/span.h"

namespace ash
{
namespace rhi
{

struct Driver
{
  virtual ~Driver()                                                                                                                                                                                                                   = 0;
  virtual gfx::FormatProperties    get_format_properties(gfx::Format format)                                                                                                                                                          = 0;
  virtual gfx::Buffer              create(gfx::BufferDesc const &desc)                                                                                                                                                                = 0;
  virtual gfx::BufferView          create(gfx::BufferViewDesc const &desc)                                                                                                                                                            = 0;
  virtual gfx::Image               create(gfx::ImageDesc const &desc)                                                                                                                                                                 = 0;
  virtual gfx::ImageView           create(gfx::ImageViewDesc const &desc)                                                                                                                                                             = 0;
  virtual gfx::RenderPass          create(gfx::RenderPassDesc const &desc)                                                                                                                                                            = 0;
  virtual gfx::Framebuffer         create(gfx::FramebufferDesc const &desc)                                                                                                                                                           = 0;
  virtual gfx::Sampler             create(gfx::SamplerDesc const &sampler)                                                                                                                                                            = 0;
  virtual gfx::DescriptorSetLayout create(gfx::DescriptorSetDesc const &descriptor_set_layout)                                                                                                                                        = 0;
  virtual gfx::Shader              create_shader(stx::Span<u32 const> shader)                                                                                                                                                         = 0;
  virtual gfx::ComputePipeline     create(gfx::ComputePipelineDesc const &desc)                                                                                                                                                       = 0;
  virtual gfx::GraphicsPipeline    create(gfx::GraphicsPipelineDesc const &desc)                                                                                                                                                      = 0;
  virtual gfx::CommandBuffer       create_command_buffer()                                                                                                                                                                            = 0;
  virtual void                     release(gfx::Buffer buffer)                                                                                                                                                                        = 0;
  virtual void                     release(gfx::BufferView buffer_view)                                                                                                                                                               = 0;
  virtual void                     release(gfx::Image image)                                                                                                                                                                          = 0;
  virtual void                     release(gfx::ImageView image_view)                                                                                                                                                                 = 0;
  virtual void                     release(gfx::RenderPass render_pass)                                                                                                                                                               = 0;
  virtual void                     release(gfx::Framebuffer framebuffer)                                                                                                                                                              = 0;
  virtual void                     release(gfx::Sampler sampler)                                                                                                                                                                      = 0;
  virtual void                     release(gfx::DescriptorSetLayout descriptor_set_layout)                                                                                                                                            = 0;
  virtual void                     release(gfx::Shader shader)                                                                                                                                                                        = 0;
  virtual void                     release(gfx::ComputePipeline pipeline)                                                                                                                                                             = 0;
  virtual void                     release(gfx::GraphicsPipeline pipeline)                                                                                                                                                            = 0;
  virtual void                     release(gfx::CommandBuffer command_buffer)                                                                                                                                                         = 0;
  virtual void                     cmd_fill_buffer(gfx::CommandBuffer command_buffer, gfx::Buffer buffer, u64 offset, u64 size, u32 data)                                                                                             = 0;
  virtual void                     cmd_copy_buffer(gfx::CommandBuffer command_buffer, gfx::Buffer src, gfx::Buffer dst, stx::Span<gfx::BufferCopy const> copies)                                                                      = 0;
  virtual void                     cmd_update_buffer(gfx::CommandBuffer command_buffer, stx::Span<u8 const> src, u64 dst_offset, gfx::Buffer dst)                                                                                     = 0;
  virtual void                     cmd_copy_image(gfx::CommandBuffer command_buffer, gfx::Image src, gfx::Image dst, stx::Span<gfx::ImageCopy const> copies)                                                                          = 0;
  virtual void                     cmd_copy_buffer_to_image(gfx::CommandBuffer command_buffer, gfx::Buffer src, gfx::Image dst, stx::Span<gfx::BufferImageCopy const> copies)                                                         = 0;
  virtual void                     cmd_blit_image(gfx::CommandBuffer command_buffer, gfx::Image src, gfx::Image dst, stx::Span<gfx::ImageBlit const> blits, gfx::Filter filter)                                                       = 0;
  virtual void                     cmd_begin_render_pass(gfx::CommandBuffer command_buffer, gfx::RenderPassBeginInfo const &info)                                                                                                     = 0;
  virtual void                     cmd_end_render_pass(gfx::CommandBuffer command_buffer)                                                                                                                                             = 0;
  virtual void                     cmd_bind_pipeline(gfx::CommandBuffer command_buffer, gfx::ComputePipeline pipeline)                                                                                                                = 0;
  virtual void                     cmd_bind_pipeline(gfx::CommandBuffer command_buffer, gfx::GraphicsPipeline pipeline)                                                                                                               = 0;
  virtual void                     cmd_bind_vertex_buffers(gfx::CommandBuffer command_buffer, u32 first_binding, stx::Span<gfx::Buffer const> vertex_buffers, stx::Span<u64 const> offsets)                                           = 0;
  virtual void                     cmd_bind_index_buffer(gfx::CommandBuffer command_buffer, gfx::Buffer index_buffer, u64 offset)                                                                                                     = 0;
  virtual void                     cmd_push_constants(gfx::CommandBuffer command_buffer, gfx::PipelineBindPoint bind_point, stx::Span<u8 const> constants)                                                                            = 0;
  virtual void                     cmd_push_descriptor_set(gfx::CommandBuffer command_buffer, u32 set, gfx::DescriptorSetBindings const &bindings)                                                                                    = 0;
  virtual void                     cmd_set_scissor(gfx::CommandBuffer command_buffer, IRect scissor)                                                                                                                                  = 0;
  virtual void                     cmd_set_viewport(gfx::CommandBuffer command_buffer, gfx::Viewport const &viewport)                                                                                                                 = 0;
  virtual void                     cmd_set_blend_constants(gfx::CommandBuffer command_buffer, f32 r, f32 g, f32 b, f32 a)                                                                                                             = 0;
  virtual void                     cmd_set_stencil_compare_mask(gfx::CommandBuffer command_buffer, gfx::StencilFaces faces, u32 compare_mask)                                                                                         = 0;
  virtual void                     cmd_set_stencil_reference(gfx::CommandBuffer command_buffer, gfx::StencilFaces faces, u32 reference)                                                                                               = 0;
  virtual void                     cmd_set_stencil_write_mask(gfx::CommandBuffer command_buffer, gfx::StencilFaces faces, u32 write_mask)                                                                                             = 0;
  virtual void                     cmd_dispatch(gfx::CommandBuffer command_buffer, u32 group_count_x, u32 group_count_y, u32 group_count_z)                                                                                           = 0;
  virtual void                     cmd_dispatch_indirect(gfx::CommandBuffer command_buffer, gfx::Buffer buffer, u64 offset)                                                                                                           = 0;
  virtual void                     cmd_draw(gfx::CommandBuffer command_buffer, u32 first_vertex, u32 vertex_count, u32 instance_count, u32 first_instance_id)                                                                         = 0;
  virtual void                     cmd_draw_indexed(gfx::CommandBuffer command_buffer, u32 first_index, u32 index_count, u32 instance_count, i32 vertex_offset, u32 first_instance_id)                                                = 0;
  virtual void                     cmd_draw_indexed_indirect(gfx::CommandBuffer command_buffer, gfx::Buffer buffer, u64 offset, u32 draw_count, u32 stride)                                                                           = 0;
  virtual void                     cmd_insert_barriers(gfx::CommandBuffer command_buffer, stx::Span<gfx::QueueBufferMemoryBarrier const> buffer_memory_barriers, stx::Span<gfx::QueueImageMemoryBarrier const> image_memory_barriers) = 0;
};

};        // namespace rhi
}        // namespace ash
