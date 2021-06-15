
#pragma once

#include <algorithm>
#include <chrono>
#include <tuple>
#include <vector>

#include "stx/span.h"
#include "vlk/ui/layout.h"
#include "vlk/ui/primitives.h"
#include "vlk/ui/widget.h"

namespace vlk {
namespace ui {

// this tree is very hazardrous and fragile with memory addresses, be sure to
// know what you're doing, especially whilst binding references to nodes whithin
// callbacks.
//

constexpr Extent flex_fit(Direction direction, Fit main_fit, Fit cross_fit,
                          Extent const &span, Extent const &allotted_extent) {
  Extent result{};

  if (main_fit == Fit::Shrink) {
    if (direction == Direction::Row) {
      result.width = std::min(span.width, allotted_extent.width);
    } else {
      result.height = std::min(span.height, allotted_extent.height);
    }
  } else {
    // expand
    if (direction == Direction::Row) {
      result.width = allotted_extent.width;
    } else {
      result.height = allotted_extent.height;
    }
  }

  if (cross_fit == Fit::Shrink) {
    if (direction == Direction::Row) {
      result.height = std::min(span.height, allotted_extent.height);
    } else {
      result.width = std::min(span.width, allotted_extent.width);
    }
  } else {
    // expand
    if (direction == Direction::Row) {
      result.height = allotted_extent.height;
    } else {
      result.width = allotted_extent.width;
    }
  }

  return result;
}

// we also need to shrink?
// for a view, we need to fit its view_extent with its self_extent
// i.e. we want view_extent's height to be in sync with self_extent's height

// perform layout along this so so so dimension
// be careful: if any flex uses stretching, it'll use up all of the dimensions
// so we need to state ahead of time where it needs to layout to
//
// we might need a constrain?
//
// problem is that the root view doesn't know its dimensions ahead of time, so
// if any child uses an expand property in any of the dimensions, the constraint
// solver will fail
//
//
constexpr Extent view_fit(ViewFit fit, Extent const &view_extent,
                          Extent const &final_self_extent) {
  Extent result_view_extent = view_extent;
  if ((fit & ViewFit::Width) != ViewFit::None) {
    result_view_extent.width = final_self_extent.width;
  }

  if ((fit & ViewFit::Height) != ViewFit::None) {
    result_view_extent.width = final_self_extent.width;
  }

  return result_view_extent;
}

constexpr Extent view_fit_self_extent(ViewFit fit,
                                      Extent const &resolved_self_extent,
                                      Extent const &view_extent) {
  Extent result_self_extent = resolved_self_extent;
  if ((fit & ViewFit::Width) != ViewFit::None &&
      view_extent.width <= resolved_self_extent.width) {
    result_self_extent.width = view_extent.width;
  }

  if ((fit & ViewFit::Height) != ViewFit::None &&
      view_extent.height <= resolved_self_extent.height) {
    result_self_extent.height = view_extent.height;
  }

  return result_self_extent;
}

// returns the content rect relative to the resolved_extent
constexpr std::tuple<Rect, Padding> resolve_content_rect(
    Extent const &resolved_extent, Padding const &padding) {
  uint32_t const resolved_padding_top =
      std::min(padding.top, resolved_extent.height);
  uint32_t const resolved_padding_bottom =
      std::min(resolved_extent.height - resolved_padding_top, padding.bottom);

  uint32_t const resolved_padding_left =
      std::min(padding.left, resolved_extent.width);
  uint32_t const resolved_padding_right =
      std::min(resolved_extent.width - resolved_padding_left, padding.right);

  Offset const offset{resolved_padding_left, resolved_padding_top};
  Extent const extent{
      resolved_extent.width - resolved_padding_left - resolved_padding_right,
      resolved_extent.height - resolved_padding_top - resolved_padding_bottom};

  return std::make_tuple(
      Rect{offset, extent},
      Padding::trbl(resolved_padding_top, resolved_padding_right,
                    resolved_padding_bottom, resolved_padding_left));
}

// cache invalidation sources:
// - layout change
// - viewport resize
//
// invalidates:
// - view tree
// - tile cache
//
//
//
struct LayoutTree {
  struct Node {
    /// target widget
    Widget *widget{};

    /// target widget type
    WidgetType type{};

    /// part of the parent view this widget occupies
    Extent self_extent{};

    /// part of the parent widget this widget occupies
    Offset parent_offset{};

    /// initial parent view offset for this widget
    Offset parent_view_offset{};

