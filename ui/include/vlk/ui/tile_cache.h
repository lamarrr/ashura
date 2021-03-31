#pragma once

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

// TODO(lamarrr): won't there be other parts of the program needing to use
// the on_view_offset_dirty and on_raster_dirty callbacks?

struct TileCache {
  // both raster and view widgets are added here. when a view widget's offset
  // are dirty, it marks its spanning raster tiles as dirty
  struct Entry {
    ZIndex zindex;

    Widget *widget;

    Widget::Type type;

    // can we use a single view widget to represent the viewport, so all of its
    // chilren reference it?
    IOffset const *screen_offset;

    // represents the extent of the widget
    Extent const *extent;

    ViewTree::View const *parent_view;

    void draw(RasterCache &cache, Rect const &cache_screen_area) const {
      // use tile index and size to determine tile position on the screen and
      // use that as a translation matrix relative to the objects own position
      // on the screen

      // TODO(lamarrr!!!!): we need a clip on this widget irregardless, there's
      // no guarantee that the widget will not exceed its region while drawing
      // itself

      IRect const widget_screen_area =
          IRect{*this->screen_offset, *this->extent};

      VLK_DEBUG_ENSURE(IRect(cache_screen_area).overlaps(widget_screen_area));

      Canvas canvas = cache.get_recording_canvas();
      SkCanvas *sk_canvas =
          canvas.as_skia().expect("not implemented for non-skia backend");

      IOffset translation =
          IOffset{widget_screen_area.offset.x - cache_screen_area.offset.x,
                  widget_screen_area.offset.y - cache_screen_area.offset.y};

      sk_canvas->setMatrix(SkMatrix::Translate(translation.x, translation.y));

      IRect clip_rect = widget_screen_area;

      ViewTree::View const *ancestor = parent_view;

      while (ancestor != nullptr) {
        IRect ancestor_view_screen_rect{ancestor->screen_offset,
                                        ancestor->layout_node->self_extent};

        if (!ancestor_view_screen_rect.overlaps(clip_rect)) {
          clip_rect = IRect{IOffset{}, Extent{}};
          break;
        } else {
          clip_rect = ancestor_view_screen_rect.intersect(clip_rect);
        }

        ancestor = ancestor->parent;
      }

      if (clip_rect == widget_screen_area) {
        // draw without clip
        widget->draw(canvas);
      } else if (clip_rect.extent.visible()) {
        // draw with clip

        // backup matrix and clip state
        sk_canvas->save();

        // apply clip
        // TODO(lamarrr): review once more
        int64_t clip_start_x = clip_rect.offset.x - widget_screen_area.offset.x;
        int64_t clip_start_y = clip_rect.offset.y - widget_screen_area.offset.y;

        sk_canvas->clipRect(SkRect::MakeXYWH(clip_start_x, clip_start_y,
                                             clip_rect.extent.width,
                                             clip_rect.extent.height));

        widget->draw(canvas);

        // restore matrix and clip state
        sk_canvas->restore();
      } else {
        // draw nothing
      }

      sk_canvas->resetMatrix();
    }
  };

  RasterContext *context = nullptr;

  std::vector<Entry> entries{};

  // must be very small to reserve space, make 2 and increase as necessary
  // oov means out-of-view or out-of-focus
  Ticks max_oof_ticks = Ticks{2};

  RasterTiles tiles{Extent{0, 0}, Extent{256, 256}};

  std::vector<bool> tile_is_dirty{};
  std::vector<bool> tile_is_in_focus{};
  std::vector<Ticks> tile_oof_ticks{};

  // needs to re-bind callbacks
  // void update_extent(Extent const &new_extent) { tiles.resize(new_extent); }

  // void build(ViewTree::View &view);
  // void build(ViewTree::View::Entry &entry);

  void build(ViewTree &view_tree, RasterContext &context) {
    // TODO(lamarrr): logic for tile focusing
    // update extent
    // resize tile_is_dirty and set all to false
    // resize tile_is_in_focus and set all to false
    // reisze tile_oov_ticks
  }

  void rebuild(ViewTree &view_tree) {}

  // TODO(lamarrr): this should be done when building the entry
  void bind() {
    for (size_t entry_index = 0; entry_index < entries.size(); entry_index++) {
      Entry &entry = entries[entry_index];

      WidgetStateProxyAccessor::access(*entry.widget).on_render_dirty =
          [this, entry_index]() {
            Entry &entry = this->entries[entry_index];

            for (uint32_t j =
                     (entry.screen_offset->y) / this->tiles.extent().height;
                 j < (entry.screen_offset->y + entry.extent->height) /
                         this->tiles.extent().height;
                 j++)
              for (uint32_t i =
                       (entry.screen_offset->x) / this->tiles.extent().width;
                   i < (entry.screen_offset->x + entry.extent->width) /
                           this->tiles.extent().width;
                   i++) {
                this->tile_is_dirty[j * this->tiles.rows() + i] = true;
              }
          };
    }
  }

  void tick(std::chrono::nanoseconds const &) {
    {
      size_t i = 0;

      // subtiles should be marked as dirty and as in focus or out of focus as
      // necessary before entering here
      for (RasterCache &subtile : tiles.get_tiles()) {
        if (tile_is_dirty[i]) {
          subtile.discard_recording();
          subtile.begin_recording();

          if (tile_is_in_focus[i]) {
            tile_oof_ticks[i].reset();
          } else {
            tile_oof_ticks[i]++;
            if (tile_oof_ticks[i] > max_oof_ticks) {
              subtile.deinit_surface();
              // recording is always kept and not discarded
            }
          }
        }

        i++;
      }
    }

    // recordings are updated even when not in view
    for (Entry &entry : entries) {
      // viewport affecting it?
      // raster tile dirtiness is also affected by movement of the widgets
      // (typically by viewport scrolling), but the viewport itself
      // invalidates the whole area so this is solved? won't the viewport also
      // be doing too much work? no since all its widget will be moved
      for (uint32_t j = (entry.screen_offset->y) / tiles.extent().height;
           j < (entry.screen_offset->y + entry.extent->height) /
                   tiles.extent().height;
           j++)
        for (uint32_t i = (entry.screen_offset->x) / tiles.extent().width;
             i < (entry.screen_offset->x + entry.extent->width) /
                     tiles.extent().width;
             i++) {
          RasterCache &subtile = tiles.tile_at_index(i, j);

          if (tile_is_dirty[j * tiles.rows() + i]) {
            // draw to appropriate position relative to the tile size. and
            // also respect the view clipping
            Extent tile_extent = tiles.tile_extent();
            Offset tile_screen_offset =
                Offset{i * tile_extent.width, j * tile_extent.height};

            entry.draw(subtile, Rect{tile_screen_offset, tile_extent});
          }
        }
    }

    {
      size_t i = 0;
      for (RasterCache &subtile : tiles.get_tiles()) {
        if (tile_is_dirty[i]) {
          subtile.finish_recording();
          tile_is_dirty[i] = false;

          // tile caches are only updated if the tile is in focus
          // we need to submit
          if (tile_oof_ticks[i] <= max_oof_ticks) {
            subtile.rasterize();
            // TODO(lamarrr): update backing store with tile
          }
        }
      }
    }
  }

  // about breaking text into multiple ones, we can: raterize on the CPU and
  // divide as necessary using a text widget
  // or split into multiple widgets outselves using the text metrics
  // waiting for vsync so we don't do too much work
  /// IMPORTANT: make a visualization of the text boundaries
};

}  // namespace ui
}  // namespace vlk
