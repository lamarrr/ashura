#define SDL_MAIN_HANDLED
#include "SDL2/SDL.h"
#include "ashura/app.h"

int main(int, char*[]) {
  SDL_SetMainReady();
  asr::App app{asr::AppConfig{.enable_validation_layers = true}};

  while (true) app.tick({});

  return 0;
}
