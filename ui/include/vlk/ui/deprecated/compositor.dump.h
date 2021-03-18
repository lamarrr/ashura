
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




  /*  for (size_t i = 0; i < child_count; i++) {
      WidgetLayoutTree::Node &child = node.children[i];
      ChildLayout const &child_allotted_layout = children_layout[i];

      Offset child_allotted_offset{
          resolve_child_allotted_layout(child_allotted_layout.x,
                                        node.type == Widget::Type::View
                                            ? view_child_allotted_width
                                            : allotted_extent.width),
          resolve_child_allotted_layout(child_allotted_layout.y,
                                        node.type == Widget::Type::View
                                            ? view_child_allotted_height
                                            : allotted_extent.height)};

      Extent child_allotted_extent{
          resolve_child_allotted_layout(child_allotted_layout.width,
                                        node.type == Widget::Type::View
                                            ? view_child_allotted_width
                                            : allotted_extent.width),
          resolve_child_allotted_layout(child_allotted_layout.height,
                                        node.type == Widget::Type::View
                                            ? view_child_allotted_height
                                            : allotted_extent.height)};

      clean_layout_tree_(child, child_allotted_offset, child_allotted_extent,
                         node.type == Widget::Type::View
                             ? child_allotted_offset
                             : (allotted_view_offset + child_allotted_offset));
    }


    Extent self_extent{};



    node.parent_offset = allotted_parent_offset;
    node.parent_view_area = Rect{Offset{allotted_view_offset}, self_extent};

      node.view_extent = {};

    if (node.type == Widget::Type::View) {
      ViewOffset view_offset = node.widget->get_view_offset();
      node.view_offset.x =
          resolve_view_offset(view_offset.x, node.view_extent.width);
      node.view_offset.y =
          resolve_view_offset(view_offset.y, node.view_extent.height);
    } else {
      node.view_offset = {};
    }
  */