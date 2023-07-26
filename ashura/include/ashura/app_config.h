#pragma once

#include "ashura/font.h"
#include "ashura/pipeline.h"
#include "ashura/primitives.h"
#include "ashura/utils.h"
#include "ashura/version.h"
#include "ashura/window.h"
#include "stx/c_string_view.h"
#include "stx/string.h"

namespace ash
{
using namespace stx::literals;

// refresh rate and various other settings
// log directories, file outputs, configuration files etc
struct AppConfig
{
  stx::CStringView                    name = "Ashura Engine";
  Version                             version{0, 0, 1};
  bool                                enable_validation_layers = false;
  stx::Span<FontSpec const>           fonts;
  stx::Span<CanvasPipelineSpec const> pipelines;
  stx::CStringView                    log_file                 = "log.txt";
  WindowType                          root_window_type         = WindowType::Normal;
  WindowCreateFlags                   root_window_create_flags = WindowCreateFlags::None;
  ash::extent                         root_window_extent{1920, 1080};
};

}        // namespace ash
