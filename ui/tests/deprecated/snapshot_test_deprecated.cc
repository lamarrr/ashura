#include "vlk/ui/compositor.h"
#include "vlk/ui/surface_provider.h"
#include "vlk/ui/widget.h"
#include "vlk/ui/widgets/row.h"

#include <iostream>
#include "gtest/gtest.h"

using namespace vlk::ui;

TEST(SnapshotTest, Snapshot) {
  using namespace vlk::ui::impl;

  auto row = Row({});

  auto snapshot =
      Snapshot::CreateRecorded(row, Rect{Offset{0, 0}, Extent{64, 64}});

  EXPECT_TRUE(snapshot.is_draw_commands_recorded());

  CpuSurfaceProvider provider;
  snapshot.rasterize(provider);

  snapshot.discard_draw_commands();
  snapshot.discard_image();

  sk_sp surface = provider.make_surface(snapshot.area().extent);

  auto* canvas = surface->getCanvas();

  SkPaint paint;
  paint.setStroke(false);
  paint.setColor(colors::Red.argb());

  canvas->drawRect(SkRect::MakeXYWH(10, 10, 50, 50), paint);

  sk_sp isnapshot = surface->makeImageSnapshot();
  auto iinfo = isnapshot->imageInfo();

  std::vector<uint8_t> buff;
  buff.resize(iinfo.width() * iinfo.height() * iinfo.bytesPerPixel());

  isnapshot->readPixels(
      SkImageInfo::MakeN32Premul(iinfo.width(), iinfo.height()), buff.data(),
      iinfo.width() * iinfo.bytesPerPixel(), 0, 0);
}

TEST(CompositorTest, IsOverlapping) {
  using namespace vlk::ui::impl;

  {
    EXPECT_FALSE(is_overlapping(Rect{Offset{0, 0}, Extent{10, 10}},
                                Rect{Offset{10, 10}, Extent{10, 10}}));

    EXPECT_FALSE(is_overlapping(Rect{Offset{0, 0}, Extent{10, 10}},
                                Rect{Offset{5, 10}, Extent{20, 20}}));

    EXPECT_TRUE(is_overlapping(Rect{Offset{0, 0}, Extent{10, 10}},
                               Rect{Offset{5, 9}, Extent{30, 30}}));
  }
  {
    EXPECT_FALSE(is_overlapping(Rect{Offset{10, 10}, Extent{10, 10}},
                                Rect{Offset{0, 0}, Extent{10, 10}}));

    EXPECT_FALSE(is_overlapping(Rect{Offset{5, 10}, Extent{20, 20}},
                                Rect{Offset{0, 0}, Extent{10, 10}}));

    EXPECT_TRUE(is_overlapping(Rect{Offset{5, 9}, Extent{30, 30}},
                               Rect{Offset{0, 0}, Extent{10, 10}}));
  }
}

TEST(CompositorTest, Residuals) {
  using namespace vlk::ui::impl;

  auto row0 = Row({});
  auto row1 = Row({});

  Residuals<> residuals;
  residuals.emplace_back(CacheEntry(
      Snapshot::CreateRecorded(row0, Rect{Offset{0, 0}, Extent{64, 64}}), 0,
      0));
  residuals.emplace_back(CacheEntry(
      Snapshot::CreateRecorded(row1, Rect{Offset{0, 0}, Extent{32, 32}}), 0,
      1));

  EXPECT_EQ(residuals.size(), 2);
  EXPECT_EQ(stx::Span(residuals)[0].out_of_view_ticks, 0);
  EXPECT_EQ(stx::Span(residuals)[0].snapshot.widget(), &row0);
  EXPECT_EQ(stx::Span(residuals)[1].out_of_view_ticks, 1);
  EXPECT_EQ(stx::Span(residuals)[1].snapshot.widget(), &row1);
}

