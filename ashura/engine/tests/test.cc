#include "ashura/engine/window.h"

ash::Logger panic_logger;

int main()
{
  using namespace ash;
  sdl_window_system->init();
  u32 win = sdl_window_system->create_window("Main").unwrap();
}