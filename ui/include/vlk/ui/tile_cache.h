#pragma once

#include <algorithm>
#include <queue>
#include <vector>

#include "vlk/ui/impl/widget_state_proxy_accessor.h"
#include "vlk/ui/primitives.h"
#include "vlk/ui/raster_cache.h"
#include "vlk/ui/raster_context.h"
#include "vlk/ui/raster_tiles.h"
#include "vlk/ui/view_tree.h"
#include "vlk/ui/widget.h"

namespace vlk {
namespace ui {

// TODO(lamarrr): TiledRecorder
// SlidingRasterizer with placeholder?

// cache invalidation sources:
// - view offset change
// - viewport resize
// - layout change
// - viewport scrolling
//

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

    // non-null
    ViewTree::View const *parent_view;

    // invalid
    Entry()
        : z_index{0},
          widget{nullptr},
          screen_offset{nullptr},
          extent{nullptr},
          parent_view{nullptr} {}

    Entry(ViewTree::View::Entry const &entry,
          ViewTree::View const &init_parent_view) {
      z_index = entry.z_index;
      widget = entry.layout_node->widget;
      screen_offset = &entry.screen_offset;
      extent = &entry.layout_node->self_extent;
      parent_view = &init_parent_view;
    }

    void draw(RasterCache &cache, IRect const &tile_screen_area) const {
      // use tile index and size to determine tile position on the screen and
      // use that as a translation matrix relative to the objects own position
      // on the screen

      // widgets need to use a clip on the provided extent if they know they
      // can't exactly use the provided extent on the canvas. with this
      // procedure, the widget can exceed its allotted region while drawing
      // itself

      IRect const widget_screen_area = IRect{*screen_offset, *extent};

      VLK_DEBUG_ENSURE(IRect(tile_screen_area).overlaps(widget_screen_area));

      // points to the portion of the widget that would be visible
      IRect clip_rect = widget_screen_area;

      // TODO(lamarrr): move clip rect processing
      // only mark intersecting tiles as dirty if its clip rect is visible in
      // both scenarios

      ViewTree::View const *ancestor = parent_view;

      while (ancestor != nullptr && clip_rect.visible()) {
        IRect ancestor_view_screen_rect{ancestor->screen_offset,
                                        ancestor->layout_node->self_extent};

        if (!ancestor_view_screen_rect.overlaps(clip_rect)) {
          clip_rect = clip_rect.with_extent(Extent{});
        } else {
          clip_rect = ancestor_view_screen_rect.intersect(clip_rect);
        }

        ancestor = ancestor->parent;
      }

      Canvas canvas = cache.get_recording_canvas();
      SkCanvas *sk_canvas =
          canvas.as_skia().expect("canvas backend is not Skia");

      Canvas widget_canvas{*sk_canvas, widget_screen_area.extent};

      // backup matrix and clip state
      sk_canvas->save();

      IOffset const translation =
          widget_screen_area.offset - tile_screen_area.offset;

      sk_canvas->translate(translation.x, translation.y);

      if (clip_rect == widget_screen_area) {
        // draw without clip
        widget->draw(widget_canvas);
      } else if (clip_rect.visible()) {
        // draw with clip

        // apply clip
        // note that this is performed relative to the widget's extent
        IOffset const clip_start = clip_rect.offset - widget_screen_area.offset;

        sk_canvas->clipRect(SkRect::MakeXYWH(
            clip_start.x, clip_start.y, clip_rect.width(), clip_rect.height()));

        widget->draw(widget_canvas);

      } else {
        // draw nothing
      }

      // restore matrix and clip state
      sk_canvas->restore();
    }
  };

  TileCache() = default;

  TileCache(TileCache const &) = delete;
  TileCache(TileCache &&) = delete;

  TileCache &operator=(TileCache const &) = delete;
  TileCache &operator=(TileCache &&) = delete;

  ~TileCache() = default;

  RasterContext *context = nullptr;

  // entries are sorted in descending z-index order
  std::vector<Entry> entries{};

  IOffset viewport_scroll_offset = IOffset{0, 0};
  bool viewport_scrolled = true;

  Extent viewport_extent = Extent{1920, 1080};
  bool viewport_resized = true;

  // this is used for preloading some of the tiles
  // constant throughout lifetime.
  Extent focus_extension = Extent{0, 0};

