/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/canvas.h"
#include "ashura/engine/color.h"
#include "ashura/engine/input.h"
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
/// @param rmin  clamps the source size relatively. i.e. value should be at
/// least 0.5 of source size
/// @param rmax  clamps the source size relatively. i.e. value should be at
/// most 0.5 of source size
/// @param min clamps the source size, i.e. value should be at least 20px
/// @param max clamps the source size, i.e. value should be at most 100px
struct Size
{
  f32 offset = 0;
  f32 scale  = 0;
  f32 rmin   = 0;
  f32 rmax   = 1;
  f32 min    = F32_MIN;
  f32 max    = F32_MAX;

  constexpr f32 operator()(f32 value) const
  {
    return clamp(clamp(offset + value * scale, rmin * value, rmax * value), min,
                 max);
  }
};

struct Frame
{
  Size width  = {};
  Size height = {};

  constexpr Vec2 operator()(Vec2 extent) const
  {
    return Vec2{width(extent.x), height(extent.y)};
  }
};

struct CornerRadii
{
  Size top_left     = Size{};
  Size top_right    = Size{};
  Size bottom_left  = Size{};
  Size bottom_right = Size{};

  static constexpr CornerRadii all(Size s)
  {
    return CornerRadii{s, s, s, s};
  }

  constexpr Vec4 operator()(f32 height) const
  {
    return Vec4{top_left(height), top_right(height), bottom_left(height),
                bottom_right(height)};
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

/// @param mounted view has been mounted to the view tree and has now
/// received an ID.
/// @param drag_start drag event has begun on this view
/// @param dragging an update on the drag state has been gotten
/// @param drag_end the dragging of this view has completed
/// @param drag_in drag data has entered this view and might be dropped
/// @param drag_out drag data has left the view without being dropped
/// @param drag_over drag data is moving over this view as destination without
/// beiung dropped
/// @param drop drag data is now available for the view to consume
/// @param view_hit called on every frame the view is viewed on the viewport.
/// Can be used for partial loading
/// @param view_miss called on every frame that the view is not seen on the
/// viewport this can be because it has hidden visibility, is clipped away, or
/// parent positioned out of the visible region. Can be used for
/// partial unloading.
/// @param focus_in the view has received keyboard focus
/// @param focus_out the view has lost keyboard focus
/// @param text_input the view has received composition text
struct ViewEvents
{
  bool mounted : 1      = false;
  bool view_hit : 1     = false;
  bool mouse_in : 1     = false;
  bool mouse_out : 1    = false;
  bool mouse_down : 1   = false;
  bool mouse_up : 1     = false;
  bool mouse_moved : 1  = false;
  bool mouse_scroll : 1 = false;
  bool drag_start : 1   = false;
  bool dragging : 1     = false;
  bool drag_end : 1     = false;
  bool drag_in : 1      = false;
  bool drag_out : 1     = false;
  bool drag_over : 1    = false;
  bool drop : 1         = false;
  bool focus_in : 1     = false;
  bool focus_out : 1    = false;
  bool key_down : 1     = false;
  bool key_up : 1       = false;
  bool text_input : 1   = false;
};

/// @brief Global View Context, Properties of the context all the views for
/// a specific window are in.
/// @param globals It is important that views only access data from
/// global/system objects and not some global state to ensure portability and
/// adaptiveness, i.e. during hot-reloading where they might be attached to a
/// different context. The ViewGlobals may also contain references to other
/// subsystems.
/// @param focused the current view scope (window) has focus
/// @param button current button states
/// @param drag_payload attached drag and drop payload data
/// @param theme the current theme from the UI system
/// @param direction the text direction of the host system
/// @param key_states bit array of the key states (indexed by keycode)
/// @param scan_states bit array of the key states (indexed by scancode)
/// @param text, text_utf32 current text input data from the IME or keyboard
/// @param timestamp timestamp of current frame
/// @param timedelta time elapsed between previous and current frame
struct ViewContext
{
  struct Mouse
  {
    bool         in : 1             = false;
    bool         out : 1            = false;
    bool         focused : 1        = false;
    bool         moved : 1          = false;
    bool         wheel_scrolled : 1 = false;
    MouseButtons downs : 8          = MouseButtons::None;
    MouseButtons ups : 8            = MouseButtons::None;
    MouseButtons states : 8         = MouseButtons::None;
    u32          num_clicks         = 0;
    Vec2         position           = {};
    Vec2         translation        = {};
    Vec2         wheel_translation  = {};
  };

  struct KeyBoard
  {
    bool                down : 1    = false;
    bool                up : 1      = false;
    Bits<u64, NUM_KEYS> downs       = {};
    Bits<u64, NUM_KEYS> ups         = {};
    Bits<u64, NUM_KEYS> states      = {};
    Bits<u64, NUM_KEYS> scan_downs  = {};
    Bits<u64, NUM_KEYS> scan_ups    = {};
    Bits<u64, NUM_KEYS> scan_states = {};
  };

  void                    *app              = nullptr;
  StrHashMap<void *>       globals          = {};
  steady_clock::time_point timestamp        = {};
  nanoseconds              timedelta        = {};
  ClipBoard               *clipboard        = nullptr;
  SystemTheme              theme : 2        = SystemTheme::None;
  TextDirection            direction : 1    = TextDirection::LeftToRight;
  Mouse                    mouse            = {};
  KeyBoard                 keyboard         = {};
  Span<u8 const>           drag_payload     = {};
  Span<u8 const>           text_input       = {};
  Span<u32 const>          text_input_utf32 = {};
  Vec2                     viewport_size    = {};

  constexpr bool key_down(KeyCode key) const
  {
    return get_bit(span(keyboard.downs), (usize) key);
  }

  constexpr bool key_up(KeyCode key) const
  {
    return get_bit(span(keyboard.ups), (usize) key);
  }

  constexpr bool key_state(KeyCode key) const
  {
    return get_bit(span(keyboard.states), (usize) key);
  }

  constexpr bool key_down(ScanCode key) const
  {
    return get_bit(span(keyboard.scan_downs), (usize) key);
  }

  constexpr bool key_up(ScanCode key) const
  {
    return get_bit(span(keyboard.scan_ups), (usize) key);
  }

  constexpr bool key_state(ScanCode key) const
  {
    return get_bit(span(keyboard.scan_states), (usize) key);
  }

  constexpr bool mouse_down(MouseButtons b) const
  {
    return has_bits(mouse.downs, b);
  }

  constexpr bool mouse_up(MouseButtons b) const
  {
    return has_bits(mouse.ups, b);
  }

  constexpr bool mouse_state(MouseButtons b) const
  {
    return has_bits(mouse.states, b);
  }
};

/// @param tab Tab Index for Focus-Based Navigation. desired tab index, I32_MIN
/// meaning the default tab order based on the hierarchy of the parent to
/// children and siblings (depth-first traversal). Negative values have are
/// focused before positive values.
/// @param hidden if the view should be hidden from view (will not receive
/// visual events, but still receive tick events)
/// @param pointable can receive mouse enter/move/leave events
/// @param clickable can receive mouse press events
/// @param scrollable can receive mouse scroll events
/// @param draggable can receive drag events
/// @param focusable can receive keyboard focus (ordered by `tab`) and keyboard
/// events
/// @param text_input can receive text input when focused (excluding tab
/// key/non-text keys)
/// @param tab_input can receive `Tab` key as input when focused
/// @param grab_focus user to focus on view
/// @param lose_focus lose view focus
struct ViewState
{
  i32  tab            = I32_MIN;
  bool hidden : 1     = false;
  bool pointable : 1  = false;
  bool clickable : 1  = false;
  bool scrollable : 1 = false;
  bool draggable : 1  = false;
  bool droppable : 1  = false;
  bool focusable : 1  = false;
  bool text_input : 1 = false;
  bool tab_input : 1  = false;
  bool grab_focus : 1 = false;
  bool lose_focus : 1 = false;
};

struct CoreViewTheme
{
  Vec4 background        = {};
  Vec4 surface           = {};
  Vec4 surface_variant   = {};
  Vec4 primary           = {};
  Vec4 primary_variant   = {};
  Vec4 secondary         = {};
  Vec4 secondary_variant = {};
  Vec4 error             = {};
  Vec4 warning           = {};
  Vec4 success           = {};
  Vec4 active            = {};
  Vec4 inactive          = {};
  Vec4 on_background     = {};
  Vec4 on_surface        = {};
  Vec4 on_primary        = {};
  Vec4 on_secondary      = {};
  Vec4 on_error          = {};
  Vec4 on_warning        = {};
  Vec4 on_success        = {};
  f32  body_font_height  = {};
  f32  h1_font_height    = {};
  f32  h2_font_height    = {};
  f32  h3_font_height    = {};
  f32  line_height       = {};
};

constexpr CoreViewTheme DEFAULT_THEME = {
    .background        = Vec4U8{0x19, 0x19, 0x19, 0xFF}.norm(),
    .surface           = Vec4U8{0x33, 0x33, 0x33, 0xFF}.norm(),
    .primary           = mdc::DEEP_ORANGE_600.norm(),
    .primary_variant   = mdc::DEEP_ORANGE_400.norm(),
    .secondary         = mdc::PURPLE_600.norm(),
    .secondary_variant = mdc::PURPLE_400.norm(),
    .error             = mdc::RED_500.norm(),
    .warning           = mdc::YELLOW_800.norm(),
    .success           = mdc::GREEN_700.norm(),
    .active            = Vec4U8{0x70, 0x70, 0x70, 0xFF}.norm(),
    .inactive          = Vec4U8{0x47, 0x47, 0x47, 0xFF}.norm(),
    .on_background     = mdc::WHITE.norm(),
    .on_surface        = mdc::WHITE.norm(),
    .on_primary        = mdc::WHITE.norm(),
    .on_secondary      = mdc::WHITE.norm(),
    .on_error          = mdc::WHITE.norm(),
    .on_warning        = mdc::WHITE.norm(),
    .on_success        = mdc::WHITE.norm(),
    .body_font_height  = 16,
    .h1_font_height    = 30,
    .h2_font_height    = 27,
    .h3_font_height    = 22,
    .line_height       = 1.2F};

/// @brief Base view class. All view types must inherit from this struct.
/// Views are plain visual elements that define spatial relationships,
/// visual state changes, and forward events to other subsystems.
/// @note State changes must only happen in the `tick` method. for child view
/// switching, it should be handled by a flag in the tick method and switch in
/// the child method based on the flag.
struct View
{
  struct
  {
    u64   id                  = 0;
    u64   last_rendered_frame = 0;
    CRect region              = {};
  } inner = {};

  constexpr View()                        = default;
  constexpr View(View const &)            = default;
  constexpr View(View &&)                 = default;
  constexpr View &operator=(View const &) = default;
  constexpr View &operator=(View &&)      = default;
  constexpr virtual ~View()               = default;

  /// @returns the ID currently allocated to the view or 0
  constexpr u64 id() const
  {
    return inner.id;
  }

  /// @brief called on every frame. used for state changes, animations, task
  /// dispatch and lightweight processing related to the GUI. heavy-weight and
  /// non-sub-millisecond tasks should be dispatched to a Subsystem that would
  /// handle that. i.e. using the multi-tasking system.
  /// @param build callback to be called to insert subviews.
  //
  constexpr virtual ViewState tick(ViewContext const &ctx, CRect const &region,
                                   ViewEvents events, Fn<void(View &)> build)
  {
    (void) ctx;
    (void) region;
    (void) events;
    (void) build;
    return ViewState{};
  }

  /// @brief distributes the size allocated to it to its child views.
  /// @param allocated the size allocated to this view
  /// @param[out] sizes sizes allocated to the children.
  constexpr virtual void size(Vec2 allocated, Span<Vec2> sizes)
  {
    (void) allocated;
    fill(sizes, Vec2{0, 0});
  }

  /// @brief fits itself around its children and positions child views
  /// relative to its center
  /// @param allocated the size allocated to this view
  /// @param sizes sizes of the child views
  /// @param[out] offsets offsets of the views from the parent's center
  /// @return this view's fitted extent
  constexpr virtual Vec2 fit(Vec2 allocated, Span<Vec2 const> sizes,
                             Span<Vec2> offsets)
  {
    (void) allocated;
    (void) sizes;
    fill(offsets, Vec2{0, 0});
    return Vec2{0, 0};
  }

  /// @brief this is used for absolute positioning of the view
  /// @param center the allocated absolute center of this view relative
  /// to the viewport
  constexpr virtual Vec2 position(CRect const &region)
  {
    return region.center;
  }

  /// @brief returns the stacking layer index
  /// @param allocated stacking layer index allocated to this view
  /// by parent. This functions similar to the CSS stacking context. The layer
  /// index has a higher priority over the z-index.
  /// @return
  constexpr virtual i32 stack(i32 allocated)
  {
    return allocated;
  }

  /// @brief returns the z-index of itself and assigns z-indices to its children
  /// @param allocated z-index allocated to this view by parent
  /// @param[out] indices z-index assigned to children
  /// @return
  constexpr virtual i32 z_index(i32 allocated, Span<i32> indices)
  {
    fill(indices, allocated + 1);
    return allocated;
  }

  /// @brief this is used for clipping views. the provided clip is
  /// relative to the root viewport. Used for nested viewports where there are
  /// multiple intersecting clips.
  constexpr virtual CRect clip(CRect const &region, CRect const &allocated)
  {
    (void) region;
    return allocated;
  }

  /// @brief record draw commands needed to render this view. this method is
  /// only called if the view passes the visibility tests. this is called on
  /// every frame.
  /// @param canvas
  constexpr virtual void render(CRect const &region, CRect const &clip,
                                Canvas &canvas)
  {
    (void) region;
    (void) clip;
    (void) canvas;
  }

  /// @brief Used for hit-testing regions of views.
  /// @param area area of view within the viewport
  /// @param offset offset of pointer within area
  /// @return true if in hit region
  constexpr virtual bool hit(CRect const &region, Vec2 offset)
  {
    (void) region;
    (void) offset;
    return true;
  }

  /// @brief Select cursor type given a highlighted region of the view.
  constexpr virtual Cursor cursor(CRect const &region, Vec2 offset)
  {
    (void) region;
    (void) offset;
    return Cursor::Default;
  }
};

}        // namespace ash
