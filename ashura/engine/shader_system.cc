/// SPDX-License-Identifier: MIT
#include "ashura/engine/shader_system.h"
#include "ashura/engine/file_system.h"
#include "ashura/engine/gpu_system.h"
#include "ashura/engine/systems.h"

namespace ash
{

void IShaderSys::shutdown()
{
  while (!shaders_.is_empty())
  {
    unload(ShaderId{shaders_.to_id(0)});
  }
}

Result<ShaderInfo, ShaderLoadErr>
  IShaderSys::load_from_memory(Vec<char> label, Span<u32 const> spirv)
{
  gpu::Shader object =
    sys.gpu->device()
      ->create_shader(gpu::ShaderInfo{.label = label, .spirv_code = spirv})
      .unwrap();

  ShaderId id =
    ShaderId{shaders_.push(Shader{.label = std::move(label), .shader = object})
               .unwrap()};

  Shader & shader = shaders_[(usize) id].v0;
  shader.id       = id;

  return Ok{shader.view()};
}

Future<Result<ShaderInfo, ShaderLoadErr>>
  IShaderSys::load_from_path(Vec<char> label, Str path)
{
  Future load_fut = sys.file->load_file(allocator_, path);
  Future fut = future<Result<ShaderInfo, ShaderLoadErr>>(allocator_).unwrap();

  scheduler->once(
    [fut = fut.alias(), load_fut = load_fut.alias(), label = std::move(label),
     this]() mutable {
      load_fut.get().match(
        [&, label = std::move(label), this](Vec<u8> & spirv) mutable {
          scheduler->once(
            [fut = std::move(fut), spirv = std::move(spirv),
             label = std::move(label), this]() mutable {
              static_assert(spirv.alignment() >= alignof(u32));
              static_assert(std::endian::native == std::endian::little);
              fut
                .yield(load_from_memory(std::move(label),
                                        spirv.view().reinterpret<u32>()))
                .unwrap();
            },
            Ready{}, ThreadId::Main);
        },
        [&](IoErr err) {
          fut
            .yield(Err{err == IoErr::InvalidFileOrDir ?
                         ShaderLoadErr::InvalidPath :
                         ShaderLoadErr::IOErr})
            .unwrap();
        });
    },
    AwaitFutures{load_fut.alias()}, ThreadId::Main);

  return fut;
}

ShaderInfo IShaderSys::get(ShaderId id)
{
  CHECK(shaders_.is_valid_id((usize) id), "");
  return shaders_[(usize) id].v0.view();
}

Option<ShaderInfo> IShaderSys::get(Str label)
{
  for (auto [shader] : shaders_)
  {
    if (mem::eq(label, shader.label.view()))
    {
      return shader.view();
    }
  }

  return none;
}

void IShaderSys::unload(ShaderId id)
{
  Shader & shader = shaders_[(usize) id].v0;
  sys.gpu->plan()->add_preframe_task(
    [shader_h = shader.shader, dev = sys.gpu->device()] {
      dev->uninit(shader_h);
    });
  shaders_.erase((usize) id);
}

}    // namespace ash
