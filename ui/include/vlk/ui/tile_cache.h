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

// cache invalidation sources:
// - view offset change
// - viewport resize
// - layout change
// - viewport scrolling
//

constexpr auto get_tile_region(Extent const &tile_extent, int64_t const nrows,
                               int64_t const ncols, IRect const &region) {
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
  auto const [x_min, x_max, y_min, y_max] = region.bounds();

  int64_t const i_begin = x_min / tile_extent.width;
  int64_t const i_end = ((x_max + tile_extent.width) - 1) / tile_extent.width;

  int64_t const j_begin = y_min / tile_extent.height;
  int64_t const j_end = ((y_max + tile_extent.height) - 1) / tile_extent.height;

  int64_t const i_begin_c = std::clamp<int64_t>(i_begin, 0, nrows);
  int64_t const i_end_c = std::clamp<int64_t>(i_end, 0, nrows);

  int64_t const j_begin_c = std::clamp<int64_t>(j_begin, 0, ncols);
  int64_t const j_end_c = std::clamp<int64_t>(j_end, 0, ncols);

  return std::make_tuple(i_begin_c, i_end_c, j_begin_c, j_end_c);
}

// consider making the parent inject the effects and add them to an effect
// tree, with all of the widgets having individual effects as a result we
// need to be able to render the effects independent of the widget, we'll
// thus need bindings for them

struct TileCache {
  // both raster and view widgets are added here. when a view widget's offset
  // are dirty, it marks its spanning raster tiles as dirty
  struct Entry {
    ZIndex z_index;

    // can we use a single view widget to represent the viewport, so all of its
    // chilren reference it?
    Widget *widget;

    IOffset const *screen_offset;

    // represents the extent of the widget
    Extent const *extent;

    IRect const *clip_rect;

    // invalid
    Entry()
        : z_index{0},
          widget{nullptr},
          screen_offset{nullptr},
          extent{nullptr} {}

    explicit Entry(ViewTree::View::Entry const &entry) {
      z_index = entry.z_index;
      widget = entry.layout_node->widget;
      screen_offset = &entry.screen_offset;
      extent = &entry.layout_node->self_extent;
      clip_rect = &entry.clip_rect;
    }

    void draw(RasterCache &cache, IRect const &tile_screen_area) const {
      // use tile index and size to determine tile position on the screen and
      // use that as a translation matrix relative to the objects own position
      // on the screen

      // widgets need to use a clip on the provided extent if they know they
      // can't exactly use the provided extent on the canvas. with this
      // procedure, the widget can exceed its allotted region while drawing
      // itself

      // points to the portion of the widget that would be visible
      IRect const widget_clip_rect = *clip_rect;

      IRect const widget_screen_area = IRect{*screen_offset, *extent};

      VLK_ENSURE(IRect(tile_screen_area).overlaps(widget_screen_area));

      Canvas canvas = cache.get_recording_canvas();
      SkCanvas &sk_canvas = canvas.to_skia();

      Canvas widget_canvas{sk_canvas, widget_screen_area.extent};

      // backup matrix and clip state
      sk_canvas.save();

      IOffset const translation =
          widget_screen_area.offset - tile_screen_area.offset;

      sk_canvas.translate(translation.x, translation.y);

      if (widget_clip_rect.visible()) {
        if (widget_clip_rect == widget_screen_area) {
          // draw without clip
          widget->draw(widget_canvas);
        } else {
          // draw with clip

          // apply clip
          // note that this is performed relative to the widget's extent
          IOffset const clip_start =
              widget_clip_rect.offset - widget_screen_area.offset;

          sk_canvas.clipRect(SkRect::MakeXYWH(clip_start.x, clip_start.y,
                                              widget_clip_rect.width(),
                                              widget_clip_rect.height()));
          widget->draw(widget_canvas);
        }
      } else {
        // draw nothing
      }

      // restore matrix and clip state
      sk_canvas.restore();
    }
  };

  TileCache() = default;

  TileCache(TileCache const &) = delete;
  TileCache(TileCache &&) = delete;

  TileCache &operator=(TileCache const &) = delete;
  TileCache &operator=(TileCache &&) = delete;

  ~TileCache() = default;

  // must be very small to reserve space, make 2 and increase as necessary
  // oov means out-of-view or out-of-focus
  // constant througout lifetime.
  static constexpr Ticks max_oof_ticks = Ticks{1};
  static constexpr Extent tile_extent = Extent{256, 256};

  RenderContext const *context = nullptr;

  // entries are sorted in descending z-index order
  std::vector<Entry> entries{};

  IOffset backing_store_offset = IOffset{0, 0};
  bool backing_store_offset_changed = true;

  Extent backing_store_extent = Extent{1920, 1080};  // non-zero
  bool backing_store_resized = true;

