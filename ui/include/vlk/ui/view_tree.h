#pragma once

#include <utility>
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

// translates widget from its normal position on its parent view.
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
    // problem now is that we either have to accumulate all of the offsets
    // when trying to render for each widget or we update the screen offsets
    // for all the child widgets on scroll
    struct Entry {
      LayoutTree::Node const *layout_node;

      IOffset screen_offset;

      // offset on the parent view after translation (i.e. by scrolling)
      IOffset effective_parent_view_offset;

      // can be null
      View const *parent;

      ZIndex z_index;

      void build(LayoutTree::Node const &init_layout_node,
                 View const *view_parent, ZIndex init_z_index) {
        layout_node = &init_layout_node;

        // not yet updated
        effective_parent_view_offset = {};
        screen_offset = {};

        parent = view_parent;

        z_index = init_z_index;
      }
    };

    // non-null
    LayoutTree::Node const *layout_node;

    IOffset screen_offset;

    // offset on the parent view after translation (i.e. by scrolling)
    IOffset effective_parent_view_offset;

    // non-view widgets. not sorted in any particular order
    std::vector<Entry> entries;

    // will make processing clips easier
    View const *parent;

    // view widgets. not sorted in any particular order
    std::vector<View> subviews;

    ZIndex z_index;

    void build(LayoutTree::Node const &node, View const *parent_view,
               ZIndex init_z_index) {
      layout_node = &node;
      parent = parent_view;
      z_index = node.widget->get_z_index().unwrap_or(std::move(init_z_index));

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
      // TODO(lamarrr): we might need to keep track of the translation
      // incase the layout changes
      //
      //
      WidgetStateProxyAccessor::access(*node.widget).on_view_offset_dirty =
          ([this, widget = node.widget, layout_node = &node] {
            this->translate(
                widget->get_view_offset().resolve(layout_node->view_extent));
          });

      // needs to be updated after building the tree, by triggering a
      // on_view_offset dirty at the root.
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
          subviews[subview_index].build(node.children[i], this, z_index + 1);
          subview_index++;
        } else {
          entries[render_index].build(node.children[i], this, z_index + 1);
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
      // now update tile bindings or defer it for
      // the render tree to process and
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
      child.screen_offset =
          parent.screen_offset + child.effective_parent_view_offset;
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
      // adjust the view offset of the parent view.
      // and shifts (translates) the children accordingly. we have to
      // recursively perform passes to update the sceen offsets in the children.

      for (View::Entry &child : entries) {
        // translate the children's effective parent view offset by
        // `translation`.
        view_translate_helper_(child, translation);
        // update the resulting screen offset to match the new view position
        // using the parent view's
        update_screen_offset_helper_(child, *this);
      }

      // NOTE that this only requires that we update this view's child views
      // offset and no need for translating them
      for (View &subview : subviews) {
        view_translate_helper_(subview, translation);
        update_screen_offset(subview, *this);
      }
    }

    void clean_offsets() {
      layout_node->widget->mark_view_offset_dirty();
      for (auto &subview : subviews) {
        subview.clean_offsets();
      }
    }
  };

  View root_view;

  void clean() { root_view.clean_offsets(); }

  void build(LayoutTree const &layout_tree) {
    VLK_ENSURE(root_view.layout_node == nullptr);
    VLK_ENSURE(root_view.subviews.empty());
    VLK_ENSURE(root_view.entries.empty());

    VLK_ENSURE(layout_tree.root_node.type == Widget::Type::View);
    VLK_ENSURE(layout_tree.root_node.widget != nullptr);

    root_view.build(layout_tree.root_node, nullptr, 0);
    clean();
  }

  void tick(std::chrono::nanoseconds const &) {}
};

}  // namespace ui
}  // namespace vlk
