#pragma once
#include "ashura/primitives.h"
#include "ashura/utils.h"

namespace ash
{
namespace lgfx
{

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

using rid             = u32;
constexpr rid RID_MAX = 0xFFFFFFFF;

enum class MemoryProperties : u32
{
  DeviceLocal  = 0x00000001,
  HostVisible  = 0x00000002,
  HostCoherent = 0x00000004,
  HostCached   = 0x00000008
};

enum class ImageFormat : u32
{
  Undefined      = 0,
  R8_Unorm       = 9,
  BGRA8888_Unorm = 30,
  RGBA8888_Unorm = 37,
  D32_SFloat     = 126
};

enum class ImageLayout : u32
{
  Undefined                     = 0,
  General                       = 1,
  ColorAttachmentOptimal        = 2,
  DepthStencilAttachmentOptimal = 3,
  DepthStencilReadOnlyOptimal   = 4,
  ShaderReadOnlyOptimal         = 5,
  TransferSrcOptimal            = 6,
  TransferDstOptimal            = 7,
  PresentSrc                    = 1000001002
};

constexpr bool is_color_format(ImageFormat fmt)
{
  switch (fmt)
  {
    case ImageFormat::R8_Unorm:
    case ImageFormat::BGRA8888_Unorm:
    case ImageFormat::RGBA8888_Unorm:
      return true;
    default:
      return false;
  }
}

constexpr u32 pixel_byte_size(ImageFormat fmt)
{
  switch (fmt)
  {
    case ImageFormat::Undefined:
      return 0;
    case ImageFormat::R8_Unorm:
      return 1;
    case ImageFormat::BGRA8888_Unorm:
      return 4;
    case ImageFormat::RGBA8888_Unorm:
      return 4;
    case ImageFormat::D32_SFloat:
      return 4;
    default:
      return 4;
  }
}

enum class PipelineStages : u32
{
  None                  = 0x00000000,
  TopOfPipe             = 0x00000001,
  VertexShader          = 0x00000008,
  FragmentShader        = 0x00000080,
  ColorAttachmentOutput = 0x00000400,
  ComputeShader         = 0x00000800,
  Transfer              = 0x00001000,
  BottomOfPipe          = 0x00002000,
  RayTracingShader      = 0x00200000
};

STX_DEFINE_ENUM_BIT_OPS(PipelineStages)

enum class BufferUsages : u32
{
  TransferSrc               = 0x00000001,
  TransferDst               = 0x00000002,
  UniformTexelBuffer        = 0x00000004,
  UniformStorageTexelBuffer = 0x00000008,
  UniformBuffer             = 0x00000010,
  StorageBuffer             = 0x00000020,
  IndexBuffer               = 0x00000040,
  VertexBuffer              = 0x00000080
};

STX_DEFINE_ENUM_BIT_OPS(BufferUsages)

enum class ImageUsages : u32
{
  TransferSrc            = 0x00000001,
  TransferDst            = 0x00000002,
  Sampled                = 0x00000004,
  Storage                = 0x00000008,
  ColorAttachment        = 0x00000010,
  DepthStencilAttachment = 0x00000020
};

STX_DEFINE_ENUM_BIT_OPS(ImageUsages)

enum class ResourceAccess : u32
{
  None                        = 0x00000000,
  IndexRead                   = 0x00000002,
  VertexAttributeRead         = 0x00000004,
  UniformRead                 = 0x00000008,
  ShaderRead                  = 0x00000020,
  ShaderWrite                 = 0x00000040,
  ColorAttachmentRead         = 0x00000080,
  ColorAttachmentWrite        = 0x00000100,
  DepthStencilAttachmentRead  = 0x00000200,
  DepthStencilAttachmentWrite = 0x00000400,
  TransferRead                = 0x00000800,
  TransferWrite               = 0x00001000,
  HostRead                    = 0x00002000,
  HostWrite                   = 0x00004000,
  MemoryRead                  = 0x00008000,
  MemoryWrite                 = 0x00010000
};

STX_DEFINE_ENUM_BIT_OPS(ResourceAccess)

struct image_desc
{
  char const *pass;
  char const *name;
  ImageFormat format;
  ImageUsages usages;
  extent      size;
  u32         mips;
};

struct buffer_desc
{
  char const      *pass;
  char const      *name;
  u64              size;
  MemoryProperties properties;
  BufferUsages     usages;
};

struct renderpass_desc
{
  char const *pass;
  char const *name;
  ImageFormat color_format;
  ImageFormat depth_stencil_format;
};

struct framebuffer_desc
{
  char const *pass;
  char const *name;
  rid         render_pass;
  rid         color;
  rid         depth_stencil;        // OPTIONAL
};

struct ResourceStates
{
  ResourceAccess *accesses     = nullptr;
  u32             num_accesses = 0;
  ImageLayout    *layout_transitions;        // TODO(lamarrr): what about when it has already been transitioned
  u32             num_state_transitions = 0;
};

struct Graph
{
  u32               num_images     = 0;
  u32               num_buffers    = 0;
  image_desc       *images         = nullptr;
  buffer_desc      *buffers        = nullptr;
  ResourceStates   *images_states  = nullptr;
  ResourceStates   *buffers_states = nullptr;
  renderpass_desc  *render_passes  = nullptr;
  framebuffer_desc *framebuffers   = nullptr;

