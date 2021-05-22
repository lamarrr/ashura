
#pragma once

#include <cstddef>
#include <cstdint>
#include <numeric>
#include <utility>
#include <vector>
#include <map>

#include "vlk/ui/primitives.h"
#include "vlk/ui/primtives_utils.h"
#include "vlk/ui/surface_provider.h"
#include "vlk/ui/trace.h"
#include "vlk/ui/widget.h"

#include "vlk/utils/limits.h"
#include "vlk/utils/utils.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"
#include "stx/span.h"

// Thought: so many templates? well. you only need one compositor per translation unit.

#define VLK_ENABLE_COMPOSITOR_TRACING 0

#if VLK_ENABLE_COMPOSITOR_TRACING
#define VLK_COMPOSITOR_TRACE_SCOPE VLK_SCOPE_EVENT_TRACE_TO_SINK(CompositorTraceSink);
#define VLK_COMPOSITOR_TRACE_SCALAR(scalar) VLK_SCALAR_TRACE_TO_SINK(scalar, CompositorTraceSink)
#else
#define VLK_COMPOSITOR_TRACE_SCOPE \
  do {                             \
  } while (false)
#define VLK_COMPOSITOR_TRACE_SCALAR(scalar) \
  do {                                      \
  } while (false)
#endif

VLK_DECLARE_TRACE_SINK(CompositorTraceSink);

namespace vlk {
namespace ui {
namespace impl {

namespace {
// this is a strict overlapping check

}

// This snapshot is not aware of the parent-child dimensional relationship. It only has dimensional
// data required to position the render data of the widget on a target view.
struct Snapshot {
 private:
  Widget *widget_;

  // The raw rasterized image data, always constant for stateless widgets. For
  // stateful widgets, the rasterized image changes whenever its `is_dirty`
  // method returns true. For both stateful and stateless widgets, the image is
  // discarded when moved into residuals.
  sk_sp<SkImage> image_;

  // always valid. constant for stateless widgets. For stateful widgets it is
  // invalidated when its `is_dirty` method returns true.
  sk_sp<SkPicture> draw_commands_;

  // rect's offset represents distance from the parent view widget to this
  // widget, and its extent represents the dimensions of the widget
  Rect area_;

  explicit Snapshot(Widget &widget, Rect const &area)
      : widget_{&widget}, image_{}, draw_commands_{}, area_{area} {}

 public:
  Snapshot() = default;
  Snapshot(Snapshot const &) = delete;
  Snapshot &operator=(Snapshot const &) = delete;
  Snapshot(Snapshot &&) = default;
  Snapshot &operator=(Snapshot &&) = default;
  ~Snapshot() noexcept {
    VLK_DEBUG_ENSURE(image_ == nullptr,
                     "reached destructor without moving or discarding widget raster");
    VLK_DEBUG_ENSURE(draw_commands_ == nullptr,
                     "reached destructor without moving or discarding draw commands");
  }

  // returns the estimated memory usage of the raster image (if any)
  uint64_t image_size() const {
    if (image_ == nullptr) return 0;
    return image_->imageInfo().computeMinByteSize();
  }

  Widget *widget() noexcept { return widget_; }

  // represents the area of the parent view this widget occupies.
  Rect const &area() const noexcept { return area_; }

  // return a snapshot with no raster cache
  static Snapshot CreateRecorded(Widget &widget, Rect const &area) noexcept {
    auto snapshot = Snapshot(widget, area);
    snapshot.record_draw_commands();
    return snapshot;
  }

  void discard_image() {
    VLK_COMPOSITOR_TRACE_SCOPE;
    VLK_DEBUG_ENSURE(image_ != nullptr,
                     "called `discard_image` with no previous rasterization result/image");
    image_.reset(nullptr);
  }

  void discard_draw_commands() {
    VLK_COMPOSITOR_TRACE_SCOPE;
    VLK_DEBUG_ENSURE(draw_commands_ != nullptr,
                     "called `discard_draw_commands` with no previous draw "
                     "command recorded");
    draw_commands_.reset(nullptr);
  }

