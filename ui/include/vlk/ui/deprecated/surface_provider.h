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


  sk_sp<SkImage> get_gpu_surface_texture_read_only_ref(RasterContext& context) {
    VLK_DEBUG_ENSURE(is_surface_init());
    VLK_ENSURE(context.target == RasterContext::Target::Gpu);

    auto render_target = surface_->getBackendRenderTarget(
        SkSurface::BackendHandleAccess::kFlushRead_BackendHandleAccess);


    GrBackendTexture backend_texture = surface_->getBackendTexture(
        SkSurface::BackendHandleAccess::kFlushRead_BackendHandleAccess);
    return SkImage::MakeFromAdoptedTexture(
        context.recording_context.clone().unwrap(), backend_texture,
        context.surface_origin, context.color_type, context.alpha_type,
        context.color_space);
  }
  
}  // namespace ui
}  // namespace vlk
