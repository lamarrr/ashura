
#pragma once

#include <cstddef>
#include <cstdint>
#include <numeric>
#include <utility>
#include <vector>

#include "vlk/ui2d/primitives.h"
#include "vlk/ui2d/surface_provider.h"
#include "vlk/ui2d/trace.h"
#include "vlk/ui2d/widget.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"
#include "stx/span.h"

#include "vlk/utils/limits.h"
#include "vlk/utils/utils.h"

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
namespace ui2d {
namespace impl {

namespace {
// `view_area` is relative to the overall surface, this is a strict overlapping check
STX_FORCE_INLINE constexpr bool is_overlapping(Rect const &rect_a, Rect const &rect_b) noexcept {
  uint64_t x1_min = rect_a.offset.x;
  uint64_t x1_max = x1_min + rect_a.extent.width;
  uint64_t y1_min = rect_a.offset.y;
  uint64_t y1_max = y1_min + rect_a.extent.height;
  uint64_t x2_min = rect_b.offset.x;
  uint64_t x2_max = x2_min + rect_b.extent.width;
  uint64_t y2_min = rect_b.offset.y;
  uint64_t y2_max = y2_min + rect_b.extent.height;

  return (x1_max > x2_min && x2_max > x1_min) && (y1_max > y2_min && y2_max > y1_min);
}
}

// This snapshot is not aware of the parent-child dimensional relationship. It
// only has dimensional data required to position the widget on a target
// surface.
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

  // rect's offset represents distance from the whole render surface to this
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

  Rect const &area() const noexcept { return area_; }

  // return an empty snapshot
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
    canvas->clear(colors::Transparent.argb());
    canvas->drawPicture(
        draw_commands_.get());  // SkPaint, and SkMatrix for transform (surface zooming)
    image_ = gpu_surface->makeImageSnapshot();
  }

