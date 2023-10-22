#include "ashura/gfx.h"

namespace ash
{
namespace gfx
{

bool BufferSyncScope::sync(MemoryAccess memory_access, PipelineStages stages,
                           QueueBufferMemoryBarrier &barrier)



// TODO(lamarrr): index buffer can be used from a generated compute stage, will our graph handle
// this? we need to check for read/write compatibility
// TODO(lamarrr): on every device idle, we can reset resource states.
// TODO(lamarrr): for each renderpass execution we need to get accessed resources and generate
// barriers for them
//
// requirements:
// - allow simultaneous reads
// - prevent simultaneous writes
// - prevent simultaneous writes and reads
//
// Also, See:
// https://stackoverflow.com/questions/60339191/synchronizing-a-render-pass-layout-transition-with-a-semaphore-in-acquire-pres
bool BufferState::sync(BufferAccess request, QueueBufferMemoryBarrier &barrier)
{
  MemoryOps const ops = get_memory_ops(request.access);

  switch (sequence)
  {
      // no sync needed, no accessor before this
    case AccessSequence::None:
    {
      if ((ops & MemoryOps::Write) != MemoryOps::None)
      {
        sequence  = AccessSequence::NoneAfterWrite;
        access[0] = BufferAccess{.stages = request.stages, .access = request.access};
        return false;
      }
      else if ((ops & MemoryOps::Read) != MemoryOps::None)
      {
        sequence  = AccessSequence::NoneAfterRead;
        access[0] = BufferAccess{.stages = request.stages, .access = request.access};
        return false;
      }
    }
    break;
    case AccessSequence::NoneAfterRead:
    {
      if ((ops & MemoryOps::Write) != MemoryOps::None)
      {
        // wait till done reading before modifying
        // reset access sequence since all stages following this write need to wait on this write
        sequence                          = AccessSequence::NoneAfterWrite;
        BufferAccess const previous_reads = access[0];
        access[0]          = BufferAccess{.stages = request.stages, .access = request.access};
        access[1]          = BufferAccess{};
        barrier.src_stages = previous_reads.stages;
        barrier.src_access = previous_reads.access;
        barrier.dst_stages = request.stages;
        barrier.dst_access = request.access;
        return true;
      }
      else if ((ops & MemoryOps::Read) != MemoryOps::None)
      {
        // combine all subsequent reads, so the next writer knows to wait on all combined reads to
        // complete
        sequence                          = AccessSequence::NoneAfterRead;
        BufferAccess const previous_reads = access[0];
        access[0] = BufferAccess{.stages = previous_reads.stages | request.stages,
                                 .access = previous_reads.access | request.access};
        return false;
      }
    }
    break;
    case AccessSequence::NoneAfterWrite:
    {
      if ((ops & MemoryOps::Write) != MemoryOps::None)
      {
        // wait till done writing before modifying
        // remove previous write since this access already waits on another access to complete
        // and the next access will have to wait on this access
        sequence                          = AccessSequence::NoneAfterWrite;
        BufferAccess const previous_write = access[0];
        access[0]          = BufferAccess{.stages = request.stages, .access = request.access};
        access[1]          = BufferAccess{};
        barrier.src_stages = previous_write.stages;
        barrier.src_access = previous_write.access;
        barrier.dst_stages = request.stages;
        barrier.dst_access = request.access;
        return true;
      }
      else if ((ops & MemoryOps::Read) != MemoryOps::None)
      {
        // wait till all write stages are done
        sequence           = AccessSequence::ReadAfterWrite;
        access[1]          = BufferAccess{.stages = request.stages, .access = request.access};
        barrier.src_stages = access[0].stages;
        barrier.src_access = access[0].access;
        barrier.dst_stages = request.stages;
        barrier.dst_access = request.access;
        return true;
      }
    }
    break;
    case AccessSequence::ReadAfterWrite:
    {
      if ((ops & MemoryOps::Write) != MemoryOps::None)
      {
        // wait for all reading stages only
        // stages can be reset and point only to the latest write stage, since they all need to wait
        // for this write anyway.
        sequence                          = AccessSequence::NoneAfterWrite;
        BufferAccess const previous_reads = access[1];
        access[0]          = BufferAccess{.stages = request.stages, .access = request.access};
        access[1]          = BufferAccess{};
        barrier.src_stages = previous_reads.stages;
        barrier.src_access = previous_reads.access;
        barrier.dst_stages = request.stages;
        barrier.dst_access = request.access;
        return true;
      }
      else if ((ops & MemoryOps::Read) != MemoryOps::None)
      {
        // wait for all write stages to be done
        // no need to wait on other reads since we are only performing a read
        // mask all subsequent reads so next writer knows to wait on all reads to complete
        sequence = AccessSequence::ReadAfterWrite;
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
    default:
      return false;
      break;
  }
}

void BufferState::on_drain()
{
  access[0].access = Access::None;
  access[0].stages = PipelineStages::None;
  sequence         = AccessSequence::None;
}

// layout transitions are considered write operations even if only a read happens so multiple ones
// can't happen at the same time
bool ImageState::sync(ImageAccess request, QueueImageMemoryBarrier &barrier)
{
  ImageLayout const current_layout = access[0].layout;

  // TODO(lamarrr): make sure aspects are filled
  bool const      needs_layout_transition = (current_layout != request.layout);
  MemoryOps const ops                     = get_memory_ops(request.access) |
                        (needs_layout_transition ? MemoryOps::Write : MemoryOps::None);

  switch (sequence)
  {
      // no sync needed, no accessor before this
    case AccessSequence::None:
    {
      if ((ops & MemoryOps::Write) != MemoryOps::None)
      {
        sequence  = AccessSequence::NoneAfterWrite;
        access[0] = ImageAccess{
            .stages = request.stages, .access = request.access, .layout = request.layout};

        if (needs_layout_transition)
        {
          barrier.first_mip_level   = 0;
          barrier.num_mip_levels    = REMAINING_MIP_LEVELS;
          barrier.first_array_layer = 0;
          barrier.num_array_layers  = REMAINING_ARRAY_LAYERS;
          barrier.old_layout        = current_layout;
          barrier.new_layout        = request.layout;
          barrier.src_stages        = PipelineStages::None;
          barrier.dst_stages        = request.stages;
          barrier.src_access        = Access::None;
          barrier.dst_access        = request.access;
          return true;
        }
        else
        {
          return false;
        }
      }
      else if ((ops & MemoryOps::Read) != MemoryOps::None)
      {
        sequence  = AccessSequence::NoneAfterRead;
        access[0] = ImageAccess{
            .stages = request.stages, .access = request.access, .layout = request.layout};
        return false;
      }
    }
    break;
    case AccessSequence::NoneAfterRead:
    {
      if ((ops & MemoryOps::Write) != MemoryOps::None)
      {
        // wait till done reading before modifying
        // reset access sequence since all stages following this write need to wait on this write
        sequence                         = AccessSequence::NoneAfterWrite;
        ImageAccess const previous_reads = access[0];
        access[0]                        = ImageAccess{
                                   .stages = request.stages, .access = request.access, .layout = request.layout};
        access[1]                 = ImageAccess{};
        barrier.first_mip_level   = 0;
        barrier.num_mip_levels    = REMAINING_MIP_LEVELS;
        barrier.first_array_layer = 0;
        barrier.num_array_layers  = REMAINING_ARRAY_LAYERS;
        barrier.old_layout        = current_layout;
        barrier.new_layout        = request.layout;
        barrier.src_stages        = previous_reads.stages;
        barrier.dst_stages        = request.stages;
        barrier.src_access        = previous_reads.access;
        barrier.dst_access        = request.access;
        return true;
      }
      else if ((ops & MemoryOps::Read) != MemoryOps::None)
      {
        // combine all subsequent reads, so the next writer knows to wait on all combined reads to
        // complete
        sequence                         = AccessSequence::NoneAfterRead;
        ImageAccess const previous_reads = access[0];
        access[0] = ImageAccess{.stages = previous_reads.stages | request.stages,
                                .access = previous_reads.access | request.access,
                                .layout = request.layout};
        return false;
      }
    }
    break;
    case AccessSequence::NoneAfterWrite:
    {
      if ((ops & MemoryOps::Write) != MemoryOps::None)
      {
        // wait till done writing before modifying
        // remove previous write since this access already waits on another access to complete
        // and the next access will have to wait on this access
        sequence                         = AccessSequence::NoneAfterWrite;
        ImageAccess const previous_write = access[0];
        access[0]                        = ImageAccess{
                                   .stages = request.stages, .access = request.access, .layout = request.layout};
        access[1]                 = ImageAccess{};
        barrier.first_mip_level   = 0;
        barrier.num_mip_levels    = REMAINING_MIP_LEVELS;
        barrier.first_array_layer = 0;
        barrier.num_array_layers  = REMAINING_ARRAY_LAYERS;
        barrier.old_layout        = current_layout;
        barrier.new_layout        = request.layout;
        barrier.src_stages        = previous_write.stages;
        barrier.dst_stages        = request.stages;
        barrier.src_access        = previous_write.access;
        barrier.dst_access        = request.access;
        return true;
      }
      else if ((ops & MemoryOps::Read) != MemoryOps::None)
      {
        // wait till all write stages are done
        sequence  = AccessSequence::ReadAfterWrite;
        access[1] = ImageAccess{
            .stages = request.stages, .access = request.access, .layout = request.layout};
        barrier.first_mip_level   = 0;
        barrier.num_mip_levels    = REMAINING_MIP_LEVELS;
        barrier.first_array_layer = 0;
        barrier.num_array_layers  = REMAINING_ARRAY_LAYERS;
        barrier.old_layout        = current_layout;
        barrier.new_layout        = request.layout;
        barrier.src_stages        = access[0].stages;
        barrier.dst_stages        = request.stages;
        barrier.src_access        = access[0].access;
        barrier.dst_access        = request.access;
        return true;
      }
    }
    break;
    case AccessSequence::ReadAfterWrite:
    {
      if ((ops & MemoryOps::Write) != MemoryOps::None)
      {
        // wait for all reading stages only
        // stages can be reset and point only to the latest write stage, since they all need to wait
        // for this write anyway.
        sequence                         = AccessSequence::NoneAfterWrite;
        ImageAccess const previous_reads = access[1];
        access[0]                        = ImageAccess{
                                   .stages = request.stages, .access = request.access, .layout = request.layout};
        access[1]                 = ImageAccess{};
        barrier.first_mip_level   = 0;
        barrier.num_mip_levels    = REMAINING_MIP_LEVELS;
        barrier.first_array_layer = 0;
        barrier.num_array_layers  = REMAINING_ARRAY_LAYERS;
        barrier.old_layout        = current_layout;
        barrier.new_layout        = request.layout;
        barrier.src_stages        = previous_reads.stages;
        barrier.dst_stages        = request.stages;
        barrier.src_access        = previous_reads.access;
        barrier.dst_access        = request.access;
        return true;
      }
      else if ((ops & MemoryOps::Read) != MemoryOps::None)
      {
        // wait for all write stages to be done
        // no need to wait on other reads since we are only performing a read
        // mask all subsequent reads so next writer knows to wait on all reads to complete
        sequence = AccessSequence::ReadAfterWrite;
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
    default:
      return false;
  }
}

void ImageState::on_drain()
{
  sequence         = AccessSequence::None;
  access[0].access = Access::None;
  access[0].stages = PipelineStages::None;
}

}        // namespace gfx
}        // namespace ash