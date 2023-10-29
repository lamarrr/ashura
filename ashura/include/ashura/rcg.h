#pragma once
#include "ashura/gfx.h"
#include "ashura/rhi.h"
#include "ashura/sparse_vec.h"
#include "stx/fn.h"
#include "stx/vec.h"

namespace ash
{
namespace rcg
{

constexpr u8 MAX_FRAMES_IN_FLIGHT = 2;

// we'll support GLSL->SPIRV and Shader Editor -> C++ -> GLSL -> SPIRV
// contains all loaded shaders
//
//
// TODO(lamarrr): multi-pass dependency? knowing when to free resources
//
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

struct Graph;

// used for: validation layer, logging, warning, and driver dispatch
struct CommandBufferHook
{
  Graph *graph = nullptr;

  // todo(lamarrr): clear subresource
  virtual void fill_buffer(gfx::Buffer buffer, u64 offset, u64 size, u32 data)         = 0;
  virtual void copy_buffer(gfx::Buffer src, gfx::Buffer dst,
                           stx::Span<gfx::BufferCopy const> copies)                    = 0;
  virtual void update_buffer(stx::Span<u8 const> src, u64 dst_offset, gfx::Buffer dst) = 0;
  virtual void clear_image(gfx::Image dst, gfx::Color color)                           = 0;
  virtual void clear_image(gfx::Image dst, gfx::DepthStencil depth_stencil)            = 0;
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

struct CommandBuffer
{
  Graph                          *graph            = nullptr;
  CommandBufferHook              *hook             = nullptr;
  gfx::RenderPass                 render_pass      = nullptr;
  gfx::Framebuffer                framebuffer      = nullptr;
  stx::Vec<stx::UniqueFn<void()>> completion_tasks = {};        // MUST be run in reverse order
  gfx::CommandBuffer              handle           = nullptr;

  void fill_buffer(gfx::Buffer buffer, u64 offset, u64 size, u32 data);
  void copy_buffer(gfx::Buffer src, gfx::Buffer dst, stx::Span<gfx::BufferCopy const> copies);
  void update_buffer(stx::Span<u8 const> src, u64 dst_offset, gfx::Buffer dst);
  void clear_image(gfx::Image dst, gfx::Color color);
  void clear_image(gfx::Image dst, gfx::DepthStencil depth_stencil);
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

