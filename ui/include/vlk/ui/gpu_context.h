
#pragma once

#include "include/core/SkSurface.h"

#include "vlk/ui/primitives.h"

namespace vlk {
namespace ui {

struct RasterContext {
  SkBudgeted budgeted = SkBudgeted::kYes;
  SkAlphaType alpha_type = SkAlphaType::kPremul_SkAlphaType;
  SkColorType color_type = SkColorType::kRGBA_8888_SkColorType;

  // optional only if not using gpu
  GrRecordingContext* recording_context = nullptr;

  sk_sp<SkColorSpace> color_space = nullptr;

  GrSurfaceOrigin const surface_origin =
      GrSurfaceOrigin::kTopLeft_GrSurfaceOrigin;

  sk_sp<SkSurface> create_gpu_surface(Extent const& extent) {
    VLK_ENSURE(recording_context != nullptr);
    return SkSurface::MakeRenderTarget(
        recording_context, budgeted,
        SkImageInfo::Make(extent.width, extent.height, color_type, alpha_type,
                          color_space));
  }

  sk_sp<SkSurface> create_cpu_surface(Extent const& extent) {
    return SkSurface::MakeRaster(SkImageInfo::Make(
        extent.width, extent.height, color_type, alpha_type, color_space));
  }

}  // namespace ui
}  // namespace ui
