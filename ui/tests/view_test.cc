
#include "gtest/gtest.h"
#include "vlk/ui/layout_tree.h"
#include "vlk/ui/view_tree.h"
#include "vlk/ui/widget.h"
#include "vlk/ui/widget_builder.h"

using namespace vlk::ui;
using namespace vlk;

namespace view_test {

struct MockSized : public Widget {
  MockSized(Extent extent, stx::Option<ZIndex> const& z_index = stx::None,
            Padding padding = {})
      : Widget{WidgetType::Render} {
    Widget::init_is_flex(false);
    Widget::update_self_extent(SelfExtent{Constrain::absolute(extent.width),
                                          Constrain::absolute(extent.height)});
    Widget::update_padding(padding);
    Widget::init_z_index(z_index.clone());
  }
  ~MockSized() override {}
};

struct MockFlex : public Widget {
  MockFlex(std::initializer_list<Widget*> const& children,
           stx::Option<ZIndex> const& z_index = stx::None)
      : Widget{WidgetType::Render} {
    children_ = children;
    Widget::init_is_flex(true);
    Widget::update_children(children_);
    Widget::init_z_index(z_index.clone());
    Widget::update_self_extent(SelfExtent{Constrain{1.0f}, Constrain{1.0f}});
  }

  ~MockFlex() override {
    // no freeing
  }

  std::vector<Widget*> children_;
};

// this optimization would mean we can't move the widgets since the pointer
// address would change, even if it is a std::array
struct MockView : public Widget {
  MockView(Widget* child) : Widget{WidgetType::View} {
    child_ = child;
    Widget::init_is_flex(true);
    Widget::update_children(stx::Span<Widget*>(&child_, 1));
    Widget::update_flex(Flex{});
    Widget::update_self_extent(SelfExtent{Constrain{1.0f}, Constrain{1.0f}});
    Widget::update_view_extent(ViewExtent{Constrain{1.0f}, Constrain{1.0f}});
    Widget::update_padding(Padding{});
  }

  ~MockView() override {
    // no freeing
  }

  Widget* child_;
};

}  // namespace view_test

TEST(ViewTree, Hierarchy_And_Scrolling) {
  using namespace view_test;

  auto w1 = MockSized{Extent{20, 20}, stx::Some<ZIndex>(2)};
  auto w2 = MockSized{Extent{30, 50}};
  auto f1 = MockFlex{{&w1, &w2}};
  auto v1 = MockView{&f1};

  auto w3 = MockSized{Extent{30, 50}};
  auto v2 = MockView{&w3};

  auto froot = MockFlex{{&v1, &v2}, stx::Some<ZIndex>(5)};
  auto vroot = MockView{&froot};

  LayoutTree layout_tree;
  layout_tree.allot_extent(Extent{1920, 1080});
  layout_tree.build(vroot);

  ViewTree view_tree;
  view_tree.build(layout_tree.root_node);

  layout_tree.tick(std::chrono::nanoseconds(0));

  vroot.update_view_offset(ViewOffset{Constrain{0.0f, 10}, Constrain{0.0f}});

  view_tree.tick(std::chrono::nanoseconds(0));

  EXPECT_EQ(view_tree.root_view.layout_node->widget, &vroot);

  EXPECT_EQ(view_tree.root_view.screen_offset.x, 0);
  EXPECT_EQ(view_tree.root_view.screen_offset.y, 0);
  EXPECT_EQ(view_tree.root_view.effective_parent_view_offset.x, 0);
  EXPECT_EQ(view_tree.root_view.effective_parent_view_offset.y, 0);
  EXPECT_EQ(view_tree.root_view.parent, nullptr);
  EXPECT_EQ(view_tree.root_view.z_index, 0);

  EXPECT_EQ(view_tree.root_view.entries.size(), 1);
  EXPECT_EQ(view_tree.root_view.subviews.size(), 2);

  {
    // vroot
    EXPECT_EQ(view_tree.root_view.layout_node->widget, &vroot);
    auto screen_offset = view_tree.root_view.entries[0].screen_offset;
    EXPECT_EQ(screen_offset.x, 10);
    EXPECT_EQ(screen_offset.y, 0);
  }

  {
    // v1
    EXPECT_EQ(view_tree.root_view.subviews[0].layout_node->widget, &v1);
    auto screen_offset = view_tree.root_view.subviews[0].screen_offset;
    EXPECT_EQ(screen_offset.x, 10);
    EXPECT_EQ(screen_offset.y, 0);
  }

  // v1
  view_tree.root_view.subviews[0].layout_node->widget->update_view_offset(
      ViewOffset{
          Constrain{0.0f, 90, stx::i64_min, stx::i64_max, Clamp{0.0f, 200.0f}},
          Constrain{0.0f}});

  AssetManager asset_manager;

  WidgetSystemProxy::tick(*view_tree.root_view.subviews[0].layout_node->widget,
                          std::chrono::nanoseconds(0), asset_manager);

  view_tree.tick(std::chrono::nanoseconds(0));

  {
    // v1
    auto screen_offset = view_tree.root_view.subviews[0].screen_offset;
    EXPECT_EQ(screen_offset.x, 10);
    EXPECT_EQ(screen_offset.y, 0);
  }

  {
    // f1
    EXPECT_EQ(view_tree.root_view.subviews[0].entries[0].layout_node->widget,
              &f1);
    auto screen_offset =
        view_tree.root_view.subviews[0].entries[0].screen_offset;
    EXPECT_EQ(screen_offset.x, 100);
    EXPECT_EQ(screen_offset.y, 0);
  }
}

namespace view_test {

struct Body : public Widget {
  Body(Widget* child, ViewFit const& view_fit) {
    children_[0] = child;
    Widget::init_type(WidgetType::View);
    Widget::init_is_flex(true);
    Widget::update_children(children_);
    Widget::update_flex(Flex{});
    Widget::update_view_fit(view_fit);
    // we need a way to make self extent match view extent and our problems will
    // be solved
    Widget::update_self_extent(SelfExtent{Constrain{1.0f}, Constrain{1.0f}});
    Widget::update_view_extent(ViewExtent{Constrain{1.0f}, Constrain{1.0f}});
    Widget::update_padding(Padding{});
  }

  ~Body() override {
    // no freeing
  }

  Widget* children_[1];
};

}  // namespace view_test

TEST(BodyTest, UnconstrainedRoot) {
  using namespace view_test;
  auto w1 = MockSized{Extent{20, 20}};
  auto body = Body{&w1, ViewFit::Height};

  LayoutTree layout_tree;
  layout_tree.allot_extent(Extent{1920, stx::u32_max});
  layout_tree.build(body);

  layout_tree.tick(std::chrono::nanoseconds(0));

  auto const& node = layout_tree.root_node;
  EXPECT_EQ(node.self_extent.width, 1920);
  EXPECT_EQ(node.self_extent.height, 20);

  EXPECT_EQ(node.view_extent.width, 20);
  EXPECT_EQ(node.view_extent.height, 20);
}