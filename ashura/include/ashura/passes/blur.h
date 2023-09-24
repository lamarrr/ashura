#pragma once
#include <string_view>

#include "ashura/lgfx.h"
#include "ashura/primitives.h"
#include "ashura/utils.h"
#include "stx/fn.h"
#include "stx/vec.h"

namespace ash
{
namespace lgfx
{

// ShaderMap, combinations

struct ViewInfo
{
  // pass 1 info
  // pass 2 info
  // pass 3 info
  // pass 4 info
};

struct BlurCapturePass
{
  struct Arguments
  {
    Extent blur_radius                  = Extent{};
    Extent input_image_subregion_extent = Extent{};
    Format input_image_format           = Format::R8G8B8A8_UNORM;
  } arguments;

  struct Resources
  {
    Buffer kernel_buffer           = Buffer::None;
    Image  sample_image            = Image::None;
    u32    sample_image_mip_levels = 0;
    Extent sample_image_extent     = Extent{};
    Buffer sample_buffer           = Buffer::None;
    Buffer result_buffer           = Buffer::None;
  } resources;

  struct State
  {
    ImageBlit       mip_down_blits[6];
    ImageBlit       mip_up_blits[6];
    ResourceBinding pipeline_bindings[32];
  } state;

  struct Bindings
  {
    Image  input_image        = Image::None;
    u32    input_image_mip    = 0;
    Offset input_image_offset = Offset{};
  } bindings;

  constexpr u8 pixel_byte_size(Format)
  {
    return 1;
  }

  virtual void create(Graph &graph, CmdBuffer &cmd_buffer)
  {
    // SETUP
    // TODO(lamarrr): how do we handle alpha??
    // TODO(lamarrr): we can just blit from the source image down to mips directly
    resources.sample_image_mip_levels = std::min(arguments.input_image_subregion_extent.max_mip_levels(), 6U);
    resources.sample_image_extent     = arguments.input_image_subregion_extent;
    Extent downsampled_input_extent   = arguments.input_image_subregion_extent.at_mip_level(resources.sample_image_mip_levels - 1);
    resources.kernel_buffer           = graph.create_buffer(BufferDesc{.size       = arguments.blur_radius.area(),
                                                                       .properties = graph.ctx.device_info.memory_heaps.has_unified_memory() ? (MemoryProperties::DeviceLocal | MemoryProperties::HostVisible) : MemoryProperties::HostVisible,
                                                                       .usages     = BufferUsages::UniformBuffer});
    resources.sample_image            = graph.create_image(ImageDesc{.format = arguments.input_image_format,
                                                                     .usages = ImageUsages::Sampled,
                                                                     .extent = arguments.input_image_subregion_extent,
                                                                     .mips   = resources.sample_image_mip_levels});
    resources.sample_buffer           = graph.create_buffer(BufferDesc{.size       = downsampled_input_extent.area() * pixel_byte_size(arguments.input_image_format),
                                                                       .properties = MemoryProperties::DeviceLocal,
                                                                       .usages     = BufferUsages::TransferDst | BufferUsages::TransferSrc | BufferUsages::StorageBuffer});
    resources.result_buffer           = graph.create_buffer(BufferDesc{.size       = downsampled_input_extent.area() * pixel_byte_size(arguments.input_image_format),
                                                                       .properties = MemoryProperties::DeviceLocal,
                                                                       .usages     = BufferUsages::TransferDst | BufferUsages::TransferSrc | BufferUsages::StorageBuffer});
    // resource initialization stage
    // resource pre-render setup
    //
  }

  // to check if to recreate resources
  bool diff(Graph const &graph, Arguments const &new_args)
  {
    return false;
  }

