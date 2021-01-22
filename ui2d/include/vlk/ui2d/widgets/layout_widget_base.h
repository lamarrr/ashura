#pragma once

#include <memory>
#include <vector>

#include "vlk/ui2d/canvas.h"
#include "vlk/ui2d/primitives.h"
#include "vlk/ui2d/widget.h"

#include "stx/span.h"
#include "vlk/utils/utils.h"

namespace vlk {
namespace ui2d {

template <bool IsStateful, typename ChildDeleter = DefaultWidgetChildDeleter,
          typename PointerAllocator = std::allocator<Widget *>>
struct LayoutWidgetBase : public Widget {
  explicit LayoutWidgetBase(stx::Span<Widget *const> const &children) {
    add_children_(children);
  }

  explicit LayoutWidgetBase(std::initializer_list<Widget *> const &children) {
    add_children_(children);
  }

  virtual bool is_layout_type() const noexcept final override { return true; }

  virtual bool is_stateful() const noexcept final override {
    return IsStateful;
  }

  virtual stx::Span<Widget *const> get_children() const
      noexcept final override {
    return children_;
  }

  virtual Rect compute_area(Extent const &allotted_extent,
                            stx::Span<Rect> const &children_area) override = 0;

  virtual void draw(
      [[maybe_unused]] Canvas &canvas,
      [[maybe_unused]] Extent const &requested_extent) final override {
    // no-op, won't be called since `is_positioning_type` is true.
  }

  virtual ~LayoutWidgetBase() noexcept override { delete_children_(); }

 private:
  std::vector<Widget *, PointerAllocator> children_;

  void add_children_(stx::Span<Widget *const> const &children) {
    // exception needs to be handled because we've taken ownership of the widget
    VLK_ENSURE(children.size() <= static_cast<uint64_t>(u32_max),
               "size of widget children exceeded u32_max", children.size());

    try {
      children_.insert(children_.end(), children.begin(), children.end());
    } catch (std::exception &oom) {
      for (Widget *child : children) {
        ChildDeleter{}(child);
      }
      throw oom;
    }
  }

  void delete_children_() {
    for (auto *child : children_) {
      ChildDeleter{}(child);
    }
    children_.clear();
  }
};

template <bool IsStateful, typename ChildDeleter = DefaultWidgetChildDeleter>
struct BoxLayoutWidgetBase : public Widget {
  explicit BoxLayoutWidgetBase(Widget *child) : children_{child} {}

  virtual bool is_layout_type() const noexcept final override { return true; }

  virtual bool is_stateful() const noexcept final override {
    return IsStateful;
  }

  virtual stx::Span<Widget *const> get_children() const
      noexcept final override {
    return children_;
  }

  virtual Rect compute_area(Extent const &allotted_extent,
                            stx::Span<Rect> const &children_area) override = 0;

  virtual void draw(
      [[maybe_unused]] Canvas &canvas,
      [[maybe_unused]] Extent const &requested_extent) final override {
    // no-op, won't be called since `is_positioning_type` is true.
  }

  virtual ~BoxLayoutWidgetBase() noexcept override {
    ChildDeleter{}(children_[0]);
  }

 private:
  Widget *children_[1];
};

}  // namespace ui2d
}  // namespace vlk
