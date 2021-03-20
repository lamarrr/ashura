#pragma once

#include <utility>

#include "include/core/SkCanvas.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrBackendSurface.h"

#include "vlk/ui/canvas.h"
#include "vlk/ui/primitives.h"
#include "vlk/ui/surface_provider.h"

#include "vlk/utils/utils.h"

namespace vlk {
namespace ui {

// this should cover the whole extent of the widgets. this should be allotted to
// the size of the root view widget. they are only activated when in focus, this
// optimizes for scrolling especially when the content don't really change and
// only their raster content change.
struct RasterCache {
  // a render widget will belong to at least one tile.
  // each render widget will thus need to send a dirtiness notification to at
  // least one tile.
  RasterCache(
      IRect const& cull_rect)  // float pixel_ratio = 1.0f shouldn't be here
      : surface_{nullptr},
        recording_{nullptr},
        is_recording_{false},
        recorder_{},
        cull_rect_{cull_rect} {
    VLK_DEBUG_ENSURE(cull_rect.extent.is_visible());
  }

  // NOTE: copy & move construction/assignment were disabled for
  // SkPictureRecorder

  RasterCache(RasterCache const& other) = delete;
  RasterCache& operator=(RasterCache const&) = delete;

  RasterCache(RasterCache&& other)
      : surface_{std::move(other.surface_)},
        recording_{std::move(other.recording_)},
        is_recording_{false},
        recorder_{},
        cull_rect_{std::move(other.cull_rect_)} {}

  RasterCache& operator=(RasterCache&& other) {
    surface_ = std::move(other.surface_);
    recording_ = std::move(other.recording_);
    auto tmp = std::move(other.is_recording_);
    is_recording_ = false;
    recorder_.finishRecordingAsPicture();
    cull_rect_ = std::move(other.cull_rect_);

    return *this;
  }

  bool is_recording() const { return is_recording_; }

  void begin_recording() {
    VLK_DEBUG_ENSURE(!is_recording());
    is_recording_ = true;
    recorder_.beginRecording(
        SkRect::MakeXYWH(cull_rect_.offset.x, cull_rect_.offset.y,
                         cull_rect_.extent.width, cull_rect_.extent.height));
  }

  void finish_recording() {
    VLK_DEBUG_ENSURE(is_recording());
    is_recording_ = false;
  }

  void discard_recording() { recording_.reset(); }

  bool has_recording() const { return recording_ != nullptr; }

  bool has_surface() const { return surface_ != nullptr; }

  Canvas get_recording_canvas() {
    VLK_DEBUG_ENSURE(is_recording());
    return Canvas::from_skia(recorder_.getRecordingCanvas(), cull_rect_.extent);
  }

  void init_surface(RasterContext& context, SurfaceProvider& surface_provider) {
    // initialize cache with a surface the size of extent
    VLK_DEBUG_ENSURE(cull_rect_.extent.is_visible());
    surface_ = surface_provider.create_surface(context, cull_rect_.extent);
  }

  void deinit_surface() { surface_.reset(); }

  bool is_surface_init() const { return surface_ != nullptr; }

  void rasterize_recording() {
    VLK_DEBUG_ENSURE(is_surface_init());
    SkCanvas* canvas = surface_->getCanvas();
    canvas->clear(SK_ColorTRANSPARENT);
    canvas->drawPicture(recording_);
    surface_->flushAndSubmit(false);
  }

  // obtains a reference to the backend surface's image without copying its
  // content.
  sk_sp<SkImage> get_surface_texture_read_only_ref(RasterContext& context) {
    VLK_DEBUG_ENSURE(is_surface_init());

    GrBackendTexture backend_texture = surface_->getBackendTexture(
        SkSurface::BackendHandleAccess::kFlushRead_BackendHandleAccess);
    return SkImage::MakeFromAdoptedTexture(
        context.recording_context, backend_texture, context.surface_origin,
        context.color_type, context.alpha_type, context.color_space);
  }

 private:
  sk_sp<SkSurface> surface_;

  sk_sp<SkPicture> recording_;

  bool is_recording_;

  SkPictureRecorder recorder_;

  IRect cull_rect_;
};

}  // namespace ui
}  // namespace vlk
