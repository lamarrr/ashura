#pragma once

#include <numeric>
#include <vector>

#include "include/core/SkSurface.h"
#include "stx/span.h"
#include "vlk/ui/primitives.h"
#include "vlk/ui/raster_cache.h"

namespace vlk {
namespace ui {

// this should cover the whole extent of the widgets. this should be allotted to
// the self extent of the root view widget. they are only activated when in
// focus, this optimizes for scrolling especially when the content don't really
// change and only their raster content change. if we are re-drawing for a tile
// for example, we can check if it intersects with the tile and only redraw for
// the widgets that intersect with the tile. This enable us to process
// rasetrization commands in batches rather than on a per-widget basis.
//
struct RasterTiles {
  using Tile = RasterCache;

  RasterTiles(Extent const &extent, Extent const &tile_size)
      : extent_{extent}, tile_size_{tile_size}, tiles_{} {
    VLK_ENSURE(tile_size.visible());
    for (size_t i = 0; i < rows() * columns(); i++)
      tiles_.emplace_back(Tile{IRect{IOffset{}, tile_size_}});
  }

  uint32_t rows() const {
    return ((extent_.width + tile_size_.width) / tile_size_.width);
  }

  uint32_t columns() const {
    return ((extent_.height + tile_size_.height) / tile_size_.height);
  }

  // checked if debug checks are enabled
  Tile &tile_at_index(uint32_t row, uint32_t column) {
    VLK_ENSURE(row < rows());
    VLK_ENSURE(column < columns());

    return tiles_[column * rows() + row];
  }

  // checked if debug checks are enabled
  Tile &tile_at_point(Offset const &offset) {
    VLK_ENSURE(offset.x < extent().width);
    VLK_ENSURE(offset.y < extent().height);

    uint32_t i = offset.x / tile_extent().width;
    uint32_t j = offset.y / tile_extent().height;

    return tile_at_index(i, j);
  }

  Extent extent() const { return extent_; }

  Extent tile_extent() const { return tile_size_; }

  stx::Span<Tile> get_tiles() { return tiles_; }

  stx::Span<Tile const> get_tiles() const { return tiles_; }

  size_t storage_size_estimate() const {
    return std::accumulate(tiles_.begin(), tiles_.end(), size_t{0},
                           [](size_t size, Tile const &tile) {
                             return size + tile.storage_size();
                           });
  }

  void resize(Extent const &new_extent) {
    extent_ = new_extent;

    auto const available_tiles = tiles_.size();
    auto const num_required_tiles = rows() * columns();

    if (available_tiles == num_required_tiles) {
      // do nothing
    } else if (available_tiles > num_required_tiles) {
      for (size_t i = 0; i < (available_tiles - num_required_tiles); i++)
        tiles_.pop_back();
    } else {
      // num_required_tiles > available_tiles
      for (size_t i = 0; i < (num_required_tiles - available_tiles); i++)
        tiles_.emplace_back(Tile{IRect{IOffset{}, tile_extent()}});
    }

    for (Tile &tile : tiles_) {
      tile.recycle(IOffset{});
    }
  }

 private:
  // future-TODO(lamarrr): implement pixel ratio usage and zooming, how to.
  // should this stay here?

  Extent extent_;

  Extent tile_size_;

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