  // must be very small to reserve space, make 2 and increase as necessary
  // oov means out-of-view or out-of-focus
  // constant througout lifetime.
  Ticks max_oof_ticks = Ticks{1};

  // accumulates the cache result of all the tiles
  // resized on viewport resize.
  RasterCache backing_store{viewport_extent};

  RasterTiles tiles{Extent{0, 0}, Extent{256, 256}};

  bool any_tile_dirty = true;
  std::vector<bool> tile_is_dirty{};

  std::vector<bool> tile_is_in_focus{};

  std::vector<Ticks> tile_oof_ticks{};

  ViewTree::View const *root_view = nullptr;

  // focusing helps us preload a part of the screen into the tiles
  IRect get_focus_rect() const {
    uint32_t const focus_x = focus_extension.width / 2;
    uint32_t const focus_y = focus_extension.height / 2;

    int64_t const x_min = viewport_scroll_offset.x - focus_x;
    int64_t const y_min = viewport_scroll_offset.y - focus_y;

    return IRect{IOffset{x_min, y_min},
                 Extent{viewport_extent.width + focus_extension.width,
                        viewport_extent.height + focus_extension.height}};
  }

  IRect get_viewport_rect() const {
    return IRect{viewport_scroll_offset, viewport_extent};
  }

  void scroll_viewport(ViewOffset const &new_viewport_offset) {
    viewport_scroll_offset = new_viewport_offset.resolve(viewport_extent);
    viewport_scrolled = true;
  }

  void resize_viewport(Extent const &new_viewport_extent) {
    viewport_extent = new_viewport_extent;
    viewport_resized = true;
  }

 public:
  void build_from(ViewTree::View &view) {
    // insert by z-index order
    for (ViewTree::View::Entry &view_entry : view.entries) {
      auto const insert_pos =
          std::lower_bound(entries.begin(), entries.end(), view_entry,
                           [](Entry const &a, ViewTree::View::Entry const &b) {
                             return a.z_index < b.z_index;
                           });
      entries.insert(insert_pos, Entry{view_entry, view});
    }

    for (ViewTree::View &subview : view.subviews) {
      build_from(subview);
    }
  }

  void build(ViewTree &view_tree, RasterContext &raster_context) {
    context = &raster_context;

    root_view = &view_tree.root_view;
    build_from(view_tree.root_view);

    for (Entry &entry : entries) {
      WidgetStateProxyAccessor::access(*entry.widget).on_render_dirty =
          [this, &entry] {
            // note that tile binding is semi-automatic and determined by the
            // screen offset

            Extent const tile_extent = this->tiles.tile_extent();

            // the .screen_offset can exceed the whole screen's extent (due to
            // scrolling) so we need to clamp
            int64_t const x_min = entry.screen_offset->x;
            int64_t const x_max = x_min + entry.extent->width;

            int64_t const y_min = entry.screen_offset->y;
            int64_t const y_max = y_min + entry.extent->height;

            int64_t const j_begin = y_min / tile_extent.height;
            int64_t const j_end = (y_max / tile_extent.height) + 1;

            int64_t const i_begin = x_min / tile_extent.width;
            int64_t const i_end = (x_max / tile_extent.width) + 1;

            int64_t const ncols = this->tiles.columns();
            int64_t const nrows = this->tiles.rows();

            // the widget could be an outlieing widget
            //
            // screen offset can be negative
            //
            // we don't want to clamp so we don't mark the tiles at the edges as
            // dirty (if any)
            // also note that the `end`s can be negative
            for (int64_t j = j_begin; j < j_end && j < ncols; j++) {
              if (j < 0) continue;
              for (int64_t i = i_begin; i < i_end && i < nrows; i++) {
                if (i < 0) continue;
                // this is here because it should only mark as dirty when at
                // least one of the actual tiles is dirty
                any_tile_dirty = true;
                tile_is_dirty[j * nrows + i] = true;
              }
            }
          };
    }

    tick(std::chrono::nanoseconds(0));
  }

  void recycle(ViewTree &view_tree) {}

  // consider scroll swapping instead of allocating and de-allocating

  void tick(std::chrono::nanoseconds const &interval) {
    // TODO(lamarrr): fix and make all correct

    // for now, we'll check if any in focus tile is dirty to make the backing
    // store dirty instead of any in viewport tile dirty
    bool backing_store_dirty =
        viewport_resized ||
        viewport_scrolled;  // || any_ in viewport tile dirty;

    if (viewport_resized) {
      backing_store = RasterCache{viewport_extent};
      backing_store.init_surface(*context);

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
        tile_is_in_focus[i] = true;
        tile_oof_ticks[i] = Ticks{max_oof_ticks};
      }

      viewport_resized = false;
    }

