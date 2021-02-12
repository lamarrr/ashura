
#pragma once

#include "vlk/ui/constraint_solver.h"
#include "vlk/ui/primitives.h"
#include "vlk/ui/widget.h"

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

inline void resolve_extent(WidgetLayoutTree::Node &node,
                           Extent const &allotted_extent);

// unbounded flex gets u32_max
// we need px_max

// widgets with children must have a flex layout
// this is essentially a layout pipeline
// TODO(lamarrr): all widgets with flex layout and no-wrap behaviour must be a
// view as we need to composite for them and their children might extend outside
// of the canvas

inline void flex_layout_no_flex_factor(
    Flex::Direction direction, Flex::Wrap wrap, Flex::MainAlign main_align,
    Flex::CrossAlign cross_align, Extent const &allotted,
    stx::Span<WidgetLayoutTree::Node> const &child_nodes) {
  // give the first child all of the extent
  // give the next child the remaining extent
  // continue till end
  // since each widget is a flex, there is no need for a dependent/independent
  // relationship, we just need a SelfSizing or a Flex parameter

  // does flex factor adding to one mean that's a block?

  // widget is a flex type

  // TODO(lamarrr): call on resolve with u32_min and u32_max and compare
  // result
  // TODO(lamarrr): change the limits to a reasonable limit. i.e. [0, 2048] by
  // [0, 2048] by default

  // constrained by parent along the main axis but flexible accross the cross
  // axis, raise warning if the allotted cross axis extent is not enough.

  auto present_block_start = child_nodes.begin();
  auto child_it = child_nodes.begin();

  [[maybe_unused]] uint32_t block_max_width = 0;

  uint32_t block_max_height = 0;
  Offset present_offset{0, 0};
  uint32_t num_blocks = 0;

  while (child_it < child_nodes.end()) {
    // do we still give the whole extent accross y?
    // ensure that none of the child widgets is flexible
    // should flex widgets be flexible?
    // use allotted_extent, even if u32_max
    // at the end if allotted_extent is u32_max this means it is allowed to be
    // flexible and must shrink itself? we have to consider cross-alignment.
    // or if the allotted extent isn't a flexible one we have to use the given
    // extent and use cross-axis alignment with the remaining space. else just
    // shrink to it.

    // allott the children a width equal to the parent's allotted main axis
    // extent. allott an unconstrained height (we don't want to use the
    // remaining parent's extent).
    // the width allotted to this widget **must** be constrained.
    resolve_extent(*child_it, Extent{allotted.width, vlk::u32_max});

    child_it->parent_offset.x = present_offset.x;
    child_it->parent_offset.y = present_offset.y;

    // TODO(lamarrr): extent clamping in relation to overflow of viewports

    block_max_height =
        std::max(block_max_height, child_it->parent_view_area.extent.height);

    auto next_child_it = child_it + 1;

    // at the end of the block or at the end of the children list
    if ((next_child_it < child_nodes.end() &&
         ((present_offset.x + next_child_it->parent_view_area.extent.width) >
          allotted.width)) ||
        next_child_it == child_nodes.end()) {
      // each block will have at least one widget
      for (auto &child : stx::Span<WidgetLayoutTree::Node>(present_block_start,
                                                           next_child_it)) {
        // cross-axis alignment
        uint32_t cross_space =
            block_max_height - child.parent_view_area.extent.height;

        if (cross_align == Flex::CrossAlign::Center) {
          uint32_t cross_space_center = cross_space / 2;
          child.parent_offset.y += cross_space_center;
        } else if (cross_align == Flex::CrossAlign::End) {
          uint32_t cross_space_end = cross_space;
          child.parent_offset.y += cross_space_end;
        } else if (cross_align == Flex::CrossAlign::Stretch) {
          // relayout the child to the max block height
          if (child.parent_view_area.extent.height != block_max_height) {
            resolve_extent(*child_it, Extent{allotted.width, block_max_height});
          }
        } else if (cross_align == Flex::CrossAlign::Start || true) {
          // already done
        }
      }

      // we presently allow widgets to exceed the parent extent
      // since widgets are drawn individually it'd make sense to make these a
      // view instead? else if the wrapped widgets exceed the extent, it'd cause
      // them to be drawn over other widgets. or should we make the invisible
      // ones have a 0 offset and 0 extent
      uint32_t main_space =
          allotted.width -
          (child_it->parent_offset.x + child_it->parent_view_area.extent.width);
      uint32_t num_block_children = next_child_it - present_block_start;

      if (main_align == Flex::MainAlign::End) {
        uint32_t main_space_end = main_space;
        for (auto &child : stx::Span<WidgetLayoutTree::Node>(
                 present_block_start, next_child_it)) {
          child.parent_offset.x += main_space_end;
        }
      } else if (main_align == Flex::MainAlign::SpaceAround) {
        uint32_t main_space_around = main_space / num_block_children;
        main_space_around /= 2;
        uint32_t new_offset = 0;
        for (auto &child : stx::Span<WidgetLayoutTree::Node>(
                 present_block_start, next_child_it)) {
          new_offset += main_space_around;
          child.parent_offset.x = new_offset;
          new_offset += child.parent_view_area.extent.width + main_space_around;
        }
      } else if (main_align == Flex::MainAlign::SpaceBetween) {
        uint32_t new_offset = 0;

        new_offset += present_block_start->parent_view_area.extent.width;

        // there's always atleast one element in a block
        for (auto &child : stx::Span<WidgetLayoutTree::Node>(
                 present_block_start + 1, next_child_it)) {
          // this expression is in the block scope due to possible
          // division-by-zero if it only has one element, this loop will only
          // be entered if it has at-least 2 children
          uint32_t main_space_between = main_space / (num_block_children - 1);

          new_offset += main_space_between;
          child.parent_offset.x = new_offset;
          new_offset += child.parent_view_area.extent.width;
        }

      } else if (main_align == Flex::MainAlign::SpaceEvenly) {
        uint32_t main_space_evenly = main_space / (num_block_children + 1);
        uint32_t new_offset = main_space_evenly;
        for (auto &child :
             stx::Span<WidgetLayoutTree::Node>(present_block_start, child_it)) {
          child.parent_offset.x = new_offset;
          new_offset += child.parent_view_area.extent.width + main_space_evenly;
        }
        child_it->parent_offset.x = new_offset;

      } else if (main_align == Flex::MainAlign::Start || true) {
        // already done
      }

      if (wrap == Flex::Wrap::None) {
        present_offset.x += child_it->parent_view_area.extent.width;
        // present_offset.y never changes
      } else {
        // move to the next row
        present_offset.x = 0;
        present_offset.y += block_max_height;
        present_block_start = child_it + 1;
        num_blocks++;
      }

    } else {
      present_offset.x += child_it->parent_view_area.extent.width;
    }

    child_it++;
  }
}

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
  RasterizationContext(GrRecordingContext &context) {
    recording_context_ = &context;
  }

  uint16_t sample_count() const { return sample_count_; }

  GrRecordingContext &recording_context() { return *recording_context_; }

  SkBudgeted is_budgeted() const { return budgeted_; }

  stx::Option<stx::ConstRef<SkSurfaceProps>> surface_properties() const {
    if (surface_properties_ == nullptr) return stx::None;

    return stx::Some<stx::ConstRef<SkSurfaceProps>>(*surface_properties_);
  }

  sk_sp<SkSurface> make_surface_from_image(sk_sp<SkImage> image) {
    auto info = image->imageInfo();
    auto surface = SkSurface::MakeFromBackendTexture(
        recording_context_, image->getBackendTexture(true),
        GrSurfaceOrigin::kTopLeft_GrSurfaceOrigin, sample_count_,
        info.colorType(), sk_ref_sp(info.colorSpace()), nullptr);
    VLK_DEBUG_ENSURE(surface != nullptr);
    return surface;
  }

  sk_sp<SkSurface> make_surface(Extent const &extent) {
    sk_sp surface = SkSurface::MakeRenderTarget(
        recording_context_, budgeted_,
        SkImageInfo::Make(SkISize::Make(extent.width, extent.height),
                          SkColorInfo(color_type_, alpha_type_, color_space_)));

    VLK_DEBUG_ENSURE(extent.width != 0 && extent.height != 0 &&
                     surface == nullptr);
    return surface;
  }

 private:
  // non-null
  GrRecordingContext *recording_context_;

  uint16_t sample_count_ = 1;
  SkColorType color_type_ = kN32_SkColorType;
  SkAlphaType alpha_type_ = kPremul_SkAlphaType;
  sk_sp<SkColorSpace> color_space_ = SkColorSpace::MakeSRGB();

  SkBudgeted budgeted_ = SkBudgeted::kYes;
  SkSurfaceProps *surface_properties_ = nullptr;
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
  auto const extent = snapshot.parent_view_area->extent;

  if (extent.width == 0 || extent.height == 0) {
    snapshot.cache.reset();
    return;
  }

  // TODO(lamarrr): bounds checking

  bool const needs_new_cache =
      snapshot.cache == nullptr ||
      static_cast<uint32_t>(snapshot.cache->imageInfo().width()) !=
          extent.width ||
      static_cast<uint32_t>(snapshot.cache->imageInfo().height()) !=
          extent.height;

  if (needs_new_cache) {
    engage_raster_cache(snapshot, context);
  } else {
    sk_sp sk_surface = context.make_surface_from_image(snapshot.cache);
    draw_to_surface(snapshot, sk_surface);
    sk_surface->flushAndSubmit(false);  // we don't need to sync with the
                                        // cpu, everything stays on GPU
  }
}

