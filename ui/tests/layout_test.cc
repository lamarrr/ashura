

#include "vlk/ui/layout_tree.h"
#include "vlk/ui/widget.h"
#include "vlk/ui/widget_builder.h"

#include "gtest/gtest.h"

using namespace vlk::ui;
using namespace vlk;

struct MockSized : public Widget {
  MockSized(Extent extent, Padding padding = {}) : Widget{Type::Render} {
    Widget::init_is_flex(false);
    Widget::update_self_extent(SelfExtent{Constrain::absolute(extent.width),
                                          Constrain::absolute(extent.height)});
    Widget::update_padding(padding);
  }
  ~MockSized() override {}
};

struct MockFlex : public Widget {
  MockFlex(std::initializer_list<Widget*> const& children, Flex const& flex,
           SelfExtent const& self_extent, Padding padding)
      : Widget{Type::Render} {
    children_ = children;
    Widget::init_is_flex(true);
    Widget::update_children(children_);
    Widget::update_flex(flex);
    Widget::update_self_extent(self_extent);
    Widget::update_padding(padding);
  }

  ~MockFlex() override {
    for (auto& child : children_) {
      delete child;
    }
  }

  std::vector<Widget*> children_;
};

TEST(LayoutTest, Sized) {
  MockSized sized{Extent{20, 75}, Padding::all(0)};

  LayoutTree tree;

  tree.build(sized);
  tree.allot_extent(Extent{u32_max, u32_max});
  tree.tick(std::chrono::nanoseconds(0));

  auto& node = tree.root_node;

  EXPECT_EQ(node.widget, static_cast<Widget*>(&sized));
  EXPECT_EQ(node.self_extent.width, 20);
  EXPECT_EQ(node.self_extent.height, 75);
  EXPECT_FALSE(node.children.size() > 0);
  EXPECT_EQ(node.parent_offset, Offset{});
  EXPECT_EQ(node.parent_view_offset, Offset{});
  EXPECT_EQ(node.view_extent, node.self_extent);
}

TEST(LayoutTest, SizedPadded) {
  MockSized sized{Extent{20, 75}, Padding::all(20)};

  LayoutTree tree;

  tree.build(sized);
  tree.allot_extent(Extent{u32_max, u32_max});
  tree.tick(std::chrono::nanoseconds(0));

  auto& node = tree.root_node;

  EXPECT_EQ(node.widget, static_cast<Widget*>(&sized));
  EXPECT_EQ(node.self_extent.width, 20);
  EXPECT_EQ(node.self_extent.height, 75);
  EXPECT_FALSE(node.children.size() > 0);
  EXPECT_EQ(node.parent_offset, Offset{});
  EXPECT_EQ(node.parent_view_offset, Offset{});
  EXPECT_EQ(node.view_extent, node.self_extent);
}

TEST(LayoutTest, Flex_MainShrink_CrossShrink) {
  MockFlex flex{
      {new MockSized{Extent{20, 20}}, new MockSized{Extent{30, 50}}},
      Flex{Flex::Direction::Row, Flex::Wrap::Wrap, Flex::MainAlign::Start,
           Flex::CrossAlign::Start, Flex::Fit::Shrink, Flex::Fit::Shrink},
      SelfExtent{Constrain::relative(1.0f), Constrain::relative(1.0f)},
      Padding::all(0)};

  LayoutTree tree;

  tree.build(flex);
  tree.allot_extent(Extent{u32_max, u32_max});
  tree.tick(std::chrono::nanoseconds(0));

  auto& node = tree.root_node;

  EXPECT_EQ(node.widget, static_cast<Widget*>(&flex));
  EXPECT_EQ(node.self_extent.width, 50);
  EXPECT_EQ(node.self_extent.height, 50);
  EXPECT_GT(node.children.size(), 0);
  EXPECT_EQ(node.parent_offset, Offset{});
  EXPECT_EQ(node.parent_view_offset, Offset{});
  EXPECT_EQ(node.view_extent, node.self_extent);

  {
    auto& child = node.children[0];
    EXPECT_EQ(child.self_extent.width, 20);
    EXPECT_EQ(child.self_extent.height, 20);
    EXPECT_EQ(child.parent_offset.x, 0);
    EXPECT_EQ(child.parent_offset.y, 0);
  }

  {
    auto& child = node.children[1];
    EXPECT_EQ(child.self_extent.width, 30);
    EXPECT_EQ(child.self_extent.height, 50);
    EXPECT_EQ(child.parent_offset.x, 20);
    EXPECT_EQ(child.parent_offset.y, 0);
  }
}

