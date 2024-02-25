#pragma once
#include "ashura/engine/camera.h"
#include "ashura/engine/light.h"
#include "ashura/engine/render_graph.h"

namespace ash
{

enum class ShaderBindingType : u8
{
  None                 = 0,
  Buffer               = 1,
  TexelBuffer          = 2,
  Sampler              = 3,
  SampledImage         = 4,
  CombinedImageSampler = 5,
};

// struct ShaderParamDesc
// {
//   Span<char const> name = {};
//   ShaderBindingType  type = ShaderBindingType::None;
// };

template <typename T>
struct UniformBuffer
{
  typedef T   Type;
  gfx::Buffer buffer;
};

template <typename T>
struct StorageBuffer
{
  typedef T   Type;
  gfx::Buffer buffer;
};

struct SampleShaderParam
{
  Vec4 a;
  Vec2 b;
  f32  x;
  f32  y;
};

// TODO(lamarrr): use shader setter and getter instead, get layout, get bindings

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
