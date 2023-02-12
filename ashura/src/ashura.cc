#define SDL_MAIN_HANDLED
#include "SDL2/SDL.h"
#include "ashura/app.h"

int main(int, char*[]) {
  SDL_SetMainReady();
  ash::AppConfig cfg{.enable_validation_layers = true};
  cfg.window_config.borderless = false;
  ash::App app{std::move(cfg)};

  while (true) app.tick({});

  return 0;
}
