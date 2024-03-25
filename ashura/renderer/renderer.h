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

// sky render pass
// render 3d scene pass + custom shaders (pipeline + fragment + vertex shader)
// perform bloom, blur, msaa on 3d scene
// render UI pass + custom shaders, blur ???
// copy and composite 3d and 2d scenes
struct RenderPasses
{
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

  void init(gfx::DeviceImpl device, gfx::PipelineCache pipeline_cache,
            StrHashMap<gfx::Shader> shader_map, u32 max_frames_in_flight = 2)
  {
    ctx.device         = device;
    ctx.shader_map     = shader_map;
    ctx.pipeline_cache = pipeline_cache;
    // select formats
    // init uniform heaps vec
    // init uniform layout
    // init frame_info
    // init frame context
    // init scratch attachments
    //
    // provided we might not want the renderer to be display-dependent, we have
    // to ensure the color and depth stencil scratch images are always large
    // enough
    init_passes();
  }

  void init_passes()
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