  void record_draw_commands() {
    VLK_COMPOSITOR_TRACE_SCOPE;
    VLK_DEBUG_ENSURE(draw_commands_ == nullptr,
                     "Attempting to record draw commands whilst still having "
                     "an undiscarded on");
    SkPictureRecorder recorder;
    SkCanvas *canvas_pimpl = recorder.beginRecording(area_.extent.width, area_.extent.height);
    VLK_DEBUG_ENSURE(canvas_pimpl != nullptr);

    auto canvas = Canvas::FromSkia(canvas_pimpl);
    widget_->draw(canvas, area_.extent);
    draw_commands_ = recorder.finishRecordingAsPicture();
  }

  void rasterize(SurfaceProvider &surface_provider) {
    VLK_COMPOSITOR_TRACE_SCOPE;
    VLK_DEBUG_ENSURE(draw_commands_ != nullptr,
                     "called `rasterize()` with no previously recorded draw command",
                     widget_->get_type_hint());
    sk_sp gpu_surface = surface_provider.make_surface(area_.extent);
    VLK_DEBUG_ENSURE(gpu_surface != nullptr, "Returned surface from surface provider is nullptr");

    SkCanvas *canvas = gpu_surface->getCanvas();

    VLK_DEBUG_ENSURE(canvas != nullptr, "SkCanvas is nullptr");

    canvas->clear(colors::Transparent.argb());
    canvas->drawPicture(
        draw_commands_.get());  // SkPaint, and SkMatrix for transform (surface zooming)
    image_ = gpu_surface->makeImageSnapshot();
  }

  // TODO(lamarrr):  this should take the view_area, and the surface_area of the view
  void render_cache(SkCanvas &view_canvas, Rect const &view_area) {
    VLK_COMPOSITOR_TRACE_SCOPE;
    VLK_DEBUG_ENSURE(image_ != nullptr,
                     "called `render_cache()` with no previous rasterization cache/image");

    VLK_DEBUG_ENSURE(is_overlapping(view_area, area_),
                     "attempting to render without actually being in view");

    // skia accepts floating point negative coordinates so we use that to draw on the view canvas
    int64_t x_start =
        static_cast<int64_t>(area_.offset.x) - static_cast<int64_t>(view_area.offset.x);
    int64_t y_start =
        static_cast<int64_t>(area_.offset.y) - static_cast<int64_t>(view_area.offset.y);

    view_canvas.drawImage(image_.get(), static_cast<float>(x_start), static_cast<float>(y_start));
  }

  bool is_draw_commands_recorded() const noexcept { return draw_commands_ != nullptr; }

  bool is_rasterized() const noexcept { return image_ != nullptr; }
};

// TODO(lamarrr): dynamic z_index? or static only?
struct CacheEntry {
  Snapshot snapshot;

  uint32_t z_index;

  // represents the amount of time since the widget left the root surface view.
  // if it exceeds a specified maximum, the cache entry is removed.
  uint64_t out_of_view_ticks;

  CacheEntry() = default;
  CacheEntry(Snapshot &&snapShot, uint32_t zIndex, uint64_t outOfViewTicks = 0)
      : snapshot{std::move(snapShot)}, z_index{zIndex}, out_of_view_ticks{outOfViewTicks} {}

  static CacheEntry MakeStub() { return CacheEntry(); }

  CacheEntry(CacheEntry const &) = delete;
  CacheEntry &operator=(CacheEntry const &) = delete;
  CacheEntry(CacheEntry &&) = default;
  CacheEntry &operator=(CacheEntry &&) = default;
  ~CacheEntry() = default;
};

namespace {
STX_FORCE_INLINE void discard_all_snapshot_images(stx::Span<CacheEntry> const &entries) {
  for (auto &entry : entries) entry.snapshot.discard_image();
}

STX_FORCE_INLINE void discard_all_snapshot_draw_commands(stx::Span<CacheEntry> const &entries) {
  for (auto &entry : entries) entry.snapshot.discard_draw_commands();
}

STX_FORCE_INLINE void discard_all_snapshots(stx::Span<CacheEntry> const &entries) {
  for (auto &entry : entries) {
    entry.snapshot.discard_draw_commands();
    entry.snapshot.discard_image();
  }
}

STX_FORCE_INLINE void update_out_of_view_ticks(CacheEntry &entry, Rect const &view_area) noexcept {
  if (is_overlapping(view_area, entry.snapshot.area())) {
    entry.out_of_view_ticks = 0;
  } else {
    entry.out_of_view_ticks++;
  }
}
}  // namespace

template <typename CacheEntryAllocator = std::allocator<CacheEntry>>
struct Residuals : private std::vector<CacheEntry, CacheEntryAllocator> {
  using base = std::vector<CacheEntry, CacheEntryAllocator>;

