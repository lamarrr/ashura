
#pragma once

#include <chrono>
#include <cinttypes>
#include <functional>
#include <string_view>

#include "stx/option.h"
#include "stx/report.h"
#include "stx/span.h"
#include "vlk/ui/asset_manager.h"
#include "vlk/ui/canvas.h"
#include "vlk/ui/layout.h"
#include "vlk/utils/limits.h"
#include "vlk/utils/utils.h"

namespace vlk {
namespace ui {

//! NOTE: Widget is a very large struct (about 420-bytes). avoid
//! touching the struct in hot code paths as it could disrupt cache
//! if you're touching a large number of them, especially whilst not all fields
//! of it are touched
//! NOTE: this struct's data is always accessed from the main thread.

enum class WidgetType : uint8_t {
  //! occupies space and has render data
  Render,
  //! for view-based scrolling, has no render data
  View
};

enum class WidgetDirtiness : uint8_t {
  None = 0,
  Render = 1,
  ViewOffset = 2,
  Layout = 4,
  Children = 8
};

VLK_DEFINE_ENUM_BIT_OPS(WidgetDirtiness)

struct WidgetDebugInfo {
  std::string_view name = "<unnamed>";
  std::string_view type_hint = "<none>";
};

// binds to different parts of the the pipeline and its trees that we want to
// abstract as much as possible. the callbacks also function to capture various
// values.
struct WidgetStateProxy {
  //! informs the system that the widget's render data has changed
  std::function<void()> on_render_dirty = [] {};

  //! informs the system that the widget's layout has changed
  std::function<void()> on_layout_dirty = [] {};

  //! informs the system that a view-widgets's offset (or visible area)
  //! has changed
  std::function<void()> on_view_offset_dirty = [] {};

  //! informs the system that the widget's children has changed (possibly
  //! requiring a full rebuild of the pipeline)
  std::function<void()> on_children_changed = [] {};

  //! we need to be able to consult the tree for the widget's offset i.e. in the
  //! scenario where we need to scroll to it
};

// important: if layout is updated multiple times in between ticks the ticked
// one is not tracked, the widget only tracks the latest one
//
//
// TODO(lamarrr): visibility
//
//
// do we maintain a separate layout tree for them? or still reside on the main
// layout tree Positioning{Normal, Viewport}
//
// we'll need a list of sticky widgets
// fixed positioning?
//
//

struct Widget {
  friend struct WidgetSystemProxy;

  Widget(WidgetType type = WidgetType::Render, bool is_flex = false,
         SelfExtent self_extent = SelfExtent{}, bool needs_trimming = false,
         Padding padding = Padding{}, Flex flex = Flex{},
         stx::Span<Widget *const> children = {},
         ViewExtent view_extent = ViewExtent{},
         ViewOffset view_offset = ViewOffset{},
         ViewFit view_fit = ViewFit::None,
         stx::Option<ZIndex> z_index = stx::None,
         WidgetDebugInfo debug_info = WidgetDebugInfo{})
      : type_{type},
        is_flex_{is_flex},
        self_extent_{self_extent},
        needs_trimming_{needs_trimming},
        padding_{padding},
        flex_{flex},
        children_{children},
        view_extent_{view_extent},
        view_offset_{view_offset},
        view_fit_{view_fit},
        z_index_{z_index},
        debug_info_{debug_info},
        dirtiness_{WidgetDirtiness::None},
        state_proxy_{} {}

  Widget(Widget const &) = delete;
  Widget(Widget &&) = delete;

  Widget &operator=(Widget const &) = delete;
  Widget &operator=(Widget &&) = delete;

  WidgetType get_type() const { return type_; }

  bool is_flex() const { return is_flex_; }

  SelfExtent get_self_extent() const { return self_extent_; }

  bool needs_trimming() const { return needs_trimming_; }

  Padding get_padding() const { return padding_; }

  Flex get_flex() const { return flex_; }

  stx::Span<Widget *const> get_children() const { return children_; }

  bool has_children() const { return !get_children().empty(); }

  ViewExtent get_view_extent() const { return view_extent_; }

  ViewOffset get_view_offset() const { return view_offset_; }

  ViewFit get_view_fit() const { return view_fit_; }

  stx::Option<ZIndex> get_z_index() const { return z_index_; }

  WidgetDebugInfo get_debug_info() const { return debug_info_; }

  WidgetDirtiness get_dirtiness() const { return dirtiness_; }

  //! create draw commands
  //! NOTE: states, variables, or properties that could affect rendering must
  //! not change in the draw method until `mark_rendering_dirty()` is called,
  //! else this would lead to partial updates in a tile-based rendering
  //! scenario.
  //!
  virtual void draw([[maybe_unused]] Canvas &) {
    // no-op
  }

  //! process any event you need to process here.
  //! animations and property updates can and should happen here.
  virtual void tick([[maybe_unused]] std::chrono::nanoseconds interval,
                    [[maybe_unused]] AssetManager &asset_manager) {
    // no-op
  }

  virtual Extent trim(Extent extent) { return extent; }

  virtual ~Widget() {}

  void init_type(WidgetType type) { type_ = type; }

  void init_is_flex(bool is_flex) { is_flex_ = is_flex; }

  void update_self_extent(SelfExtent self_extent) {
    if (self_extent_ != self_extent) {
      self_extent_ = self_extent;
      mark_layout_dirty();
    }
  }

