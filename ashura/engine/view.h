/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/canvas.h"
#include "ashura/engine/event.h"
#include "ashura/engine/mime.h"
#include "ashura/engine/renderer.h"
#include "ashura/engine/text.h"
#include "ashura/std/math.h"
#include "ashura/std/time.h"
#include "ashura/std/types.h"

namespace ash
{

/// @brief Simple Adaptive Layout Constraint Model
/// @param offset adding or subtracting from the source size, i.e. value should
/// be source size - 20px
/// @param scale scales the source size, i.e. value should be 0.5 of source
/// size
/// @param min clamps the source size, i.e. value should be at least 20px
/// @param max clamps the source size, i.e. value should be at most 100px
/// @param rmin  clamps the source size relatively. i.e. value should be at
/// least 0.5 of source size
/// @param rmax  clamps the source size relatively. i.e. value should be at
/// most 0.5 of source size
struct SizeConstraint
{
  f32 offset = 0;
  f32 scale  = 0;
  f32 min    = F32_MIN;
  f32 max    = F32_MAX;
  f32 rmin   = 0;
  f32 rmax   = 1;

  constexpr f32 resolve(f32 value) const
  {
    return clamp(clamp(offset + value * scale, min, max), rmin * value,
                 rmax * value);
  }
};

enum class MainAlign : u8
{
  Start        = 0,
  End          = 1,
  SpaceBetween = 2,
  SpaceAround  = 3,
  SpaceEvenly  = 4
};

/// @param DragStart drag event has begun on this view
/// @param DragUpdate the mouse has been moved whilst this view is being
/// dragged
/// @param DragEnd the dragging of this view has completed
/// @param DragEnter drag data has entered this view and might be dropped
/// @param DragLeave drag data has left the view without being dropped
/// @param Drop drag data is now available for the view to consume
/// @param ViewHit called on every frame the view is viewed on the viewport.
/// Can be used for partial loading
/// @param ViewMiss called on every frame that the view is not seen on the
/// viewport this can be because it has hidden visibility, is clipped away, or
/// parent positioned out of the visible region. Can be used for
/// partial unloading.
/// @param FocusIn the view has received keyboard focus
/// @param FocusOut the view has lost keyboard focus
/// @param TextInput the view has received composition text
enum class ViewEventTypes : u64
{
  None         = 0x0000000000000000ULL,
  MouseDown    = 0x0000000000000001ULL,
  MouseUp      = 0x0000000000000002ULL,
  MousePressed = 0x0000000000000004ULL,
  MouseMove    = 0x0000000000000008ULL,
  MouseEnter   = 0x0000000000000010ULL,
  MouseEscaped = 0x0000000000000020ULL,
  MouseLeave   = 0x0000000000000040ULL,
  MouseScroll  = 0x0000000000000080ULL,
  DragStart    = 0x0000000000000100ULL,
  DragUpdate   = 0x0000000000000200ULL,
  DragEnd      = 0x0000000000000400ULL,
  DragEnter    = 0x0000000000000800ULL,
  DragOver     = 0x0000000000001000ULL,
  DragLeave    = 0x0000000000002000ULL,
  Drop         = 0x0000000000004000ULL,
  ViewHit      = 0x0000000000008000ULL,
  ViewMiss     = 0x0000000000010000ULL,
  FocusIn      = 0x0000000000020000ULL,
  FocusOut     = 0x0000000000040000ULL,
  KeyDown      = 0x0000000000080000ULL,
  KeyUp        = 0x0000000000100000ULL,
  KeyPressed   = 0x0000000000200000ULL,
  TextInput    = 0x0000000000400000ULL
};

ASH_DEFINE_ENUM_BIT_OPS(ViewEventTypes)

/// @param Visible if the view is visible or not. Visibility propagates down
/// to the children
/// @param Draggable if the view can receive drag events
/// @param Droppable if the view can receive drop events
/// @param Focusable can receive view focus events (typically keyboard events)
/// @param TextArea receives text input and not just Keyboard press/release
enum class ViewAttributes : u32
{
  None       = 0x00000000U,
  Visible    = 0x00000001U,
  Clickable  = 0x00000002U,
  Scrollable = 0x00000004U,
  Draggable  = 0x00000008U,
  Droppable  = 0x00000010U,
  Focusable  = 0x00000020U,
  TextArea   = 0x00000040U
};

ASH_DEFINE_ENUM_BIT_OPS(ViewAttributes)

/// @brief It is important that widgets only access data from
/// context objects and not some global state to ensure portability and
/// adaptiveness, i.e. during hot-reloading. The App context may also contain
/// references to other subsystems.
/// @param type used to recognize the payload, should be used to check if the
/// data matches the expected content/type.
/// @param data the user context data for this application
/// @param next link to the next attached context.
struct AppContext
{
  Span<u8 const> type = {};
  AppContext    *next = nullptr;
  void          *data = nullptr;
};

/// @brief Global View Context, Properties of the context all the widgets for
/// a specific window are in.
/// @param has_focus the current view scope (window) has focus
/// @param button current button states
/// @param drag_payload attached drag and drop payload data
/// @param theme the current theme from the UI system
/// @param direction the text direction of the host system
/// @param key_states bit array of the key states (indexed by keycode)
/// @param scan_states bit array of the key states (indexed by scancode)
/// @param text current text input data from the IME (keyboard, TTS, virtual
/// keyboard, etc)
struct ViewContext
{
  AppContext              *app                     = nullptr;
  bool                     has_focus               = false;
  MouseButtons             mouse_buttons           = MouseButtons::None;
  Vec2                     mouse_position          = {};
  Vec2                     mouse_translation       = {};
  u32                      num_clicks              = 0;
  Vec2                     mouse_wheel_translation = {};
  Span<u8 const>           drag_payload            = {};
  SystemTheme              theme                   = SystemTheme::None;
  TextDirection            direction               = TextDirection::LeftToRight;
  u64                      key_states[NUM_KEYS / 64]  = {};
  u64                      scan_states[NUM_KEYS / 64] = {};
  Span<u32 const>          text                       = {};
  steady_clock::time_point time_stamp                 = {};
  nanoseconds              time_delta                 = {};

