#pragma once

#include <fstream>
#include <utility>

#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkSurface.h"
#include "vlk/ui/canvas.h"
#include "vlk/ui/primitives.h"
#include "vlk/ui/render_context.h"
#include "vlk/utils/utils.h"

namespace vlk {
namespace ui {
// how do we re-record the raster tile content? without allocating extra memory
// too frequently
// what if we store chunks in each tile?

// raster cache, even view widgets are added here, all widgets are layout
// widgets, we thus don't reed a separate view on the render tree

struct RasterCache {
  // a render widget will belong to at least one tile.
  // each render widget will thus need to send a dirtiness notification to at
  // least one tile.
  explicit RasterCache(IRect const& cull_rect)
      : surface_{nullptr},
        picture_{nullptr},
        is_recording_{false},
        recorder_{},
        cull_rect_{cull_rect} {
    VLK_ENSURE(cull_rect.visible());
  }

  explicit RasterCache(Extent const& extent)
      : RasterCache{IRect{IOffset{0, 0}, extent}} {}

  // NOTE: copy & move constructor/assignment were disabled for
  // SkPictureRecorder, so the ones here are basically workarounds

  RasterCache(RasterCache const& other) = delete;
  RasterCache& operator=(RasterCache const&) = delete;

  RasterCache(RasterCache&& other)
      : is_recording_{false},
        recorder_{},
        cull_rect_{std::move(other.cull_rect_)} {
    if (other.is_recording()) {
      other.finish_recording();
    }
    surface_ = std::move(other.surface_);
    picture_ = std::move(other.picture_);
  }

  RasterCache& operator=(RasterCache&& other) {
    if (is_recording()) {
      finish_recording();
    }
    if (other.is_recording()) {
      other.finish_recording();
    }
    surface_ = std::move(other.surface_);
    picture_ = std::move(other.picture_);
    [[maybe_unused]] bool tmp = std::move(other.is_recording_);

    cull_rect_ = std::move(other.cull_rect_);

    return *this;
  }

  bool has_recording() const { return picture_ != nullptr; }

  bool has_surface() const { return surface_ != nullptr; }

  bool is_recording() const { return is_recording_; }

  void begin_recording() {
    VLK_ENSURE(!is_recording());
    is_recording_ = true;
    recorder_.beginRecording(SkRect::MakeXYWH(cull_rect_.x(), cull_rect_.y(),
                                              cull_rect_.width(),
                                              cull_rect_.height()));

    Canvas canvas = get_recording_canvas();
    canvas.to_skia().clear(SK_ColorWHITE);
  }

  void finish_recording() {
    VLK_ENSURE(is_recording());
    is_recording_ = false;
    picture_ = recorder_.finishRecordingAsPicture();
  }

  void discard_recording() { picture_.reset(); }

  Canvas get_recording_canvas() {
    VLK_ENSURE(is_recording());
    SkCanvas* recording_canvas = recorder_.getRecordingCanvas();
    VLK_ENSURE(recording_canvas != nullptr);
    return Canvas::from_skia(*recording_canvas, cull_rect_.extent);
  }

  void init_surface(RenderContext const& context) {
    // initialize cache with a surface the size of extent
    VLK_ENSURE(cull_rect_.visible());
    surface_ = context.create_target_surface(cull_rect_.extent);
  }

  SkSurface& get_surface_ref() {
    VLK_ENSURE(is_surface_init());
    return *surface_;
  }

  void deinit_surface() { surface_.reset(); }

  bool is_surface_init() const { return surface_ != nullptr; }

  void rasterize() {
    VLK_ENSURE(is_surface_init());
    SkCanvas* canvas = surface_->getCanvas();
    canvas->drawPicture(picture_);
    surface_->flushAndSubmit(false);
  }

  void write_to(SkCanvas& canvas, IOffset const& offset) {
    VLK_ENSURE(is_surface_init());
    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc);
    surface_->draw(&canvas, static_cast<int>(offset.x),
                   static_cast<int>(offset.y), &paint);
  }

  size_t storage_size() const {
    if (surface_ == nullptr) return 0;
    return surface_->imageInfo().computeByteSize(
        surface_->imageInfo().minRowBytes());
  }

  // provides re-using the cache surface. NOTE: it doesn't discard its surface
  // nor recording
  void recycle(IOffset new_cull_offset) {
    VLK_ENSURE(!is_recording());
    cull_rect_.offset = new_cull_offset;
  }

  void save_pixels_to_file(std::string const& path) {
    VLK_ENSURE(is_surface_init());

    std::ofstream file(path, std::ios_base::out);
    sk_sp<SkImage> image = surface_->makeImageSnapshot();

    auto const& image_info = image->imageInfo();
    int const width = image_info.width();
    int const height = image_info.height();

    std::vector<uint8_t> buff;
    buff.resize(height * width * 4);

    image->readPixels(
        SkImageInfo::Make(width, height, SkColorType::kRGBA_8888_SkColorType,
                          SkAlphaType::kUnpremul_SkAlphaType),
        buff.data(), width * 4, 0, 0);

    for (uint8_t c : buff) {
      file << static_cast<int>(c) << ", ";
    }
    file.flush();
  }

 private:
  sk_sp<SkSurface> surface_;

  sk_sp<SkPicture> picture_;

  bool is_recording_;

  SkPictureRecorder recorder_;

  IRect cull_rect_;
};

}  // namespace ui
}  // namespace vlk
