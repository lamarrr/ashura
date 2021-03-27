
#pragma once

#include <algorithm>
#include <chrono>
#include <utility>
#include <vector>

#include "stx/span.h"

#include "vlk/ui/impl/widget_state_proxy_accessor.h"
#include "vlk/ui/layout.h"
#include "vlk/ui/primitives.h"
#include "vlk/ui/widget.h"

namespace vlk {
namespace ui {

// on layout dirty, this must update the view tree
// whilst these trees use a std::vector, we try to ensure that the addresses
// (even the ones in the callbacks) are valid and updated as necessary
//
//
// this tree is very hazardrous and fragile to memory addresses, be sure to know
// what you're doing, especially whilst binding references
//
//
// the layout tree also connects to other widgets and is like the proxy to
// detach or add widgets to them.
//
//

constexpr Extent apply_flex_fit(Flex::Direction direction, Flex::Fit main_fit,
                                Flex::Fit cross_fit, Extent const &span,
                                Extent const &allotted_extent) {
  Extent result = {};

  if (main_fit == Flex::Fit::Shrink) {
    if (direction == Flex::Direction::Row) {
      result.width = std::min(span.width, allotted_extent.width);
    } else {
      result.height = std::min(span.height, allotted_extent.height);
    }
  } else {
    // expand
    if (direction == Flex::Direction::Row) {
      result.width = allotted_extent.width;
    } else {
      result.height = allotted_extent.height;
    }
  }

  if (cross_fit == Flex::Fit::Shrink) {
    if (direction == Flex::Direction::Row) {
      result.height = std::min(span.height, allotted_extent.height);
    } else {
      result.width = std::min(span.width, allotted_extent.width);
    }
  } else {
    // expand
    if (direction == Flex::Direction::Row) {
      result.height = allotted_extent.height;
    } else {
      result.width = allotted_extent.width;
    }
  }

  return result;
}

struct LayoutTree {
  struct Node {
    /// target widget
    Widget *widget;

    /// target widget type
    Widget::Type type;

    /// part of the parent view this widget occupies
    Extent self_extent;

    /// part of the parent widget this widget occupies
    Offset parent_offset;

    /// initial parent view offset for this widget
    Offset parent_view_offset;

    /// for view widgets
    Extent view_extent;

    /// lets us know how deep into the heirarchy tree we are
    uint64_t tree_depth;

    /// the child nodes (corresponds to child widgets)
    std::vector<Node> children;

    bool has_children() const { return !children.empty(); }

    // forcing parent widgets to allow a child to detach itself can be
    // catastrophic. ie. if the parent widget forgets to detach or pass in a
    // reference
    // TODO(lamarrr): fix this
    struct LinkInfo {
      struct View {
        uintptr_t parent_view;
        size_t parent_view_index;
      } view_loc;

      size_t render_tree_loc;
    } link_info;
  };

  Node root_node;

  struct ChildrenUpdateRequest {
    uint64_t tree_depth;
    Node *node;
  };

  std::vector<ChildrenUpdateRequest> children_update_requests;

  // for now, we just re-perform layout when any of the widgets is dirty
  bool is_layout_dirty;