    /// for view widgets
    Extent view_extent{};

    /// the child nodes (corresponds to child widgets)
    std::vector<Node> children{};

    void build(Widget &in_widget, LayoutTree &tree) {
      widget = &in_widget;
      type = widget->get_type();
      self_extent = Extent{};
      parent_offset = Offset{};
      parent_view_offset = Offset{};
      view_extent = Extent{};

      // NOTE: allocates memory, we might need an extra step to bind the lambda
      // references if we want to utilize cache to the max
      WidgetSystemProxy::get_state_proxy(in_widget).on_layout_dirty = [&tree] {
        tree.is_layout_dirty = true;
      };

      size_t const num_children = in_widget.get_children().size();

      // note that we are not releasing the memory used by the
      // layout tree already during build for a rebuild (if it fits)
      //
      // IMPORTANT: layout tree rebuild should also use this function
      //

      children.resize(num_children, Node{});

      for (size_t i = 0; i < num_children; i++) {
        children[i].build(*in_widget.get_children()[i], tree);
      }
    }
  };

  LayoutTree() = default;

  LayoutTree(LayoutTree const &) = delete;
  LayoutTree(LayoutTree &&) = delete;

  LayoutTree &operator=(LayoutTree const &) = delete;
  LayoutTree &operator=(LayoutTree &&) = delete;

  ~LayoutTree() = default;

  Node root_node{};
  Extent allotted_extent{0, 0};

  bool is_layout_dirty = true;

  static void force_clean_parent_view_offset(Node &node,
                                             Offset parent_view_offset) {
    node.parent_view_offset = parent_view_offset;

    // we move parent_view_offset calculation out of the layout step and perform
    // that in another step since we can't calculate it until the whole layout
    // is done? else we'd perform more recursive iterations than necessary
    for (auto &child : node.children) {
      force_clean_parent_view_offset(
          child, node.type == WidgetType::View
                     ? child.parent_offset
                     : (child.parent_offset + parent_view_offset));
    }
  }

  // if we resize will the view be able to keep track of its translation?
  static void perform_layout(LayoutTree::Node &node,
                             Extent const &allotted_extent) {
    // assumptions:
    //  no it's not        - being infinite in size is ok, it won't be drawn
    //  anyway
    //          - there's no numeric overflow since the parent can't allot
    //          more than its own extent and the constraint parameters
    //          enforces that for the children via clamping, so even if the
    //          child widget has a fixed extent, its resulting extent can't
    //          exceed the parent's

    Widget const &widget = *node.widget;

    WidgetType const type = widget.get_type();

    SelfExtent const self_extent = widget.get_self_extent();
    Extent const resolved_self_extent = self_extent.resolve(allotted_extent);

    ViewFit const view_fit = widget.get_view_fit();

    ViewExtent const view_extent = widget.get_view_extent();
    Extent const resolved_view_extent = view_extent.resolve(allotted_extent);

    Padding const padding = widget.get_padding();

    if (widget.is_flex()) {
      Flex const flex = widget.get_flex();

      auto const [view_content_rect, resolved_view_padding] =
          resolve_content_rect(resolved_view_extent, padding);
      auto const [self_content_rect, resolved_self_padding] =
          resolve_content_rect(resolved_self_extent, padding);

      Extent const flex_span = perform_flex_children_layout(
          flex,
          type == WidgetType::View ? view_content_rect.extent
                                   : self_content_rect.extent,
          node.children);

      // layout of children along parent is now done,
      // but the layout was performed relative to the {0, 0} offset along the
      // content rect (content_rect.extent without respecting padding).
      //
      // we also now need to initialize the layout along the parent view.
      for (LayoutTree::Node &child : node.children) {
        child.parent_offset =
            child.parent_offset + (type == WidgetType::View
                                       ? view_content_rect.offset
                                       : self_content_rect.offset);
      }

      if (type == WidgetType::View) {
        Extent const fitted_view_content_extent =
            flex_fit(flex.direction, flex.main_fit, flex.cross_fit, flex_span,
                     view_content_rect.extent);
        // padding already has a higher priority and its space is always
        // deducted first from the allotted extent
        //  so there's no need for re-calculating the padding.
        node.view_extent =
            fitted_view_content_extent +
            Extent{resolved_view_padding.left + resolved_view_padding.top,
                   resolved_view_padding.top + resolved_view_padding.bottom};
        node.self_extent = view_fit_self_extent(view_fit, resolved_self_extent,
                                                node.view_extent);
      } else {
        Extent const fitted_self_content_extent =
            flex_fit(flex.direction, flex.main_fit, flex.cross_fit, flex_span,
                     self_content_rect.extent);
        node.self_extent =
            fitted_self_content_extent +
            Extent{resolved_self_padding.left + resolved_self_padding.top,
                   resolved_self_padding.top + resolved_self_padding.bottom};

        // really shouldn't be used for non-view widgets, but set for
        // correctness purpose
        node.view_extent = node.self_extent;
      }
    } else {
      if (type == WidgetType::View) {
        node.view_extent = resolved_view_extent;
        node.self_extent = view_fit_self_extent(view_fit, resolved_self_extent,
                                                node.view_extent);
      } else {
        node.self_extent = resolved_self_extent;

        if (widget.needs_trimming()) {
          Extent const trimmed_extent = node.widget->trim(node.self_extent);
          node.self_extent = node.self_extent.constrain(trimmed_extent);
        }

        node.view_extent = node.self_extent;
      }
    }
  }

