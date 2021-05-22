
#include <fstream>

// clang-format off
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#include "stb_image.h"
// clang-format on

#include "vlk/ui/image_asset.h"

auto vlk::ui::impl::StbiImageBuffer::load_from_file(
    std::filesystem::path const& path, vlk::ui::ImageFormat target_format)
    -> stx::Result<vlk::ui::impl::StbiImageBuffer, vlk::ui::ImageLoadError> {
  if (!std::filesystem::exists(path))
    return stx::Err(vlk::ui::ImageLoadError::InvalidPath);

  std::ifstream stream(path, std::ios_base::ate | std::ios_base::binary);

  auto const file_size = stream.tellg();

  std::unique_ptr<uint8_t[]> encoded_buffer(new uint8_t[file_size]);

  stream.seekg(0);

  stream.read(reinterpret_cast<char*>(encoded_buffer.get()), file_size);

  int width = 0, height = 0, inbuilt_channels = 0,
      target_channels = vlk::ui::impl::unaligned_channel_size(target_format);

  stbi_uc* image_uc = stbi_load_from_memory(
      reinterpret_cast<stbi_uc const*>(encoded_buffer.get()), file_size, &width,
      &height, &inbuilt_channels, target_channels);

  if (image_uc == nullptr) return stx::Err(vlk::ui::ImageLoadError::LoadFailed);

  return stx::Ok(vlk::ui::impl::StbiImageBuffer{
      image_uc, ImageInfo{Extent{static_cast<uint32_t>(width),
                                 static_cast<uint32_t>(height)},
                          target_format}});
}

uint64_t vlk::ui::MemoryImageSource::make_uid() {
  static std::atomic<uint64_t> latest_uid = 0;

  uint64_t expected = 0;
  uint64_t desired = 1;

  do {
    desired = expected + 1;
  } while (!latest_uid.compare_exchange_strong(expected, desired));
}

std::shared_ptr<vlk::ui::AssetLoader const>
vlk::ui::impl::FileImageLoader::get_default() {
  static std::shared_ptr loader = std::make_shared<FileImageLoader const>();
  return loader;
}

std::shared_ptr<vlk::ui::AssetLoader const>
vlk::ui::impl::MemoryImageLoader::get_default() {
  static std::shared_ptr loader = std::make_shared<MemoryImageLoader const>();
  return loader;
}

vlk::ui::impl::StbiImageBuffer::~StbiImageBuffer() {
  if (stbi_image_buffer_ != nullptr) {
    stbi_image_free(stbi_image_buffer_);
  }
}