  virtual void change_cursor(Cursor cursor) const
  {
    (void) cursor;
  }

  virtual Cursor get_cursor() const
  {
    return Cursor::None;
  }

  constexpr bool key_down(KeyCode key) const
  {
    u16 const i     = (u32) key;
    u64       state = key_states[i >> 6];
    state           = (state >> (i & 63)) & 1;
    return state != 0;
  }

  constexpr bool scan_down(ScanCode key) const
  {
    u16 const i     = (u32) key;
    u64       state = scan_states[i >> 6];
    state           = (state >> (i & 63)) & 1;
    return state != 0;
  }

  virtual Span<u8 const> get_clipboard_data(Span<char const> mime) const
  {
    (void) mime;
    return {};
  }

  virtual void set_clipboard_data(Span<char const> mime,
                                  Span<u8 const>   data) const
  {
    (void) mime;
    (void) data;
  }

  Span<u8 const> get_clipboard_text() const
  {
    return get_clipboard_data(span(MIME_TEXT_UTF8));
  }

  void set_clipboard_text(Span<char const> text) const
  {
    set_clipboard_data(span(MIME_TEXT_UTF8), text.as_u8());
  }
};

/// @brief Base view class. All view types must inherit from this struct.
/// Views are plain visual elements that define spatial relationships,
/// visual state changes, and forward events to other subsystems.
/// @note State changes should only happen in the `tick` method
struct View
{
  uid id = UID_MAX;

  View()                        = default;
  View(View const &)            = default;
  View(View &&)                 = default;
  View &operator=(View const &) = default;
  View &operator=(View &&)      = default;
  virtual ~View()               = default;

