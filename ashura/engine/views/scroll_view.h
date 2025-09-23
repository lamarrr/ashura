/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/view.h"
#include "ashura/std/types.h"

namespace ash
{

namespace ui
{

struct ScrollState
{
  /// @brief The center of the scroll. This is relative to the visible region's center
  f32 center_ = 0;

  /// @brief The delta to move by for each key press
  f32 delta_ = 0.1F;

  /// @brief The visible extent the scroll bar represents
  f32 visible_extent_ = 0;

  /// @brief The total extent the scroll bar represents or INF
  f32 content_extent_ = 0;

  /// @brief The visual representation of the track extent
  f32 track_extent_ = 0;

  ScrollState & clamp()
  {
    visible_extent_ = max(visible_extent_, 0.0F);
    content_extent_ = max(content_extent_, 0.0F);
    track_extent_   = max(track_extent_, 0.0F);
    visible_extent_ = min(visible_extent_, content_extent_);
    delta_          = ash::clamp(delta_, 0.0F, 1.0F);
    center_ = ash::clamp(center_, 0.0F, content_extent_ - visible_extent_);
    return *this;
  }

  ScrollState & center(f32 v)
  {
    center_ = v;
    clamp();
    return *this;
  }

  ScrollState & delta(f32 v)
  {
    delta_ = v;
    clamp();
    return *this;
  }

  ScrollState & extent(f32 visible, f32 content, f32 track)
  {
    visible_extent_ = visible;
    content_extent_ = content;
    track_extent_   = track;
    clamp();
    return *this;
  }

  f32 center() const
  {
    return center_;
  }

  f32 delta() const
  {
    return delta_;
  }

  f32 visible_extent() const
  {
    return visible_extent_;
  }

  f32 content_extent() const
  {
    return content_extent_;
  }

  f32 track_extent() const
  {
    return track_extent_;
  }
};

// [ ] states should have firing event types

// [ ] resolve extents correctly
// [ ] WE SHOULD PROBABLY USE OFFSET?
// [ ] use alignment positioning, let scrollview use offset
struct ScrollBar : View
{
  struct State
  {
    bool disabled : 1 = false;

    bool hidden : 1 = false;

    bool dragging : 1 = false;

    bool focused : 1 = false;

    bool hovered : 1 = false;

    ScrollState scroll = {};
  } state_;

  struct Style
  {
    // [ ] impl
    f32 origin = ALIGNMENT_CENTER;

    f32 thickness = 11.5F;

    f32 nudge = 5.0F;

    Axis axis = Axis::X;

    u8x4 thumb_color = theme.inactive;

    u8x4 thumb_hovered_color = theme.primary_variant;

    u8x4 thumb_dragging_color = theme.primary;

    CornerRadii thumb_corner_radii = CornerRadii::all(2);

    u8x4 track_color = theme.inactive.with_w(128);

    CornerRadii track_corner_radii = CornerRadii::all(2);

  } style_;

  ScrollBar();
  ScrollBar(ScrollBar const &)             = delete;
  ScrollBar(ScrollBar &&)                  = default;
  ScrollBar & operator=(ScrollBar const &) = delete;
  ScrollBar & operator=(ScrollBar &&)      = default;
  virtual ~ScrollBar() override            = default;

  ScrollBar & center(f32 v);

  ScrollBar & delta(f32 v);

  ScrollBar & extent(f32 visible, f32 content, f32 track);

  ScrollBar & thickness(f32 t);

  ScrollBar & disable(bool d);

  ScrollBar & thumb_color(u8x4 color);

  ScrollBar & thumb_hovered_color(u8x4 color);

  ScrollBar & thumb_dragging_color(u8x4 color);

  ScrollBar & thumb_corner_radii(CornerRadii const & c);

  ScrollBar & track_color(u8x4 color);

  ScrollBar & track_corner_radii(CornerRadii const & c);

  ScrollBar & axis(Axis axis);

