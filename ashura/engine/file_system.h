/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/std/allocator.h"
#include "ashura/std/async.h"
#include "ashura/std/fs.h"
#include "ashura/std/types.h"
#include "ashura/std/vec.h"

namespace ash
{

typedef struct IFileSys * FileSys;

struct IFileSys
{
  explicit IFileSys(Allocator allocator);

  IFileSys(IFileSys const &)             = delete;
  IFileSys(IFileSys &&)                  = delete;
  IFileSys & operator=(IFileSys const &) = delete;
  IFileSys & operator=(IFileSys &&)      = delete;
  ~IFileSys();

  void shutdown();

  Future<Result<Vec<u8>, IoErr>> load_file(Allocator allocator, Str path);
};

}    // namespace ash
