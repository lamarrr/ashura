
#pragma once

#include <chrono>
#include <cstdint>
#include <functional>
#include <string_view>
#include <utility>

#include "stx/option.h"
#include "stx/span.h"
#include "vlk/utils/limits.h"
#include "vlk/utils/utils.h"

#include "vlk/ui/canvas.h"
#include "vlk/ui/constraints.h"

namespace vlk {
namespace ui {

using Effect = std::function<void(Canvas &)>;

struct Widget {
  friend struct WidgetStateProxyAdapter;

  struct StateProxy {
    /// informs the compositor that the widget's render data has changed
    std::function<void()> on_render_dirty = [] {};

    /// informs the compositor that the widget's layout has changed
    std::function<void()> on_layout_dirty = [] {};

    /// informs the compositor that the widget's children has changed (possibly
    /// requiring a full rebuild)
    std::function<void()> on_children_changed = [] {};

    /// informs the compositor that the effect on the widget has changed
    std::function<void()> on_effect_changed = [] {};

    /// informs the compositor that a view-widgets's offset (or visible area)
    /// has changed
    std::function<void()> on_view_offset_dirty = [] {};
  };

  /// builds a list of widgets, the function is called with the index, until it
  /// returns `stx::None`
  using Builder = std::function<stx::Option<Widget *>(size_t)>;

  enum class Type : uint8_t {
    /// occupies space and has render data
    Render,
    /// only occupies space but doesn't have render data
    Layout,
    /// marks a separate composition context, has no render data
    View,
    /// TODO(lamarrr): Effect?
  };

  enum class ViewEvent : uint8_t {
    /// widget is in-view
    Enter,
    /// widget is not in-view
    Leave,
  };

  Widget(Type type = Type::Render, bool should_cache = true,
         SelfLayout self_layout = {},
         stx::Span<ChildLayout const> children_layout = {},
         stx::Span<Widget *const> children = {},
         std::string_view name = "<unnamed widget>",
         std::string_view type_hint = "Widget",
         stx::Option<uint32_t> preferred_stack_index = stx::None,
         ViewExtent view_extent = {}, ViewOffset view_offset = {})
      : type_{type},
        should_cache_{should_cache},
        self_layout_{self_layout},
        children_layout_{children_layout},
        children_{children},
        name_{name},
        type_hint_{type_hint},
        preferred_stack_index_{preferred_stack_index.clone()},
        view_extent_{view_extent},
        view_offset_{view_offset},
        state_proxy_{} {}

  Type get_type() const noexcept { return type_; }

  bool should_cache() const noexcept { return should_cache_; }

  SelfLayout get_self_layout() const noexcept { return self_layout_; }

  bool is_flexible() const noexcept {
    bool flexible_dependent_width = f32_eq(
        std::holds_alternative<DependentParameters>(get_self_layout().width) &&
            std::get<DependentParameters>(get_self_layout().width)
                .children_allocation.scale,
        0.0f);

    // is it technically unbounded if it's width depends on the children's?

    // min max same
    bool flexible_independent_width = f32_eq(
        std::holds_alternative<IndependentParameters>(
            get_self_layout().width) &&
            std::get<IndependentParameters>(get_self_layout().width).scale,
        0.0f);

    bool flexible_dependent_height = f32_eq(
        std::holds_alternative<DependentParameters>(get_self_layout().height) &&
            std::get<DependentParameters>(get_self_layout().height)
                .children_allocation.scale,
        0.0f);

    bool flexible_independent_height = f32_eq(
        std::holds_alternative<IndependentParameters>(
            get_self_layout().height) &&
            std::get<IndependentParameters>(get_self_layout().height).scale,
        0.0f);

    return flexible_dependent_height || flexible_dependent_width ||
           flexible_independent_height || flexible_independent_width;
  }

  stx::Span<Widget *const> get_children() const noexcept { return children_; }

  bool has_children() const noexcept { return get_children().size() != 0; }

  stx::Span<ChildLayout const> get_children_layout() const noexcept {
    return children_layout_;
  }

  stx::Option<uint32_t> get_preferred_stack_index() const noexcept {
    return preferred_stack_index_.clone();
  }

