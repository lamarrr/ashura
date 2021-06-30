#pragma once

#include "vlk/ui/primitives.h"

namespace vlk {
namespace ui {

enum class MouseButton : uint8_t {
  Primary,
  Secondary,
  Middle,
  A1,
  A2,
  A3,
  A4,
  A5
};

enum class MouseID : uint32_t {};

enum class MouseAction : uint8_t {
  Press,
  Release,
};

struct MouseMotionEvent {
  MouseID mouse_id{};
  IOffset offset;
  IOffset translation;
};

struct MouseButtonEvent {
  MouseID mouse_id{};
  IOffset offset;
  uint32_t clicks = 0;
  MouseButton button = MouseButton::Primary;
  MouseAction action = MouseAction::Press;
};

}  // namespace ui
}  // namespace vlk
