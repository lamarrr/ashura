#pragma once

#include <algorithm>
#include <queue>
#include <vector>

#include "vlk/ui/primitives.h"
#include "vlk/ui/raster_cache.h"
#include "vlk/ui/raster_tiles.h"
#include "vlk/ui/render_context.h"
#include "vlk/ui/view_tree.h"
#include "vlk/ui/widget.h"

namespace vlk {
namespace ui {

enum class BackingStoreDiff : uint8_t { None, Some };

// cache invalidation sources:
// - view offset change
// - viewport resize
// - layout change
// - viewport scrolling
//

constexpr auto get_tiles_range(Extent const &tile_extent, int64_t nrows,
                               int64_t ncols, IRect const &region) {
  // the .screen_offset can exceed the whole screen's extent (due to
  // scrolling) so we need to clamp

  // the widget could be an outlieing widget
  //
  // screen offset can be negative
  //
  // screen offset can even change due to a resize, we don't need to
  // worry about outlieing since it is handled
  //
  // we don't want to clamp so we don't mark the tiles at the edges as
  // dirty (if any)
  // also note that the `end`s can be negative
  //
  //
  auto [x_min, x_max, y_min, y_max] = region.bounds();

  int64_t i_begin = x_min / tile_extent.width;
  int64_t i_end = ((x_max + tile_extent.width) - 1) / tile_extent.width;

  int64_t j_begin = y_min / tile_extent.height;
  int64_t j_end = ((y_max + tile_extent.height) - 1) / tile_extent.height;

  int64_t i_begin_c = std::clamp<int64_t>(i_begin, 0, nrows);
  int64_t i_end_c = std::clamp<int64_t>(i_end, 0, nrows);

  int64_t j_begin_c = std::clamp<int64_t>(j_begin, 0, ncols);
  int64_t j_end_c = std::clamp<int64_t>(j_end, 0, ncols);

  return std::make_tuple(i_begin_c, i_end_c, j_begin_c, j_end_c);
}

//
//
// For zooming support, we need to decouple tiles from records.
// zoom also needs to be a rasterization argument?
//
//
// NICE-TO-HAVE: zooming support for RTL setting
//
//
//
//
//
//
struct TileCache {
  static constexpr Extent kTilePhysicalExtent = Extent{256, 256};

  // both raster and view widgets are added here. when a view widget's offset
  // are dirty, it marks its spanning raster tiles as dirty
  struct Entry {
    ZIndex z_index = 0;

    // can we use a single view widget to represent the viewport, so all of its
    // chilren reference it?
    Widget *widget = nullptr;

    IOffset const *screen_offset = nullptr;

    // represents the extent of the widget
    Extent const *extent = nullptr;

    IRect const *clip_rect = nullptr;

    explicit Entry(ViewTree::View::Entry const &entry) {
      z_index = entry.z_index;
      widget = entry.layout_node->widget;
      screen_offset = &entry.screen_offset;
      extent = &entry.layout_node->self_extent;
      clip_rect = &entry.clip_rect;
    }

    //! NOTE: all dimensions here are in the logical coordinates
    void draw(RasterRecord &record, VRect const &tile_screen_area,
              Dpr dpr) const {
      // use tile index and size to determine tile position on the screen and
      // use that as a translation matrix relative to the objects own position
      // on the screen

      // widgets need to use a clip on the provided extent if they know they
      // can't exactly use the provided extent on the canvas. with this
      // procedure, the widget can exceed its allotted region while drawing
      // itself

      // points to the portion of the widget that would be visible
      IRect widget_clip_rect = *clip_rect;

      IRect widget_screen_area{*screen_offset, *extent};

      VLK_ENSURE(tile_screen_area.overlaps(virtualize(widget_screen_area)));

      SkCanvas &sk_canvas = record.get_recording_canvas();

      Canvas widget_canvas{sk_canvas, widget_screen_area.extent, dpr};

      // backup transform matrix and clip state
      sk_canvas.save();

      VOffset translation =
          virtualize(widget_screen_area.offset) - tile_screen_area.offset;

      sk_canvas.translate(translation.x, translation.y);

      if (widget_clip_rect.visible()) {
        if (widget_clip_rect == widget_screen_area) {
          // draw without clip
          widget->draw(widget_canvas);
        } else {
          // draw with clip

          // apply clip
          // note that this is performed relative to the widget's extent, i.e.
          // starting offset of the clip is relative to the widget's extent
          IOffset clip_start =
              widget_clip_rect.offset - widget_screen_area.offset;

          IRect translated_clip_rect{clip_start, widget_clip_rect.extent};

          sk_canvas.clipRect(to_sk_rect(translated_clip_rect));

          widget->draw(widget_canvas);
        }
      } else {
        // draw nothing
      }

      // restore matrix and clip state
      sk_canvas.restore();
    }
  };

