

#include "gtest/gtest.h"
#include "vlk/ui/layout_tree.h"
#include "vlk/ui/widget.h"
#include "vlk/ui/widget_builder.h"

using namespace vlk::ui;
using namespace vlk;

inline namespace layout_test {

struct MockSized : public Widget {
  MockSized(Extent extent, Padding padding = {}) : Widget{WidgetType::Render} {
    Widget::init_is_flex(false);
    Widget::update_self_extent(SelfExtent::absolute(extent));
    Widget::update_padding(padding);
  }
  ~MockSized() override {}
};

struct MockFlex : public Widget {
  MockFlex(std::initializer_list<Widget*> const& children, Flex const& flex,
           SelfExtent const& self_extent, Padding padding)
      : Widget{WidgetType::Render} {
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

struct Body : public Widget {
  Body(Widget* child, ViewFit const& view_fit) {
    children_[0] = child;
    Widget::init_type(WidgetType::View);
    Widget::init_is_flex(true);
    Widget::update_children(children_);
    // flex shrink to original size
    Widget::update_flex(Flex{});
    // fit the view_extent onto the self_extent
    Widget::update_view_fit(view_fit);
    // we need a way to make self extent match view extent and our problems will
    // be solved
    Widget::update_self_extent(SelfExtent::relative(1.0f, 1.0f));
    Widget::update_view_extent(ViewExtent::relative(1.0f, 1.0f));
    Widget::update_padding(Padding{});
  }

  ~Body() override {
    // no freeing
  }

  Widget* children_[1];
};

}  // namespace layout_test

TEST(LayoutTest, Sized) {
  MockSized sized{Extent{20, 75}, Padding::all(0)};

  Body body = Body{&sized, ViewFit::Width | ViewFit::Height};

  LayoutTree tree;

  tree.build(body);
  tree.allot_extent(Extent{stx::u32_max, stx::u32_max});
  tree.tick(std::chrono::nanoseconds(0));

  auto& node = tree.root_node.children[0];

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

  Body body = Body{&sized, ViewFit::Width | ViewFit::Height};

  tree.build(body);
  tree.allot_extent(Extent{stx::u32_max, stx::u32_max});
  tree.tick(std::chrono::nanoseconds(0));

  auto& node = tree.root_node.children[0];

  EXPECT_EQ(node.widget, static_cast<Widget*>(&sized));
  EXPECT_EQ(node.self_extent.width, 20);
  EXPECT_EQ(node.self_extent.height, 75);
  EXPECT_FALSE(node.children.size() > 0);
  EXPECT_EQ(node.parent_offset, Offset{});
  EXPECT_EQ(node.parent_view_offset, Offset{});
  EXPECT_EQ(node.view_extent, node.self_extent);
}

TEST(LayoutTest, Flex_MainShrink_CrossShrink) {
  MockFlex flex{{new MockSized{Extent{20, 20}}, new MockSized{Extent{30, 50}}},
                Flex{Direction::Row, Wrap::Wrap, MainAlign::Start,
                     CrossAlign::Start, Fit::Shrink, Fit::Shrink},
                SelfExtent::relative(1.0f, 1.0f),
                Padding::all(0)};

  LayoutTree tree;

  tree.build(flex);
  tree.allot_extent(Extent{stx::u32_max, stx::u32_max});
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
  MockFlex flex{{new MockSized{Extent{20, 20}}, new MockSized{Extent{30, 50}}},
                Flex{Direction::Column, Wrap::Wrap, MainAlign::Start,
                     CrossAlign::Start, Fit::Shrink, Fit::Shrink},
                SelfExtent::relative(1.0f, 1.0f),
                Padding::all(0)};

  LayoutTree tree;

  tree.build(flex);
  tree.allot_extent(Extent{stx::u32_max, stx::u32_max});
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
      Flex{Direction::Row, Wrap::Wrap, MainAlign::SpaceEvenly,
           CrossAlign::Center, Fit::Shrink, Fit::Shrink},
      SelfExtent{Constrain{1.0f, 0, 0, 720}, Constrain{1.0f, 0, 0, 720}},
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
        Flex{Direction::Row, Wrap::Wrap, MainAlign::Start, CrossAlign::Start,
             Fit::Shrink, Fit::Shrink},
        SelfExtent::relative(1.0f, 1.0f),
        Padding::all(15)};

    LayoutTree tree;

    tree.build(flex);
    tree.allot_extent(Extent{stx::u32_max, stx::u32_max});
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
    MockFlex flex{{new MockSized{Extent{20, 20}}},
                  Flex{Direction::Row, Wrap::Wrap, MainAlign::Start,
                       CrossAlign::Start, Fit::Shrink, Fit::Shrink},
                  SelfExtent::relative(1.0f, 1.0f),
                  Padding::all(15)};

    LayoutTree tree;

    tree.build(flex);
    tree.allot_extent(Extent{stx::u32_max, stx::u32_max});
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
    MockFlex flex{{},
                  Flex{Direction::Row, Wrap::Wrap, MainAlign::Start,
                       CrossAlign::Start, Fit::Shrink, Fit::Shrink},
                  SelfExtent::relative(1.0f, 1.0f),
                  Padding::all(15)};

    LayoutTree tree;

    tree.build(flex);
    tree.allot_extent(Extent{stx::u32_max, stx::u32_max});
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
      Flex{Direction::Row, Wrap::Wrap, MainAlign::Start, CrossAlign::Start,
           Fit::Expand, Fit::Shrink},
      SelfExtent{Constrain{1.0f, 0, 0, 720}, Constrain{1.0f, 0, 0, 720}},
      Padding::all(0)};

  LayoutTree tree;

  Body body = Body{&flex, ViewFit::Width | ViewFit::Height};

  tree.build(body);
  tree.allot_extent(Extent{stx::u32_max, stx::u32_max});
  tree.tick(std::chrono::nanoseconds(0));

  auto& node = tree.root_node.children[0];

  EXPECT_EQ(node.widget, static_cast<Widget*>(&flex));
  EXPECT_EQ(node.self_extent.width, 720);
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
  MockFlex flex{{new MockSized{Extent{20, 20}}, new MockSized{Extent{20, 20}}},
                Flex{Direction::Row, Wrap::Wrap, MainAlign::Start,
                     CrossAlign::Start, Fit::Expand, Fit::Shrink},
                SelfExtent::relative(1.0f, 1.0f),
                Padding::all(0)};

  LayoutTree tree;

  tree.build(flex);
  tree.allot_extent(Extent{1920, stx::u32_max});
  tree.tick(std::chrono::nanoseconds(0));

  auto& node = tree.root_node;

  EXPECT_EQ(node.widget, static_cast<Widget*>(&flex));
  EXPECT_EQ(node.self_extent.width, 1920);
  EXPECT_EQ(node.self_extent.height, 20);
  EXPECT_GT(node.children.size(), 0);
  EXPECT_EQ(node.parent_offset, Offset{});
  EXPECT_EQ(node.parent_view_offset, Offset{});
  EXPECT_EQ(node.view_extent, node.self_extent);
}

TEST(LayoutTest, Flex_Shrink) {
  MockFlex flex{{new MockSized{Extent{20, 20}}, new MockSized{Extent{20, 20}}},
                Flex{Direction::Row, Wrap::Wrap, MainAlign::Start,
                     CrossAlign::Start, Fit::Shrink, Fit::Shrink},
                SelfExtent::relative(1.0f, 1.0f),
                Padding::all(0)};

  LayoutTree tree;

  tree.build(flex);
  tree.allot_extent(Extent{stx::u32_max, stx::u32_max});
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
  MockFlex flex{{new MockSized{Extent{20, 20}}, new MockSized{Extent{30, 50}}},
                Flex{Direction::Row, Wrap::Wrap, MainAlign::Start,
                     CrossAlign::Start, Fit::Shrink, Fit::Shrink},
                SelfExtent::relative(1.0f, 1.0f),
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
  MockFlex flex{{new MockSized{Extent{20, 20}}, new MockSized{Extent{30, 50}}},
                Flex{Direction::Row, Wrap::Wrap, MainAlign::Start,
                     CrossAlign::Start, Fit::Expand, Fit::Expand},
                SelfExtent::relative(1.0f, 1.0f),
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
