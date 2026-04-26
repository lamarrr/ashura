/// SPDX-License-Identifier: MIT
#include "ashura/engine/shader_system.hpp"
#include "ashura/engine/file_system.hpp"
#include "ashura/engine/gpu_system.hpp"
#include "ashura/engine/systems.hpp"
#include "ashura/std/trace.hpp"

namespace ash
{

AwaitFuturesVec IShaderSys::init(Allocator allocator)
{
    tracing::ScopeTrace trace;

    static constexpr u8 BEZIER_STENCIL_SHADER[] = {
#embed "shaders/bezier_stencil.spv"
    };

    static constexpr u8 BLUR_DOWNSAMPLE_SHADER[] = {
#embed "shaders/blur_downsample.spv"
    };

    static constexpr u8 BLUR_UPSAMPLE_SHADER[] = {
#embed "shaders/blur_upsample.spv"
    };

    static constexpr u8 COMPOSITE_SDF_SHADER[] = {
#embed "shaders/composite_sdf.spv"
    };

    static constexpr u8 FILL_STENCIL_SHADER[] = {
#embed "shaders/fill_stencil.spv"
    };

    static constexpr u8 PBR_BASE_SHADER[] = {
#embed "shaders/pbr_base.spv"
    };

    static constexpr u8 QUAD_SHADER[] = {
#embed "shaders/quad_base.spv"
    };

    static constexpr u8 SDF_GRADIENT_SHADER[] = {
#embed "shaders/sdf_gradient.spv"
    };

    static constexpr u8 SDF_NOISE_SHADER[] = {
#embed "shaders/sdf_noise.spv"
    };

    static constexpr u8 SDF_MESH_GRADIENT_SHADER[] = {
#embed "shaders/sdf_mesh_gradient.spv"
    };

    static constexpr u8 TRIANGLE_FILL_SHADER[] = {
#embed "shaders/triangle_fill.spv"
    };

    static constexpr u8 VECTOR_PATH_COVERAGE_SHADER[] = {
#embed "shaders/vector_path_coverage.spv"
    };

    static constexpr u8 VECTOR_PATH_FILL_SHADER[] = {
#embed "shaders/vector_path_fill.spv"
    };

    allocator_ = allocator;
    shaders_   = SparseVec<ShaderId, Shader>{allocator_};

    Vec<AnyFuture> load_futs{allocator_};

    Tuple<Str, Span<u8 const>> const shaders[] = {
      {"defaults/bezier_stencil"_s,       BEZIER_STENCIL_SHADER      },
      {"defaults/blur_downsample"_s,      BLUR_DOWNSAMPLE_SHADER     },
      {"defaults/blur_upsample"_s,        BLUR_UPSAMPLE_SHADER       },
      {"defaults/composite_sdf"_s,        COMPOSITE_SDF_SHADER       },
      {"defaults/fill_stencil"_s,         FILL_STENCIL_SHADER        },
      {"defaults/pbr_base"_s,             PBR_BASE_SHADER            },
      {"defaults/quad_base"_s,            QUAD_SHADER                },
      {"defaults/sdf_gradient"_s,         SDF_GRADIENT_SHADER        },
      {"defaults/sdf_noise"_s,            SDF_NOISE_SHADER           },
      {"defaults/sdf_mesh_gradient"_s,    SDF_MESH_GRADIENT_SHADER   },
      {"defaults/triangle_fill"_s,        TRIANGLE_FILL_SHADER       },
      {"defaults/vector_path_coverage"_s, VECTOR_PATH_COVERAGE_SHADER},
      {"defaults/vector_path_fill"_s,     VECTOR_PATH_FILL_SHADER    }
    };

    for (auto [label, spirv] : shaders)
    {
        load_futs
          .push(scheduler_
                  ->run(allocator_, WorkerThread::Any,
                        [label, spirv, this]() mutable -> Result<ShaderId, SysErr> {
                            return load_from_memory(label,
                                                    spirv.reinterpret<u32 const>());
                        })
                  .unwrap())
          .unwrap();
    }

    return AwaitFuturesVec{std::move(load_futs)};
}

void IShaderSys::shutdown()
{
    WriteGuard guard{rw_lock_};
    while (!shaders_.is_empty())
    {
        unload_(shaders_.to_id(0));
    }
}

Result<ShaderId, SysErr> IShaderSys::load_from_memory(Str             label_span,
                                                      Span<u32 const> spirv)
{
    tracing::ScopeTrace trace;

    Vec<char> label{allocator_};
    label.append(label_span).unwrap();
    gpu::Shader object =
      gpu_sys_->device()
        ->create_shader(gpu::ShaderInfo{.label = label, .spirv_code = spirv})
        .unwrap();

    {
        WriteGuard guard{rw_lock_};
        ShaderId   id = ShaderId{
          shaders_.push(Shader{.label = std::move(label), .shader = object}).unwrap()};

        return Ok{id};
    }
}

Future<Result<ShaderId, SysErr>> IShaderSys::load_from_path(Str label_span, Str path)
{
    StrVec label{allocator_};
    label.append(label_span).unwrap();

    return scheduler_
      ->then(
        allocator_, WorkerThread::Any,
        [label = std::move(label), this](Result<Vec<u8>, SysErr> & file_r) {
            using R = Result<ShaderId, SysErr>;
            return file_r.match(
              [&, this](Vec<u8> & spirv) -> R {
                  static_assert(spirv.alignment() >= alignof(u32));
                  static_assert(__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__);
                  return load_from_memory(label, spirv.view().reinterpret<u32>());
              },
              [](SysErr err) -> R { return Err{err}; });
        },
        file_sys_->load_file(allocator_, path))
      .unwrap();
}

ShaderInfo IShaderSys::get(ShaderId id)
{
    ReadGuard guard{rw_lock_};
    ASH_CHECK(shaders_.is_valid_id(id), "");
    return shaders_[id].v0.view();
}

Option<ShaderInfo> IShaderSys::get(Str label)
{
    ReadGuard guard{rw_lock_};
    for (auto [shader] : shaders_)
    {
        if (mem::eq(label, shader.label.view()))
        {
            return shader.view();
        }
    }

    return none;
}

void IShaderSys::unload_(ShaderId id)
{
    Shader & shader = shaders_[id].v0;
    gpu_sys_->current_plan()->add_preframe_task(
      [shader_h = shader.shader, dev = gpu_sys_->device()](GpuFrame) {
          dev->uninit(shader_h);
      });
    shaders_.erase(id);
}

void IShaderSys::unload(ShaderId id)
{
    WriteGuard guard{rw_lock_};
    unload_(id);
}

}    // namespace ash