  /// sorted by tree depth (highest to lowest)
  /// TODO(lamarrr): test
  void submit_children_update_request(ChildrenUpdateRequest &&value) {
    size_t partition_start = 0;
    for (; partition_start < children_update_requests.size();
         partition_start++) {
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

  // if we resize will the view be able to keep track of its translation?
  static void perform_layout(LayoutTree::Node &node,
                             Extent const &allotted_extent,
                             Offset const &parent_view_offset) {
    Widget const &widget = *node.widget;

    SelfExtent const self_extent = widget.get_self_extent();
    Extent const resolved_self_extent = self_extent.resolve(allotted_extent);

    // TODO(lamarrr): what if this is a flex that doesn't have children?
    // or view without children?
    if (node.has_children()) {
      Flex const flex = widget.get_flex();

      ViewExtent const view_extent = widget.get_view_extent();

      // TODO(lamarrr): being infinite in size is ok???? it won't be drawn
      // anyway????
      Extent const resolved_view_extent = view_extent.resolve(allotted_extent);

      Extent const flex_span = perform_flex_children_layout(
          flex,
          node.type == Widget::Type::View ? resolved_view_extent
                                          : resolved_self_extent,
          node.children);

      // layout of children along parent is now done, we can now layout along
      // the view
      for (LayoutTree::Node &child : node.children) {
        child.parent_view_offset =
            node.type == Widget::Type::View
                ? child.parent_offset
                : (parent_view_offset + child.parent_offset);
      }

      if (node.type == Widget::Type::View) {
        node.self_extent = resolved_self_extent;
        // unconstrained
        node.view_extent =
            apply_flex_fit(flex.direction, flex.main_fit, flex.cross_fit,
                           flex_span, Extent{vlk::u32_max, vlk::u32_max});
      } else {
        node.self_extent =
            apply_flex_fit(flex.direction, flex.main_fit, flex.cross_fit,
                           flex_span, allotted_extent);
        // really shouldn't be used for non-view widgets, but set for
        // correctness
        node.view_extent = node.self_extent;
      }
    } else {
      node.self_extent = resolved_self_extent;
      node.view_extent = node.self_extent;
    }
  }

  template <Flex::Direction direction>
  static Extent perform_flex_children_layout_(
      Flex const &flex, Extent const &self_extent,

      stx::Span<LayoutTree::Node> const &children) {
    for (LayoutTree::Node &child : children) {
      // TODO(lamarrr): backing up scroll offsets
      //
      //
      // the width allotted to these child widgets **must** be
      // constrained, this especially due to the view widgets that may have a
      // u32_max extent. overflow shouldn't occur since the child widget's
      // extent is resolved using the parent's
      perform_layout(child, self_extent, Offset{0, 0});

      VLK_ENSURE(child.self_extent.width != u32_max &&
                     child.self_extent.height != u32_max,
                 "", *child.widget);
    }

    Flex::CrossAlign const cross_align = flex.cross_align;
    Flex::MainAlign const main_align = flex.main_align;
    Flex::Wrap const wrap = flex.wrap;

    auto present_block_start = children.begin();
    auto child_it = children.begin();

    uint32_t block_max_width = 0;
    uint32_t block_max_height = 0;

    Offset present_offset{0, 0};

    // we'll have scenarios where the extent passed down to the flex (parent)
    // will be infinite, the flex isn't allowed to use the infinite extent,
    // we'll need to shrink it to the effective size of its children [flex
    // span], by shrinking the flex's extent.
    //
    // some alignments like center, evenly, end, space around, and space between
    // use the whole space and some don't.
    Extent flex_span{0, 0};

    // alignments not positioned to start always utilize the full allotted
    // extent
    if (cross_align != Flex::CrossAlign::Start) {
      if constexpr (direction == Flex::Direction::Row) {
        flex_span.height = self_extent.height;
      } else {
        flex_span.width = self_extent.width;
      }
    }

    if (main_align != Flex::MainAlign::Start) {
      if constexpr (direction == Flex::Direction::Row) {
        flex_span.width = self_extent.width;
      } else {
        flex_span.height = self_extent.height;
      }
    }

    while (child_it < children.end()) {
      child_it->parent_offset.x = present_offset.x;
      child_it->parent_offset.y = present_offset.y;

      block_max_width = std::max(block_max_width, child_it->self_extent.width);
      block_max_height =
          std::max(block_max_height, child_it->self_extent.height);

      auto next_child_it = child_it + 1;

      // next widget is at the end of the block or at the end of the children
      // list
      if ((next_child_it < children.end() &&
           ((direction == Flex::Direction::Row &&
             (child_it->parent_offset.x + child_it->self_extent.width +
              next_child_it->self_extent.width) > self_extent.width) ||
            (direction == Flex::Direction::Column &&
             (child_it->parent_offset.y + child_it->self_extent.height +
              next_child_it->self_extent.height) > self_extent.height))) ||
          next_child_it == children.end()) {
        // each block will have at least one widget
        for (auto &child :
             stx::Span<LayoutTree::Node>(present_block_start, next_child_it)) {
          // cross-axis alignment
          uint32_t cross_space = 0;

          if constexpr (direction == Flex::Direction::Row) {
            cross_space = block_max_height - child.self_extent.height;
          } else {
            cross_space = block_max_width - child.self_extent.width;
          }

          // determine cross-axis span
          if (cross_align == Flex::CrossAlign::Start) {
            if constexpr (direction == Flex::Direction::Row) {
              flex_span.height =
                  std::max(flex_span.height, self_extent.height - cross_space);
            } else {
              flex_span.width =
                  std::max(flex_span.width, self_extent.width - cross_space);
            }
          }

          // determine main-axis span
          if (main_align == Flex::MainAlign::Start) {
            if constexpr (direction == Flex::Direction::Row) {
              flex_span.width =
                  std::max(flex_span.width, self_extent.width - cross_space);
            } else {
              flex_span.height =
                  std::max(flex_span.height, self_extent.height - cross_space);
            }
          }

          if (cross_align == Flex::CrossAlign::Center) {
            uint32_t cross_space_center = cross_space / 2;
            if constexpr (direction == Flex::Direction::Row) {
              child.parent_offset.y += cross_space_center;
            } else {
              child.parent_offset.x += cross_space_center;
            }
          } else if (cross_align == Flex::CrossAlign::End) {
            uint32_t cross_space_end = cross_space;
            if constexpr (direction == Flex::Direction::Row) {
              child.parent_offset.y += cross_space_end;
            } else {
              child.parent_offset.x += cross_space_end;
            }
          } else if (cross_align == Flex::CrossAlign::Stretch) {
            if constexpr (direction == Flex::Direction::Row) {
              // re-layout the child to the max block height
              if (child.self_extent.height != block_max_height) {
                perform_layout(*child_it,
                               Extent{self_extent.width, block_max_height},
                               Offset{0, 0});
                VLK_ENSURE(child.self_extent.width != u32_max &&
                               child.self_extent.height != u32_max,
                           "", *child.widget);
              }
            } else {
              // re-layout the child to the max block width
              if (child.self_extent.width != block_max_width) {
                perform_layout(*child_it,
                               Extent{block_max_width, self_extent.height},
                               Offset{0, 0});
                VLK_ENSURE(child.self_extent.width != u32_max &&
                               child.self_extent.height != u32_max,
                           "", *child.widget);
              }
            }
          } else if (cross_align == Flex::CrossAlign::Start || true) {
            // already done
          }
        }

        uint32_t main_space = 0;

        if constexpr (direction == Flex::Direction::Row) {
          main_space = self_extent.width - (child_it->parent_offset.x +
                                            child_it->self_extent.width);
        } else {
          main_space = self_extent.height - (child_it->parent_offset.y +
                                             child_it->self_extent.height);
        }

        uint32_t num_block_children = next_child_it - present_block_start;

        if (main_align == Flex::MainAlign::End) {
          uint32_t main_space_end = main_space;
          for (auto &child : stx::Span<LayoutTree::Node>(present_block_start,
                                                         next_child_it)) {
            if constexpr (direction == Flex::Direction::Row) {
              child.parent_offset.x += main_space_end;
            } else {
              child.parent_offset.y += main_space_end;
            }
          }
        } else if (main_align == Flex::MainAlign::SpaceAround) {
          uint32_t main_space_around = main_space / num_block_children;
          main_space_around /= 2;
          uint32_t new_offset = 0;

          for (auto &child : stx::Span<LayoutTree::Node>(present_block_start,
                                                         next_child_it)) {
            new_offset += main_space_around;
            if constexpr (direction == Flex::Direction::Row) {
              child.parent_offset.x = new_offset;
              new_offset += child.self_extent.width + main_space_around;
            } else {
              child.parent_offset.y = new_offset;
              new_offset += child.self_extent.height + main_space_around;
            }
          }
        } else if (main_align == Flex::MainAlign::SpaceBetween) {
          uint32_t new_offset = 0;

          if constexpr (direction == Flex::Direction::Row) {
            new_offset += present_block_start->self_extent.width;
          } else {
            new_offset += present_block_start->self_extent.height;
          }

          // there's always atleast one element in a block
          for (auto &child : stx::Span<LayoutTree::Node>(
                   present_block_start + 1, next_child_it)) {
            // this expression is in the block scope due to possible
            // division-by-zero if it only has one element, this loop will only
            // be entered if it has at-least 2 children
            uint32_t main_space_between = main_space / (num_block_children - 1);
            new_offset += main_space_between;

            if constexpr (direction == Flex::Direction::Row) {
              child.parent_offset.x = new_offset;
              new_offset += child.self_extent.width;
            } else {
              child.parent_offset.y = new_offset;
              new_offset += child.self_extent.height;
            }
          }

        } else if (main_align == Flex::MainAlign::SpaceEvenly) {
          uint32_t main_space_evenly = main_space / (num_block_children + 1);
          uint32_t new_offset = main_space_evenly;
          for (auto &child :
               stx::Span<LayoutTree::Node>(present_block_start, child_it)) {
            if constexpr (direction == Flex::Direction::Row) {
              child.parent_offset.x = new_offset;
              new_offset += child.self_extent.width + main_space_evenly;
            } else {
              child.parent_offset.y = new_offset;
              new_offset += child.self_extent.height + main_space_evenly;
            }
          }

          if constexpr (direction == Flex::Direction::Row) {
            child_it->parent_offset.x = new_offset;
          } else {
            child_it->parent_offset.y = new_offset;
          }

        } else if (main_align == Flex::MainAlign::Start || true) {
          // already done
        }

        if (wrap == Flex::Wrap::None) {
          if constexpr (direction == Flex::Direction::Row) {
            present_offset.x += child_it->self_extent.width;
          } else {
            present_offset.y += child_it->self_extent.height;
          }
        } else {
          // move to the next row/column
          if constexpr (direction == Flex::Direction::Row) {
            present_offset.x = 0;
            present_offset.y += block_max_height;
          } else {
            present_offset.y = 0;
            present_offset.x += block_max_width;
          }

          present_block_start = child_it + 1;
        }

      } else {
        if constexpr (direction == Flex::Direction::Row) {
          present_offset.x += child_it->self_extent.width;
        } else {
          present_offset.y += child_it->self_extent.height;
        }
      }

      child_it++;
    }

    return flex_span;
  }

  static Extent perform_flex_children_layout(
      Flex const &flex, Extent const &self_extent,
      stx::Span<LayoutTree::Node> const &child_nodes) {
    if (flex.direction == Flex::Direction::Row) {
      return perform_flex_children_layout_<Flex::Direction::Row>(
          flex, self_extent, child_nodes);
    } else {
      return perform_flex_children_layout_<Flex::Direction::Column>(
          flex, self_extent, child_nodes);
    }
  }

  void clean(Extent const &allotted_extent) {
    perform_layout(root_node, allotted_extent, Offset{0, 0});
    is_layout_dirty = false;
  }

  void build_node(Widget &widget, LayoutTree::Node &node, uint64_t tree_depth) {
    node.widget = &widget;
    node.type = node.widget->get_type();
    node.tree_depth = tree_depth;

    WidgetStateProxyAccessor::access(widget).on_layout_dirty = [this] {
      this->is_layout_dirty = true;
    };

    size_t const num_children = widget.get_children().size();

    node.children.resize(num_children, LayoutTree::Node{});

    for (size_t i = 0; i < num_children; i++) {
      build_node(*widget.get_children()[i], node.children[i], tree_depth + 1);
    }

    // we don't really want to process this imeediately as the user could misuse
    // it and dereference data that shouldn't be dereferenced, we need a vector
    // to store the dirty children info, instead of modifying it here any
    // binded-to structure must not be moved nor its address changed
    WidgetStateProxyAccessor::access(widget).on_children_changed = [&node,
                                                                    this] {
      // optimizations: if it is a view type, detach the whole view from the
      // view tree
      // node.detach_info;
      // notify us of children changed or should we make modifications here?
      //
      // recursively detach child nodes from view tree with a best-case of it
      // being a view (thid means we need to pass a boolean paramter to signify
      // if it has already been removed) recursively detach child noes from the
      // render tree
      //
      // void * view_tree_binding;
      // view tree would have to reference some structs here
      //
      // on detach, the listeners must be removed
      //
      // we need to prevent the user callback from calling this multiple times
      // what if a child has already been removed? we need to sort requests by
      // tree depth too so we first remove the one at the lowest depth before
      // proceeding upwards
      //
      // how do we remove widgets and what data structures are necessary? we'll
      // need the view pointer from removing from the view tree, and the widget
      // pointer for removing from the raster tree (we could back up the z-index
      // to make searching on the raster tree faster)
      //
      //
      //
      //
      //
      // TODO(lamarrr): how do we process in trasversal order? i.e. if the
      // parent is detached before the child and a new detach for the child
      // comes in, we'd be referencing a dead node. we might need a fast tag to
      // recognize the hierarchical relationship.
      //
      // perform a priority insertion with the lowest tree index being the first
      // in the slot
      //
      //
      // recurse into children, remove child from the render tree, and view tree
      // and then remove from layout tree
      //
      // now do this for the lower tree depth widgets
      //
      //
      // test for uniqueness, find request insertion position using partition
      //
      //
      //
      //
      //
      // Pop from layout tree, then pop from render tree and mark the area they
      // occupy as dirty
      //
      //
      //
      //
      //
      ChildrenUpdateRequest request{};
      request.node = &node;
      request.tree_depth = node.tree_depth;
      this->submit_children_update_request(std::move(request));
    };
  }

  void build(Widget &widget) {
    VLK_ENSURE(!this->root_node.has_children());
    VLK_ENSURE(this->root_node.widget == nullptr);

    root_node.self_extent = {};
    root_node.parent_offset = {};
    root_node.view_extent = {};

    build_node(widget, root_node, 0);
  }

  // just recursively detach all callbacks
  void teardown();

  void tick(std::chrono::nanoseconds const &interval) {
    // tick all widgets
    // TODO(lamarrr): how to process detach requests whilst preserving cache
    // process view offset
    // process dirtiness
    // rasterize
    // forward changes to backing store
    // repeat
    for (auto const &children_update_request : children_update_requests) {
    }

    children_update_requests.clear();
  }
};

}  // namespace ui
}  // namespace vlk
