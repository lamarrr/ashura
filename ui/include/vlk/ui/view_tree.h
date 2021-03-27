#pragma once

#include <vector>

#include "vlk/ui/layout_tree.h"
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

// translates widget from the its normal position on its parent view.
// TODO(lamarrr): this will also affect the dirty area updating
template <typename T>
constexpr void view_translate_helper_(T &entry, IOffset const &translation) {
  IOffset ioffset =
      IOffset{static_cast<int64_t>(entry.layout_node->parent_view_offset.x) +
                  translation.x,
              static_cast<int64_t>(entry.layout_node->parent_view_offset.y) +
                  translation.y};

  entry.effective_parent_view_offset.x = ioffset.x;
  entry.effective_parent_view_offset.y = ioffset.y;
}

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
      // Widget *widget;
      LayoutTree::Node const *layout_node;

      // problem now is that we either have to accumulate all of the offsets
      // when trying to render for each widget or we update the screen offsets
      // for all the child widgets on scroll
      IOffset screen_offset;

      // this never changes until a re-layout occurs
      // TODO(lamarrr): this is entirely dependent on the layout tree's
      // statefulness? or should we perform an update-with recursion where they
      // are tied?
      // Offset parent_view_offset;

      // offset on the parent view after translation (i.e. by scrolling)
      IOffset effective_parent_view_offset;

      // can be null
      View const *parent;

      void build(LayoutTree::Node &node, View const *view_parent) {
        parent = view_parent;
        // parent_view_area = &node.parent_view_area;
        // widget = node.widget;
        layout_node = &node;

        effective_parent_view_offset = {};
        screen_offset = {};
      }
    };

    // which part of the parent view it occupies
    // TODO(lamarrr): will this be hazardrous if null?
    // Rect const *parent_view_area;
    // represents the overall extent of the view widget (including the
    // non-visible or internal area)
    // Extent const *view_extent;
    LayoutTree::Node const *layout_node;

    IOffset screen_offset;

    // this never changes until a re-layout occurs
    // Offset parent_view_offset;

    // offset on the parent view after translation (i.e. by scrolling)
    IOffset effective_parent_view_offset;

    // raster widgets. not sorted in any particular order
    std::vector<Entry> entries;

    // will make processing clips easier
    View const *parent;

    // view widgets. not sorted in any particular order
    std::vector<View> subviews;

    void build(LayoutTree::Node &node, View const *parent_view) {
      parent = parent_view;
      // parent_view_area = &node.parent_view_area;
      // view_extent = &node.view_extent;
      // widget = node.widget;
      layout_node = &node;

      // it is safe for the widget to access this multiple times even though the
      // user could pay a possibly minor perf penalty. this saves us the stress
      // of accumulating scroll offsets into different vectors.
      //
      // TODO(lamarrr): what if the user already requested a new translation and
      // we need to process the new one the sequence of the requests is
      // important
      //
      // this is primarily risky because we can't guarantee that
      // `translate_view` will not change and then begin touching the widgets
      // itself
      //
      //
      WidgetStateProxyAccessor::access(*node.widget).on_view_offset_dirty =
          ([this](ViewOffset const &translation) {
            // TODO(lamarrr): we might need to keep track of the translation
            // incase the layout changes
            this->translate(
                translation.resolve(this->layout_node->view_extent));
          });

      // needs to be updated after first build and on any insertion or removal
      // from the tree, by triggering a IOffset{0, 0} translation at the root
      effective_parent_view_offset = {};
      screen_offset = {};

      auto const num_children = node.children.size();
      auto const num_view_children =
          std::count_if(node.children.begin(), node.children.end(),
                        [](LayoutTree::Node const &child) {
                          return child.type == Widget::Type::View;
                        });
      auto const num_render_children = node.children.size() - num_view_children;

      subviews.resize(num_view_children);
      entries.resize(num_render_children);

      size_t subview_index = 0;
      size_t render_index = 0;

      for (size_t i = 0; i < num_children; i++) {
        if (node.type == Widget::Type::View) {
          subviews[subview_index].build(node.children[i], this);
          subview_index++;
        } else {
          entries[render_index].build(node.children[i], this);
          render_index++;
        }
      }
    }

    // if we are re-drawing for a tile for example, we can check if it
    // intersects with the tile and only redraw for the ones that intersect with
    // the tile
    // clips don't cross views
    // consider making the parent inject the effects and add them to an effect
    // tree, with all of the widgets having individual effects as a result we
    // need to be able to render the effects independent of the widget, we'll
    // thus need bindings for them

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

    void translate(IOffset const &translation) {
      // adjusts the view offset of the parent view.
      // and shifts (translates) the children accordingly. we have to
      // recursively perform passes to update the sceen offsets in the children.

      for (View::Entry &child : entries) {
        view_translate_helper_(child, translation);
        update_screen_offset_helper_(child, *this);
      }

      for (View &subview : subviews) {
        view_translate_helper_(subview, translation);
        update_screen_offset(subview, *this);
      }
    }
  };

  View root_view;

  void build(LayoutTree &tree) {
    /// FIXME:
    VLK_ENSURE(root_view.layout_node == nullptr);
    VLK_ENSURE(root_view.subviews.empty());
    VLK_ENSURE(root_view.entries.empty());
    VLK_ENSURE(tree.root_node.type == Widget::Type::View);

    root_view.build(tree.root_node, nullptr);
  }

  void tick(std::chrono::nanoseconds const &interval) {}
};

}  // namespace ui
}  // namespace vlk
