#pragma once
#include "ashura/renderer/camera.h"
#include "ashura/renderer/passes/rrect.h"
#include "ashura/renderer/render_graph.h"

namespace ash
{

// custom passes/ shaders?, pbr, i.e. dilating objects, glowing objects with noise texture input
struct UIObject
{
  void *params;
  void (*pass)(RenderGraph *graph, void *);
};

struct UIParams
{
  Span<UIObject const> objects;
};

struct UIPass
{
  static void add_pass(RenderGraph *graph, UIParams const *params);
};

}        // namespace ash
