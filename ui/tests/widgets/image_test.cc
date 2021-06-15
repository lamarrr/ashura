
#include "vlk/ui/widgets/image.h"

#include "gtest/gtest.h"

using namespace vlk;
using namespace vlk::ui;

TEST(ImageTest, BasicTest) {
  RenderContext context;

  Image(ImageProps{FileImageSource{"/home/lamar/Desktop/2711345.jpg",
                                   stx::Some(ImageFormat::RGB)}});
}
