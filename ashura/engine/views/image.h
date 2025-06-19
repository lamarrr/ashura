/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/view.h"
#include "ashura/std/types.h"

namespace ash
{

namespace ui
{

using ImageSrc = Enum<None, ImageId, Future<Result<ImageId, ImageLoadErr>>>;

enum class ImageFit : u8
{
  /// @brief try to contain the image within the frame
  /// without distorting it (preserving aspect ratio)
  Contain = 0,

  /// @brief crop the image to fit within the frame
  Crop = 1,

  /// @brief distort the image to fill the frame
  Fit = 2
};

struct Image : View
{
  struct State
  {
    Enum<None, Option<ash::ImageInfo>, ImageLoadErr> resolved = none;
  } state_;

  struct Style
  {
    Option<f32> aspect_ratio = none;

    Frame frame = Frame{}.abs(250, 250);

    CornerRadii radii = CornerRadii::all(2);

    ColorGradient tint = colors::WHITE;

    ImageFit fit = ImageFit::Contain;

    Vec2 alignment = ALIGNMENT_CENTER_CENTER;
  } style_;

  ImageSrc src_;

  Image(ImageSrc src = None{});

  Image & source(ImageSrc src);

  Image & aspect_ratio(f32 width, f32 height);

  Image & aspect_ratio(Option<f32> ratio);

  Image & frame(Frame frame);

  Image & corner_radii(CornerRadii const & radii);

  Image & tint(ColorGradient const & color);

  Image & fit(ImageFit fit);

  Image & align(Vec2 alignment);

  virtual ui::State tick(Ctx const & ctx, Events const & events,
                         Fn<void(View &)> build) override;

  virtual Layout fit(Vec2 allocated, Span<Vec2 const> sizes,
                     Span<Vec2> centers) override;

  virtual void render(Canvas & canvas, RenderInfo const & info) override;
};

}    // namespace ui

}    // namespace ash
