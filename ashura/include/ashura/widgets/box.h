#pragma once

#include "ashura/palletes.h"
#include "ashura/primitives.h"
#include "ashura/widget.h"
#include "ashura/widgets/image.h"

namespace ash
{

struct BoxProps
{
  color                    background_color;
  EdgeInsets               padding;
  f32                      border_thickness = 0;
  color                    border_color;
  vec4                     border_radius;
  stx::Option<ImageSource> background_image;
};

struct Box : public Widget
{
  template <WidgetImpl DerivedWidget>
  Box(BoxProps, DerivedWidget);

  stx::Vec<Widget *> children;
};

};        // namespace ash