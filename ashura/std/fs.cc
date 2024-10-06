
/// SPDX-License-Identifier: MIT
#include "ashura/std/fs.h"
#include "ashura/std/error.h"
#include "ashura/std/log.h"

namespace ash
{

IoError read_file(Span<char const> path, Vec<u8> &buff)
{
  CHECK(!path.is_empty());
  Vec<char> path_s;
  defer     path_cleanup{[&] { path_s.reset(); }};
  if (!path_s.extend_copy(path) || !path_s.push((char) 0))
  {
    return IoError::OutOfMemory;
  }

  FILE *file = fopen(path_s.data(), "rb");
  if (file == nullptr)
  {
    return (IoError) errno;
  }
  defer _{[&] { fclose(file); }};

  int error = fseek(file, 0, SEEK_END);
  if (error != 0)
  {
    return (IoError) errno;
  }

  long file_size = ftell(file);
  if (file_size == -1L)
  {
    return (IoError) errno;
  }

  error = fseek(file, 0, SEEK_SET);
  if (error != 0)
  {
    return (IoError) errno;
  }

  usize buff_offset = buff.size();
  if (!buff.extend_uninitialized(file_size))
  {
    return IoError::OutOfMemory;
  }

  if (fread(buff.data() + buff_offset, 1, (usize) file_size, file) !=
      (usize) file_size)
  {
    return (IoError) errno;
  }

  return IoError::None;
}

}        // namespace ash