TEST(CompositorTest, ViewTicksUpdate) {
  using namespace vlk::ui::impl;

  auto row0 = Row({});
  auto row1 = Row({});
  auto row2 = Row({});

  std::vector<CacheEntry> entries;
  entries.emplace_back(CacheEntry(
      Snapshot::CreateRecorded(row0, Rect{Offset{0, 0}, Extent{64, 64}}), 0,
      0));
  entries.emplace_back(CacheEntry(
      Snapshot::CreateRecorded(row1, Rect{Offset{0, 0}, Extent{32, 32}}), 0,
      1));
  entries.emplace_back(CacheEntry(
      Snapshot::CreateRecorded(row2, Rect{Offset{0, 0}, Extent{128, 128}}), 0,
      2));

  auto view_area = Rect{Offset{64, 64}, Extent{32, 32}};
  update_out_of_view_ticks(entries[0], view_area);
  update_out_of_view_ticks(entries[1], view_area);
  update_out_of_view_ticks(entries[2], view_area);

  auto s_entries = stx::Span(entries);

  EXPECT_EQ(s_entries[0].out_of_view_ticks, 1);
  EXPECT_EQ(s_entries[1].out_of_view_ticks, 2);
  EXPECT_EQ(s_entries[2].out_of_view_ticks, 0);

  discard_all_snapshot_draw_commands(entries);
}

TEST(CompositorTest, LRUResolve) {
  using namespace vlk::ui::impl;

  auto row0 = Row({});
  auto row1 = Row({});
  auto row2 = Row({});
  auto row3 = Row({});
  auto row4 = Row({});

  Residuals residuals;

  auto view_area = Rect{Offset{64, 64}, Extent{32, 32}};

  // out of view
  residuals.emplace_back(CacheEntry(
      Snapshot::CreateRecorded(row0, Rect{Offset{0, 0}, Extent{64, 64}}), 4,
      1));

  // out of view
  residuals.emplace_back(CacheEntry(
      Snapshot::CreateRecorded(row1, Rect{Offset{0, 0}, Extent{64, 64}}), 1,
      1));

  // out of view
  residuals.emplace_back(CacheEntry(
      Snapshot::CreateRecorded(row2, Rect{Offset{0, 0}, Extent{32, 32}}), 2,
      1));

  // in view
  residuals.emplace_back(CacheEntry(
      Snapshot::CreateRecorded(row3, Rect{Offset{0, 0}, Extent{128, 128}}), 0,
      0));

  // in view
  residuals.emplace_back(CacheEntry(
      Snapshot::CreateRecorded(row4, Rect{Offset{0, 0}, Extent{128, 128}}), 3,
      2));

  Cache cache{};
  CpuSurfaceProvider provider;

  LRU_resolve<false>(residuals, cache, provider, view_area, 1);

  EXPECT_EQ(residuals.size(), 3);
  EXPECT_EQ(cache.size(), 2);
  EXPECT_NE(std::find_if(residuals.begin(), residuals.end(),
                         [&row0](auto& entry) {
                           return entry.snapshot.widget() == &row0;
                         }),
            residuals.end());
  EXPECT_NE(std::find_if(residuals.begin(), residuals.end(),
                         [&row1](auto& entry) {
                           return entry.snapshot.widget() == &row1;
                         }),
            residuals.end());
  EXPECT_NE(std::find_if(residuals.begin(), residuals.end(),
                         [&row2](auto& entry) {
                           return entry.snapshot.widget() == &row2;
                         }),
            residuals.end());

  EXPECT_NE(std::find_if(cache.begin(), cache.end(),
                         [&row3](auto& entry) {
                           return entry.snapshot.widget() == &row3;
                         }),
            residuals.end());
  EXPECT_NE(std::find_if(cache.begin(), cache.end(),
                         [&row4](auto& entry) {
                           return entry.snapshot.widget() == &row4;
                         }),
            residuals.end());

  EXPECT_TRUE(std::is_sorted(cache.begin(), cache.end(),
                             [](CacheEntry const& a, CacheEntry const& b) {
                               return a.z_index < b.z_index;
                             }));
}
