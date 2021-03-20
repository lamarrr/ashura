#pragma once

#include "vlk/ui/primitives.h"
#include "vlk/ui/widget.h"

#include <vector>

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

// how do we make view widgets and normal widgets get their z-indexes
// respected?, now we don't need to reset the z-indexes of view widgets we can
// now have absolute-positioned widgets. we don't need another layout tree, this
// will effectively become our layout tree since layout and positioning is
// highly dependent on views.

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
      Widget *widget_;

      // problem now is that we either have to accumulate all of the offsets
      // when trying to render for each widget or we update the screen offsets
      // for all the child widgets on scroll
      IOffset screen_offset_;

      // this never changes until a re-layout occurs
      Rect parent_view_area_;

      // offset on the parent view after translation (i.e. by scrolling)
      IOffset effective_parent_view_offset_;

      View const *parent_;
    };

    // the widget associated with this view
    Widget *widget_;

    IOffset screen_offset_;

    // which part of the parent view it occupies
    Rect parent_view_area_;

    // offset on the parent view after translation (i.e. by scrolling)
    IOffset effective_parent_view_offset_;

    // represents the overall extent of the view widget (including the
    // non-visible or internal area)
    Extent view_extent_;

    // raster widgets. not sorted in any particular order
    std::vector<Entry> entries_;

    // will make processing clips easier
    View const *parent_;

    // TODO(lamarrr): consider changing to list for easier insertion and
    // deletion

    // view widgets. not sorted in any particular order
    std::vector<View> subviews_;

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
          IOffset{static_cast<int64_t>(entry.parent_view_area_.offset.x) +
                      translation.x,
                  static_cast<int64_t>(entry.parent_view_area_.offset.y) +
                      translation.y};

      entry.effective_parent_view_offset_.x = ioffset.x;
      entry.effective_parent_view_offset_.y = ioffset.y;
    }

    template <typename T>
    static void update_screen_offset_helper_(T &child, View &parent) {
      child.screen_offset_ =
          parent.screen_offset_ + child.effective_parent_view_offset_;
      // now update tile bindings or defer it for the render tree to process and
      // re-attach as necessary. the render tree will be aware of the dirtiness
      // since the parent view raster area would have been marked as dirty
      // TODO(lamarrr): we need to also update the tile binding whilst marking
      // the previous as invalid by calling its attached callback i think the
      // cache should also reserve a vector of bool to know which offset changed
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

      for (View::Entry &entry : child.entries_) {
        update_screen_offset_helper_(entry, child);
      }

      for (View &subview : child.subviews_) {
        update_screen_offset(subview, child);
      }
    }

    static void translate_view(View &view, IOffset const &translation) {
      for (View::Entry &child : view.entries_) {
        View::translate_view_helper_(child, translation);
        update_screen_offset_helper_(child, view);
      }

      for (View &subview : view.subviews_) {
        // this is bad, it should only adjust the positioning of the parent view
        // and just shift the children accordingly, not make them be as if the
        // parent view shifted their child views we have to perform passes to
        // correct this in the children recurse and update screen_offsets
        // subview.parent_view_area_.extent;
        // subview.effective_parent_view_offset_;
        View::translate_view_helper_(subview, translation);
        update_screen_offset(subview, view);
      }
    }

    void bind() {
      WidgetStateProxyAccessor::access(*widget_).on_view_offset_dirty =
          ([this](ViewOffset const &translation) {
            translate_view(*this, translation.resolve(this->view_extent_));
          });
    }
  };
};

}  // namespace ui
}  // namespace vlk
