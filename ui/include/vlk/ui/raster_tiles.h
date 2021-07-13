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
struct RasterCacheTiles {
  using Tile = RasterCache;

  explicit RasterCacheTiles(Extent tile_physical_extent)
      : tile_physical_extent_{tile_physical_extent} {
    VLK_ENSURE(tile_physical_extent_.visible());
  }

  uint32_t rows() const {
    return ((physical_extent_.width + tile_physical_extent_.width) /
            tile_physical_extent_.width);
  }

  uint32_t columns() const {
    return ((physical_extent_.height + tile_physical_extent_.height) /
            tile_physical_extent_.height);
  }

  // checked if debug checks are enabled
  Tile &tile_at_index(uint32_t row, uint32_t column) {
    VLK_ENSURE(row < rows());
    VLK_ENSURE(column < columns());

    return tiles_[column * rows() + row];
  }

  Extent physical_extent() const { return physical_extent_; }

  Extent tile_physical_extent() const { return tile_physical_extent_; }

  stx::Span<Tile> get_tiles() { return tiles_; }

  stx::Span<Tile const> get_tiles() const { return tiles_; }

  size_t storage_size_estimate() const {
    return std::accumulate(tiles_.begin(), tiles_.end(), size_t{0},
                           [](size_t size, Tile const &tile) {
                             return size + tile.surface_size();
                           });
  }

  void resize(Extent const &new_physical_extent) {
    physical_extent_ = new_physical_extent;
    auto const num_required_tiles = static_cast<size_t>(rows()) * columns();

    tiles_.resize(num_required_tiles);
  }

 private:
  Extent physical_extent_;

  Extent tile_physical_extent_;

  // a grid of tiles sorted in row-major order.
  // each tile is of extent `tile_physical_extent_`
  std::vector<Tile> tiles_;
};

struct RasterRecordTiles {
  using Tile = RasterRecord;

  RasterRecordTiles(uint32_t nrows, uint32_t ncolumns)
      : rows_{nrows}, columns_{ncolumns} {
    resize(rows_, columns_);
  }

  RasterRecordTiles() = default;

  uint32_t rows() const { return rows_; }

  uint32_t columns() const { return columns_; }

  stx::Span<Tile> get_tiles() { return tiles_; }

  stx::Span<Tile const> get_tiles() const { return tiles_; }

  void resize(uint32_t nrows, uint32_t ncols) {
    size_t num_required_tiles = nrows * static_cast<size_t>(ncols);

    tiles_.resize(num_required_tiles);

    rows_ = nrows;
    columns_ = ncols;
  }

 private:
  // a grid of tiles sorted in row-major order.
  uint32_t rows_ = 0, columns_ = 0;
  std::vector<Tile> tiles_;
};

}  // namespace ui
}  // namespace vlk