  virtual void begin(CmdBuffer &cmd_buffer)
  {
    // TODO(lamarrr): assume this is correct
    state.mip_up_blits[resources.sample_image_mip_levels - 1] = state.mip_down_blits[0] = ImageBlit{
        .src_area      = URect{.offset = {0, 0}, .extent = resources.sample_image_extent},
        .src_mip_level = 0,
        .src_aspect    = ImageAspect::Color,
        .dst_area      = URect{.offset = {0, 0}, .extent = resources.sample_image_extent},
        .dst_mip_level = 0,
        .dst_aspect    = ImageAspect::Color};

    for (u32 i = 1; i < resources.sample_image_mip_levels; i++)
    {
      state.mip_up_blits[resources.sample_image_mip_levels - 1 - i] = state.mip_down_blits[i] = ImageBlit{
          .src_area      = URect{.offset = {0, 0}, .extent = resources.sample_image_extent.at_mip_level(i - 1)},
          .src_mip_level = i - 1,
          .src_aspect    = ImageAspect::Color,
          .dst_area      = URect{.offset = {0, 0}, .extent = resources.sample_image_extent.at_mip_level(i)},
          .dst_mip_level = i,
          .dst_aspect    = ImageAspect::Color};
    }

    cmd_buffer.add(cmd::BlitImage{.src    = bindings.input_image,
                                  .dst    = resources.sample_image,
                                  .blits  = stx::Span{state.mip_down_blits, resources.sample_image_mip_levels},
                                  .filter = Filter::Nearest});

    // HIGH-LEVEL RENDER & COMPUTE PIPELINE COMPONENTS (ABSTRACTION)
    // EFFECTS & POST-PROCESSING
    // MESH MANAGEMENT
    // MESH BATCHING & INSTANCING
    // MATERIAL MANAGEMENT
    // RESOURCE MANAGEMENT
    // CAMERA MANAGEMENT
    // LIGHT MANAGEMENT
    // SCENE GRAPH (SORTING, CULLING)

    // MID-LEVEL RENDER & COMPUTE PIPELINE COMPONENTS
    // RESOURCE SYNCHRONIZATION & MANAGEMENT (I.E. BARRIERS)
    // TASK GRAPHS

    // LOW-LEVEL RENDER & COMPUTE PIPELINE COMPONENTS (PLATFORM-SPECIFIC)
    // RENDER PASSES
    // COMPUTE PASSES
    // PIPELINES
    // SHADERS
    // PSO & PSO CACHES

    /**
     * HANDLES:
     * - Resource state tracking and transition (barriers)
     * - Resource creation, recreation, and management
     */
    //
    //
    //
    // TODO(lamarrr): Note: the same resource can be used in many ways
    // framebuffers are cached by the system on a per-pass basis
    // this signifies a draw call/compute call reception command
    //
    // this is for the FXPass receiver to use the information to perform draw calls
    // the FXPass receiver decides which shaders to use, what shader parameters and the inputs, which would also require a command receiver
    //
    //
    // or should we receive draw calls???? this will cause a lot of churn and logic within the EffectPass???
    // what if we need vertices computed externally or something???
    //
    //
    //
    // create pass
    // initialize with render/compute shaders for pass creation
    // take present render/compute shaders and pipeline, bind it to the
    //
    //
    // DispatchPipeline
    // DispatchComputePipeline? what about bindings?
    // we need: dynamic pipelines, dynamic (cached) shaders
    //
    // split into create, begin, end, destroy???
    //
    //
    //
    // DeviceLocal::HostVisible when available
    //
    // or AccessType::OnDeviceOnly? this will enable it to write directly
    // AccessType::Host AccessType::DeviceAndHost
    //
    // or request UsageHints
    //
    // renderpass cache
    //
    // RESOURCE CREATION
    // TODO(lamarrr): we need to check for image aliasing for the renderpasses
    //
    // TODO(lamarrr): some states in the graphics ctx might change, we need to diff them for the passes
    // Resource utilization optimization passes
    //
    // the graphics api already knows how to optimize and multi-thread accesses, we just need to insert barriers appropriately
    //
    // each operation will have a number of barriers that need to be inserted before it executes
    //
    // TODO(lamarrr): we will cache and create new renderpasses every time
    //
    /**slots description to be fed to pipeline and allow pipeline structure itself*/
    /**combination of images to feed to pipeline along with renderpass*/
    // we can hash the frame buffer description and store it somewhere and then re-use it for the gpu?
    // render passes are used for computing tiling strategy on the GPU. we can create the render passes along with the framebuffer object
    // it is a property of the passes we create, and it should be enough
    //
    // render pass just needs to be a compatible render pass with pre-computed tiling strategy
    // we can cache renderpasses on a per-pass basis or cache the data it computes and just use a compatible renderpass that has the same operations and format
    //
    //
    // TODO(lamarrr): how do we perform: beginrenderpass and endrenderpass then?
    //
    //
    // on framebuffer creation we can use a different renderpass than the originally created one
    //
    // renderpasses are cached ATTACHMENT_UNUSED slots
    //
    //
    // we can cache framebuffers as they can be dynamic for some stype of passes
    // TODO(lamarrr): how do we know when a renderpass and framebuffer can be destroyed?
    //
    //
    //
    //
    //
    //
    //
    // TODO(lamarrr): since we are performing transfers on the same queue family, we can just insert upload barriers instead of waiting on uploads to finish??
    // but since we are writing to a possibly in-use memory, we will need to sync up or check if an upload is already in progress with the buffer?
    // or use vulkan events to let us know when uploads are in progress and when they are done
    //
    // Action and synchronization commands recorded to a command buffer execute the
    // VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT pipeline stage in submission order - forming an implicit
    // execution dependency between this stage in each command.
    //
    // each command executes VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT in the order they were submitted, this is the only execution order guarantee on the queue
    //
    // https://github.com/KhronosGroup/Vulkan-Docs/issues/552
    //
    // A Queue is an out-of-order execution unit that executes a specific set of tasks
    //
    // TODO(lamarrr): each task might have different synchronization tasks to perform
    //
    //
    // we need to keep track of:
    // where it was last used, what it was last used for and how
    // where it will next be used at, what it will next be used for
    //
    //
    //
    //

    // for each blur capture
    // get the input texture
    // don't copy, just reference it if
    // copy to first mip of images using transfer queue
    // mip down to the smallest mip using transfer queue
    // perform gaussian blur on smallest mip using compute shader
    // upscale to other mips
    //
    //
    // a = create_onscreen_pass(blur_arguments) -> pass will contain resources and scheduling order
    // b = create_blur_pass()
    //
    // a.draw_rect()
    // a.draw_rect()
    //
    // b.execute(blur_bindings: a) // TODO(lamarrr): also make blur pass creation easy
    // a.draw_image(b.output)
    //
    //
    //
    // c = create_highlight_pass(highlight_arguments), this for example is just disabling of the pipeline before drawing,
    //
    //
    //
    //
    //
    // I DON'T THINK THIS ABSTRACTION IS NECESSARY, LET THE USER CONFIGURE THE PIPELINE THEMSELVES AND CONNECT NODES TOGETHER APPROPRIATELY
    //
    //
    //
    //

    //
    // TODO(lamarrr): mips up to a certain extent, sanity check the mip level
    // prepare constants
    // RENDER
    //
    // for each blur pass
    // for vert and horz pass
    //
    // transition src image layout to transfer src
    // copy from src image to the buffer
    // use buffer as SSBO
    // perform seperable vert and horz gaussian blur in compute shader
    //
    // hand over image to the next user and let them decide how to use them? only valid usage is sampled image
    //
    //
    // copy image from input to sample mip level 0
    // blit image to image mip level N-1
    // what if we want to execute compute shader at creation time?
    // this should be done externally, we are only concerned with pass resource management????
    // even if done externally, we still need to track resources?
    //
    // TODO(lamarrr): what if pipeline clears depth stencil multiple times?
    //
    //
    // a concerning usage would be:
    //
    // perform pass
    // render draw commands//
    // render draw commands// here, we would need to generate multiple barriers depending on the number of tasks being performed, we can make it a part of the binding argument
    // render draw commands//
    // render draw commands//
    // perform pass
    //
    //
    //
    //
    // TODO(lamarrr): what about intermediate drawing passes? will this be handled by the executor??? will this worsen resource management?
    // TODO(lamarrr): what about specifying multiple commands??? with inputs??

    state.pipeline_bindings[0] = BufferBinding{.buffer = resources.kernel_buffer, .access = Access::ShaderRead, .stages = PipelineStages::ComputeShader};
    state.pipeline_bindings[1] = BufferBinding{.buffer = resources.sample_buffer, .access = Access::ShaderRead, .stages = PipelineStages::ComputeShader};
    state.pipeline_bindings[2] = BufferBinding{.buffer = resources.result_buffer, .access = Access::ShaderStorageWrite, .stages = PipelineStages::ComputeShader};

    // for graphics passes that write to the framebuffer, it will generate sync primitives for them
    cmd_buffer.add(cmd::BeginRenderPass{.render_pass = RenderPass::None});
    cmd_buffer.add(cmd::DispatchTask{.index    = 0,
                                     .type     = PipelineType::Compute,
                                     .bindings = stx::Span{state.pipeline_bindings, 2}});
    cmd_buffer.add(cmd::EndRenderPass{});

    // each render task execution will need to wait on the framebuffer
    cmd_buffer.add(cmd::DispatchTask{.index    = 0,
                                     .type     = PipelineType::Compute,
                                     .bindings = {}});

    cmd_buffer.add(cmd::BlitImage{.src    = bindings.input_image,
                                  .dst    = resources.sample_image,
                                  .blits  = stx::Span{state.mip_up_blits, resources.sample_image_mip_levels},
                                  .filter = Filter::Nearest});
  }

  virtual void end(CmdBuffer &cmd_buffer)
  {}
};

}        // namespace lgfx
}        // namespace ash
