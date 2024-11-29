/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/renderer.h"
#include "ashura/std/types.h"

namespace ash
{

enum class ShaderType : u8
{
  Compute  = 0,
  Vertex   = 1,
  Fragment = 2,
  Mesh     = 3
};

enum class ShaderCompileError : i32
{
  None                  = 0,
  OutOfMemory           = 1,
  IOError               = 2,
  CompileFailed         = 3,
  LinkFailed            = 4,
  SpirvConversionFailed = 5,
  InitError             = 6
};

ShaderCompileError
    compile_shader(Logger & logger, Vec<u32> & spirv, Span<char const> file,
                   ShaderType type, Span<char const> preamble,
                   Span<char const>             entry_point,
                   Span<Span<char const> const> system_directories,
                   Span<Span<char const> const> local_directories);

struct ShaderUnit
{
  Span<char const> id       = {};
  Span<char const> file     = {};
  Span<char const> preamble = {};
};

ShaderCompileError
    pack_shaders(Vec<Tuple<Span<char const>, Vec<u32>>> & compiled,
                 Span<ShaderUnit const>                   entries,
                 Span<char const>                         root_directory);

}        // namespace ash
