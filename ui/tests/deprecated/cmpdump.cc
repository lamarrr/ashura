
inline void install_layout_listeners(WidgetLayoutTree &tree,
                                     WidgetLayoutTree::Node &node) {
  // out of viewwidgets can affect overall positioning too
  StateProxyAdapter::install_on_layout_dirty(
      *node.widget, [&tree]() { tree.any_layout_changed = true; });

  for (WidgetLayoutTree::Node &child : node.children) {
    install_layout_listeners(tree, child);
  }
}


inline void install_render_listeners(RenderTree::View &view) {
  size_t i = 0;

  for (WidgetSnapshot &snapshot : view.in_view_snapshots) {
    StateProxyAdapter::install_on_render_dirty(
        *snapshot.widget, [&view, i, widget = snapshot.widget]() {
          view.render_dirtiness[i] = true;
        });

    i++;
  }

  for (auto &child_view : view.in_view_child_views) {
    install_render_listeners(child_view);
  }
}
