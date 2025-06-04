/// SPDX-License-Identifier: MIT
#include "ashura/engine/input.h"

namespace ash
{

void KeyState::clear()
{
  focused  = false;
  in       = false;
  out      = false;
  any_down = false;
  any_up   = false;
  input    = false;
  text.clear();
  key_downs   = {};
  key_ups     = {};
  key_states  = {};
  scan_downs  = {};
  scan_ups    = {};
  scan_states = {};
  mod_downs   = {};
  mod_ups     = {};
  mod_states  = {};
}

KeyState & KeyState::copy(KeyState const & other)
{
  clear();
  focused  = other.focused;
  in       = other.in;
  out      = other.out;
  any_down = other.any_down;
  any_up   = other.any_up;
  input    = other.input;
  text.extend(other.text).unwrap();
  key_downs   = other.key_downs;
  key_ups     = other.key_ups;
  key_states  = other.key_states;
  scan_downs  = other.scan_downs;
  scan_ups    = other.scan_ups;
  scan_states = other.scan_states;
  mod_downs   = other.mod_downs;
  mod_ups     = other.mod_ups;
  mod_states  = other.mod_states;
  return *this;
}

void DropState::clear()
{
  event = Event::None;
  data.clear();
}

DropState & DropState::copy(DropState const & other)
{
  event = other.event;
  data.clear();
  data.extend(other.data).unwrap();
  return *this;
}

void InputState::stamp(time_point time, nanoseconds delta)
{
  timestamp = time;
  timedelta = delta;
}

void InputState::clear()
{
  timestamp = {};
  timedelta = {};
  window    = {};
  mouse     = {};
  theme     = {};
  key.clear();
  drop.clear();
}

InputState & InputState::copy(InputState const & other)
{
  timestamp = other.timestamp;
  timedelta = other.timedelta;
  window    = other.window;
  mouse     = other.mouse;
  theme     = other.theme;
  key.copy(other.key);
  drop.copy(other.drop);
  return *this;
}

}    // namespace ash
