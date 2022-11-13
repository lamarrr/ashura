#pragma once

#include "ashura/primitives.h"
#include "ashura/utils.h"
#include "ashura/version.h"
#include "ashura/window.h"
#include "stx/string.h"

namespace asr {
using namespace stx::literals;

// refresh rate and various other settings
// log directories, file outputs, configuration files etc
struct AppConfig {
  stx::String name = "ashura Engine"_ss;
  Version version = Version{0, 0, 1};
  bool enable_validation_layers = false;
  stx::String log_file = "log.txt"_ss;
  WindowConfig window_config;
  // asset dir, shader dir, etc
};

}  // namespace asr