/// SPDX-License-Identifier: MIT
#include "ashura/engine/file_system.h"
#include "ashura/std/fs.h"

namespace ash
{

IFileSys::IFileSys(Scheduler scheduler, [[maybe_unused]] Allocator allocator) :
  scheduler_(scheduler)
{
}

SysErr to_sys_err(IoErr err)
{
    switch (err)
    {
        case IoErr::None:
            return SysErr::None;
        case IoErr::OutOfMemory:
            return SysErr::OutOfMemory;
        case IoErr::InvalidFileOrDir:
            return SysErr::InvalidPath;
        default:
            return SysErr::IoErr;
    }
}

Future<Result<Vec<u8>, SysErr>> IFileSys::load_file(Allocator allocator, Str path)
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
                         [&](IoErr err) -> R { return Err{to_sys_err(err)}; });
            })
      .unwrap();
}

void IFileSys::shutdown()
{
}

}    // namespace ash
