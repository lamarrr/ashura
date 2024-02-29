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
  u32 zZ_heap_index = -1;
};

// TODO(lamarrr): global deletion queue in render context?
// TODO(lamarrr): automatic uniform buffer setup?
// get buffer descriptionsm and batch all parameters together into a single
// buffer
//
template <Derives<ShaderParameterBase> Param>
struct ShaderParameterManager
{
  // no stalling
  void                init(u32 batch_size);
  void                deinit();
  void                flush_parameter(Param *parameter);
  void                remove_parameter(Param *parameter);
  gfx::DescriptorHeap get_heap();
  u32                 get_heap_index(Param *parameter);
};

struct Shader;

}        // namespace ash
