
#include "ashura/layout.h"

#include "ashura/widgets/flex.h"
#include "ashura/widgets/image.h"
#include "gtest/gtest.h"

using namespace ash;

TEST(LayoutTest, Start) {
  Flex flex{FlexProps{.width = constraint{.scale = 1},
                      .height = constraint{.scale = 1}},
            Widget{},
            Image{ImageProps{.width = constraint{.bias = 100},
                             .height = constraint{.bias = 100}}},
            Image{ImageProps{.width = constraint{.bias = 200},
                             .height = constraint{.bias = 200}}}};

  perform_layout(&flex,
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
}

TEST(ImageTest, SerializationDeserialization) {
  Image im{ImageProps{.width = constraint{.bias = 100},
                      .height = constraint{.bias = 100},
                      .border_radius = vec4{1, 2, 3, 4},
                      .alt = stx::string::make_static("Hello")}};

  simdjson::dom::parser p;
  im.restore(im.save(p));

  EXPECT_EQ(im.props.alt, "Hello");
  EXPECT_EQ(im.props.width.bias, 100);
  EXPECT_EQ(im.props.height.bias, 100);
  EXPECT_EQ(im.props.border_radius, (vec4{1, 2, 3, 4}));
}

TEST(LayoutTest, SpaceAround) {
  Flex flex{FlexProps{.direction = Direction::Row,
                      .main_align = MainAlign::SpaceAround,
                      .cross_align = CrossAlign::Center,
                      .width = constraint{.scale = 1},
                      .height = constraint{.scale = 1}},
            Widget{},
            Image{ImageProps{.width = constraint{.bias = 100},
                             .height = constraint{.bias = 100}}},
            Image{ImageProps{.width = constraint{.bias = 200},
                             .height = constraint{.bias = 200}}}};

  perform_layout(&flex,
                 rect{.offset = vec2{50, 50}, .extent = vec2{1920, 1080}});

  f32 space = (1920 - 300) / 6;
  f32 x = 50;

  EXPECT_EQ(flex.area.offset.x, x);
  EXPECT_EQ(flex.area.offset.y, 50);
  EXPECT_EQ(flex.area.extent.x, 1920);
  EXPECT_EQ(flex.area.extent.y, 1080);

  x = x + space;
  EXPECT_EQ(flex.get_children()[0]->area.offset.x, x);
  EXPECT_EQ(flex.get_children()[0]->area.offset.y, 150);
  EXPECT_EQ(flex.get_children()[0]->area.extent.x, 0);
  EXPECT_EQ(flex.get_children()[0]->area.extent.y, 0);

  x = x + 0 + space + space;
  EXPECT_EQ(flex.get_children()[1]->area.offset.x, x);
  EXPECT_EQ(flex.get_children()[1]->area.offset.y, 100);
  EXPECT_EQ(flex.get_children()[1]->area.extent.x, 100);
  EXPECT_EQ(flex.get_children()[1]->area.extent.y, 100);

  x = x + 100 + space + space;
  EXPECT_EQ(flex.get_children()[2]->area.offset.x, x);
  EXPECT_EQ(flex.get_children()[2]->area.offset.y, 50);
  EXPECT_EQ(flex.get_children()[2]->area.extent.x, 200);
  EXPECT_EQ(flex.get_children()[2]->area.extent.y, 200);
}

TEST(LayoutTest, SpaceEvenly) {
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

  perform_layout(&flex,
                 rect{.offset = vec2{50, 50}, .extent = vec2{1920, 1080}});

  f32 x = 50;
  f32 space = (1920 - 300) / 4.0f;

  EXPECT_EQ(flex.area.offset.x, x);
  EXPECT_EQ(flex.area.offset.y, 50);
  EXPECT_EQ(flex.area.extent.x, 1920);
  EXPECT_EQ(flex.area.extent.y, 1080);

  x = x + space;
  EXPECT_EQ(flex.get_children()[0]->area.offset.x, x);
  EXPECT_EQ(flex.get_children()[0]->area.offset.y, 150);
  EXPECT_EQ(flex.get_children()[0]->area.extent.x, 0);
  EXPECT_EQ(flex.get_children()[0]->area.extent.y, 0);

  x = x + 0 + space;
  EXPECT_EQ(flex.get_children()[1]->area.offset.x, x);
  EXPECT_EQ(flex.get_children()[1]->area.offset.y, 100);
  EXPECT_EQ(flex.get_children()[1]->area.extent.x, 100);
  EXPECT_EQ(flex.get_children()[1]->area.extent.y, 100);

  x = x + 100 + space;
  EXPECT_EQ(flex.get_children()[2]->area.offset.x, x);
  EXPECT_EQ(flex.get_children()[2]->area.offset.y, 50);
  EXPECT_EQ(flex.get_children()[2]->area.extent.x, 200);
  EXPECT_EQ(flex.get_children()[2]->area.extent.y, 200);
}

TEST(LayoutTest, SpaceBetween) {
  Flex flex{FlexProps{.direction = Direction::Row,
                      .main_align = MainAlign::SpaceBetween,
                      .cross_align = CrossAlign::Center,
                      .width = constraint{.scale = 1},
                      .height = constraint{.scale = 1}},
            Widget{},
            Image{ImageProps{.width = constraint{.bias = 100},
                             .height = constraint{.bias = 100}}},
            Image{ImageProps{.width = constraint{.bias = 200},
                             .height = constraint{.bias = 200}}}};

  perform_layout(&flex,
                 rect{.offset = vec2{50, 50}, .extent = vec2{1920, 1080}});

  f32 x = 50;
  f32 space = (1920 - 300) / 2;

  EXPECT_EQ(flex.area.offset.x, x);
  EXPECT_EQ(flex.area.offset.y, 50);
  EXPECT_EQ(flex.area.extent.x, 1920);
  EXPECT_EQ(flex.area.extent.y, 1080);

  EXPECT_EQ(flex.get_children()[0]->area.offset.x, x);
  EXPECT_EQ(flex.get_children()[0]->area.offset.y, 150);
  EXPECT_EQ(flex.get_children()[0]->area.extent.x, 0);
  EXPECT_EQ(flex.get_children()[0]->area.extent.y, 0);

  x = x + space;
  EXPECT_EQ(flex.get_children()[1]->area.offset.x, x);
  EXPECT_EQ(flex.get_children()[1]->area.offset.y, 100);
  EXPECT_EQ(flex.get_children()[1]->area.extent.x, 100);
  EXPECT_EQ(flex.get_children()[1]->area.extent.y, 100);

  x = x + 100 + space;
  EXPECT_EQ(flex.get_children()[2]->area.offset.x, x);
  EXPECT_EQ(flex.get_children()[2]->area.offset.y, 50);
  EXPECT_EQ(flex.get_children()[2]->area.extent.x, 200);
  EXPECT_EQ(flex.get_children()[2]->area.extent.y, 200);
}
