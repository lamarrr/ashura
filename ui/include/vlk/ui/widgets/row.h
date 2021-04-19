#pragma once

#include <vector>

#include "vlk/ui/layout.h"
#include "vlk/ui/primitives.h"
#include "vlk/ui/widget.h"
#include "vlk/ui/widget_builder.h"

#include "stx/span.h"

namespace vlk {
namespace ui {

// TODO(lamarrr): widgets can be made to be in source

struct RowProps {
  constexpr RowProps() {}

  constexpr RowProps wrap(Wrap row_wrap) const {
    RowProps out{*this};
    out.wrap_ = row_wrap;
    return out;
  }

  constexpr Wrap wrap() const { return wrap_; }

  constexpr RowProps main_align(MainAlign align) const {
    RowProps out{*this};
    out.main_align_ = align;
    return out;
  }

  constexpr MainAlign main_align() const { return main_align_; }

  constexpr RowProps cross_align(CrossAlign align) const {
    RowProps out{*this};
    out.cross_align_ = align;
    return out;
  }

  constexpr CrossAlign cross_align() const { return cross_align_; }

  constexpr RowProps main_fit(Fit fit) const {
    RowProps out{*this};
    out.main_fit_ = fit;
    return out;
  }

  constexpr Fit main_fit() const { return main_fit_; }

  constexpr RowProps cross_fit(Fit fit) const {
    RowProps out{*this};
    out.cross_fit_ = fit;
    return out;
  }

  constexpr Fit cross_fit() const { return cross_fit_; }

 private:
  Wrap wrap_ = Wrap::Wrap;
  MainAlign main_align_ = MainAlign::Start;
  CrossAlign cross_align_ = CrossAlign::Start;
  Fit main_fit_ = Fit::Shrink;
  Fit cross_fit_ = Fit::Shrink;
};

struct Row : public Widget {
  Row(WidgetBuilder children_builder, RowProps const& props = {}) {
    Widget::init_is_flex(true);
    children_ = build_children(children_builder);
    Widget::update_children(children_);
    update_props(props);
  }

  Row(std::vector<Widget*>&& children, RowProps const& props = {}) {
    Widget::init_is_flex(true);
    children_ = std::move(children);
    Widget::update_children(children_);
    update_props(props);
  }

  Row(std::initializer_list<Widget*> const& children,
      RowProps const& props = {}) {
    Widget::init_is_flex(true);
    children_ = build_children(children);
    Widget::update_children(children_);
    update_props(props);
  }

  Row(stx::Span<Widget*> const& children, RowProps const& props = {}) {
    Widget::init_is_flex(true);
    children_ = build_children(children);
    Widget::update_children(children_);
    update_props(props);
  }

  Row() = delete;
  Row(Row const&) = delete;
  Row(Row&&) = delete;
  Row& operator=(Row const&) = delete;
  Row& operator=(Row&&) = delete;
  ~Row() = default;

  void update_props(RowProps const& props) {
    Flex flex{};
    flex.direction = Direction::Row;
    flex.wrap = props.wrap();
    flex.main_align = props.main_align();
    flex.cross_align = props.cross_align();
    flex.main_fit = props.main_fit();
    flex.cross_fit = props.cross_fit();
    Widget::update_flex(flex);
    Widget::update_self_extent(SelfExtent::relative(1.0f, 1.0f));
  }

  virtual void draw(Canvas&) override {
    // no-op
  }

 private:
  std::vector<Widget*> children_;
};

}  // namespace ui
}  // namespace vlk
