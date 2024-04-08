#include "ashura/engine/shader.h"
#include "ashura/engine/window.h"
#include "ashura/gfx/vulkan.h"
#include "ashura/renderer/render_context.h"
#include "ashura/renderer/renderer.h"
#include "ashura/renderer/shader.h"
#include "ashura/std/fs.h"
#include "stdlib.h"
#include <thread>

ash::Logger *default_logger;

namespace ash
{
// HOW TO STRUCTURE SHADER PACKS
// SPIRV will be the result of editor shader compilation
//
// ALL shaders are compiled to a pack with spirv
//
//
// we load these pre-compiled SPIRVs at load-time for dseployed builds, and
// for editior we load once changed
//
//
// other shaders should be able to include their own code and our default
// library code
//
//
// editor combines shader nodes into shaders
// at editor build time, the shaders are then compiled into spirv along and
// tagged. this goes into a shader pack with spirv code and ids load spirv and
// ids at runtime
//


struct ShaderEntry
{
  Span<char const> id;
  Span<char const> file;
  Span<char const> preamble;
};

ShaderCompileError pack_shader(StrHashMap<Vec<u32>> &compiled,
                               Span<char const>      id,
                               Span<char const>      root_directory,
                               Span<char const> file, Span<char const> preamble)
{
  ShaderType type = ShaderType::Compute;

  if (ends_with(file, ".comp"_span))
  {
    type = ShaderType::Compute;
  }
  else if (ends_with(file, ".frag"_span))
  {
    type = ShaderType::Fragment;
  }
  else if (ends_with(file, ".vert"_span))
  {
    type = ShaderType::Vertex;
  }
  else
  {
    CHECK(false);
  }

  Vec<char> file_path;
  defer     file_path_del{[&] { file_path.reset(); }};
  if (!file_path.extend_copy(root_directory) || !path_append(file_path, file))
  {
    return ShaderCompileError::OutOfMemory;
  }

  Vec<u32>           spirv;
  defer              spirv_del{[&] { spirv.reset(); }};
  ShaderCompileError error =
      compile_shader(*default_logger, spirv, to_span(file_path), type, preamble,
                     "main"_span, to_span({root_directory}), {});

  if (error != ShaderCompileError::None)
  {
    return error;
  }

  bool exists;
  if (!compiled.insert(exists, nullptr, id, spirv))
  {
    return ShaderCompileError::OutOfMemory;
  }
  CHECK(!exists);
  spirv = {};
  return ShaderCompileError::None;
}

ShaderCompileError pack_shaders(StrHashMap<Vec<u32>>   &compiled,
                                Span<ShaderEntry const> entries,
                                Span<char const>        root_directory)
{
  for (ShaderEntry const &entry : entries)
  {
    ShaderCompileError error = pack_shader(compiled, entry.id, root_directory,
                                           entry.file, entry.preamble);
    if (error != ShaderCompileError::None)
    {
      return error;
    }
  }
  return ShaderCompileError::None;
}

void load_shader_pack(StrHashMap<gfx::Shader> &shaders,
                      gfx::DeviceImpl const   &device);

}        // namespace ash

int main(int, char **)
{
  using namespace ash;
  StdioSink sink;
  default_logger = create_logger(to_span<LogSink *>({&sink}), heap_allocator);
  WindowSystem *win_sys = sdl_window_system;
  win_sys->init();
  gfx::InstanceImpl instance =
      vk::instance_interface.create(heap_allocator, default_logger, true)
          .unwrap();
  u32 win = win_sys->create_window(instance, "Main").unwrap();
  win_sys->maximize(win);
  win_sys->set_title(win, "Harro");
  bool should_close = false;
  auto close_fn     = [&](WindowEvent const &) { should_close = true; };
  win_sys->listen(win, WindowEventTypes::CloseRequested, to_fn_ref(close_fn));
  gfx::Surface    surface = win_sys->get_surface(win);
  gfx::DeviceImpl device =
      instance
          ->create_device(
              instance.self,
              to_span({gfx::DeviceType::DiscreteGpu, gfx::DeviceType::Cpu,
                       gfx::DeviceType::IntegratedGpu}),
              to_span({surface}), default_allocator)
          .unwrap();

  StrHashMap<Vec<u32>> spirv;

  CHECK(
      pack_shaders(
          spirv,
          to_span<ShaderEntry>(
              {{.id = "Glyph:FS"_span, .file = "glyph.frag"_span},
               {.id = "Glyph:VS"_span, .file = "glyph.vert"_span},
               {.id       = "KawaseBlur_UpSample:FS"_span,
                .file     = "kawase_blur.frag"_span,
                .preamble = "#define UPSAMPLE 1"_span},
               {.id       = "KawaseBlur_UpSample:VS"_span,
                .file     = "kawase_blur.vert"_span,
                .preamble = "#define UPSAMPLE 1"_span},
               {.id       = "KawaseBlur_DownSample:FS"_span,
                .file     = "kawase_blur.frag"_span,
                .preamble = "#define UPSAMPLE 0"_span},
               {.id       = "KawaseBlur_DownSample:VS"_span,
                .file     = "kawase_blur.vert"_span,
                .preamble = "#define UPSAMPLE 0"_span},
               {.id = "PBR:FS"_span, .file = "pbr.frag"_span},
               {.id = "PBR:VS"_span, .file = "pbr.vert"_span},
               {.id = "RRect:FS"_span, .file = "rrect.frag"_span},
               {.id = "RRect:VS"_span, .file = "rrect.vert"_span}}),
          "C:/Users/rlama/Documents/workspace/oss/ashura/ashura/shaders/"_span) ==
      ShaderCompileError::None)

  StrHashMap<gfx::Shader> shaders;

  spirv.for_each([&](Span<char const> id, Vec<u32> &spirv) {
    bool exists;
    CHECK(shaders.insert(
        exists, nullptr, id,
        device
            ->create_shader(
                device.self,
                gfx::ShaderDesc{.label = nullptr, .spirv_code = to_span(spirv)})
            .unwrap()));
    CHECK(!exists);
  });

  default_logger->info("compiled shaders");
  default_logger->flush();

  Renderer renderer;
  renderer.init(device, nullptr, shaders, {});

  while (!should_close)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    win_sys->poll_events();
  }
}