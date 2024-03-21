#include "ashura/engine/window.h"
#include "ashura/gfx/vulkan.h"

ash::Logger panic_logger;

int main()
{
  using namespace ash;
  // todo(lamarrr): fix logger interface
  StdioSink   sink;
  LogSinkImpl sink_impl{.self      = (LogSink) &sink,
                        .interface = &stdio_sink_interface};
  create_logger(&panic_logger, to_span({sink_impl}), heap_allocator);
  sdl_window_system->init();
  gfx::InstanceImpl instance =
      vk::instance_interface.create(heap_allocator, &panic_logger, true)
          .unwrap();
  u32 win = sdl_window_system->create_window(instance, "Main").unwrap();
}