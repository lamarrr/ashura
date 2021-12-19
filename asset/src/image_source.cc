#include "vlk/image_source.h"

#include "fmt/format.h"

namespace vlk {

namespace {

inline auto make_file_image_source_data(
    std::filesystem::path&& path, stx::Option<ImageFormat>&& target_format) {
  auto format_str = target_format.copy()
                        .map([](ImageFormat value) { return format(value); })
                        .unwrap_or("Internal");

  auto tag =
      fmt::format("FileImage(path: {}, format: {})", path.c_str(), format_str);

  return stx::rc::make_inplace<FileImageSourceData const>(
             stx::os_allocator, std::move(path), target_format, std::move(tag))
      .unwrap();
}

inline auto make_memory_image_source_data(ImageInfo image_info,
                                          std::vector<uint8_t>&& image_buffer) {
  // VLK_ENSURE(image_info.extent.width * image_info.extent.height *
  // impl::unaligned_channel_size(image_info.format) ==
  // image_buffer.size());
  // VLK_ENSURE(image_info.extent.visible());
  uint64_t uid = MemoryImageSource::make_uid();
  auto tag = fmt::format("MemoryImage(uid: {})", uid);

  return stx::rc::make_inplace<MemoryImageSourceData const>(
             stx::os_allocator, image_info, std::move(image_buffer),
             std::move(tag))
      .unwrap();
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

uint64_t MemoryImageSource::make_uid() {
  static std::atomic<uint64_t> latest_uid = 0;

  return latest_uid.fetch_add(1, std::memory_order_seq_cst);
}

}  // namespace vlk
