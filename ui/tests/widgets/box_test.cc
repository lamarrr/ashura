
#include "vlk/ui/widgets/box.h"

#include <filesystem>
#include <fstream>
#include <vector>

#include "gtest/gtest.h"
#include "mock_widgets.h"
#include "stx/panic.h"
#include "vlk/ui/pipeline.h"
#include "vlk/ui/tile_cache.h"
#include "vlk/ui/widgets/text.h"

using namespace vlk::ui;
using namespace vlk;

TEST(BoxTest, BasicTest) {
  RenderContext context;

  // TODO(lamarrr):
  // we need text layout in the system

  MockView v1{{new Box{
      new Text{"gfx::RenderText is a stateful API - an instance of a "
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
                   .color(colors::Red)
                   .font_size(16.0f)
                   .font(SystemFont{"Arial"})},
      BoxProps()
          .image(FileImageSource{"/home/lamar/Desktop/wraith.jpg"})
          .padding(Padding::all(200))
          .border_radius(BorderRadius::all(0))}}};

  Extent screen_extent{2000, 1000};

  /*
    Pipeline pipeline{v1};

    pipeline.viewport.resize(screen_extent);

    constexpr auto n = 20;
    for (size_t i = 0; i < n; i++) {
      constexpr float mul = 1 / (n * 1.f);
      // cache.scroll_viewport_to(ViewOffset{{mul * i}, {mul * i}});
      pipeline.tick(std::chrono::nanoseconds(0));
      pipeline.tile_cache.backing_store.save_pixels_to_file("./ui_output_" +
                                                            std::to_string(i));
      VLK_LOG("written tick {}", i);
    }
    */
}