  using base::base;
  using base::begin;
  using base::cbegin;
  using base::cend;
  using base::crbegin;
  using base::crend;
  using base::data;
  using base::emplace_back;
  using base::empty;
  using base::end;
  using base::rbegin;
  using base::rend;
  using base::resize;
  using base::size;

  void uncache(CacheEntry &&cache_entry) {
    VLK_COMPOSITOR_TRACE_SCOPE;
    cache_entry.snapshot.discard_image();
    base::emplace_back(std::move(cache_entry));
  }

  void uncache(stx::Span<CacheEntry> const &cache_entries) {
    VLK_COMPOSITOR_TRACE_SCOPE;
    for (CacheEntry &entry : cache_entries) {
      entry.snapshot.discard_image();
      base::emplace_back(std::move(entry));
    }
  }

  ~Residuals() { discard_all_snapshot_draw_commands(*this); }
};

// Least Recently Used cache
template <typename CacheEntryAllocator = std::allocator<CacheEntry>>
struct Cache : private std::vector<CacheEntry, CacheEntryAllocator> {
  using base = std::vector<CacheEntry, CacheEntryAllocator>;

  using base::base;
  using base::begin;
  using base::cbegin;
  using base::cend;
  using base::crbegin;
  using base::crend;
  using base::data;
  using base::emplace_back;
  using base::empty;
  using base::end;
  using base::rbegin;
  using base::rend;
  using base::resize;
  using base::size;

  // widgets in cache are always sorted by z-index (increasing order)
  void cache(stx::Span<CacheEntry> const &residuals, SurfaceProvider &surface_provider) {
    VLK_COMPOSITOR_TRACE_SCOPE;

    for (CacheEntry &residual : residuals) {
      residual.snapshot.rasterize(surface_provider);
      auto insert_pos = std::lower_bound(
          base::begin(), base::end(), residual,
          [](CacheEntry const &a, CacheEntry const &b) { return a.z_index < b.z_index; });
      base::insert(typename base::const_iterator{insert_pos}, std::move(residual));
    }
  }

  uint64_t images_size() const noexcept {
    return std::accumulate(
        base::begin(), base::end(), static_cast<uint64_t>(0),
        [](uint64_t size, CacheEntry const &entry) { return entry.snapshot.image_size() + size; });
  }

  ~Cache() { discard_all_snapshots(*this); }
};

// TODO(lamarrr): should views have z_indexes?
// TODO(lamarrr): remove std::pair?
template <typename CacheEntryAllocator = std::allocator<CacheEntry>,
          typename LayoutWidgetsInfoAllocator = std::allocator<std::pair<Widget *, Rect>>>
struct View {
  using cache_type = Cache<CacheEntryAllocator>;
  using residuals_type = Residuals<CacheEntryAllocator>;
  using layout_widgets_info_type =
      std::vector<std::pair<Widget *, Rect>, LayoutWidgetsInfoAllocator>;

  Widget *view_widget;

  // position and extent of the view on the overall surface
  Rect parent_view_area;

  // subset of `surface_area`'s extent
  Rect present_view;

  cache_type stateful_cache;   // cache is initialized on the first
                               // render call. when the widget goes out
                               // of view it goes into the residual bin
  cache_type stateless_cache;  // cache is initialized on the first render
                               // call. the snapshot is updated if the widget
                               // becomes dirty or is moved from the residual
                               // bin to the cache

  residuals_type stateless_residuals;
  residuals_type stateful_residuals;

