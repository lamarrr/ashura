#pragma once
#include "ashura/gfx.h"
#include "ashura/rhi.h"
#include "ashura/sparse_array.h"
#include "stx/fn.h"
#include "stx/vec.h"

namespace ash
{
namespace rcg
{
struct CommandBufferHook;
struct CommandBuffer;
struct GraphHook;
struct Graph;

// used for: validation layer, logging, warning, and driver dispatch
struct CommandBufferHook
{
  Graph *graph = nullptr;

  virtual void fill_buffer(gfx::Buffer buffer, u64 offset, u64 size, u32 data)         = 0;
  virtual void copy_buffer(gfx::Buffer src, gfx::Buffer dst,
                           stx::Span<gfx::BufferCopy const> copies)                    = 0;
  virtual void update_buffer(stx::Span<u8 const> src, u64 dst_offset, gfx::Buffer dst) = 0;
  virtual void copy_image(gfx::Image src, gfx::Image dst,
                          stx::Span<gfx::ImageCopy const> copies)                      = 0;
  virtual void copy_buffer_to_image(gfx::Buffer src, gfx::Image dst,
                                    stx::Span<gfx::BufferImageCopy const> copies)      = 0;
  virtual void blit_image(gfx::Image src, gfx::Image dst, stx::Span<gfx::ImageBlit const> blits,
                          gfx::Filter filter)                                          = 0;
  virtual void begin_render_pass(
      gfx::Framebuffer framebuffer, gfx::RenderPass render_pass, IRect render_area,
      stx::Span<gfx::Color const>        color_attachments_clear_values,
      stx::Span<gfx::DepthStencil const> depth_stencil_attachments_clear_values) = 0;
  virtual void end_render_pass()                                                 = 0;
  virtual void dispatch(gfx::ComputePipeline pipeline, u32 group_count_x, u32 group_count_y,
                        u32 group_count_z, gfx::DescriptorSetBindings const &bindings,
                        stx::Span<u8 const> push_constants_data)                 = 0;
  virtual void dispatch_indirect(gfx::ComputePipeline pipeline, gfx::Buffer buffer, u64 offset,
                                 gfx::DescriptorSetBindings const &bindings,
                                 stx::Span<u8 const>               push_constants_data)        = 0;
  virtual void draw(gfx::GraphicsPipeline pipeline, gfx::RenderState const &state,
                    stx::Span<gfx::Buffer const> vertex_buffers, gfx::Buffer index_buffer,
                    u32 first_index, u32 num_indices, u32 vertex_offset, u32 first_instance,
                    u32 num_instances, gfx::DescriptorSetBindings const &bindings,
                    stx::Span<u8 const> push_constants_data);
  virtual void draw_indirect(gfx::GraphicsPipeline pipeline, gfx::RenderState const &state,
                             stx::Span<gfx::Buffer const> vertex_buffers, gfx::Buffer index_buffer,
                             gfx::Buffer buffer, u64 offset, u32 draw_count, u32 stride,
                             gfx::DescriptorSetBindings const &bindings,
                             stx::Span<u8 const>               push_constants_data)    = 0;
  virtual void on_execution_complete_fn(stx::UniqueFn<void()> const &fn) = 0;
  virtual ~CommandBufferHook()                                           = 0;
};

// USE IMAGE ACCESS INSTEAD? AND COALESCE THEM? i.e. used as read and used as write
struct CommandBuffer
{
  Graph                          *graph            = nullptr;
  CommandBufferHook              *hook             = nullptr;
  gfx::RenderPass                 render_pass      = nullptr;
  gfx::Framebuffer                framebuffer      = nullptr;
  stx::Vec<stx::UniqueFn<void()>> completion_tasks = {};        // MUST be run in reverse order
  gfx::CommandBuffer              handle           = nullptr;
  stx::Vec<gfx::QueueBufferMemoryBarrier> buffer_memory_barriers;
  stx::Vec<gfx::QueueImageMemoryBarrier>  image_memory_barriers;

