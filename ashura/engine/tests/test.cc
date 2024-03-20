#include "ashura/engine/window.h"

int main()
{
  using namespace ash;
  sdl_window_system->init();
  u32 win = sdl_window_system->create_window("Main").unwrap();
  while (true)
  {
    /* code */
  }
}