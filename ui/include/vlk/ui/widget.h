
#pragma once

#include <chrono>
#include <cinttypes>
#include <functional>
#include <string>
#include <string_view>

#include "stx/option.h"
#include "stx/report.h"
#include "stx/span.h"
#include "vlk/utils/limits.h"
#include "vlk/utils/utils.h"

#include "vlk/ui/canvas.h"
#include "vlk/ui/layout.h"

// TODO(lamarrr): should we pass a rasterization constext to the draw function?
// (do draw commands keep references to image data)?
// can we abstract the datasource provider to destroy returned memeory
// immediately after use so we can copy elsewhere? also, we need to have async
// first asset loading as loading synchronously will be slow

namespace vlk {
namespace ui {

/// NOTE: Widget is a very large struct (about 420-bytes). avoid
/// touching the struct in hot code paths as it could disrupt cache
/// if you're touching a large number of them, especially whilst not all fields
/// of it are touched
/// NOTE: this struct's data is always accessed from the main thread.
struct Widget {
  friend struct WidgetStateProxyAccessor;

  struct DebugInfo {
    DebugInfo(std::string_view const &name = "<unnamed>",
              std::string_view const &type_hint = "<none>")
        : name{name}, type_hint{type_hint} {}

    std::string_view name;
    std::string_view type_hint;
  };

  struct StateProxy {
    StateProxy()
        : on_render_dirty{[] {}},
          on_layout_dirty{[] {}},
          on_view_offset_dirty{[] {}},
          on_children_changed{[] {}} {}

    /// informs the system that the widget's render data has changed
    std::function<void()> on_render_dirty;

    /// informs the system that the widget's layout has changed
    std::function<void()> on_layout_dirty;

    /// informs the system that a view-widgets's offset (or visible area)
    /// has changed
    std::function<void()> on_view_offset_dirty;

    /// informs the system that the widget's children has changed (possibly
    /// requiring a full rebuild of the pipeline)
    std::function<void()> on_children_changed;
  };

  enum class Type : uint8_t {
    /// occupies space and has render data
    Render,
    /// for view-based scrolling, has no render data
    View
  };

  Widget(Type const &type = Type::Render, bool is_flex = false,
         SelfExtent const &self_extent = {}, bool const needs_trimming = false,
         Padding const &padding = Padding{}, Flex const &flex = {},
         stx::Span<Widget *const> const &children = {},
         ViewExtent const &view_extent = {}, ViewOffset const &view_offset = {},
         ViewFit const &view_fit = ViewFit::None,
         stx::Option<ZIndex> const &z_index = stx::None,
         DebugInfo const &debug_info = DebugInfo{})
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
        z_index_{z_index.clone()},
        debug_info_{debug_info},
        state_proxy_{} {}

  Widget(Widget const &) = delete;
  Widget(Widget &&) = delete;

  Widget &operator=(Widget const &) = delete;
  Widget &operator=(Widget &&) = delete;

  Type get_type() const { return type_; }

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

  stx::Option<ZIndex> get_z_index() const { return z_index_.clone(); }

  DebugInfo get_debug_info() const { return debug_info_; }

  virtual void draw([[maybe_unused]] Canvas &, AssetManager &) {
    // no-op
  }

  /// process any event you need to process here.
  virtual void tick([[maybe_unused]] std::chrono::nanoseconds const &interval) {
    // no-op
  }

  virtual Extent trim(Extent const &extent) { return extent; }

  virtual ~Widget() {}

  void init_type(Type type) { type_ = type; }

  void init_is_flex(bool is_flex) { is_flex_ = is_flex; }

  void update_self_extent(SelfExtent const &self_extent) {
    self_extent_ = self_extent;
    mark_layout_dirty();
  }

  void update_needs_trimming(bool need_trim) {
    VLK_ENSURE(!is_flex(), "Only non-flex Widgets can be trimmed", *this);
    needs_trimming_ = need_trim;
    mark_layout_dirty();
  }

  void update_padding(Padding const &padding) {
    padding_ = padding;
    mark_layout_dirty();
  }

  void update_flex(Flex const &flex) {
    flex_ = flex;
    VLK_ENSURE(is_flex(), "Widget is not a flex type", *this);
    mark_layout_dirty();
  }

  /// MOTE: this does not free the memory associated with the referenced
  /// container. the derived widget is in charge of freeing memory as necessary.
  /// avoid using this as much as possible as it can cause a full re-build of
  /// the pipeline.
  void update_children(stx::Span<Widget *const> children) {
    VLK_ENSURE(is_flex(), "Widget is not a flex type", *this);
    children_ = children;
    mark_children_dirty();
  }

  void update_view_extent(ViewExtent view_extent) {
    VLK_ENSURE(get_type() == Type::View, "Widget is not a view type", *this);
    view_extent_ = view_extent;
    mark_layout_dirty();
  }

  void update_view_offset(ViewOffset view_offset) {
    VLK_ENSURE(get_type() == Type::View, "Widget is not a view type", *this);
    view_offset_ = view_offset;
    mark_view_offset_dirty();
  }

  void update_view_fit(ViewFit view_fit) {
    VLK_ENSURE(get_type() == Type::View, "Widget is not a view type", *this);
    view_fit_ = view_fit;
    mark_layout_dirty();
  }

  void init_z_index(stx::Option<ZIndex> const &z_index) {
    z_index_ = z_index.clone();
  }

  void set_debug_info(DebugInfo const &info) { debug_info_ = info; }

  void mark_children_dirty() const { state_proxy_.on_children_changed(); }

  void mark_layout_dirty() const { state_proxy_.on_layout_dirty(); }

  void mark_view_offset_dirty() const { state_proxy_.on_view_offset_dirty(); }

  void mark_render_dirty() const { state_proxy_.on_render_dirty(); }

 private:
  /// constant throughout lifetime
  Type type_;

  /// constant throughout lifetime
  bool is_flex_;

  /// variable throughout lifetime. communicate changes using `on_layout_dirty`.
  // for view widgets, this is effectively the size that's actually visible.
  SelfExtent self_extent_;

  bool needs_trimming_;

  /// variable throughout lifetime. communicate changes using `on_layout_dirty`
  Padding padding_;

  /// variable throughout lifetime. communicate changes using `on_layout_dirty`
  Flex flex_;

  /// variable throughout lifetime. communicate changes using
  /// `on_children_changed`
  stx::Span<Widget *const> children_;

  /// for view widgets (used for laying out its children).
  ///
  /// variable throughout lifetime.
  /// resolved using the parent allotted extent.
  ViewExtent view_extent_;

  /// for view widgets (used for scrolling or moving of the view)
  ///
  /// variable throughout lifetime. communicate changes with
  /// `on_view_offset_changed`.
  /// resolved using the view extent.
  ViewOffset view_offset_;

  // variable throughout lifetime. communicate changes using `on_layout_dirty`
  ViewFit view_fit_;

  /// constant throughout lifetime
  stx::Option<ZIndex> z_index_;

  /// variable throughout lifetime
  DebugInfo debug_info_;

  /// modified and used for communication of updates to the system
  StateProxy state_proxy_;
};

inline stx::FixedReport operator>>(stx::ReportQuery, Widget const &widget) {
  Widget::DebugInfo const debug_info = widget.get_debug_info();
  std::string message = "Widget: ";
  message += debug_info.name;
  message += " (type hint: ";
  message += debug_info.type_hint;
  message += ", address: " + std::to_string((uintptr_t)&widget) + ")";
  return stx::FixedReport(message);
}

}  // namespace ui
}  // namespace vlk