  VLK_DEFAULT_CONSTRUCTOR(TileCache)
  VLK_MAKE_PINNED(TileCache)
  VLK_DEFAULT_DESTRUCTOR(TileCache)

  // we can have zoom value that directly affects the draw method? that would
  // mean we'd have to re-record on zoom

  RenderContext const *context = nullptr;

  Dpr device_pixel_ratio;

  // entries are sorted in ascending z-index order
  std::vector<Entry> entries;

  IOffset backing_store_physical_offset;
  bool backing_store_physical_offset_changed = true;
  IOffset backing_store_logical_offset;  // front-end for
                                         // backing_store_physical_offset

  Extent backing_store_physical_extent = Extent{50, 50};  // non-zero
  bool backing_store_physical_extent_changed = true;
  Extent backing_store_logical_extent =
      Extent{50, 50};  // front-end for backing_store_physical_extent

  // accumulates the cache result of all the tiles.
  // resized on viewport resize.
  RasterCache backing_store_cache;

  RasterCacheTiles cache_tiles{kTilePhysicalExtent};
  bool tiles_extent_dirty = true;

  RasterRecordTiles record_tiles;
  std::vector<bool> tile_record_is_dirty;

  std::vector<bool> tile_is_in_focus;

  ViewTree::View *root_view = nullptr;

  void update_dpr(Dpr new_dpr) {
    // TODO(lamarrr): implement this function to make dirty area updating work?
    // is this behaviour correct?
    // ensure it invalidates everything
    //
    //
    // we have to re-size the tiles to the new extent
    // we don't have to discard recordings, but we have to re-rasterize tiles to
    // the target DPR
    if (device_pixel_ratio != new_dpr) {
      VLK_LOG("Tile Cache DPR changed to ({}, {})", new_dpr.x, new_dpr.y);
      device_pixel_ratio = new_dpr;
      // all functions that operate in the logical space must now be notified of
      // this change in dpr
      resize_backing_store_logical(backing_store_logical_extent);
      scroll_backing_store_logical(backing_store_logical_offset);
      // recording for some widgets can be dependent on DPR. i.e. text
      // rendering, offscreen rendered widgets, etc.
      mark_all_tile_records_dirty();
      mark_tiles_extent_dirty();
    }
  }

  IRect get_backing_store_physical_rect() const {
    return IRect{backing_store_physical_offset, backing_store_physical_extent};
  }

  void scroll_backing_store_logical(IOffset new_logical_offset) {
    backing_store_logical_offset = new_logical_offset;
    VOffset new_virtual_physical_offset =
        logical_to_physical(device_pixel_ratio, new_logical_offset);
    IOffset new_physical_offset =
        devirtualize_to_ioffset(new_virtual_physical_offset);
    scroll_backing_store_physical(new_physical_offset);
  }

  void resize_backing_store_logical(Extent new_logical_extent) {
    backing_store_logical_extent = new_logical_extent;
    VExtent new_virtual_physical_extent =
        logical_to_physical(device_pixel_ratio, new_logical_extent);
    Extent new_physical_extent =
        devirtualize_to_extent(new_virtual_physical_extent);
    resize_backing_store_physical(new_physical_extent);
  }

 private:
  void scroll_backing_store_physical(IOffset new_physical_offset) {
    if (backing_store_physical_offset != new_physical_offset) {
      backing_store_physical_offset = new_physical_offset;
      backing_store_physical_offset_changed = true;
    }
  }

  void resize_backing_store_physical(Extent new_physical_extent) {
    VLK_ENSURE(new_physical_extent.visible());

    if (backing_store_physical_extent != new_physical_extent) {
      backing_store_physical_extent = new_physical_extent;
      backing_store_physical_extent_changed = true;
    }
  }

 public:
  // notifies that we now need to fetch the new tiles extent from the layout
  // tree
  // TODO(lamarrr): this is an absured method and we should probably manually
  // forward the new extents
  void mark_tiles_extent_dirty() { tiles_extent_dirty = true; }

  void mark_all_tile_records_dirty() {
    for (size_t i = 0; i < tile_record_is_dirty.size(); i++) {
      tile_record_is_dirty[i] = true;
    }
  }

  void build_entries(ViewTree::View &view) {
    // insert by z-index order
    for (ViewTree::View::Entry &view_entry : view.entries) {
      auto const insert_pos =
          std::upper_bound(entries.begin(), entries.end(), view_entry,
                           [](ViewTree::View::Entry const &a, Entry const &b) {
                             return a.z_index < b.z_index;
                           });
      entries.insert(insert_pos, Entry{view_entry});
    }

    for (ViewTree::View &subview : view.subviews) {
      build_entries(subview);
    }
  }