  void update_needs_trimming(bool needs_trimming) {
    VLK_ENSURE(!is_flex(), "Only non-flex Widgets can be trimmed", *this);

    if (needs_trimming_ != needs_trimming) {
      needs_trimming_ = needs_trimming;
      mark_layout_dirty();
    }
  }

  void update_padding(Padding padding) {
    if (padding_ != padding) {
      padding_ = padding;
      mark_layout_dirty();
    }
  }

  void update_flex(Flex flex) {
    VLK_ENSURE(is_flex(), "Widget is not a flex type", *this);
    if (flex_ != flex) {
      flex_ = flex;
      mark_layout_dirty();
    }
  }

  //! MOTE: this does not free the memory associated with the referenced
  //! container. the derived widget is in charge of freeing memory as necessary.
  //! avoid using this as much as possible as it can cause a full re-build of
  //! the pipeline.
  void update_children(stx::Span<Widget *const> children) {
    // we assume the memory has been released or the widget still uses the same
    // children span but with the child widgets pointers changed
    VLK_ENSURE(is_flex(), "Widget is not a flex type", *this);
    children_ = children;
    mark_children_dirty();
  }

  void update_view_extent(ViewExtent view_extent) {
    VLK_ENSURE(get_type() == WidgetType::View, "Widget is not a view type",
               *this);
    if (view_extent_ != view_extent) {
      view_extent_ = view_extent;
      mark_layout_dirty();
    }
  }

  void update_view_offset(ViewOffset view_offset) {
    VLK_ENSURE(get_type() == WidgetType::View, "Widget is not a view type",
               *this);
    if (view_offset_ != view_offset) {
      view_offset_ = view_offset;
      mark_view_offset_dirty();
    }
  }

  void update_view_fit(ViewFit view_fit) {
    VLK_ENSURE(get_type() == WidgetType::View, "Widget is not a view type",
               *this);
    if (view_fit_ != view_fit) {
      view_fit_ = view_fit;
      mark_layout_dirty();
    }
  }

  void init_z_index(stx::Option<ZIndex> z_index) { z_index_ = z_index; }

  void set_debug_info(WidgetDebugInfo info) { debug_info_ = info; }

  void add_dirtiness(WidgetDirtiness dirtiness) { dirtiness_ |= dirtiness; }

  void mark_children_dirty() { dirtiness_ |= WidgetDirtiness::Children; }

  void mark_layout_dirty() { dirtiness_ |= WidgetDirtiness::Layout; }

  void mark_view_offset_dirty() { dirtiness_ |= WidgetDirtiness::ViewOffset; }

  void mark_render_dirty() { dirtiness_ |= WidgetDirtiness::Render; }

 private:
  void system_tick(std::chrono::nanoseconds interval,
                   AssetManager &asset_manager) {
    tick(interval, asset_manager);

    if ((dirtiness_ & WidgetDirtiness::Children) != WidgetDirtiness::None) {
      state_proxy_.on_children_changed();
    }

    if ((dirtiness_ & WidgetDirtiness::Layout) != WidgetDirtiness::None) {
      state_proxy_.on_layout_dirty();
    }

    if ((dirtiness_ & WidgetDirtiness::Render) != WidgetDirtiness::None) {
      state_proxy_.on_render_dirty();
    }

    if ((dirtiness_ & WidgetDirtiness::ViewOffset) != WidgetDirtiness::None) {
      state_proxy_.on_view_offset_dirty();
    }

    dirtiness_ = WidgetDirtiness::None;
  }

  //! constant throughout lifetime
  WidgetType type_;

  //! constant throughout lifetime
  bool is_flex_;

  //! variable throughout lifetime. communicate changes using `on_layout_dirty`.
  // for view widgets, this is effectively the size that's actually visible.
  SelfExtent self_extent_;

  //! variable throughout lifetime. communicate changes using `on_layout_dirty`
  bool needs_trimming_;

  //! variable throughout lifetime. communicate changes using `on_layout_dirty`
  Padding padding_;

  //! variable throughout lifetime. communicate changes using `on_layout_dirty`
  Flex flex_;

  //! variable throughout lifetime. communicate changes using
  //! `on_children_changed`
  stx::Span<Widget *const> children_;

  //! for view widgets (used for laying out its children).
  //!
  //! variable throughout lifetime.
  //! resolved using the parent allotted extent.
  ViewExtent view_extent_;

  //! for view widgets (used for scrolling or moving of the view)
  //!
  //! variable throughout lifetime. communicate changes with
  //! `on_view_offset_changed`.
  //! resolved using the view extent.
  ViewOffset view_offset_;

  // variable throughout lifetime. communicate changes using `on_layout_dirty`
  ViewFit view_fit_;

  //! constant throughout lifetime
  stx::Option<ZIndex> z_index_;

  //! variable throughout lifetime
  WidgetDebugInfo debug_info_;

  //! modified and used for communication of updates to the system
  WidgetDirtiness dirtiness_;

  //! modified and used for communication of updates to the system
  WidgetStateProxy state_proxy_;
};

std::string format(Widget const &widget);
stx::FixedReport operator>>(stx::ReportQuery, Widget const &widget);

struct WidgetSystemProxy {
  static void tick(Widget &widget, std::chrono::nanoseconds interval,
                   AssetManager &asset_manager) {
    widget.system_tick(interval, asset_manager);
  }

  static WidgetStateProxy &get_state_proxy(Widget &widget) {
    return widget.state_proxy_;
  }
};

}  // namespace ui
}  // namespace vlk
