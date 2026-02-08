/// SPDX-License-Identifier: MIT
#include "ashura/engine/input.h"

namespace ash
{

Result<KeyState> KeyState::copy(Allocator allocator) const
{
    KeyState out{allocator};
    out.focused_     = focused_;
    out.in_          = in_;
    out.out_         = out_;
    out.any_down_    = any_down_;
    out.any_up_      = any_up_;
    out.input_       = input_;
    out.mod_downs_   = mod_downs_;
    out.mod_ups_     = mod_ups_;
    out.mod_states_  = mod_states_;
    out.key_downs_   = key_downs_;
    out.key_ups_     = key_ups_;
    out.key_states_  = key_states_;
    out.scan_downs_  = scan_downs_;
    out.scan_ups_    = scan_ups_;
    out.scan_states_ = scan_states_;

    if (text_.is_some())
    {
        ASH_TRY(copy, vec::copy(allocator, text_.v0_.view()));
        out.text_ = std::move(copy);
    }

    return Ok{std::move(out)};
}

void KeyState::start_frame()
{
    focused_     = false;
    in_          = false;
    out_         = false;
    any_down_    = false;
    any_up_      = false;
    input_       = false;
    mod_downs_   = {};
    mod_ups_     = {};
    mod_states_  = {};
    key_downs_   = {};
    key_ups_     = {};
    key_states_  = {};
    scan_downs_  = {};
    scan_ups_    = {};
    scan_states_ = {};
    text_        = none;
}

MouseState MouseState::copy() const
{
    MouseState out;
    out.focused_           = focused_;
    out.in_                = in_;
    out.out_               = out_;
    out.moved_             = moved_;
    out.scrolled_          = scrolled_;
    out.any_down_          = any_down_;
    out.any_up_            = any_up_;
    out.downs_             = downs_;
    out.ups_               = ups_;
    out.states_            = states_;
    out.num_clicks_        = num_clicks_;
    out.position_          = position_;
    out.translation_       = translation_;
    out.wheel_translation_ = wheel_translation_;
    return out;
}

void MouseState::start_frame()
{
    focused_           = false;
    in_                = false;
    out_               = false;
    moved_             = false;
    scrolled_          = false;
    any_down_          = false;
    any_up_            = false;
    downs_             = {};
    ups_               = {};
    states_            = {};
    num_clicks_        = {};
    position_          = none;
    translation_       = none;
    wheel_translation_ = none;
}

ThemeState ThemeState::copy() const
{
    ThemeState out;
    out.changed_ = changed_;
    out.theme_   = theme_;
    return out;
}

void ThemeState::start_frame()
{
    changed_ = false;
}

Result<DropState> DropState::copy(Allocator allocator) const
{
    DropState out{allocator};

    out.active_ = active_;
    out.event_  = event_;

    auto ok = data_.match([](None) { return true; },
                          [&](DropFilePath const & e) {
                              auto r = vec::copy(allocator, e.path.view());
                              if (!r)
                              {
                                  return false;
                              }
                              out.data_ = DropFilePath{.path = r.unwrap()};
                              return true;
                          },
                          [&](DropText const & e) {
                              auto r = vec::copy(allocator, e.text.view());
                              if (!r)
                              {
                                  return false;
                              }
                              out.data_ = DropText{.text = r.unwrap()};
                              return true;
                          });

    if (!ok)
    {
        return Err{};
    }

    out.position_ = position_;

    return Ok{std::move(out)};
}

void DropState::start_frame()
{
    switch (event_)
    {
        case Event::Complete:
        {
            active_   = false;
            data_     = none;
            position_ = none;
            event_    = Event::None;
        }
        break;

        default:
        {
            event_ = Event::None;
        }
        break;
    }
}

SystemState SystemState::copy() const
{
    SystemState out;
    out.timestamp_ = timestamp_;
    out.timedelta_ = timedelta_;
    out.theme_     = theme_.copy();
    return out;
}

void SystemState::start_frame(time_point time, nanoseconds delta)
{
    timestamp_ = time;
    timedelta_ = delta;
    theme_.start_frame();
}

Result<WindowState> WindowState::copy(Allocator allocator) const
{
    WindowState out{allocator};
    out.extent_           = extent_;
    out.surface_extent_   = surface_extent_;
    out.shown_            = shown_;
    out.hidden_           = hidden_;
    out.exposed_          = exposed_;
    out.moved_            = moved_;
    out.resized_          = resized_;
    out.surface_resized_  = surface_resized_;
    out.minimized_        = minimized_;
    out.maximized_        = maximized_;
    out.restored_         = restored_;
    out.close_requested_  = close_requested_;
    out.occluded_         = occluded_;
    out.enter_fullscreen_ = enter_fullscreen_;
    out.leave_fullscreen_ = leave_fullscreen_;

    auto key_r = key_.copy(allocator);

    if (!key_r)
    {
        return Err{};
    }

    out.key_ = key_r.unwrap();

    out.mouse_ = mouse_.copy();

    auto drop_r = drop_.copy(allocator);

    if (!drop_r)
    {
        return Err{};
    }

    out.drop_ = drop_r.unwrap();

    return Ok{std::move(out)};
}

void WindowState::start_frame()
{
    extent_           = {};
    surface_extent_   = {};
    shown_            = false;
    hidden_           = false;
    exposed_          = false;
    moved_            = false;
    resized_          = false;
    surface_resized_  = false;
    minimized_        = false;
    maximized_        = false;
    restored_         = false;
    close_requested_  = false;
    occluded_         = false;
    enter_fullscreen_ = false;
    leave_fullscreen_ = false;
    key_.start_frame();
    mouse_.start_frame();
    drop_.start_frame();
}

}    // namespace ash
