#pragma once

#include "include/core/SkSurface.h"
#include "vlk/ui/primitives.h"
#include "vlk/utils/utils.h"

namespace vlk {

namespace ui {

// TODO(lamarrr) consider not making surface provider virtual
struct SurfaceProvider {
  virtual sk_sp<SkSurface> make_surface(Extent const& extent) = 0;
  virtual ~SurfaceProvider() {}
};

// TODO(lamarrr): provide surfaces in batches if that could be faster and more
// reasonable
struct GpuSurfaceProvider : public SurfaceProvider {
  GpuSurfaceProvider(GrRecordingContext* context, SkBudgeted budgeted)
      : context_{context}, budgeted_{budgeted} {}
  virtual sk_sp<SkSurface> make_surface(Extent const& extent) override {
    auto surface = SkSurface::MakeRenderTarget(
        context_, budgeted_,
        SkImageInfo::MakeN32Premul(extent.width == 0 ? 1 : extent.width,
                                   extent.height == 0 ? 1 : extent.height));
    VLK_DEBUG_ENSURE(surface != nullptr);
    return surface;
  }

 private:
  GrRecordingContext* context_;
  SkBudgeted budgeted_;
};

struct CpuSurfaceProvider : public SurfaceProvider {
  CpuSurfaceProvider() {}
  virtual sk_sp<SkSurface> make_surface(Extent const& extent) override {
    auto surface = SkSurface::MakeRaster(
        SkImageInfo::MakeN32Premul(extent.width == 0 ? 1 : extent.width,
                                   extent.height == 0 ? 1 : extent.height));
    VLK_DEBUG_ENSURE(surface != nullptr);
    return surface;
  }
};

}  // namespace ui
}  // namespace vlk
