#pragma once

#include <fstream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkSurface.h"
#include "vlk/ui/canvas.h"
#include "vlk/ui/primitives.h"
#include "vlk/ui/render_context.h"
#include "vlk/ui/sk_utils.h"
#include "vlk/utils/utils.h"

namespace vlk {
namespace ui {

// how do we re-record the raster tile content? without allocating extra
// memory too frequently what if we store chunks in each tile?

// raster cache, even view widgets are added here, all widgets are layout
// widgets, we thus don't reed a separate view on the render tree

struct RasterRecord {
  struct RecordSessionInfo {
    SkRect sk_logical_recording_cull_rect{};
  };

  VLK_DEFAULT_CONSTRUCTOR(RasterRecord)
  VLK_DISABLE_COPY(RasterRecord)
  VLK_DEFAULT_MOVE(RasterRecord)

  bool has_recording() const { return picture_ != nullptr; }

  bool is_recording() const { return is_recording_; }

  //! NOTE: screen coordinates are logical.
  //! recording is done in logical coordinates and rasterization in physical
  //! coordinates.
  void begin_recording(VRect virtual_logical_cull_rect) {
    VLK_ENSURE(!is_recording());

    is_recording_ = true;

    session_info_ = RecordSessionInfo{to_sk_rect(virtual_logical_cull_rect)};

    if (recorder_ == nullptr) {
      recorder_ = std::unique_ptr<SkPictureRecorder>{new SkPictureRecorder{}};
    }

    recorder_->beginRecording(session_info_.sk_logical_recording_cull_rect);
  }

  void finish_recording() {
    VLK_ENSURE(is_recording());
    VLK_ENSURE(recorder_ != nullptr);

    is_recording_ = false;

    picture_ = recorder_->finishRecordingAsPicture();
  }

  void discard() { picture_ = nullptr; }

  SkCanvas& get_recording_canvas() {
    VLK_ENSURE(is_recording());

    SkCanvas* recording_canvas = recorder_->getRecordingCanvas();

    VLK_ENSURE(recording_canvas != nullptr);

    return *recording_canvas;
  }

  SkPicture const& get_recording() const {
    VLK_ENSURE(picture_ != nullptr);
    return *picture_;
  }

 private:
  sk_sp<SkPicture> picture_;

  // apparently, skia doesn't allow move construction and assignment for this
  // type, so we defer initialize it
  std::unique_ptr<SkPictureRecorder> recorder_;

  RecordSessionInfo session_info_;

  bool is_recording_ = false;
};

struct RasterCache {
  // a render widget will belong to at least one tile.
  // each render widget will thus need to send a dirtiness notification to at
  // least one tile.

  VLK_DEFAULT_CONSTRUCTOR(RasterCache)
  VLK_DISABLE_COPY(RasterCache)
  VLK_DEFAULT_MOVE(RasterCache)

  ~RasterCache() = default;

  void init_surface(RenderContext const& context, Extent physical_extent) {
    VLK_ENSURE(physical_extent.visible());
    surface_ = context.create_target_surface(physical_extent);
    physical_extent_ = physical_extent;
  }

  bool is_surface_init() const { return surface_ != nullptr; }

  void deinit_surface() { surface_ = nullptr; }

  SkSurface& get_surface_ref() {
    VLK_ENSURE(is_surface_init());
    return *surface_;
  }

  void rasterize(Dpr target_device_pixel_ratio, RasterRecord const& record) {
    VLK_ENSURE(is_surface_init());
    // first await any pending rendering operation by performing GPU-CPU
    // synchronization
    // we can't await completion of a task if it is not submitted
    if (submitted_work_) {
      surface_->flushAndSubmit(true);
      submitted_work_ = false;
    }
    SkCanvas* canvas = surface_->getCanvas();
    canvas->clear(SK_ColorTRANSPARENT);

    // backup transform matrix and clip state
    canvas->save();
    canvas->scale(target_device_pixel_ratio.x, target_device_pixel_ratio.y);

    canvas->drawPicture(&record.get_recording());

    // restore transform matrix and clip state
    canvas->restore();

    // NOTE: GPU-CPU synchronization is not performed
    surface_->flushAndSubmit(false);
    submitted_work_ = true;
  }

  void write_to(SkCanvas& canvas, IOffset const& offset) {
    VLK_ENSURE(is_surface_init());
    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc);
    surface_->draw(&canvas, static_cast<float>(offset.x),
                   static_cast<float>(offset.y), &paint);
  }

  size_t surface_size() const {
    if (surface_ == nullptr) return 0;
    return surface_->imageInfo().computeByteSize(
        surface_->imageInfo().minRowBytes());
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
  Extent physical_extent_;
  sk_sp<SkSurface> surface_;
  bool submitted_work_ = false;
};

}  // namespace ui
}  // namespace vlk