  layout_widgets_info_type stateless_layout_widgets;
  layout_widgets_info_type stateful_layout_widgets;


// sorted by z-index
std::vector<View> child_views;


};

namespace {

template <bool IsStateful, typename AllocatorResiduals, typename AllocatorCache>
STX_FORCE_INLINE void LRU_resolve(Residuals<AllocatorResiduals> &residuals,
                                  Cache<AllocatorCache> &cache, SurfaceProvider &surface_provider,
                                  Rect const &view_area, uint64_t max_out_of_view_ticks) {
  VLK_COMPOSITOR_TRACE_SCOPE;

  // we need to preserve order of the widgets in the cache as they are sorted by z-index
  auto cache_out_of_view_iterator = std::stable_partition(
      cache.begin(), cache.end(), [view_area, max_out_of_view_ticks](CacheEntry &entry) {
        update_out_of_view_ticks(entry, view_area);
        return entry.out_of_view_ticks <= max_out_of_view_ticks;
      });

  if constexpr (IsStateful) {
    for (CacheEntry &in_view_cached_entry :
         stx::Span<CacheEntry>(cache.begin().base(), cache_out_of_view_iterator.base())) {
      if (in_view_cached_entry.snapshot.widget()->is_dirty()) {
        in_view_cached_entry.snapshot.discard_image();
        in_view_cached_entry.snapshot.rasterize(surface_provider);
        in_view_cached_entry.snapshot.widget()->mark_clean();
      }
    }
  }

  residuals.uncache(stx::Span<CacheEntry>(cache_out_of_view_iterator.base(), cache.end().base()));

  cache.resize(cache_out_of_view_iterator - cache.begin());

  // we don't need to preserve z-index order since they are out of view
  auto residuals_in_view_iterator = std::partition(
      residuals.begin(), residuals.end(), [view_area, max_out_of_view_ticks](CacheEntry &entry) {
        update_out_of_view_ticks(entry, view_area);
        return entry.out_of_view_ticks != 0;
      });

  cache.cache(stx::Span<CacheEntry>(residuals_in_view_iterator.base(), residuals.end().base()),
              surface_provider);
  residuals.resize(residuals_in_view_iterator - residuals.begin());
}

// TODO(lamarrr): calculating correct area
template <typename CacheEntryAllocator, typename LayoutWidgetsInfoAllocator>
inline void add_widget_to_view(View<CacheEntryAllocator, LayoutWidgetsInfoAllocator> &target_view,
                               Widget *widget, Rect const &surface_area, uint32_t z_index) {
  if (widget->is_layout_type()) {
    if (widget->is_stateful()) {
      target_view.stateful_layout_widgets.emplace_back(widget, surface_area);
    } else {
      target_view.stateless_layout_widgets.emplace_back(widget, surface_area);
    }
  } else {
    VLK_DEBUG_ENSURE(widget->get_type() = Widget::Type::Render);
    CacheEntry entry{Snapshot::CreateRecorded(*widget, surface_area),
                     widget->z_index().is_none() ? z_index : widget->z_index().clone().unwrap(), 0};
    if (widget->is_stateful()) {
      target_view.stateful_residuals.emplace_back(std::move(entry));
    } else {
      target_view.stateless_residuals.emplace_back(std::move(entry));
    }
  }
}

template <typename CacheEntryAllocator, typename LayoutWidgetsInfoAllocator>
inline void build_views(
    std::vector<View<CacheEntryAllocator, LayoutWidgetsInfoAllocator>> &stateless_views,
    std::vector<View<CacheEntryAllocator, LayoutWidgetsInfoAllocator>> &stateful_views,
    View<CacheEntryAllocator, LayoutWidgetsInfoAllocator> &target_view,
    Extent const &allotted_extent, Offset const &allotted_parent_view_offset,
    Offset const &parent_view_allotted_surface_offset, uint32_t start_z_index, Widget *widget) {
  using view_type = View<CacheEntryAllocator, LayoutWidgetsInfoAllocator>;

  // TODO(lamarrr): how do we prevent the widget from doing extra work on
  // computing the sizes of its children? (i.e. for a 3-level nested widget, the
  // root widget's height depends on its children and sub-children) and each of
  // them would try to compute their heights using that. Caching can also make
  // it considerably slower since there will be memory allocation for every
  // value insertion, so we don't cache for now until we sight a noticable bottleneck.

  stx::Span<Widget *const> children = widget->get_children();

  std::vector<Rect> children_allotted_area;
  auto const num_children = children.size();

  children_allotted_area.resize(num_children, Rect{});

  // constrained for non-view widgets to {allotted_extent}
  Rect desired_parent_area = widget->compute_area(allotted_extent, children_allotted_area);

  Rect widget_parent_area = clamp_rect(desired_parent_area, allotted_extent);

  // only used for the view widget and is constrained to {desired_parent_area.extent}
  Rect desired_present_widget_view =
      widget->compute_view_area(allotted_extent, widget_parent_area.extent);

  Rect present_widget_view = clamp_rect(desired_present_widget_view, widget_parent_area.extent);

  // only used for the view widget and is constrained to {allotted_extent}
  Rect view_widget_desired_parent_area =
      Rect{desired_parent_area.offset, desired_present_widget_view.extent};

  Rect view_widget_parent_area = clamp_rect(view_widget_desired_parent_area, allotted_extent);

#if VLK_ENABLE_DEBUG_CHECKS

  if (widget->is_render_type() || widget->is_layout_type()) {
    overflow_warn(desired_parent_area, allotted_extent, widget);
  } else {
    VLK_ENSURE(widget->is_view_type());
    overflow_warn(view_widget_desired_parent_area, allotted_extent, widget);
  }

#endif

  // TODO(lamarrr): is this correct?
  // Offset widget_surface_offset = allotted_surface_offset + widget_parent_offset;

  // used for actual drawing and positioning
  // Rect widget_surface_area{widget_surface_offset, widget_extent};

  view_type *children_view = &target_view;

  if (widget->is_view_type()) {
    view_type view{};
    view.view_widget = widget;
    view.present_view = present_widget_view;
    view.surface_area = Rect{};

    if (widget->is_stateless()) {
      stateless_views.emplace_back(std::move(view));
      children_view = &(stateless_views.back());
    } else {
      stateful_views.emplace_back(std::move(view));
      children_view = &(stateful_views.back());
    }

  } else {
   // add_widget_to_view(target_view, widget, widget_surface_area, start_z_index);
  }

  for (size_t i = 0; i < num_children; i++) {
    Widget *child = children[i];

   // Offset allotted_child_surface_offset = widget_surface_offset + children_allotted_area[i].offset;
   // Extent allotted_child_extent = children_allotted_area[i].extent;

    // z-index only increases for render widgets
   // build_views(stateless_views, stateful_views, *children_view, child, allotted_child_extent,
     //           allotted_child_surface_offset, start_z_index + (child->is_render_type() ? 1u : 0u));
  }
}

}  // namespace

// it is not in charge of deleting the referenced widgets
// implements a time-based least recently used (TLRU) caching behaviour
// TODO(lamarrr): fix this template mess
template <typename ViewType = View<>, typename ViewAllocator = std::allocator<ViewType>>
struct Compositor {
  // usually at 60 FPS, 45 seconds timeout per widget cache after being out of view => (60 x 45)
  static constexpr uint64_t kDefaultMaxOutOfViewTicks = 2700;

