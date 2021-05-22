
#include "vlk/ui/widgets/image.h"

#include "gtest/gtest.h"

using namespace vlk;
using namespace vlk::ui;

TEST(ImageTest, BasicTest) {
  RasterContext context;

  Image(FileImageSource{"/home/lamar/Desktop/2711345.jpg", ImageFormat::RGB},
        ImageProps{});
}