/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/gpu_system.h"
#include "ashura/gpu/gpu.h"
#include "ashura/std/types.h"

namespace ash
{

enum class BlendMode : u32
{
  Clear      = 0,
  Src        = 1,
  Dst        = 2,
  SrcOver    = 3,
  DstOver    = 4,
  SrcIn      = 5,
  DstIn      = 6,
  SrcOut     = 7,
  DstOut     = 8,
  SrcAtop    = 9,
  DstAtop    = 10,
  Xor        = 11,
  Plus       = 12,
  Modulate   = 13,
  Screen     = 14,
  Overlay    = 15,
  Darken     = 16,
  Lighten    = 17,
  ColorDodge = 18,
  ColorBurn  = 19,
  HardLight  = 20,
  SoftLight  = 21,
  Difference = 22,
  Exclusion  = 23,
  Multiply   = 24,
  Hue        = 25,
  Saturation = 26,
  Color      = 27,
  Luminosity = 28
};

/// @brief Passes are re-usable and stateless compute and graphics pipeline
/// components. They set up static resources: pipelines, shaders, and render
/// data needed for executing rendering operations. Passes dispatch
/// compute/graphics shaders using their specified arguments. They are mostly
/// used by renderers.
struct Pass
{
  virtual Str  label()   = 0;
  virtual void acquire() = 0;
  virtual void release() = 0;
  virtual ~Pass()        = default;
};

struct PassStencil
{
  gpu::StencilState front = {};
  gpu::StencilState back  = {};
};

}    // namespace ash
