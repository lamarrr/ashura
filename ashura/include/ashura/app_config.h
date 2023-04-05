#pragma once

#include "ashura/primitives.h"
#include "ashura/utils.h"
#include "ashura/version.h"
#include "ashura/window.h"
#include "stx/string.h"

namespace ash
{
using namespace stx::literals;

// refresh rate and various other settings
// log directories, file outputs, configuration files etc
struct AppConfig
{
  stx::String       name = "Ashura Engine"_str;
  Version           version{0, 0, 1};
  bool              enable_validation_layers = false;
  stx::String       log_file                 = "log.txt"_str;
  WindowType        root_window_type         = WindowType::Normal;
  WindowCreateFlags root_window_create_flags = WindowCreateFlags::None;
  ash::extent       root_window_extent;

  AppConfig copy() const
  {
    return AppConfig{.name                     = stx::string::make(stx::os_allocator, name).unwrap(),
                     .version                  = version,
                     .enable_validation_layers = enable_validation_layers,
                     .log_file                 = stx::string::make(stx::os_allocator, log_file).unwrap(),
                     .root_window_type         = root_window_type,
                     .root_window_create_flags = root_window_create_flags,
                     .root_window_extent       = root_window_extent};
  }
};

}        // namespace ash
