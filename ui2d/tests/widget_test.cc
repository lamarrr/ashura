

#include "vlk/ui2d/widget.h"
#include "vlk/ui2d/compositor.h"
#include "vlk/utils/utils.h"

#include "gtest/gtest.h"

using namespace vlk::ui2d;
using namespace vlk::ui2d::impl;

struct TraceWidget : public Column {
  using Column::Column;
};

struct Box : public Widget {
  Box() : width_{0}, height_{0} {}
  Box(uint32_t width, uint32_t height) : width_{width}, height_{height} {}

  virtual bool is_layout_type() const noexcept override { return false; }

  virtual bool is_stateful() const noexcept override { return false; }

  virtual bool is_dirty() const noexcept override { return false; }
  virtual void mark_clean() noexcept override {}

  virtual stx::Span<Widget *const> get_children() const noexcept override {
    return {};
  }

  virtual Rect compute_area(
      [[maybe_unused]] Extent const &allotted_extent,
      [[maybe_unused]] stx::Span<Rect> const &children_area) const
      noexcept override {
    return Rect{Offset{0, 0}, Extent{width_, height_}};
  }

  virtual void draw(
      [[maybe_unused]] Canvas &canvas,
      [[maybe_unused]] Extent const &requested_extent) const override {
    // no-op
  }

 private:
  uint32_t width_;
  uint32_t height_;
};

TEST(WidgetConstructionTest, Widget) {
  auto col = Column({new TraceWidget(std::initializer_list<Widget *>{}),
                     new TraceWidget(std::initializer_list<Widget *>{})});

  auto children = col.get_children();

  VLK_LOG("Widget Type Name: {}", children[0]->get_type_name());
  VLK_LOG("Widget Name: {}", children[0]->get_name());

  VLK_LOG("test children{}", children.size());
  Extent surface_extent{};
  surface_extent.height = 2000;
  surface_extent.width = 2000;

  // TODO(lamarrr)

  (void)surface_extent;
}

template <typename WidgetMap>
auto get(WidgetMap &map, Widget *widget) {
  auto pos = std::find_if(map.begin(), map.end(), [widget](auto &pair) {
    return pair.first == widget;
  });

  VLK_ENSURE(pos != map.end());
  return pos->second;
}

template <typename... Args>
auto get(Residuals<Args...> &map, Widget *widget) {
  auto pos = std::find_if(map.begin(), map.end(), [widget](auto &pair) {
    return pair.snapshot.widget() == widget;
  });

  VLK_ENSURE(pos != map.end());
  return pos->snapshot.area();
}

TEST(CompositorTest, DimensionBuilding) {
  //  impl::Compositor::CacheRegistry registry;
  //  impl::LinearCache cache;
  std::vector<Widget *> w;

  for (size_t i = 0; i < 5'000; i++) {
    w.push_back(new TraceWidget({}));
  }
  auto col =
      Column({new TraceWidget({}), new TraceWidget(w), new TraceWidget({})});
  Residuals<> stateless_residuals;
  Residuals<> stateful_residuals;
  std::vector<std::pair<Widget *, Rect>> stateless_layout_widgets;
  std::vector<std::pair<Widget *, Rect>> stateful_layout_widgets;

  Offset allotted_col_offset{0, 0};
  Extent allotted_col_extent{400, 800};
  build_widget_layout(stateless_layout_widgets, stateful_layout_widgets,
                      stateless_residuals, stateful_residuals, &col,
                      allotted_col_extent, allotted_col_offset);

  EXPECT_EQ(get(stateless_layout_widgets, &col).offset.x, 0);
  EXPECT_EQ(get(stateless_layout_widgets, &col).offset.y, 0);
  EXPECT_EQ(get(stateless_layout_widgets, &col).extent.width,
            allotted_col_extent.width);
  EXPECT_EQ(get(stateless_layout_widgets, &col).extent.height, 0);

  size_t i = 0;
  for (auto const &child : col.get_children()) {
    EXPECT_EQ(get(stateless_layout_widgets, child).offset.x,
              (allotted_col_extent.width / col.get_children().size()) * i);
    EXPECT_EQ(get(stateless_layout_widgets, child).offset.y, 0);
    EXPECT_EQ(get(stateless_layout_widgets, child).extent.width,
              (allotted_col_extent.width / col.get_children().size()));
    EXPECT_EQ(get(stateless_layout_widgets, child).extent.height, 0);
    i++;
  }
}

