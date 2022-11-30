#include "ashura/app.h"

#if STX_CFG(OS, WINDOWS)
#define main SDL_main
#endif

int main() {
  asr::App app{asr::AppConfig{.enable_validation_layers = true}};

  app.tick({});

  return 0;
}