  void attach_state_proxies() {
    for (Entry &entry : entries) {
      // attach proxy
      //
      // proxies are called before the tile_cache tick
      //
      // pipeline event dispatch => pipeline tick
      //
      //
      WidgetSystemProxy::get_state_proxy(*entry.widget)
          .on_render_dirty = [this, &entry] {
        //! NOTE: tile binding is semi-automatic and determined by the
        //! screen offset
        //! NOTE: the tile's rows and columns are not actually updated until
        //! tick, so widgets can still mark them as dirty even if a resize
        //! is needed
        int64_t const nrows = this->record_tiles.rows();
        int64_t const ncols = this->record_tiles.columns();

        IRect logical_widget_rect{*entry.screen_offset, *entry.extent};

        VRect virtual_physical_widget_rect =
            logical_to_physical(this->device_pixel_ratio, logical_widget_rect);

        IRect physical_widget_rect =
            devirtualize_to_irect(virtual_physical_widget_rect);

        auto [i_begin, i_end, j_begin, j_end] = get_tiles_range(
            kTilePhysicalExtent, nrows, ncols, physical_widget_rect);

        VLK_LOG(
            "marking tiles i={}, j={} and i={}, j={} OF nrows: {}, ncols: {}",
            i_begin, j_begin, i_end, j_end, nrows, ncols);

        for (int64_t j = j_begin; j < j_end; j++) {
          for (int64_t i = i_begin; i < i_end; i++) {
            // this is here because it should only mark as dirty when at
            // least one of the actual intersecting tiles is dirty
            tile_record_is_dirty[j * nrows + i] = true;
          }
        }
      };
    }
  }

  void build(ViewTree::View &view_tree_root,
             RenderContext const &render_context) {
    context = &render_context;

    // dpr is maintained

    entries.clear();

    // TODO(lamarrr): just use the already existing mark_dirty functions

    // backing_store_physical_offset is maintained, unless explicitly scrolled
    // backing_store_offset_changed is maintained

    // backing_store_physical_extent maintained, unless explicitly resized
    // backing_store_resized is maintained

    // backing_store_cache maintained, will be invalidated and marked as dirty
    // in tick()
    // backing_store_record maintained, will be discarded and re-recorded in
    // tick()

    // cache and record tiles are maintained, will be resized and discarded in
    // tick as appropriate and if necessary

    // tiles_extent_dirty is maintained

    // tile_record_is_dirty is all marked as true

    // tile_is_in_focus is all marked as false and resized as necessary in
    // tick()

    root_view = &view_tree_root;

    build_entries(view_tree_root);
    attach_state_proxies();

    for (size_t i = 0; i < tile_record_is_dirty.size(); i++) {
      tile_record_is_dirty[i] = true;
      tile_is_in_focus[i] = false;
    }
  }

