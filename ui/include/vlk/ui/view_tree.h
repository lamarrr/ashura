#pragma once

#include <vector>

#include "vlk/ui/primitives.h"
#include "vlk/ui/widget.h"

namespace vlk {
namespace ui {

// how do we implement stacking without overriding a very large area?

// layout recalculation, how do we process it?
// do we consult the link tree? we can have high gains by making adding or
// removing children very slow.
// the link tree will help with this.

// memory re-allocation only occurs when the layout changes

// alternatively: we can have individual widgets here and have a final render
// tree that decides what each individual widget is on the screen. rendertile
// should have all the widgets on the screen with their clip coordinates, this
// should be updated from the view tree which applies a visibility and
// rectangular clip on it. this will enable the rastertile to only be updated on
// a per-widget basis and not depend on the view itself the view tree would just
// get the tile at the index and mark it as dirty.
//
// pay special consideration to:
// re-layout effects
// children-effects
// raster dirtiness effects
//

//  now we don't need to reset the z-indexes of view widgets we can
// now have absolute-positioned widgets.

struct ViewTree {
  // IMPORTANT: how does this affect child views?
  // this also means when a scroll happens in a view, it only needs to update
  // one variable and the others will be in sync with it (pointer), rather than
  // updating for all the widgets. but then, getting the absolute screen offset
  // becomes very difficult. we can have another tree for keeping track of the
  // view calculations and keep calculations to a minimum and not have to
  // trasverse through all of the widgets to update the screen offset. the
  // tilecache will thus reference this ViewPositioning tree. the individual
  // view widgets will now update them when their view offsets are dirty.
  //
  struct View {
    // all entries are positioned relative to the view
    // how do we position views relative to views? whilst maintaining their
    // translations
    struct Entry {
      Widget *widget;

      // problem now is that we either have to accumulate all of the offsets
      // when trying to render for each widget or we update the screen offsets
      // for all the child widgets on scroll
      IOffset screen_offset;

      // this never changes until a re-layout occurs
      Rect const *parent_view_area;

      // offset on the parent view after translation (i.e. by scrolling)
      IOffset effective_parent_view_offset;

      View const *parent;
    };

    // the widget associated with this view
    Widget *widget;

    IOffset screen_offset;

    // which part of the parent view it occupies
    Rect const *parent_view_area;

    // offset on the parent view after translation (i.e. by scrolling)
    IOffset effective_parent_view_offset;

    // represents the overall extent of the view widget (including the
    // non-visible or internal area)
    Extent const *view_extent;

    // raster widgets. not sorted in any particular order
    std::vector<Entry> entries;

    // will make processing clips easier
    View const *parent;

    // view widgets. not sorted in any particular order
    std::vector<View> subviews;

    // if we are re-drawing for a tile for example, we can check if it
    // intersects with the tile and only redraw for the ones that intersect with
    // the tile
    // clips don't cross views
    // consider making the parent inject the effects and add them to an effect
    // tree, with all of the widgets having individual effects as a result we
    // need to be able to render the effects independent of the widget, we'll
    // thus need bindings for them

    // translates widget from the its normal position on its parent view.
    // TODO(lamarrr): this will also affect the dirty area updating
    template <typename T>
    static constexpr void translate_view_helper_(
        T &entry, IOffset const &translation = IOffset{0, 0}) {
      IOffset ioffset =
          IOffset{static_cast<int64_t>(entry.parent_view_area->offset.x) +
                      translation.x,
                  static_cast<int64_t>(entry.parent_view_area->offset.y) +
                      translation.y};

      entry.effective_parent_view_offset.x = ioffset.x;
      entry.effective_parent_view_offset.y = ioffset.y;
    }

    template <typename T>
    static void update_screen_offset_helper_(T &child, View &parent) {
      child.screen_offset =
          parent.screen_offset + child.effective_parent_view_offset;
      // now update tile bindings or defer it for the render tree to process and
      // re-attach as necessary. the render tree will be aware of the dirtiness
      // since the parent view raster area would have been marked as dirty
      // TODO(lamarrr): we need to also update the tile binding whilst marking
      // the previous as invalid by calling its attached callback i think the
      // cache should also reserve a vector of bool to know which offset changed
      //
      // we can have a function proxy to rebind the widget to another tile.
      // since the parent view will mark the whole region as dirty, we don't
      // need to worry too much about passing the dirtiness information to the
      // tile cache
      //
      // TODO(lamarrr): synchronising the offset and render dirtiness of the
      // view widgets
      //
      // on view offset dirty must call on raster dirty and it must be safe and
      // okay to call on raster dirty multiple times
      //
      // I think we don't need this since the view will mark its area as
      // invalidated anyway
    }

    static void update_screen_offset(View &child, View &parent) {
      update_screen_offset_helper_(child, parent);

      for (View::Entry &entry : child.entries) {
        update_screen_offset_helper_(entry, child);
      }

      for (View &subview : child.subviews) {
        update_screen_offset(subview, child);
      }
    }

    static void translate_view(View &view, IOffset const &translation) {
      // adjusts the view offset of the parent view.
      // and shifts (translates) the children accordingly. we have to
      // recursively perform passes to update the sceen offsets in the children.

      for (View::Entry &child : view.entries) {
        View::translate_view_helper_(child, translation);
        update_screen_offset_helper_(child, view);
      }

      for (View &subview : view.subviews) {
        View::translate_view_helper_(subview, translation);
        update_screen_offset(subview, view);
      }
    }

    void bind() {
      WidgetStateProxyAccessor::access(*widget).on_view_offset_dirty =
          ([this](ViewOffset const &translation) {
            translate_view(*this, translation.resolve(*this->view_extent));
          });
    }
  };
};

}  // namespace ui
}  // namespace vlk