  // accumulates the cache result of all the tiles
  // resized on viewport resize.
  RasterCache backing_store{backing_store_extent};

  RasterTiles tiles{Extent{0, 0}, tile_extent};
  bool tiles_extent_dirty = true;

  bool any_tile_dirty = true;

  std::vector<bool> tile_is_dirty{};

  std::vector<bool> tile_is_in_focus{};

  std::vector<Ticks> tile_oof_ticks{};

  ViewTree::View *root_view = nullptr;

  // checkerboard or clear color?

  IRect get_backing_store_rect() const {
    return IRect{backing_store_offset, backing_store_extent};
  }

  void scroll_backing_store(IOffset new_offset) {
    if (backing_store_offset != new_offset) {
      backing_store_offset = new_offset;
      backing_store_offset_changed = true;
    }
  }

  void resize_backing_store(Extent new_extent) {
    if (backing_store_extent != new_extent) {
      backing_store_extent = new_extent;
      backing_store_resized = true;
    }
  }

  void mark_tiles_extent_dirty() { tiles_extent_dirty = true; }

  void mark_all_tiles_dirty() {
    any_tile_dirty = true;
    for (size_t i = 0; i < tile_is_dirty.size(); i++) {
      tile_is_dirty[i] = true;
    }
  }

  void mark_backing_store_offset_dirty() {
    backing_store_offset_changed = true;
  }

