/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/errors.hpp"
#include "ashura/engine/systems.hpp"
#include "ashura/gpu/gpu.hpp"
#include "ashura/std/async.hpp"
#include "ashura/std/types.hpp"

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

    Vec<char> label;

    gpu::Shader shader = nullptr;

    constexpr ShaderInfo view() const
    {
        return ShaderInfo{.id = id, .label = label, .shader = shader};
    }
};

struct IShaderSys
{
    Allocator                   allocator_;
    IRWSpinLock                 rw_lock_;
    SparseVec<ShaderId, Shader> shaders_;
    GpuSys                      gpu_sys_;
    FileSys                     file_sys_;

    IShaderSys(GpuSys gpu_sys, FileSys file_sys) :
      allocator_{noop_allocator},
      rw_lock_{},
      shaders_{noop_allocator},
      gpu_sys_{gpu_sys},
      file_sys_{file_sys}
    {
    }

    IShaderSys(IShaderSys const &)             = delete;
    IShaderSys(IShaderSys &&)                  = delete;
    IShaderSys & operator=(IShaderSys const &) = delete;
    IShaderSys & operator=(IShaderSys &&)      = delete;
    ~IShaderSys()                              = default;

    AwaitFuturesVec init(Allocator allocator);

    void shutdown();

    Result<ShaderId, SysErr> load_from_memory(Str label, Span<u32 const> spirv);

    Future<Result<ShaderId, SysErr>> load_from_path(Str label, Str path);

    ShaderInfo get(ShaderId id);

    Option<ShaderInfo> get(Str label);

    void unload_(ShaderId);

    /// @warning Resources are not reference counted. Holding on to their info
    /// structs after unloading them will be catastrophic
    void unload(ShaderId);
};

}    // namespace ash
