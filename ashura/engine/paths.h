#pragma once

#include "ashura/std/types.h"

namespace ash
{

struct Paths
{
  Span<char const> shaders_dir;
  Span<char const> shader_spirv_cache_dir;
  Span<char const> pipeline_cache_dir;
  Span<char const> font_cache_dir;
};

}        // namespace ash