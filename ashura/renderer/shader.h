#pragma once
#include "ashura/renderer/camera.h"
#include "ashura/renderer/light.h"
#include "ashura/renderer/render_graph.h"

namespace ash
{

// TODO(lamarrr): use shader setter and getter instead, get layout, get bindings
// Param::desc();
// Param::update();
// Param::BATCH_SIZE
// Param::TYPE_NAME
// uniform buffer setup?????

struct ShaderParameterInterface
{
  Span<gfx::DescriptorBindingDesc const> (*get_desc)();
  void (*update)(gfx::DescriptorHeapImpl const &heap, u32 group, u32 set,
                 void *);
  // bind descriptors to pipeline
};

struct ShaderParameterManager
{
  template <typename Param>
  uid32 create_parameter(Param *parameter);
  template <typename Param>        // no stalling
  void update_parameter(Param *parameter);
  void remove_parameter(uid32 parameter);
};

struct Shader;

}        // namespace ash