  void mark_backing_store_render_dirty() { mark_backing_store_offset_dirty(); }

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
      WidgetSystemProxy::get_state_proxy(*entry.widget).on_render_dirty =
          [this, &entry] {
            // note that tile binding is semi-automatic and determined by the
            // screen offset
            int64_t const nrows = this->tiles.rows();
            auto const [i_begin, i_end, j_begin, j_end] = get_tile_region(
                this->tiles.tile_extent(), nrows, this->tiles.columns(),
                IRect{*entry.screen_offset, *entry.extent});

            for (int64_t j = j_begin; j < j_end; j++) {
              for (int64_t i = i_begin; i < i_end; i++) {
                // this is here because it should only mark as dirty when at
                // least one of the actual tiles is dirty
                any_tile_dirty = true;
                tile_is_dirty[j * nrows + i] = true;
              }
            }
          };
    }
  }

  // TODO(lamarrr): raster_context shared_ptr?, even internally? ....
  //
  void build(ViewTree::View &view_tree_root,
             RenderContext const &raster_context) {
    context = &raster_context;

    entries.clear();

    // scroll offset maintained, unless explicitly scrolled

    backing_store_offset_changed = true;

    // viewport extent maintained, unless explicitly resized

    backing_store_resized =
        true;  // forces the tiles to be resized and marked as dirty

    // backing_store maintained, will be invalidated and marked as dirty in
    // tick() due to viewport_resized and viewport_scrolled

    // tiles maintained, will be resized in tick as appropriate and if necessary
    // they will all be forcibly marked dirty due to resize and scroll of
    // viewport

    tiles_extent_dirty = true;

    // any_tile_dirty maintained, will be updated in tick() to true
    // [tiles_extent_dirty]

    // tile_is_dirty maintained, will be resized and updated in tick() to all
    // true [tiles_extent_dirty]

    // tile_is_in_focus maintained, will be resized and updated in tick()
    // [tiles_extent_dirty] tile_oof_ticks maintained, will be updated and
    // resized in tick() [tiles_extent_dirty]

    root_view = &view_tree_root;

    build_entries(view_tree_root);
    attach_state_proxies();
  }

  void tick(std::chrono::nanoseconds) {
    // marks that the backing store needs repainting
    bool backing_store_dirty = false;

    if (backing_store_resized) {
      backing_store = RasterCache{backing_store_extent};
      backing_store.init_surface(*context);

      backing_store_dirty = true;

      backing_store_resized = false;
    }

    if (backing_store_offset_changed) {
      backing_store_dirty = true;

      backing_store_offset_changed = false;
    }

    if (tiles_extent_dirty) {
      // layout changes, screen offsets are automatically updated as long as
      // view_tree is cleaned we therefore need to assume that all the tiles are
      // now dirty. and resize? them
      tiles.resize(root_view->layout_node->self_extent);
      size_t const num_tiles = tiles.get_tiles().size();

      tile_is_dirty.resize(num_tiles);
      tile_is_in_focus.resize(num_tiles);
      tile_oof_ticks.resize(num_tiles);

      any_tile_dirty = true;

      for (size_t i = 0; i < num_tiles; i++) {
        tile_is_dirty[i] = true;
        tile_is_in_focus[i] = false;
        tile_oof_ticks[i] = Ticks{0};
      }

      backing_store_dirty = true;

      tiles_extent_dirty = false;
    }

    IRect const backing_store_rect = get_backing_store_rect();

    for (uint32_t j = 0; j < tiles.columns(); j++) {
      for (uint32_t i = 0; i < tiles.rows(); i++) {
        IOffset const offset{i * tiles.tile_extent().width,
                             j * tiles.tile_extent().height};
        tile_is_in_focus[j * tiles.rows() + i] =
            IRect{offset, tiles.tile_extent()}.overlaps(backing_store_rect);
      }
    }

    // subtiles should be marked as dirty and as in focus or out of focus as
    // necessary before entering here
    for (size_t i = 0; i < tiles.get_tiles().size(); i++) {
      RasterCache &subtile = tiles.get_tiles()[i];

      if (tile_is_in_focus[i] && !subtile.is_surface_init()) {
        // prepare for rasterization
        subtile.init_surface(*context);
      }

      if (tile_is_in_focus[i] && tile_is_dirty[i]) {
        // mark the backing store as dirty
        backing_store_dirty = true;
      }

      if (tile_is_in_focus[i]) {
        tile_oof_ticks[i].reset();
      } else {
        tile_oof_ticks[i]++;
      }

      if (!tile_is_in_focus[i] && tile_oof_ticks[i] > max_oof_ticks) {
        // recording is always kept and not discarded
        // but we need to detach the surface it owns to save memory as we'll
        // have multiple of these tiles in memory and their total number would
        // be proportional to the total extent of the root widget
        // TODO(lamarrr): can we have a stack of surfaces we can pop and attach
        // to? allocating these tiles can be costly, BENCHMARK.
        // consider scroll swapping instead of allocating and de-allocating.
        // i.e. tile surface stack. we might need to re-allocate when the
        // viewport zoom occurs?
        // the stack will always have enough tile surfaces for the tiles covered
        // by the viewport
        subtile.deinit_surface();
        // discard recording
      }

      if (any_tile_dirty) {
        if (tile_is_dirty[i]) {  // && tile_is_in_focus[i], we'll still need to
                                 // take care of any_tile_dirty and our
                                 // dirtiness handling criteria since it is now
                                 // not attended to immediately
          subtile.discard_recording();
          subtile.begin_recording();
        }
      }
    }

    // recordings are updated even when not in view
    if (any_tile_dirty) {
      for (Entry &entry : entries) {
        int64_t const nrows = this->tiles.rows();
        auto const [i_begin, i_end, j_begin, j_end] = get_tile_region(
            this->tiles.tile_extent(), nrows, this->tiles.columns(),
            IRect{*entry.screen_offset, *entry.extent});

        for (int64_t j = j_begin; j < j_end; j++) {
          for (int64_t i = i_begin; i < i_end; i++) {
            RasterCache &subtile = tiles.tile_at_index(i, j);

            // TODO(lamarrr): notify of entering or leaving view irregardless of
            // whether tile is dirty, we'd thus need to not first check
            // `any_tile_dirty`
            if (tile_is_dirty[j * tiles.rows() + i]) {
              // draw to appropriate position relative to the tile size. and
              // also respect the view clipping
              Extent tile_extent = tiles.tile_extent();
              IOffset tile_screen_offset =
                  IOffset{i * tile_extent.width, j * tile_extent.height};

              entry.draw(subtile, IRect{tile_screen_offset, tile_extent});
            }
          }
        }
      }
    }

    if (any_tile_dirty) {
      for (size_t i = 0; i < tiles.get_tiles().size(); i++) {
        RasterCache &subtile = tiles.get_tiles()[i];

        if (tile_is_dirty[i]) {  // && tile_is_in_focus[i]
          subtile.finish_recording();

          // tile caches are only updated if the tile is in focus
          // we need to submit
          if (tile_oof_ticks[i] <= max_oof_ticks) {
            subtile.rasterize();
          }

          tile_is_dirty[i] = false;
        }
      }

      any_tile_dirty = false;
    }

    if (backing_store_dirty) {
      // accumulate raster cache into backing store

      backing_store.begin_recording();

      Canvas canvas = backing_store.get_recording_canvas();
      SkCanvas &sk_canvas = canvas.to_skia();

      Extent tile_extent = tiles.tile_extent();

      for (int64_t j = 0; j < tiles.columns(); j++) {
        for (int64_t i = 0; i < tiles.rows(); i++) {
          RasterCache &subtile = tiles.tile_at_index(i, j);
          IOffset tile_screen_offset =
              IOffset{i * tile_extent.width, j * tile_extent.height};

          if (IRect{tile_screen_offset, tile_extent}.overlaps(
                  backing_store_rect)) {
            subtile.write_to(sk_canvas,
                             tile_screen_offset - backing_store_offset);
          }
        }
      }

      backing_store.finish_recording();
      backing_store.rasterize();

      backing_store_dirty = false;
    }
  }
};

}  // namespace ui
}  // namespace vlk
