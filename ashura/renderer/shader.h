#pragma once
#include "ashura/renderer/camera.h"
#include "ashura/renderer/light.h"
#include "ashura/renderer/render_graph.h"

namespace ash
{

// TODO(lamarrr): use shader setter and getter instead, get layout, get bindings
// TODO(lamarrr): !!!uniform buffer setup?????
struct ShaderParameterBase
{
  u32 zZ_heap_index         = -1;
  u32 zZ_uniform_heap_index = -1;
};

// This isn't feasible because we won't be able to use an existing shader
// parameter in another shader that accepts the same parameter type
template <typename T>
concept ShaderParameter = requires(T *param) {
  // TODO(lamarrr): make these optional? and make it possible to set the target
  // descriptor sets
  {
    static_cast<ShaderParameterBase *>(param)
  };
  {
    // all uniform data must be packed into one uniform buffer
    // uniforms are always in set 0, even if there is no uniform
    param->uniform
  };
  {
    param->samplers
  };
  {
    param->sampled_images
  };
  {
    param->combined_image_samplers
  };
  {
    param->storage_images
  };
  {
    param->uniform_buffers
  };
  {
    param->storage_buffers
  };
  {
    param->uniform_texel_buffers
  };
  {
    param->storage_texel_buffers
  };
};

struct ShaderParameterBinding
{
  gfx::DescriptorHeapImpl heap                  = {};
  u32                     heap_index            = 0;
  u64                     uniform_buffer_offset = 0;
};

// TODO(lamarrr): global deletion queue in render context?
// TODO(lamarrr): automatic uniform buffer setup?
// get buffer descriptionsm and batch all parameters together into a single
// buffer
//
template <ShaderParameter Param>
struct ShaderParameterManager
{
  // no stalling
  void                   init(u32 batch_size);
  void                   deinit();
  void                   flush_parameter(Param *parameter);
  void                   remove_parameter(Param *parameter);
  u32                    get_heap_index(Param *parameter);
  ShaderParameterBinding get_binding(Param *parameter);
};

struct Shader;

}        // namespace ash
