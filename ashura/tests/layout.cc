
#include "ashura/layout.h"

#include "ashura/widgets/flex.h"
#include "ashura/widgets/image.h"
#include "gtest/gtest.h"

using namespace ash;

TEST(LayoutTest, Basic) {
  Flex flex{FlexProps{.width = constraint{.scale = 1},
                      .height = constraint{.scale = 1}},
            Widget{},
            Image{ImageProps{.width = constraint{.bias = 100},
                             .height = constraint{.bias = 100}}},
            Image{ImageProps{.width = constraint{.bias = 200},
                             .height = constraint{.bias = 200}}}};

  perform_layout(WidgetImpl::make(&flex),
                 rect{.offset = vec2{50, 50}, .extent = vec2{1920, 1080}});

  EXPECT_EQ(flex.area.offset.x, 50);
  EXPECT_EQ(flex.area.offset.y, 50);
  EXPECT_EQ(flex.area.extent.x, 300);
  EXPECT_EQ(flex.area.extent.y, 200);

  EXPECT_EQ(flex.get_children()[0]->area.offset.x, 50);
  EXPECT_EQ(flex.get_children()[0]->area.offset.y, 50);
  EXPECT_EQ(flex.get_children()[0]->area.extent.x, 0);
  EXPECT_EQ(flex.get_children()[0]->area.extent.y, 0);

  EXPECT_EQ(flex.get_children()[1]->area.offset.x, 50);
  EXPECT_EQ(flex.get_children()[1]->area.offset.y, 50);
  EXPECT_EQ(flex.get_children()[1]->area.extent.x, 100);
  EXPECT_EQ(flex.get_children()[1]->area.extent.y, 100);

  EXPECT_EQ(flex.get_children()[2]->area.offset.x, 150);
  EXPECT_EQ(flex.get_children()[2]->area.offset.y, 50);
  EXPECT_EQ(flex.get_children()[2]->area.extent.x, 200);
  EXPECT_EQ(flex.get_children()[2]->area.extent.y, 200);

  simdjson::dom::parser p;
  flex.restore(flex.save(p));
  flex.children[1]->restore(flex.children[1]->save(p));
}

TEST(LayoutTest, Space) {
  Flex flex{FlexProps{.direction = Direction::Row,
                      .main_align = MainAlign::SpaceEvenly,
                      .cross_align = CrossAlign::Center,
                      .width = constraint{.scale = 1},
                      .height = constraint{.scale = 1}},
            Widget{},
            Image{ImageProps{.width = constraint{.bias = 100},
                             .height = constraint{.bias = 100}}},
            Image{ImageProps{.width = constraint{.bias = 200},
                             .height = constraint{.bias = 200}}}};

  perform_layout(WidgetImpl::make(&flex),
                 rect{.offset = vec2{50, 50}, .extent = vec2{1920, 1080}});

  EXPECT_EQ(flex.area.offset.x, 50);
  EXPECT_EQ(flex.area.offset.y, 50);
  EXPECT_EQ(flex.area.extent.x, 1920);
  EXPECT_EQ(flex.area.extent.y, 1080);

  EXPECT_EQ(flex.get_children()[0]->area.offset.x, (1920 - 300) / 4.0f + 50);
  EXPECT_EQ(flex.get_children()[0]->area.offset.y, 150);
  EXPECT_EQ(flex.get_children()[0]->area.extent.x, 0);
  EXPECT_EQ(flex.get_children()[0]->area.extent.y, 0);

  EXPECT_EQ(flex.get_children()[1]->area.offset.x, (1920 - 300) / 2.0f + 50);
  EXPECT_EQ(flex.get_children()[1]->area.offset.y, 100);
  EXPECT_EQ(flex.get_children()[1]->area.extent.x, 100);
  EXPECT_EQ(flex.get_children()[1]->area.extent.y, 100);

  EXPECT_EQ(flex.get_children()[2]->area.offset.x,
            ((1920 - 300) / 4.0f) * 3 + 150);
  EXPECT_EQ(flex.get_children()[2]->area.offset.y, 50);
  EXPECT_EQ(flex.get_children()[2]->area.extent.x, 200);
  EXPECT_EQ(flex.get_children()[2]->area.extent.y, 200);
}