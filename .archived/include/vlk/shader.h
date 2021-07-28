#pragma once

#include <filesystem>
#include <fstream>
#include <string>
#include <utility>

namespace vlk {

enum class FileError { NotExist, OpenError };

stx::Result<std::basic_string<uint32_t>, FileError> load_spirv_binary(
    std::filesystem::path const& path) {
  if (!std::filesystem::exists(path)) return stx::Err(FileError::NotExist);
  auto file =
      std::ifstream(path.c_str(), std::ios_base::ate | std::ios_base::binary);

  if (!file.is_open()) return stx::Err(FileError::OpenError);

  auto size = file.tellg();
  file.seekg(0);

  VLK_ENSURE(size % 4 == 0, "Files byte contents are unaligned to uint32_t");

  std::basic_string<uint32_t> bytes;
  bytes.resize(size / 4);

  file.read(reinterpret_cast<char*>(bytes.data()), size);

  return stx::Ok(std::move(bytes));
}
}  // namespace vlk
