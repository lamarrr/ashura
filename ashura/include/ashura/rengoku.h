#pragma once

#include "ashura/image.h"
#include "ashura/primitives.h"

namespace ash
{
namespace rgk
{
struct vertex
{
  vec3 position;
  vec2 uv;
  vec3 normal;
  vec4 color;
};

struct AttachmentSpec
{
  u32 load_op;
  u32 store_op;
  u32 stencil_load_op;
  u32 stencil_store_op;
  u32 format;
};

// struct RenderSubpassSpec
// {
// };

struct RenderPassSpec
{
  // input,output depth attachments format
  // input,output color attachments format
  AttachmentSpec color_attachment;
  AttachmentSpec depth_attachment;
  AttachmentSpec depth_stencil_attachment;
};

// static and ideally shouldn't change
struct RenderPipelineSpec
{
  // push constant size => 128
  // uniform buffer layout
  char const *id;
  char const *vertex_shader_src;
  char const *fragment_shader_src;
  u32         render_pass;
};

// static and ideally shouldn't change
struct ComputePipelineSpec
{
  // push constant size => 128
  // uniform buffer layout
  char const *id;
  char const *compute_shader_src;
  u32         render_pass;
};

// has to be re-constructed everytime the attachments change
struct FrameBufferSpec
{
  // render pass
  char const *id;
  gfx::image  depth_attachment;
  gfx::image  color_attachment;
  extent      extent;
};

struct ComputePassSpec
{
  char const *id;
  char const *compute_shader_src;
};

struct PbrMaterial
{
  gfx::image albedo            = 0;
  gfx::image normal            = 0;
  gfx::image metalic           = 0;
  gfx::image roughness         = 0;
  gfx::image ambient_occlusion = 0;
  gfx::image emissive          = 0;
};

struct blur_effect
{
  extent offset;
};

// Perlin noise for cloud generation
// standard character mesh with bones
// vulkan full screen exclusive

// bloom
// fog
// chromatic aberration
// depth of field
// shadow mapping
// particle effects
// bokeh effect
// hdr support
// global illumination

}        // namespace rgk
}        // namespace ash