    if (viewport_scrolled) {
      // just marks the backing store as dirty
      viewport_scrolled = false;
    }

    IRect const focus_rect = get_focus_rect();

    for (uint32_t j = 0; j < tiles.columns(); j++) {
      for (uint32_t i = 0; i < tiles.rows(); i++) {
        IOffset const offset{i * tiles.tile_extent().width,
                             j * tiles.tile_extent().height};
        tile_is_in_focus[j * tiles.rows() + i] =
            IRect{offset, tiles.tile_extent()}.overlaps(focus_rect);
      }
    }

    if (any_tile_dirty) {
      // subtiles should be marked as dirty and as in focus or out of focus as
      // necessary before entering here
      for (size_t i = 0; i < tiles.get_tiles().size(); i++) {
        RasterCache &subtile = tiles.get_tiles()[i];

        if (tile_is_dirty[i]) {
          subtile.discard_recording();
          subtile.begin_recording();

          if (tile_is_in_focus[i]) {
            // prepare for rasterization
            if (!subtile.is_surface_init()) {
              subtile.init_surface(*context);
            }

            tile_oof_ticks[i].reset();

            // mark the backing store as dirty
            backing_store_dirty = true;
          } else {
            tile_oof_ticks[i]++;
            if (tile_oof_ticks[i] > max_oof_ticks) {
              subtile.deinit_surface();
              // recording is always kept and not discarded
            } else {
              // prepare for rasterization
              if (!subtile.is_surface_init()) {
                subtile.init_surface(*context);
              }
            }
          }
        }
      }
    }

    // recordings are updated even when not in view
    if (any_tile_dirty) {
      for (Entry &entry : entries) {
        // viewport affecting it?
        // raster tile dirtiness is also affected by movement of the widgets
        // (typically by viewport scrolling), but the viewport itself
        // invalidates the whole area so this is solved? won't the viewport also
        // be doing too much work? no since all its widget will be moved

        Extent const tile_extent = this->tiles.tile_extent();

        int64_t const x_min = entry.screen_offset->x;
        int64_t const x_max = x_min + entry.extent->width;

        int64_t const y_min = entry.screen_offset->y;
        int64_t const y_max = y_min + entry.extent->height;

        int64_t const j_begin = y_min / tile_extent.height;
        int64_t const j_end = (y_max / tile_extent.height) + 1;

        int64_t const i_begin = x_min / tile_extent.width;
        int64_t const i_end = (x_max / tile_extent.width) + 1;

        int64_t const ncols = this->tiles.columns();
        int64_t const nrows = this->tiles.rows();

        for (int64_t j = j_begin; j < j_end && j < ncols; j++) {
          if (j < 0) continue;
          for (int64_t i = i_begin; i < i_end && i < nrows; i++) {
            if (i < 0) continue;
            RasterCache &subtile = tiles.tile_at_index(i, j);

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

        if (tile_is_dirty[i]) {
          subtile.finish_recording();

          // tile caches are only updated if the tile is in focus
          // we need to submit
          if (tile_oof_ticks[i] <= max_oof_ticks) {
            subtile.rasterize();
            backing_store_dirty = true;
          }

          tile_is_dirty[i] = false;
        }
      }

      any_tile_dirty = false;
    }

    if (backing_store_dirty) {
      // accumulate raster cache

      backing_store.begin_recording();

      Canvas canvas = backing_store.get_recording_canvas();
      SkCanvas *sk_canvas =
          canvas.as_skia().expect("canvas is not using a skia backend");
      sk_canvas->clear(SkColors::kTransparent);

      Extent tile_extent = tiles.tile_extent();

      for (int64_t j = 0; j < tiles.columns(); j++) {
        for (int64_t i = 0; i < tiles.rows(); i++) {
          RasterCache &subtile = tiles.tile_at_index(i, j);
          IOffset tile_screen_offset =
              IOffset{i * tile_extent.width, j * tile_extent.height};

          if (IRect{tile_screen_offset, tile_extent}.overlaps(
                  IRect{viewport_scroll_offset, viewport_extent})) {
            subtile.write_to(*sk_canvas,
                             tile_screen_offset - viewport_scroll_offset);
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
