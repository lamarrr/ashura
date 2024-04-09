#include "ashura/engine/shader.h"
#include "ashura/engine/window.h"
#include "ashura/gfx/vulkan.h"
#include "ashura/renderer/render_context.h"
#include "ashura/renderer/renderer.h"
#include "ashura/renderer/shader.h"
#include "stdlib.h"
#include <thread>

ash::Logger *default_logger;

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

  Vec<Tuple<Span<char const>, Vec<u32>>> spirvs;

  CHECK(
      pack_shaders(
          spirvs,
          to_span<ShaderPackEntry>(
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

  for (auto &[id, spirv] : spirvs)
  {
    bool exists;
    CHECK(shaders.insert(
        exists, nullptr, id,
        device
            ->create_shader(
                device.self,
                gfx::ShaderDesc{.label = nullptr, .spirv_code = to_span(spirv)})
            .unwrap()));
    CHECK(!exists);
  }

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