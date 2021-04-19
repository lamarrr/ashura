

#include "vlk/ui/widgets/row.h"

#include "vlk/ui/pipeline.h"
#include "vlk/ui/raster_context.h"
#include "vlk/ui/tile_cache.h"
#include "vlk/ui/widgets/box.h"
#include "vlk/ui/widgets/text.h"

#include "mock_widgets.h"

#include "gtest/gtest.h"

using namespace vlk::ui;
using namespace vlk;

TEST(RowTest, BasicTest) {
  RasterContext context;

  MockView view{{new Row{[](size_t i) {
    if (i >= 3) return stx::make_none<Widget*>();
    return stx::make_some<Widget*>(
        new Box(new Text(std::to_string(i), TextProps()
                                                .font_size(30.0f)
                                                .font_weight(FontWeight::Bold)
                                                .color(colors::Red)),
                BoxProps().padding(Padding::all(20)),
                BoxDecoration().color(colors::Black)));
  }}}};

  Extent screen_extent{1600, 600};

  LayoutTree layout_tree;
  layout_tree.allot_extent(screen_extent);
  layout_tree.build(view);
  layout_tree.tick(std::chrono::nanoseconds(0));

  ViewTree view_tree;
  view_tree.build(layout_tree);
  view_tree.tick(std::chrono::nanoseconds(0));

  TileCache cache;

  cache.build(view_tree, context);

  cache.resize_viewport(screen_extent);

  for (size_t i = 0; i < 5; i++) {
    constexpr float mul = 1 / 5.0f;
    cache.scroll_viewport(ViewOffset{{mul * i}, {mul * i}});
    cache.tick(std::chrono::nanoseconds(0));
    cache.backing_store.save_pixels_to_file("./ui_output_row_" +
                                            std::to_string(i));
  }
}