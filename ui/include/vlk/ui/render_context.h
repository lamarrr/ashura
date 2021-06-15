
#pragma once

#include "include/core/SkSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "stx/option.h"
#include "vlk/ui/primitives.h"

namespace vlk {
namespace ui {

enum class RasterTarget : uint8_t { Cpu, Gpu };

struct RenderContext {
  RenderContext(RasterTarget target = RasterTarget::Cpu,
                stx::Option<std::shared_ptr<GrRecordingContext>>&&
                    recording_context = stx::None,
                SkColorType color_type = SkColorType::kRGBA_8888_SkColorType,
                SkAlphaType alpha_type = SkAlphaType::kPremul_SkAlphaType,
                stx::Option<sk_sp<SkColorSpace>>&& color_space = stx::None,
                GrSurfaceOrigin surface_origin = kTopLeft_GrSurfaceOrigin,
                SkBudgeted budgeted = SkBudgeted::kYes)
      : target_{target},
        recording_context_{std::move(recording_context)},
        color_type_{color_type},
        alpha_type_{alpha_type},
        color_space_{std::move(color_space)},
        surface_origin_{surface_origin},
        budgeted_{budgeted} {
    VLK_ENSURE(!(target_ != RasterTarget::Cpu && recording_context_.is_none()),
               "Using GPU Target but no recording context is set");
  }

  sk_sp<SkSurface> create_cpu_surface(Extent const& extent) const {
    VLK_ENSURE(extent.visible());
    sk_sp surface = SkSurface::MakeRaster(
        SkImageInfo::Make(extent.width, extent.height, color_type_, alpha_type_,
                          color_space_.clone().unwrap_or(nullptr)));
    VLK_ENSURE(surface != nullptr);
    return surface;
  }

  sk_sp<SkSurface> create_target_surface(Extent const& extent) const {
    return create_target_texture(extent, color_type_, alpha_type_);
  }

  sk_sp<SkSurface> create_target_texture(
      Extent const& extent, SkColorType required_color_type,
      SkAlphaType required_alpha_type) const {
    if (target_ == RasterTarget::Gpu) {
      VLK_ENSURE(extent.visible());
      sk_sp surface = SkSurface::MakeRenderTarget(
          recording_context_.as_cref().unwrap().get().get(), budgeted_,
          SkImageInfo::Make(extent.width, extent.height, required_color_type,
                            required_alpha_type,
                            color_space_.clone().unwrap_or(nullptr)));
      VLK_ENSURE(surface != nullptr);
      return surface;
    } else {
      return create_cpu_surface(extent);
    }
  }

 private:
  RasterTarget target_;
  stx::Option<std::shared_ptr<GrRecordingContext>> recording_context_;
  SkColorType color_type_;
  SkAlphaType alpha_type_;
  stx::Option<sk_sp<SkColorSpace>> color_space_;
  GrSurfaceOrigin const surface_origin_;
  SkBudgeted budgeted_ = SkBudgeted::kYes;
};

}  // namespace ui
}  // namespace vlk