TEST(CompositorTest, LayoutBuilding) {
  //  impl::Compositor::CacheRegistry registry;
  //  impl::LinearCache cache;
  // Overall surface width should be uint64_t Rect
  uint32_t heights[] = {200, 100, 100};
  uint32_t widths[] = {300, 300, 300};
  auto col =
      Column({new Box(widths[0], heights[0]), new Box(widths[1], heights[1]),
              new Box(widths[2], heights[2])});

  VLK_LOG("Name: {}", col.get_children()[0]->get_type_name());

  Residuals<> stateless_residuals;
  Residuals<> stateful_residuals;
  std::vector<std::pair<Widget *, Rect>> stateless_layout_widgets;
  std::vector<std::pair<Widget *, Rect>> stateful_layout_widgets;

  Offset allotted_col_offset{10, 20};
  // an overdraw across the x-axis will happen here
  Extent allotted_col_extent{400, 800};
  build_widget_layout(stateless_layout_widgets, stateful_layout_widgets,
                      stateless_residuals, stateful_residuals, &col,
                      allotted_col_extent, allotted_col_offset);

  uint32_t expected_width = allotted_col_extent.width / std::size(widths);

  auto &map = stateless_layout_widgets;

  ASSERT_EQ(map.size(), 1);
  EXPECT_EQ(get(map, &col).offset.x, 10);
  EXPECT_EQ(get(map, &col).offset.y, 20);
  EXPECT_EQ(get(map, &col).extent.width, allotted_col_extent.width);
  EXPECT_EQ(get(map, &col).extent.height, 200);

  size_t i = 0;
  for (auto const &child : col.get_children()) {
    EXPECT_EQ(get(stateless_residuals, child).offset.x,
              allotted_col_offset.x +
                  (allotted_col_extent.width / col.get_children().size()) * i);
    EXPECT_EQ(get(stateless_residuals, child).offset.y, allotted_col_offset.y);
    EXPECT_EQ(get(stateless_residuals, child).extent.width, expected_width);
    EXPECT_EQ(get(stateless_residuals, child).extent.height, heights[i]);
    i++;
  }
}

TEST(CompositorTest, NestedColumn) {
  {
    auto col =
        Column{new Column{new Column{new Box(4, 6)}, new Column{new Box(6, 8)}},
               new Column{new Box(6, 6), new Box(6, 6)}};

    Residuals<> stateless_residuals;
    Residuals<> stateful_residuals;
    std::vector<std::pair<Widget *, Rect>> stateless_layout_widgets;
    std::vector<std::pair<Widget *, Rect>> stateful_layout_widgets;

    Offset allotted_col_offset{0, 0};
    // an overdraw across the x-axis will happen here
    Extent allotted_col_extent{600, 800};
    build_widget_layout(stateless_layout_widgets, stateful_layout_widgets,
                        stateless_residuals, stateful_residuals, &col,
                        allotted_col_extent, allotted_col_offset);

    auto &map = stateless_layout_widgets;

    EXPECT_EQ(get(map, &col).offset.x, 0);
    EXPECT_EQ(get(map, &col).offset.y, 0);
    EXPECT_EQ(get(map, &col).extent.width, allotted_col_extent.width);
    EXPECT_EQ(get(map, &col).extent.height, 8);

    auto children = col.get_children();
    auto child_0 = children[0];
    auto child_1 = children[1];
    auto child_0_width = allotted_col_extent.width / 2;
    auto child_1_width = allotted_col_extent.width / 2;
    auto child_0_height = 8;
    auto child_1_height = 6;

    EXPECT_EQ(get(map, child_0).extent.width, child_0_width);
    EXPECT_EQ(get(map, child_1).extent.width, child_1_width);
    EXPECT_EQ(get(map, child_0).extent.height, child_0_height);
    EXPECT_EQ(get(map, child_1).extent.height, child_1_height);
  }
}

// nested children testing