  void render_cache(SkCanvas &view_canvas, Rect const &view_area) {
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

  // called when the surface extent changes.
  // must be called irregardless of whether this is in the residual state or
  // not. this can trigger a resize event in the widgets.
  template <bool IsResidual>
  void on_surface_area_changed(Rect const &new_widget_area, SurfaceProvider &gpu_surface_provider) {
    Rect previous_area = area_;
    area_ = new_widget_area;

    if constexpr (IsResidual) {
      VLK_DEBUG_ENSURE(!IsResidual, "calling `on_surface_area_changed` on a residual snapshot");
      return;
    }

    // in already rasterized state

    if (previous_area.extent.width == area_.extent.width &&
        previous_area.extent.height == previous_area.extent.height)
      return;

    discard_draw_commands();
    record_draw_commands();

    rasterize(gpu_surface_provider);
  }

  // typically for dispatching events to a widget
  template <bool IsResidual>
  void dispatch_spatial_event() {
    if constexpr (IsResidual) {
      VLK_DEBUG_ENSURE(!IsResidual, "calling `dispatch_spatial_event` on a residual snapshot");
      return;
    }
  }
};

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

template <typename SlWAllocator, typename SfWAllocator, typename SlRAllocator,
          typename SfRAllocator>
inline void build_widget_layout_helper(
    std::vector<std::pair<Widget *, Rect>, SlWAllocator> &stateless_layout_widgets,
    std::vector<std::pair<Widget *, Rect>, SfWAllocator> &stateful_layout_widgets,
    Residuals<SlRAllocator> &stateless_residuals, Residuals<SfRAllocator> &stateful_residuals,
    Widget *widget, Rect const &surface_area, uint32_t z_index) {
  VLK_COMPOSITOR_TRACE_SCOPE;

  if (widget->is_layout_type()) {
    if (widget->is_stateful()) {
      stateful_layout_widgets.emplace_back(widget, surface_area);
    } else {
      stateless_layout_widgets.emplace_back(widget, surface_area);
    }
  } else {
    CacheEntry entry{Snapshot::CreateRecorded(*widget, surface_area),
                     widget->z_index().is_none() ? z_index : widget->z_index().clone().unwrap(), 0};
    if (widget->is_stateful()) {
      stateful_residuals.emplace_back(std::move(entry));
    } else {
      stateless_residuals.emplace_back(std::move(entry));
    }
  }
}

template <typename SlWAllocator, typename SfWAllocator, typename SlRAllocator,
          typename SfRAllocator>
inline void build_widget_layout(
    std::vector<std::pair<Widget *, Rect>, SlWAllocator> &stateless_layout_widgets,
    std::vector<std::pair<Widget *, Rect>, SfWAllocator> &stateful_layout_widgets,
    Residuals<SlRAllocator> &stateless_residuals, Residuals<SfRAllocator> &stateful_residuals,
    Widget *widget, Extent const &allotted_extent, Offset const &allotted_surface_offset,
    uint32_t start_z_index) {
  VLK_DEBUG_ENSURE(widget != nullptr, "Found nullptr Widget");

  // TODO(lamarrr): centralized window zooming

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

  Rect desired_parent_area = widget->compute_area(allotted_extent, children_allotted_area);

#if VLK_ENABLE_DEBUG_CHECKS

  auto const widget_x_max = desired_parent_area.extent.width + desired_parent_area.offset.x;

  auto const widget_y_max = desired_parent_area.extent.height + desired_parent_area.offset.y;

  // TODO: fix message add offset info
  if (!widget->is_layout_type()) {
    VLK_WARN_IF(desired_parent_area.extent.width == u32_max,
                "widget {}'s (type: {}, address: {}) width is u32_max", widget->get_name(),
                widget->get_type_hint(), static_cast<void *>(widget));
    VLK_WARN_IF(desired_parent_area.extent.height == u32_max,
                "widget {}'s (type: {}, address: {}) height is u32_max", widget->get_name(),
                widget->get_type_hint(), static_cast<void *>(widget));

    VLK_WARN_IF(widget_x_max > allotted_extent.width,
                "overflow on x-axis by {}px detected in widget: {} (type: {}, address: {}) >>> "
                "parent allotted width: {}px, widget requested: {}px offset and {}px extent",
                widget_x_max - allotted_extent.width, widget->get_name(), widget->get_type_hint(),
                static_cast<void *>(widget), allotted_extent.width, desired_parent_area.offset.x,
                desired_parent_area.extent.width);
    VLK_WARN_IF(widget_y_max > allotted_extent.height,
                "overflow on y-axis by {}px detected in widget: {} (type: {}, address: {}) >>> "
                "parent allotted height: {}px, widget requested: {}px offset and {}px extent",
                widget_y_max - allotted_extent.height, widget->get_name(), widget->get_type_hint(),
                static_cast<void *>(widget), allotted_extent.height, desired_parent_area.offset.y,
                desired_parent_area.extent.height);
  }
#endif

  Offset widget_parent_offset = {};
  widget_parent_offset.x = std::min(desired_parent_area.offset.x, allotted_extent.width);
  widget_parent_offset.y = std::min(desired_parent_area.offset.y, allotted_extent.height);

  Extent widget_extent{};
  widget_extent.width = std::min(widget_parent_offset.x + desired_parent_area.extent.width,
                                 allotted_extent.width - widget_parent_offset.x);
  widget_extent.height = std::min(widget_parent_offset.y + desired_parent_area.extent.height,
                                  allotted_extent.height - widget_parent_offset.y);

  // is this correct
  Offset widget_surface_offset = allotted_surface_offset + widget_parent_offset;

  // used for actual drawing and positioning
  Rect widget_surface_area = {widget_surface_offset, widget_extent};

  build_widget_layout_helper(stateless_layout_widgets, stateful_layout_widgets, stateless_residuals,
                             stateful_residuals, widget, widget_surface_area, start_z_index);

  for (size_t i = 0; i < num_children; i++) {
    Widget *child = children[i];

    Offset allotted_child_surface_offset = widget_surface_offset + children_allotted_area[i].offset;
    Extent allotted_child_extent = children_allotted_area[i].extent;

    build_widget_layout(stateless_layout_widgets, stateful_layout_widgets, stateless_residuals,
                        stateful_residuals, child, allotted_child_extent,
                        allotted_child_surface_offset,
                        start_z_index + (child->is_layout_type() ? 0u : 1u));
  }
}

}  // namespace

// it is not in charge of deleting the referenced widgets
// implements a time-based least recently used (TLRU) caching behaviour
template <typename CacheEntryAllocator = std::allocator<CacheEntry>,
          typename PointerAllocator = std::allocator<Widget *>>
