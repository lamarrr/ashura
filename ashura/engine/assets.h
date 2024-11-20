
/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/font.h"
#include "ashura/gpu/gpu.h"
#include "ashura/std/map.h"
#include "ashura/std/vec.h"

namespace ash
{

typedef StrMap<gpu::Shader> ShaderMap;

typedef StrMap<Dyn<Font *>> FontMap;

// [ ] images

struct AssetMap
{
  explicit AssetMap(AllocatorImpl allocator) :
      shaders{allocator}, fonts{allocator}
  {
  }

  ShaderMap shaders;
  FontMap   fonts;
};

}        // namespace ash
