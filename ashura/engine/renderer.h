/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/canvas.h"
#include "ashura/engine/passes/bloom.h"
#include "ashura/engine/passes/blur.h"
#include "ashura/engine/passes/ngon.h"
#include "ashura/engine/passes/pbr.h"
#include "ashura/engine/passes/rrect.h"
#include "ashura/engine/render_context.h"
#include "ashura/std/hash_map.h"
#include "ashura/std/math.h"

namespace ash
{

typedef struct RenderPass_T *RenderPass;

struct RenderPassImpl
{
  RenderPass pass                                = nullptr;
  void (*init)(RenderPass p, RenderContext &r)   = nullptr;
  void (*uninit)(RenderPass p, RenderContext &r) = nullptr;
};

/// @brief sets up resources, pipelines, shaders, and data needed for rendering
/// the passes
struct PassContext
{
  BloomPass                  bloom  = {};
  BlurPass                   blur   = {};
  NgonPass                   ngon   = {};
  PBRPass                    pbr    = {};
  RRectPass                  rrect  = {};
  StrHashMap<RenderPassImpl> custom = {};

  void init(RenderContext &ctx);
  void uninit(RenderContext &ctx);
};

struct SSBO
{
  gfx::Buffer        buffer = nullptr;
  u64                size   = 0;
  gfx::DescriptorSet ssbo   = nullptr;

  void  uninit(RenderContext &ctx);
  void  reserve(RenderContext &ctx, u64 size, Span<char const> label);
  void  copy(RenderContext &ctx, Span<u8 const> src, Span<char const> label);
  void *map(RenderContext &ctx);
  void  unmap(RenderContext &ctx);
  void  flush(RenderContext &ctx);
};

struct CanvasResources
{
  SSBO vertices     = {};
  SSBO indices      = {};
  SSBO ngon_params  = {};
  SSBO rrect_params = {};

  void uninit(RenderContext &ctx);
};

struct CanvasRenderer
{
  CanvasResources resources[gfx::MAX_FRAME_BUFFERING];

  void init(RenderContext &ctx);
  void uninit(RenderContext &ctx);
  void begin(RenderContext &ctx, PassContext &passes, Canvas const &canvas,
             gfx::RenderingInfo const &info, gfx::DescriptorSet texture);
  void render(RenderContext &ctx, PassContext &passes,
              gfx::RenderingInfo const &info, gfx::Viewport const &viewport,
              gfx::Extent surface_extent, gfx::DescriptorSet texture,
              Canvas const &canvas, u32 first = 0, u32 num = U32_MAX);
};

struct PBRResources
{
  SSBO params = {};
  SSBO lights = {};

  void init(RenderContext &ctx);
  void uninit(RenderContext &ctx);
  void reserve(RenderContext &ctx, u32 num_objects, u32 num_lights);
};

struct PBRRenderer
{
  PBRResources resources[gfx::MAX_FRAME_BUFFERING];

  void init(RenderContext &ctx);
  void uninit(RenderContext &ctx);

  void begin(RenderContext &ctx, PassContext &passes,
             gfx::RenderingInfo const &info);
  void render(RenderContext &ctx, PassContext &passes,
              gfx::RenderingInfo const &info);
};

}        // namespace ash
