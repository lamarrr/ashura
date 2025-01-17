/// SPDX-License-Identifier: MIT
#include "ashura/engine/view.h"
#include "ashura/engine/color.h"

namespace ash
{
namespace ui
{

Theme theme = {
  .background        = {0x19, 0x19, 0x19, 0xFF},
  .surface           = {0x33, 0x33, 0x33, 0xFF},
  .surface_variant   = {0x5C, 0x5C, 0x5C, 0xFF},
  .primary           = mdc::DEEP_ORANGE_600,
  .primary_variant   = mdc::DEEP_ORANGE_400,
  .secondary         = mdc::PURPLE_600,
  .secondary_variant = mdc::PURPLE_400,
  .error             = mdc::RED_500,
  .warning           = mdc::YELLOW_800,
  .success           = mdc::GREEN_700,
  .active            = {0x70, 0x70, 0x70, 0xFF},
  .inactive          = {0x47, 0x47, 0x47, 0xFF},
  .on_background     = mdc::WHITE,
  .on_surface        = mdc::WHITE,
  .on_primary        = mdc::WHITE,
  .on_secondary      = mdc::WHITE,
  .on_error          = mdc::WHITE,
  .on_warning        = mdc::WHITE,
  .on_success        = mdc::WHITE,
  .focus             = mdc::BLUE_300,
  .head_font_height  = 30,
  .body_font_height  = 25,
  .line_height       = 1.2F,
  .focus_thickness   = 1
};

}
}    // namespace ash
