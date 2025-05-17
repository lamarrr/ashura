/// SPDX-License-Identifier: MIT
#include "ashura/engine/view.h"
#include "ashura/engine/color.h"

namespace ash
{
namespace ui
{

Theme theme = {
  .background      = {0x19,                           0x19,                           0x19, 0xFF},
  .surface         = {0x33,                           0x33,                           0x33, 0xFF},
  .surface_variant = {0x5C,                           0x5C,                           0x5C, 0xFF},
  .primary         = ios::accessible::DARK_INDIGO,
  .primary_variant = ios::accessible::LIGHT_INDIGO,
  .error           = mdc::RED_500,
  .warning         = mdc::YELLOW_800,
  .success         = mdc::GREEN_700,
  .active          = {0x70,                           0x70,                           0x70, 0xFF},
  .inactive        = {0x47,                           0x47,                           0x47, 0xFF},
  .on_background   = mdc::WHITE,
  .on_surface      = mdc::WHITE,
  .on_primary      = mdc::WHITE,
  .on_error        = mdc::WHITE,
  .on_warning      = mdc::WHITE,
  .on_success      = mdc::WHITE,
  .focus           = mdc::BLUE_300,
  // [ ] fix
  .highlight = {ios::accessible::DARK_INDIGO.x, ios::accessible::DARK_INDIGO.w,
                      ios::accessible::DARK_INDIGO.z,                                       128 },
  .caret     = mdc::WHITE,
  .head_font_height = 30,
  .body_font_height = 25,
  .line_height      = 1.2F
};

void DropCtx::clear()
{
  phase = Phase::None;
  type  = DropType::None;
  data.clear();
}

DropCtx & DropCtx::copy(DropCtx const & other)
{
  phase = other.phase;
  type  = other.type;
  data.clear();
  data.extend(other.data).unwrap();
  return *this;
}

void Ctx::tick(InputState const & input)
{
  timestamp = input.timestamp;
  timedelta = input.timedelta;
  mouse     = input.mouse;
  key.copy(input.key);

  // if the there was a data drop on the last frame clear the buffer
  switch (input.drop.event)
  {
    case DropState::Event::None:
    {
      drop.phase = DropCtx::Phase::None;
      drop.type  = DropType::None;
    }
    break;
    case DropState::Event::Begin:
    {
      drop.data.clear();
      drop.phase = DropCtx::Phase::Begin;
      drop.type  = DropType::None;
    }
    break;
    case DropState::Event::FilePath:
    {
      drop.data.extend(input.drop.data).unwrap();
      drop.phase = DropCtx::Phase::Over;
      drop.type  = DropType::FilePath;
    }
    break;
    case DropState::Event::Bytes:
    {
      drop.data.extend(input.drop.data).unwrap();
      drop.phase = DropCtx::Phase::Over;
      drop.type  = DropType::Bytes;
    }
    break;
    case DropState::Event::End:
    {
      drop.data.clear();
      drop.phase = DropCtx::Phase::End;
      drop.type  = DropType::None;
    }
    break;
  }

  closing = closing || input.window.close_requested;
  focused = none;
  cursor  = none;
}

}    // namespace ui
}    // namespace ash
