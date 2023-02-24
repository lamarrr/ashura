#pragma once

#include "ashura/primitives.h"
#include "ashura/utils.h"
#include "ashura/version.h"
#include "ashura/window.h"
#include "stx/string.h"

namespace ash {
using namespace stx::literals;

// refresh rate and various other settings
// log directories, file outputs, configuration files etc
struct AppConfig {
  stx::String name = "ashura Engine"_str;
  Version version{0, 0, 1};
  bool enable_validation_layers = false;
  stx::String log_file = "log.txt"_str;
  WindowConfig window_config;

  AppConfig copy() const {
    return AppConfig{
        .name = stx::string::make(stx::os_allocator, name).unwrap(),
        .version = version,
        .enable_validation_layers = enable_validation_layers,
        .log_file = stx::string::make(stx::os_allocator, log_file).unwrap(),
        .window_config = window_config.copy()};
  }
};

}  // namespace ash
