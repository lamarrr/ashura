#pragma once

#include "vlk/ui2d/primitives.h"

#include "stx/option.h"

namespace vlk {
namespace ui2d {

struct Border : public TopRightBottomLeft {
  using TopRightBottomLeft::TopRightBottomLeft;
};

struct Padding : public TopRightBottomLeft {
  using TopRightBottomLeft::TopRightBottomLeft;
};

struct Margin : public TopRightBottomLeft {
  using TopRightBottomLeft::TopRightBottomLeft;
};

struct BackgroundImage {
  // needs caching and a resource manager
  assets::desc::Image2D image;
  RelativeCoordinates coordinates;
  Normalized<float> opacity;
};

struct BoxDecoration {
  stx::Option<Color> background_color;
  stx::Option<BackgroundImage> background_image;
};

}  // namespace ui2d
}  // namespace vlk
