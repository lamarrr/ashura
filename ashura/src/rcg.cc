#include "ashura/rcg.h"

namespace ash
{
namespace rcg
{

constexpr u32 MAX_SCOPE_BARRIERS = 16;
constexpr u64 BATCH_SIZE         = 16;

// warnings: can't be used as depth stencil and color attachment
// load op clear op, read write matches imageusagescope
// ssbo matches scope
// push constant size match check
// NOTE: renderpass attachments MUST not be accessed in shaders within that renderpass
// NOTE: update_buffer and fill_buffer MUST be multiple of 4 for dst offset and dst size
struct BufferScope
{
  gfx::PipelineStages stages = gfx::PipelineStages::None;
  gfx::Access         access = gfx::Access::None;
};

struct ImageScope
{
  gfx::PipelineStages stages = gfx::PipelineStages::None;
  gfx::Access         access = gfx::Access::None;
  gfx::ImageLayout    layout = gfx::ImageLayout::Undefined;
};

template <usize Capacity>
inline void acquire_buffer(gfx::Buffer buffer, gfx::BufferUsageScope scope,
                           gfx::PipelineStages dst_stages, gfx::Access dst_access,
                           stx::Array<gfx::BufferMemoryBarrier, Capacity> &barriers)
{
  if (has_bits(scope, gfx::BufferUsageScope::TransferSrc | gfx::BufferUsageScope::TransferDst))
  {
    barriers
        .push(gfx::BufferMemoryBarrier{.src_stages = gfx::PipelineStages::Transfer,
                                       .dst_stages = dst_stages,
                                       .src_access =
                                           gfx::Access::TransferRead | gfx::Access::TransferWrite,
                                       .dst_access = dst_access,
                                       .buffer     = buffer})
        .unwrap();
  }
  else if (has_bits(scope, gfx::BufferUsageScope::TransferSrc))
  {
    barriers
        .push(gfx::BufferMemoryBarrier{.src_stages = gfx::PipelineStages::Transfer,
                                       .dst_stages = dst_stages,
                                       .src_access = gfx::Access::TransferRead,
                                       .dst_access = dst_access,
                                       .buffer     = buffer})
        .unwrap();
  }
  else if (has_bits(scope, gfx::BufferUsageScope::TransferDst))
  {
    barriers
        .push(gfx::BufferMemoryBarrier{.src_stages = gfx::PipelineStages::Transfer,
                                       .dst_stages = dst_stages,
                                       .src_access = gfx::Access::TransferWrite,
                                       .dst_access = dst_access,
                                       .buffer     = buffer})
        .unwrap();
  }

  if (has_bits(scope, gfx::BufferUsageScope::IndirectCommand))
  {
    barriers
        .push(gfx::BufferMemoryBarrier{.src_stages = gfx::PipelineStages::DrawIndirect,
                                       .dst_stages = dst_stages,
                                       .src_access = gfx::Access::IndirectCommandRead,
                                       .dst_access = dst_access,
                                       .buffer     = buffer})
        .unwrap();
  }

  if (has_any_bit(scope, gfx::BufferUsageScope::ComputeShaderUniform |
                             gfx::BufferUsageScope::ComputeShaderUniformTexel))
  {
    barriers
        .push(gfx::BufferMemoryBarrier{.src_stages = gfx::PipelineStages::ComputeShader,
                                       .dst_stages = dst_stages,
                                       .src_access = gfx::Access::ShaderRead,
                                       .dst_access = dst_access,
                                       .buffer     = buffer})
        .unwrap();
  }

  if (has_any_bit(scope, gfx::BufferUsageScope::ComputeShaderStorage |
                             gfx::BufferUsageScope::ComputeShaderStorageTexel))
  {
    barriers
        .push(gfx::BufferMemoryBarrier{.src_stages = gfx::PipelineStages::ComputeShader,
                                       .dst_stages = dst_stages,
                                       .src_access = gfx::Access::ShaderWrite,
                                       .dst_access = dst_access,
                                       .buffer     = buffer})
        .unwrap();
  }

  if (has_bits(scope, gfx::BufferUsageScope::IndexBuffer))
  {
    barriers
        .push(gfx::BufferMemoryBarrier{.src_stages = gfx::PipelineStages::VertexInput,
                                       .dst_stages = dst_stages,
                                       .src_access = gfx::Access::IndexRead,
                                       .dst_access = dst_access,
                                       .buffer     = buffer})
        .unwrap();
  }

  if (has_bits(scope, gfx::BufferUsageScope::VertexBuffer))
  {
    barriers
        .push(gfx::BufferMemoryBarrier{.src_stages = gfx::PipelineStages::VertexInput,
                                       .dst_stages = dst_stages,
                                       .src_access = gfx::Access::VertexAttributeRead,
                                       .dst_access = dst_access,
                                       .buffer     = buffer})
        .unwrap();
  }

  if (has_bits(scope, gfx::BufferUsageScope::VertexShaderUniform |
                          gfx::BufferUsageScope::FragmentShaderUniform))
  {
    barriers
        .push(gfx::BufferMemoryBarrier{.src_stages = gfx::PipelineStages::VertexShader |
                                                     gfx::PipelineStages::FragmentShader,
                                       .dst_stages = dst_stages,
                                       .src_access = gfx::Access::ShaderRead,
                                       .dst_access = dst_access,
                                       .buffer     = buffer})
        .unwrap();
  }
  else if (has_bits(scope, gfx::BufferUsageScope::VertexShaderUniform))
  {
    barriers
        .push(gfx::BufferMemoryBarrier{.src_stages = gfx::PipelineStages::VertexShader,
                                       .dst_stages = dst_stages,
                                       .src_access = gfx::Access::ShaderRead,
                                       .dst_access = dst_access,
                                       .buffer     = buffer})
        .unwrap();
  }
  else if (has_bits(scope, gfx::BufferUsageScope::FragmentShaderUniform))
  {
    barriers
        .push(gfx::BufferMemoryBarrier{.src_stages = gfx::PipelineStages::FragmentShader,
                                       .dst_stages = dst_stages,
                                       .src_access = gfx::Access::ShaderRead,
                                       .dst_access = dst_access,
                                       .buffer     = buffer})
        .unwrap();
  }
}

// release side-effects to operations that are allowed to have concurrent non-mutating access on the
// resource
template <usize Capacity>
inline void release_buffer(gfx::Buffer buffer, gfx::BufferUsageScope scope,
                           gfx::PipelineStages src_stages, gfx::Access src_access,
                           stx::Array<gfx::BufferMemoryBarrier, Capacity> &barriers)
{
  if (has_bits(scope, gfx::BufferUsageScope::TransferSrc))
  {
    barriers
        .push(gfx::BufferMemoryBarrier{.src_stages = src_stages,
                                       .dst_stages = gfx::PipelineStages::Transfer,
                                       .src_access = src_access,
                                       .dst_access = gfx::Access::TransferRead,
                                       .buffer     = buffer})
        .unwrap();
  }

  if (has_bits(scope, gfx::BufferUsageScope::IndirectCommand))
  {
    barriers
        .push(gfx::BufferMemoryBarrier{.src_stages = src_stages,
                                       .dst_stages = gfx::PipelineStages::DrawIndirect,
                                       .src_access = src_access,
                                       .dst_access = gfx::Access::IndirectCommandRead,
                                       .buffer     = buffer})
        .unwrap();
  }

  if (has_any_bit(scope, gfx::BufferUsageScope::ComputeShaderUniform |
                             gfx::BufferUsageScope::ComputeShaderUniformTexel))
  {
    barriers
        .push(gfx::BufferMemoryBarrier{.src_stages = src_stages,
                                       .dst_stages = gfx::PipelineStages::ComputeShader,
                                       .src_access = src_access,
                                       .dst_access = gfx::Access::ShaderRead,
                                       .buffer     = buffer})
        .unwrap();
  }

  if (has_bits(scope, gfx::BufferUsageScope::IndexBuffer))
  {
    barriers
        .push(gfx::BufferMemoryBarrier{.src_stages = src_stages,
                                       .dst_stages = gfx::PipelineStages::VertexInput,
                                       .src_access = src_access,
                                       .dst_access = gfx::Access::IndexRead,
                                       .buffer     = buffer})
        .unwrap();
  }

  if (has_bits(scope, gfx::BufferUsageScope::VertexBuffer))
  {
    barriers
        .push(gfx::BufferMemoryBarrier{.src_stages = src_stages,
                                       .dst_stages = gfx::PipelineStages::VertexInput,
                                       .src_access = src_access,
                                       .dst_access = gfx::Access::VertexAttributeRead,
                                       .buffer     = buffer})
        .unwrap();
  }

  if (has_bits(scope, gfx::BufferUsageScope::VertexShaderUniform |
                          gfx::BufferUsageScope::FragmentShaderUniform))
  {
    barriers
        .push(gfx::BufferMemoryBarrier{.src_stages = src_stages,
                                       .dst_stages = gfx::PipelineStages::VertexShader |
                                                     gfx::PipelineStages::FragmentShader,
                                       .src_access = src_access,
                                       .dst_access = gfx::Access::ShaderRead,
                                       .buffer     = buffer})
        .unwrap();
  }
  else if (has_bits(scope, gfx::BufferUsageScope::VertexShaderUniform))
  {
    barriers
        .push(gfx::BufferMemoryBarrier{.src_stages = src_stages,
                                       .dst_stages = gfx::PipelineStages::VertexShader,
                                       .src_access = src_access,
                                       .dst_access = gfx::Access::ShaderRead,
                                       .buffer     = buffer})
        .unwrap();
  }
  else if (has_bits(scope, gfx::BufferUsageScope::FragmentShaderUniform))
  {
    barriers
        .push(gfx::BufferMemoryBarrier{.src_stages = src_stages,
                                       .dst_stages = gfx::PipelineStages::FragmentShader,
                                       .src_access = src_access,
                                       .dst_access = gfx::Access::ShaderRead,
                                       .buffer     = buffer})
        .unwrap();
  }
}

constexpr BufferScope transfer_buffer_scope(gfx::BufferUsageScope scope)
{
  gfx::PipelineStages stages = gfx::PipelineStages::Transfer;
  gfx::Access         access = gfx::Access::None;

  if (has_bits(scope, gfx::BufferUsageScope::TransferSrc))
  {
    access |= gfx::Access::TransferRead;
  }

  if (has_bits(scope, gfx::BufferUsageScope::TransferDst))
  {
    access |= gfx::Access::TransferWrite;
  }

  return BufferScope{.stages = stages, .access = access};
}

constexpr BufferScope compute_storage_buffer_scope(gfx::BufferUsageScope scope)
{
  gfx::PipelineStages stages = gfx::PipelineStages::ComputeShader;
  gfx::Access         access = gfx::Access::None;

  if (has_bits(scope, gfx::BufferUsageScope::ComputeShaderStorage))
  {
    access |= gfx::Access::ShaderWrite;
  }

  if (has_bits(scope, gfx::BufferUsageScope::ComputeShaderStorageTexel))
  {
    access |= gfx::Access::ShaderWrite;
  }

  if (has_bits(scope, gfx::BufferUsageScope::ComputeShaderUniform))
  {
    access |= gfx::Access::ShaderRead;
  }

  if (has_bits(scope, gfx::BufferUsageScope::ComputeShaderUniformTexel))
  {
    access |= gfx::Access::ShaderRead;
  }

  return BufferScope{.stages = stages, .access = access};
}

// we need a single barrier for stages where the layout is different from the base layout
// (readonlyoptimal) i.e. storage, transfer
//
// only called on dst
// Q(lamarrr): by inserting multiple barriers, will they all execute???
// only one of them will ever get executed because the previous barriers would have drained the work
// they had only executes if there is still work left to be drained???? but there will be multiple
// works to be drained once a work completes and there are multiple barriers waiting on it, will all
// the barriers perform the wait
//, no because this will implicitly depend on any op that performs read, writes, or layout
//transitions
//
// Q(lamarrr): if there's no task that has the required access and stages, will the barrier still
// execute?
//
//
// images and buffers must have a first command that operate on them? to transition them and provide
// access to other commands?
//
// what if no previous operation meets the barrier's requirements? then it will continue executing
// the requesting command
//
// no previous cmd means contents are undefined
//
//
// and if image was never transitioned we should be good and use undefined src layout?
//
// pre -> op -> post
// if image was already on the queue scope takes care of it
// layout+scope -> transfer src|dst optimal
// all ops that have side-effects
// transfer transfer src|dst optimal -> scope + layout???? not needed?
//
// for transfer post-stages, we can omit the barriers, we only need to give barriers to readers
//
//
// Requirements:
// - layout merging:
//      - used as storage and sampled in the same command
//      - sampled and input attachment  in the same command
//      - transfer src and transfer dst in the same command
//
//
// TODO(lamarrr): undefined layout handling
//
// we need: initial layout, operations providing the initial layout and releasing it to other scopes
// for pickup
//
// - easiest and best would be to have an Undefined + Init function that specifies and places
// barriers for the scopes
//
// - they must all have an initial transfer-dst operation? -
// - acquire_init -> transfer -> release
//
// - acquire_init -> render as color attachment -> release
//
// we need a base layout
// just use fill??? even if not specified with initial data
// Scope Layout -> Other Scopes layout
// can't use fill cos we might not know the initial layout
//
// our barriers assume they are coming from one usagescope and transition to same or another
// usagescope initialization is not representable
//
//
//
// release only upon init
//
// Undefined, None Access, TopOfPipe -> scopes, NoAccessMask will not affect accessmasks
// - multiple setup will mean????
//
// release to transfer will be as transfer dst
// release to color attachment will be as write-only non-flushed
//
//
// multiple none-accesses will cause multiple transitions because there will be no dependency chains
//
// the previous acquire barriers only work on
//
//
// initial layout top of pipe, none access, transition layout to initial layout
//
//
// add is_first?
//
//
template <usize Capacity>
inline void acquire_image(gfx::Image image, gfx::ImageUsageScope scope,
                          gfx::PipelineStages dst_stages, gfx::Access dst_access,
                          gfx::ImageLayout new_layout, gfx::ImageAspects aspects,
                          stx::Array<gfx::ImageMemoryBarrier, Capacity> &barriers)
{
  if (has_bits(scope, gfx::ImageUsageScope::TransferSrc | gfx::ImageUsageScope::TransferDst))
  {
    barriers
        .push(gfx::ImageMemoryBarrier{.src_stages = gfx::PipelineStages::Transfer,
                                      .dst_stages = dst_stages,
                                      .src_access =
                                          gfx::Access::TransferRead | gfx::Access::TransferWrite,
                                      .dst_access = dst_access,
                                      .old_layout = gfx::ImageLayout::General,
                                      .new_layout = new_layout,
                                      .image      = image,
                                      .aspects    = aspects})
        .unwrap();
  }
  else if (has_bits(scope, gfx::ImageUsageScope::TransferSrc))
  {
    barriers
        .push(gfx::ImageMemoryBarrier{.src_stages = gfx::PipelineStages::Transfer,
                                      .dst_stages = dst_stages,
                                      .src_access = gfx::Access::TransferRead,
                                      .dst_access = dst_access,
                                      .old_layout = gfx::ImageLayout::TransferSrcOptimal,
                                      .new_layout = new_layout,
                                      .image      = image,
                                      .aspects    = aspects})
        .unwrap();
  }
  else if (has_bits(scope, gfx::ImageUsageScope::TransferDst))
  {
    barriers
        .push(gfx::ImageMemoryBarrier{.src_stages = gfx::PipelineStages::Transfer,
                                      .dst_stages = dst_stages,
                                      .src_access = gfx::Access::TransferWrite,
                                      .dst_access = dst_access,
                                      .old_layout = gfx::ImageLayout::TransferDstOptimal,
                                      .new_layout = new_layout,
                                      .image      = image,
                                      .aspects    = aspects})
        .unwrap();
  }

  // if scope has compute shader write then it will always be transitioned to general
  if (has_bits(scope, gfx::ImageUsageScope::ComputeShaderStorage))
  {
    barriers
        .push(gfx::ImageMemoryBarrier{.src_stages = gfx::PipelineStages::ComputeShader,
                                      .dst_stages = dst_stages,
                                      .src_access =
                                          gfx::Access::ShaderWrite | gfx::Access::ShaderRead,
                                      .dst_access = dst_access,
                                      .old_layout = gfx::ImageLayout::General,
                                      .new_layout = new_layout,
                                      .image      = image,
                                      .aspects    = aspects})
        .unwrap();
  }
  else if (has_bits(scope, gfx::ImageUsageScope::ComputeShaderSampled))
  {
    barriers
        .push(gfx::ImageMemoryBarrier{.src_stages = gfx::PipelineStages::ComputeShader,
                                      .dst_stages = dst_stages,
                                      .src_access = gfx::Access::ShaderRead,
                                      .dst_access = dst_access,
                                      .old_layout = gfx::ImageLayout::ShaderReadOnlyOptimal,
                                      .new_layout = new_layout,
                                      .image      = image,
                                      .aspects    = aspects})
        .unwrap();
  }

  if (has_bits(scope, gfx::ImageUsageScope::VertexShaderSampled))
  {
    barriers
        .push(gfx::ImageMemoryBarrier{.src_stages = gfx::PipelineStages::VertexShader,
                                      .dst_stages = dst_stages,
                                      .src_access = gfx::Access::ShaderRead,
                                      .dst_access = dst_access,
                                      .old_layout = gfx::ImageLayout::ShaderReadOnlyOptimal,
                                      .new_layout = new_layout,
                                      .image      = image,
                                      .aspects    = aspects})
        .unwrap();
  }

  if (has_bits(scope, gfx::ImageUsageScope::FragmentShaderSampled))
  {
    barriers
        .push(gfx::ImageMemoryBarrier{.src_stages = gfx::PipelineStages::FragmentShader,
                                      .dst_stages = dst_stages,
                                      .src_access = gfx::Access::ShaderRead,
                                      .dst_access = dst_access,
                                      .old_layout = gfx::ImageLayout::ShaderReadOnlyOptimal,
                                      .new_layout = new_layout,
                                      .image      = image,
                                      .aspects    = aspects})
        .unwrap();
  }

  if (has_bits(scope, gfx::ImageUsageScope::InputAttachment))
  {
    barriers
        .push(gfx::ImageMemoryBarrier{.src_stages = gfx::PipelineStages::FragmentShader,
                                      .dst_stages = dst_stages,
                                      .src_access = gfx::Access::InputAttachmentRead,
                                      .dst_access = dst_access,
                                      .old_layout = gfx::ImageLayout::ShaderReadOnlyOptimal,
                                      .new_layout = new_layout,
                                      .image      = image,
                                      .aspects    = aspects})
        .unwrap();
  }

  if (has_bits(scope, gfx::ImageUsageScope::ReadColorAttachment |
                          gfx::ImageUsageScope::WriteColorAttachment))
  {
    barriers
        .push(gfx::ImageMemoryBarrier{.src_stages = gfx::PipelineStages::ColorAttachmentOutput,
                                      .dst_stages = dst_stages,
                                      .src_access = gfx::Access::ColorAttachmentRead |
                                                    gfx::Access::ColorAttachmentWrite,
                                      .dst_access = dst_access,
                                      .old_layout = gfx::ImageLayout::ColorAttachmentOptimal,
                                      .new_layout = new_layout,
                                      .image      = image,
                                      .aspects    = aspects})
        .unwrap();
  }
  else if (has_bits(scope, gfx::ImageUsageScope::ReadColorAttachment))
  {
    barriers
        .push(gfx::ImageMemoryBarrier{.src_stages = gfx::PipelineStages::ColorAttachmentOutput,
                                      .dst_stages = dst_stages,
                                      .src_access = gfx::Access::ColorAttachmentRead,
                                      .dst_access = dst_access,
                                      .old_layout = gfx::ImageLayout::ColorAttachmentOptimal,
                                      .new_layout = new_layout,
                                      .image      = image,
                                      .aspects    = aspects})
        .unwrap();
  }
  else if (has_bits(scope, gfx::ImageUsageScope::WriteColorAttachment))
  {
    barriers
        .push(gfx::ImageMemoryBarrier{.src_stages = gfx::PipelineStages::ColorAttachmentOutput,
                                      .dst_stages = dst_stages,
                                      .src_access = gfx::Access::ColorAttachmentWrite,
                                      .dst_access = dst_access,
                                      .old_layout = gfx::ImageLayout::ColorAttachmentOptimal,
                                      .new_layout = new_layout,
                                      .image      = image,
                                      .aspects    = aspects})
        .unwrap();
  }

  if (has_bits(scope, gfx::ImageUsageScope::ReadDepthStencilAttachment |
                          gfx::ImageUsageScope::WriteDepthStencilAttachment))
  {
    barriers
        .push(gfx::ImageMemoryBarrier{.src_stages = gfx::PipelineStages::EarlyFragmentTests |
                                                    gfx::PipelineStages::LateFragmentTests,
                                      .dst_stages = dst_stages,
                                      .src_access = gfx::Access::DepthStencilAttachmentRead |
                                                    gfx::Access::DepthStencilAttachmentWrite,
                                      .dst_access = dst_access,
                                      .old_layout = gfx::ImageLayout::DepthStencilAttachmentOptimal,
                                      .new_layout = new_layout,
                                      .image      = image,
                                      .aspects    = aspects})
        .unwrap();
  }
  else if (has_bits(scope, gfx::ImageUsageScope::ReadDepthStencilAttachment))
  {
    barriers
        .push(gfx::ImageMemoryBarrier{.src_stages = gfx::PipelineStages::EarlyFragmentTests,
                                      .dst_stages = dst_stages,
                                      .src_access = gfx::Access::DepthStencilAttachmentRead,
                                      .dst_access = dst_access,
                                      .old_layout = gfx::ImageLayout::DepthStencilReadOnlyOptimal,
                                      .new_layout = new_layout,
                                      .image      = image,
                                      .aspects    = aspects})
        .unwrap();
  }
  else if (has_bits(scope, gfx::ImageUsageScope::WriteDepthStencilAttachment))
  {
    barriers
        .push(gfx::ImageMemoryBarrier{.src_stages = gfx::PipelineStages::LateFragmentTests,
                                      .dst_stages = dst_stages,
                                      .src_access = gfx::Access::DepthStencilAttachmentWrite,
                                      .dst_access = dst_access,
                                      .old_layout = gfx::ImageLayout::DepthStencilAttachmentOptimal,
                                      .new_layout = new_layout,
                                      .image      = image,
                                      .aspects    = aspects})
        .unwrap();
  }
}

// release side-effects to operations that are allowed to have concurrent non-mutating access on the
// resource
template <usize Capacity>
inline void release_image(gfx::Image image, gfx::ImageUsageScope scope,
                          gfx::PipelineStages src_stages, gfx::Access src_access,
                          gfx::ImageLayout old_layout, gfx::ImageAspects aspects,
                          stx::Array<gfx::ImageMemoryBarrier, Capacity> &barriers)
{
  // only shader-sampled images can run parallel to other command views
  // only transitioned to Shader read only if it is not used as storage at the same stage
  //
  // for all non-shader-read-only-optimal usages, an acquire must be performed
  //
  if (has_bits(scope, gfx::ImageUsageScope::ComputeShaderSampled) &&
      !has_bits(scope, gfx::ImageUsageScope::ComputeShaderStorage))
  {
    barriers
        .push(gfx::ImageMemoryBarrier{.src_stages = src_stages,
                                      .dst_stages = gfx::PipelineStages::ComputeShader,
                                      .src_access = src_access,
                                      .dst_access = gfx::Access::ShaderRead,
                                      .old_layout = old_layout,
                                      .new_layout = gfx::ImageLayout::ShaderReadOnlyOptimal,
                                      .image      = image,
                                      .aspects    = aspects})
        .unwrap();
  }

  if (has_any_bit(scope, gfx::ImageUsageScope::VertexShaderSampled |
                             gfx::ImageUsageScope::FragmentShaderSampled |
                             gfx::ImageUsageScope::InputAttachment))
  {
    gfx::PipelineStages dst_stages = gfx::PipelineStages::None;

    if (has_bits(scope, gfx::ImageUsageScope::VertexShaderSampled))
    {
      dst_stages |= gfx::PipelineStages::VertexShader;
    }

    if (has_any_bit(scope, gfx::ImageUsageScope::FragmentShaderSampled |
                               gfx::ImageUsageScope::InputAttachment))
    {
      dst_stages |= gfx::PipelineStages::FragmentShader;
    }

    barriers
        .push(gfx::ImageMemoryBarrier{.src_stages = src_stages,
                                      .dst_stages = dst_stages,
                                      .src_access = src_access,
                                      .dst_access = gfx::Access::ShaderRead,
                                      .old_layout = old_layout,
                                      .new_layout = gfx::ImageLayout::ShaderReadOnlyOptimal,
                                      .image      = image,
                                      .aspects    = aspects})
        .unwrap();
  }
}

// apply to both src and dst since they require layout transitions
constexpr ImageScope transfer_image_scope(gfx::ImageUsageScope scope)
{
  gfx::ImageLayout    layout = gfx::ImageLayout::Undefined;
  gfx::PipelineStages stages = gfx::PipelineStages::Transfer;
  gfx::Access         access = gfx::Access::None;

  if (has_bits(scope, gfx::ImageUsageScope::TransferSrc | gfx::ImageUsageScope::TransferDst))
  {
    access = gfx::Access::TransferRead | gfx::Access::TransferWrite;
    layout = gfx::ImageLayout::General;
  }
  else if (has_bits(scope, gfx::ImageUsageScope::TransferSrc))
  {
    access = gfx::Access::TransferRead;
    layout = gfx::ImageLayout::TransferSrcOptimal;
  }
  else if (has_bits(scope, gfx::ImageUsageScope::TransferDst))
  {
    access = gfx::Access::TransferWrite;
    layout = gfx::ImageLayout::TransferDstOptimal;
  }

  return ImageScope{.stages = stages, .access = access, .layout = layout};
}

constexpr ImageScope compute_storage_image_scope(gfx::ImageUsageScope scope)
{
  gfx::ImageLayout    layout = gfx::ImageLayout::Undefined;
  gfx::PipelineStages stages = gfx::PipelineStages::ComputeShader;
  gfx::Access         access = gfx::Access::None;

  if (has_bits(scope, gfx::ImageUsageScope::ComputeShaderSampled |
                          gfx::ImageUsageScope::ComputeShaderStorage))
  {
    access = gfx::Access::ShaderRead | gfx::Access::ShaderWrite;
    layout = gfx::ImageLayout::General;
  }
  else if (has_bits(scope, gfx::ImageUsageScope::ComputeShaderSampled))
  {
    access = gfx::Access::ShaderRead;
    layout = gfx::ImageLayout::ShaderReadOnlyOptimal;
  }
  else if (has_bits(scope, gfx::ImageUsageScope::ComputeShaderStorage))
  {
    access = gfx::Access::ShaderWrite;
    layout = gfx::ImageLayout::General;
  }

  return ImageScope{.stages = stages, .access = access, .layout = layout};
}

constexpr ImageScope color_attachment_image_scope(gfx::ImageUsageScope scope)
{
  gfx::ImageLayout    layout = gfx::ImageLayout::ColorAttachmentOptimal;
  gfx::PipelineStages stages = gfx::PipelineStages::ColorAttachmentOutput;
  gfx::Access         access = gfx::Access::None;

  if (has_bits(scope, gfx::ImageUsageScope::ReadColorAttachment))
  {
    access |= gfx::Access::ColorAttachmentRead;
  }

  if (has_bits(scope, gfx::ImageUsageScope::WriteColorAttachment))
  {
    access |= gfx::Access::ColorAttachmentWrite;
  }

  return ImageScope{.stages = stages, .access = access, .layout = layout};
}

constexpr ImageScope depth_stencil_attachment_image_scope(gfx::ImageUsageScope scope)
{
  gfx::ImageLayout    layout = gfx::ImageLayout::Undefined;
  gfx::PipelineStages stages = gfx::PipelineStages::None;
  gfx::Access         access = gfx::Access::None;

  if (has_bits(scope, gfx::ImageUsageScope::ReadDepthStencilAttachment |
                          gfx::ImageUsageScope::WriteDepthStencilAttachment))
  {
    stages = gfx::PipelineStages::EarlyFragmentTests | gfx::PipelineStages::LateFragmentTests;
    access = gfx::Access::DepthStencilAttachmentRead | gfx::Access::DepthStencilAttachmentWrite;
    layout = gfx::ImageLayout::DepthStencilAttachmentOptimal;
  }
  else if (has_bits(scope, gfx::ImageUsageScope::ReadDepthStencilAttachment))
  {
    stages = gfx::PipelineStages::EarlyFragmentTests;
    access = gfx::Access::DepthStencilAttachmentRead;
    layout = gfx::ImageLayout::DepthStencilReadOnlyOptimal;
  }
  else if (has_bits(scope, gfx::ImageUsageScope::WriteDepthStencilAttachment))
  {
    stages = gfx::PipelineStages::LateFragmentTests;
    access = gfx::Access::DepthStencilAttachmentWrite;
    layout = gfx::ImageLayout::DepthStencilAttachmentOptimal;
  }

  return ImageScope{.stages = stages, .access = access, .layout = layout};
}

constexpr ImageScope color_attachment_image_scope(gfx::RenderPassAttachment const &attachment)
{
  gfx::ImageLayout layout = gfx::ImageLayout::ColorAttachmentOptimal;
  gfx::Access      access = gfx::Access::None;

  if (attachment.load_op == gfx::LoadOp::Clear || attachment.load_op == gfx::LoadOp::DontCare ||
      attachment.store_op == gfx::StoreOp::Store)
  {
    access |= gfx::Access::ColorAttachmentWrite;
  }

  if (attachment.load_op == gfx::LoadOp::Load)
  {
    access |= gfx::Access::ColorAttachmentRead;
  }

  return ImageScope{
      .stages = gfx::PipelineStages::ColorAttachmentOutput, .access = access, .layout = layout};
}

constexpr ImageScope
    depth_stencil_attachment_image_scope(gfx::RenderPassAttachment const &attachment)
{
  gfx::ImageLayout    layout = gfx::ImageLayout::Undefined;
  gfx::Access         access = gfx::Access::None;
  gfx::PipelineStages stages = gfx::PipelineStages::None;

  if (attachment.load_op == gfx::LoadOp::Clear || attachment.load_op == gfx::LoadOp::DontCare ||
      attachment.store_op == gfx::StoreOp::Store || attachment.store_op == gfx::StoreOp::DontCare ||
      attachment.stencil_load_op == gfx::LoadOp::Clear ||
      attachment.stencil_load_op == gfx::LoadOp::DontCare ||
      attachment.stencil_store_op == gfx::StoreOp::Store ||
      attachment.stencil_store_op == gfx::StoreOp::DontCare)
  {
    access |= gfx::Access::DepthStencilAttachmentWrite;
    stages |= gfx::PipelineStages::LateFragmentTests;
  }

  if (attachment.load_op == gfx::LoadOp::Load || attachment.stencil_load_op == gfx::LoadOp::Load)
  {
    access |= gfx::Access::DepthStencilAttachmentRead;
    stages |= gfx::PipelineStages::EarlyFragmentTests;
  }

  if (has_bits(access, gfx::Access::ColorAttachmentWrite))
  {
    layout = gfx::ImageLayout::DepthStencilAttachmentOptimal;
  }
  else if (has_bits(access, gfx::Access::ColorAttachmentRead))
  {
    layout = gfx::ImageLayout::DepthStencilReadOnlyOptimal;
  }

  return ImageScope{.stages = stages, .access = access, .layout = layout};
}

void CommandBuffer::fill_buffer(gfx::Buffer dst, u64 offset, u64 size, u32 data)
{
  hook->fill_buffer(dst, offset, size, data);

  BufferScope scope = transfer_buffer_scope(graph->buffers[dst].desc.scope);

  tmp_buffer_barriers.clear();
  acquire_buffer(graph->to_rhi(dst), graph->buffers[dst].desc.scope, scope.stages, scope.access,
                 tmp_buffer_barriers);

  driver->cmd_insert_barriers(rhi, tmp_buffer_barriers, {});

  driver->cmd_fill_buffer(rhi, graph->to_rhi(dst), offset, size, data);

  tmp_buffer_barriers.clear();
  release_buffer(graph->to_rhi(dst), graph->buffers[dst].desc.scope, scope.stages, scope.access,
                 tmp_buffer_barriers);

  driver->cmd_insert_barriers(rhi, tmp_buffer_barriers, {});
}

void CommandBuffer::copy_buffer(gfx::Buffer src, gfx::Buffer dst,
                                stx::Span<gfx::BufferCopy const> copies)
{
  hook->copy_buffer(src, dst, copies);

  BufferScope src_scope = transfer_buffer_scope(graph->buffers[src].desc.scope);
  BufferScope dst_scope = transfer_buffer_scope(graph->buffers[dst].desc.scope);

  tmp_buffer_barriers.clear();

  acquire_buffer(graph->to_rhi(src), graph->buffers[src].desc.scope, src_scope.stages,
                 src_scope.access, tmp_buffer_barriers);
  acquire_buffer(graph->to_rhi(dst), graph->buffers[dst].desc.scope, dst_scope.stages,
                 dst_scope.access, tmp_buffer_barriers);

  driver->cmd_insert_barriers(rhi, tmp_buffer_barriers, {});

  driver->cmd_copy_buffer(rhi, graph->to_rhi(src), graph->to_rhi(dst), copies);

  tmp_buffer_barriers.clear();

  release_buffer(graph->to_rhi(src), graph->buffers[src].desc.scope, src_scope.stages,
                 src_scope.access, tmp_buffer_barriers);
  release_buffer(graph->to_rhi(dst), graph->buffers[dst].desc.scope, dst_scope.stages,
                 dst_scope.access, tmp_buffer_barriers);

  driver->cmd_insert_barriers(rhi, tmp_buffer_barriers, {});
}

void CommandBuffer::update_buffer(stx::Span<u8 const> src, u64 dst_offset, gfx::Buffer dst)
{
  hook->update_buffer(src, dst_offset, dst);

  BufferScope scope = transfer_buffer_scope(graph->buffers[dst].desc.scope);

  tmp_buffer_barriers.clear();

  acquire_buffer(graph->to_rhi(dst), graph->buffers[dst].desc.scope, scope.stages, scope.access,
                 tmp_buffer_barriers);

  driver->cmd_insert_barriers(rhi, tmp_buffer_barriers, {});

  driver->cmd_update_buffer(rhi, src, dst_offset, graph->to_rhi(dst));

  tmp_buffer_barriers.clear();
  release_buffer(graph->to_rhi(dst), graph->buffers[dst].desc.scope, scope.stages, scope.access,
                 tmp_buffer_barriers);

  driver->cmd_insert_barriers(rhi, tmp_buffer_barriers, {});
}

void CommandBuffer::clear_color_image(gfx::Image dst, stx::Span<gfx::Color const> clear_colors,
                                      stx::Span<gfx::ImageSubresourceRange const> ranges)

{
  hook->clear_color_image(dst, clear_colors, ranges);

  ImageScope scope = transfer_image_scope(graph->images[dst].desc.scope);

  tmp_image_barriers.clear();

  acquire_image(graph->to_rhi(dst), graph->images[dst].desc.scope, scope.stages, scope.access,
                scope.layout, graph->images[dst].desc.aspects, tmp_image_barriers);

  driver->cmd_insert_barriers(rhi, {}, tmp_image_barriers);

  driver->cmd_clear_color_image(rhi, graph->to_rhi(dst), clear_colors, ranges);

  tmp_image_barriers.clear();

  release_image(graph->to_rhi(dst), graph->images[dst].desc.scope, scope.stages, scope.access,
                scope.layout, graph->images[dst].desc.aspects, tmp_image_barriers);

  driver->cmd_insert_barriers(rhi, {}, tmp_image_barriers);
}

void CommandBuffer::clear_depth_stencil_image(
    gfx::Image dst, stx::Span<gfx::DepthStencil const> clear_depth_stencils,
    stx::Span<gfx::ImageSubresourceRange const> ranges)

{
  hook->clear_depth_stencil_image(dst, clear_depth_stencils, ranges);

  ImageScope scope = transfer_image_scope(graph->images[dst].desc.scope);

  tmp_image_barriers.clear();

  acquire_image(graph->to_rhi(dst), graph->images[dst].desc.scope, scope.stages, scope.access,
                scope.layout, graph->images[dst].desc.aspects, tmp_image_barriers);

  driver->cmd_insert_barriers(rhi, {}, tmp_image_barriers);

  driver->cmd_clear_depth_stencil_image(rhi, graph->to_rhi(dst), clear_depth_stencils, ranges);

  tmp_image_barriers.clear();

  release_image(graph->to_rhi(dst), graph->images[dst].desc.scope, scope.stages, scope.access,
                scope.layout, graph->images[dst].desc.aspects, tmp_image_barriers);

  driver->cmd_insert_barriers(rhi, {}, tmp_image_barriers);
}

void CommandBuffer::copy_image(gfx::Image src, gfx::Image dst,
                               stx::Span<gfx::ImageCopy const> copies)
{
  hook->copy_image(src, dst, copies);

  ImageScope src_scope = transfer_image_scope(graph->images[src].desc.scope);
  ImageScope dst_scope = transfer_image_scope(graph->images[dst].desc.scope);

  tmp_image_barriers.clear();

  acquire_image(graph->to_rhi(src), graph->images[src].desc.scope, src_scope.stages,
                src_scope.access, src_scope.layout, graph->images[src].desc.aspects,
                tmp_image_barriers);
  acquire_image(graph->to_rhi(dst), graph->images[dst].desc.scope, dst_scope.stages,
                dst_scope.access, dst_scope.layout, graph->images[dst].desc.aspects,
                tmp_image_barriers);

  driver->cmd_insert_barriers(rhi, {}, tmp_image_barriers);

  driver->cmd_copy_image(rhi, src, dst, copies);

  tmp_image_barriers.clear();

  release_image(graph->to_rhi(src), graph->images[src].desc.scope, src_scope.stages,
                src_scope.access, src_scope.layout, graph->images[src].desc.aspects,
                tmp_image_barriers);
  release_image(graph->to_rhi(dst), graph->images[dst].desc.scope, dst_scope.stages,
                dst_scope.access, dst_scope.layout, graph->images[dst].desc.aspects,
                tmp_image_barriers);

  driver->cmd_insert_barriers(rhi, {}, tmp_image_barriers);
}

void CommandBuffer::copy_buffer_to_image(gfx::Buffer src, gfx::Image dst,
                                         stx::Span<gfx::BufferImageCopy const> copies)
{
  hook->copy_buffer_to_image(src, dst, copies);

  BufferScope src_scope = transfer_buffer_scope(graph->buffers[src].desc.scope);
  ImageScope  dst_scope = transfer_image_scope(graph->images[dst].desc.scope);

  tmp_buffer_barriers.clear();
  tmp_image_barriers.clear();

  acquire_buffer(graph->to_rhi(src), graph->buffers[src].desc.scope, src_scope.stages,
                 src_scope.access, tmp_buffer_barriers);
  acquire_image(graph->to_rhi(dst), graph->images[dst].desc.scope, dst_scope.stages,
                dst_scope.access, dst_scope.layout, graph->images[dst].desc.aspects,
                tmp_image_barriers);

  driver->cmd_insert_barriers(rhi, tmp_buffer_barriers, tmp_image_barriers);

  driver->cmd_copy_buffer_to_image(rhi, src, dst, copies);

  tmp_buffer_barriers.clear();
  tmp_image_barriers.clear();

  release_buffer(graph->to_rhi(src), graph->buffers[src].desc.scope, src_scope.stages,
                 src_scope.access, tmp_buffer_barriers);
  release_image(graph->to_rhi(dst), graph->images[dst].desc.scope, dst_scope.stages,
                dst_scope.access, dst_scope.layout, graph->images[dst].desc.aspects,
                tmp_image_barriers);

  driver->cmd_insert_barriers(rhi, tmp_buffer_barriers, tmp_image_barriers);
}

void CommandBuffer::blit_image(gfx::Image src, gfx::Image dst,
                               stx::Span<gfx::ImageBlit const> blits, gfx::Filter filter)
{
  hook->blit_image(src, dst, blits, filter);

  ImageScope src_scope = transfer_image_scope(graph->images[src].desc.scope);
  ImageScope dst_scope = transfer_image_scope(graph->images[dst].desc.scope);

  tmp_image_barriers.clear();

  acquire_image(graph->to_rhi(src), graph->images[src].desc.scope, src_scope.stages,
                src_scope.access, src_scope.layout, graph->images[src].desc.aspects,
                tmp_image_barriers);
  acquire_image(graph->to_rhi(dst), graph->images[dst].desc.scope, dst_scope.stages,
                dst_scope.access, dst_scope.layout, graph->images[dst].desc.aspects,
                tmp_image_barriers);

  driver->cmd_insert_barriers(rhi, {}, tmp_image_barriers);

  driver->cmd_blit_image(rhi, src, dst, blits, filter);

  tmp_image_barriers.clear();

  release_image(graph->to_rhi(src), graph->images[src].desc.scope, src_scope.stages,
                src_scope.access, src_scope.layout, graph->images[src].desc.aspects,
                tmp_image_barriers);
  release_image(graph->to_rhi(dst), graph->images[dst].desc.scope, dst_scope.stages,
                dst_scope.access, dst_scope.layout, graph->images[dst].desc.aspects,
                tmp_image_barriers);

  driver->cmd_insert_barriers(rhi, {}, tmp_image_barriers);
}

void CommandBuffer::begin_render_pass(
    gfx::Framebuffer framebuffer, gfx::RenderPass render_pass, IRect render_area,
    stx::Span<gfx::Color const>        color_attachments_clear_values,
    stx::Span<gfx::DepthStencil const> depth_stencil_attachments_clear_values)
{
  hook->begin_render_pass(framebuffer, render_pass, render_area, color_attachments_clear_values,
                          depth_stencil_attachments_clear_values);
  this->framebuffer = framebuffer;
  this->render_pass = render_pass;

  for (gfx::ImageView view : graph->framebuffers[framebuffer].desc.color_attachments)
  {
    if (view == nullptr)
    {
      continue;
    }
    tmp_image_barriers.clear();
    gfx::ImageResource const &image_resource = graph->images[graph->image_views[view].desc.image];
    ImageScope                scope = color_attachment_image_scope(image_resource.desc.scope);
    acquire_image(image_resource.handle, image_resource.desc.scope, scope.stages, scope.access,
                  scope.layout, image_resource.desc.aspects, tmp_image_barriers);
    driver->cmd_insert_barriers(rhi, {}, tmp_image_barriers);
  }

  {
    gfx::ImageView view = graph->framebuffers[framebuffer].desc.depth_stencil_attachment;
    if (view != nullptr)
    {
      tmp_image_barriers.clear();
      gfx::ImageResource const &image_resource = graph->images[graph->image_views[view].desc.image];
      ImageScope scope = depth_stencil_attachment_image_scope(image_resource.desc.scope);
      acquire_image(image_resource.handle, image_resource.desc.scope, scope.stages, scope.access,
                    scope.layout, image_resource.desc.aspects, tmp_image_barriers);
      driver->cmd_insert_barriers(rhi, {}, tmp_image_barriers);
    }
  }

  driver->cmd_begin_render_pass(
      rhi, graph->framebuffers[framebuffer].handle, graph->render_passes[render_pass].handle,
      render_area, color_attachments_clear_values, depth_stencil_attachments_clear_values);
}

void CommandBuffer::end_render_pass()
{
  hook->end_render_pass();
  driver->cmd_end_render_pass(rhi);

  for (gfx::ImageView view : graph->framebuffers[framebuffer].desc.color_attachments)
  {
    if (view == nullptr)
    {
      continue;
    }
    tmp_image_barriers.clear();
    gfx::ImageResource const &image_resource = graph->images[graph->image_views[view].desc.image];
    ImageScope                scope = color_attachment_image_scope(image_resource.desc.scope);
    release_image(image_resource.handle, image_resource.desc.scope, scope.stages, scope.access,
                  scope.layout, image_resource.desc.aspects, tmp_image_barriers);
    driver->cmd_insert_barriers(rhi, {}, tmp_image_barriers);
  }

  {
    gfx::ImageView view = graph->framebuffers[framebuffer].desc.depth_stencil_attachment;
    if (view != nullptr)
    {
      tmp_image_barriers.clear();
      gfx::ImageResource const &image_resource = graph->images[graph->image_views[view].desc.image];
      ImageScope scope = depth_stencil_attachment_image_scope(image_resource.desc.scope);
      release_image(image_resource.handle, image_resource.desc.scope, scope.stages, scope.access,
                    scope.layout, image_resource.desc.aspects, tmp_image_barriers);
      driver->cmd_insert_barriers(rhi, {}, tmp_image_barriers);
    }
  }
}

void CommandBuffer::dispatch(gfx::ComputePipeline pipeline, u32 group_count_x, u32 group_count_y,
                             u32 group_count_z, gfx::DescriptorSetBindings const &bindings,
                             stx::Span<u8 const> push_constants_data)
{
  hook->dispatch(pipeline, group_count_x, group_count_y, group_count_z, bindings,
                 push_constants_data);
  for (gfx::StorageImageBinding const &binding : bindings.storage_images)
  {
    tmp_image_barriers.clear();
    gfx::ImageResource const &resource =
        graph->images[graph->image_views[binding.image_view].desc.image];
    ImageScope scope = compute_storage_image_scope(resource.desc.scope);
    acquire_image(resource.handle, resource.desc.scope, scope.stages, scope.access, scope.layout,
                  resource.desc.aspects, tmp_image_barriers);
    driver->cmd_insert_barriers(rhi, {}, tmp_image_barriers);
  }

  for (gfx::StorageTexelBufferBinding const &binding : bindings.storage_texel_buffers)
  {
    tmp_buffer_barriers.clear();
    gfx::BufferResource const &resource =
        graph->buffers[graph->buffer_views[binding.buffer_view].desc.buffer];
    BufferScope scope = compute_storage_buffer_scope(resource.desc.scope);
    acquire_buffer(resource.handle, resource.desc.scope, scope.stages, scope.access,
                   tmp_buffer_barriers);
    driver->cmd_insert_barriers(rhi, tmp_buffer_barriers, {});
  }

  for (gfx::StorageBufferBinding const &binding : bindings.storage_buffers)
  {
    tmp_buffer_barriers.clear();
    gfx::BufferResource const &resource = graph->buffers[binding.buffer];
    BufferScope                scope    = compute_storage_buffer_scope(resource.desc.scope);
    acquire_buffer(resource.handle, resource.desc.scope, scope.stages, scope.access,
                   tmp_buffer_barriers);
    driver->cmd_insert_barriers(rhi, tmp_buffer_barriers, {});
  }

  driver->cmd_dispatch(rhi, pipeline, group_count_x, group_count_y, group_count_z, bindings,
                       push_constants_data);

  for (gfx::StorageImageBinding const &binding : bindings.storage_images)
  {
    tmp_image_barriers.clear();
    gfx::ImageResource const &resource =
        graph->images[graph->image_views[binding.image_view].desc.image];
    ImageScope scope = compute_storage_image_scope(resource.desc.scope);
    release_image(resource.handle, resource.desc.scope, scope.stages, scope.access, scope.layout,
                  resource.desc.aspects, tmp_image_barriers);
    driver->cmd_insert_barriers(rhi, {}, tmp_image_barriers);
  }

  for (gfx::StorageTexelBufferBinding const &binding : bindings.storage_texel_buffers)
  {
    tmp_buffer_barriers.clear();
    gfx::BufferResource const &resource =
        graph->buffers[graph->buffer_views[binding.buffer_view].desc.buffer];
    BufferScope scope = compute_storage_buffer_scope(resource.desc.scope);
    release_buffer(resource.handle, resource.desc.scope, scope.stages, scope.access,
                   tmp_buffer_barriers);
    driver->cmd_insert_barriers(rhi, tmp_buffer_barriers, {});
  }

  for (gfx::StorageBufferBinding const &binding : bindings.storage_buffers)
  {
    tmp_buffer_barriers.clear();
    gfx::BufferResource const &resource = graph->buffers[binding.buffer];
    BufferScope                scope    = compute_storage_buffer_scope(resource.desc.scope);
    release_buffer(resource.handle, resource.desc.scope, scope.stages, scope.access,
                   tmp_buffer_barriers);
    driver->cmd_insert_barriers(rhi, tmp_buffer_barriers, {});
  }
}

void CommandBuffer::dispatch_indirect(gfx::ComputePipeline pipeline, gfx::Buffer buffer, u64 offset,
                                      gfx::DescriptorSetBindings const &bindings,
                                      stx::Span<u8 const>               push_constants_data)
{
  hook->dispatch_indirect(pipeline, buffer, offset, bindings, push_constants_data);
  for (gfx::StorageImageBinding const &binding : bindings.storage_images)
  {
    tmp_image_barriers.clear();
    gfx::ImageResource const &resource =
        graph->images[graph->image_views[binding.image_view].desc.image];
    ImageScope scope = compute_storage_image_scope(resource.desc.scope);
    acquire_image(resource.handle, resource.desc.scope, scope.stages, scope.access, scope.layout,
                  resource.desc.aspects, tmp_image_barriers);
    driver->cmd_insert_barriers(rhi, {}, tmp_image_barriers);
  }

  for (gfx::StorageTexelBufferBinding const &binding : bindings.storage_texel_buffers)
  {
    tmp_buffer_barriers.clear();
    gfx::BufferResource const &resource =
        graph->buffers[graph->buffer_views[binding.buffer_view].desc.buffer];
    BufferScope scope = compute_storage_buffer_scope(resource.desc.scope);
    acquire_buffer(resource.handle, resource.desc.scope, scope.stages, scope.access,
                   tmp_buffer_barriers);
    driver->cmd_insert_barriers(rhi, tmp_buffer_barriers, {});
  }

  for (gfx::StorageBufferBinding const &binding : bindings.storage_buffers)
  {
    tmp_buffer_barriers.clear();
    gfx::BufferResource const &resource = graph->buffers[binding.buffer];
    BufferScope                scope    = compute_storage_buffer_scope(resource.desc.scope);
    acquire_buffer(resource.handle, resource.desc.scope, scope.stages, scope.access,
                   tmp_buffer_barriers);
    driver->cmd_insert_barriers(rhi, tmp_buffer_barriers, {});
  }

  driver->cmd_dispatch_indirect(rhi, graph->to_rhi(pipeline), graph->to_rhi(buffer), offset,
                                bindings, push_constants_data);

  for (gfx::StorageImageBinding const &binding : bindings.storage_images)
  {
    tmp_image_barriers.clear();
    gfx::ImageResource const &resource =
        graph->images[graph->image_views[binding.image_view].desc.image];
    ImageScope scope = compute_storage_image_scope(resource.desc.scope);
    release_image(resource.handle, resource.desc.scope, scope.stages, scope.access, scope.layout,
                  resource.desc.aspects, tmp_image_barriers);
    driver->cmd_insert_barriers(rhi, {}, tmp_image_barriers);
  }

  for (gfx::StorageTexelBufferBinding const &binding : bindings.storage_texel_buffers)
  {
    tmp_buffer_barriers.clear();
    gfx::BufferResource const &resource =
        graph->buffers[graph->buffer_views[binding.buffer_view].desc.buffer];
    BufferScope scope = compute_storage_buffer_scope(resource.desc.scope);
    release_buffer(resource.handle, resource.desc.scope, scope.stages, scope.access,
                   tmp_buffer_barriers);
    driver->cmd_insert_barriers(rhi, tmp_buffer_barriers, {});
  }

  for (gfx::StorageBufferBinding const &binding : bindings.storage_buffers)
  {
    tmp_buffer_barriers.clear();
    gfx::BufferResource const &resource = graph->buffers[binding.buffer];
    BufferScope                scope    = compute_storage_buffer_scope(resource.desc.scope);
    release_buffer(resource.handle, resource.desc.scope, scope.stages, scope.access,
                   tmp_buffer_barriers);
    driver->cmd_insert_barriers(rhi, tmp_buffer_barriers, {});
  }
}

void CommandBuffer::draw(gfx::GraphicsPipeline pipeline, gfx::RenderState const &state,
                         stx::Span<gfx::Buffer const> vertex_buffers, gfx::Buffer index_buffer,
                         u32 first_index, u32 num_indices, u32 vertex_offset, u32 first_instance,
                         u32 num_instances, gfx::DescriptorSetBindings const &bindings,
                         stx::Span<u8 const> push_constants_data)
{
  hook->draw(pipeline, state, vertex_buffers, index_buffer, first_index, num_indices, vertex_offset,
             first_instance, num_instances, bindings, push_constants_data);
  driver->cmd_draw(rhi, pipeline, state, vertex_buffers, index_buffer, first_index, num_indices,
                   vertex_offset, first_instance, num_instances, bindings, push_constants_data);
}

void CommandBuffer::draw_indirect(gfx::GraphicsPipeline pipeline, gfx::RenderState const &state,
                                  stx::Span<gfx::Buffer const> vertex_buffers,
                                  gfx::Buffer index_buffer, gfx::Buffer buffer, u64 offset,
                                  u32 draw_count, u32 stride,
                                  gfx::DescriptorSetBindings const &bindings,
                                  stx::Span<u8 const>               push_constants_data)
{
  hook->draw_indirect(pipeline, state, vertex_buffers, index_buffer, buffer, offset, draw_count,
                      stride, bindings, push_constants_data);
  driver->cmd_draw_indirect(rhi, pipeline, state, vertex_buffers, index_buffer, buffer, offset,
                            draw_count, stride, bindings, push_constants_data);
}

void CommandBuffer::on_execution_complete_fn(stx::UniqueFn<void()> &&fn)
{
  hook->on_execution_complete_fn(fn);
  completion_tasks.push(std::move(fn)).unwrap();
}

}        // namespace rcg
}        // namespace ash
