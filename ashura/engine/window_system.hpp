/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/input.hpp"
#include "ashura/gpu/gpu.h"
#include "ashura/std/dyn.hpp"
#include "ashura/std/image.hpp"
#include "ashura/std/math.hpp"
#include "ashura/std/option.hpp"
#include "ashura/std/result.hpp"
#include "ashura/std/types.hpp"

namespace ash
{

typedef struct IWindow * Window;

typedef struct IWindowSys * WindowSys;

enum class WindowListenerId : usize
{
    Undefined = USIZE_MAX
};

struct IWindowSys
{
    static Dyn<WindowSys> create_SDL(Allocator allocator);

    virtual ~IWindowSys() = default;

    virtual void shutdown() = 0;

    virtual Option<IWindow &> create_window(gpu::Instance instance, Str title) = 0;

    virtual void uninit_window(Window window) = 0;

    virtual void set_title(Window window, Str title) = 0;

    virtual char const * get_title(Window window) = 0;

    virtual void maximize(Window window) = 0;

    virtual void minimize(Window window) = 0;

    virtual void set_extent(Window window, u32x2 extent) = 0;

    virtual void center(Window window) = 0;

    virtual u32x2 get_extent(Window window) = 0;

    virtual u32x2 get_surface_extent(Window window) = 0;

    virtual void set_position(Window window, i32x2 pos) = 0;

    virtual i32x2 get_position(Window window) = 0;

    virtual void set_min_extent(Window window, u32x2 min) = 0;

    virtual u32x2 get_min_extent(Window window) = 0;

    virtual void set_max_extent(Window window, u32x2 max) = 0;

    virtual u32x2 get_max_extent(Window window) = 0;

    virtual void set_icon(Window window, ImageSpan<u8 const, 4> image,
                          gpu::Format format) = 0;

    virtual void make_bordered(Window window) = 0;

    virtual void make_borderless(Window window) = 0;

    virtual void show(Window window) = 0;

    virtual void hide(Window window) = 0;

    virtual void raise(Window window) = 0;

    virtual void restore(Window window) = 0;

    virtual void request_attention(Window window, bool briefly) = 0;

    virtual void make_fullscreen(Window window) = 0;

    virtual void make_windowed(Window window) = 0;

    virtual void make_resizable(Window window) = 0;

    virtual void make_unresizable(Window window) = 0;

    virtual WindowListenerId listen(Fn<void(SystemEvent const &)> callback) = 0;

    virtual WindowListenerId listen(Window                        window,
                                    Fn<void(WindowEvent const &)> callback) = 0;

    virtual void unlisten(Window window, WindowListenerId listener) = 0;

    virtual Result<> set_hit_test(Window window, Fn<WindowRegion(u32x2)> hit) = 0;

    virtual gpu::Surface get_surface(Window window) = 0;

    virtual SystemTheme get_theme() = 0;

    virtual void poll_events() = 0;

    virtual ClipBoard get_clipboard() = 0;

    virtual Tuple<KeyModifiers, Option<IWindow &>>
      get_keyboard_state(BitSpan<u64> scan_state, BitSpan<u64> key_state) = 0;

    virtual Tuple<MouseButtons, f32x2, Option<IWindow &>> get_mouse_state() = 0;

    virtual void set_text_input(Window window, Option<TextInputInfo> info) = 0;

    virtual void set_text_input_area(Window window, RectU const & rect,
                                     i32 cursor_position) = 0;

    virtual void set_cursor(Option<Cursor> cursor) = 0;

    virtual void lock_cursor(Window window, bool lock) = 0;
};

}    // namespace ash
