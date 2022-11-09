#pragma once

#include "ashura/primitives.h"
#include "ashura/utils.h"
#include "ashura/version.h"
#include "ashura/window.h"
#include "stx/string.h"

namespace asr {

// refresh rate and various other settings
// log directories, file outputs, configuration files etc
struct AppConfig {
  stx::String name = stx::string::make_static("ashura Engine");
  Version version = Version{0, 0, 1};
  bool enable_validation_layers = false;
  stx::String log_file = stx::string::make_static("log.txt");
  WindowConfig window_config;
  // asset dir, shader dir, etc
};

}  // namespace asr