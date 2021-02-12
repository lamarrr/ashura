#include <algorithm>
#include <functional>
#include <memory>
#include <vector>

#include "vlk/ui/primitives.h"
#include "vlk/ui/widget.h"
#include "vlk/utils/utils.h"

#include "stx/span.h"

namespace vlk {
namespace ui {

enum class ImageFormat : uint8_t {
  Rgba8888 = 0,  //!< pixel with 8 bits for red, green, blue, alpha; in 32-bit
                 //!< word
  Rgbx8888,  //!< pixel with 8 bits each for red, green, blue; in 32-bit word,
  Bgra8888,  //!< pixel with 8 bits for blue, green, red, alpha;
             //!< in 32-bit word
  Rgb565,    //!< pixel with 5 bits red, 6 bits green, 5 bits blue, in 16-bit
             //!< word
  Argb4444,  //!< pixel with 4 bits for alpha, red, green, blue; in 16-bit
             //!< word
  Gray8,     //!< pixel with grayscale level in 8-bit word
  ImageFormatMin = Rgba8888,
  ImageFormatMax = Gray8
};

STX_FORCE_INLINE SkColorType to_sk_type(ImageFormat format) {
  switch (format) {
    case ImageFormat::Rgba8888:
      return kRGBA_8888_SkColorType;
    case ImageFormat::Rgbx8888:
      return kRGB_888x_SkColorType;
    case ImageFormat::Bgra8888:
      return kBGRA_8888_SkColorType;
    case ImageFormat::Rgb565:
      return kRGB_565_SkColorType;
    case ImageFormat::Argb4444:
      return kARGB_4444_SkColorType;
    case ImageFormat::Gray8:
      return kGray_8_SkColorType;
  }
}

STX_FORCE_INLINE uint32_t channel_size(ImageFormat format) noexcept {
  VLK_ENSURE(vlk::enum_ut(format) >=
                 vlk::enum_ut(ImageFormat::ImageFormatMin) &&
             vlk::enum_ut(format) <= vlk::enum_ut(ImageFormat::ImageFormatMax));
  switch (format) {
    case ImageFormat::Rgba8888:
    case ImageFormat::Rgbx8888:
    case ImageFormat::Bgra8888:
      return 4;
    case ImageFormat::Rgb565:
    case ImageFormat::Argb4444:
      return 2;
    case ImageFormat::Gray8:
      return 1;
  }
}

struct ImageInfo {
  Extent extent;
  ImageFormat format;
};

struct ImageData {
  ImageInfo info;
  stx::Span<uint8_t const> pixels;
};

// TODO(lamarrr): handling statefulness.
// TODO(lamarrr): caching policy, how will the caching policy affect the clips
// and others?

// To copy or not to copy?
template <bool IsStateful,
          typename PixelDataAllocator = std::allocator<uint32_t>>
struct Image : public Widget {
  // Note: we can use sizing here because width and height is known ahead of
  // time.
  explicit Image(stx::Span<uint8_t const> const &pixels, ImageInfo const &info,
                 Sizing const &sizing = Sizing::relative())
      : info_{info}, sizing_{sizing} {
    VLK_ENSURE(info.extent.width * info.extent.height *
                   channel_size(info.format) ==
               pixels.size_bytes());
    pixels_.resize(pixels.size_bytes() / sizeof(uint32_t));
    // memory write here is always aligned
    std::copy(pixels.begin(), pixels.end(),
              reinterpret_cast<uint8_t *>(pixels_.begin().base()));
  }

  virtual Type get_type() const noexcept override { return Type::Render; }
  virtual bool is_stateful() const noexcept override { return IsStateful; }
  virtual bool is_dirty() const noexcept override {
    if constexpr (IsStateful)
      return image_updated_;
    else
      return false;
  }

  virtual void mark_clean() noexcept override {}

  virtual std::string_view get_type_hint() const noexcept override {
    return "Image";
  }

  virtual stx::Span<Widget *const> get_children() const noexcept override {
    return {};
  }

  virtual Rect compute_area(
      Extent const &allotted_extent,
      [[maybe_unused]] stx::Span<Rect> const &children_area) override {
    return Rect{Offset{0, 0}, allotted_extent};
  }

  virtual void draw(Canvas &canvas, Extent const &requested_extent) override {
    if (info_.extent.width == 0 || info_.extent.height == 0 || pixels_.empty())
      return;

    SkCanvas *sk_canvas = canvas.as_skia();

    // TODO(lamarrr): does SkPicture keep a reference to the image?
    sk_sp<SkData> sk_data =
        SkData::MakeWithoutCopy(pixels_.data(), pixels_.size());
    sk_sp<SkImage> image = SkImage::MakeRasterData(
        SkImageInfo::Make(info_.extent.width, info_.extent.height,
                          to_sk_type(info_.format),
                          SkAlphaType::kPremul_SkAlphaType),
        sk_data, info_.extent.width * 4);
    sk_canvas->drawImage(image, 0, 0);
  }

  void update_image(stx::Span<uint8_t const> const &pixels) noexcept {
    VLK_ENSURE(IsStateful);
    VLK_ENSURE(pixels.size() == pixels_.size());
    std::copy(pixels.begin(), pixels.end(),
              reinterpret_cast<uint8_t *>(pixels_.begin().base()));
    image_updated_ = true;
  }

 private:
  ImageInfo info_;
  // each image type has alignment requirements. i.e. each RGB565 pixel must be
  // aligned to a 16-bit boundary, same as for RGBA8888 which must be aligned to
  // a 32-bit boundary. uint32_t is suitable for all of these.
  std::vector<uint32_t, PixelDataAllocator> pixels_;
  Sizing sizing_;
  bool image_updated_;
};

struct DeferredImage {};

namespace ops {

struct Fused : public Widget {
  Widget *first;
  Widget *second;

  Widget *operator|(Widget const &) { return nullptr; }
};

struct Blend {
  Widget *b;
};

struct Clip {
  struct Shape {};
};

// TODO(lamarrr): we need a widget inhibitor, the translate widget will now
// take inputs on behalf of the Widget type.
// TODO(lamarrr): i.e. type hint for Opacity widget for Button will be
// "Opacity for Button"
// TODO(lamarr): Concrete structs for ops and then an Effect widget that takes
// the ops as an argument, we'll be able to avoid the virtual function overhead.
// alsom how do we map fusing them to a concrete type? Fuse<Clip, Blend,
// Translate> {   void draw(){  // for each, draw   };       }
// output of one is passed to another
//  Translate | Rotate => Fused<Translate, Rotate, Clip, Draw>
struct Translate {};
struct Rotate {};

};  // namespace ops

}  // namespace ui
};  // namespace vlk
