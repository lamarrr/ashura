
#pragma once

#include "vlk/ui/layout.h"
#include "vlk/ui/primitives.h"
#include "vlk/ui/widget.h"

#include <algorithm>
#include <utility>
#include <vector>

#include "include/core/SkCanvas.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrBackendSurface.h"

namespace vlk {
namespace ui {

// we'll need to test this on column and others
// give all the children all of the space of the parent, if it exhausts it

/// Layout tree is optimized and used for layout calculations and trasversal of
/// each widget's extent. In a child-parent constrain manner. We move down the
/// tree with an allotted extent, and then move back up with the
/// resolved/calculated layouts/dimensions.
/// there is an implicit constraint that a child's extent should not exceed the
/// parent's extent.
struct WidgetLayoutTree {
  struct Node {
    /// target widget
    Widget *widget;

    /// target widget type
    Widget::Type type;

    /// part of the parent view this widget occupies
    Rect parent_view_area;

    /// part of the parent widget this widget occupies
    Offset parent_offset;

    /// the child nodes (corresponds to child widgets)
    std::vector<Node> children;

    /// for view widgets
    Offset view_offset;

    /// for view widgets
    Extent view_extent;
  };

  Node root_node;

  bool is_layout_dirty;
};

struct WidgetSnapshot {
  /// target widget
  Widget *widget;

  /// the view area of its parent view that the target widget occupies, this
  /// references the `parent_view_area` on the layout tree.
  Rect const *parent_view_area;

  /// widget's z-index
  uint32_t z_index;

  /// widget's draw commands
  sk_sp<SkPicture> draw_commands;

  /// cache data
  sk_sp<SkImage> cache;

  /// time since the widget left its parent view
  Ticks out_of_view_ticks;

  /// if the widget desires to cache itself (this doesn't change and isn't
  /// updated either)
  bool needs_caching;

  bool is_dirty;
};

/// RenderTree is a tree that is optimized and used for rendering and
/// maintaining widget snapshots. we can just take the list of snapshots and
/// render immediately.
///
/// The snapshots are sorted by draw order (z-index), each
/// z-index is relative to the parent view. A view's widgets can't be
/// interleaved with another views widgets as parent views are drawn before
/// the child view.
struct RenderTree {
  struct View {
    Widget *widget;

    uint32_t z_index;

    Rect const *parent_view_area;

    /// sorted by stack index.
    std::vector<WidgetSnapshot> in_view_snapshots;

    /// their raster cache will not be updated.
    /// not sorted in any particular order.
    std::vector<WidgetSnapshot> out_of_view_snapshots;

    /// sorted by stack index.
    std::vector<View> in_view_child_views;

    /// not sorted in any particular order.
    std::vector<View> out_of_view_child_views;

    /// time since the view left its parent view
    Ticks out_of_view_ticks;

    bool is_view_offset_dirty;

    Offset view_offset;
  };

  View root_view;
};

inline sk_sp<SkPicture> record_draw_commands(Widget &widget,
                                             Extent const &canvas_extent) {
  SkPictureRecorder picture_recorder;

  SkCanvas *sk_recorder_canvas = picture_recorder.beginRecording(
      canvas_extent.width, canvas_extent.height);

  Canvas recorder_canvas{sk_recorder_canvas, canvas_extent};

  widget.draw(recorder_canvas);

  return picture_recorder.finishRecordingAsPicture();
}

struct RasterizationContext {
  // lifetime depends on [context]
  RasterizationContext(
      GrRecordingContext &context, uint16_t sample_count = 1,
      SkColorType color_type = kN32_SkColorType,
      SkAlphaType alpha_type = kPremul_SkAlphaType,
      sk_sp<SkColorSpace> const &color_space = SkColorSpace::MakeSRGB(),
      SkBudgeted budgeted = SkBudgeted::kYes,
      SkSurfaceProps *surface_properties = nullptr)
      : recording_context_{&context},
        sample_count_{sample_count},
        color_type_{color_type},
        alpha_type_{alpha_type},
        color_space_{color_space},
        budgeted_{budgeted},
        surface_properties_{surface_properties} {}

  uint16_t sample_count() const { return sample_count_; }

  GrRecordingContext &recording_context() { return *recording_context_; }

