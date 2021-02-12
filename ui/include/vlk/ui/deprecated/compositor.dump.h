
/*
  // called when the surface extent changes.
  // must be called irregardless of whether this is in the residual state or
  // not. this can trigger a resize event in the widgets.

*/

/*    VLK_COMPOSITOR_TRACE_SCALAR(stateless_cache_.images_size());
    VLK_COMPOSITOR_TRACE_SCALAR(stateful_cache_.images_size());

    VLK_COMPOSITOR_TRACE_SCALAR(stateless_cache_.size());
    VLK_COMPOSITOR_TRACE_SCALAR(stateful_cache_.size());

    VLK_COMPOSITOR_TRACE_SCALAR(stateless_residuals_.size());
    VLK_COMPOSITOR_TRACE_SCALAR(stateful_residuals_.size());

    VLK_COMPOSITOR_TRACE_SCALAR(stateless_layout_widgets_.size());
    VLK_COMPOSITOR_TRACE_SCALAR(stateful_layout_widgets_.size());
*/

struct Snapshot {
  template <bool IsResidual>
  void on_view_resized(Rect const &new_widget_area,
                       SurfaceProvider &gpu_surface_provider) {

                         // Optimizing resizing
                         // - if widget's size doesn't change then there's no need to dispose the canvas.
    Rect previous_area = area_;
    area_ = new_widget_area;

    if constexpr (IsResidual) {
      VLK_DEBUG_ENSURE(!IsResidual, "calling `on_surface_area_changed` on a
  residual snapshot"); return;
    }

    // in already rasterized state

widget->compute_area();

    if (previous_area.extent.width == area_.extent.width &&
        previous_area.extent.height == previous_area.extent.height)
      return;

    discard_draw_commands();
    record_draw_commands();

    rasterize(gpu_surface_provider);
  }

  // typically for dispatching events to a widget
  template <bool IsResidual>
  void dispatch_spatial_event() {
    if constexpr (IsResidual) {
      VLK_DEBUG_ENSURE(!IsResidual, "calling `dispatch_spatial_event` on a
  residual snapshot"); return;
    }
  }
};