  ViewExtent get_view_extent() const noexcept { return view_extent_; }
  ViewOffset get_view_offset() const noexcept { return view_offset_; }

  stx::Span<Effect const> get_post_effects() const noexcept {
    return post_effects_;
  }

  stx::Span<Effect const> get_pre_effects() const noexcept {
    return pre_effects_;
  }

  void mark_render_dirty() { state_proxy_.on_render_dirty(); }

  virtual void on_view(ViewEvent) {}

  // we can use bulk allocation here
  virtual void draw([[maybe_unused]] Canvas &canvas) {}

  /// process any event you need to process here.
  virtual void tick([[maybe_unused]] std::chrono::nanoseconds const &interval) {
    // always called on the same thread this widget was created from
    // not called until the render thread is done.
  }

  virtual ~Widget() {}

  void update_self_layout(SelfLayout layout) {
    self_layout_ = layout;
    mark_layout_dirty_();
  }

  void update_children_layout(stx::Span<ChildLayout const> layout) {
    children_layout_ = layout;
    mark_layout_dirty_();
  }

  // this does not free the children, the derived widget is in charge of freeing
  // any memory belonging to the list
  void update_children(stx::Span<Widget *const> children) {
    children_ = children;
    mark_children_dirty_();
  }

  void update_view_extent(ViewExtent extent) {
    VLK_DEBUG_ENSURE(type_ == Type::View);
    view_extent_ = extent;
    mark_layout_dirty_();
  }

  void update_view_offset(ViewOffset offset) {
    VLK_DEBUG_ENSURE(type_ == Type::View);
    view_offset_ = offset;
    mark_view_offset_dirty_();
  }

 private:
  /// constant throughout lifetime
  Type type_;

  /// constant throughout lifetime
  bool should_cache_;

  /// variable throughout lifetime. communicate changes with `on_layout_dirty`
  SelfLayout self_layout_;

  /// variable throughout lifetime. communicate changes with `on_layout_dirty`
  stx::Span<ChildLayout const> children_layout_;

  /// variable throughout lifetime. communicate changes with
  /// `on_children_changed`
  stx::Span<Widget *const> children_;

  /// constant throughout lifetime
  std::string_view name_;

  /// constant throughout lifetime
  std::string_view type_hint_;

  /// constant throughout lifetime
  stx::Option<uint32_t> preferred_stack_index_;

  /// for view widgets (used for laying out its children).
  ///
  /// variable throughout lifetime ????.
  ViewExtent view_extent_;

  /// for view widgets (used for scrolling or moving of the view)
  ///
  /// variable throughout lifetime. communicate changes with
  /// `on_view_offset_changed`.
  ViewOffset view_offset_;

  StateProxy state_proxy_;

  stx::Span<Effect const> pre_effects_{};
  stx::Span<Effect const> post_effects_{};

  void mark_children_dirty_() { state_proxy_.on_children_changed(); }
  void mark_layout_dirty_() { state_proxy_.on_layout_dirty(); }
  void mark_view_offset_dirty_() { state_proxy_.on_view_offset_dirty(); }
};

struct WidgetStateProxyAdapter {
  template <typename Callback>
  static void install_on_render_dirty(Widget &widget,
                                      Callback &&on_render_dirty) {
    widget.state_proxy_.on_render_dirty =
        std::forward<Callback &&>(on_render_dirty);
  }

  static void detach_on_render_dirty(Widget &widget) {
    widget.state_proxy_.on_render_dirty = [] {};
  }

  template <typename Callback>
  static void install_on_layout_dirty(Widget &widget,
                                      Callback &&on_layout_dirty) {
    widget.state_proxy_.on_layout_dirty =
        std::forward<Callback &&>(on_layout_dirty);
  }

  static void detach_on_layout_dirty(Widget &widget) {
    widget.state_proxy_.on_layout_dirty = [] {};
  }
};

}  // namespace ui
}  // namespace vlk

/*


*/

/*struct EffectNode {
  std::shared_ptr<EffectNode> child;

  stx::Span<Effect const> pre_effect;
  stx::Span<Effect const> post_effect;
  bool is_updated;
};
*/
