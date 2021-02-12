#include <algorithm>
#include <iostream>
#include <numeric>

#include "gtest/gtest.h"

#include "vlk/ui/compositor.h"

#include "vlk/ui/widget.h"
#include "vlk/ui/widget_utils.h"

using namespace vlk::ui;

constexpr inline SelfLayout image_sizing() { return SelfLayout{}; }

constexpr inline SelfLayout make_column_layout() {
  return SelfLayout{
      // uses all the allotted width
      IndependentParameters{},
      // uses the max child height and allot the whole height to the children
      DependentParameters{}};
}

inline void build_column_children_layout(
    stx::Span<ChildLayout> const &children_area,
    stx::Span<float const> const &flex_factor) {
  VLK_DEBUG_ENSURE(std::all_of(flex_factor.begin(), flex_factor.end(),
                               [](float flex) { return flex >= 0.0f; }) ||
                       std::all_of(flex_factor.begin(), flex_factor.end(),
                                   [](float flex) { return flex <= 0.0f; }),
                   "Flex factors must be all-negative or all-positive");
  VLK_DEBUG_ENSURE(children_area.size() == flex_factor.size());

  auto total_flex =
      std::accumulate(flex_factor.begin(), flex_factor.end(), 0.0f);

  size_t children_count = flex_factor.size();

  float previous_percentages = 0.0f;

  for (size_t i = 0; i < children_count; i++) {
    float const percentage =
        flex_factor[i] / (total_flex > 0.0f ? total_flex : vlk::f32_epsilon);

    auto &area = children_area[i];

    area.x = IndependentParameters{previous_percentages};
    area.y = IndependentParameters{0.0f};

    area.width = IndependentParameters{percentage};

    // take full height
    area.height = IndependentParameters{1.0f};

    previous_percentages += percentage;
  }
}

// TODO(lamarrr): Parmeters x, y, width, height?
// we need to
inline void build_row_children_layout(
    stx::Span<ChildLayout> const &children_area,
    stx::Span<Widget *const> const &children) {
  VLK_DEBUG_ENSURE(children_area.size() == children.size());

  size_t children_count = children.size();

  Parameters previous_height = IndependentParameters{0.0};

  for (size_t i = 0; i < children_count; i++) {
    auto const child_layout = children[i]->get_self_layout();

    auto &area = children_area[i];

    area.x = IndependentParameters{0.0};
    area.y = IndependentParameters{

    };

    // take full width
    area.width = IndependentParameters{1.0f};

    area.height = IndependentParameters{1.0f};
  }
}

// if this widget is moved an invalidation will occur?

struct MockWidget : public Widget {
  MockWidget() : Widget{} {}
  ~MockWidget() override {}
};

struct MockSized : public Widget {
  MockSized(int32_t width, int32_t height) : Widget{Type::Render} {
    Widget::update_self_layout(SelfLayout{IndependentParameters{0.0, width},
                                          IndependentParameters{0.0, height}});
  }
  ~MockSized() override {}
};

// TODO(lamarrr): shouldn't bias be int64_t ?

struct MockView : public Widget {
  MockView(uint32_t width, uint32_t height, uint32_t view_height,
           stx::Span<Widget *const> const &children, uint32_t padding_left = 0)
      : Widget{Type::View} {
    Widget::update_self_layout(SelfLayout{
        // independent extent
        IndependentParameters{0.0, width}, IndependentParameters{0.0, height}});

    Widget::update_view_extent(ViewExtent{
        // view width depends on child's width
        DependentParameters{},
        // view height is fixed
        IndependentParameters{0.0, view_height},
    });

    // start at 0,0
    Widget::update_view_offset(
        ViewOffset{IndependentParameters{0.0}, IndependentParameters{0.0}});

    build_children(children_, children);
    Widget::update_children(children_);

    std::vector<float> flex;
    flex.resize(children.size(), 1.0f);

    children_layout_.resize(children_.size());

    build_column_children_layout(children_layout_, flex);

    Widget::update_children_layout(children_layout_);
  }
  ~MockView() override {}

  std::vector<Widget *> children_;
  std::vector<ChildLayout> children_layout_;
};

struct MockColumn : public Widget {
  explicit MockColumn(stx::Span<Widget *const> children) : Widget{} {
    children_.resize(children.size());
    std::copy(children.begin(), children.end(), children_.begin());
    Widget::update_children(children_);

    Widget::update_self_layout(make_column_layout());

    std::vector<float> flex;
    flex.resize(children.size(), 1.0f);

    children_layout_.resize(children.size());

    build_column_children_layout(children_layout_, flex);
    Widget::update_children_layout(children_layout_);
  }

  ~MockColumn() override {}

  std::vector<Widget *> children_;
  std::vector<ChildLayout> children_layout_;
};

