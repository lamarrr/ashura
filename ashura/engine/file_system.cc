/// SPDX-License-Identifier: MIT
#include "ashura/engine/file_system.h"

namespace ash
{

IFileSys::IFileSys(Scheduler scheduler, [[maybe_unused]] Allocator allocator) :
  scheduler_(scheduler)
{
}

Future<Result<Vec<u8>, SysErr>> IFileSys::load_file(Allocator allocator,
                                                    Str       path)
{
  Vec<char> path_copy{allocator};
  path_copy.append(path).unwrap();

  return scheduler_
    ->run(allocator, WorkerThread::Any,
          [allocator, path = std::move(path_copy)]() {
            Vec<u8> data{allocator};
            using R = Result<Vec<u8>, SysErr>;
            return read_file(path, data, allocator)
              .match([&](Void) -> R { return Ok{std::move(data)}; },
                     [&](IoErr) -> R {
                       // [ ] Domain transfer from IoErr to SysErr
                       return Err{SysErr::IoErr};
                     });
          })
    .unwrap();
}

void IFileSys::shutdown()
{
}

}    // namespace ash