  template <Direction direction>
  static Extent perform_flex_children_layout_(
      Flex const &flex, Extent const &content_extent,

      stx::Span<LayoutTree::Node> const &children) {
    for (LayoutTree::Node &child : children) {
      // the width allotted to these child widgets **must** be
      // constrained, this especially due to the view widgets that may have a
      // u32_max extent. overflow shouldn't occur since the child widget's
      // extent is resolved using the parent's
      perform_layout(child, content_extent);
    }

    CrossAlign const cross_align = flex.cross_align;
    MainAlign const main_align = flex.main_align;
    Wrap const wrap = flex.wrap;

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
    if (cross_align != CrossAlign::Start) {
      if constexpr (direction == Direction::Row) {
        flex_span.height = content_extent.height;
      } else {
        flex_span.width = content_extent.width;
      }
    }

    if (main_align != MainAlign::Start) {
      if constexpr (direction == Direction::Row) {
        flex_span.width = content_extent.width;
      } else {
        flex_span.height = content_extent.height;
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
      // list, then we need to perform alignment
      if ((next_child_it < children.end() &&
           ((direction == Direction::Row &&
             (child_it->parent_offset.x + child_it->self_extent.width +
              next_child_it->self_extent.width) > content_extent.width) ||
            (direction == Direction::Column &&
             (child_it->parent_offset.y + child_it->self_extent.height +
              next_child_it->self_extent.height) > content_extent.height))) ||
          next_child_it == children.end()) {
        // each block will have at least one widget
        for (auto &child :
             stx::Span<LayoutTree::Node>(present_block_start, next_child_it)) {
          // cross-axis alignment
          uint32_t cross_space = 0;

          if constexpr (direction == Direction::Row) {
            cross_space = block_max_height - child.self_extent.height;
          } else {
            cross_space = block_max_width - child.self_extent.width;
          }

          // determine cross-axis span
          if (cross_align == CrossAlign::Start) {
            if constexpr (direction == Direction::Row) {
              flex_span.height =
                  std::max(flex_span.height,
                           child.parent_offset.y + child.self_extent.height);
            } else {
              flex_span.width =
                  std::max(flex_span.width,
                           child.parent_offset.x + child.self_extent.width);
            }
          }

          // determine main-axis span
          if (main_align == MainAlign::Start) {
            if constexpr (direction == Direction::Row) {
              flex_span.width =
                  std::max(flex_span.width,
                           child.parent_offset.x + child.self_extent.width);
            } else {
              flex_span.height =
                  std::max(flex_span.height,
                           child.parent_offset.y + child.self_extent.height);
            }
          }

          if (cross_align == CrossAlign::Center) {
            uint32_t cross_space_center = cross_space / 2;
            if constexpr (direction == Direction::Row) {
              child.parent_offset.y += cross_space_center;
            } else {
              child.parent_offset.x += cross_space_center;
            }
          } else if (cross_align == CrossAlign::End) {
            uint32_t cross_space_end = cross_space;
            if constexpr (direction == Direction::Row) {
              child.parent_offset.y += cross_space_end;
            } else {
              child.parent_offset.x += cross_space_end;
            }
          } else if (cross_align == CrossAlign::Stretch) {
            if constexpr (direction == Direction::Row) {
              // re-layout the child to the max block height
              if (child.self_extent.height != block_max_height) {
                perform_layout(*child_it,
                               Extent{content_extent.width, block_max_height});
              }
            } else {
              // re-layout the child to the max block width
              if (child.self_extent.width != block_max_width) {
                perform_layout(*child_it,
                               Extent{block_max_width, content_extent.height});
              }
            }
          } else if (cross_align == CrossAlign::Start || true) {
            // already done
          }
        }

        uint32_t main_space = 0;

        if constexpr (direction == Direction::Row) {
          main_space = content_extent.width - (child_it->parent_offset.x +
                                               child_it->self_extent.width);
        } else {
          main_space = content_extent.height - (child_it->parent_offset.y +
                                                child_it->self_extent.height);
        }

        uint32_t num_block_children = next_child_it - present_block_start;

        if (main_align == MainAlign::End) {
          uint32_t main_space_end = main_space;
          for (auto &child : stx::Span<LayoutTree::Node>(present_block_start,
                                                         next_child_it)) {
            if constexpr (direction == Direction::Row) {
              child.parent_offset.x += main_space_end;
            } else {
              child.parent_offset.y += main_space_end;
            }
          }
        } else if (main_align == MainAlign::SpaceAround) {
          uint32_t main_space_around = main_space / num_block_children;
          main_space_around /= 2;
          uint32_t new_offset = 0;

          for (auto &child : stx::Span<LayoutTree::Node>(present_block_start,
                                                         next_child_it)) {
            new_offset += main_space_around;
            if constexpr (direction == Direction::Row) {
              child.parent_offset.x = new_offset;
              new_offset += child.self_extent.width + main_space_around;
            } else {
              child.parent_offset.y = new_offset;
              new_offset += child.self_extent.height + main_space_around;
            }
          }
        } else if (main_align == MainAlign::SpaceBetween) {
          uint32_t new_offset = 0;

          if constexpr (direction == Direction::Row) {
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

            if constexpr (direction == Direction::Row) {
              child.parent_offset.x = new_offset;
              new_offset += child.self_extent.width;
            } else {
              child.parent_offset.y = new_offset;
              new_offset += child.self_extent.height;
            }
          }

        } else if (main_align == MainAlign::SpaceEvenly) {
          uint32_t main_space_evenly = main_space / (num_block_children + 1);
          uint32_t new_offset = main_space_evenly;
          for (auto &child :
               stx::Span<LayoutTree::Node>(present_block_start, child_it)) {
            if constexpr (direction == Direction::Row) {
              child.parent_offset.x = new_offset;
              new_offset += child.self_extent.width + main_space_evenly;
            } else {
              child.parent_offset.y = new_offset;
              new_offset += child.self_extent.height + main_space_evenly;
            }
          }

          if constexpr (direction == Direction::Row) {
            child_it->parent_offset.x = new_offset;
          } else {
            child_it->parent_offset.y = new_offset;
          }

        } else if (main_align == MainAlign::Start || true) {
          // already done
        }

        if (wrap == Wrap::None) {
          if constexpr (direction == Direction::Row) {
            present_offset.x += child_it->self_extent.width;
          } else {
            present_offset.y += child_it->self_extent.height;
          }
        } else {
          // move to the next row/column
          if constexpr (direction == Direction::Row) {
            present_offset.x = 0;
            present_offset.y += block_max_height;
          } else {
            present_offset.x += block_max_width;
            present_offset.y = 0;
          }

          present_block_start = child_it + 1;
        }

      } else {
        // no wrapping nor alignment needed
        if constexpr (direction == Direction::Row) {
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
    if (flex.direction == Direction::Row) {
      return perform_flex_children_layout_<Direction::Row>(flex, self_extent,
                                                           child_nodes);
    } else {
      return perform_flex_children_layout_<Direction::Column>(flex, self_extent,
                                                              child_nodes);
    }
  }

  void allot_extent(Extent const &new_allotted_extent) {
    if (allotted_extent != new_allotted_extent) {
      allotted_extent = new_allotted_extent;
      is_layout_dirty = true;
    }
  }

  void build(Widget &root_widget) {
    is_layout_dirty = true;
    // allotted_extent needs to be explicitly set
    root_node.build(root_widget, *this);
  }

  void tick(std::chrono::nanoseconds) {
    if (is_layout_dirty) {
      perform_layout(root_node, allotted_extent);
      force_clean_parent_view_offset(root_node, Offset{0, 0});

      is_layout_dirty = false;
    }
  }
};

}  // namespace ui
}  // namespace vlk
