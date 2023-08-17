#include "ashura/palletes.h"
#include "ashura/widget_tree.h"
#include "ashura/widgets/box.h"
#include "ashura/widgets/flex.h"
#include "ashura/widgets/grid.h"
#include "ashura/widgets/image.h"
#include "ashura/widgets/padding.h"
#include "ashura/widgets/stack.h"
#include "ashura/widgets/text.h"
#include "gtest/gtest.h"

using namespace ash;

Context    ctx;
WidgetTree tree;


TEST(FlexLayout, Start)
{
  Flex flex{
      FlexProps{}, Widget{},
      Image{ImageProps{.size = SizeConstraint::absolute(100, 100)}},
      Image{ImageProps{.size = SizeConstraint::absolute(200, 200)}}};

  tree.build(ctx, flex);
  tree.layout(ctx, vec2{1920, 1080});

  EXPECT_EQ(flex.area.offset.x, 0);
  EXPECT_EQ(flex.area.offset.y, 0);
  EXPECT_EQ(flex.area.extent.x, 300);
  EXPECT_EQ(flex.area.extent.y, 200);

  stx::Span children = flex.get_children(ctx);

  EXPECT_EQ(children[0]->area.offset.x, 0);
  EXPECT_EQ(children[0]->area.offset.y, 0);
  EXPECT_EQ(children[0]->area.extent.x, 0);
  EXPECT_EQ(children[0]->area.extent.y, 0);

  EXPECT_EQ(children[1]->area.offset.x, 0);
  EXPECT_EQ(children[1]->area.offset.y, 0);
  EXPECT_EQ(children[1]->area.extent.x, 100);
  EXPECT_EQ(children[1]->area.extent.y, 100);

  EXPECT_EQ(children[2]->area.offset.x, 100);
  EXPECT_EQ(children[2]->area.offset.y, 0);
  EXPECT_EQ(children[2]->area.extent.x, 200);
  EXPECT_EQ(children[2]->area.extent.y, 200);
}

TEST(FlexLayout, SpaceAround)
{
  Flex flex{
      FlexProps{
          .direction   = Direction::H,
          .main_align  = MainAlign::SpaceAround,
          .cross_align = CrossAlign::Center},
      Widget{}, Image{ImageProps{.size = SizeConstraint::absolute(100, 100)}},
      Image{ImageProps{.size = SizeConstraint::absolute(200, 200)}}};

  tree.build(ctx, flex);
  tree.layout(ctx, vec2{1920, 1080});

  f32 space = (1920 - 300) / 6;
  f32 x     = 0;

  EXPECT_EQ(flex.area.offset.x, x);
  EXPECT_EQ(flex.area.offset.y, 0);
  EXPECT_EQ(flex.area.extent.x, 1920);
  EXPECT_EQ(flex.area.extent.y, 200);

  stx::Span children = flex.get_children(ctx);

  x = x + space;
  EXPECT_EQ(children[0]->area.offset.x, x);
  EXPECT_EQ(children[0]->area.offset.y, 100);
  EXPECT_EQ(children[0]->area.extent.x, 0);
  EXPECT_EQ(children[0]->area.extent.y, 0);

  x = x + 0 + space + space;
  EXPECT_EQ(children[1]->area.offset.x, x);
  EXPECT_EQ(children[1]->area.offset.y, 50);
  EXPECT_EQ(children[1]->area.extent.x, 100);
  EXPECT_EQ(children[1]->area.extent.y, 100);

  x = x + 100 + space + space;
  EXPECT_EQ(children[2]->area.offset.x, x);
  EXPECT_EQ(children[2]->area.offset.y, 0);
  EXPECT_EQ(children[2]->area.extent.x, 200);
  EXPECT_EQ(children[2]->area.extent.y, 200);
}

TEST(FlexLayout, SpaceEvenly)
{
  Flex flex{
      FlexProps{.direction   = Direction::H,
                .main_align  = MainAlign::SpaceEvenly,
                .cross_align = CrossAlign::Center},
      Widget{}, Image{ImageProps{.size = SizeConstraint::absolute(100, 100)}},
      Image{ImageProps{.size = SizeConstraint::absolute(200, 200)}}};

  tree.build(ctx, flex);
  tree.layout(ctx, vec2{1920, 1080});

  f32 x     = 0;
  f32 space = (1920 - 300) / 4.0f;

  EXPECT_EQ(flex.area.offset.x, x);
  EXPECT_EQ(flex.area.offset.y, 0);
  EXPECT_EQ(flex.area.extent.x, 1920);
  EXPECT_EQ(flex.area.extent.y, 200);

  stx::Span children = flex.get_children(ctx);

  x = x + space;
  EXPECT_EQ(children[0]->area.offset.x, x);
  EXPECT_EQ(children[0]->area.offset.y, 100);
  EXPECT_EQ(children[0]->area.extent.x, 0);
  EXPECT_EQ(children[0]->area.extent.y, 0);

  x = x + 0 + space;
  EXPECT_EQ(children[1]->area.offset.x, x);
  EXPECT_EQ(children[1]->area.offset.y, 50);
  EXPECT_EQ(children[1]->area.extent.x, 100);
  EXPECT_EQ(children[1]->area.extent.y, 100);

  x = x + 100 + space;
  EXPECT_EQ(children[2]->area.offset.x, x);
  EXPECT_EQ(children[2]->area.offset.y, 0);
  EXPECT_EQ(children[2]->area.extent.x, 200);
  EXPECT_EQ(children[2]->area.extent.y, 200);
}

TEST(FlexLayout, SpaceBetween)
{
  Flex flex{
      FlexProps{.direction   = Direction::H,
                .main_align  = MainAlign::SpaceBetween,
                .cross_align = CrossAlign::Center},
      Widget{}, Image{ImageProps{.size = SizeConstraint::absolute(100, 100)}},
      Image{ImageProps{.size = SizeConstraint::absolute(200, 200)}}};

  tree.build(ctx, flex);
  tree.layout(ctx, vec2{1920, 1080});

  f32 x     = 0;
  f32 space = (1920 - 300) / 2;

  EXPECT_EQ(flex.area.offset.x, x);
  EXPECT_EQ(flex.area.offset.y, 0);
  EXPECT_EQ(flex.area.extent.x, 1920);
  EXPECT_EQ(flex.area.extent.y, 200);

  stx::Span children = flex.get_children(ctx);

  EXPECT_EQ(children[0]->area.offset.x, x);
  EXPECT_EQ(children[0]->area.offset.y, 100);
  EXPECT_EQ(children[0]->area.extent.x, 0);
  EXPECT_EQ(children[0]->area.extent.y, 0);

  x = x + space;
  EXPECT_EQ(children[1]->area.offset.x, x);
  EXPECT_EQ(children[1]->area.offset.y, 50);
  EXPECT_EQ(children[1]->area.extent.x, 100);
  EXPECT_EQ(children[1]->area.extent.y, 100);

  x = x + 100 + space;
  EXPECT_EQ(children[2]->area.offset.x, x);
  EXPECT_EQ(children[2]->area.offset.y, 0);
  EXPECT_EQ(children[2]->area.extent.x, 200);
  EXPECT_EQ(children[2]->area.extent.y, 200);
}