TEST(LayoutTest, Flex_Column) {
  MockFlex flex{
      {new MockSized{Extent{20, 20}}, new MockSized{Extent{30, 50}}},
      Flex{Flex::Direction::Column, Flex::Wrap::Wrap, Flex::MainAlign::Start,
           Flex::CrossAlign::Start, Flex::Fit::Shrink, Flex::Fit::Shrink},
      SelfExtent{Constrain::relative(1.0f), Constrain::relative(1.0f)},
      Padding::all(0)};

  LayoutTree tree;

  tree.build(flex);
  tree.allot_extent(Extent{u32_max, u32_max});
  tree.tick(std::chrono::nanoseconds(0));

  auto& node = tree.root_node;

  EXPECT_EQ(node.widget, static_cast<Widget*>(&flex));
  EXPECT_EQ(node.self_extent.width, 30);
  EXPECT_EQ(node.self_extent.height, 70);
  EXPECT_GT(node.children.size(), 0);
  EXPECT_EQ(node.parent_offset, Offset{});
  EXPECT_EQ(node.parent_view_offset, Offset{});
  EXPECT_EQ(node.view_extent, node.self_extent);

  {
    auto& child = node.children[0];
    EXPECT_EQ(child.self_extent.width, 20);
    EXPECT_EQ(child.self_extent.height, 20);
    EXPECT_EQ(child.parent_offset.x, 0);
    EXPECT_EQ(child.parent_offset.y, 0);
  }

  {
    auto& child = node.children[1];
    EXPECT_EQ(child.self_extent.width, 30);
    EXPECT_EQ(child.self_extent.height, 50);
    EXPECT_EQ(child.parent_offset.x, 0);
    EXPECT_EQ(child.parent_offset.y, 20);
  }
}

TEST(LayoutTest, Flex_Row_MainAlignCenter_CrossAlignCenter) {
  MockFlex flex{
      {new MockSized{Extent{20, 20}}, new MockSized{Extent{30, 50}}},
      Flex{Flex::Direction::Row, Flex::Wrap::Wrap, Flex::MainAlign::SpaceEvenly,
           Flex::CrossAlign::Center, Flex::Fit::Shrink, Flex::Fit::Shrink},
      SelfExtent{Constrain::relative(1.0f), Constrain::relative(1.0f)},
      Padding::all(0)};

  LayoutTree tree;

  tree.build(flex);
  tree.allot_extent(Extent{100, 100});
  tree.tick(std::chrono::nanoseconds(0));

  auto& node = tree.root_node;

  EXPECT_EQ(node.widget, static_cast<Widget*>(&flex));
  EXPECT_EQ(node.self_extent.width, 100);
  EXPECT_EQ(node.self_extent.height, 100);
  EXPECT_GT(node.children.size(), 0);
  EXPECT_EQ(node.parent_offset, Offset{});
  EXPECT_EQ(node.parent_view_offset, Offset{});
  EXPECT_EQ(node.view_extent, node.self_extent);

  {
    auto& child = node.children[0];
    EXPECT_EQ(child.self_extent.width, 20);
    EXPECT_EQ(child.self_extent.height, 20);
    EXPECT_EQ(child.parent_offset.x, 50 / 3);
    EXPECT_EQ(child.parent_offset.y, 15);
  }

  {
    auto& child = node.children[1];
    EXPECT_EQ(child.self_extent.width, 30);
    EXPECT_EQ(child.self_extent.height, 50);
    EXPECT_EQ(child.parent_offset.x, (50 / 3) + 20 + (50 / 3));
    EXPECT_EQ(child.parent_offset.y, 0);
  }
}

