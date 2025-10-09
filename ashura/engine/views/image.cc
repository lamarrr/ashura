/// SPDX-License-Identifier: MIT
#include "ashura/engine/views/image.h"
#include "ashura/engine/engine.h"

namespace ash
{

namespace ui
{

Image & Image::source(ImageSrc src)
{
  src_            = std::move(src);
  state_.resolved = none;
  return *this;
}

Image & Image::aspect_ratio(f32 width, f32 height)
{
  style_.aspect_ratio = (width == 0 || height == 0) ? 1 : (width / height);
  return *this;
}

Image & Image::aspect_ratio(Option<f32> ratio)
{
  style_.aspect_ratio = ratio;
  return *this;
}

Image & Image::frame(Frame frame)
{
  style_.frame = frame;
  return *this;
}

Image & Image::corner_radii(CornerRadii const & radii)
{
  style_.radii = radii;
  return *this;
}

Image & Image::tint(ColorGradient const & color)
{
  style_.tint = color;
  return *this;
}

Image & Image::fit(ImageFit fit)
{
  style_.fit = fit;
  return *this;
}

Image & Image::align(f32x2 a)
{
  style_.alignment = a;
  return *this;
}

ui::State Image::tick(Ctx const &, Events const &, Fn<void(View &)>)
{
  state_.resolved.match(
    [&](None) {
      src_.match(
        [&](None) { state_.resolved = Option<ash::ImageInfo>{none}; },
        [&](ImageId id) { state_.resolved = Option{sys->image.get(id)}; },
        [&](Future<Result<ImageId, ImageLoadErr>> & f) {
          f.poll().match(
            [&](Result<ImageId, ImageLoadErr> & r) {
              r.match(
                [&](ImageId id) {
                  state_.resolved = Option{sys->image.get(id)};
                },
                [&](ImageLoadErr err) { state_.resolved = err; });
            },
            [&](Void) { state_.resolved = none; });
        });
    },
    [](auto &) {}, [](auto &) {});

  src_ = none;

  return ui::State{};
}

Layout Image::fit(f32x2 allocated, Span<f32x2 const>, Span<f32x2>)
{
  auto const frame = style_.frame(allocated);

  if (style_.aspect_ratio.is_none())
  {
    return Layout{.extent = frame};
  }

  return Layout{.extent = with_aspect(frame, style_.aspect_ratio.v())};
}

static Tuple<f32x2, f32x2, f32x2> fit_image(f32x2 extent, f32x2 region_extent,
                                            ImageFit fit)
{
  switch (fit)
  {
    case ImageFit::Crop:
    {
      f32x2 const ar        = {extent.x / extent.y, 1};
      f32 const   dst_ar    = region_extent.x / region_extent.y;
      auto const  uv_extent = with_aspect(ar, dst_ar) / ar;
      auto const  space     = (1 - uv_extent) * 0.5F;
      return {region_extent, space, 1 - space};
    }
    case ImageFit::Fit:
    {
      return {
        region_extent, {0, 0},
         {1, 1}
      };
    }
    default:
    case ImageFit::Contain:
    {
      f32 const ar = extent.x / extent.y;
      return {
        with_aspect(region_extent, ar), {0, 0},
          {1, 1}
      };
    }
  }
}

static void render_image(Canvas & canvas, CRect const & region,
                         CRect const & clip, ash::ImageInfo const & img,
                         Image::Style const & style)
{
  auto const [extent, uv0, uv1] =
    fit_image(as_vec2(img.info.extent.xy()), region.extent, style.fit);

  auto const center = space_align(region.extent, extent, style.alignment);

  canvas.rrect({
    .area{region.center + center, extent},
    .corner_radii = style.radii,
    .tint         = style.tint,
    .sampler      = SamplerIndex::LinearClamped,
    .texture      = img.textures[0],
    .uv{uv0,                    uv1   },
    .clip = clip
  });
}

void Image::render(Canvas & canvas, RenderInfo const & info)
{
  state_.resolved.match([&](None) {},
                        [&](Option<ash::ImageInfo> & opt) {
                          opt.match(
                            [&](ash::ImageInfo & img) {
                              render_image(canvas, info.canvas_region,
                                           info.clip, img, style_);
                            },
                            []() {});
                        },
                        [&](ImageLoadErr) {});
}

}    // namespace ui

}    // namespace ash
