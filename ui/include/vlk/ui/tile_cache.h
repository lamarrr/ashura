#pragma once

#include <vector>

#include "vlk/ui/impl/widget_state_proxy_accessor.h"
#include "vlk/ui/primitives.h"
#include "vlk/ui/raster_cache.h"
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
    // represents the extent of the widget to be drawn at offset `screen_offset`
    Extent const *extent;

    // what we actually have is a series of nested clips and not a single one?
    // each view applies a clip on the widgets ClipTree::Node *clip;
    ViewTree::View const *parent_view;

    // TODO(lamarrr): cross-check all the ensure uses
    void draw(RasterCache &cache, Rect const &cache_screen_area) const {
      // use tile index and size to determine tile position on the screen and
      // use that as a translation matrix relative to the objects own position
      // on the screen

      IRect const widget_area = IRect{*screen_offset, *extent};

      VLK_DEBUG_ENSURE(IRect(cache_screen_area).overlaps(widget_area));

      IRect clip_rect = widget_area;

      ViewTree::View const *view = parent_view;

      while (view != nullptr) {
        IRect parent_view_rect{parent_view->screen_offset_,
                               parent_view->parent_view_area_.extent};

        if (!parent_view_rect.overlaps(clip_rect)) {
          clip_rect = IRect{IOffset{}, Extent{}};
          break;
        } else {
          clip_rect = parent_view_rect.intersect(clip_rect);
        }

        view = view->parent_;
      }

      Canvas canvas = cache.get_recording_canvas();

      if (IRect(cache_screen_area).contains(clip_rect)) {
        // draw without clip
        widget->draw(canvas);
      } else {
        // draw with clip
      }
    }
  };

  std::vector<Entry> entries_;

  // for view widgets this means their offset changed and for raster widgets,
  // this means their raster (draw commands changed).
  // std::vector<bool> entry_is_dirty_;

  // must be very small  to reserve space, make 1 and increase as necessary
  Ticks max_oov_ticks_;

  RasterTiles tile_;
  std::vector<bool> tile_is_dirty_;
  // bool any_in_focus_tile_dirty_;

  std::vector<bool> tile_is_in_focus_;
  std::vector<Ticks> tile_out_of_view_ticks_;

  // RasterTaskScheduler scheduler_;

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
            // TODO(lamarrr): this needs to depend on the widget's and tile's
            // coordinates
            // FIXME: correct rect
            entry.draw(subtile, Rect{i * tile_.extent().width});
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
          if (tile_is_in_focus_[i]) {
            // the aggregator should be aware of the multithreaded nature of the
            // tile and possibly offer pending updating
            // scheduler_.submit_tile(subtile);
          }
        }
      }
    }

    // pool.await_all_submissions();
  }
  // about breaking text into multiple ones, we can: raterize on the CPU and
  // divide as necessary using a text widget
  //
  // waiting for vsync so we don't do too much work
};

}  // namespace ui
}  // namespace vlk
