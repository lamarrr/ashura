#pragma once

#include "vlk/ui/layout_tree.h"
#include "vlk/ui/tile_cache.h"
#include "vlk/ui/view_tree.h"
#include "vlk/ui/widget.h"

namespace vlk {
namespace ui {

void recursive_tick(Widget& widget, std::chrono::nanoseconds const& interval) {
  widget.tick(interval);

  for (Widget* child : widget.get_children()) {
    recursive_tick(*child, interval);
  }
}

struct Pipeline {
  RasterContext* context = nullptr;
  Widget* root_widget = nullptr;

  LayoutTree layout_tree{};
  ViewTree view_tree{};
  TileCache tile_cache{};

  bool needs_rebuild = false;

  void bind(Widget& widget) {
    // we don't really want to process this imeediately as the user could misuse
    // it and dereference data that shouldn't be dereferenced, we need a vector
    // to store the dirty children info, instead of modifying it here any
    // binded-to structure must not be moved nor its address changed
    WidgetStateProxyAccessor::access(widget).on_children_changed = [this] {
      this->needs_rebuild = true;
    };

    for (Widget* child : widget.get_children()) {
      bind(*child);
    }
  }

  // TODO(lamarrr): we need to keep track of the present_view_offset so we can
  // get that for the view while re-initializing the view tree and maintain the
  // offset.

  void build(Widget& new_root_widget, RasterContext& raster_context) {
    // pass widget to pipeline => bind on children changed
    // pass link tree to layout tree => bind on layout changed
    // pass link tree to view tree => bind on view offset changed
    // pass link tree to tile_cache => bind on raster dirty

    root_widget = &new_root_widget;

    layout_tree = {};
    view_tree = {};

    bind(*root_widget);
    layout_tree.build(*root_widget);
    view_tree.build(layout_tree);
    tile_cache.build(view_tree, raster_context);
  }

  // root widget must be constant and be re-used.
  void rebuild() {}

  void tick(std::chrono::nanoseconds const& interval) {
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

      rebuild();

      // needs resizing to match and possibly discarding of recordings, just
      // mark all as dirty and recording will be updated anyway we need to
      // re-use this as it can be expensive
      for (size_t i = 0; i < tile_cache.tile_is_dirty.size(); i++) {
        tile_cache.tile_is_dirty[i] = true;
      }

      // for the view tree, after detaching the view, the children can still
      // make callbacks, won't that be fatal? rebuild tile cache too and modify
      // the callbacks as necessary

      // tile_cache.resize(...);
      layout_tree.tick(interval);

      // TODO(lamarrr): invalidation interaction between view tree and the tile
      // cache

      view_tree.tick(interval);
      tile_cache.tick(interval);
    } else {
      if (layout_tree.is_layout_dirty) {
        layout_tree.tick(interval);
        view_tree.clean();
        view_tree.tick(interval);

        // update bindings since the layout has changed
        // tile_cache.rebind();
      }

      tile_cache.tick(interval);
    }

    // now detach and re-attach to each tree

    // layout cleaning will occur if necessary
  }
};

}  // namespace ui
}  // namespace vlk