  SkBudgeted is_budgeted() const { return budgeted_; }

  stx::Option<stx::ConstRef<SkSurfaceProps>> surface_properties() const {
    if (surface_properties_ == nullptr) return stx::None;

    return stx::Some<stx::ConstRef<SkSurfaceProps>>(*surface_properties_);
  }

  sk_sp<SkSurface> make_surface_from_image(sk_sp<SkImage> const &image) {
    auto info = image->imageInfo();
    auto surface = SkSurface::MakeFromBackendTexture(
        recording_context_, image->getBackendTexture(true),
        GrSurfaceOrigin::kTopLeft_GrSurfaceOrigin, sample_count_,
        info.colorType(), sk_ref_sp(info.colorSpace()), nullptr);
    VLK_DEBUG_ENSURE(surface != nullptr);
    return surface;
  }

  sk_sp<SkSurface> make_surface(Extent const &extent) {
    VLK_DEBUG_ENSURE(extent.width <= i32_max);
    VLK_DEBUG_ENSURE(extent.height <= i32_max);

    sk_sp surface = SkSurface::MakeRenderTarget(
        recording_context_, budgeted_,
        SkImageInfo::Make(SkISize::Make(extent.width, extent.height),
                          SkColorInfo(color_type_, alpha_type_, color_space_)));

    VLK_DEBUG_ENSURE(extent.width != 0);
    VLK_DEBUG_ENSURE(extent.height != 0);
    VLK_DEBUG_ENSURE(surface != nullptr);

    return surface;
  }

 private:
  // non-null
  GrRecordingContext *recording_context_;

  uint16_t sample_count_;
  SkColorType color_type_;
  SkAlphaType alpha_type_;

  // non-null
  sk_sp<SkColorSpace> color_space_;

  SkBudgeted budgeted_;

