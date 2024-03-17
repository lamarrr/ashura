#pragma once
#include "ashura/renderer/passes/bloom.h"
#include "ashura/renderer/passes/blur.h"
#include "ashura/renderer/passes/custom.h"
#include "ashura/renderer/passes/fxaa.h"
#include "ashura/renderer/passes/msaa.h"
#include "ashura/renderer/passes/pbr.h"
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
  BloomPass        bloom;
  BlurPass         blur;
  FXAAPass         fxaa;
  MSAAPass         msaa;
  PBRPass          pbr;
  CustomShaderPass custom;
  RRectPass        rrect;
};

struct Renderer
{
  RenderPasses  passes;
  RenderContext ctx;

  void init()
  {
    passes.bloom.init(ctx);
    passes.blur.init(ctx);
    passes.fxaa.init(ctx);
    passes.msaa.init(ctx);
    passes.pbr.init(ctx);
    passes.custom.init(ctx);
    passes.rrect.init(ctx);
  }

  void uninit()
  {
    passes.bloom.uninit(ctx);
    passes.blur.uninit(ctx);
    passes.fxaa.uninit(ctx);
    passes.msaa.uninit(ctx);
    passes.pbr.uninit(ctx);
    passes.custom.uninit(ctx);
    passes.rrect.uninit(ctx);
  }

  void begin_frame();
  void record_frame();
  void end_frame();
};

}        // namespace ash