  Compositor(SurfaceProvider &surface_provider, Extent const &surface_extent, Rect const &view_area,
             Widget &root_widget, uint64_t max_out_of_view_ticks = kDefaultMaxOutOfViewTicks)
      : surface_provider_{&surface_provider},
        view_surface_{},
        view_area_{view_area},
        surface_extent_{surface_extent},
        root_widget_{&root_widget},
        max_out_of_view_ticks_{max_out_of_view_ticks} {
    VLK_COMPOSITOR_TRACE_SCOPE;

    // process:
    // build view (whilst keeping track of covered area), build child widgets or layouts or render
    // widgets
    // the stored cache entry represents the position of the widget in the parent view.
 //   build_views(stateless_views_, stateful_views_, root_widget_, surface_extent,
   //             surface_area.offset, 0);

    view_surface_ = surface_provider_->make_surface(view_area_.extent);
  }

  ~Compositor() = default;

  // TODO(lamarrr): this can effectivelty be used as a multi-layer cache
  // first render and fill caches, continue rendering with caches
  // this is the post-initial fill
  //
  // add the stateless ones to the oneshot cache, if (maximum depth not
  // exceeded) else, add to residuals.
  //
  // tick must be called before render_widgets
  // TODO(lamarrr): ticking the compositor should make it also send tick events
  // to the widgets. helps to prevent having multiple copies of the widgets
  sk_sp<SkImage> tick([[maybe_unused]] std::chrono::nanoseconds interval) {
    VLK_COMPOSITOR_TRACE_SCOPE;

    // TODO(lamarrr): render to surface argument instead of having own surface
    // TODO(lamarrr): all widgets are stateful on first render pass
    // TODO(lamarrr): do we need to render even the cache again if no part of the scene has changed?

    for (ViewType &view : stateful_views_) {
      if (view.view_widget->is_dirty()) {
        // TODO(lamarrr): rebuild view.
        // won't we be doing too much work? especially if the whole treee needs to be rebuilt
        view.view_widget->mark_clean();
      }
    }

    stx::Span<ViewType> const all_views[] = {stx::Span(stateless_views_),
                                             stx::Span(stateful_views_)};

    for (stx::Span views : all_views)
      for (ViewType &view : views) {
        for ([[maybe_unused]] auto &[layout_widget, surface_area] : view.stateful_layout_widgets) {
          // TODO(lamarrr): rebuild widget tree, and loop through all widgets and
          // update their individual areas on the widget tree
          // consider what data will be invalidated and optimize for that
          // if(layout_widget->is_stateful())
        }
      }

    // TODO(lamarrr): LRU resolve shouldn't be modified, view.view_area?

    for (stx::Span views : all_views)
      for (ViewType &view : views) {
        LRU_resolve<false>(view.stateless_residuals, view.stateless_cache, *surface_provider_,
                           view.view_area, max_out_of_view_ticks_);
        LRU_resolve<true>(view.stateful_residuals, view.stateful_cache, *surface_provider_,
                          view.view_area, max_out_of_view_ticks_);
      }

    // render the widgets

    SkCanvas *view_canvas = view_surface_->getCanvas();
    view_canvas->clear(colors::Transparent.argb());

    VLK_ENSURE(view_canvas != nullptr);

    for (stx::Span views : all_views)
      for (ViewType &view : views) {
        if (is_overlapping(view.surface_area, view_area_)) {
          // TODO(lamarrr): are these correct?
          for (CacheEntry &entry : view.stateless_cache) {
            // TODO(lamarrr): snapshot.area() should be relative to the root view and renamed to
            // view_area?
            if (is_overlapping(entry.snapshot.area(), view.surface_area))
              entry.snapshot.render_cache(*view_canvas, view.surface_area);
          }

          for (CacheEntry &entry : view.stateful_cache) {
            if (is_overlapping(entry.snapshot.area(), view.surface_area))
              entry.snapshot.render_cache(*view_canvas, view.surface_area);
          }
        }
      }

    // TODO(lamarrr): how to optimize resizing with layout widgets
    // process on_surface_area_changed events

    return view_surface_->makeImageSnapshot();
  }

  auto &get_stateless_views() noexcept { return stateless_views_; }
  auto &get_stateful_views() noexcept { return stateful_views_; }

 private:
  SurfaceProvider *surface_provider_;

  sk_sp<SkSurface> view_surface_;

  std::vector<ViewType, ViewAllocator> stateless_views_;
  std::vector<ViewType, ViewAllocator> stateful_views_;

  Rect view_area_;

  // when building, it is checked whether a child view belongs to another view and doesn't exceed

  Extent surface_extent_;

  Widget *root_widget_;
  uint64_t max_out_of_view_ticks_;
  std::map<Widget *, std::vector<Widget *, Widget *>> view_widget_mapping;
};

}  // namespace impl
}  // namespace ui
}  // namespace vlk
