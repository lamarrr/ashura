

#include "gtest/gtest.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"

//

#include "vlk/ui/asset_manager.h"
#include "vlk/ui/image_asset.h"

TEST(AssetManagerTest, Simple) {
  using namespace vlk::ui;
  using namespace vlk::ui::impl;

  RenderContext context;
  AssetManager asset_manager{context};

  // TODO(lamarrr): think about how to specify alt images

  auto image_source = FileImageSource{
      std::filesystem::path("/home/lamar/Pictures/IMG_0079.JPG"),
      stx::Some(ImageFormat::RGBA)};

  add_asset(asset_manager, image_source).unwrap();

  uint64_t i = 0;

  asset_manager.tick(std::chrono::seconds(100));
  i++;
  get_asset(asset_manager, image_source).match([](auto) {}, [](auto) {});
}
