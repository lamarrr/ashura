#pragma once

#include "include/core/SkSurface.h"

#include "vlk/ui/primitives.h"

#include "vlk/ui/gpu_context.h"
#include "vlk/utils/utils.h"

namespace vlk {

namespace ui {

struct SurfaceProvider {
  virtual sk_sp<SkSurface> create_surface(RasterContext& context,
                                          Extent const& extent) {
    VLK_ENSURE(extent.is_visible());
    sk_sp surface = on_create_surface(context, extent);
    VLK_ENSURE(surface != nullptr);
    return surface;
  }

  virtual ~SurfaceProvider() {}

 protected:
  virtual sk_sp<SkSurface> on_create_surface(RasterContext& context,
                                             Extent const& extent) = 0;
};

struct GpuSurfaceProvider : public SurfaceProvider {
  GpuSurfaceProvider() {}

 private:
  virtual sk_sp<SkSurface> on_create_surface(
      RasterContext& context, Extent const& extent) override final {
    return SkSurface::MakeRenderTarget(
        context.recording_context, context.budgeted,
        SkImageInfo::Make(extent.width, extent.height, context.color_type,
                          context.alpha_type, context.color_space));
  }
};

struct CpuSurfaceProvider : public SurfaceProvider {
  CpuSurfaceProvider() {}

 private:
  virtual sk_sp<SkSurface> on_create_surface(
      RasterContext& context, Extent const& extent) override final {
    return SkSurface::MakeRaster(
        SkImageInfo::Make(extent.width, extent.height, context.color_type,
                          context.alpha_type, context.color_space));
  }
};

}  // namespace ui
}  // namespace vlk
