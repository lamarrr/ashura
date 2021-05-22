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

// TODO(lamarrr): make this a virtual function? with a unique_ptr static
// ::default return to abstract from the user and reduce compile-time or
// unrequired dependencies
// this will be in the impl namespace
struct Pipeline {
  Widget* root_widget;
  impl::Viewport viewport;
  RasterContext raster_context;

  LayoutTree layout_tree;
  ViewTree view_tree;
  TileCache tile_cache;
  AssetManager asset_manager;

  bool needs_rebuild = false;

  Pipeline(Widget& init_root_widget, impl::Viewport init_viewport = {},
           RasterContext init_raster_context = RasterContext{})
      : root_widget{&init_root_widget},
        viewport{init_viewport},
        raster_context{init_raster_context},
        layout_tree{},
        view_tree{},
        tile_cache{},
        asset_manager{raster_context} {
    // pass widget to pipeline => bind on children changed
    // pass to layout tree => bind on layout changed
    // pass to view tree => bind on view offset changed
    // pass to tile_cache => bind on raster dirty

    build_references(*root_widget);

    layout_tree.build(*root_widget);
    view_tree.build(layout_tree);
    tile_cache.build(view_tree, raster_context, asset_manager);

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

    viewport.get_on_resize() = [this] {
      Extent const new_viewport_extent = viewport.get_extent();
      Extent const widgets_allocation =
          viewport.get_widgets_allocation().resolve(new_viewport_extent);

      // if the new extent is invisible we will not respond to the request, we
      // can't have a zero-sized surface
      if (new_viewport_extent.visible() &&
          new_viewport_extent != tile_cache.viewport_extent) {
        // responsive viewport-based layout, lol
        // the view is cleaned when the layout becomes dirty (see:
        // `Pipeline::tick` method)
        layout_tree.allot_extent(widgets_allocation);
        tile_cache.resize_viewport(new_viewport_extent);
      }
    };

    viewport.get_on_scroll() = [this] {
      tile_cache.scroll_viewport(viewport.get_offset());
    };

    viewport.get_on_resize()();
    viewport.get_on_scroll()();
  }

  Pipeline(Pipeline&&) = delete;
  Pipeline(Pipeline const&) = delete;
  Pipeline& operator=(Pipeline const&) = delete;
  Pipeline& operator=(Pipeline&&) = delete;

  ~Pipeline() = default;

  void build_references(Widget& widget) {
    // we don't really want to process this imediately as the user could misuse
    // it and dereference data that shouldn't be dereferenced
    //
    // any binded-to structure must not be moved nor its address changed
    WidgetStateProxyAccessor::access(widget).on_children_changed = [this] {
      this->needs_rebuild = true;
    };

    for (Widget* child : widget.get_children()) {
      build_references(*child);
    }
  }

  // root widget must be constant and be re-used?
  void rebuild() {}

  static void recursive_tick(Widget& widget,
                             std::chrono::nanoseconds interval) {
    widget.tick(interval);

    for (Widget* child : widget.get_children()) {
      recursive_tick(*child, interval);
    }
  }

  void tick(std::chrono::nanoseconds interval) {
    // child will be removed as necessary from the tick callback. of course we
    // assume that the children have been deleted by the parent so there's no
    // need to detach the previously attached state proxies. this also means we
    // can't touch the previous children we got. the children widgets can be
    // re-used since we'll rebind the state proxy callbacks. this means that a
    // detached child must not use its stateproxy callbacks.
    // implication: detach children can only be called during ticking.
    recursive_tick(*root_widget, interval);

    // TODO(lamarrr): interactions between each of them
    // if layout changes then we need to rebuild the tile cache with a matching
    // extent if needed

    // what happens if we call update_children outside of tick?
    // no event will be responded to until `tick` so the widget is free to make
    // this change outside of the tick function.

    if (needs_rebuild) {
      // destructors will be called and callbacks detached???? but then the
      // widget would have possibly been removed from the list. what we need to
      // do is to teardown_abort without touching the widgets.
      // we thus won't need to require that the children pointers still be valid
      // after being removed.
      //
      // we should probably detach the callbacks in the
      // mark_children_changed callback itself?. no the span would have gone out
      // of mem
      //
      // needs resizing to match and possibly discarding of recordings, just
      // mark all as dirty and recording will be updated anyway we need to
      // re-use this as it can be expensive
      //
      // for the view tree, after detaching the view, the children can still
      // make callbacks, won't that be fatal? rebuild tile cache too and modify
      // the callbacks as necessary

      // TODO(lamarrr): invalidation interaction between view tree and the tile
      // cache
      // this would mean that we need to re-attach the callbacks after the view
      // offset become dirty.
      rebuild();
    }

    bool const layout_tree_was_dirty = layout_tree.is_layout_dirty;
    layout_tree.tick(interval);
    if (layout_tree_was_dirty) {
      view_tree.force_clean_offsets();
    }

    view_tree.tick(interval);
    tile_cache.tick(interval);
    asset_manager.tick(interval);
    // if layout tree becomes dirty we need to force a total re-draw by marking
    // all of the tiles as dirty view_tree.tick(interval);

    // update bindings since the layout has changed
    // now detach and re-attach to each tree
    // layout cleaning will occur if necessary
  }
};

}  // namespace ui
}  // namespace vlk
