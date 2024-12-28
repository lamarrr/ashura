/// SPDX-License-Identifier: MIT
#include "ashura/std/fs.h"
#include "ashura/std/error.h"
#include "ashura/std/log.h"
#include <cstdio>

namespace ash
{

Result<Void, IoErr> read_file(AllocatorRef allocator, Span<char const> path,
                              Vec<u8> & buff)
{
  Vec<char> path_c_str{allocator};

  if (!path_c_str.extend_uninit(path.size() + 1))
  {
    return Err{IoErr::OutOfMemory};
  }

  CHECK(to_c_str(path, path_c_str));

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

Result<Void, IoErr> write_to_file(AllocatorRef allocator, Span<char const> path,
                                  Span<u8 const> buff, bool append)
{
  Vec<char> path_c_str{allocator};

  if (!path_c_str.extend_uninit(path.size() + 1))
  {
    return Err{IoErr::OutOfMemory};
  }

  CHECK(to_c_str(path, path_c_str));

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