  // TODO(lamarrr): use descriptor sets multiple times per-pass
  // NOTE:::: MUST be multiple of 4 for dst offset and dst size
  void fill_buffer(gfx::Buffer buffer, u64 offset, u64 size, u32 data);
  void copy_buffer(gfx::Buffer src, gfx::Buffer dst, stx::Span<gfx::BufferCopy const> copies);
  // NOTE:::: MUST be multiple of 4 for dst offset and dst size
  void update_buffer(stx::Span<u8 const> src, u64 dst_offset, gfx::Buffer dst);
  void copy_image(gfx::Image src, gfx::Image dst, stx::Span<gfx::ImageCopy const> copies);
  void copy_buffer_to_image(gfx::Buffer src, gfx::Image dst,
                            stx::Span<gfx::BufferImageCopy const> copies);
  void blit_image(gfx::Image src, gfx::Image dst, stx::Span<gfx::ImageBlit const> blits,
                  gfx::Filter filter);
  void begin_render_pass(gfx::Framebuffer framebuffer, gfx::RenderPass render_pass,
                         IRect                              render_area,
                         stx::Span<gfx::Color const>        color_attachments_clear_values,
                         stx::Span<gfx::DepthStencil const> depth_stencil_attachments_clear_values);
  void end_render_pass();
  void dispatch(gfx::ComputePipeline pipeline, u32 group_count_x, u32 group_count_y,
                u32 group_count_z, gfx::DescriptorSetBindings const &bindings,
                stx::Span<u8 const> push_constants_data);
  void dispatch_indirect(gfx::ComputePipeline pipeline, gfx::Buffer buffer, u64 offset,
                         gfx::DescriptorSetBindings const &bindings,
                         stx::Span<u8 const>               push_constants_data);
  void draw(gfx::GraphicsPipeline pipeline, gfx::RenderState const &state,
            stx::Span<gfx::Buffer const> vertex_buffers, gfx::Buffer index_buffer, u32 first_index,
            u32 num_indices, u32 vertex_offset, u32 first_instance, u32 num_instances,
            gfx::DescriptorSetBindings const &bindings, stx::Span<u8 const> push_constants_data);
  void draw_indirect(gfx::GraphicsPipeline pipeline, gfx::RenderState const &state,
                     stx::Span<gfx::Buffer const> vertex_buffers, gfx::Buffer index_buffer,
                     gfx::Buffer buffer, u64 offset, u32 draw_count, u32 stride,
                     gfx::DescriptorSetBindings const &bindings,
                     stx::Span<u8 const>               push_constants_data);
  void on_execution_complete_fn(stx::UniqueFn<void()> &&fn);
  template <typename TaskLambda>
  void on_execution_complete(TaskLambda &&lambda)
  {
    static_assert(std::is_invocable_v<TaskLambda &>);
    on_execution_complete_fn(
        stx::fn::rc::make_unique_functor(stx::os_allocator, std::forward<TaskLambda>(lambda))
            .unwrap());
  }
};

// used for: validation layer, logging
struct GraphHook
{
  Graph *graph = nullptr;

