/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/errors.h"
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
  Scheduler scheduler_;

  explicit IFileSys(Scheduler scheduler, Allocator allocator);

  IFileSys(IFileSys const &)             = delete;
  IFileSys(IFileSys &&)                  = delete;
  IFileSys & operator=(IFileSys const &) = delete;
  IFileSys & operator=(IFileSys &&)      = delete;
  ~IFileSys();

  void shutdown();

  Future<Result<Vec<u8>, SysErr>> load_file(Allocator allocator, Str path);

  Result<RcBlob8, SysErr> load_memory_mapped(Allocator allocator, Str path);

  void load_async();
};

}    // namespace ash
