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

struct ShaderDefine
{
  Span<char const> id    = {};
  i32              value = 0;
};

struct ShaderDecl
{
  Span<char const>         id      = {};
  Span<char const>         file    = {};
  ShaderType               type    = ShaderType::Compute;
  Span<ShaderDefine const> defines = {};
};

int load_shaders(StrHashMap<gfx::Shader> &shader_map,
                 Span<ShaderDecl const>   shaders);

}        // namespace ash
