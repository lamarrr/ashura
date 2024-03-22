#include "ashura/engine/window.h"
#include "ashura/gfx/vulkan.h"
#include "ashura/renderer/render_context.h"
#include "ashura/renderer/renderer.h"
#include "ashura/renderer/shader.h"
#include <thread>

ash::Logger *panic_logger;

int main()
{
  using namespace ash;
  StdioSink sink;
  panic_logger = create_logger(to_span<LogSink *>({&sink}), heap_allocator);
  WindowSystem *win_sys = sdl_window_system;
  win_sys->init();
  gfx::InstanceImpl instance =
      vk::instance_interface.create(heap_allocator, panic_logger, true)
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
      make_functor_fn(cb));
  // win_sys->unlisten(win, listener);
  gfx::Surface surface = win_sys->get_surface(win);

  gfx::DeviceImpl device =
      instance
          ->create_device(
              instance.self,
              to_span({gfx::DeviceType::DiscreteGpu, gfx::DeviceType::Cpu}),
              to_span({surface}), default_allocator)
          .unwrap();

  Renderer renderer;
  renderer.init();
  UniformHeap heap;
  heap.init(device);
  heap.push(32);

  while (true)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    win_sys->poll_events();
  }
}