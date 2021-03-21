

struct ViewChild;
struct RasterNode;
struct View;

// we would need to store a pointer to the widget's node in the widget itself
// the widget might need to consult its parent
struct LinkTree {
  struct Node {
    // points to the view it belongs to
    ViewChild* view_node;

    // points to the widget's parent view (i.e. the view it belongs to)
    View* parent_view;

    // points to the widget's position on the raster list
    RasterNode* raster_node;

    // points to the widget's parent
    Node* parent;
  };
};
