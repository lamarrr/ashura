
/// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"

#include "ashura/engine/view.h"
#include "ashura/engine/view_system.h"
#include "ashura/engine/views.h"

using namespace ash;

struct SwitchStack : StackView
{
  Switch switches[2] = {

  };

  SwitchStack()
  {
    style.alignment         = {-1, -1};
    switches[0].style.frame = Frame{.width = {10}, .height = {10}};
    switches[1].style.frame = Frame{.width = {20}, .height = {20}};
  }

  virtual ViewState tick(ViewContext const &, CRect const &, f32, ViewEvents,
                         Fn<void(View &)> build) override
  {
    build(switches[0]);
    build(switches[1]);
    return {};
  }
};

struct BasicViewport : View
{
  SwitchStack stack;

  BasicViewport()
  {
  }

  virtual ViewState tick(ViewContext const &, CRect const &, f32, ViewEvents,
                         Fn<void(View &)> build) override
  {
    build(stack);
    return {.viewport = true};
  }

  constexpr virtual void size(Vec2 allocated, Span<Vec2> sizes) override
  {
    fill(sizes, allocated);
  }

  constexpr virtual ViewLayout fit(Vec2, Span<Vec2 const> sizes,
                                   Span<Vec2> centers) override
  {
    fill(centers, Vec2{0, 0});
    return {.extent          = {2, 2},
            .viewport_extent = {20, 20},
            .viewport_transform =
                scroll_transform({20, 20}, {2, 2}, {0, 0}, 1)};
  }
};

TEST(ViewSystem, Basic)
{
  ViewSystem view_sys{default_allocator};

  BasicViewport root;
  ViewContext   ctx;
  ctx.viewport_extent = {200, 200};
  Canvas canvas{default_allocator};
  view_sys.tick(ctx, root, canvas);
}