TEST(LayoutTest, ChildrenMaxHeight) {
  using Node = WidgetLayoutTree::Node;

  MockSized a(200, 400);
  MockSized b(250, 600);
  MockSized c(100, 200);
  MockSized d(600, 60);

  Widget *const children[] = {&a, &b, &c, &d};
  MockColumn column(children);

  WidgetLayoutTree tree;
  build_widget_layout_tree(tree, column);
  clean_layout_tree(tree, Extent{1920, 1080});

  EXPECT_EQ(tree.root_node.parent_view_area.extent.width, 1920);
  EXPECT_EQ(tree.root_node.parent_view_area.extent.height, 600);

  auto &chlrn = tree.root_node.children;

  Node &na = chlrn[0];
  Node &nb = chlrn[1];
  Node &nc = chlrn[2];
  Node &nd = chlrn[3];

  // parent offset and parent_view_area.offset should be same here

  EXPECT_EQ(na.parent_view_area.extent.width, 200);
  EXPECT_EQ(na.parent_view_area.extent.height, 400);
  EXPECT_EQ(na.parent_offset.x, 0);
  EXPECT_EQ(na.parent_offset.y, 0);
  EXPECT_EQ(na.parent_view_area.offset.x, 0);
  EXPECT_EQ(na.parent_view_area.offset.y, 0);

  EXPECT_EQ(nb.parent_view_area.extent.width, 250);
  EXPECT_EQ(nb.parent_view_area.extent.height, 600);
  EXPECT_EQ(nb.parent_offset.x, 1920 / 4);
  EXPECT_EQ(nb.parent_offset.y, 0);
  EXPECT_EQ(nb.parent_view_area.offset.x, 1920 / 4);
  EXPECT_EQ(nb.parent_view_area.offset.y, 0);

  EXPECT_EQ(nc.parent_view_area.extent.width, 100);
  EXPECT_EQ(nc.parent_view_area.extent.height, 200);
  EXPECT_EQ(nc.parent_offset.x, (1920 / 4) * 2);
  EXPECT_EQ(nc.parent_offset.y, 0);
  EXPECT_EQ(nc.parent_view_area.offset.x, (1920 / 4) * 2);
  EXPECT_EQ(nc.parent_view_area.offset.y, 0);

  EXPECT_EQ(nd.parent_view_area.extent.width, 1920 / 4);
  EXPECT_EQ(nd.parent_view_area.extent.height, 60);
  EXPECT_EQ(nd.parent_offset.x, (1920 / 4) * 3);
  EXPECT_EQ(nd.parent_offset.y, 0);
  EXPECT_EQ(nd.parent_view_area.offset.x, (1920 / 4) * 3);
  EXPECT_EQ(nd.parent_view_area.offset.y, 0);
}

// test view offset with padding?

TEST(LayoutTest, View) {
  using Node = WidgetLayoutTree::Node;

  MockSized a(200, 400);
  MockSized b(250, 600);
  MockSized c(100, 200);
  MockSized d(600, 60);

  MockSized side(200, 500);

  Widget *const subview_children[] = {&a, &b, &c, &d};
  MockView subview(200, 200, vlk::u32_max, subview_children);
  Widget *const root_children[] = {&subview, &side};

  MockColumn column(root_children);

  WidgetLayoutTree tree;
  build_widget_layout_tree(tree, column);
  clean_layout_tree(tree, Extent{1920, 1080});

  EXPECT_EQ(tree.root_node.parent_view_area.extent.width, 1920);
  EXPECT_EQ(tree.root_node.parent_view_area.extent.height, 500);

  auto &chlrn = tree.root_node.children;

  // parent offset and parent_view_area.offset should be same here
  {
    Node &subview = chlrn[0];
    Node &side = chlrn[1];

    EXPECT_EQ(subview.parent_view_area.extent.width, 200);
    EXPECT_EQ(subview.parent_view_area.extent.height, 200);
    EXPECT_EQ(subview.parent_offset.x, 0);
    EXPECT_EQ(subview.parent_offset.y, 0);
    EXPECT_EQ(subview.parent_view_area.offset.x, 0);
    EXPECT_EQ(subview.parent_view_area.offset.y, 0);

    EXPECT_EQ(side.parent_view_area.extent.width, 200);
    EXPECT_EQ(side.parent_view_area.extent.height, 500);
    EXPECT_EQ(side.parent_offset.x, 1920 / 2);
    EXPECT_EQ(side.parent_offset.y, 0);
    EXPECT_EQ(side.parent_view_area.offset.x, 1920 / 2);
    EXPECT_EQ(side.parent_view_area.offset.y, 0);
  }

  {
    auto &schlrn = chlrn[0].children;
    Node &a = schlrn[0];
    Node &b = schlrn[1];
    Node &c = schlrn[2];
    Node &d = schlrn[3];

    EXPECT_EQ(a.parent_view_area.extent.width, 200);
    EXPECT_EQ(a.parent_view_area.extent.height, 400);
    EXPECT_EQ(a.parent_offset.x, 0);
    EXPECT_EQ(a.parent_offset.y, 0);
    EXPECT_EQ(a.parent_view_area.offset.x, 0);
    EXPECT_EQ(a.parent_view_area.offset.y, 0);
  }
}

TEST(ColumnLayout, LayoutTest) {
  MockWidget a{}, b{}, c{};
  Widget *children[3] = {&a, &b, &c};

  MockColumn container{children};

  WidgetLayoutTree layout_tree;
  build_widget_layout_tree(layout_tree, container);
  clean_layout_tree(layout_tree, Extent{1920, 1080});

  EXPECT_EQ(layout_tree.root_node.children.size(), 3);
  EXPECT_EQ(layout_tree.root_node.parent_offset.x, 0);
  EXPECT_EQ(layout_tree.root_node.parent_offset.y, 0);
  EXPECT_EQ(layout_tree.root_node.parent_view_area.extent.width, 1920);
  EXPECT_EQ(layout_tree.root_node.parent_view_area.extent.height, 1080);
  /* for (auto &child : layout_tree.root_node.children) {
     std::cout << child.parent_offset.x << ", " << child.parent_offset.y
               << std::endl;
     std::cout << child.parent_view_area.extent.width << ", "
               << child.parent_view_area.extent.height << std::endl
               << std::endl;
   }
 */
  RenderTree render_tree;
  build_render_tree(render_tree, layout_tree.root_node);
}
