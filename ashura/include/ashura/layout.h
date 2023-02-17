#pragma once

#include <algorithm>

#include "ashura/widget.h"

namespace ash {

inline vec2 perform_children_layout(Layout const &layout,
                                    stx::Span<Widget *const> children);

inline void perform_layout(Widget *widget, rect allotted_area) {
  Layout layout = widget->layout(allotted_area);

  vec2 span = perform_children_layout(layout, widget->get_children());

  vec2 extent = layout.flex.fit(span, allotted_area.extent);

  widget->area = rect{.offset = layout.area.offset, .extent = extent};
}

/// NOTE: we always dictate the offset for the children unless their
/// position is Static which makes them independent of the flex's layout and
/// do not participate in it
/// TODO(lamarrr): implement layout for Position::Static widgets
inline vec2 perform_children_layout(Layout const &layout,
                                    stx::Span<Widget *const> children) {
  if (children.is_empty()) return layout.area.extent;

  for (Widget *child : children) {
    perform_layout(child, layout.area);
  }

  vec2 cursor;

  // some alignments like center, evenly, end, space around, and space
  // between use the whole space and some don't.
  vec2 span;
  f32 max_block_element_width = 0;
  f32 max_block_element_height = 0;

  // alignments not positioned to start always utilize the full allotted
  // extent
  if (layout.flex.cross_align != CrossAlign::Start) {
    if (layout.flex.direction == Direction::Row) {
      span.y = layout.area.extent.y;
    } else {
      span.x = layout.area.extent.x;
    }
  }

  if (layout.flex.main_align != MainAlign::Start) {
    if (layout.flex.direction == Direction::Row) {
      span.x = layout.area.extent.x;
    } else {
      span.y = layout.area.extent.y;
    }
  }

  for (Widget *const *child_it = children.begin(), *const *present_block_start =
                                                       children.begin();
       child_it < children.end(); child_it++) {
    (*child_it)->area.offset = layout.area.offset + cursor;
    max_block_element_width =
        std::max(max_block_element_width, (*child_it)->area.extent.x);
    max_block_element_height =
        std::max(max_block_element_height, (*child_it)->area.extent.y);

    Widget *const *next_child_it = child_it + 1;

    // next widget is at the end of the block or at the end of the
    // children list, then we need to perform alignment
    if ((next_child_it < children.end() &&
         ((layout.flex.direction == Direction::Row &&
           ((*child_it)->area.offset.x + (*child_it)->area.extent.x +
            (*next_child_it)->area.extent.x) >
               (layout.area.offset.x + layout.area.extent.x)) ||
          (layout.flex.direction == Direction::Column &&
           ((*child_it)->area.offset.y + (*child_it)->area.extent.y +
            (*next_child_it)->area.extent.y) >
               (layout.area.offset.x + layout.area.extent.y)))) ||
        next_child_it == children.end()) {
      // each block will have at least one widget
      for (Widget *const *block_it = present_block_start;
           block_it < next_child_it; block_it++) {
        // cross-axis alignment
        f32 cross_space =
            layout.flex.direction == Direction::Row
                ? (max_block_element_height - (*block_it)->area.extent.y)
                : (max_block_element_width - (*block_it)->area.extent.x);

        // determine cross-axis span
        if (layout.flex.cross_align == CrossAlign::Start) {
          if (layout.flex.direction == Direction::Row) {
            span.y = std::max(span.y, cursor.y + (*block_it)->area.extent.y);
          } else {
            span.x = std::max(span.x, cursor.x + (*block_it)->area.extent.x);
          }
        }

        // determine main-axis span
        if (layout.flex.main_align == MainAlign::Start) {
          if (layout.flex.direction == Direction::Row) {
            span.x = std::max(span.x, cursor.x + (*block_it)->area.extent.x);
          } else {
            span.y = std::max(span.y, cursor.y + (*block_it)->area.extent.y);
          }
        }

        if (layout.flex.cross_align == CrossAlign::Center) {
          f32 cross_space_center = cross_space / 2;
          if (layout.flex.direction == Direction::Row) {
            (*block_it)->area.offset.y += cross_space_center;
          } else {
            (*block_it)->area.offset.x += cross_space_center;
          }
        } else if (layout.flex.cross_align == CrossAlign::End) {
          if (layout.flex.direction == Direction::Row) {
            (*block_it)->area.offset.y += cross_space;
          } else {
            (*block_it)->area.offset.x += cross_space;
          }
        } else if (layout.flex.cross_align == CrossAlign::Stretch) {
          vec2 new_offset = (*present_block_start)->area.offset;

          if (layout.flex.direction == Direction::Row) {
            // re-layout the child to the max block height
            if ((*block_it)->area.extent.y != max_block_element_height) {
              perform_layout(*block_it,
                             rect{.offset = new_offset,
                                  .extent = vec2{layout.area.extent.x,
                                                 max_block_element_height}});
              new_offset.x += (*block_it)->area.extent.x;
            }
          } else {
            // re-layout the child to the max block width
            if ((*block_it)->area.extent.x != max_block_element_width) {
              perform_layout(*block_it,
                             rect{.offset = new_offset,
                                  .extent = vec2{max_block_element_width,
                                                 layout.area.extent.y}});
              new_offset.y += (*block_it)->area.extent.y;
            }
          }
        } else {
          // already done
        }
      }

      f32 main_space = 0;

      // child_it is last element in block
      if (layout.flex.direction == Direction::Row) {
        main_space = layout.area.extent.x -
                     (((*child_it)->area.offset.x - layout.area.offset.x) +
                      (*child_it)->area.extent.x);
      } else {
        main_space = layout.area.extent.y -
                     (((*child_it)->area.offset.y - layout.area.offset.y) +
                      (*child_it)->area.extent.y);
      }

      usize nblock_children = next_child_it - present_block_start;

      if (layout.flex.main_align == MainAlign::End) {
        f32 main_space_end = main_space;

        for (Widget *const *block_it = present_block_start;
             block_it < next_child_it; block_it++) {
          if (layout.flex.direction == Direction::Row) {
            (*block_it)->area.offset.x += main_space_end;
          } else {
            (*block_it)->area.offset.y += main_space_end;
          }
        }
      } else if (layout.flex.main_align == MainAlign::SpaceAround) {
        f32 main_space_around = main_space / (nblock_children * 2);
        f32 new_offset = layout.flex.direction == Direction::Row
                             ? (*present_block_start)->area.offset.x
                             : (*present_block_start)->area.offset.y;

        for (Widget *const *block_it = present_block_start;
             block_it < next_child_it; block_it++) {
          new_offset += main_space_around;
          if (layout.flex.direction == Direction::Row) {
            (*block_it)->area.offset.x = new_offset;
            new_offset += (*block_it)->area.extent.x + main_space_around;
          } else {
            (*block_it)->area.offset.y = new_offset;
            new_offset += (*block_it)->area.extent.y + main_space_around;
          }
        }
      } else if (layout.flex.main_align == MainAlign::SpaceBetween) {
        f32 new_offset = layout.flex.direction == Direction::Row
                             ? (*present_block_start)->area.offset.x
                             : (*present_block_start)->area.offset.y;

        // there's always atleast one element in a block
        for (Widget *const *block_it = present_block_start + 1;
             block_it < next_child_it; block_it++) {
          // this expression is in the block scope due to possible
          // division-by-zero if it only has one element, this loop will
          // only be entered if it has at-least 2 children
          f32 main_space_between = main_space / (nblock_children - 1);
          new_offset += main_space_between;

          if (layout.flex.direction == Direction::Row) {
            (*block_it)->area.offset.x = new_offset;
            new_offset += (*block_it)->area.extent.x;
          } else {
            (*block_it)->area.offset.y = new_offset;
            new_offset += (*block_it)->area.extent.y;
          }
        }

      } else if (layout.flex.main_align == MainAlign::SpaceEvenly) {
        f32 main_space_evenly = main_space / (nblock_children + 1);
        f32 new_offset = layout.flex.direction == Direction::Row
                             ? (*present_block_start)->area.offset.x
                             : (*present_block_start)->area.offset.y;
        new_offset += main_space_evenly;

        for (Widget *const *block_it = present_block_start; block_it < child_it;
             block_it++) {
          if (layout.flex.direction == Direction::Row) {
            (*block_it)->area.offset.x = new_offset;
            new_offset += (*block_it)->area.extent.x + main_space_evenly;
          } else {
            (*block_it)->area.offset.y = new_offset;
            new_offset += (*block_it)->area.extent.y + main_space_evenly;
          }
        }

        if (layout.flex.direction == Direction::Row) {
          (*child_it)->area.offset.x = new_offset;
        } else {
          (*child_it)->area.offset.y = new_offset;
        }

      } else {
        // already done
      }

      if (layout.flex.wrap == Wrap::None) {
        if (layout.flex.direction == Direction::Row) {
          cursor.x += (*child_it)->area.extent.x;
        } else {
          cursor.y += (*child_it)->area.extent.y;
        }
      } else {
        // move to the next row/column
        if (layout.flex.direction == Direction::Row) {
          cursor.x = layout.area.offset.x;
          cursor.y += max_block_element_height;
        } else {
          cursor.x += max_block_element_width;
          cursor.y = layout.area.offset.y;
        }

        present_block_start = child_it + 1;

        // we are now at the end of the block, we thus need to reset the max
        // block width and height
        max_block_element_width = 0;
        max_block_element_height = 0;
      }
    } else {
      if (layout.flex.direction == Direction::Row) {
        cursor.x += (*child_it)->area.extent.x;
      } else {
        cursor.y += (*child_it)->area.extent.y;
      }
    }
  }

  return span;
}
}  // namespace ash
