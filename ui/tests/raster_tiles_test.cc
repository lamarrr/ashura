
#include "gtest/gtest.h"

#include "vlk/ui/tile_cache.h"

TEST(RasterTilesTest, BasicTest) {
  using namespace vlk::ui;

  RasterTiles tiles{Extent{1920, 1080}, Extent{256, 256}};

  EXPECT_EQ(tiles.rows(), (1920 + 256) / 256);
  EXPECT_EQ(tiles.columns(), (1080 + 256) / 256);

EXPECT_EQ(tiles.rows() * tiles.columns(), tiles.get_tiles().size());

  for (RasterTiles::Tile& tile : tiles.get_tiles()) {
    EXPECT_FALSE(tile.is_recording());
    EXPECT_FALSE(tile.is_surface_init());
    tile.begin_recording();
    EXPECT_TRUE(tile.is_recording());
    tile.finish_recording();
    EXPECT_FALSE(tile.is_recording());
  }
}
