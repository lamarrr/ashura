/// SPDX-License-Identifier: MIT
#include "ashura/engine/file_system.h"

namespace ash
{

IFileSys::IFileSys([[maybe_unused]] Allocator allocator)
{
}

Future<Result<Vec<u8>, IoErr>> IFileSys::load_file(Allocator allocator,
                                                   Str       path)
{
  Vec<char> path_copy{allocator};
  path_copy.extend(path).unwrap();

  Future fut = future<Result<Vec<u8>, IoErr>>(allocator).unwrap();

  scheduler->once(
    [allocator, path = std::move(path_copy), fut = fut.alias()]() {
      Vec<u8> data{allocator};
      read_file(path, data)
        .match([&](Void) { fut.yield(Ok{std::move(data)}).unwrap(); },
               [&](IoErr err) { fut.yield(Err{err}).unwrap(); });
    },
    Ready{}, ThreadId::AnyWorker);

  return fut;
}

void IFileSys::shutdown()
{
}

}    // namespace ash
