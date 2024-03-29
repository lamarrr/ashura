#pragma once

#include "ashura/renderer/renderer.h"
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

struct ShaderSource
{
  Span<char const> file        = {};
  ShaderType       type        = ShaderType::Compute;
  Span<char const> preamble    = {};
  Span<char const> entry_point = "main"_span;
};

// how to merge error types?
enum class ShaderLoadError : i32
{
  None        = 0,
  InvalidPath = 1,
  Unknown     = 3,
  // IoFailed=,
  LinkFailed,
  CompileFailed,
  SpirvConversionFailed,
  OutOfMemory,
};

void            reflect_spirv(Span<u32 const> spirv);
ShaderLoadError load_shader(Logger &logger, Vec<u32> &spirv,
                            ShaderSource const          &source,
                            Span<Span<char const> const> system_directories,
                            Span<Span<char const> const> local_directories);

}        // namespace ash