  BackingStoreDiff tick(std::chrono::nanoseconds) {
    // marks that the backing store needs updating
    bool backing_store_dirty = false;
    BackingStoreDiff backing_store_diff = BackingStoreDiff::None;

    if (backing_store_physical_extent_changed) {
      backing_store_cache.init_surface(*context, backing_store_physical_extent);

      backing_store_dirty = true;

      backing_store_physical_extent_changed = false;

      backing_store_diff = BackingStoreDiff::Some;
    }

    if (backing_store_physical_offset_changed) {
      backing_store_dirty = true;

      backing_store_physical_offset_changed = false;

      backing_store_diff = BackingStoreDiff::Some;
    }

    if (tiles_extent_dirty) {
      // layout changes, screen offsets are automatically updated as long as
      // view_tree is cleaned we therefore need to assume that all the tiles are
      // now dirty. and resize? them
      Extent tiles_logical_extent = root_view->layout_node->self_extent;
      VExtent tiles_virtual_physical_extent =
          logical_to_physical(device_pixel_ratio, tiles_logical_extent);
      Extent tiles_physical_extent =
          devirtualize_to_extent(tiles_virtual_physical_extent);

      cache_tiles.resize(tiles_physical_extent);
      record_tiles.resize(cache_tiles.rows(), cache_tiles.columns());

      size_t const num_tiles = record_tiles.get_tiles().size();

      tile_record_is_dirty.resize(num_tiles);
      tile_is_in_focus.resize(num_tiles);

      // TODO(lamarrr): find a way to ensure we don't discard the recordings

      for (size_t i = 0; i < num_tiles; i++) {
        tile_record_is_dirty[i] = true;
        tile_is_in_focus[i] = false;
      }

      backing_store_dirty = true;

      backing_store_diff = BackingStoreDiff::Some;

      tiles_extent_dirty = false;
    }

    IRect backing_store_physical_rect = get_backing_store_physical_rect();

    for (uint32_t j = 0; j < cache_tiles.columns(); j++) {
      for (uint32_t i = 0; i < cache_tiles.rows(); i++) {
        IOffset tile_physical_offset{i * kTilePhysicalExtent.width,
                                     j * kTilePhysicalExtent.height};
        IRect tile_physical_rect{tile_physical_offset, kTilePhysicalExtent};
        tile_is_in_focus[j * cache_tiles.rows() + i] =
            tile_physical_rect.overlaps(backing_store_physical_rect);
      }
    }

    // subtiles should be marked as dirty and as in focus or out of focus as
    // necessary before entering here
    for (size_t i = 0; i < cache_tiles.get_tiles().size(); i++) {
      RasterCache &cache = cache_tiles.get_tiles()[i];
      RasterRecord &record = record_tiles.get_tiles()[i];

      if (tile_is_in_focus[i] && !cache.is_surface_init()) {
        // add rasterization surface if not present
        // NOTE: tiles are not initialized with a surface or even recorded until
        // they are actually in view.
        cache.init_surface(*context, kTilePhysicalExtent);
      }

      if (!tile_is_in_focus[i]) {
        cache.deinit_surface();
        record.discard();
      }

      if (tile_is_in_focus[i] && tile_record_is_dirty[i]) {
        // mark the backing store as dirty if any of the in-focus tiles is dirty
        backing_store_dirty = true;
        backing_store_diff = BackingStoreDiff::Some;

        // prepare subtile for recording and rasterization
        record.discard();

        VRect tile_virtual_logical_rect = physical_to_logical(
            device_pixel_ratio, IRect{IOffset{0, 0}, kTilePhysicalExtent});

        record.begin_recording(tile_virtual_logical_rect);
      }
    }

    for (Entry &entry : entries) {
      int64_t const nrows = record_tiles.rows();
      int64_t const ncols = record_tiles.columns();

      IRect entry_logical_area{*entry.screen_offset, *entry.extent};

      VRect entry_virtual_physical_area =
          logical_to_physical(device_pixel_ratio, entry_logical_area);

      IRect entry_physical_area =
          devirtualize_to_irect(entry_virtual_physical_area);

      auto const [i_begin, i_end, j_begin, j_end] = get_tiles_range(
          kTilePhysicalExtent, nrows, ncols, entry_physical_area);

      for (int64_t j = j_begin; j < j_end; j++) {
        for (int64_t i = i_begin; i < i_end; i++) {
          int64_t const tile_index = j * nrows + i;

          RasterRecord &record = record_tiles.get_tiles()[tile_index];

          if (tile_is_in_focus[tile_index]) {
            WidgetSystemProxy::mark_non_stale(*entry.widget);
          }

          if (tile_is_in_focus[tile_index] &&
              tile_record_is_dirty[tile_index]) {
            // draw to appropriate position relative to the tile size. and
            // also respect the view clipping
            IOffset tile_physical_offset{i * kTilePhysicalExtent.width,
                                         j * kTilePhysicalExtent.height};
            IRect tile_physical_rect{tile_physical_offset, kTilePhysicalExtent};

            VRect tile_virtual_logical_rect =
                physical_to_logical(device_pixel_ratio, tile_physical_rect);

            entry.draw(record, tile_virtual_logical_rect, device_pixel_ratio);
          }
        }
      }
    }

    for (size_t i = 0; i < cache_tiles.get_tiles().size(); i++) {
      RasterCache &cache = cache_tiles.get_tiles()[i];
      RasterRecord &record = record_tiles.get_tiles()[i];

      if (tile_record_is_dirty[i] && tile_is_in_focus[i]) {
        record.finish_recording();

        // tile caches are only updated if the tile is in focus
        // we need to submit
        cache.rasterize(device_pixel_ratio, record);

        tile_record_is_dirty[i] = false;
      }
    }

    // should backing store wrap a backend texture?
    if (backing_store_dirty) {
      // accumulate raster cache into backing store

      SkCanvas *sk_canvas = backing_store_cache.get_surface_ref().getCanvas();
      VLK_ENSURE(sk_canvas != nullptr);
      sk_canvas->clear(SK_ColorTRANSPARENT);

      int64_t const ncols = cache_tiles.columns();
      int64_t const nrows = cache_tiles.rows();

      for (int64_t j = 0; j < ncols; j++) {
        for (int64_t i = 0; i < nrows; i++) {
          RasterCache &cache = cache_tiles.tile_at_index(i, j);

          IOffset tile_screen_physical_offset{i * kTilePhysicalExtent.width,
                                              j * kTilePhysicalExtent.height};

          IRect tile_screen_physical_rect{tile_screen_physical_offset,
                                          kTilePhysicalExtent};

          if (tile_screen_physical_rect.overlaps(backing_store_physical_rect)) {
            cache.write_to(*sk_canvas, tile_screen_physical_offset -
                                           backing_store_physical_offset);
          }
        }
      }

      backing_store_dirty = false;
    }

    return backing_store_diff;
  }
};

}  // namespace ui
}  // namespace vlk
