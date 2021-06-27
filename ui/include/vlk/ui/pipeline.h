#pragma once

#include "vlk/ui/asset_manager.h"
#include "vlk/ui/layout.h"
#include "vlk/ui/layout_tree.h"
#include "vlk/ui/tile_cache.h"
#include "vlk/ui/view_tree.h"
#include "vlk/ui/viewport.h"
#include "vlk/ui/widget.h"

namespace vlk {
namespace ui {

struct Pipeline {
  Widget* root_widget;
  Viewport viewport;
  RenderContext raster_context;

  LayoutTree layout_tree;
  ViewTree view_tree;
  TileCache tile_cache;
  AssetManager asset_manager;

  bool needs_rebuild = false;
  bool is_first_tick = true;

  Pipeline(Widget& init_root_widget,
           Viewport init_viewport = Viewport{Extent{1920, 1080}, ViewOffset{}},
           RenderContext init_raster_context = RenderContext{})
      : root_widget{&init_root_widget},
        viewport{init_viewport},
        raster_context{std::move(init_raster_context)},
        layout_tree{},
        view_tree{},
        tile_cache{},
        asset_manager{raster_context} {
    // pass widget to pipeline => bind on children changed
    // pass to layout tree => bind on layout changed
    // pass to view tree => bind on view offset changed
    // pass to tile_cache => bind on raster dirty

    attach_state_proxies(*root_widget);

    // we might need to tell the root view to expand or shrink? or preserve its
    // requested size?
    //
    //
    // we might need to force this on the layout and view tree
    //
    // the root view should not scroll it's self_extent should correspond to the
    // tiles extent
    // does the layout tree take care of this? how?
    // or the root view should make its self_extent equal to the view_extent
    // both can be allocated infinite extents

    // TODO(lamarrr): handle zero-extent viewport

    ViewportSystemProxy::get_state_proxy(viewport).on_resize = [this] {
      Extent const new_viewport_extent = viewport.get_extent();
      Extent const widgets_allocation = viewport.get_widgets_allocation();
      layout_tree.allot_extent(widgets_allocation);
      tile_cache.resize_backing_store(new_viewport_extent);
    };

    ViewportSystemProxy::get_state_proxy(viewport).on_scroll = [this] {
      tile_cache.scroll_backing_store(viewport.get_offset());
    };
  }

  Pipeline(Pipeline&&) = delete;
  Pipeline(Pipeline const&) = delete;
  Pipeline& operator=(Pipeline const&) = delete;
  Pipeline& operator=(Pipeline&&) = delete;

  void attach_state_proxies(Widget& widget) {
    WidgetSystemProxy::get_state_proxy(widget).on_children_changed = [this] {
      this->needs_rebuild = true;
    };

    for (Widget* child : widget.get_children()) {
      attach_state_proxies(*child);
    }
  }

  void recursive_tick(Widget& widget, std::chrono::nanoseconds interval) {
    // child will be removed as necessary from the tick callback. of course we
    // assume that the children have been deleted by the parent so there's no
    // need to detach the previously attached state proxies. this also means we
    // can't touch the previous children we got. this also implies that a
    // detached child must not use its state proxy callbacks or have its
    // `system_tick` method called unless its state proxies have been updated.

    WidgetSystemProxy::tick(widget, interval, asset_manager);
    WidgetSystemProxy::mark_stale(widget);

    for (Widget* child : widget.get_children()) {
      // note that we only touch the latest updated pointers to the children,
      // and if the widget updated its children, we'll rebuild the trees
      recursive_tick(*child, interval);
    }
  }

  void tick(std::chrono::nanoseconds interval) {
    recursive_tick(*root_widget, interval);

    if (needs_rebuild || is_first_tick) {
      // note that each build method is optimized for rebuilding and should not
      // re-allocate too much of memory in the case where the tree sizes don't
      // really change and we thus have a `build()` method separated from the
      // constructor that uses `.resize` and `.clear` for their vectors to
      // prevent forcing a memory re-allocation when the available space is
      // enough
      layout_tree.build(*root_widget);
      view_tree.build(layout_tree.root_node);
      tile_cache.build(view_tree.root_view, raster_context);
      needs_rebuild = false;
      is_first_tick = false;
    }

    ViewportSystemProxy::tick(viewport);

    bool const layout_tree_was_cleaned = layout_tree.is_layout_dirty;
    layout_tree.tick(interval);

    // if layout tree becomes dirty we need to force a total re-draw by marking
    // all of the tiles as dirty
    if (layout_tree_was_cleaned) {
      view_tree.mark_views_dirty();
      // resize tiles to layout extent
      tile_cache.mark_tiles_extent_dirty();
      // NOTE: resizing might not trigger a dirtiness event in some
      // cases and we seem to over-rely on that in many parts of the codebase
    }

    view_tree.tick(interval);
    tile_cache.tick(interval);
    asset_manager.tick(interval);
  }
};

}  // namespace ui
}  // namespace vlk
