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
    // represents the whole extent of the widget (equivalent to viewtree's
    // .parent_view_area.extent)
    Extent const *extent;

    // what we actually have is a series of nested clips and not a single one?
    // each view applies a clip on the widgets ClipTree::Node *clip;
    ViewTree::View const *parent_view;

    RasterContext *context;

    void draw(RasterCache &cache, Rect const &cache_screen_area) const {
      // use tile index and size to determine tile position on the screen and
      // use that as a translation matrix relative to the objects own position
      // on the screen

      IRect const widget_screen_area =
          IRect{*this->screen_offset, *this->extent};

      VLK_DEBUG_ENSURE(IRect(cache_screen_area).overlaps(widget_screen_area));

      Canvas canvas = cache.get_recording_canvas();
      SkCanvas *sk_canvas = canvas.as_skia();

      IOffset translation =
          IOffset{widget_screen_area.offset.x - cache_screen_area.offset.x,
                  widget_screen_area.offset.y - cache_screen_area.offset.y};

      sk_canvas->setMatrix(SkMatrix::Translate(translation.x, translation.y));

      if (IRect(cache_screen_area).contains(widget_screen_area)) {
        // draw without clip
        widget->draw(canvas);
      } else {
        // draw with clip
        IRect clip_rect = widget_screen_area;

        ViewTree::View const *ancestor = parent_view;

        while (ancestor != nullptr) {
          IRect ancestor_view_screen_rect{ancestor->screen_offset_,
                                          ancestor->parent_view_area_.extent};

          if (!ancestor_view_screen_rect.overlaps(clip_rect)) {
            clip_rect = IRect{IOffset{}, Extent{}};
            break;
          } else {
            clip_rect = ancestor_view_screen_rect.intersect(clip_rect);
          }

          ancestor = ancestor->parent_;
        }

        if (clip_rect.extent.is_visible()) {
          // backup matrix and clip state
          sk_canvas->save();

          // apply clip
          int64_t clip_start_x =
              clip_rect.offset.x - widget_screen_area.offset.x;
          int64_t clip_start_y =
              clip_rect.offset.y - widget_screen_area.offset.y;

          sk_canvas->clipRect(SkRect::MakeXYWH(clip_start_x, clip_start_y,
                                               clip_rect.extent.width,
                                               clip_rect.extent.height));

          widget->draw(canvas);

          // restore matrix and clip state
          sk_canvas->restore();
        }
      }

      sk_canvas->resetMatrix();
    }
  };

  std::vector<Entry> entries_;

  // must be very small  to reserve space, make 1 and increase as necessary
  Ticks max_oov_ticks_ = Ticks{1};

  RasterTiles tile_;
  std::vector<bool> tile_is_dirty_;
  // bool any_in_focus_tile_dirty_;

  std::vector<bool> tile_is_in_focus_;
  std::vector<Ticks> tile_out_of_view_ticks_;

  uint64_t bytes_size();

  // using the layout tree
  void build();

  void bind() {
    for (size_t entry_index = 0; entry_index < entries_.size(); entry_index++) {
      // won't there be other parts of the program needing to use the
      // on_view_offset_dirty and on_raster_dirty callbacks?
      Entry &entry = entries_[entry_index];

      WidgetStateProxyAccessor::access(*(entry.widget)).on_render_dirty =
          [this, entry_index]() {
            Entry &entry = this->entries_[entry_index];

            for (uint32_t j =
                     (entry.screen_offset->y) / this->tile_.extent().height;
                 j < (entry.screen_offset->y + entry.extent->height) /
                         this->tile_.extent().height;
                 j++)
              for (uint32_t i =
                       (entry.screen_offset->x) / this->tile_.extent().width;
                   i < (entry.screen_offset->x + entry.extent->width) /
                           this->tile_.extent().width;
                   i++) {
                RasterCache &subtile = this->tile_.tile_at_index(i, j);
                this->tile_is_dirty_[j * this->tile_.rows() + i] = true;
              }
          };
    }
  }

  void tick([[maybe_unused]] std::chrono::nanoseconds const &interval) {
    {
      size_t i = 0;

      // subtiles should be marked as dirty and as in focus or out of focus as
      // necessary before entering here
      for (RasterCache &subtile : tile_.get_tiles()) {
        if (tile_is_dirty_[i]) {
          subtile.discard_recording();
          subtile.begin_recording();

          if (tile_is_in_focus_[i]) {
            tile_out_of_view_ticks_[i].reset();
          } else {
            tile_out_of_view_ticks_[i]++;
            if (tile_out_of_view_ticks_[i] > max_oov_ticks_) {
              subtile.deinit_surface();
              // recording is always kept and not discarded
            }
          }
        }

        i++;
      }
    }

    // recordings are updated even when not in view
    for (Entry &entry : entries_) {
      // viewport affecting it?
      // raster tile dirtiness is also affected by movement of the widgets
      // (typically by viewport scrolling), but the viewport itself
      // invalidates the whole area so this is solved? won't the viewport also
      // be doing too much work? no since all its widget will be moved
      for (uint32_t j = (entry.screen_offset->y) / tile_.extent().height;
           j < (entry.screen_offset->y + entry.extent->height) /
                   tile_.extent().height;
           j++)
        for (uint32_t i = (entry.screen_offset->x) / tile_.extent().width;
             i < (entry.screen_offset->x + entry.extent->width) /
                     tile_.extent().width;
             i++) {
          RasterCache &subtile = tile_.tile_at_index(i, j);

          if (tile_is_dirty_[j * tile_.rows() + i]) {
            Canvas canvas = subtile.get_recording_canvas();

            // draw to appropriate position relative to the tile size. and
            // also respect the viewport cropping
            Extent tile_extent = tile_.tile_extent();
            Offset tile_screen_offset =
                Offset{tile_screen_offset.x = i * tile_extent.width,
                       tile_screen_offset.y = j * tile_extent.height};

            entry.draw(subtile, Rect{tile_screen_offset, tile_extent});
          }
        }
    }

    {
      size_t i = 0;
      for (RasterCache &subtile : tile_.get_tiles()) {
        if (tile_is_dirty_[i]) {
          subtile.finish_recording();
          tile_is_dirty_[i] = false;

          // tile caches are only updated if the tile is in focus
          // we need to submit
          if (tile_out_of_view_ticks_[i] <= max_oov_ticks_) {
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