  /// @brief get child widgets, this is a virtual iterator, return null once
  /// there's no other children
  /// @param i child index
  /// @return child view pointer or nullptr meaning no more child left.
  virtual View *child(u32 i)
  {
    (void) i;
    return nullptr;
  }

  /// @brief distributes the size allocated to it to its child widgets.
  /// @param allocated the size allocated to this view
  /// @param[out] sizes sizes allocated to the children.
  virtual void size(Vec2 allocated, Span<Vec2> sizes)
  {
    (void) allocated;
    fill(sizes, Vec2{0, 0});
  }

  /// @brief fits itself around its children and positions child widgets
  /// relative to its center
  /// @param allocated the size allocated to this view
  /// @param sizes sizes of the child widgets
  /// @param[out] offsets offsets of the widgets from the parent's center
  /// @return this view's fitted extent
  virtual Vec2 fit(Vec2 allocated, Span<Vec2 const> sizes, Span<Vec2> offsets)
  {
    (void) allocated;
    (void) sizes;
    fill(offsets, Vec2{0, 0});
    return Vec2{0, 0};
  }

  /// @brief this is used for absolute positioning of the view
  /// @param center the allocated absolute center of this view relative
  /// to the viewport
  virtual Vec2 position(CRect const &region)
  {
    return region.center;
  }

  /// @brief Used for hit-testing regions of widgets.
  /// @param area area of view within the viewport
  /// @param offset offset of pointer within area
  /// @return
  virtual bool hit(CRect const &region, Vec2 offset)
  {
    (void) region;
    (void) offset;
    return true;
  }

  /// @brief Used for visibility, scroll, and drag testing
  /// @return
  virtual ViewAttributes attributes()
  {
    return ViewAttributes::Visible;
  }

  virtual Cursor cursor(CRect const &region, Vec2 offset)
  {
    (void) region;
    (void) offset;
    return Cursor::Default;
  }

  /// @brief returns the z-index of itself and assigns z-indices to its children
  /// @param z_index z-index allocated to this view by parent
  /// @param[out] allocation z-index assigned to children
  /// @return
  virtual i32 stack(i32 z_index, Span<i32> allocation)
  {
    fill(allocation, z_index + 1);
    return z_index;
  }

  /// @brief Tab Index for Focus-Based Navigation.
  /// @return desired tab index, 0 meaning the default tab order based on the
  /// hierarchy of the parent to children and siblings. negative values have
  /// higher tab index priority while positive indices have lower tab priority.
  virtual i32 tab()
  {
    return 0;
  }

  /// @brief this is used for clipping views. the provided clip is
  /// relative to the root viewport. Used for nested viewports where there are
  /// multiple intersecting clips.
  virtual CRect clip(CRect const &region, CRect const &allocated)
  {
    (void) region;
    return allocated;
  }

  /// @brief record draw commands needed to render this view. this method is
  /// only called if the view passes the visibility tests. this is called on
  /// every frame.
  /// @param canvas
  virtual void render(CRect const &region, Canvas &canvas)
  {
    (void) region;
    (void) canvas;
  }

  /// @brief called on every frame. used for state changes, animations, task
  /// dispatch and lightweight processing related to the GUI. heavy-weight and
  /// non-sub-millisecond tasks should be dispatched to a Subsystem that would
  /// handle that. i.e. using the multi-tasking system.
  /// @param dt time passed since last call to this method
  //
  virtual void tick(ViewContext const &ctx, CRect const &region,
                    ViewEventTypes events)
  {
    (void) ctx;
    (void) region;
    (void) events;
  }
};

template <usize N>
constexpr View *child_iter(View *const (&children)[N], u32 i)
{
  return i < N ? children[i] : nullptr;
}

constexpr View *child_iter(Span<View *const> children, u32 i)
{
  return i < children.size() ? children[i] : nullptr;
}

}        // namespace ash