TEST(LayoutTest, Flex_Padded) {
  {
    MockFlex flex{
        {new MockSized{Extent{20, 20}}, new MockSized{Extent{30, 50}}},
        Flex{Flex::Direction::Row, Flex::Wrap::Wrap, Flex::MainAlign::Start,
             Flex::CrossAlign::Start, Flex::Fit::Shrink, Flex::Fit::Shrink},
        SelfExtent{Constrain::relative(1.0f), Constrain::relative(1.0f)},
        Padding::all(15)};

    LayoutTree tree;

    tree.build(flex);
    tree.allot_extent(Extent{u32_max, u32_max});
    tree.tick(std::chrono::nanoseconds(0));

    auto& node = tree.root_node;

    EXPECT_EQ(node.widget, static_cast<Widget*>(&flex));
    EXPECT_EQ(node.self_extent.width, 80);
    EXPECT_EQ(node.self_extent.height, 80);
    EXPECT_GT(node.children.size(), 0);
    EXPECT_EQ(node.parent_offset, Offset{});
    EXPECT_EQ(node.parent_view_offset, Offset{});
    EXPECT_EQ(node.view_extent, node.self_extent);

    {
      auto& child = node.children[0];
      EXPECT_EQ(child.self_extent.width, 20);
      EXPECT_EQ(child.self_extent.height, 20);
      EXPECT_EQ(child.parent_offset.x, 15);
      EXPECT_EQ(child.parent_offset.y, 15);
    }

    {
      auto& child = node.children[1];
      EXPECT_EQ(child.self_extent.width, 30);
      EXPECT_EQ(child.self_extent.height, 50);
      EXPECT_EQ(child.parent_offset.x, 35);
      EXPECT_EQ(child.parent_offset.y, 15);
    }
  }

  // one child
  {
    MockFlex flex{
        {new MockSized{Extent{20, 20}}},
        Flex{Flex::Direction::Row, Flex::Wrap::Wrap, Flex::MainAlign::Start,
             Flex::CrossAlign::Start, Flex::Fit::Shrink, Flex::Fit::Shrink},
        SelfExtent{Constrain::relative(1.0f), Constrain::relative(1.0f)},
        Padding::all(15)};

    LayoutTree tree;

    tree.build(flex);
    tree.allot_extent(Extent{u32_max, u32_max});
    tree.tick(std::chrono::nanoseconds(0));

    auto& node = tree.root_node;

    EXPECT_EQ(node.widget, static_cast<Widget*>(&flex));
    EXPECT_EQ(node.self_extent.width, 50);
    EXPECT_EQ(node.self_extent.height, 50);
    EXPECT_GT(node.children.size(), 0);
    EXPECT_EQ(node.parent_offset, Offset{});
    EXPECT_EQ(node.parent_view_offset, Offset{});
    EXPECT_EQ(node.view_extent, node.self_extent);

    {
      auto& child = node.children[0];
      EXPECT_EQ(child.self_extent.width, 20);
      EXPECT_EQ(child.self_extent.height, 20);
      EXPECT_EQ(child.parent_offset.x, 15);
      EXPECT_EQ(child.parent_offset.y, 15);
    }
  }

  // no child
  {
    MockFlex flex{
        {},
        Flex{Flex::Direction::Row, Flex::Wrap::Wrap, Flex::MainAlign::Start,
             Flex::CrossAlign::Start, Flex::Fit::Shrink, Flex::Fit::Shrink},
        SelfExtent{Constrain::relative(1.0f), Constrain::relative(1.0f)},
        Padding::all(15)};

    LayoutTree tree;

    tree.build(flex);
    tree.allot_extent(Extent{u32_max, u32_max});
    tree.tick(std::chrono::nanoseconds(0));

    auto& node = tree.root_node;

    EXPECT_EQ(node.widget, static_cast<Widget*>(&flex));
    EXPECT_EQ(node.self_extent.width, 30);
    EXPECT_EQ(node.self_extent.height, 30);
    EXPECT_FALSE(node.children.size() > 0);
    EXPECT_EQ(node.parent_offset, Offset{});
    EXPECT_EQ(node.parent_view_offset, Offset{});
    EXPECT_EQ(node.view_extent, node.self_extent);
  }
}