  // can be null
  SkSurfaceProps *surface_properties_;
};

inline void draw_to_surface(WidgetSnapshot &snapshot,
                            sk_sp<SkSurface> &sk_surface) {
  VLK_DEBUG_ENSURE(sk_surface != nullptr);
  SkCanvas *sk_canvas = sk_surface->getCanvas();
  VLK_DEBUG_ENSURE(sk_canvas != nullptr);
  sk_canvas->clear(SK_ColorTRANSPARENT);
  sk_canvas->drawPicture(snapshot.draw_commands.get());
}

inline void engage_raster_cache(WidgetSnapshot &snapshot,
                                RasterizationContext &context) {
  sk_sp sk_surface = context.make_surface(snapshot.parent_view_area->extent);
  draw_to_surface(snapshot, sk_surface);

  snapshot.cache = sk_surface->makeImageSnapshot();
  VLK_DEBUG_ENSURE(snapshot.cache != nullptr);
}

inline void update_raster_cache(WidgetSnapshot &snapshot,
                                RasterizationContext &context) {
  // we first check if the sizing has changed and re-use the raster cache
  // instead of re-allocating the raster cache.
  auto const extent = snapshot.parent_view_area->extent;

  if (extent.width == 0 || extent.height == 0) {
    snapshot.cache.reset();
    return;
  }

  VLK_DEBUG_ENSURE(extent.width <= i32_max);
  VLK_DEBUG_ENSURE(extent.height <= i32_max);

  bool const needs_new_cache =
      snapshot.cache == nullptr ||
      static_cast<uint32_t>(snapshot.cache->imageInfo().width()) !=
          extent.width ||
      static_cast<uint32_t>(snapshot.cache->imageInfo().height()) !=
          extent.height;

  if (needs_new_cache) {
    engage_raster_cache(snapshot, context);
  } else {
    // re-use the existing raster cache
    sk_sp sk_surface = context.make_surface_from_image(snapshot.cache);
    draw_to_surface(snapshot, sk_surface);
    sk_surface->flushAndSubmit(false);  // we don't need to sync with the
                                        // cpu, everything stays on GPU
  }
}


inline void LRU_resolve_cache(RenderTree::View &view, Rect const &view_area,
                              Ticks max_out_of_view_ticks,
                              RasterizationContext &context) {
  //  we try to keep the memory allocations to a minimum
  //
  // perform stable partion on the in_view snapshot (so the stack indexes
  // are maintained). discard the cache content of the lower (out_of_view)
  // partition. remove all associated proxies that maintain render data
  // state. and call their `on_leave_view` methods

  auto const just_out_of_view_snapshots_begin = std::stable_partition(
      view.in_view_snapshots.begin(), view.in_view_snapshots.end(),
      [max_out_of_view_ticks](WidgetSnapshot const &snapshot) {
        return snapshot.out_of_view_ticks <= max_out_of_view_ticks;
      });

  bool const any_leave_view =
      just_out_of_view_snapshots_begin != view.in_view_snapshots.end();

  // cache clean-up
  for (WidgetSnapshot &snapshot :
       stx::Span<WidgetSnapshot>(view.in_view_snapshots.begin().base(),
                                 just_out_of_view_snapshots_begin.base())) {
    if (snapshot.is_dirty) {
      snapshot.draw_commands = record_draw_commands(
          *snapshot.widget, snapshot.parent_view_area->extent);

      if (snapshot.needs_caching) {
        update_raster_cache(snapshot, context);
      } else {
        snapshot.cache = nullptr;
      }

      snapshot.is_dirty = false;
    }
  }

  // cache discard
  for (WidgetSnapshot &snapshot :
       stx::Span<WidgetSnapshot>(just_out_of_view_snapshots_begin.base(),
                                 view.in_view_snapshots.end().base())) {
    if (snapshot.needs_caching) snapshot.cache.reset();

    if (snapshot.is_dirty) {
      snapshot.draw_commands = record_draw_commands(
          *snapshot.widget, snapshot.parent_view_area->extent);
      snapshot.is_dirty = false;
    }

    snapshot.widget->on_view(Widget::ViewEvent::Leave);
  }

  // create a new vector using the just out of view partition and move the
  // newly oov partition into the vector.
  //
  // the in view vector should resize itself to
  // the size of the still in-view snapshots (already moved), thereby
  // discarding unused slots (this will not necessarily cause a memory
  // re-allocation)

  std::vector<WidgetSnapshot> out_of_view_snapshots_tmp;

  out_of_view_snapshots_tmp.resize(view.in_view_snapshots.end() -
                                   just_out_of_view_snapshots_begin);

  std::move(just_out_of_view_snapshots_begin, view.in_view_snapshots.end(),
            out_of_view_snapshots_tmp.begin());

  view.in_view_snapshots.resize(just_out_of_view_snapshots_begin -
                                view.in_view_snapshots.begin());

  // perform unstable partiton on the out_of_view snapshots (we don't care
  // about the order of the snapshots).
  //
  // reload the cache content of the lower (now in-view) partition and call
  // their `on_enter_view()` handler.
  //
  // tell the in view snapshot vector to reserve enough to store the still
  // in view widgets and the newly in view widgets.
  //
  // tell the in_view snapshot to reserve a capacity enough to store the
  // number of still in view snapshots plus the number of newly in view
  // snapshots perform a sorted insert of each newly in view widget into the
  // in view vector.
  auto just_in_view_snapshots_begin = std::partition(
      view.out_of_view_snapshots.begin(), view.out_of_view_snapshots.end(),
      [](WidgetSnapshot const &snapshot) {
        return snapshot.out_of_view_ticks != Ticks{0};
      });

  bool const any_enter_view =
      just_in_view_snapshots_begin != view.out_of_view_snapshots.end();

  // cache engaging
  for (WidgetSnapshot &snapshot :
       stx::Span<WidgetSnapshot>(just_in_view_snapshots_begin.base(),
                                 view.out_of_view_snapshots.end().base())) {
    if (snapshot.is_dirty) {
      snapshot.draw_commands = record_draw_commands(
          *snapshot.widget, snapshot.parent_view_area->extent);
      snapshot.is_dirty = false;
    }

    if (snapshot.needs_caching) {
      engage_raster_cache(snapshot, context);
    }

    snapshot.widget->on_view(Widget::ViewEvent::Enter);

    sorted_insert(view.in_view_snapshots, std::move(snapshot),
                  [](WidgetSnapshot const &sn1, WidgetSnapshot const &sn2) {
                    return sn1.out_of_view_ticks < sn2.out_of_view_ticks;
                  });
  }

  // tell the newly out of view widget to reserve enough to store its size
  // plus the size of the still out of view widgets. move the the still out
  // of view widgets to the vector for the newly out of view snapshots.
  // replace the out of view vector with the newly out of view vector,
  // thereby freeing unused memory.
  auto const just_out_of_view_size = out_of_view_snapshots_tmp.size();
  auto const still_out_of_view_size =
      just_in_view_snapshots_begin - view.out_of_view_snapshots.begin();

  out_of_view_snapshots_tmp.resize(out_of_view_snapshots_tmp.size() +
                                   still_out_of_view_size);

  std::move(view.out_of_view_snapshots.begin(), just_in_view_snapshots_begin,
            out_of_view_snapshots_tmp.begin() + just_out_of_view_size);

  view.out_of_view_snapshots = std::move(out_of_view_snapshots_tmp);

  // if any of the snapshots is moved, the addresses of each snapshot
  // element is changed so we need to re-install the callbacks
  if (any_enter_view || any_leave_view) {
    for (WidgetSnapshot &snapshot : view.in_view_snapshots) {
      WidgetStateProxyAdapter::install_on_render_dirty(
          *snapshot.widget, [&snapshot] { snapshot.is_dirty = true; });
    }

    for (WidgetSnapshot &snapshot : view.out_of_view_snapshots) {
      WidgetStateProxyAdapter::install_on_render_dirty(
          *snapshot.widget, [&snapshot] { snapshot.is_dirty = true; });
    }
  }
}

inline Extent perform_flex_children_layout(
    Flex const &flex, Extent const &self_extent,
    stx::Span<WidgetLayoutTree::Node> const &child_nodes);

inline void perform_layout(WidgetLayoutTree::Node &node,
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

    Extent widget_extent = self_extent.resolve(allotted_extent);
    SelfExtent view_extent = widget.get_view_extent();

    Extent flex_span = perform_flex_children_layout(
        flex,
        widget.get_type() == Widget::Type::View
            ? view_extent.resolve(Extent{u32_max, u32_max})
            : widget_extent,
        node.children);

    // how to use children_span
    // shrink to fit? will mess up offsets
    // problem: extent allotted to flex might not be enough or might be too
    // much.
    for (WidgetLayoutTree::Node &child : node.children) {
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
    stx::Span<WidgetLayoutTree::Node> const &child_nodes) {
  Flex::CrossAlign cross_align = flex.cross_align;
  Flex::MainAlign main_align = flex.main_align;
  Flex::Wrap wrap = flex.wrap;

  auto present_block_start = child_nodes.begin();
  auto child_it = child_nodes.begin();

  uint32_t block_max_width = 0;
  uint32_t block_max_height = 0;

  Offset present_offset{0, 0};
  uint32_t num_blocks = 0;

  for (WidgetLayoutTree::Node &child : child_nodes) {
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
      for (auto &child : stx::Span<WidgetLayoutTree::Node>(present_block_start,
                                                           next_child_it)) {
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
        for (auto &child : stx::Span<WidgetLayoutTree::Node>(
                 present_block_start, next_child_it)) {
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

        for (auto &child : stx::Span<WidgetLayoutTree::Node>(
                 present_block_start, next_child_it)) {
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
        for (auto &child : stx::Span<WidgetLayoutTree::Node>(
                 present_block_start + 1, next_child_it)) {
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
             stx::Span<WidgetLayoutTree::Node>(present_block_start, child_it)) {
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
  for (WidgetLayoutTree::Node &child : child_nodes) {
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
    stx::Span<WidgetLayoutTree::Node> const &child_nodes) {
  if (flex.direction == Flex::Direction::Row) {
    return perform_flex_children_layout_<Flex::Direction::Row>(
        flex, self_extent, child_nodes);
  } else {
    return perform_flex_children_layout_<Flex::Direction::Column>(
        flex, self_extent, child_nodes);
  }
}

inline void clean_layout_tree(WidgetLayoutTree &layout_tree,
                              Extent const &start_extent) {
  perform_layout(layout_tree.root_node, Offset{0, 0}, start_extent,
                 Offset{0, 0});
  layout_tree.is_layout_dirty = false;
}

inline void append_widget_layout_tree_node_(
    WidgetLayoutTree &tree, Widget &widget,
    WidgetLayoutTree::Node &parent_node) {
  WidgetLayoutTree::Node node{};
  node.type = widget.get_type();
  node.widget = &widget;
  node.parent_offset = {};     // not yet initialized
  node.parent_view_area = {};  // not yet initialized

  WidgetStateProxyAdapter::install_on_layout_dirty(
      widget, [&tree]() { tree.is_layout_dirty = true; });

  parent_node.children.push_back(std::move(node));
  for (auto &child : widget.get_children()) {
    append_widget_layout_tree_node_(tree, *child, parent_node.children.back());
  }
}

inline void build_widget_layout_tree(WidgetLayoutTree &tree, Widget &widget) {
  WidgetLayoutTree::Node node;
  append_widget_layout_tree_node_(tree, widget, node);
  tree.root_node = std::move(node.children[0]);
}

inline void build_render_tree_(WidgetLayoutTree::Node &present_node,
                               RenderTree::View &target_view,
                               uint32_t parent_draw_order) {
  Widget *widget = present_node.widget;
  uint32_t draw_order = widget->get_type() == Widget::Type::View
                            ? uint32_t{0}
                            : widget->get_preferred_stack_index().unwrap_or(
                                  parent_draw_order + 1);

  switch (present_node.type) {
    // layout widgets are not added to the render tree since they don't have
    // render data
    case Widget::Type::Layout: {
      for (WidgetLayoutTree::Node &child : present_node.children) {
        build_render_tree_(child, target_view, draw_order);
      }
    } break;

    case Widget::Type::Render: {
      WidgetSnapshot snapshot{};
      snapshot.cache = nullptr;
      snapshot.is_dirty = true;
      snapshot.draw_commands =
          record_draw_commands(*widget, present_node.parent_view_area.extent);
      snapshot.needs_caching = widget->should_cache();
      snapshot.out_of_view_ticks = Ticks{0};
      snapshot.parent_view_area = &present_node.parent_view_area;
      // snapshot.post_effects = snapshot.widget->get_post_effects();
      // snapshot.pre_effects = snapshot.widget->get_pre_effects();
      // we'll need to listen for effects dirtiness (post and pre) and also
      // establish this for the view tree
      snapshot.widget = widget;
      snapshot.z_index = draw_order;

      target_view.out_of_view_snapshots.emplace_back(std::move(snapshot));
      for (WidgetLayoutTree::Node &child : present_node.children) {
        build_render_tree_(child, target_view, draw_order);
      }

    } break;

    case Widget::Type::View: {
      RenderTree::View view{};

      view.is_view_offset_dirty = true;
      view.out_of_view_ticks = Ticks{0};
      view.parent_view_area = &present_node.parent_view_area;
      view.view_offset = widget->get_view_offset().resolve(
          present_node.parent_view_area.extent);
      view.widget = widget;
      view.z_index = draw_order;

      target_view.out_of_view_child_views.emplace_back(std::move(view));

      for (WidgetLayoutTree::Node &child : present_node.children) {
        build_render_tree_(child, target_view.out_of_view_child_views.back(),
                           draw_order);
      }

    } break;

    default:
      VLK_PANIC("Unimplemented widget type");
  }
}

inline void build_render_tree(RenderTree &tree,
                              WidgetLayoutTree::Node &root_node) {
  build_render_tree_(root_node, tree.root_view, 0);
}

inline void attach_view_listeners_(RenderTree::View &view) {
  WidgetStateProxyAdapter::install_on_view_offset_dirty(
      *view.widget, [&view]() {
        view.view_offset = view.widget->get_view_offset().resolve(
            view.parent_view_area->extent);
        view.is_view_offset_dirty = true;
      });
  // out of view children, in view children
  // this effectively means its cached raster data is dirty
}

inline void detach_view_listeners_(RenderTree::View &view) {
  WidgetStateProxyAdapter::detach_on_view_offset_dirty(*view.widget);
}

inline void attach_view_listeners(RenderTree &tree) {}

struct Composition {
  // accumulated into on a src-over blending mode
  // we need a method to return a read-only view to this image, so the user does
  // not modify it, since we'll be updating it as necessary
  // dirty-region updating or whole frame updating
  sk_sp<SkImage> result;
};

}  // namespace ui
}  // namespace vlk
