/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/errors.h"
#include "ashura/gpu/gpu.h"
#include "ashura/std/async.h"
#include "ashura/std/types.h"

namespace ash
{

enum class ShaderId : u64
{
  None = U64_MAX
};

struct ShaderInfo
{
  ShaderId id{};

  Str label{};

  gpu::Shader shader = nullptr;
};

struct Shader
{
  ShaderId id{};

  Vec<char> label{};

  gpu::Shader shader = nullptr;

  constexpr ShaderInfo view() const
  {
    return ShaderInfo{.id = id, .label = label, .shader = shader};
  }
};

typedef struct IShaderSys * ShaderSys;

struct IShaderSys
{
  Allocator         allocator_;
  SparseVec<Shader> shaders_;

  IShaderSys(Allocator allocator) : allocator_{allocator}, shaders_{allocator}
  {
  }

  IShaderSys(IShaderSys const &)             = delete;
  IShaderSys(IShaderSys &&)                  = default;
  IShaderSys & operator=(IShaderSys const &) = delete;
  IShaderSys & operator=(IShaderSys &&)      = default;
  ~IShaderSys()                              = default;

  void shutdown();

  Result<ShaderInfo, ShaderLoadErr> load_from_memory(Vec<char>       label,
                                                     Span<u32 const> spirv);

  Future<Result<ShaderInfo, ShaderLoadErr>> load_from_path(Vec<char> label,
                                                           Str       path);

  ShaderInfo get(ShaderId id);

  Option<ShaderInfo> get(Str label);

  void unload(ShaderId);
};

}    // namespace ash
