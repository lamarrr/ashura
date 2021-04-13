
#include "vlk/ui/widgets/box.h"
#include <filesystem>
#include <fstream>
#include <vector>
#include "gtest/gtest.h"
#include "vlk/ui/tile_cache.h"

#include "mock_widgets.h"

using namespace vlk::ui;
using namespace vlk;


// TODO(lamarrr): if image is not opaque and its size is preserved we can perform a direct write to the surface.
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
  //
  // TODO(lamarrr): image source instead
  RasterContext context;
  Box box{
      new MockSized{Extent{1000, 1000}},
      BoxProps()
          .padding(Padding::all(80))
          .border_radius(BorderRadius::all(100))
          .border(Border::all(colors::Black, 5)),
      BoxDecoration()
          .image(BoxDecoration::Image::load("/home/lamar/Pictures/loba.jpg",
                                            vlk::desc::Image2D::Format::RGBA)
                     .unwrap())
          .color(colors::Black.with_alpha(62))};
  MockView v1{{&box}};

  Extent screen_extent{1080, 1080};

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

  for (size_t i = 0; i < 2000; i++) {
    constexpr float mul = 1 / 50.0f;
    cache.scroll_viewport(ViewOffset{{mul * i}, {mul * i}});
    cache.tick(std::chrono::nanoseconds(0));
    //cache.backing_store.save_pixels_to_file("./ui_output_" +
    // std::to_string(i));
  }
}