  // RESOURCE CREATION
  rid create_buffer(u64 size, MemoryProperties properties, BufferUsages usages);
  rid create_image(ImageFormat format, ImageUsages usages, extent size, u32 mips);
  rid create_render_pass(/**slots description to be fed to pipeline and allow pipeline structure itself*/);
  rid create_frame_buffer(rid render_pass, rid color);            /**combination of images to feed to pipeline along with renderpass*/
  rid create_frame_buffer(rid render_pass, rid color, rid depth); /**combination of images to feed to pipeline along with renderpass*/

  // RESOURCE ACCESS DESCRIPTIONS
  void copy_data_to_buffer(rid buffer, stx::Span<u8 const> bytes);
  void blit_image(rid src_image, rid dst_image, rid mip_level);
  void copy_buffer_to_image(rid src_buffer, rid dst_image, u32 mip_level);
  void shader_read_uniform_buffer(rid buffer, PipelineStages stages);
  void shader_read_image(rid image, PipelineStages stages);
  void shader_read_texel_buffer(rid buffer, PipelineStages stages);
  void shader_write_texel_buffer(rid buffer, PipelineStages stages);
  void shader_write_storage_buffer(rid buffer, PipelineStages stages);
  void render(rid renderpass, rid framebuffer);
  void present(rid image);
};

struct ScreenPassCtx
{
  extent      extent;
  ImageFormat format;
  bool        suboptimal  = false;        // updated by vulkan
  u32         num_buffers = 1;
};

struct ScreenPassResources
{
  rid color_images[16];        // screen has implicit pass to present the screen_color_image
  rid depth_stencil_images[16];
  rid render_passes[16];
  rid framebuffers[16];
};

struct ScreenPassBindings
{
  u32 image_index = 0;
};

struct ScreenPass
{
  ScreenPassCtx       ctx;
  ScreenPassResources resources;
  ScreenPassBindings  bindings;
};

struct ScreenPass;

struct GraphCtx
{
  ScreenPass screen_pass;
};

void onscreen_draw_pass(Graph &graph, GraphCtx &ctx)
{
  // RENDER
  // transition color attachment layout from presentation optimal to color attachment optimal
  //
  //
  // perform intermediate rendering operations
  //
  //
  // transition color attachment layout from color_attachment optimal to presentation optimal
  // THIS IS POINTLESSSS, it is on-screen
  // TODO(lamarrr): graph check?

  ASH_CHECK(ctx.screen_pass.ctx.num_buffers <= 16);

  for (u32 i = 0; i < ctx.screen_pass.ctx.num_buffers; i++)
  {
    ctx.screen_pass.resources.color_images[i]         = graph.create_image(ctx.screen_pass.ctx.format, ImageUsages::ColorAttachment, ctx.screen_pass.ctx.extent, 1);
    ctx.screen_pass.resources.depth_stencil_images[i] = graph.create_image(ImageFormat::D32_SFloat, ImageUsages::DepthStencilAttachment, ctx.screen_pass.ctx.extent, 1);
    ctx.screen_pass.resources.render_passes[i]        = graph.create_render_pass();
    ctx.screen_pass.resources.framebuffers[i]         = graph.create_frame_buffer(ctx.screen_pass.resources.render_passes[i], ctx.screen_pass.resources.color_images[i], ctx.screen_pass.resources.depth_stencil_images[i]);
  }
  // record render ops
}

void onscreen_draw_pass_update(Graph &graph, GraphCtx &ctx)
{
}

// to check if to recreate resources
// bool diff_context(Graph &graph, GraphCtx &ctx, OffscreenDrawPassContext const &prev_ctx, OffscreenDrawPassContext const &curr_ctx)
// {
// }

// if these changes, the resources need to be recreated
struct OffscreenDrawPassContext
{
  bool        depth_test_required = true;
  extent      area_extent;
  ImageFormat format;        // use sanity checks
};

struct OffscreenDrawPassResources
{
  rid color_attachment;                // output
  rid depth_stencil_attachment;        // output
  rid render_pass;
};

// bindings don't require changes to the resources, and can change for every task execution
// these are input or output bindings
struct OffscreenDrawPassBindings
{
};

// to check if to recreate resources
bool diff_context(Graph &graph, GraphCtx &ctx, OffscreenDrawPassContext const &prev_ctx, OffscreenDrawPassContext const &curr_ctx)
{
}

void offscreen_draw_pass(Graph &graph, GraphCtx const &ctx, OffscreenDrawPassContext const &args, OffscreenDrawPassResources &resources, OffscreenDrawPassBindings const &binding)
{
  // SETUP
  //
  // get the number maximum number of offscreen draw passes in the scene = N
  //
  // create N color output render targets with undefined layout
  // optionally create N depth stencil output render targets with undefined layout
  // left to the pipeline to determine the inputs???
  //
  if (args.depth_test_required)
  {
    resources.color_attachment         = graph.create_image(args.format, ImageUsages::ColorAttachment, args.area_extent, 1);
    resources.depth_stencil_attachment = graph.create_image(args.format, ImageUsages::DepthStencilAttachment, args.area_extent, 1);
    resources.render_pass              = graph.create_render_pass();
  }
  else
  {
    resources.color_attachment = graph.create_image(args.format, ImageUsages::ColorAttachment, args.area_extent, 1);
    resources.render_pass      = graph.create_render_pass();
  }

  //
  // for all N outputs insert barrier to convert from used or newly created layout to color attachment output layout
  //
  // for each N batch:
  //
  // for each z-sorted offscreen render pass:
  //
  //
  // RENDER
  // perform all intermediate rendering operations
  //
  // transition layout of color render target to shader read or transfer src or dst
  //
  // render to target
  // insert barrier to convert layout back to color attachment output
  //
  // we might want to leave the final image layout or state until completion of the pipeline as we don't know how exactly the will be used
  //
}

void clipped_draw_pass()
{
}

struct BlurCapturePassContext
{
  extent      blur_radius;
  extent      input_image_subregion_extent;
  ImageFormat input_image_format;
};

struct BlurCapturePassResources
{
  rid constants_buffer;
  rid sample_image;
  u32 sample_image_mip_levels = 1;
  rid sample_buffer;
  rid result_buffer;
};

struct BlurCapturePassBindings
{
  rid    input_image;
  u32    input_image_mip = 0;
  offset input_image_offset;
};

void blur_capture_pass(Graph &graph, GraphCtx const &ctx, BlurCapturePassContext const &args, BlurCapturePassBindings const &bindings, BlurCapturePassResources &resources)
{
  // SETUP
  // TODO(lamarrr): how do we handle alpha??
  ASH_CHECK(is_color_format(args.input_image_format));
  ASH_CHECK(args.input_image_subregion_extent.is_visible());

  resources.sample_image_mip_levels = std::min(args.input_image_subregion_extent.max_mip_levels(), 6U);
  extent downsampled_input_extent   = args.input_image_subregion_extent.at_mip_level(resources.sample_image_mip_levels - 1);
  resources.constants_buffer        = graph.create_buffer(args.blur_radius.area(), MemoryProperties::HostVisible, BufferUsages::UniformBuffer);
  resources.sample_image            = graph.create_image(args.input_image_format, ImageUsages::Sampled, args.input_image_subregion_extent, 6);
  resources.sample_buffer           = graph.create_buffer(downsampled_input_extent.area() * pixel_byte_size(args.input_image_format), MemoryProperties::DeviceLocal, BufferUsages::TransferDst | BufferUsages::TransferSrc | BufferUsages::StorageBuffer);
  resources.result_buffer           = graph.create_buffer(downsampled_input_extent.area() * pixel_byte_size(args.input_image_format), MemoryProperties::DeviceLocal, BufferUsages::TransferDst | BufferUsages::TransferSrc | BufferUsages::StorageBuffer);
  // resource initialization stage
  // resource pre-render setup
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
}

void bloom3d_pass()
{
  // SETUP
  // RENDER
}

void outline3d_pass()
{
  // SETUP
  // create depth attachment
  // RENDER
  // clear depth attachment
  // disable depth test and depth buffer
  // draw commands using colors only
  // enable depth test and depth buffer
  // draw object
}

void chromatic_aberration_pass()
{
  // https://www.shadertoy.com/view/Mds3zn
  // SETUP
  // RENDER
}

void effect_pass()
{
  // SETUP
  // RENDER
}

struct Pipeline
{};
struct ShaderMatrix
{};
// TODO(lamarrr): dynamic shaders / baking
struct DynamicShader
{
  // bindings
  // script
};

struct Blur
{
  int x = 20;
};

}        // namespace lgfx
}        // namespace ash
