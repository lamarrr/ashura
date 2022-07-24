#pragma once
#include <utility>

#include "vlk/subsystem/context.h"
#include "vlk/subsystems/asset_loader.h"
#include "vlk/subsystems/keyboard.h"
#include "vlk/subsystems/scheduler.h"
#include "vlk/ui/event.h"
#include "vlk/ui/layout.h"
#include "vlk/ui/layout_tree.h"
#include "vlk/ui/tile_cache.h"
#include "vlk/ui/view_tree.h"
#include "vlk/ui/viewport.h"
#include "vlk/ui/widget.h"

namespace vlk {
namespace ui {

// TODO(lamarrr): asset manager should be stored in the subsystems map?
// `render_context` and `root_widget` must outlive the pipeline.
// `tick()` must not be called with either of them deleted.
struct Pipeline {
  Widget* root_widget;
  Viewport viewport;
  RenderContext const* render_context;

  LayoutTree layout_tree;
  ViewTree view_tree;
  TileCache tile_cache;
  bool needs_rebuild = true;

  SubsystemsContext context{};

  Pipeline(Widget& init_root_widget, RenderContext const& init_render_context)
      : root_widget{&init_root_widget}, render_context{&init_render_context} {
    // pass widget to pipeline => bind on children changed
    // pass to layout tree => bind on layout changed
    // pass to view tree => bind on view offset changed
    // pass to tile_cache => bind on raster dirty

    attach_state_proxies(*root_widget);

    context
        .__register_subsystem(
            "VLK_AssetLoader",
            stx::rc::make_inplace<AssetLoader>(stx::os_allocator).unwrap())
        .unwrap();

    context
        .__register_subsystem(
            "VLK_TaskScheduler",
            stx::rc::make_inplace<TaskScheduler>(
                stx::os_allocator, std::chrono::steady_clock::now(),
                stx::os_allocator)
                .unwrap())
        .unwrap();

    context
        .__register_subsystem("VLK_Keyboard",
                              stx::rc::make_inplace<Keyboard>(stx::os_allocator,
                                                              stx::os_allocator)
                                  .unwrap())
        .unwrap();

    context.__link();

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
  }

  STX_DEFAULT_CONSTRUCTOR(Pipeline)
  STX_MAKE_PINNED(Pipeline)

  void dispatch_events(stx::Span<MouseButtonEvent const> mouse_button_events,
                       stx::Span<WindowEvent const> window_events) {
    // TODO(lamarrr): forward events
    for (auto& event : mouse_button_events) {
      VLK_LOG("Mouse {}, clicks: {}",
              event.action == MouseAction::Press ? "Press" : "Release",
              event.clicks);
    }
  }

  void attach_state_proxies(Widget& widget) {
    WidgetSystemProxy::get_state_proxy(widget).on_children_changed =
        stx::fn::rc::make_functor(stx::os_allocator, [this] {
          this->needs_rebuild = true;
        }).unwrap();

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

    WidgetSystemProxy::tick(widget, interval, context);
    WidgetSystemProxy::mark_stale(widget);

    for (Widget* child : widget.get_children()) {
      // note that we only touch the latest updated pointers to the children,
      // and if the widget updated its children, we'll rebuild the trees
      recursive_tick(*child, interval);
    }
  }

  // TODO(lamarrrr): should be ticked with subsytem map
  BackingStoreDiff tick(std::chrono::nanoseconds interval) {
    recursive_tick(*root_widget, interval);

    // dpr

    if (needs_rebuild) {
      // note that each build method is optimized for rebuilding and should not
      // re-allocate too much of memory in the case where the tree sizes don't
      // really change and we thus have a `build()` method separated from the
      // constructor that uses `.resize` and `.clear` for their vectors to
      // prevent forcing a memory re-allocation when the available space is
      // enough
      layout_tree.build(*root_widget);
      view_tree.build(layout_tree.root_node);
      tile_cache.build(view_tree.root_view, *render_context);
      needs_rebuild = false;
    }

    {
      if (viewport.is_scrolled()) {
        tile_cache.scroll_backing_store_logical(viewport.get_offset());
      }

      if (viewport.is_resized()) {
        Extent new_viewport_extent = viewport.get_extent();
        tile_cache.resize_backing_store_logical(new_viewport_extent);
      }

      if (viewport.is_widgets_allocation_changed()) {
        Extent new_widgets_allocation = viewport.get_widgets_allocation();
        layout_tree.allot_extent(new_widgets_allocation);
      }

      ViewportSystemProxy::mark_clean(viewport);
    }

    bool const layout_tree_was_cleaned = layout_tree.is_layout_dirty;
    layout_tree.tick(interval);

    // if layout tree becomes dirty we need to force a total re-draw by marking
    // all of the tiles as dirty
    if (layout_tree_was_cleaned) {
      view_tree.mark_views_dirty();
      // resize tiles to layout extent
      tile_cache.mark_tiles_extent_dirty();
    }

    view_tree.tick(interval);

    BackingStoreDiff backing_store_diff = tile_cache.tick(interval);

    context.__tick(interval);

    return backing_store_diff;
  }
};

}  // namespace ui
}  // namespace vlk