TEST(LayoutTest, Flex_MainExpand_CrossShrink) {
  MockFlex flex{
      {new MockSized{Extent{20, 20}}, new MockSized{Extent{30, 50}}},
      Flex{Flex::Direction::Row, Flex::Wrap::Wrap, Flex::MainAlign::Start,
           Flex::CrossAlign::Start, Flex::Fit::Expand, Flex::Fit::Shrink},
      SelfExtent{Constrain::relative(1.0f), Constrain::relative(1.0f)},
      Padding::all(0)};

  LayoutTree tree;

  tree.build(flex);
  tree.allot_extent(Extent{u32_max, u32_max});
  tree.tick(std::chrono::nanoseconds(0));

  auto& node = tree.root_node;

  EXPECT_EQ(node.widget, static_cast<Widget*>(&flex));
  EXPECT_EQ(node.self_extent.width, u32_max);
  EXPECT_EQ(node.self_extent.height, 50);
  EXPECT_GT(node.children.size(), 0);
  EXPECT_EQ(node.parent_offset, Offset{});
  EXPECT_EQ(node.parent_view_offset, Offset{});
  EXPECT_EQ(node.view_extent, node.self_extent);

  {
    auto& child = node.children[0];
    EXPECT_EQ(child.self_extent.width, 20);
    EXPECT_EQ(child.self_extent.height, 20);
    EXPECT_EQ(child.parent_offset.x, 0);
    EXPECT_EQ(child.parent_offset.y, 0);
  }

  {
    auto& child = node.children[1];
    EXPECT_EQ(child.self_extent.width, 30);
    EXPECT_EQ(child.self_extent.height, 50);
    EXPECT_EQ(child.parent_offset.x, 20);
    EXPECT_EQ(child.parent_offset.y, 0);
  }
}

TEST(LayoutTest, Flex_MainExpand_MainExpand) {
  MockFlex flex{
      {new MockSized{Extent{20, 20}}, new MockSized{Extent{20, 20}}},
      Flex{Flex::Direction::Row, Flex::Wrap::Wrap, Flex::MainAlign::Start,
           Flex::CrossAlign::Start, Flex::Fit::Expand, Flex::Fit::Shrink},
      SelfExtent{Constrain::relative(1.0f), Constrain::relative(1.0f)},
      Padding::all(0)};

  LayoutTree tree;

  tree.build(flex);
  tree.allot_extent(Extent{u32_max, u32_max});
  tree.tick(std::chrono::nanoseconds(0));

  auto& node = tree.root_node;

  EXPECT_EQ(node.widget, static_cast<Widget*>(&flex));
  EXPECT_EQ(node.self_extent.width, u32_max);
  EXPECT_EQ(node.self_extent.height, 20);
  EXPECT_GT(node.children.size(), 0);
  EXPECT_EQ(node.parent_offset, Offset{});
  EXPECT_EQ(node.parent_view_offset, Offset{});
  EXPECT_EQ(node.view_extent, node.self_extent);
}

TEST(LayoutTest, Flex_Shrink) {
  MockFlex flex{
      {new MockSized{Extent{20, 20}}, new MockSized{Extent{20, 20}}},
      Flex{Flex::Direction::Row, Flex::Wrap::Wrap, Flex::MainAlign::Start,
           Flex::CrossAlign::Start, Flex::Fit::Shrink, Flex::Fit::Shrink},
      SelfExtent{Constrain::relative(1.0f), Constrain::relative(1.0f)},
      Padding::all(0)};

  LayoutTree tree;

  tree.build(flex);
  tree.allot_extent(Extent{u32_max, u32_max});
  tree.tick(std::chrono::nanoseconds(0));

  auto& node = tree.root_node;

  EXPECT_EQ(node.widget, static_cast<Widget*>(&flex));
  EXPECT_EQ(node.self_extent.width, 40);
  EXPECT_EQ(node.self_extent.height, 20);
  EXPECT_GT(node.children.size(), 0);
  EXPECT_EQ(node.parent_offset, Offset{});
  EXPECT_EQ(node.parent_view_offset, Offset{});
  EXPECT_EQ(node.view_extent, node.self_extent);
}