  virtual void create(gfx::BufferDesc const &desc)                         = 0;
  virtual void create(gfx::BufferViewDesc const &desc)                     = 0;
  virtual void create(gfx::ImageDesc const &desc)                          = 0;
  virtual void create(gfx::ImageViewDesc const &desc)                      = 0;
  virtual void create(gfx::SamplerDesc const &sampler)                     = 0;
  virtual void create(gfx::RenderPassDesc const &desc)                     = 0;
  virtual void create(gfx::FramebufferDesc const &desc)                    = 0;
  virtual void create(gfx::DescriptorSetDesc const &descriptor_set_layout) = 0;
  virtual void create(gfx::ComputePipelineDesc const &desc)                = 0;
  virtual void create(gfx::GraphicsPipelineDesc const &desc)               = 0;
  virtual void release(gfx::Buffer buffer)                                 = 0;
  virtual void release(gfx::BufferView buffer_view)                        = 0;
  virtual void release(gfx::Image image)                                   = 0;
  virtual void release(gfx::ImageView image_view)                          = 0;
  virtual void release(gfx::Sampler sampler)                               = 0;
  virtual void release(gfx::RenderPass render_pass)                        = 0;
  virtual void release(gfx::Framebuffer framebuffer)                       = 0;
  virtual void release(gfx::DescriptorSetLayout descriptor_set_layout)     = 0;
  virtual void release(gfx::ComputePipeline pipeline)                      = 0;
  virtual void release(gfx::GraphicsPipeline pipeline)                     = 0;
};

// interceptor that is used for validation and adding barriers
// this is separate from the graph as it could be recorded????
//
// TASKS: use flow information to insert barrier and perform optimal synchronization and resource
// layout conversions
// TODO(lamarrr): how do we enable features like raytracing dynamically at runtime?
// each pass declares flags?
//
// TODO(lamarrr): async shader compilation and loading, how?
//
//
// NOW: how to insert barrier and validation hooks
//
//
// for each creation and release commands, we'll insert optional hooks that check that the
// parameters are correct we need to check the graph for information regarding the type and its
// dependencies
//
//
// on_queue_drain should be called on every device/queue idle wait
// on scheduled frame_fence sent check if any resource has been requested to be released, if so,
// release
//
// temporary images, temporary buffers,
//
// add callbacks to execute at end of fence on every frame
//
//
//
struct Graph
{
  gfx::Buffer              create(gfx::BufferDesc const &desc);
  gfx::BufferView          create(gfx::BufferViewDesc const &desc);
  gfx::Image               create(gfx::ImageDesc const &desc);
  gfx::ImageView           create(gfx::ImageViewDesc const &desc);
  gfx::Sampler             create(gfx::SamplerDesc const &sampler);
  gfx::RenderPass          create(gfx::RenderPassDesc const &desc);
  gfx::Framebuffer         create(gfx::FramebufferDesc const &desc);
  gfx::DescriptorSetLayout create(gfx::DescriptorSetDesc const &descriptor_set_layout);
  gfx::ComputePipeline     create(gfx::ComputePipelineDesc const &desc);
  gfx::GraphicsPipeline    create(gfx::GraphicsPipelineDesc const &desc);
  void                     release(gfx::Buffer buffer);
  void                     release(gfx::BufferView buffer_view);
  void                     release(gfx::Image image);
  void                     release(gfx::ImageView image_view);
  void                     release(gfx::Sampler sampler);
  void                     release(gfx::RenderPass render_pass);
  void                     release(gfx::Framebuffer framebuffer);
  void                     release(gfx::DescriptorSetLayout descriptor_set_layout);
  void                     release(gfx::ComputePipeline pipeline);
  void                     release(gfx::GraphicsPipeline pipeline);
  void                     on_drain();
  SparseArray<gfx::BufferResource, gfx::Buffer>                           buffers;
  SparseArray<gfx::BufferViewResource, gfx::BufferView>                   buffer_views;
  SparseArray<gfx::ImageResource, gfx::Image>                             images;
  SparseArray<gfx::ImageViewResource, gfx::ImageView>                     image_views;
  SparseArray<gfx::SamplerResource, gfx::Sampler>                         samplers;
  SparseArray<gfx::RenderPassResource, gfx::RenderPass>                   render_passes;
  SparseArray<gfx::FramebufferResource, gfx::Framebuffer>                 framebuffers;
  SparseArray<gfx::DescriptorSetLayoutResource, gfx::DescriptorSetLayout> descriptor_set_layouts;
  SparseArray<gfx::ComputePipelineResource, gfx::ComputePipeline>         compute_pipelines;
  SparseArray<gfx::GraphicsPipelineResource, gfx::GraphicsPipeline>       graphics_pipelines;
  rhi::Driver                                                            *driver = nullptr;
  GraphHook                                                              *hook   = nullptr;
};

// we'll support GLSL->SPIRV and Shader Editor -> C++ -> GLSL -> SPIRV
// contains all loaded shaders
struct ShaderMap
{
  // shaders are always compiled and loaded at startup time
  // select shader
  // shader will have vendor, context, and name to compare to
  // if none was given then how?
  // we don't allow shaders to change at runtime, they must be baked and compiled AOT
  //
  // TODO(lamarrr): PSO caches
  //
  //
};

struct PipelineCacheMap
{
  // vendor id, pass name, name
  // frag shader id, vert shader id
};

}        // namespace rcg
}        // namespace ash
