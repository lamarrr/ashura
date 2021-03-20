#pragma once

#include <vector>

#include "include/core/SkSurface.h"
#include "stx/span.h"

#include "vlk/ui/primitives.h"
#include "vlk/ui/raster_cache.h"

namespace vlk {
namespace ui {
//
// tile resizing when screen changes
//
// enum class RasterCacheState
//  {
//   Clean,
//   Dirty,
//   Rasterizing
//  };

// problem: recording and updating pictures multiple times can be costly, we
// need effects

// how do we re-record the raster tile content? without allocating extra memory
// too frequently
// what if we store chunks in each tile?

// raster cache, even view widgets are added here, all widgets are layout
// widgets, we thus don't reed a separate view on the render tree

// how do we want to model raster task scheduling?

struct RasterTiles {
  using Tile = RasterCache;

  RasterTiles(Extent const &extent, Extent const &tile_size)
      : extent_{extent}, tile_size_{tile_size}, tiles_{} {
    for (size_t i = 0; i < rows() * columns(); i++)
      tiles_.emplace_back(Tile{IRect{IOffset{}, tile_size_}});
  }

  ~RasterTiles() = default;

  uint32_t rows() const {
    return ((extent_.width + tile_size_.width) / tile_size_.width);
  }

  uint32_t columns() const {
    return ((extent_.height + tile_size_.height) / tile_size_.height);
  }

  // checked if debug checks are enabled
  Tile &tile_at_index(uint32_t row, uint32_t column) {
    VLK_DEBUG_ENSURE(row < rows());
    VLK_DEBUG_ENSURE(column < columns());

    return tiles_[column * rows() + row];
  }

  // checked if debug checks are enabled
  Tile &tile_at_point(Offset const &offset) {
    VLK_DEBUG_ENSURE(offset.x < extent().width);
    VLK_DEBUG_ENSURE(offset.y < extent().height);

    uint32_t i = offset.x / tile_size().width;
    uint32_t j = offset.y / tile_size().height;

    return tile_at_index(i, j);
  }

  Extent extent() const { return extent_; }

  Extent tile_size() const { return tile_size_; }

  stx::Span<Tile> get_tiles() { return tiles_; }

  stx::Span<Tile const> get_tiles() const { return tiles_; }

 private:
  // future-TODO(lamarrr): implement pixel ratio usage, how to. should this stay
  // here?
  // float pixel_ratio_ = 1.0f;

  Extent extent_ = {1920, 1080};

  Extent tile_size_ = {64, 64};

  // a grid of tiles
  // each tile is of extent `tile_size_` * pixel_ratio_
  // sorted in row-major order.
  std::vector<Tile> tiles_;
};

// after the first render, we don't need to update if none of the inview ones
// is dirty accumulating cache
//  sk_sp<SkSurface> accumulation_ = nullptr;

}  // namespace ui
}  // namespace vlk
