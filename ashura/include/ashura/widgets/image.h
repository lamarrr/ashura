#pragma once

#include <string_view>
#include <utility>
#include <variant>

#include "ashura/primitives.h"
#include "ashura/vulkan.h"
#include "ashura/widget.h"
#include "stx/span.h"
#include "stx/string.h"

namespace ash {

// RGBA image
struct MemoryImageSource {
  stx::Span<u8 const> pixels;
  extent extent;
};

struct FileImageSource {
  stx::String path;
};

struct NetworkImageSource {
  stx::String uri;
};

using ImageSource =
    std::variant<MemoryImageSource, FileImageSource, NetworkImageSource>;

// alt

struct ImageProps {
  ImageSource source;
  constraint width;
  constraint height;
  vec4 border_radius;
  stx::Option<extent> aspect_ratio;
};

enum class ImageState : u8 {
  /// the image has not been in view yet
  Inactive,

  /// the image is loading
  Loading,

  /// a failure occured while loading the image
  LoadFailed,

  /// the image has been successfully loaded
  Loaded
};

/// Usage Needs
///
/// - Add image to asset bundle and upload to GPU for fast transfers (i.e. zero
/// copy over PCIE from CPU to GPU during rendering)
/// - once the image arrives, get a reference to it
/// - Update widget state to show that the image is loading
///
struct Image : public Widget {
  explicit Image(ImageProps image_props) : props{std::move(image_props)} {}

  //   virtual Extent trim(Extent extent) override {
  //     return aspect_ratio_trim(
  //         storage_.props.aspect_ratio().expect(
  //             "Image::trim() called without an aspect ratio set"),
  //         extent);
  //   }

  virtual ash::layout layout(rect area);

  virtual simdjson::dom::element save(simdjson::dom::parser& parser);

  virtual void restore(simdjson::dom::element const& element);

  ImageProps props;
  ImageState state = ImageState::Inactive;
  stx::Option<stx::Rc<vk::ImageResource*>> image;
};

}  // namespace ash