  virtual void create(gfx::BufferDesc const &desc)                                         = 0;
  virtual void create(gfx::BufferViewDesc const &desc)                                     = 0;
  virtual void create(gfx::ImageDesc const &desc, gfx::Color initial_color)                = 0;
  virtual void create(gfx::ImageDesc const &desc, gfx::DepthStencil initial_depth_stencil) = 0;
  virtual void create(gfx::ImageDesc const &desc, gfx::Buffer initial_data)                = 0;
  virtual void create(gfx::ImageViewDesc const &desc)                                      = 0;
  virtual void create(gfx::SamplerDesc const &sampler)                                     = 0;
  virtual void create(gfx::RenderPassDesc const &desc)                                     = 0;
  virtual void create(gfx::FramebufferDesc const &desc)                                    = 0;
  virtual void create(gfx::DescriptorSetLayoutDesc const &desc)                            = 0;
  virtual void create(gfx::ComputePipelineDesc const &desc)                                = 0;
  virtual void create(gfx::GraphicsPipelineDesc const &desc)                               = 0;
  virtual void ref(gfx::Buffer buffer)                                                     = 0;
  virtual void ref(gfx::BufferView buffer_view)                                            = 0;
  virtual void ref(gfx::Image image)                                                       = 0;
  virtual void ref(gfx::ImageView image_view)                                              = 0;
  virtual void ref(gfx::Sampler sampler)                                                   = 0;
  virtual void ref(gfx::RenderPass render_pass)                                            = 0;
  virtual void ref(gfx::Framebuffer framebuffer)                                           = 0;
  virtual void ref(gfx::DescriptorSetLayout descriptor_set_layout)                         = 0;
  virtual void ref(gfx::ComputePipeline pipeline)                                          = 0;
  virtual void ref(gfx::GraphicsPipeline pipeline)                                         = 0;
  virtual void unref(gfx::Buffer buffer)                                                   = 0;
  virtual void unref(gfx::BufferView buffer_view)                                          = 0;
  virtual void unref(gfx::Image image)                                                     = 0;
  virtual void unref(gfx::ImageView image_view)                                            = 0;
  virtual void unref(gfx::Sampler sampler)                                                 = 0;
  virtual void unref(gfx::RenderPass render_pass)                                          = 0;
  virtual void unref(gfx::Framebuffer framebuffer)                                         = 0;
  virtual void unref(gfx::DescriptorSetLayout descriptor_set_layout)                       = 0;
  virtual void unref(gfx::ComputePipeline pipeline)                                        = 0;
  virtual void unref(gfx::GraphicsPipeline pipeline)                                       = 0;
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
// for each creation and unref commands, we'll insert optional hooks that check that the
// parameters are correct we need to check the graph for information regarding the type and its
// dependencies
//
//
// on_queue_drain should be called on every device/queue idle wait
// on scheduled frame_fence sent check if any resource has been requested to be unrefd, if so,
// unref
//
// temporary images, temporary buffers,
//
// add callbacks to execute at end of fence on every frame
//
//
//
struct Graph
{
  gfx::Buffer      create(gfx::BufferDesc const &desc);
  gfx::BufferView  create(gfx::BufferViewDesc const &desc);
  gfx::Image       create(gfx::ImageDesc const &desc, gfx::Color initial_color);
  gfx::Image       create(gfx::ImageDesc const &desc, gfx::DepthStencil initial_depth_stencil);
  gfx::Image       create(gfx::ImageDesc const &desc, gfx::Buffer initial_data);
  gfx::ImageView   create(gfx::ImageViewDesc const &desc);
  gfx::Sampler     create(gfx::SamplerDesc const &sampler);
  gfx::RenderPass  create(gfx::RenderPassDesc const &desc);
  gfx::Framebuffer create(gfx::FramebufferDesc const &desc);
  gfx::DescriptorSetLayout                    create(gfx::DescriptorSetLayoutDesc const &desc);
  gfx::ComputePipeline                        create(gfx::ComputePipelineDesc const &desc);
  gfx::GraphicsPipeline                       create(gfx::GraphicsPipelineDesc const &desc);
  void                                        ref(gfx::Buffer buffer);
  void                                        ref(gfx::BufferView buffer_view);
  void                                        ref(gfx::Image image);
  void                                        ref(gfx::ImageView image_view);
  void                                        ref(gfx::Sampler sampler);
  void                                        ref(gfx::RenderPass render_pass);
  void                                        ref(gfx::Framebuffer framebuffer);
  void                                        ref(gfx::DescriptorSetLayout descriptor_set_layout);
  void                                        ref(gfx::ComputePipeline pipeline);
  void                                        ref(gfx::GraphicsPipeline pipeline);
  void                                        unref(gfx::Buffer buffer);
  void                                        unref(gfx::BufferView buffer_view);
  void                                        unref(gfx::Image image);
  void                                        unref(gfx::ImageView image_view);
  void                                        unref(gfx::Sampler sampler);
  void                                        unref(gfx::RenderPass render_pass);
  void                                        unref(gfx::Framebuffer framebuffer);
  void                                        unref(gfx::DescriptorSetLayout descriptor_set_layout);
  void                                        unref(gfx::ComputePipeline pipeline);
  void                                        unref(gfx::GraphicsPipeline pipeline);
  SparseVec<gfx::BufferResource, gfx::Buffer> buffers;
  SparseVec<gfx::BufferViewResource, gfx::BufferView>                   buffer_views;
  SparseVec<gfx::ImageResource, gfx::Image>                             images;
  SparseVec<gfx::ImageViewResource, gfx::ImageView>                     image_views;
  SparseVec<gfx::SamplerResource, gfx::Sampler>                         samplers;
  SparseVec<gfx::RenderPassResource, gfx::RenderPass>                   render_passes;
  SparseVec<gfx::FramebufferResource, gfx::Framebuffer>                 framebuffers;
  SparseVec<gfx::DescriptorSetLayoutResource, gfx::DescriptorSetLayout> descriptor_set_layouts;
  SparseVec<gfx::ComputePipelineResource, gfx::ComputePipeline>         compute_pipelines;
  SparseVec<gfx::GraphicsPipelineResource, gfx::GraphicsPipeline>       graphics_pipelines;
  rhi::Driver                                                          *driver = nullptr;
  GraphHook                                                            *hook   = nullptr;
  CommandBuffer command_buffers[MAX_FRAMES_IN_FLIGHT]                          = {};
  u8            current_command_buffer                                         = 0;
};

}        // namespace rcg
}        // namespace ash
