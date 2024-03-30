#include "ashura/engine/shader.h"
#include "ashura/engine/window.h"
#include "ashura/gfx/vulkan.h"
#include "ashura/renderer/render_context.h"
#include "ashura/renderer/renderer.h"
#include "ashura/renderer/shader.h"
#include <thread>

ash::Logger *default_logger;

namespace ash
{

void compile_shaders();
void pack_shaders();
void load_shader_pack();

void load_shaders(StrHashMap<gfx::Shader> &shaders,
                  gfx::DeviceImpl const &device, Span<char const> id)
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
  Vec<u32> spirv;
  defer    spirv_del{[&] { spirv.reset(); }};
  CHECK(
      compile_shader(
          *default_logger, spirv,
          R"(C:\Users\rlama\Documents\workspace\oss\ashura\ashura\shaders\canvas.vert)"_span,
          ShaderType::Vertex, "#define RRERS 20338"_span, "main"_span,
          to_span(
              {R"(C:\Users\rlama\Documents\workspace\oss\ashura\ashura\shaders\modules)"_span}),
          {}) == ShaderCompileError::None);

  gfx::Shader shader =
      device
          ->create_shader(
              device.self,
              gfx::ShaderDesc{.label = nullptr, .spirv_code = to_span(spirv)})
          .unwrap();

  bool exists;
  CHECK(shaders.insert(exists, nullptr, id, shader));
  CHECK(!exists);
}
}        // namespace ash

int main()
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
  auto cb = [&](WindowEvent const &e) {
    if (e.type == WindowEventTypes::Key)
    {
      printf("pressed key: %d\n ", (int) e.key.key);
    }
    else
    {
      // printf("mouse motion: %f, %f\n", e.mouse_motion.position.x,
      // e.mouse_motion.position.y);
    }
  };
  u32 listener = win_sys->listen(
      win, WindowEventTypes::Key | WindowEventTypes::MouseMotion,
      to_fn_ref(cb));
  // win_sys->unlisten(win, listener);
  gfx::Surface surface = win_sys->get_surface(win);

  gfx::DeviceImpl device =
      instance
          ->create_device(
              instance.self,
              to_span({gfx::DeviceType::Cpu, gfx::DeviceType::IntegratedGpu,
                       gfx::DeviceType::DiscreteGpu}),
              to_span({surface}), default_allocator)
          .unwrap();

  StrHashMap<gfx::Shader> shaders;
  Vec<u32>                spirv;
  defer                   spirv_del{[&] { spirv.reset(); }};
  CHECK(
      compile_shader(
          *default_logger, spirv,
          R"(C:\Users\rlama\Documents\workspace\oss\ashura\ashura\shaders\canvas.vert)"_span,
          ShaderType::Vertex, "#define RRERS 20338"_span, "main"_span,
          to_span(
              {R"(C:\Users\rlama\Documents\workspace\oss\ashura\ashura\shaders\modules)"_span}),
          {}) == ShaderCompileError::None);

  device
      ->create_shader(
          device.self,
          gfx::ShaderDesc{.label = "Kawase", .spirv_code = to_span(spirv)})
      .unwrap();

  Renderer renderer;
  default_logger->info("initing renderer");
  default_logger->flush();
  renderer.init(device, nullptr, {}, {});

  default_logger->info("Polling");
  default_logger->flush();
  while (true)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    win_sys->poll_events();
  }
}