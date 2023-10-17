#include "ashura/gfx.h"

namespace ash
{
namespace gfx
{

BufferAccess to_pipeline_access(BufferBindings bindings, PipelineStages binding_stages)
{
  Access         access = Access::None;
  PipelineStages stages = PipelineStages::None;

  if ((bindings & (BufferBindings::Uniform | BufferBindings::Storage |
                   BufferBindings::UniformTexel | BufferBindings::StorageTexel)) !=
      BufferBindings::None)
  {
    stages |= binding_stages;
  }

  if ((bindings & (BufferBindings::IndexBuffer | BufferBindings::VertexBuffer)) !=
      BufferBindings::None)
  {
    stages |= PipelineStages::VertexInput;
  }

  if ((bindings & BufferBindings::IndexBuffer) != BufferBindings::None)
  {
    access |= Access::IndexRead;
  }

  if ((bindings & BufferBindings::VertexBuffer) != BufferBindings::None)
  {
    access |= Access::VertexAttributeRead;
  }

  if ((bindings & (BufferBindings::Uniform | BufferBindings::UniformTexel)) != BufferBindings::None)
  {
    access |= Access::ShaderRead;
  }

  if ((bindings & (BufferBindings::Storage | BufferBindings::StorageTexel)) != BufferBindings::None)
  {
    access |= Access::ShaderWrite;
  }

  return BufferAccess{.stages = stages, .access = access};
}

ImageAccess to_pipeline_access(ImageBindings bindings, PipelineStages binding_stages)
{
  Access         access = Access::None;
  ImageLayout    layout = ImageLayout::Undefined;
  PipelineStages stages = PipelineStages::None;

  if ((bindings & ImageBindings::InputAttachment) != ImageBindings::None)
  {
    access |= Access::InputAttachmentRead;
  }

  if ((bindings & ImageBindings::Sampled) != ImageBindings::None)
  {
    access |= Access::ShaderRead;
  }

  if ((bindings & ImageBindings::Storage) != ImageBindings::None)
  {
    access |= Access::ShaderWrite;
  }

  if ((bindings & ImageBindings::Storage) != ImageBindings::None)
  {
    layout = ImageLayout::General;
  }
  else if ((bindings & (ImageBindings::Sampled | ImageBindings::InputAttachment)) !=
           ImageBindings::None)
  {
    layout = ImageLayout::ShaderReadOnlyOptimal;
  }

  if (bindings == ImageBindings::InputAttachment)
  {
    stages = PipelineStages::FragmentShader;
  }
  else if (bindings != ImageBindings::None)
  {
    stages = binding_stages;
  }

  return ImageAccess{.stages = stages, .access = access, .layout = layout};
}

ImageAccess RenderPassAttachment::get_color_image_access() const
{
  ImageLayout layout    = ImageLayout::ColorAttachmentOptimal;
  bool        has_write = false;
  bool        has_read  = false;
  Access      access    = Access::None;

  if (load_op == LoadOp::Clear || load_op == LoadOp::DontCare || store_op == StoreOp::Store)
  {
    has_write = true;
  }

  if (load_op == LoadOp::Load)
  {
    has_read = true;
  }

  if (has_write)
  {
    access |= Access::ColorAttachmentWrite;
  }

  if (has_read)
  {
    access |= Access::ColorAttachmentRead;
  }

  return ImageAccess{
      .stages = PipelineStages::ColorAttachmentOutput, .access = access, .layout = layout};
}

ImageAccess RenderPassAttachment::get_depth_stencil_image_access() const
{
  ImageLayout layout    = ImageLayout::Undefined;
  bool        has_write = false;
  bool        has_read  = false;
  Access      access    = Access::None;

  if (load_op == LoadOp::Clear || load_op == LoadOp::DontCare || store_op == StoreOp::Store ||
      store_op == StoreOp::DontCare || stencil_load_op == LoadOp::Clear ||
      stencil_load_op == LoadOp::DontCare || stencil_store_op == StoreOp::Store ||
      stencil_store_op == StoreOp::DontCare)
  {
    has_write = true;
  }

  if (load_op == LoadOp::Load || stencil_load_op == LoadOp::Load)
  {
    has_read = true;
  }

  if (has_write)
  {
    access |= Access::DepthStencilAttachmentWrite;
  }

  if (has_read)
  {
    access |= Access::DepthStencilAttachmentRead;
  }

  if (has_write)
  {
    layout = ImageLayout::DepthStencilAttachmentOptimal;
  }
  else
  {
    layout = ImageLayout::DepthStencilReadOnlyOptimal;
  }

  return ImageAccess{.stages = PipelineStages::EarlyFragmentTests |
                               PipelineStages::LateFragmentTests |
                               PipelineStages::ColorAttachmentOutput,
                     .access = access,
                     .layout = layout};
}

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