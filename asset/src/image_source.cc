#include "vlk/image_source.h"

#include "fmt/format.h"

namespace vlk {

namespace {

inline auto make_file_image_source_data(
    std::filesystem::path&& path, stx::Option<ImageFormat>&& target_format) {
  auto format_str = target_format.clone()
                        .map([](ImageFormat value) { return format(value); })
                        .unwrap_or("Internal");

  auto tag =
      fmt::format("FileImage(path: {}, format: {})", path.c_str(), format_str);

  return stx::mem::make_rc_inplace<FileImageSourceData const>(
      std::move(path), target_format, std::move(tag));
}

inline auto make_memory_image_source_data(ImageInfo image_info,
                                          std::vector<uint8_t>&& image_buffer) {
  // VLK_ENSURE(image_info.extent.width * image_info.extent.height *
  // impl::unaligned_channel_size(image_info.format) ==
  // image_buffer.size());
  // VLK_ENSURE(image_info.extent.visible());
  uint64_t uid = MemoryImageSource::make_uid();
  auto tag = fmt::format("MemoryImage(uid: {})", uid);

  return stx::mem::make_rc_inplace<MemoryImageSourceData const>(
      image_info, std::move(image_buffer), std::move(tag));
}

}  // namespace

FileImageSource::FileImageSource(std::filesystem::path path,
                                 stx::Option<ImageFormat> target_format)
    : data{make_file_image_source_data(std::move(path),
                                       std::move(target_format))} {}

MemoryImageSource::MemoryImageSource(ImageInfo image_info,
                                     std::vector<uint8_t>&& image_buffer)
    : data{make_memory_image_source_data(image_info, std::move(image_buffer))} {
}

}  // namespace vlk
