

#include "vlk/ui/widgets/row.h"

#include "gtest/gtest.h"
#include "mock_widgets.h"
#include "vlk/ui/palettes/material.h"
#include "vlk/ui/pipeline.h"
#include "vlk/ui/raster_context.h"
#include "vlk/ui/tile_cache.h"
#include "vlk/ui/widgets/box.h"
#include "vlk/ui/widgets/text.h"

using namespace vlk::ui;
using namespace vlk;

// RowProps constrain

// what to work on next?
// on image loading the user needs to use a default fallback image or a provided
// one. transition?
//
//
// Add imgui and glfw for testing
//

TEST(RowTest, BasicTest) {
  RasterContext context;

  constexpr Color color_list[] = {material::RedA400,    material::OrangeA400,
                                  material::YellowA400, material::BlueGrey800,
                                  material::PurpleA400, material::GreenA400};

  MockView view{{new Row{[&](size_t i) -> Widget* {
    if (i >= 60) return nullptr;
    return new Box(new Box(new Text("Number " + std::to_string(i),
                                    TextProps()
                                        .font_size(15.0f)
                                        .font_weight(FontWeight::Normal)
                                        .color(colors::Black)
                                        .font_family("Roboto")),
                           BoxProps()
                               .padding(Padding::all(15))
                               .border_radius(BorderRadius::all(20))
                               .blur(Blur{2.5f, 2.5f})
                               .color(colors::White.with_alpha(0x7F))),
                   BoxProps()
                       .padding(Padding::all(35))
                       .border(Border::all(material::AmberA100, 20))
                       .border_radius(BorderRadius::all(20))
                       .color(color_list[i % std::size(color_list)]));
  }}}};

  Extent screen_extent{1600, 600};

  LayoutTree layout_tree;
  layout_tree.allot_extent(screen_extent);
  layout_tree.build(view);

  ViewTree view_tree;
  view_tree.build(layout_tree);

  // TODO(lamarrr): we don't need tick after build

  TileCache cache;

  AssetManager asset_manager{context};

  cache.build(view_tree, context, asset_manager);

  cache.resize_viewport(screen_extent);

  for (size_t i = 0; i < 5; i++) {
    constexpr float mul = 1 / 5.0f;
    cache.scroll_viewport(ViewOffset{{mul * i}, {mul * i}});
    cache.tick(std::chrono::nanoseconds(0));
    cache.backing_store.save_pixels_to_file("./ui_output_row_" +
                                            std::to_string(i));
  }
}