template <typename T, typename Value, typename Cmp>
inline void sorted_insert(std::vector<T> &vector, Value &&value, Cmp &&cmp) {
  auto insert_pos = std::lower_bound(vector.begin(), vector.end(), value,
                                     std::forward<Cmp &&>(cmp));
  vector.insert(insert_pos, std::forward<Value &&>(value));
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

void clean_layout_tree_(WidgetLayoutTree::Node &node,
                        Offset const &allotted_parent_offset,
                        Extent const &allotted_extent,
                        Offset const &allotted_view_offset) {
  SelfLayout self_layout = node.widget->get_self_layout();

  // if any of the node's width or height is dependent on the child's
  // extent, we need to get all of the child's extent in one pass as we
  // can't afford walking the tree for each property.
  //
  // this means the child must have updated its own layout based on the
  // allotted extent before the parent can decide what extent to choose for
  // itself.

  stx::Span<ChildLayout const> children_layout =
      node.widget->get_children_layout();

  size_t child_count = children_layout.size();

  uint32_t view_child_allotted_width = 0;
  uint32_t view_child_allotted_height = 0;
  ViewExtent view_extent = node.widget->get_view_extent();

  if (node.type == Widget::Type::View) {
    view_child_allotted_width =
        is_dependent(view_extent.width)
            ? resolve_view_child_allotted_layout(
                  std::get<DependentParameters>(view_extent.width)
                      .children_allocation,
                  allotted_extent.width)
            : resolve_view_child_allotted_layout(
                  std::get<IndependentParameters>(view_extent.width),
                  allotted_extent.width);

    view_child_allotted_height =
        is_dependent(view_extent.height)
            ? resolve_view_child_allotted_layout(
                  std::get<DependentParameters>(view_extent.height)
                      .children_allocation,
                  allotted_extent.height)
            : resolve_view_child_allotted_layout(
                  std::get<IndependentParameters>(view_extent.height),
                  allotted_extent.height);
  }

  for (size_t i = 0; i < child_count; i++) {
    WidgetLayoutTree::Node &child = node.children[i];
    ChildLayout const &child_allotted_layout = children_layout[i];

    Offset child_allotted_offset{
        resolve_child_allotted_layout(child_allotted_layout.x,
                                      node.type == Widget::Type::View
                                          ? view_child_allotted_width
                                          : allotted_extent.width),
        resolve_child_allotted_layout(child_allotted_layout.y,
                                      node.type == Widget::Type::View
                                          ? view_child_allotted_height
                                          : allotted_extent.height)};

    Extent child_allotted_extent{
        resolve_child_allotted_layout(child_allotted_layout.width,
                                      node.type == Widget::Type::View
                                          ? view_child_allotted_width
                                          : allotted_extent.width),
        resolve_child_allotted_layout(child_allotted_layout.height,
                                      node.type == Widget::Type::View
                                          ? view_child_allotted_height
                                          : allotted_extent.height)};

    clean_layout_tree_(child, child_allotted_offset, child_allotted_extent,
                       node.type == Widget::Type::View
                           ? child_allotted_offset
                           : (allotted_view_offset + child_allotted_offset));
  }

  // we can now proceed to calculating for the parent.
  // for each dependent one, we use the max value of the children's layouts
  // to calculate its extent since it is expected to contain all of its
  // children within its allotted extent.
  //
  // this represents the span of the children within the parent's allotted
  // area.
  Extent MaxChildExtent = {0, 0};

  for (WidgetLayoutTree::Node &child : node.children) {
    MaxChildExtent.width =
        std::max(MaxChildExtent.width,
                 child.parent_offset.x + child.parent_view_area.extent.width);
    MaxChildExtent.height =
        std::max(MaxChildExtent.height,
                 child.parent_offset.y + child.parent_view_area.extent.height);
  }

  Extent self_extent{};

  // TODO(lamarrr): we'd have to walk the tree anyway so why separate
  // dependent and indepent parameters since we can just set scale to 0 in
  // dependent for the independent?

  if (is_dependent(self_layout.width) || is_dependent(self_layout.height) ||
      (node.type == Widget::Type::View &&
       (is_dependent(view_extent.width) || is_dependent(view_extent.height))))
    VLK_DEBUG_ENSURE(node.widget->has_children(),
                     "Widget with dependent layout has no children");

  if (is_dependent(self_layout.width)) {
    self_extent.width =
        resolve_self_layout(std::get<DependentParameters>(self_layout.width),
                            MaxChildExtent.width, allotted_extent.width);
  } else {
    self_extent.width =
        resolve_self_layout(std::get<IndependentParameters>(self_layout.width),
                            allotted_extent.width);
  }

  if (is_dependent(self_layout.height)) {
    self_extent.height =
        resolve_self_layout(std::get<DependentParameters>(self_layout.height),
                            MaxChildExtent.height, allotted_extent.height);
  } else {
    self_extent.height =
        resolve_self_layout(std::get<IndependentParameters>(self_layout.height),
                            allotted_extent.height);
  }

  node.parent_offset = allotted_parent_offset;
  node.parent_view_area = Rect{Offset{allotted_view_offset}, self_extent};

  if (node.type == Widget::Type::View) {
    if (is_dependent(view_extent.width)) {
      node.view_extent.width =
          resolve_view_extent(std::get<DependentParameters>(view_extent.width),
                              MaxChildExtent.width, allotted_extent.width);
    } else {
      node.view_extent.width = resolve_view_extent(
          std::get<IndependentParameters>(view_extent.width),
          allotted_extent.width);
    }

    if (is_dependent(view_extent.height)) {
      node.view_extent.height =
          resolve_view_extent(std::get<DependentParameters>(view_extent.height),
                              MaxChildExtent.height, allotted_extent.height);
    } else {
      node.view_extent.height = resolve_view_extent(
          std::get<IndependentParameters>(view_extent.height),
          allotted_extent.height);
    }
  } else {
    node.view_extent = {};
  }

  if (node.type == Widget::Type::View) {
    ViewOffset view_offset = node.widget->get_view_offset();
    node.view_offset.x =
        resolve_view_offset(view_offset.x, node.view_extent.width);
    node.view_offset.y =
        resolve_view_offset(view_offset.y, node.view_extent.height);
  } else {
    node.view_offset = {};
  }
}

inline void clean_layout_tree(WidgetLayoutTree &layout_tree,
                              Extent const &start_extent) {
  clean_layout_tree_(layout_tree.root_node, Offset{0, 0}, start_extent,
                     Offset{0, 0});
  layout_tree.is_layout_dirty = false;
}

inline void append_widget_layout_tree_node_(
    WidgetLayoutTree &tree, Widget &widget,
    WidgetLayoutTree::Node &parent_node) {
  WidgetLayoutTree::Node node{};
  node.parent_offset = {};     // not yet initialized
  node.parent_view_area = {};  // not yet initialized
  node.type = widget.get_type();
  node.widget = &widget;
  WidgetStateProxyAdapter::install_on_layout_dirty(
      widget, [&tree]() { tree.is_layout_dirty = true; });

  parent_node.children.push_back(std::move(node));
  for (auto &child : widget.get_children()) {
    append_widget_layout_tree_node_(tree, *child, parent_node.children.back());
  }
}

void build_widget_layout_tree(WidgetLayoutTree &tree, Widget &widget) {
  WidgetLayoutTree::Node node;
  append_widget_layout_tree_node_(tree, widget, node);
  tree.root_node = std::move(node.children[0]);
}

void build_render_tree_(WidgetLayoutTree::Node &present_node,
                        RenderTree::View &target_view,
                        uint32_t parent_draw_order) {
  Widget *widget = present_node.widget;
  uint32_t draw_order = widget->get_type() == Widget::Type::View
                            ? 0
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
      view.view_offset = Offset{0, 0};
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

void build_render_tree(RenderTree &tree, WidgetLayoutTree::Node &root_node) {
  build_render_tree_(root_node, tree.root_view, 0);
}

}  // namespace ui
}  // namespace vlk
