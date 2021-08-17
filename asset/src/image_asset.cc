
#include "vlk/image_asset.h"

#include "include/core/SkImage.h"
#include "vlk/utils.h"

namespace vlk {

ImageAsset::ImageAsset(sk_sp<SkImage> raw_image) : raw_{std::move(raw_image)} {
  VLK_ENSURE(raw_ != nullptr);
  Asset::update_size(raw_->imageInfo().computeMinByteSize());
}
}  // namespace vlk

/*
    : load_result_{std::move(load_result)} {
  load_result_.as_cref().match(
      [&](sk_sp<SkImage> const& image) {

      },
      [&](ImageLoadError) { Asset::update_size(0); });
}



}


#include <fstream>

#include "stx/span.h"

// clang-format off
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#include "stb_image.h"
// clang-format on

#include "vlk/ui/image_asset.h"
#include "include/core/SkPaint.h"
#include "include/core/SkCanvas.h"

namespace vlk {
namespace ui {

inline ImageFormat stbi_channels_to_format(int channels) {
switch (channels) {
  case 1:
    return ImageFormat::Gray;
  case 3:
    return ImageFormat::RGB;
  case 4:
    return ImageFormat::RGBA;
  default:
    VLK_PANIC("Unsupported image channel size", channels);
}
}

namespace impl {

static constexpr SkColorType to_skia(ImageFormat format) {
switch (format) {
  case ImageFormat::RGB:
    return SkColorType::kRGB_888x_SkColorType;
  case ImageFormat::RGBA:
    return SkColorType::kRGBA_8888_SkColorType;
  case ImageFormat::Gray:
    return SkColorType::kGray_8_SkColorType;
  default:
    return SkColorType::kUnknown_SkColorType;
}
}

static inline SkImageInfo to_skia(ImageInfo const& info) {
return SkImageInfo::Make(info.extent.width, info.extent.height,
                         to_skia(info.format),
                         SkAlphaType::kUnpremul_SkAlphaType);
}

constexpr uint8_t aligned_channel_size(ImageFormat format) {
switch (format) {
  case ImageFormat::RGB:
    return 4;
  case ImageFormat::RGBA:
    return 4;
  case ImageFormat::Gray:
    return 1;
  default:
    return 0;
}
}

inline std::unique_ptr<uint32_t[]> make_aligned_RGBx_buffer(
  uint8_t const* buffer, uint64_t width, uint64_t height) {
std::unique_ptr<uint32_t[]> aligned_memory(new uint32_t[height * width]);

for (uint64_t pack_index = 0, i = 0; i < height * width * 3;
     pack_index++, i += 3) {
  uint32_t const r = buffer[i];
  uint32_t const g = buffer[i + 1];
  uint32_t const b = buffer[i + 2];
  uint8_t* dest_pack =
      reinterpret_cast<uint8_t*>(&aligned_memory[pack_index]);
  dest_pack[0] = r;
  dest_pack[1] = g;
  dest_pack[2] = b;
  dest_pack[3] = 0xFF;
}
return aligned_memory;
}

inline std::unique_ptr<uint32_t[]> make_aligned_RGBA_buffer(
  uint8_t const* buffer, uint64_t width, uint64_t height) {
std::unique_ptr<uint32_t[]> aligned_memory(new uint32_t[height * width]);

for (uint64_t pack_index = 0, i = 0; i < height * width * 4;
     pack_index++, i += 4) {
  uint32_t const r = buffer[i];
  uint32_t const g = buffer[i + 1];
  uint32_t const b = buffer[i + 2];
  uint32_t const a = buffer[i + 3];
  uint8_t* dest_pack =
      reinterpret_cast<uint8_t*>(&aligned_memory[pack_index]);
  dest_pack[0] = r;
  dest_pack[1] = g;
  dest_pack[2] = b;
  dest_pack[3] = a;
}
return aligned_memory;
}

struct StbiImageBuffer {
explicit StbiImageBuffer() {}

StbiImageBuffer(u_char* stbi_buffer, ImageInfo image_info)
    : stbi_image_buffer_{stbi_buffer}, info_{image_info} {
  VLK_ENSURE(stbi_buffer != nullptr);
}

stx::Span<uint8_t const> span() const {
  return stx::Span<u_char const>(stbi_image_buffer_,
                                 info_.extent.width * info_.extent.height *
                                     unaligned_channel_size(info_.format))
      .as_u8();
}

static auto load_from_file(std::filesystem::path const& path,
                           stx::Option<ImageFormat> target_format)
    -> stx::Result<StbiImageBuffer, ImageLoadError> {
  if (!(std::filesystem::exists(path) &&
        std::filesystem::is_regular_file(path)))
    return stx::Err(ImageLoadError::InvalidPath);

  std::ifstream stream(path, std::ios_base::ate | std::ios_base::binary);

  auto const file_size = stream.tellg();

  std::unique_ptr<uint8_t[]> encoded_buffer(new uint8_t[file_size]);

  stream.seekg(0);

  stream.read(reinterpret_cast<char*>(encoded_buffer.get()), file_size);

  int width = 0, height = 0, inbuilt_channels = 0,
      target_channels =
          target_format.as_cref().map_or(unaligned_channel_size, 0);

  stbi_uc* image_uc = stbi_load_from_memory(
      reinterpret_cast<stbi_uc const*>(encoded_buffer.get()), file_size,
      &width, &height, &inbuilt_channels, target_channels);

  if (image_uc == nullptr) return stx::Err(ImageLoadError::LoadFailed);

  ImageFormat result_format = target_format.is_none()
                                  ? stbi_channels_to_format(inbuilt_channels)
                                  : stbi_channels_to_format(target_channels);

  return stx::Ok(StbiImageBuffer{
      image_uc, ImageInfo{Extent{static_cast<uint32_t>(width),
                                 static_cast<uint32_t>(height)},
                          result_format}});
}

StbiImageBuffer(StbiImageBuffer const&) = delete;
StbiImageBuffer& operator=(StbiImageBuffer const&) = delete;

StbiImageBuffer(StbiImageBuffer&& other) {
  stbi_image_buffer_ = other.stbi_image_buffer_;
  other.stbi_image_buffer_ = nullptr;
  info_ = other.info_;
  other.info_ = ImageInfo{};
}

StbiImageBuffer& operator=(StbiImageBuffer&& other) {
  std::swap(stbi_image_buffer_, other.stbi_image_buffer_);
  std::swap(info_, other.info_);
  return *this;
}

~StbiImageBuffer() {
  if (stbi_image_buffer_ != nullptr) {
    stbi_image_free(stbi_image_buffer_);
  }
}

ImageInfo info() const { return info_; }

private:
u_char* stbi_image_buffer_ = nullptr;
ImageInfo info_;
};

// bufffer memory is aligned for optimal use
// in the skia API (8-bit alignment for single channel images i.e. Grey. and
// 32-bit alignment for multi-channel images i.e. RGB and RGBA).
// a new buffer establishes alignment requirement for the image if necessary
static inline sk_sp<SkImage> make_sk_image(
  ImageInfo const& info, stx::Span<uint8_t const> unaligned_pixels) {
switch (info.format) {
  case ImageFormat::Gray: {
    sk_sp data = SkData::MakeWithCopy(unaligned_pixels.data(),
                                      unaligned_pixels.size_bytes());
    sk_sp image =
        SkImage::MakeRasterData(to_skia(info), data, info.extent.width * 1);
    VLK_ENSURE(image != nullptr);
    return image;
  };
  case ImageFormat::RGB: {
    auto aligned_buffer = make_aligned_RGBx_buffer(
        unaligned_pixels.data(), info.extent.width, info.extent.height);
    sk_sp data = SkData::MakeWithCopy(aligned_buffer.get(),
                                      info.extent.width * info.extent.height *
                                          aligned_channel_size(info.format));
    sk_sp image =
        SkImage::MakeRasterData(to_skia(info), data, info.extent.width * 4);
    VLK_ENSURE(image != nullptr);
    return image;
  } break;
  case ImageFormat::RGBA: {
    auto aligned_buffer = make_aligned_RGBA_buffer(
        unaligned_pixels.data(), info.extent.width, info.extent.height);
    sk_sp data = SkData::MakeWithCopy(aligned_buffer.get(),
                                      info.extent.width * info.extent.height *
                                          aligned_channel_size(info.format));
    sk_sp image =
        SkImage::MakeRasterData(to_skia(info), data, info.extent.width * 4);
    VLK_ENSURE(image != nullptr);
    return image;
  } break;

  default:
    VLK_PANIC("Unsupported Image Format", info.format);
}
}

// remove pixmap and replace with this function instead
static inline sk_sp<SkImage> make_sk_image(
  StbiImageBuffer const& unaligned_buffer) {
return make_sk_image(unaligned_buffer.info(), unaligned_buffer.span());
}

std::unique_ptr<Asset> FileImageLoader::load(AssetLoadArgs const& args) const {
auto const& load_args =
    upcast<impl::FileImageLoadArgs const>(args).expect("Upcast failed").get();

std::shared_ptr source_data = load_args.source_data();

stx::Result load_result = StbiImageBuffer::load_from_file(
    source_data->path, source_data->target_format);

if (load_result.is_err())
  return std::make_unique<ImageAsset>(
      ImageAsset{stx::Err(std::move(load_result).unwrap_err())});

return std::make_unique<ImageAsset>(
    ImageAsset{stx::Ok(make_sk_image(load_result.value()))});
}

std::shared_ptr<AssetLoader const> FileImageLoader::get_default() {
static std::shared_ptr loader = std::make_shared<FileImageLoader const>();
return loader;
}

std::unique_ptr<Asset> MemoryImageLoader::load(
  AssetLoadArgs const& args) const {
auto const& load_args =
    upcast<impl::MemoryImageLoadArgs const>(args).expect("Upcast failed").get();

std::shared_ptr source_data = load_args.source_data();

return std::make_unique<ImageAsset>(ImageAsset{
    stx::Ok(make_sk_image(source_data->info, source_data->buffer))});
}

std::shared_ptr<AssetLoader const> MemoryImageLoader::get_default() {
static std::shared_ptr loader = std::make_shared<MemoryImageLoader const>();
return loader;
}

}  // namespace impl

uint64_t MemoryImageSource::make_uid() {
static std::atomic<uint64_t> latest_uid = 0;

return latest_uid.fetch_add(1, std::memory_order_seq_cst);
}

// TODO(lamarrr): WEBP support

}  // namespace ui
}  // namespace vlk

*/