#include "ashura/gfx.h"

namespace ash
{
namespace gfx
{

bool BufferState::sync(BufferAccess request, QueueBufferMemoryBarrier &barrier)
{
  MemoryAccess const memory_access = get_memory_access(request.access);

  switch (sequence)
  {
      // no sync needed, no accessor before this
    case RWSequence::None:
    {
      if ((memory_access & MemoryAccess::Write) != MemoryAccess::None)
      {
        sequence  = RWSequence::NoneAfterWrite;
        access[0] = BufferAccess{.stages = request.stages,
                                 .access = request.access};
        return false;
      }
      else if ((memory_access & MemoryAccess::Read) != MemoryAccess::None)
      {
        sequence  = RWSequence::NoneAfterRead;
        access[0] = BufferAccess{.stages = request.stages,
                                 .access = request.access};
        return false;
      }
    }
    break;
    case RWSequence::NoneAfterRead:
    {
      if ((memory_access & MemoryAccess::Write) != MemoryAccess::None)
      {
        // wait till done reading before modifying
        // reset access sequence since all stages following this write need to wait on this write
        sequence                            = RWSequence::NoneAfterWrite;
        BufferAccess const prev_read_access = access[0];
        access[0]                           = BufferAccess{.stages = request.stages,
                                                           .access = request.access};
        access[1]                           = BufferAccess{};
        barrier.src_stages                  = prev_read_access.stages;
        barrier.src_access                  = prev_read_access.access;
        barrier.dst_stages                  = request.stages;
        barrier.dst_access                  = request.access;
        return true;
      }
      else if ((memory_access & MemoryAccess::Read) != MemoryAccess::None)
      {
        // combine all reads, so the next writer knows to wait on all combined reads to complete
        sequence                       = RWSequence::NoneAfterRead;
        BufferAccess const prev_access = access[0];
        access[0]                      = BufferAccess{.stages = prev_access.stages | request.stages,
                                                      .access = prev_access.access | request.access};
        return false;
      }
    }
    break;
    case RWSequence::NoneAfterWrite:
    {
      if ((memory_access & MemoryAccess::Write) != MemoryAccess::None)
      {
        // wait till done writing before modifying
        // collapse write since this access already waits on another access to complete
        // and the next access will have to wait on this access
        sequence                       = RWSequence::NoneAfterWrite;
        BufferAccess const prev_access = access[0];
        access[0]                      = BufferAccess{.stages = request.stages,
                                                      .access = request.access};
        access[1]                      = BufferAccess{};
        barrier.src_stages             = prev_access.stages;
        barrier.src_access             = prev_access.access;
        barrier.dst_stages             = request.stages;
        barrier.dst_access             = request.access;
        return true;
      }
      else if ((memory_access & MemoryAccess::Read) != MemoryAccess::None)
      {
        // wait till all write stages are done
        // no read/write stages waits on read-only stages so we need to retain the sequence
        sequence           = RWSequence::ReadAfterWrite;
        access[1]          = BufferAccess{.stages = request.stages,
                                          .access = request.access};
        barrier.src_stages = access[0].stages;
        barrier.src_access = access[0].access;
        barrier.dst_stages = request.stages;
        barrier.dst_access = request.access;
        return true;
      }
    }
    break;
    case RWSequence::ReadAfterWrite:
    {
      if ((memory_access & MemoryAccess::Write) != MemoryAccess::None)
      {
        // wait for all reading stages only
        // stages can be reset and point only to the latest write stage, since they all need to wait for writes anyway.
        sequence                            = RWSequence::NoneAfterWrite;
        BufferAccess const prev_read_access = access[1];
        access[0]                           = BufferAccess{.stages = request.stages,
                                                           .access = request.access};
        access[1]                           = BufferAccess{};
        barrier.src_stages                  = prev_read_access.stages;
        barrier.src_access                  = prev_read_access.access;
        barrier.dst_stages                  = request.stages;
        barrier.dst_access                  = request.access;
        return true;
      }
      else if ((memory_access & MemoryAccess::Read) != MemoryAccess::None)
      {
        // wait for all write stages to be done
        // mask all subsequent reads so next writer knows to wait on all reads to complete
        sequence = RWSequence::ReadAfterWrite;
        access[1].stages |= request.stages;
        access[1].access |= request.access;
        barrier.src_stages = access[0].stages;
        barrier.src_access = access[0].access;
        barrier.dst_stages = request.stages;
        barrier.dst_access = request.access;
        return true;
      }
    }
    break;
    case RWSequence::WriteAfterRead:
      // {
      //   if ((memory_access & MemoryAccess::Write) != MemoryAccess::None)
      //   {
      //     // wait for write stage only since the write after the read is already waiting
      //     // stages can be reset and point only to the latest write stage, since they all need to wait for writes anyway.
      //     sequence                             = RWSequence::NoneAfterWrite;
      //     BufferAccess const prev_write_access = access[1];
      //     access[0]                            = BufferAccess{.stages = request.stages,
      //                                                         .access = request.access};
      //     access[1]                            = BufferAccess{};
      //     barrier.src_stages                   = prev_write_access.stages;
      //     barrier.src_access                   = prev_write_access.access;
      //     barrier.dst_stages                   = request.stages;
      //     barrier.dst_access                   = request.access;
      //     return true;
      //   }
      //   else if ((memory_access & MemoryAccess::Read) != MemoryAccess::None)
      //   {
      //     // wait for all write stages to be done
      //     // but combine all read
      //     // mask all subsequent reads so next writer knows to wait on all reads to complete
      //     sequence = RWSequence::ReadAfterWrite;
      //     access[1].stages |= request.stages;
      //     access[1].access |= request.access;
      //     barrier.src_stages = access[0].stages;
      //     barrier.src_access = access[0].access;
      //     barrier.dst_stages = request.stages;
      //     barrier.dst_access = request.access;
      //     return true;
      //   }
      // }
      break;
  }
}
}        // namespace gfx
}        // namespace ash