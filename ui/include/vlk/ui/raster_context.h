
#pragma once

#include "include/core/SkSurface.h"
#include "stx/option.h"

#include "vlk/ui/primitives.h"

namespace vlk {
namespace ui {

struct RasterContext {
  SkBudgeted budgeted = SkBudgeted::kYes;
  SkAlphaType alpha_type = SkAlphaType::kPremul_SkAlphaType;
  SkColorType color_type = SkColorType::kRGBA_8888_SkColorType;
  sk_sp<SkColorSpace> color_space = nullptr;
  GrSurfaceOrigin const surface_origin =
      GrSurfaceOrigin::kTopLeft_GrSurfaceOrigin;
  enum class Target { Cpu, Gpu } target = Target::Cpu;
  stx::Option<GrRecordingContext*> recording_context = stx::None;

  sk_sp<SkSurface> create_cpu_surface(Extent const& extent) {
    VLK_ENSURE(extent.is_visible());
    sk_sp surface = SkSurface::MakeRaster(SkImageInfo::Make(
        extent.width, extent.height, color_type, alpha_type, color_space));
    VLK_ENSURE(surface != nullptr);
    return surface;
  }

  sk_sp<SkSurface> create_target_surface(Extent const& extent) {
    if (target == Target::Gpu) {
      VLK_ENSURE(extent.is_visible());
      sk_sp surface = SkSurface::MakeRenderTarget(
          recording_context.clone().expect(
              "Using GPU Target but no recording context is set"),
          budgeted,
          SkImageInfo::Make(extent.width, extent.height, color_type, alpha_type,
                            color_space));
      VLK_ENSURE(surface != nullptr);
      return surface;
    } else {
      return create_cpu_surface(extent);
    }
  }
};

}  // namespace ui
}  // namespace vlk
