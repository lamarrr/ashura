
/// SPDX-License-Identifier: MIT
#include "ashura/std/fs.h"
#include "ashura/std/error.h"
#include "ashura/std/log.h"
#include <cstdio>

namespace ash
{

Result<Void, IoError> read_file(Span<char const> path, Vec<u8> &buff)
{
  CHECK(!path.is_empty());
  Vec<char> path_s;
  if (!path_s.extend_copy(path) || !path_s.push((char) 0))
  {
    return Err{IoError::OutOfMemory};
  }

  std::FILE *file = std::fopen(path_s.data(), "rb");
  if (file == nullptr)
  {
    return Err{(IoError) errno};
  }
  defer file_{[&] { std::fclose(file); }};

  int error = std::fseek(file, 0, SEEK_END);
  if (error != 0)
  {
    return Err{(IoError) errno};
  }

  long file_size = std::ftell(file);
  if (file_size == -1L)
  {
    return Err{(IoError) errno};
  }

  error = std::fseek(file, 0, SEEK_SET);
  if (error != 0)
  {
    return Err{(IoError) errno};
  }

  usize buff_offset = buff.size();
  if (!buff.extend_uninit(file_size))
  {
    return Err{IoError::OutOfMemory};
  }

  if (std::fread(buff.data() + buff_offset, 1, (usize) file_size, file) !=
      (usize) file_size)
  {
    return Err{(IoError) errno};
  }

  return Ok{};
}

}        // namespace ash