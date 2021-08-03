

#include <fstream>
#include <utility>
#include <vector>

#include "vlk/assets/image.h"
#include "vlk/utils.h"

// clang-format off
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#include "stb_image.h"
// clang-format on

using namespace vlk;

stx::Result<data::Image, data::Error> data::Image::load(
    desc::Image const& desc) {
  if (!std::filesystem::exists(desc.path))
    return stx::Err(data::Error::InvalidPath);

  std::ifstream stream(desc.path, std::ios_base::ate | std::ios_base::binary);

  auto size = stream.tellg();

  std::vector<char> buffer;
  buffer.resize(size);

  stream.seekg(0);
  stream.read(buffer.data(), buffer.size());

  int width, height, inbuilt_channels;

  int target_channels = 0;

  auto const format = desc.target_format;

  switch (format) {
    case desc::Image::Format::Internal:
      target_channels = 0;
      break;
    case desc::Image::Format::Grey:
      target_channels = 1;
      break;
    case desc::Image::Format::GreyAlpha:
      target_channels = 2;
      break;
    case desc::Image::Format::RGB:
      target_channels = 3;
      break;
    case desc::Image::Format::RGBA:
      target_channels = 4;
      break;
    default:
      VLK_ENSURE(false, "Unrecognized Image Format");
  }

  stbi_set_flip_vertically_on_load(desc.flip_vertically);

  auto* image_uc = stbi_load_from_memory(
      reinterpret_cast<stbi_uc const*>(buffer.data()), buffer.size(), &width,
      &height, &inbuilt_channels, target_channels);

  if (image_uc == nullptr) return stx::Err(data::Error::Internal);

  int resulting_channels = 0;
  if (target_channels == 0) {
    resulting_channels = inbuilt_channels;
  } else {
    resulting_channels = target_channels;
  }

  data::Image image{};
  image.pixel_data_ = image_uc;
  image.format_ = static_cast<data::Image::Format>(resulting_channels);
  image.width_ = width;
  image.height_ = height;

  return stx::Ok(std::move(image));
}

data::Image::Image(data::Image&& other) noexcept
    : pixel_data_{other.pixel_data_},
      width_{other.width_},
      height_{other.height_},
      format_{other.format_} {
  other.pixel_data_ = nullptr;
  other.width_ = 0;
  other.height_ = 0;
}

data::Image& data::Image::operator=(data::Image&& other) noexcept {
  auto deleter = std::move(*this);

  format_ = other.format_;
  width_ = other.width_;
  height_ = other.height_;
  pixel_data_ = other.pixel_data_;

  other.width_ = 0;
  other.height_ = 0;
  other.pixel_data_ = nullptr;

  return *this;
}

data::Image::~Image() {
  // the image is allocated by stb library using malloc
  if (pixel_data_ != nullptr) free(pixel_data_);
}
