#pragma once

#include "asura/primitives.h"
#include "asura/utils.h"
#include "asura/version.h"
#include "asura/window.h"
#include "stx/string.h"

namespace asr {

// refresh rate and various other settings
// log directories, file outputs, configuration files etc
struct AppConfig {
  stx::String name = stx::string::make_static("Asura Engine");
  Version version = Version{0, 0, 1};
  bool enable_validation_layers = false;
  stx::String log_file = stx::string::make_static("log.txt");
  WindowConfig window_config;
  // asset dir, shader dir, etc
};

}  // namespace asr