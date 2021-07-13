
#include "gtest/gtest.h"
#include "vlk/ui/tile_cache.h"

TEST(RasterTilesTest, BasicTest) {
  using namespace vlk::ui;
  using namespace vlk;

  RasterCacheTiles caches{Extent{256, 256}};
  RasterRecordTiles records;

  caches.resize(Extent{1920, 1080});
  records.resize(caches.rows(), caches.columns());

  EXPECT_EQ(caches.rows(), (1920 + 256) / 256);
  EXPECT_EQ(caches.columns(), (1080 + 256) / 256);
  EXPECT_EQ(caches.rows(), records.rows());
  EXPECT_EQ(caches.columns(), records.columns());

  EXPECT_EQ(caches.rows() * caches.columns(), caches.get_tiles().size());

  for (auto& cache : caches.get_tiles()) {
    EXPECT_FALSE(cache.is_surface_init());
  }

  for (auto& record : records.get_tiles()) {
    EXPECT_FALSE(record.is_recording());
    record.begin_recording(VRect{});
    EXPECT_TRUE(record.is_recording());
    record.finish_recording();
    EXPECT_FALSE(record.is_recording());
  }

  RenderContext context;

  for (auto& cache : caches.get_tiles()) {
    cache.init_surface(context, caches.tile_physical_extent());
    EXPECT_TRUE(cache.is_surface_init());
  }
}
