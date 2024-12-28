/// SPDX-License-Identifier: MIT
#pragma once

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

enum class ShaderLoadErr : u32
{
  OutOfMemory           = 0,
  InvalidPath           = 1,
  IOErr                 = 2,
  CompileFailed         = 3,
  LinkFailed            = 4,
  SpirvConversionFailed = 5,
  InitErr               = 6
};

struct ShaderCompileInfo
{
  ShaderType                                     type = ShaderType::Compute;
  Span<char const>                               path{};
  Span<char const>                               preamble{};
  Fn<void(LogLevel, Span<char const>)>           on_log{};
  Fn<Option<Span<char const>>(Span<char const>)> on_load{};
  Fn<void(Span<char const>)>                     on_drop{};
};

Result<Void, ShaderLoadErr> compile_shader(ShaderCompileInfo const & info,
                                           Vec<u32> &                spirv,
                                           AllocatorRef              allocator);

}    // namespace ash
