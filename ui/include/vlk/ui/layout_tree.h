
#pragma once

#include <vector>

#include "vlk/ui/impl/widget_state_proxy_accessor.h"
#include "vlk/ui/layout.h"
#include "vlk/ui/primitives.h"
#include "vlk/ui/widget.h"

namespace vlk {
namespace ui {

// on layout dirty, this must update the view tree
struct LayoutTree {
  // effective view offset and screen offset don't reside here
  struct Node {
    /// target widget
    Widget *widget;

    /// target widget type
    Widget::Type type;

    /// part of the parent view this widget occupies
    Rect parent_view_area;

    /// for view widgets
    Offset view_offset;  // we can move this out

    /// for view widgets
    Extent view_extent;
    // TODO(lamarrr): parent_view_area is not really needed here
    // can we remove view information from here>, I think it'd be better if we
    // have minimum info here, cross-check for ones we can remove.

    /// part of the parent widget this widget occupies
    Offset parent_offset;

    /// Rect parent_area;

    /// the child nodes (corresponds to child widgets)
    std::vector<Node> children;

    // void * view_tree_binding;
    // view tree would have to reference some structs here

    // we can have bindings in such a way that we trasverse up the tree until we
    // find a fixed extent and then begin updating the layout from there
    // bool is_fixed;
    // Node *parent;
    // bool is_dirty;
  };

  Node root_node;