  virtual ui::State tick(Ctx const & ctx, Events const & events,
                         Fn<void(View &)> build) override;

  virtual Layout fit(f32x2 allocated, Span<f32x2 const> sizes,
                     Span<f32x2> centers) override;

  virtual void render(Canvas & canvas, RenderInfo const & info) override;
};

// [ ] modals need to be at a higher index. beyond layer
// [ ] VIEEPORT resize behaviour on grab focus
// [ ] padding
struct ScrollContent : View
{
  struct Style
  {
    Frame frame = Frame{}.rel(1, 1).rel_max(F32_INF, F32_INF);
  } style_;

  ref<View> child_;

  ScrollContent(View & child);
  ScrollContent(ScrollContent const &)             = delete;
  ScrollContent(ScrollContent &&)                  = default;
  ScrollContent & operator=(ScrollContent const &) = delete;
  ScrollContent & operator=(ScrollContent &&)      = default;
  virtual ~ScrollContent() override                = default;

  ScrollContent & frame(Frame f);

  virtual ui::State tick(Ctx const & ctx, Events const & events,
                         Fn<void(View &)> build) override;

  virtual void size(f32x2 allocated, Span<f32x2> sizes) override;

  virtual Layout fit(f32x2 allocated, Span<f32x2 const> sizes,
                     Span<f32x2> centers) override;
};

// [ ] all views need to handle their zooms
struct ScrollPort : View
{
  struct State
  {
    f32x2 content_extent = {};
    f32x2 visible_extent = {};
    f32x2 zoom           = {1, 1};
    f32x2 center         = {0, 0};
  } state_;

  struct Style
  {
    Frame frame = Frame{}.abs(200, 200);
  } style_;

  ScrollContent content_;

  ScrollPort(View & child);
  ScrollPort(ScrollPort const &)             = delete;
  ScrollPort(ScrollPort &&)                  = default;
  ScrollPort & operator=(ScrollPort const &) = delete;
  ScrollPort & operator=(ScrollPort &&)      = default;
  virtual ~ScrollPort() override             = default;

  ScrollPort & frame(Frame f);

  virtual ui::State tick(Ctx const & ctx, Events const & events,
                         Fn<void(View &)> build) override;

  virtual void size(f32x2 allocated, Span<f32x2> sizes) override;

  virtual Layout fit(f32x2 allocated, Span<f32x2 const> sizes,
                     Span<f32x2> centers) override;
};

// [ ] frame: for content and for view
// [ ] resolve relative to parent?
// [ ] resizable?
struct ScrollView : View
{
  ScrollBar x_bar_;

  ScrollBar y_bar_;

  ScrollPort port_;

  ScrollView(View & child);
  ScrollView(ScrollView const &)             = delete;
  ScrollView(ScrollView &&)                  = default;
  ScrollView & operator=(ScrollView const &) = delete;
  ScrollView & operator=(ScrollView &&)      = default;
  virtual ~ScrollView() override             = default;

  ScrollView & disable(bool d);

  ScrollView & item(View & view);

  ScrollView & thumb_color(u8x4 color);

  ScrollView & thumb_hovered_color(u8x4 color);

  ScrollView & thumb_dragging_color(u8x4 color);

  ScrollView & thumb_corner_radii(CornerRadii const & c);

  ScrollView & track_color(u8x4 color);

  ScrollView & track_corner_radii(CornerRadii const & c);

  ScrollView & axes(Axes axes);

  ScrollView & view_frame(Frame f);

  ScrollView & content_frame(Frame f);

  ScrollView & bar_thickness(f32 x, f32 y);

  virtual ui::State tick(Ctx const & ctx, Events const & events,
                         Fn<void(View &)> build) override;

  virtual void size(f32x2 allocated, Span<f32x2> sizes) override;

  virtual Layout fit(f32x2 allocated, Span<f32x2 const> sizes,
                     Span<f32x2> centers) override;

  virtual i32 layer(i32 allocated, Span<i32> children) override;
};

}    // namespace ui

}    // namespace ash
