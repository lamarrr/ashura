
#include <cinttypes>

struct Mat;
struct ClipPath;
struct Metadata;

// - these properties are needed as part of the core widget type because we need
// to apply them to the childrena and they affect the children
struct XWidget {
  Mat* transform;  // rotation, translation, ortho
  Mat* path;
  float opacity;
  int64_t zindex;
  ClipPath* clip_path;
  Mat* clip;
  float blur;
  Metadata* metadata;
  XWidget* children;
};