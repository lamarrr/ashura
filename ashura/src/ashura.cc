#include "ashura/app.h"

int main() {
  asr::App app{asr::AppConfig{.enable_validation_layers = true}};

  app.tick({});
}