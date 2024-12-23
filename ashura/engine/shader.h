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

enum class ShaderCompileError : i32
{
  OutOfMemory           = 0,
  IOError               = 1,
  CompileFailed         = 2,
  LinkFailed            = 3,
  SpirvConversionFailed = 4,
  InitError             = 5
};

struct ShaderCompileInfo
{
  ShaderType                                     type = ShaderType::Compute;
  Span<char const>                               file{};
  Span<char const>                               preamble{};
  Fn<void(LogLevels, Span<char const>)>          on_log{};
  Fn<Option<Span<char const>>(Span<char const>)> on_load{};
  Fn<void(Span<char const>)>                     on_drop{};
};

Result<Void, ShaderCompileError> compile_shader(ShaderCompileInfo const & info,
                                                Vec<u32> &                spirv,
                                                AllocatorImpl allocator);

}    // namespace ash
