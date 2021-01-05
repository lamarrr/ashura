#pragma once

#include "include/core/SkSurface.h"
#include "vlk/ui2d/primitives.h"

namespace vlk {

namespace ui2d {

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
    return SkSurface::MakeRenderTarget(
        context_, budgeted_,
        SkImageInfo::MakeN32Premul(extent.width, extent.height));
  }

 private:
  GrRecordingContext* context_;
  SkBudgeted budgeted_;
};

struct CpuSurfaceProvider : public SurfaceProvider {
  CpuSurfaceProvider() {}
  virtual sk_sp<SkSurface> make_surface(Extent const& extent) override {
    return SkSurface::MakeRaster(
        SkImageInfo::MakeN32Premul(extent.width, extent.height));
  }
};

}  // namespace ui2d
}  // namespace vlk
