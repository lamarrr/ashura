/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/errors.h"
#include "ashura/std/log.h"
#include "ashura/std/types.h"
#include "ashura/std/vec.h"

namespace ash
{

enum class ShaderType : u32
{
  Compute  = 0,
  Vertex   = 1,
  Fragment = 2,
  Mesh     = 3
};

struct ShaderCompileInfo
{
  ShaderType type = ShaderType::Compute;

  Str path{};

  Str preamble{};

  Fn<void(LogLevel, Str)> on_log = noop;

  Fn<Option<Str>(Str)> on_load = [](Str) -> Option<Str> { return none; };

  Fn<void(Str)> on_drop = noop;
};

Result<Void, ShaderLoadErr> compile_shader(ShaderCompileInfo const & info,
                                           Vec<u32> &                spirv,
                                           Allocator                 allocator);

}    // namespace ash
