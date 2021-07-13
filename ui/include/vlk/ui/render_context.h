
#pragma once

#include <memory>
#include <utility>

#include "include/core/SkSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "stx/option.h"
#include "vlk/ui/primitives.h"

namespace vlk {
namespace ui {

struct RenderContext {
  RenderContext(stx::Option<sk_sp<GrDirectContext>> direct_context = stx::None,
                SkColorType color_type = SkColorType::kRGBA_8888_SkColorType,
                SkAlphaType alpha_type = SkAlphaType::kPremul_SkAlphaType,
                stx::Option<sk_sp<SkColorSpace>> color_space = stx::None,
                GrSurfaceOrigin surface_origin = kTopLeft_GrSurfaceOrigin)
      : direct_context_{std::move(direct_context)},
        color_type_{color_type},
        alpha_type_{alpha_type},
        color_space_{std::move(color_space)},
        surface_origin_{surface_origin} {}

  VLK_DISABLE_COPY(RenderContext)
  VLK_DEFAULT_MOVE(RenderContext)
  VLK_DEFAULT_DESTRUCTOR(RenderContext)

  sk_sp<SkSurface> create_cpu_surface(Extent const& extent) const {
    return create_cpu_texture(extent, color_type_, alpha_type_, color_space_);
  }

  sk_sp<SkSurface> create_cpu_texture(
      Extent const& extent, SkColorType color_type, SkAlphaType alpha_type,
      stx::Option<sk_sp<SkColorSpace>> color_space = stx::None) const {
    VLK_ENSURE(extent.visible());
    VLK_ENSURE(fits_i32(extent));

    sk_sp surface = SkSurface::MakeRaster(SkImageInfo::Make(
        SkISize{static_cast<int32_t>(extent.width),
                static_cast<int32_t>(extent.height)},
        color_type, alpha_type, std::move(color_space).unwrap_or(nullptr)));

    VLK_ENSURE(surface != nullptr);

    return surface;
  }

  sk_sp<SkSurface> create_target_surface(Extent const& extent) const {
    return create_target_texture(extent);
  }

  // TODO(lamarrr): we can't use just any texture type on GPU. the GPU has to
  // support it

  sk_sp<SkSurface> create_target_texture(Extent const& extent) const {
    if (direct_context_.is_some()) {
      VLK_ENSURE(extent.visible());
      VLK_ENSURE(fits_i32(extent));

      sk_sp surface = SkSurface::MakeRenderTarget(
          direct_context_.value().get(), budgeted_,
          SkImageInfo::Make(SkISize{static_cast<int32_t>(extent.width),
                                    static_cast<int32_t>(extent.height)},
                            color_type_, alpha_type_,
                            color_space_.clone().unwrap_or(nullptr)),
          0, surface_origin_, nullptr);

      VLK_ENSURE(surface != nullptr);

      return surface;
    } else {
      return create_cpu_texture(extent, color_type_, alpha_type_,
                                std::move(color_space_));
    }
  }

  auto get_direct_context() const { return direct_context_.clone(); }

 private:
  stx::Option<sk_sp<GrDirectContext>> direct_context_;
  SkColorType color_type_;
  SkAlphaType alpha_type_;
  stx::Option<sk_sp<SkColorSpace>> color_space_;
  // Only required for graphics backend, Skia's software rasterizer uses
  // kTopLeft_GrSurfaceOrigin
  GrSurfaceOrigin const surface_origin_;
  // TODO(lamarrr): find out what this does
  SkBudgeted budgeted_ = SkBudgeted::kNo;
};

}  // namespace ui
}  // namespace vlk
