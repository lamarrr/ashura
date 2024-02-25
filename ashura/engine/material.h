#pragma once
#include "ashura/engine/camera.h"
#include "ashura/engine/light.h"
#include "ashura/engine/render_graph.h"

namespace ash
{

enum class ShaderParamType : u8
{
  None                 = 0,
  Buffer               = 1,
  TexelBuffer          = 2,
  Sampler              = 3,
  SampledImage         = 4,
  CombinedImageSampler = 5,
};

struct ShaderParamDesc
{
  Span<char const> name = {};
  ShaderParamType  type = ShaderParamType::None;
};

template <typename T>
struct UniformBuffer
{
  using Type = T;
};

template <typename T>
struct StorageBuffer
{
  using Type = T;
};

struct ShaderParam
{
  Span<char const> name = {};
  ShaderParamType  type = ShaderParamType::None;
  union
  {
    char                             none_ = 0;
    gfx::Buffer                      buffer;
    gfx::BufferView                  texel_buffer;
    gfx::Sampler                     sampler;
    gfx::ImageView                   image;
    gfx::CombinedImageSamplerBinding combined_image_sampler;
  };
};

struct ShaderPipelineDesc
{
  Span<char const> name            = {};
  Span<char const> fragment_shader = {};
  Span<char const> vertex_shader   = {};
  bool             has_depth : 1   = true;
  bool             has_stencil : 1 = true;
  bool             alpha_blend : 1 = true;
};

// constant
struct Material
{
  Span<ShaderParam const> parameters  = {};
  ShaderPipelineDesc      shader_desc = {};
};

struct MaterialManager
{
  void update_parameter(uid32 material, u32 parameter);
  // validate against shader
  uid32 create_material();
  // batch
  // sort
};

}        // namespace ash
