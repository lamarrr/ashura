#pragma once
#include "ashura/renderer/passes/bloom.h"
#include "ashura/renderer/passes/blur.h"
#include "ashura/renderer/passes/fxaa.h"
#include "ashura/renderer/passes/msaa.h"
#include "ashura/renderer/passes/pbr.h"
#include "ashura/renderer/passes/quad.h"
#include "ashura/renderer/passes/rrect.h"
#include "ashura/renderer/render_context.h"

namespace ash
{

struct RenderPasses
{
  // sky render pass
  // render 3d scene pass + custom shaders (pipeline + fragment + vertex shader)
  // perform bloom, blur, msaa on 3d scene
  // render UI pass + custom shaders, blur ???
  // copy and composite 3d and 2d scenes
  RenderContext ctx;
  BloomPass     bloom;
  BlurPass      blur;
  FXAAPass      fxaa;
  MSAAPass      msaa;
  PBRPass       pbr;
  QuadPass      quad;
  RRectPass     rrect;
};

struct Renderer
{
  RenderPasses passes;

  void begin_frame();
  void record_frame();
  void end_frame();
};

}        // namespace ash
