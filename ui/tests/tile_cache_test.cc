#include "vlk/ui/tile_cache.h"
#include "gtest/gtest.h"

#include "mock_widgets.h"

TEST(TileCacheTest, Basic) {
  RasterContext context;

  auto w1 = MockSized{Extent{20, 20}, stx::Some<ZIndex>(2)};
  auto w2 = MockSized{Extent{30, 50}};
  auto f1 = MockFlex{{&w1, &w2}};
  auto v1 = MockView{&f1};

  auto w3 = MockSized{Extent{30, 50}};
  auto v2 = MockView{&w3};

  auto froot = MockFlex{{&v1, &v2}, stx::Some<ZIndex>(5)};
  auto vroot = MockView{&froot};

  LayoutTree layout_tree;
  layout_tree.allot_extent(Extent{2080, 1440});
  layout_tree.build(vroot);
  layout_tree.tick(std::chrono::nanoseconds(0));

  ViewTree view_tree;
  view_tree.build(layout_tree);
  vroot.update_view_offset(ViewOffset{Constrain{0.0f, 10}, Constrain{0.0f}});
  view_tree.tick(std::chrono::nanoseconds(0));

  TileCache cache;

  // TODO(lamarrr): layout tree must be ticked and view tree must be ticked
  // before ticking the tile_cache else we get invalid results
  cache.build(view_tree, context);

  EXPECT_EQ(cache.context, &context);
  EXPECT_EQ(cache.entries.size(), 5);
  EXPECT_EQ(cache.viewport_scroll_offset, (IOffset{0, 0}));
  EXPECT_FALSE(cache.viewport_scrolled);
  EXPECT_EQ(cache.viewport_extent, (Extent{1920, 1080}));
  EXPECT_FALSE(cache.viewport_resized);
  // focus_span
  // tiles_extent?
  EXPECT_FALSE(cache.any_tile_dirty);

  cache.resize_viewport(Extent{2080, 1440});
  cache.scroll_viewport(ViewOffset{Constrain{0.0f, 10}, Constrain{0.0f, 10}});

  EXPECT_EQ(cache.viewport_scroll_offset, (IOffset{10, 10}));
  EXPECT_TRUE(cache.viewport_scrolled);
  EXPECT_EQ(cache.viewport_extent, (Extent{2080, 1440}));
  EXPECT_TRUE(cache.viewport_resized);

  Extent const self_extent = view_tree.root_view.layout_node->self_extent;
  Extent const total_tile_extent = cache.tiles.extent();

  EXPECT_LE(self_extent.width, total_tile_extent.width);
  EXPECT_LE(self_extent.height, total_tile_extent.height);

  cache.tick(std::chrono::nanoseconds(0));

  EXPECT_FALSE(cache.viewport_scrolled);
  EXPECT_FALSE(cache.viewport_resized);
  EXPECT_FALSE(cache.any_tile_dirty);

  std::cout << "\nbytes estimate: " << cache.tiles.storage_size_estimate()
            << " bytes\n";
}