  bool is_layout_dirty;
};

inline Extent perform_flex_children_layout(
    Flex const &flex, Extent const &self_extent,
    stx::Span<LayoutTree::Node> const &child_nodes);

inline void perform_layout(LayoutTree::Node &node,
                           Offset const &allotted_parent_offset,
                           Extent const &allotted_extent,
                           Offset const &allotted_view_offset) {
  Widget &widget = *node.widget;

  SelfExtent self_extent = widget.get_self_extent();

  if (widget.has_children()) {
    // do we need view extent? since its expected to contain its children
    // view problems:
    // - what if its extent needs to be the extent of its view?
    // - what if its extent needs to be absolute?
    //
    Flex flex = widget.get_flex();

    SelfExtent view_extent = widget.get_view_extent();

    Extent flex_span = perform_flex_children_layout(
        flex,
        node.type == Widget::Type::View
            ? view_extent.resolve(Extent{u32_max, u32_max})
            : self_extent.resolve(allotted_extent),
        node.children);

    // how to use children_span
    // shrink to fit? will mess up offsets
    // problem: extent allotted to flex might not be enough or might be too
    // much.
    for (LayoutTree::Node &child : node.children) {
      child.parent_view_area.offset =
          (widget.get_type() == Widget::Type::View ? Offset{0, 0}
                                                   : allotted_view_offset) +
          child.parent_offset;
    }

    // how about views? this should be widget_extent for views
    // constrain span to the allotted extent
    node.parent_view_area.extent = flex_span;
  } else {
    node.parent_view_area.extent = self_extent.resolve(allotted_extent);
  }

  node.parent_view_area.offset = allotted_view_offset;
}

template <Flex::Direction direction>
inline Extent perform_flex_children_layout_(
    Flex const &flex, Extent const &self_extent,
    stx::Span<LayoutTree::Node> const &child_nodes) {
  Flex::CrossAlign cross_align = flex.cross_align;
  Flex::MainAlign main_align = flex.main_align;
  Flex::Wrap wrap = flex.wrap;

  auto present_block_start = child_nodes.begin();
  auto child_it = child_nodes.begin();

  uint32_t block_max_width = 0;
  uint32_t block_max_height = 0;

  Offset present_offset{0, 0};
  uint32_t num_blocks = 0;

  for (LayoutTree::Node &child : child_nodes) {
    // the width allotted to this widget **must** be constrained.
    // overflow shouldn't occur since the child widget's extent is resolved
    // using the parent's
    perform_layout(child, Offset{0, 0}, self_extent, Offset{0, 0});
  }

  while (child_it < child_nodes.end()) {
    child_it->parent_offset.x = present_offset.x;
    child_it->parent_offset.y = present_offset.y;

    block_max_width =
        std::max(block_max_width, child_it->parent_view_area.extent.width);
    block_max_height =
        std::max(block_max_height, child_it->parent_view_area.extent.height);

    auto next_child_it = child_it + 1;

    // next widget is at the end of the block or at the end of the children list
    if ((next_child_it < child_nodes.end() &&
         ((direction == Flex::Direction::Row &&
           (child_it->parent_offset.x +
            child_it->parent_view_area.extent.width +
            next_child_it->parent_view_area.extent.width) >
               self_extent.width) ||
          (direction == Flex::Direction::Column &&
           (child_it->parent_offset.y +
            child_it->parent_view_area.extent.height +
            next_child_it->parent_view_area.extent.height) >
               self_extent.height))) ||
        next_child_it == child_nodes.end()) {
      // each block will have at least one widget
      for (auto &child :
           stx::Span<LayoutTree::Node>(present_block_start, next_child_it)) {
        // cross-axis alignment
        uint32_t cross_space = 0;

        if constexpr (direction == Flex::Direction::Row) {
          cross_space = block_max_height - child.parent_view_area.extent.height;
        } else {
          cross_space = block_max_width - child.parent_view_area.extent.width;
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
            if (child.parent_view_area.extent.height != block_max_height) {
              perform_layout(*child_it, Offset{0, 0},
                             Extent{self_extent.width, block_max_height},
                             Offset{0, 0});
            }
          } else {
            // re-layout the child to the max block width
            if (child.parent_view_area.extent.width != block_max_width) {
              perform_layout(*child_it, Offset{0, 0},
                             Extent{block_max_width, self_extent.height},
                             Offset{0, 0});
            }
          }
        } else if (cross_align == Flex::CrossAlign::Start || true) {
          // already done
        }
      }

      // should we make the invisible ones have a 0 offset and 0 extent
      uint32_t main_space = 0;

      if constexpr (direction == Flex::Direction::Row) {
        main_space =
            self_extent.width - (child_it->parent_offset.x +
                                 child_it->parent_view_area.extent.width);
      } else {
        main_space =
            self_extent.height - (child_it->parent_offset.y +
                                  child_it->parent_view_area.extent.height);
      }

      uint32_t num_block_children = next_child_it - present_block_start;

      if (main_align == Flex::MainAlign::End) {
        uint32_t main_space_end = main_space;
        for (auto &child :
             stx::Span<LayoutTree::Node>(present_block_start, next_child_it)) {
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

        for (auto &child :
             stx::Span<LayoutTree::Node>(present_block_start, next_child_it)) {
          new_offset += main_space_around;
          if constexpr (direction == Flex::Direction::Row) {
            child.parent_offset.x = new_offset;
            new_offset +=
                child.parent_view_area.extent.width + main_space_around;
          } else {
            child.parent_offset.y = new_offset;
            new_offset +=
                child.parent_view_area.extent.height + main_space_around;
          }
        }
      } else if (main_align == Flex::MainAlign::SpaceBetween) {
        uint32_t new_offset = 0;

        if constexpr (direction == Flex::Direction::Row) {
          new_offset += present_block_start->parent_view_area.extent.width;
        } else {
          new_offset += present_block_start->parent_view_area.extent.height;
        }

        // there's always atleast one element in a block
        for (auto &child : stx::Span<LayoutTree::Node>(present_block_start + 1,
                                                       next_child_it)) {
          // this expression is in the block scope due to possible
          // division-by-zero if it only has one element, this loop will only
          // be entered if it has at-least 2 children
          uint32_t main_space_between = main_space / (num_block_children - 1);
          new_offset += main_space_between;

          if constexpr (direction == Flex::Direction::Row) {
            child.parent_offset.x = new_offset;
            new_offset += child.parent_view_area.extent.width;
          } else {
            child.parent_offset.y = new_offset;
            new_offset += child.parent_view_area.extent.height;
          }
        }

      } else if (main_align == Flex::MainAlign::SpaceEvenly) {
        uint32_t main_space_evenly = main_space / (num_block_children + 1);
        uint32_t new_offset = main_space_evenly;
        for (auto &child :
             stx::Span<LayoutTree::Node>(present_block_start, child_it)) {
          if constexpr (direction == Flex::Direction::Row) {
            child.parent_offset.x = new_offset;
            new_offset +=
                child.parent_view_area.extent.width + main_space_evenly;
          } else {
            child.parent_offset.y = new_offset;
            new_offset +=
                child.parent_view_area.extent.height + main_space_evenly;
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
          present_offset.x += child_it->parent_view_area.extent.width;
          // present_offset.y never changes
        } else {
          present_offset.y += child_it->parent_view_area.extent.height;
          // present_offset.x never changes
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
        num_blocks++;
      }

    } else {
      if constexpr (direction == Flex::Direction::Row) {
        present_offset.x += child_it->parent_view_area.extent.width;
      } else {
        present_offset.y += child_it->parent_view_area.extent.height;
      }
    }

    child_it++;
  }

  Extent flex_span{0, 0};

  // TODO(lamarrr): this isn't actually correct?
  // we can also do this on a per-block basis, width and height individually,
  // using block_max_height and main spacing
  for (LayoutTree::Node &child : child_nodes) {
    // tell the effective extent or span here
    flex_span.width =
        std::max(flex_span.width,
                 child.parent_view_area.extent.width + child.parent_offset.x);
    flex_span.height =
        std::max(flex_span.height,
                 child.parent_view_area.extent.height + child.parent_offset.y);
  }

  flex_span.width = std::min(flex_span.width, self_extent.width);
  flex_span.height = std::min(flex_span.height, self_extent.height);

  return flex_span;
}

inline Extent perform_flex_children_layout(
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

inline void clean_layout_tree(LayoutTree &layout_tree,
                              Extent const &start_extent) {
  perform_layout(layout_tree.root_node, Offset{0, 0}, start_extent,
                 Offset{0, 0});
  layout_tree.is_layout_dirty = false;
}

inline void append_widget_layout_tree_node_(LayoutTree &tree, Widget &widget,
                                            LayoutTree::Node &parent_node) {
  LayoutTree::Node node{};
  node.type = widget.get_type();
  node.widget = &widget;
  node.parent_offset = {};     // not yet initialized
  node.parent_view_area = {};  // not yet initialized

  WidgetStateProxyAccessor::access(widget).on_layout_dirty = [&tree]() {
    tree.is_layout_dirty = true;
  };

  parent_node.children.push_back(std::move(node));
  for (auto &child : widget.get_children()) {
    append_widget_layout_tree_node_(tree, *child, parent_node.children.back());
  }
}

inline void build_widget_layout_tree(LayoutTree &tree, Widget &widget) {
  LayoutTree::Node node;
  append_widget_layout_tree_node_(tree, widget, node);
  tree.root_node = std::move(node.children[0]);
}

}  // namespace ui
}  // namespace vlk
