/// SPDX-License-Identifier: MIT
#include "ashura/std/fs.h"
#include "ashura/std/allocators.h"
#include "ashura/std/error.h"
#include <cstdio>

namespace ash
{

constexpr usize PATH_RESERVED_SIZE = 256;

Result<Void, IoErr> read_file(Str path, Vec<u8> & buff)
{
  u8                reserved[PATH_RESERVED_SIZE];
  FallbackAllocator allocator{reserved, default_allocator};
  Vec<char>         path_c_str{allocator};

  if (!path_c_str.extend_uninit(path.size() + 1))
  {
    return Err{IoErr::OutOfMemory};
  }

  mem::copy(path, path_c_str.data());
  path_c_str.last() = '\0';

  std::FILE * file = std::fopen(path_c_str.data(), "rb");

  if (file == nullptr)
  {
    return Err{(IoErr) errno};
  }

  defer file_{[&] { std::fclose(file); }};

  int error = std::fseek(file, 0, SEEK_END);

  if (error != 0)
  {
    return Err{(IoErr) errno};
  }

  long file_size = std::ftell(file);

  if (file_size == -1L)
  {
    return Err{(IoErr) errno};
  }

  error = std::fseek(file, 0, SEEK_SET);

  if (error != 0)
  {
    return Err{(IoErr) errno};
  }

  usize buff_offset = buff.size();

  if (!buff.extend_uninit(file_size))
  {
    return Err{IoErr::OutOfMemory};
  }

  if (std::fread(buff.data() + buff_offset, 1, (usize) file_size, file) !=
      (usize) file_size)
  {
    return Err{(IoErr) errno};
  }

  return Ok{};
}

Result<Void, IoErr> write_to_file(Str path, Span<u8 const> buff, bool append)
{
  u8                reserved[PATH_RESERVED_SIZE];
  FallbackAllocator allocator{reserved, default_allocator};
  Vec<char>         path_c_str{allocator};

  if (!path_c_str.extend_uninit(path.size() + 1))
  {
    return Err{IoErr::OutOfMemory};
  }

  mem::copy(path, path_c_str.data());
  path_c_str.last() = '\0';

  std::FILE * file = std::fopen(path_c_str.data(), append ? "a" : "w");

  if (file == nullptr)
  {
    return Err{(IoErr) errno};
  }

  defer file_{[&] { std::fclose(file); }};

  std::fwrite(buff.data(), 1, buff.size(), file);

  return Ok{};
}

}    // namespace ash