struct Compositor {
  using cache_type = Cache<CacheEntryAllocator>;
  using residuals_type = Residuals<CacheEntryAllocator>;

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
    Rect surface_area{Offset{0, 0}, surface_extent_};

    VLK_COMPOSITOR_TRACE_SCOPE;
    build_widget_layout(stateless_layout_widgets_, stateful_layout_widgets_, stateless_residuals_,
                        stateful_residuals_, root_widget_, surface_area.extent, surface_area.offset,
                        0);

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
  sk_sp<SkImage> tick(std::chrono::nanoseconds const &interval) {
    VLK_COMPOSITOR_TRACE_SCOPE;

    // TODO(lamarrr): render to surface argument instead of having own surface
    // TODO(lamarrr): all widgets are stateful on first render pass

    for (auto &[layout_widget, surface_area] : stateful_layout_widgets_) {
      // TODO(lamarrr): rebuild widget tree, and loop through all widgets and
      // update their individual areas on the widget tree
      // consider what data will be invalidated and optimize for that
    }

    LRU_resolve<false>(stateless_residuals_, stateless_cache_, *surface_provider_, view_area_,
                       max_out_of_view_ticks_);
    LRU_resolve<true>(stateful_residuals_, stateful_cache_, *surface_provider_, view_area_,
                      max_out_of_view_ticks_);

    SkCanvas *view_canvas = view_surface_->getCanvas();
    view_canvas->clear(colors::Transparent.argb());

    VLK_ENSURE(view_canvas != nullptr);

    for (CacheEntry &entry : stateless_cache_) {
      if (is_overlapping(entry.snapshot.area(), view_area_))
        entry.snapshot.render_cache(*view_canvas, view_area_);
    }

    for (CacheEntry &entry : stateful_cache_) {
      if (is_overlapping(entry.snapshot.area(), view_area_))
        entry.snapshot.render_cache(*view_canvas, view_area_);
    }

    // process on_surface_area_changed events

    VLK_COMPOSITOR_TRACE_SCALAR(stateless_cache_.images_size());
    VLK_COMPOSITOR_TRACE_SCALAR(stateful_cache_.images_size());

    VLK_COMPOSITOR_TRACE_SCALAR(stateless_cache_.size());
    VLK_COMPOSITOR_TRACE_SCALAR(stateful_cache_.size());

    VLK_COMPOSITOR_TRACE_SCALAR(stateless_residuals_.size());
    VLK_COMPOSITOR_TRACE_SCALAR(stateful_residuals_.size());

    VLK_COMPOSITOR_TRACE_SCALAR(stateless_layout_widgets_.size());
    VLK_COMPOSITOR_TRACE_SCALAR(stateful_layout_widgets_.size());

    return view_surface_->makeImageSnapshot();
  }

  auto &get_stateless_cache() noexcept { return stateless_cache_; }
  auto &get_stateful_cache() noexcept { return stateful_cache_; }
  auto &get_stateless_residuals() noexcept { return stateless_residuals_; }
  auto &get_stateful_residuals() noexcept { return stateful_residuals_; }
  auto &get_stateless_layout_widgets() noexcept { return stateless_layout_widgets_; }
  auto &get_stateful_layout_widgets() noexcept { return stateful_layout_widgets_; }

 private:
  SurfaceProvider *surface_provider_;
  sk_sp<SkSurface> view_surface_;
  Rect view_area_;
  Extent surface_extent_;

  cache_type stateless_cache_;  // cache is initialized on the first
                                // render call. when the widget goes out
                                // of view it goes into the residual bin
  cache_type stateful_cache_;   // cache is initialized on the first render
                                // call. the snapshot is updated if the widget
                                // becomes dirty or is moved from the residual
                                // bin to the cache

  residuals_type stateless_residuals_;
  residuals_type stateful_residuals_;

  std::vector<std::pair<Widget *, Rect>> stateless_layout_widgets_;
  std::vector<std::pair<Widget *, Rect>> stateful_layout_widgets_;

  Widget *root_widget_;
  uint64_t max_out_of_view_ticks_;
};

}  // namespace impl
}  // namespace ui2d
}  // namespace vlk