TEST(LayoutTest, Flex_WrapOverflow_Shrink) {
  MockFlex flex{
      {new MockSized{Extent{20, 20}}, new MockSized{Extent{30, 50}}},
      Flex{Flex::Direction::Row, Flex::Wrap::Wrap, Flex::MainAlign::Start,
           Flex::CrossAlign::Start, Flex::Fit::Shrink, Flex::Fit::Shrink},
      SelfExtent{Constrain::relative(1.0f), Constrain::relative(1.0f)},
      Padding::all(0)};

  LayoutTree tree;

  tree.build(flex);
  tree.allot_extent(Extent{20, 20});
  tree.tick(std::chrono::nanoseconds(0));

  auto& node = tree.root_node;

  EXPECT_EQ(node.widget, static_cast<Widget*>(&flex));
  EXPECT_EQ(node.self_extent.width, 20);
  EXPECT_EQ(node.self_extent.height, 20);
  EXPECT_GT(node.children.size(), 0);
  EXPECT_EQ(node.parent_offset, Offset{});
  EXPECT_EQ(node.parent_view_offset, Offset{});
  EXPECT_EQ(node.view_extent, node.self_extent);

  {
    auto& child = node.children[0];
    EXPECT_EQ(child.self_extent.width, 20);
    EXPECT_EQ(child.self_extent.height, 20);
    EXPECT_EQ(child.parent_offset.x, 0);
    EXPECT_EQ(child.parent_offset.y, 0);
  }

  {
    auto& child = node.children[1];
    EXPECT_EQ(child.self_extent.width, 20);
    EXPECT_EQ(child.self_extent.height, 20);
    EXPECT_EQ(child.parent_offset.x, 0);
    EXPECT_EQ(child.parent_offset.y, 20);
  }
}

TEST(LayoutTest, Flex_WrapOverflow_Expand) {
  MockFlex flex{
      {new MockSized{Extent{20, 20}}, new MockSized{Extent{30, 50}}},
      Flex{Flex::Direction::Row, Flex::Wrap::Wrap, Flex::MainAlign::Start,
           Flex::CrossAlign::Start, Flex::Fit::Expand, Flex::Fit::Expand},
      SelfExtent{Constrain::relative(1.0f), Constrain::relative(1.0f)},
      Padding::all(0)};

  LayoutTree tree;

  tree.build(flex);
  tree.allot_extent(Extent{20, 20});
  tree.tick(std::chrono::nanoseconds(0));

  auto& node = tree.root_node;

  EXPECT_EQ(node.widget, static_cast<Widget*>(&flex));
  EXPECT_EQ(node.self_extent.width, 20);
  EXPECT_EQ(node.self_extent.height, 20);
  EXPECT_GT(node.children.size(), 0);
  EXPECT_EQ(node.parent_offset, Offset{});
  EXPECT_EQ(node.parent_view_offset, Offset{});
  EXPECT_EQ(node.view_extent, node.self_extent);

  {
    auto& child = node.children[0];
    EXPECT_EQ(child.self_extent.width, 20);
    EXPECT_EQ(child.self_extent.height, 20);
    EXPECT_EQ(child.parent_offset.x, 0);
    EXPECT_EQ(child.parent_offset.y, 0);
  }

  {
    auto& child = node.children[1];
    EXPECT_EQ(child.self_extent.width, 20);
    EXPECT_EQ(child.self_extent.height, 20);
    EXPECT_EQ(child.parent_offset.x, 0);
    EXPECT_EQ(child.parent_offset.y, 20);
  }
}
