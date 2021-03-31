

struct ViewChild;
struct RasterNode;
struct View;

// we would need to store a pointer to the widget's node in the widget itself
// the widget might need to consult its parent
struct LinkTree {
  struct Node {
    // points to the view it belongs to
    ViewChild* view_node;

    // points to the widget's parent view (i.e. the view it belongs to)
    View* parent_view;

    // points to the widget's position on the raster list
    RasterNode* raster_node;

    // points to the widget's parent
    Node* parent;
  };
};

/// sorted by tree depth (highest to lowest)
/// TODO(lamarrr): test
void submit_children_update_request(UpdateRequest value) {
  size_t partition_start = 0;
  for (; partition_start < children_update_requests.size(); partition_start++) {
    if (children_update_requests[partition_start].tree_depth <=
        value.tree_depth)
      break;
  }

  size_t partition_end = partition_start;

  for (; partition_end < children_update_requests.size(); partition_end++) {
    // if already inserted
    if (value.tree_depth ==
            children_update_requests[partition_end].tree_depth &&
        value.node == children_update_requests[partition_end].node) {
      return;
    }

    // this is the end of the partition
    if (children_update_requests[partition_end].tree_depth < value.tree_depth)
      break;
  }

  children_update_requests.insert(
      children_update_requests.begin() + partition_end, std::move(value));
}

// optimizations: if it is a view type, detach the whole view from the
// view tree
// node.detach_info;
// notify us of children changed or should we make modifications here?
//
// recursively detach child nodes from view tree with a best-case of
// it being a view (thid means we need to pass a boolean paramter to
// signify if it has already been removed) recursively detach child
// noes from the render tree
//
// void * view_tree_binding;
// view tree would have to reference some structs here
//
// on detach, the listeners must be removed
//
// we need to prevent the user callback from calling this multiple
// times what if a child has already been removed? we need to sort
// requests by tree depth too so we first remove the one at the lowest
// depth before proceeding upwards
//
// how do we remove widgets and what data structures are necessary?
// we'll need the view pointer from removing from the view tree, and
// the widget pointer for removing from the raster tree (we could back
// up the z-index to make searching on the raster tree faster)
//
//
//
//
//
// TODO(lamarrr): how do we process in trasversal order? i.e. if the
// parent is detached before the child and a new detach for the child
// comes in, we'd be referencing a dead node. we might need a fast tag
// to recognize the hierarchical relationship.
//
// perform a priority insertion with the lowest tree index being the
// first in the slot
//
//
// recurse into children, remove child from the render tree, and view
// tree and then remove from layout tree
//
// now do this for the lower tree depth widgets
//
//
// test for uniqueness, find request insertion position using
// partition
//
//
//
//
//
// Pop from layout tree, then pop from render tree and mark the area
// they occupy as dirty
//
//
//
//
//
UpdateRequest request{};
request.node = &node;
request.tree_depth = node.tree_depth;
this->submit_children_update_request(request);


  void tick(std::chrono::nanoseconds const& interval) {
    auto const children_update_requests_count = children_update_requests.size();
    layout_tree.is_layout_dirty |= (children_update_requests_count >= 0);
    // view_tree, mark dirty
    for (size_t i = 0; i < tile_cache.tile_is_dirty.size(); i++) {
      tile_cache.tile_is_dirty[i] =
          tile_cache.tile_is_dirty[i] | (children_update_requests_count >= 0);
    }

    for (auto& request : children_update_requests) {
      // recurse into children nodes on the link tree and clean up
      // request.node->layout_node->widget

      // request.node->children.clear();
    }

    // now detach and re-attach to each tree

    // layout cleaning will occur if necessary
    layout_tree.tick(interval);

    view_tree.tick(interval);

    // needs resizing to match and possibly discarding
    tile_cache.tick(interval);
  }


  #pragma once

#include <vector>

#include "vlk/ui/layout_tree.h"
#include "vlk/ui/tile_cache.h"
#include "vlk/ui/view_tree.h"

namespace vlk {
namespace ui {

// forcing parent widgets to allow a child to detach itself can be
// catastrophic. ie. if the parent widget forgets to detach or pass in a
// reference
// TODO(lamarrr): fix this
//
//
// TODO(lamarrr): should we have another struct that links the trees and
// processes removals?
//
// i.e. LinkManager (stores view pointer, layout tree pointer, and render
// tree pointer or index) or Pipeline
//

struct LinkTree {
  struct Node {
    Widget* widget;

    // for loacting its position on the parent layout node
    LayoutTree::Node* layout_node = nullptr;

    // for locating its position on the parent view, if its type is a view then
    // the index will point to a subview
    Widget::Type type;
    ViewTree::View* view = nullptr;
    ViewTree::View *parent_view = nullptr;
    size_t parent_view_index = 0;

    // for locating its position on the tile cache
    size_t tile_cache_index = 0;

    uint64_t tree_depth;

    // its children's info
    std::vector<Node> children;

    void pop_children_layout_tree() {
      layout_node->children.resize(0);


if(type == Widget::Type::View){


view->subviews.clear();
view->entries

}



      for (auto& child : children) {
        if (child.type == Widget::Type::Render) {
          VLK_ENSURE(parent_view_index < parent_view->entries.size());
          // TODO(lamarrr): IMPORTANT: all references to this view tree will now
          // be invalidated, even if we use indexes
          // even if the references are to member functions
          // we'd probably have to do a recursive re-bind.
          parent_view->entries.erase(parent_view->entries.begin() +
                                     parent_view_index);
        } else {
          parent_view->subviews.erase(parent_view->subviews.begin() +
                                      parent_view_index);
        }
      }
    }

    layout_node->children.resize(0);
    layout_node->children.resize(widget->get_children().size());
  };

  void build(Widget&);

  void tick();
};

}  // namespace ui
}  // namespace vlk


