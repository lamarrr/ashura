
#include "vlk/ui/widgets/box.h"
#include <filesystem>
#include <fstream>
#include <vector>
#include "gtest/gtest.h"
#include "vlk/ui/pipeline.h"
#include "vlk/ui/tile_cache.h"
#include "vlk/ui/widgets/text.h"

#include "stx/panic.h"

#include "mock_widgets.h"

using namespace vlk::ui;
using namespace vlk;

// TODO(lamarrr): if image is not opaque and its size is preserved we can
// perform a direct write to the surface.
//
// writing rgb images will be faster as it doesn't need blending
//
struct ImageSource {
  static std::unique_ptr<ImageSource> file(std::filesystem::path const& path);
  static std::unique_ptr<ImageSource> url();

  virtual ~ImageSource() = 0;
};

// resource ticker?
// renderer resource ticker or registry
// caching
struct FileImageSource : public ImageSource {
  // load
  // discard
  // tick()?
  virtual ~FileImageSource() override {}
};

TEST(BoxTest, BasicTest) {
  RasterContext context;

  // TODO(lamarrr): we need to provide a children updating abstraction that
  // would cleanup children as necessary
  //
  // TODO(lamarrr): image source instead
  // we need text layout in the system

  MockView v1{{new Box(
      new Text("gfx::RenderText is a stateful API - an instance of a "
               "gfx::RenderText subclass will cache its layout information "
               "between "
               "draw calls. Because of this, it is often more efficient to "
               "use the "
               "gfx::RenderText API directly instead of using a state-less "
               "abstraction such as the gfx::Canvas drawing calls. In "
               "particular, "
               "for text that changes rarely but that may be drawn "
               "multiple times, "
               "it is more efficient to keep an instance of "
               "gfx::RenderText around, "
               "so that the text layout would be performed only when the "
               "text is "
               "updated and not on every draw operation. Prior to the "
               "introduction "
               "of gfx::RenderText, this pattern was not possible, so you "
               "may see "
               "existing code still doing its text drawing through "
               "gfx::Canvas text "
               "drawing APIs (which in the past were not based on "
               "gfx::RenderText).",
               TextProps()
                   // .underlined()
                   // .italic()
                   .align(TextAlign::Center)
                   .color(colors::White)
                   .font_size(16.0f)
                   .font_family("Arial")),
      BoxProps().padding(Padding::all(10)).border_radius(BorderRadius::all(0)),
      BoxDecoration().color(colors::Black))}};

  Extent screen_extent{3840, 2160};

  LayoutTree layout_tree;
  layout_tree.allot_extent(screen_extent);
  layout_tree.build(v1);
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
  //  cache.backing_store.save_pixels_to_file("./ui_output_" + std::to_string(i));
  }
}