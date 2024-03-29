
#include "ashura/engine/io.h"

namespace ash
{

IoError read_file(char const *path, Vec<char> &buff)
{
  FILE *file = fopen(path, "rb");
  if (file == nullptr)
  {
    return (IoError) errno;
  }
  defer file_close{[&] { fclose(file); }};

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
  usize file_size_u = (usize) file_size;

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

  if (fread(buff.data() + buff_offset, 1, file_size_u, file) != file_size_u)
  {
    return (IoError) errno;
  }

  return IoError::None;
}

}        // namespace ash