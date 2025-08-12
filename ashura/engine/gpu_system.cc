/// SPDX-License-Identifier: MIT
#include "ashura/engine/gpu_system.h"
#include "ashura/std/range.h"
#include "ashura/std/sformat.h"
#include "ashura/std/str.h"
#include "ashura/std/trace.h"

namespace ash
{

void ColorTexture::uninit(gpu::Device device)
{
  device->uninit(sampled_texture);
  device->uninit(storage_texture);
  device->uninit(input_attachment);
  device->uninit(view);
  device->uninit(image);
}

void ColorMsaaTexture::uninit(gpu::Device device)
{
  device->uninit(view);
  device->uninit(image);
}

void DepthStencilTexture::uninit(gpu::Device device)
{
  device->uninit(depth_sampled_texture);
  device->uninit(depth_storage_texture);
  device->uninit(depth_input_attachment);
  device->uninit(depth_view);
  device->uninit(stencil_view);
  device->uninit(image);
}

void Framebuffer::uninit(gpu::Device device)
{
  color.uninit(device);
  color_msaa.match([&](auto & c) { c.uninit(device); });
  depth_stencil.uninit(device);
}

void GpuBuffer::uninit(gpu::Device device)
{
  device->uninit(const_buffer);
  device->uninit(read_struct_buffer);
  device->uninit(read_write_struct_buffer);
  device->uninit(buffer);
}

GpuBuffer GpuBuffer::create(gpu::Device device, GpuSystem const & sys,
                            u64 capacity, gpu::BufferUsage usage, Str label,
                            Allocator scratch)
{
  auto buffer_label =
    sformat(scratch, "{} / {}"_str, label, "Buffer"_str).unwrap();
  auto buffer =
    device
      ->create_buffer(gpu::BufferInfo{.label       = buffer_label,
                                      .size        = capacity,
                                      .usage       = usage,
                                      .memory_type = gpu::MemoryType::Unique,
                                      .host_mapped = true})
      .unwrap();

  auto make_set = [&](Str component, gpu::DescriptorSetLayout layout) {
    auto set_label = sformat(scratch, "{} / {}"_str, label, component).unwrap();
    auto set =
      device
        ->create_descriptor_set(gpu::DescriptorSetInfo{
          .label = set_label, .layout = layout, .variable_lengths = {}})
        .unwrap();

    device->update_descriptor_set(gpu::DescriptorSetUpdate{
      .set           = set,
      .binding       = 0,
      .first_element = 0,
      .buffers       = span(
        {gpu::BufferBinding{.buffer = buffer, .range{0, gpu::WHOLE_SIZE}}}
                )
    });

    return set;
  };

  auto const_buffer = make_set("ConstBuffer", sys.layouts_.const_buffer);
  auto read_struct_buffer =
    make_set("ReadStructBuffer", sys.layouts_.read_struct_buffer);
  auto read_write_struct_buffer =
    make_set("ReadWriteStructBuffer", sys.layouts_.read_write_struct_buffer);

  return GpuBuffer{.capacity                 = capacity,
                   .usage                    = usage,
                   .buffer                   = buffer,
                   .const_buffer             = const_buffer,
                   .read_struct_buffer       = read_struct_buffer,
                   .read_write_struct_buffer = read_write_struct_buffer};
}

void GpuQueries::uninit(gpu::Device device)
{
  device->uninit(timestamps);
  device->uninit(statistics);
}

Option<Tuple<gpu::TimestampQuery, u32>> GpuQueries::allocate_timestamp()
{
  if (next_timestamp >= cpu_timestamps.size())
  {
    return none;
  }

  auto idx = next_timestamp++;

  return Tuple{timestamps, idx};
}

Option<Tuple<gpu::StatisticsQuery, u32>> GpuQueries::allocate_statistics()
{
  if (next_statistics >= cpu_statistics.size())
  {
    return none;
  }

  auto idx = next_statistics++;

  return Tuple{statistics, idx};
}

void GpuQueries::reset()
{
  next_timestamp  = 0;
  next_statistics = 0;
}

GpuQueries GpuQueries::create(Allocator allocator, gpu::Device device,
                              Span<char const> label, f32 time_period,
                              u32 timestamps_capacity, u32 statistics_capacity,
                              Allocator scratch)
{
  CHECK(timestamps_capacity != 0, "");
  CHECK(statistics_capacity != 0, "");

  auto timestamp_label =
    sformat(scratch, "{} / TimestampQuery", label).unwrap();
  auto timestamps = device
                      ->create_timestamp_query(gpu::TimestampQueryInfo{
                        .label = timestamp_label, .count = timestamps_capacity})
                      .unwrap();

  Vec<u64> cpu_timestamps{allocator};
  cpu_timestamps.resize_uninit(timestamps_capacity).unwrap();

  auto statistics_label =
    sformat(scratch, "{} / StatisticsQuery", label).unwrap();
  auto statistics =
    device
      ->create_statistics_query(gpu::StatisticsQueryInfo{
        .label = statistics_label, .count = statistics_capacity})
      .unwrap();

  Vec<gpu::PipelineStatistics> cpu_statistics{allocator};
  cpu_statistics.resize_uninit(statistics_capacity).unwrap();

  return GpuQueries{.time_period     = time_period,
                    .timestamps      = timestamps,
                    .next_timestamp  = 0,
                    .statistics      = statistics,
                    .next_statistics = 0,
                    .cpu_timestamps  = std::move(cpu_timestamps),
                    .cpu_statistics  = std::move(cpu_statistics)};
}

void GpuDescriptorsLayout::uninit(gpu::Device device)
{
  device->uninit(samplers);
  device->uninit(sampled_textures);
  device->uninit(storage_textures);
  device->uninit(const_buffer);
  device->uninit(read_struct_buffer);
  device->uninit(read_write_struct_buffer);
  device->uninit(const_buffers);
  device->uninit(read_struct_buffers);
  device->uninit(read_write_struct_buffers);
  device->uninit(input_attachments);
}

void GpuDescriptors::uninit(gpu::Device device)
{
  device->uninit(samplers);
  device->uninit(sampled_textures);
}

void GpuFramePlan::reserve_scratch_buffer(u64 size)
{
  // [ ] atomicSize alignment if overlapping regions; buffers need to shrink when not using enough; i.e.
  // after building frame and noticed that not using enough of it to justify the size
  // [ ] need multiple channels to ensure good overlap
  // [ ] aliasing effect when read and written from same pass
  // [ ] dynamic sync partitioning?

  // [ ] specify for a specifc pass, the discrete offsets that will be used. as many layers as the maximum? or just any number?
  // [ ] use a generalizable abstraction that can extend to images

  scratch_buffer_reserve_size_ = max(size, scratch_buffer_reserve_size_);
}

void GpuFramePlan::add_preframe_task(GpuFrameTask && task)
{
  pre_frame_tasks_.push(std::move(task)).unwrap();
}

void GpuFramePlan::add_postframe_task(GpuFrameTask && task)
{
  post_frame_tasks_.push(std::move(task)).unwrap();
}

void GpuFramePlan::add_pass(GpuPass && pass)
{
  passes_.push(std::move(pass)).unwrap();
}

BufferId GpuFramePlan::push_cpu(Span<u8 const> data)
{
  auto offset = cpu_buffer_data_.size();
  cpu_buffer_data_.extend(data).unwrap();
  auto size = data.size();
  auto idx  = cpu_buffer_entries_.size();
  CHECK(cpu_buffer_data_.size() <= U32_MAX, "");
  cpu_buffer_entries_.push(offset, size).unwrap();
  auto aligned_size =
    align_offset<usize>(SIMD_ALIGNMENT, cpu_buffer_data_.size());
  cpu_buffer_data_.resize_uninit(aligned_size).unwrap();
  return BufferId{(u32) idx};
}

GpuBufferId GpuFramePlan::push_gpu(Span<u8 const> data)
{
  auto offset = gpu_buffer_data_.size();
  gpu_buffer_data_.extend(data).unwrap();
  auto size = data.size();
  auto idx  = gpu_buffer_entries_.size();
  CHECK(gpu_buffer_data_.size() <= U32_MAX, "");
  gpu_buffer_entries_.push(offset, size).unwrap();
  auto aligned_size =
    align_offset<usize>(gpu::BUFFER_OFFSET_ALIGNMENT, gpu_buffer_data_.size());
  gpu_buffer_data_.resize_uninit(aligned_size).unwrap();

  return GpuBufferId{(u32) idx};
}

void GpuFramePlan::begin()
{
}

void GpuFramePlan::end()
{
}

void GpuFramePlan::reset()
{
  // these buffers are expected to be very large so we reset them on every frame when they aren't being used
  // Target at least 75% utilization
  pre_frame_tasks_.shrink_clear().unwrap();
  post_frame_tasks_.shrink_clear().unwrap();
  frame_completed_tasks_.shrink_clear().unwrap();
  gpu_buffer_data_.shrink_clear().unwrap();
  gpu_buffer_entries_.shrink_clear().unwrap();
  cpu_buffer_data_.shrink_clear().unwrap();
  cpu_buffer_entries_.shrink_clear().unwrap();
  scratch_buffer_reserve_size_ = 0;
  passes_.shrink_clear().unwrap();
  arena_.reclaim();
}

void TextureUnion::uninit(gpu::Device device)
{
  color.uninit(device);
  depth_stencil.uninit(device);
}

TextureUnion TextureUnion::create(gpu::Device device, GpuSystem const & sys,
                                  u32x2 target_size, gpu::Format color_format,
                                  gpu::Format depth_stencil_format, Str label,
                                  Allocator scratch)
{
  auto tag = [&](auto component) {
    return sformat(scratch, "{} / {}"_str, label, component).unwrap();
  };

  auto color_label = tag("Color Image"_str);

  auto color_info = gpu::ImageInfo{.label        = color_label,
                                   .type         = gpu::ImageType::Type2D,
                                   .format       = color_format,
                                   .usage        = ColorTexture::USAGE,
                                   .aspects      = gpu::ImageAspects::Color,
                                   .extent       = target_size.append(1),
                                   .mip_levels   = 1,
                                   .array_layers = 1,
                                   .sample_count = gpu::SampleCount::C1,
                                   .memory_type  = gpu::MemoryType::Group};

  auto color_image = device->create_image(color_info).unwrap();

  auto color_view_label = tag("Color Image View"_str);

  auto color_view_info = gpu::ImageViewInfo{
    .label        = color_view_label,
    .image        = color_image,
    .view_type    = gpu::ImageViewType::Type2D,
    .view_format  = color_format,
    .mapping      = {},
    .aspects      = gpu::ImageAspects::Color,
    .mip_levels   = {0, 1},
    .array_layers = {0, 1}
  };

  auto color_image_view = device->create_image_view(color_view_info).unwrap();

  auto color_sampled_texture_label = tag("Color Sampled Texture"_str);
  auto color_sampled_texture       = device
                                 ->create_descriptor_set(gpu::DescriptorSetInfo{
                                   .label  = color_sampled_texture_label,
                                   .layout = sys.layouts_.sampled_textures,
                                   .variable_lengths = span({1U})})
                                 .unwrap();

  auto color_storage_texture_label = tag("Color Storage Texture"_str);
  auto color_storage_texture       = device
                                 ->create_descriptor_set(gpu::DescriptorSetInfo{
                                   .label  = color_storage_texture_label,
                                   .layout = sys.layouts_.storage_textures,
                                   .variable_lengths = span({1U})})
                                 .unwrap();

  auto color_input_attachment_label = tag("Color Input Attachment"_str);
  auto color_input_attachment =
    device
      ->create_descriptor_set(
        gpu::DescriptorSetInfo{.label  = color_input_attachment_label,
                               .layout = sys.layouts_.input_attachments,
                               .variable_lengths = span({1U})})
      .unwrap();

  color_info.label      = {};
  color_view_info.label = {};

  auto color = ColorTexture{.info             = color_info,
                            .view_info        = color_view_info,
                            .image            = color_image,
                            .view             = color_image_view,
                            .sampled_texture  = color_sampled_texture,
                            .storage_texture  = color_storage_texture,
                            .input_attachment = color_input_attachment};

  auto depth_stencil_label = tag("Depth Stencil Image"_str);
  auto depth_stencil_info  = gpu::ImageInfo{
     .label        = depth_stencil_label,
     .type         = gpu::ImageType::Type2D,
     .format       = depth_stencil_format,
     .usage        = ColorTexture::USAGE,
     .aspects      = gpu::ImageAspects::Depth | gpu::ImageAspects::Stencil,
     .extent       = target_size.append(1),
     .mip_levels   = 1,
     .array_layers = 1,
     .sample_count = gpu::SampleCount::C1,
     .memory_type  = gpu::MemoryType::Group};

  auto depth_stencil_image = device->create_image(depth_stencil_info).unwrap();

  auto depth_view_label = tag("Depth Image View"_str);
  auto depth_view_info  = gpu::ImageViewInfo{
     .label        = depth_view_label,
     .image        = depth_stencil_image,
     .view_type    = gpu::ImageViewType::Type2D,
     .view_format  = depth_stencil_format,
     .mapping      = {},
     .aspects      = gpu::ImageAspects::Depth,
     .mip_levels   = {0, 1},
     .array_layers = {0, 1}
  };

  auto depth_image_view = device->create_image_view(depth_view_info).unwrap();

  auto stencil_view_label = tag("Stencil Image View"_str);
  auto stencil_view_info  = gpu::ImageViewInfo{
     .label        = stencil_view_label,
     .image        = depth_stencil_image,
     .view_type    = gpu::ImageViewType::Type2D,
     .view_format  = depth_stencil_format,
     .mapping      = {},
     .aspects      = gpu::ImageAspects::Stencil,
     .mip_levels   = {0, 1},
     .array_layers = {0, 1}
  };

  auto stencil_image_view =
    device->create_image_view(stencil_view_info).unwrap();

  auto depth_sampled_texture_label = tag("Depth Sampled Texture"_str);
  auto depth_sampled_texture       = device
                                 ->create_descriptor_set(gpu::DescriptorSetInfo{
                                   .label  = depth_sampled_texture_label,
                                   .layout = sys.layouts_.sampled_textures,
                                   .variable_lengths = span({1U})})
                                 .unwrap();

  auto depth_storage_texture_label = tag("Depth Storage Texture"_str);
  auto depth_storage_texture       = device
                                 ->create_descriptor_set(gpu::DescriptorSetInfo{
                                   .label  = depth_storage_texture_label,
                                   .layout = sys.layouts_.storage_textures,
                                   .variable_lengths = span({1U})})
                                 .unwrap();

  auto depth_input_attachment_label = tag("Depth Input Attachment"_str);
  auto depth_input_attachment =
    device
      ->create_descriptor_set(
        gpu::DescriptorSetInfo{.label  = depth_input_attachment_label,
                               .layout = sys.layouts_.input_attachments,
                               .variable_lengths = span({1U})})
      .unwrap();

  depth_stencil_info.label = {};
  depth_view_info.label    = {};
  stencil_view_info.label  = {};

  auto depth_stencil =
    DepthStencilTexture{.info                   = depth_stencil_info,
                        .depth_view_info        = depth_view_info,
                        .stencil_view_info      = stencil_view_info,
                        .image                  = depth_stencil_image,
                        .depth_view             = depth_image_view,
                        .stencil_view           = stencil_image_view,
                        .depth_sampled_texture  = depth_sampled_texture,
                        .depth_storage_texture  = depth_storage_texture,
                        .depth_input_attachment = depth_input_attachment};

  return TextureUnion{.color = color, .depth_stencil = depth_stencil};
}

void ScratchTextures::uninit(gpu::Device device)
{
  for (auto & scratch : textures)
  {
    scratch.uninit(device);
  }
  textures.clear();
  device->uninit(memory_group);
}

ScratchTextures ScratchTextures::create(
  gpu::Device device, GpuSystem const & system, u32 num_scratch,
  u32x2 target_size, gpu::Format color_format, gpu::Format depth_stencil_format,
  Str label, Allocator allocator, Allocator scratch)
{
  Vec<Enum<gpu::Buffer, gpu::Image>> resources{scratch};
  Vec<u32>                           aliases{scratch};

  aliases.push(0U).unwrap();

  Vec<TextureUnion> textures{allocator};

  for (auto i : range(num_scratch))
  {
    auto union_label = sformat(scratch, "{} / {}"_str, label, i).unwrap();
    auto num_alias_elements = 2U;
    auto union_texture =
      TextureUnion::create(device, system, target_size, color_format,
                           depth_stencil_format, union_label, scratch);
    textures.push(union_texture).unwrap();
    resources.push(union_texture.color.image).unwrap();
    resources.push(union_texture.depth_stencil.image).unwrap();
    aliases.push(aliases.last() + num_alias_elements).unwrap();
  }

  auto memory_group =
    device
      ->create_memory_group(gpu::MemoryGroupInfo{.resources = resources.view(),
                                                 .aliases   = aliases.view()})
      .unwrap();

  for (auto & scratch : textures.view())
  {
    device->update_descriptor_set(gpu::DescriptorSetUpdate{
      .set           = scratch.color.sampled_texture,
      .binding       = 0,
      .first_element = 0,
      .images = span({gpu::ImageBinding{.image_view = scratch.color.view}}),
      .texel_buffers = {},
      .buffers       = {}});

    device->update_descriptor_set(gpu::DescriptorSetUpdate{
      .set           = scratch.color.storage_texture,
      .binding       = 0,
      .first_element = 0,
      .images = span({gpu::ImageBinding{.image_view = scratch.color.view}}),
      .texel_buffers = {},
      .buffers       = {}});

    device->update_descriptor_set(gpu::DescriptorSetUpdate{
      .set           = scratch.color.input_attachment,
      .binding       = 0,
      .first_element = 0,
      .images = span({gpu::ImageBinding{.image_view = scratch.color.view}}),
      .texel_buffers = {},
      .buffers       = {}});

    device->update_descriptor_set(gpu::DescriptorSetUpdate{
      .set           = scratch.depth_stencil.depth_sampled_texture,
      .binding       = 0,
      .first_element = 0,
      .images        = span(
        {gpu::ImageBinding{.image_view = scratch.depth_stencil.depth_view}}),
      .texel_buffers = {},
      .buffers       = {}});

    device->update_descriptor_set(gpu::DescriptorSetUpdate{
      .set           = scratch.depth_stencil.depth_storage_texture,
      .binding       = 0,
      .first_element = 0,
      .images        = span(
        {gpu::ImageBinding{.image_view = scratch.depth_stencil.depth_view}}),
      .texel_buffers = {},
      .buffers       = {}});

    device->update_descriptor_set(gpu::DescriptorSetUpdate{
      .set           = scratch.depth_stencil.depth_input_attachment,
      .binding       = 0,
      .first_element = 0,
      .images        = span(
        {gpu::ImageBinding{.image_view = scratch.depth_stencil.depth_view}}),
      .texel_buffers = {},
      .buffers       = {}});
  }

  return ScratchTextures{.textures     = std::move(textures),
                         .memory_group = memory_group};
}

ColorTexture create_target_texture(gpu::Device device, GpuSystem * sys,
                                   u32x2 target_size, gpu::Format color_format,
                                   Str label, Allocator scratch)
{
  auto tag = [&](auto component) {
    return sformat(scratch, "{} / {}"_str, label, component).unwrap();
  };

  auto color_label = tag("Color Image"_str);

  auto color_info = gpu::ImageInfo{.label        = color_label,
                                   .type         = gpu::ImageType::Type2D,
                                   .format       = color_format,
                                   .usage        = ColorTexture::USAGE,
                                   .aspects      = gpu::ImageAspects::Color,
                                   .extent       = target_size.append(1),
                                   .mip_levels   = 1,
                                   .array_layers = 1,
                                   .sample_count = gpu::SampleCount::C1,
                                   .memory_type  = gpu::MemoryType::Unique};

  auto color_image = device->create_image(color_info).unwrap();

  auto color_view_label = tag("Color Image View"_str);

  auto color_view_info = gpu::ImageViewInfo{
    .label        = color_view_label,
    .image        = color_image,
    .view_type    = gpu::ImageViewType::Type2D,
    .view_format  = color_format,
    .mapping      = {},
    .aspects      = gpu::ImageAspects::Color,
    .mip_levels   = {0, 1},
    .array_layers = {0, 1}
  };

  auto color_image_view = device->create_image_view(color_view_info).unwrap();

  auto sampled_color_texture_label = tag("Sampled Color Texture"_str);
  auto sampled_color_texture       = device
                                 ->create_descriptor_set(gpu::DescriptorSetInfo{
                                   .label  = sampled_color_texture_label,
                                   .layout = sys->layouts_.sampled_textures,
                                   .variable_lengths = span({1U})})
                                 .unwrap();

  device->update_descriptor_set(gpu::DescriptorSetUpdate{
    .set           = sampled_color_texture,
    .binding       = 0,
    .first_element = 0,
    .images        = span({gpu::ImageBinding{.image_view = color_image_view}}),
    .texel_buffers = {},
    .buffers       = {}});

  auto storage_color_texture_label = tag("Storage Color Texture"_str);
  auto storage_color_texture       = device
                                 ->create_descriptor_set(gpu::DescriptorSetInfo{
                                   .label  = storage_color_texture_label,
                                   .layout = sys->layouts_.storage_textures,
                                   .variable_lengths = span({1U})})
                                 .unwrap();

  device->update_descriptor_set(gpu::DescriptorSetUpdate{
    .set           = storage_color_texture,
    .binding       = 0,
    .first_element = 0,
    .images        = span({gpu::ImageBinding{.image_view = color_image_view}}),
    .texel_buffers = {},
    .buffers       = {}});

  auto input_attachment_label = tag("Input Attachment"_str);
  auto input_attachment       = device
                            ->create_descriptor_set(gpu::DescriptorSetInfo{
                              .label  = input_attachment_label,
                              .layout = sys->layouts_.input_attachments,
                              .variable_lengths = span({1U})})
                            .unwrap();

  device->update_descriptor_set(gpu::DescriptorSetUpdate{
    .set           = input_attachment,
    .binding       = 0,
    .first_element = 0,
    .images        = span({gpu::ImageBinding{.image_view = color_image_view}}),
    .texel_buffers = {},
    .buffers       = {}});

  color_info.label      = {};
  color_view_info.label = {};

  return ColorTexture{.info             = color_info,
                      .view_info        = color_view_info,
                      .image            = color_image,
                      .view             = color_image_view,
                      .sampled_texture  = sampled_color_texture,
                      .storage_texture  = storage_color_texture,
                      .input_attachment = input_attachment};
}

/*


void GpuFrame::rebuild_sampled_textures(u32 num_sampled_textures)
{
  CHECK(state_ == GpuFrameState::Reset, "");
  CHECK(num_sampled_textures > 0, "");

  // [ ] move to GPU thread

  release(sampled_textures_);

  auto label =
    ssformat<128>(allocator_, "GpuFrame ({}) / Sampled Textures", id_).unwrap();

  sampled_textures_ = device_
                        ->create_descriptor_set(gpu::DescriptorSetInfo{
                          .label            = label,
                          .layout           = system_->sampled_textures_layout_,
                          .variable_lengths = span({num_sampled_textures})})
                        .unwrap();
}

void GpuFrame::rebuild_storage_textures(u32 num_storage_textures)
{
  CHECK(state_ == GpuFrameState::Reset, "");
  CHECK(num_storage_textures > 0, "");

  // [ ] move to GPU thread
  release(storage_textures_);

  auto label =
    ssformat<128>(allocator_, "GpuFrame ({}) / Storage Textures", id_).unwrap();

  storage_textures_ = device_
                        ->create_descriptor_set(gpu::DescriptorSetInfo{
                          .label            = label,
                          .layout           = system_->storage_textures_layout_,
                          .variable_lengths = span({num_storage_textures})})
                        .unwrap();
}

void GpuFrame::rebuild_samplers(u32 num_samplers)
{
  CHECK(state_ == GpuFrameState::Reset, "");
  CHECK(num_samplers > 0, "");

  // [ ] move to GPU thread
  release(samplers_);

  auto label =
    ssformat<128>(allocator_, "GpuFrame ({}) / Samplers", id_).unwrap();

  samplers_ = device_
                ->create_descriptor_set(gpu::DescriptorSetInfo{
                  .label            = label,
                  .layout           = system_->samplers_layout_,
                  .variable_lengths = span({num_samplers})})
                .unwrap();
}

void GpuFrame::get_scratch_textures(u32                    num_scratch,
                                    Vec<ScratchTextures> & textures)
{
  // [ ] only allow for encoding; so we can defer allocating resources
  // [ ] resources must only be gotten on the encoding state

  CHECK(num_scratch < scratch_textures_.size(), "");
  CHECK(state_ == GpuFrameState::Constructing ||
          state_ == GpuFrameState::Encoding,
        "");

  for (auto _ : range(num_scratch))
  {
    auto scratch = scratch_textures_[next_scratch_texture_];
    textures.push(scratch).unwrap();
    next_scratch_texture_++;
    next_scratch_texture_ = next_scratch_texture_ % size32(scratch_textures_);
  }
}

GpuBufferSpan GpuFrame::get(GpuBufferId id)
{
  CHECK(state_ == GpuFrameState::Encoding, "");
  Slice64 slice = gpu_buffer_entries_.get((usize) id);
  return {buffer_, slice};
}

Span<u8 const> GpuFrame::get(BufferId id)
{
  CHECK(state_ == GpuFrameState::Encoding, "");
  auto slice = cpu_buffer_entries_.get((usize) id);
  return cpu_buffer_data_.view().slice(slice);
}

void GpuFrame::begin_constructing_frame()
{
  CHECK(state_ == GpuFrameState::Reset, "");
  // [ ] RESERVE SCRATCH BUFFER, SIZE = 0?
  // scratch_buffer_reserve_size_ = 0;
  state_ = GpuFrameState::Constructing;
}

void GpuFrame::end_constructing_frame()
{
  CHECK(state_ == GpuFrameState::Constructing, "");

  // [ ] resize resources?
  // [ ] reserve buffer

  state_ = GpuFrameState::Constructed;
}
*/

void GpuFrameResources::uninit(gpu::Device device)
{
  buffer.uninit(device);
  target.uninit(device);
  scratch_textures.uninit(device);
  scratch_buffer.uninit(device);
  queries.uninit(device);
}

void reserve_buffer(gpu::Device device, GpuSystem & system, Str label,
                    GpuBuffer & buffer, u64 next_capacity, Allocator scratch)
{
  if (buffer.capacity < next_capacity)
  {
    // [ ] allow static capacity
    buffer.uninit(device);
    buffer = GpuBuffer::create(device, system, next_capacity, buffer.usage,
                               label, scratch);
  }
}

void grow_buffer(gpu::Device device, GpuSystem & system, Str label,
                 GpuBuffer & buffer, u64 next_capacity, Allocator scratch)
{
  // [ ] allow static capacity
  if (buffer.capacity < next_capacity)
  {
    reserve_buffer(device, system, label, buffer, next_capacity, scratch);
  }
  else if (buffer.capacity > HalfGrowth::grow(next_capacity))
  {
    // Target at least 75% utilization
    buffer.uninit(device);
    buffer = GpuBuffer::create(device, system, next_capacity, buffer.usage,
                               label, scratch);
  }
}

void GpuFrame::uninit()
{
  resources_.uninit(device_);
  device_->uninit(command_encoder_);
  device_->uninit(command_buffer_);
}

void GpuFrame::execute_()
{
  char              scratch_space_[1'024];
  FallbackAllocator scratch{Arena::from(scratch_space_), allocator_};

  // [ ] recreate queries

  {
    auto buffer_label =
      sformat(scratch, "GpuFrame {} / Buffer"_str, id_).unwrap();
    reserve_buffer(device_, *system_, buffer_label, resources_.buffer,
                   current_plan_->gpu_buffer_data_.size(), scratch);
    mem::copy(current_plan_->gpu_buffer_data_.view(),
              device_->get_memory_map(resources_.buffer.buffer).unwrap());
  }

  {
    auto buffer_label =
      sformat(scratch, "GpuFrame {} / Scratch Buffer"_str, id_).unwrap();
    reserve_buffer(device_, *system_, buffer_label, resources_.scratch_buffer,
                   current_plan_->scratch_buffer_reserve_size_, scratch);
  }

  // [ ] msaa., sample count
  if (target_info_ != current_plan_->target_)
  {
    resources_.target.uninit(device_);
    auto label = sformat(scratch, "GpuFrame {} / Target"_str, id_).unwrap();
    resources_.target =
      create_target_texture(device_, system_, target_info_.extent,
                            target_info_.color_format, label, scratch);

    resources_.scratch_textures.uninit(device_);
    auto scratch_label =
      sformat(scratch, "GpuFrame {} / Scratch Textures"_str, id_).unwrap();
    // [ ] DEFAULT_NUM_SCRATCH_TEXTURES
    resources_.scratch_textures = ScratchTextures::create(
      device_, *system_, DEFAULT_NUM_SCRATCH_TEXTURES, target_info_.extent,
      target_info_.color_format, target_info_.depth_stencil_format,
      scratch_label, allocator_, scratch);
    next_scratch_texture_ = 0;
  }

  for (auto & task : current_plan_->pre_frame_tasks_)
  {
    task();
  }

  command_encoder_->begin();
  command_encoder_->reset_timestamp_query(
    resources_.queries.timestamps,
    Slice32{0, resources_.queries.timestamps_capacity()});
  command_encoder_->reset_statistics_query(
    resources_.queries.statistics,
    Slice32{0, resources_.queries.statistics_capacity()});

  for (auto & pass : current_plan_->passes_)
  {
    pass(*this);
  }

  command_encoder_->end().unwrap();

  // [ ] collect timings

  device_->acquire_next(swapchain_).unwrap();
  command_buffer_->begin();
  command_buffer_->record(command_encoder_);
  command_buffer_->end().unwrap();

  device_->submit(command_buffer_, queue_scope_).unwrap();

  for (auto & task : current_plan_->post_frame_tasks_)
  {
    task();
  }
}

void GpuFrame::reset_()
{
  next_scratch_texture_ = 0;
  resources_.queries.reset();
}

void GpuFrame::complete_()
{
  command_encoder_->reset();
  command_buffer_->reset();

  device_
    ->get_timestamp_query_result(resources_.queries.timestamps, 0,
                                 resources_.queries.cpu_timestamps)
    .unwrap();
  device_
    ->get_statistics_query_result(resources_.queries.statistics, 0,
                                  resources_.queries.cpu_statistics)
    .unwrap();

  for (auto & task : current_plan_->frame_completed_tasks_)
  {
    task();
  }

  // [ ] if swapchain is resized or format changes, resize target; send info to receive

  semaphore_->increment(1);
}

static Option<gpu::Format> select_hdr_format(gpu::Device & dev)
{
  for (auto fmt : ColorTexture::HDR_FORMATS)
  {
    gpu::FormatProperties props = dev.get_format_properties(fmt).unwrap();
    if (has_bits(props.optimal_tiling_features, ColorTexture::FEATURES))
    {
      return fmt;
    }
  }

  return none;
}

static Option<gpu::Format> select_sdr_format(gpu::Device & dev)
{
  for (auto fmt : ColorTexture::SDR_FORMATS)
  {
    gpu::FormatProperties props = dev.get_format_properties(fmt).unwrap();
    if (has_bits(props.optimal_tiling_features, ColorTexture::FEATURES))
    {
      return fmt;
    }
  }

  return none;
}

static Option<gpu::Format> select_depth_stencil_format(gpu::Device & dev)
{
  for (auto fmt : DepthStencilTexture::FORMATS)
  {
    gpu::FormatProperties props = dev.get_format_properties(fmt).unwrap();
    if (has_bits(props.optimal_tiling_features, DepthStencilTexture::FEATURES))
    {
      return fmt;
    }
  }

  return none;
}

GpuSystem GpuSystem::create(Allocator allocator, gpu::IDevice & device,
                            Span<u8 const> pipeline_cache_data, bool use_hdr,
                            u32 buffering, gpu::SampleCount sample_count,
                            u32x2 initial_extent)
{
  CHECK(buffering <= gpu::MAX_FRAME_BUFFERING, "");
  CHECK(initial_extent.x() > 0 && initial_extent.y() > 0, "");

  gpu::Format color_fmt = gpu::Format::Undefined;

  if (use_hdr)
  {
    select_hdr_format(device).match(
      [&](auto fmt) { color_fmt = fmt; },
      [&]() {
        warn("HDR mode requested but Device does not support "
             "HDR formats, trying SDR formats"_str);

        color_fmt = select_sdr_format(device).unwrap(
          "Device doesn't support any known color format"_str);
      });
  }
  else
  {
    color_fmt = select_sdr_format(device).unwrap(
      "Device doesn't support any known color format"_str);
  }

  gpu::Format depth_stencil_fmt = select_depth_stencil_format(device).unwrap(
    "Device doesn't support any known depth-stencil formats"_str);

  trace("Selected color format: {}"_str, color_fmt);

  trace("Selected depth stencil format: {}"_str, depth_stencil_fmt);

  gpu::PipelineCache pipeline_cache =
    device
      .create_pipeline_cache(gpu::PipelineCacheInfo{
        .label        = "Pipeline Cache"_str,
        .initial_data = pipeline_cache_data,
      })
      .unwrap();

  gpu::DescriptorSetLayout const_buffer_layout =
    device
      .create_descriptor_set_layout(gpu::DescriptorSetLayoutInfo{
        .label    = "ConstantBuffer Layout"_str,
        .bindings = span({gpu::DescriptorBindingInfo{
          .type               = gpu::DescriptorType::UniformBuffer,
          .count              = 1,
          .is_variable_length = false}}
          )
  })
      .unwrap();

  gpu::DescriptorSetLayout read_struct_buffer_layout =
    device
      .create_descriptor_set_layout(gpu::DescriptorSetLayoutInfo{
        .label    = "ReadStructBuffer Layout"_str,
        .bindings = span({gpu::DescriptorBindingInfo{
          .type               = gpu::DescriptorType::DynReadStorageBuffer,
          .count              = 1,
          .is_variable_length = false}}
          )
  })
      .unwrap();

  gpu::DescriptorSetLayout read_write_struct_buffer_layout =
    device
      .create_descriptor_set_layout(gpu::DescriptorSetLayoutInfo{
        .label    = "ReadWriteStructBuffer Layout"_str,
        .bindings = span({gpu::DescriptorBindingInfo{
          .type               = gpu::DescriptorType::DynRWStorageBuffer,
          .count              = 1,
          .is_variable_length = false}}
          )
  })
      .unwrap();

  gpu::DescriptorSetLayout sampled_textures_layout =
    device
      .create_descriptor_set_layout(gpu::DescriptorSetLayoutInfo{
        .label    = "Sampled Textures Layout"_str,
        .bindings = span(
          {gpu::DescriptorBindingInfo{.type = gpu::DescriptorType::SampledImage,
                                      .count              = NUM_TEXTURE_SLOTS,
                                      .is_variable_length = true}}
            )
  })
      .unwrap();

  gpu::DescriptorSetLayout storage_textures_layout =
    device
      .create_descriptor_set_layout(gpu::DescriptorSetLayoutInfo{
        .label    = "Storage Textures Layout"_str,
        .bindings = span(
          {gpu::DescriptorBindingInfo{.type = gpu::DescriptorType::StorageImage,
                                      .count              = NUM_TEXTURE_SLOTS,
                                      .is_variable_length = true}}
            )
  })
      .unwrap();

  gpu::DescriptorSetLayout samplers_layout =
    device
      .create_descriptor_set_layout(gpu::DescriptorSetLayoutInfo{
        .label = "Samplers Layout"_str,
        .bindings =
          span({gpu::DescriptorBindingInfo{.type = gpu::DescriptorType::Sampler,
                                           .count = NUM_SAMPLER_SLOTS,
                                           .is_variable_length = true}}
          )
  })
      .unwrap();

  gpu::DescriptorSet sampled_textures =
    device
      .create_descriptor_set(gpu::DescriptorSetInfo{
        .label            = "Texture Views"_str,
        .layout           = sampled_textures_layout,
        .variable_lengths = span<u32>({NUM_TEXTURE_SLOTS})})
      .unwrap();

  gpu::DescriptorSet samplers =
    device
      .create_descriptor_set(gpu::DescriptorSetInfo{
        .label            = "Samplers"_str,
        .layout           = samplers_layout,
        .variable_lengths = span<u32>({NUM_SAMPLER_SLOTS})})
      .unwrap();

  gpu::Image default_image =
    device
      .create_image(gpu::ImageInfo{
        .label  = "Default Image"_str,
        .type   = gpu::ImageType::Type2D,
        .format = gpu::Format::B8G8R8A8_UNORM,
        .usage  = gpu::ImageUsage::Sampled | gpu::ImageUsage::TransferDst |
                 gpu::ImageUsage::Storage | gpu::ImageUsage::Storage,
        .aspects      = gpu::ImageAspects::Color,
        .extent       = {1, 1, 1},
        .mip_levels   = 1,
        .array_layers = 1,
        .sample_count = gpu::SampleCount::C1,
        .memory_type  = gpu::MemoryType::Unique
  })
      .unwrap();

  InplaceVec<Vec<gpu::Object>, gpu::MAX_FRAME_BUFFERING> released_objects;

  for (u32 i = 0; i < buffering; i++)
  {
    released_objects.push(Vec<gpu::Object>{allocator}).unwrap();
  }

  InplaceVec<GpuQueries, gpu::MAX_FRAME_BUFFERING> queries;

  for (u32 i = 0; i < buffering; i++)
  {
    queries
      .push(GpuQueries::create(allocator, device,
                               device.get_properties().timestamp_period,
                               NUM_FRAME_TIMESPANS, NUM_FRAME_STATISTICS))
      .unwrap();
  }

  GpuSystem gpu{allocator,
                device,
                device.get_properties(),
                pipeline_cache,
                buffering,
                sample_count,
                color_fmt,
                depth_stencil_fmt,
                const_buffer_layout,
                read_struct_buffer_layout,
                read_write_struct_buffer_layout,
                sampled_textures_layout,
                storage_textures_layout,
                samplers_layout,
                sampled_textures,
                samplers,
                default_image,
                {},
                std::move(released_objects),
                FrameGraph{allocator},
                std::move(queries)};

  {
    static constexpr Tuple<Str, TextureId, gpu::ComponentMapping> mappings[] = {
      {"Default White Texture"_str,
       TextureId::White,
       {.r = gpu::ComponentSwizzle::One,
        .g = gpu::ComponentSwizzle::One,
        .b = gpu::ComponentSwizzle::One,
        .a = gpu::ComponentSwizzle::One} },
      {"Default Black Texture"_str,
       TextureId::Black,
       {.r = gpu::ComponentSwizzle::Zero,
        .g = gpu::ComponentSwizzle::Zero,
        .b = gpu::ComponentSwizzle::Zero,
        .a = gpu::ComponentSwizzle::One} },
      {"Default Transparent Texture"_str,
       TextureId::Transparent,
       {.r = gpu::ComponentSwizzle::Zero,
        .g = gpu::ComponentSwizzle::Zero,
        .b = gpu::ComponentSwizzle::Zero,
        .a = gpu::ComponentSwizzle::Zero}},
      {"Default Alpha Texture"_str,
       TextureId::Alpha,
       {.r = gpu::ComponentSwizzle::Zero,
        .g = gpu::ComponentSwizzle::Zero,
        .b = gpu::ComponentSwizzle::Zero,
        .a = gpu::ComponentSwizzle::One} },
      {"Default Red Texture"_str,
       TextureId::Red,
       {.r = gpu::ComponentSwizzle::One,
        .g = gpu::ComponentSwizzle::Zero,
        .b = gpu::ComponentSwizzle::Zero,
        .a = gpu::ComponentSwizzle::One} },
      {"Default Green Texture"_str,
       TextureId::Green,
       {.r = gpu::ComponentSwizzle::Zero,
        .g = gpu::ComponentSwizzle::One,
        .b = gpu::ComponentSwizzle::Zero,
        .a = gpu::ComponentSwizzle::One} },
      {"Default Blue Texture"_str,
       TextureId::Blue,
       {.r = gpu::ComponentSwizzle::Zero,
        .g = gpu::ComponentSwizzle::Zero,
        .b = gpu::ComponentSwizzle::One,
        .a = gpu::ComponentSwizzle::One} },
      {"Default Magenta Texture"_str,
       TextureId::Magenta,
       {.r = gpu::ComponentSwizzle::One,
        .g = gpu::ComponentSwizzle::Zero,
        .b = gpu::ComponentSwizzle::One,
        .a = gpu::ComponentSwizzle::One} },
      {"Default Cyan Texture"_str,
       TextureId::Cyan,
       {.r = gpu::ComponentSwizzle::Zero,
        .g = gpu::ComponentSwizzle::One,
        .b = gpu::ComponentSwizzle::One,
        .a = gpu::ComponentSwizzle::One} },
      {"Default Yellow Texture"_str,
       TextureId::Yellow,
       {.r = gpu::ComponentSwizzle::One,
        .g = gpu::ComponentSwizzle::One,
        .b = gpu::ComponentSwizzle::Zero,
        .a = gpu::ComponentSwizzle::One} }
    };

    static_assert(size(mappings) == NUM_DEFAULT_TEXTURES);

    for (auto const [mapping, view] : zip(mappings, gpu.default_image_views_))
    {
      view = device
               .create_image_view(gpu::ImageViewInfo{
                 .label        = mapping.v0,
                 .image        = default_image,
                 .view_type    = gpu::ImageViewType::Type2D,
                 .view_format  = gpu::Format::B8G8R8A8_UNORM,
                 .mapping      = mapping.v2,
                 .aspects      = gpu::ImageAspects::Color,
                 .mip_levels   = {0, 1},
                 .array_layers = {0, 1}
      })
               .unwrap();

      CHECK(mapping.v1 == gpu.alloc_texture_id(view), "");
    }
  }

  {
    static constexpr SamplerId default_ids[NUM_DEFAULT_SAMPLERS] = {
      SamplerId::LinearBlack, SamplerId::NearestBlack, SamplerId::LinearClamped,
      SamplerId::NearestClamped};

    static constexpr gpu::SamplerInfo infos[NUM_DEFAULT_SAMPLERS] = {
      {.label                    = "Linear+Repeat Sampler"_str,
       .mag_filter               = gpu::Filter::Linear,
       .min_filter               = gpu::Filter::Linear,
       .mip_map_mode             = gpu::SamplerMipMapMode::Linear,
       .address_mode_u           = gpu::SamplerAddressMode::Repeat,
       .address_mode_v           = gpu::SamplerAddressMode::Repeat,
       .address_mode_w           = gpu::SamplerAddressMode::Repeat,
       .mip_lod_bias             = 0,
       .anisotropy_enable        = false,
       .max_anisotropy           = 1.0,
       .compare_enable           = false,
       .compare_op               = gpu::CompareOp::Never,
       .min_lod                  = 0,
       .max_lod                  = 0,
       .border_color             = gpu::BorderColor::FloatTransparentBlack,
       .unnormalized_coordinates = false},
      {.label                    = "Nearest+Repeat Sampler"_str,
       .mag_filter               = gpu::Filter::Nearest,
       .min_filter               = gpu::Filter::Nearest,
       .mip_map_mode             = gpu::SamplerMipMapMode::Nearest,
       .address_mode_u           = gpu::SamplerAddressMode::Repeat,
       .address_mode_v           = gpu::SamplerAddressMode::Repeat,
       .address_mode_w           = gpu::SamplerAddressMode::Repeat,
       .mip_lod_bias             = 0,
       .anisotropy_enable        = false,
       .max_anisotropy           = 1.0,
       .compare_enable           = false,
       .compare_op               = gpu::CompareOp::Never,
       .min_lod                  = 0,
       .max_lod                  = 0,
       .border_color             = gpu::BorderColor::FloatTransparentBlack,
       .unnormalized_coordinates = false},
      {.label                    = "Linear+EdgeClamped Sampler"_str,
       .mag_filter               = gpu::Filter::Linear,
       .min_filter               = gpu::Filter::Linear,
       .mip_map_mode             = gpu::SamplerMipMapMode::Linear,
       .address_mode_u           = gpu::SamplerAddressMode::ClampToEdge,
       .address_mode_v           = gpu::SamplerAddressMode::ClampToEdge,
       .address_mode_w           = gpu::SamplerAddressMode::ClampToEdge,
       .mip_lod_bias             = 0,
       .anisotropy_enable        = false,
       .max_anisotropy           = 1.0,
       .compare_enable           = false,
       .compare_op               = gpu::CompareOp::Never,
       .min_lod                  = 0,
       .max_lod                  = 0,
       .border_color             = gpu::BorderColor::FloatTransparentBlack,
       .unnormalized_coordinates = false},
      {.label                    = "Nearest+EdgeClamped Sampler"_str,
       .mag_filter               = gpu::Filter::Nearest,
       .min_filter               = gpu::Filter::Nearest,
       .mip_map_mode             = gpu::SamplerMipMapMode::Nearest,
       .address_mode_u           = gpu::SamplerAddressMode::ClampToEdge,
       .address_mode_v           = gpu::SamplerAddressMode::ClampToEdge,
       .address_mode_w           = gpu::SamplerAddressMode::ClampToEdge,
       .mip_lod_bias             = 0,
       .anisotropy_enable        = false,
       .max_anisotropy           = 1.0,
       .compare_enable           = false,
       .compare_op               = gpu::CompareOp::Never,
       .min_lod                  = 0,
       .max_lod                  = 0,
       .border_color             = gpu::BorderColor::FloatTransparentBlack,
       .unnormalized_coordinates = false}
    };

    for (auto const [expected_id, info] : zip(default_ids, infos))
    {
      CHECK(gpu.create_sampler(info).id == expected_id, "");
    }
  }

  gpu.frame_graph_.acquire(gpu);
  gpu.recreate_framebuffers(initial_extent);

  return gpu;
}

void GpuSystem::shutdown(Vec<u8> & cache)
{
  for (auto & queries : queries_)
  {
    queries.uninit(*device_);
  }
  frame_graph_.release(*this);
  device_->get_pipeline_cache_data(pipeline_cache_, cache).unwrap();
  release(sampled_textures_);
  for (gpu::ImageView v : default_image_views_)
  {
    release(v);
  }
  release(default_image_);
  release(samplers_);
  release(const_buffer_layout_);
  release(read_struct_buffer_layout_);
  release(read_write_struct_buffer_layout_);
  release(sampled_textures_layout_);
  release(storage_textures_layout_);
  release(samplers_layout_);
  release(fb_);
  for (auto & tex : scratch_color_)
  {
    release(tex);
  }
  for (auto & tex : scratch_depth_stencil_)
  {
    release(tex);
  }
  for (auto const & [_, sampler] : sampler_cache_)
  {
    release(sampler.sampler);
  }
  release(pipeline_cache_);

  sampled_textures_                = {};
  default_image_views_             = {};
  default_image_                   = {};
  samplers_                        = {};
  const_buffer_layout_             = {};
  read_struct_buffer_layout_       = {};
  read_write_struct_buffer_layout_ = {};
  sampled_textures_layout_         = {};
  storage_textures_layout_         = {};
  samplers_layout_                 = {};
  fb_                              = {};
  fill(scratch_color_, ColorTexture{});
  fill(scratch_depth_stencil_, DepthStencilTexture{});
  sampler_cache_  = {};
  pipeline_cache_ = {};

  idle_reclaim();
}

static ColorTexture create_color_texture(GpuSystem & gpu, u32x2 new_extent,
                                         Str prefix, usize index)
{
  auto label =
    snformat<256>("Color Texture: {} [{}]"_str, prefix, index).unwrap();

  gpu::ImageInfo info{
    .label  = label,
    .type   = gpu::ImageType::Type2D,
    .format = gpu.color_format_,
    .usage  = gpu::ImageUsage::ColorAttachment | gpu::ImageUsage::Sampled |
             gpu::ImageUsage::Storage | gpu::ImageUsage::TransferDst |
             gpu::ImageUsage::TransferSrc,
    .aspects      = gpu::ImageAspects::Color,
    .extent       = new_extent.append(1),
    .mip_levels   = 1,
    .array_layers = 1,
    .sample_count = gpu::SampleCount::C1,
    .memory_type  = gpu::MemoryType::Unique};

  gpu::Image image = gpu.device_->create_image(info).unwrap();

  auto view_label =
    snformat<256>("Color Texture View: {} [{}]"_str, prefix, index).unwrap();

  gpu::ImageViewInfo view_info{
    .label        = view_label,
    .image        = image,
    .view_type    = gpu::ImageViewType::Type2D,
    .view_format  = info.format,
    .mapping      = {},
    .aspects      = gpu::ImageAspects::Color,
    .mip_levels   = {0, 1},
    .array_layers = {0, 1}
  };

  gpu::ImageView view = gpu.device_->create_image_view(view_info).unwrap();

  auto sampled_texture_label =
    snformat<256>("Sampled Color Texture Descriptor: {} [{}]"_str, prefix,
                  index)
      .unwrap();

  gpu::DescriptorSet sampled_texture =
    gpu.device_
      ->create_descriptor_set(
        gpu::DescriptorSetInfo{.label            = sampled_texture_label,
                               .layout           = gpu.sampled_textures_layout_,
                               .variable_lengths = span<u32>({1})})
      .unwrap();

  gpu.device_->update_descriptor_set(gpu::DescriptorSetUpdate{
    .set           = sampled_texture,
    .binding       = 0,
    .first_element = 0,
    .images        = span({gpu::ImageBinding{.image_view = view}})});

  auto storage_texture_label =
    snformat<256>("Storage Color Texture Descriptor: {} [{}]"_str, prefix,
                  index)
      .unwrap();

  gpu::DescriptorSet storage_texture =
    gpu.device_
      ->create_descriptor_set(
        gpu::DescriptorSetInfo{.label            = storage_texture_label,
                               .layout           = gpu.storage_textures_layout_,
                               .variable_lengths = span<u32>({1})})
      .unwrap();

  gpu.device_->update_descriptor_set(gpu::DescriptorSetUpdate{
    .set           = storage_texture,
    .binding       = 0,
    .first_element = 0,
    .images        = span({gpu::ImageBinding{.image_view = view}})});

  info.label      = {};
  view_info.label = {};

  return ColorTexture{.info            = info,
                      .view_info       = view_info,
                      .image           = image,
                      .view            = view,
                      .sampled_texture = sampled_texture,
                      .storage_texture = storage_texture};
}

static Option<ColorMsaaTexture> create_color_msaa_texture(GpuSystem & gpu,
                                                          u32x2 new_extent,
                                                          Str   prefix,
                                                          usize index)
{
  if (gpu.sample_count_ == gpu::SampleCount::C1)
  {
    return none;
  }

  auto label =
    snformat<256>("MSAA Color Texture: {} [{}]"_str, prefix, index).unwrap();

  gpu::ImageInfo info{.label  = label,
                      .type   = gpu::ImageType::Type2D,
                      .format = gpu.color_format_,
                      .usage  = gpu::ImageUsage::ColorAttachment |
                               gpu::ImageUsage::TransferSrc |
                               gpu::ImageUsage::TransferDst,
                      .aspects      = gpu::ImageAspects::Color,
                      .extent       = new_extent.append(1),
                      .mip_levels   = 1,
                      .array_layers = 1,
                      .sample_count = gpu.sample_count_,
                      .memory_type  = gpu::MemoryType::Unique};

  gpu::Image image = gpu.device_->create_image(info).unwrap();

  auto view_label =
    snformat<256>("MSAA Color Texture View: {} [{}]"_str, prefix, index)
      .unwrap();

  gpu::ImageViewInfo view_info{
    .label        = view_label,
    .image        = image,
    .view_type    = gpu::ImageViewType::Type2D,
    .view_format  = info.format,
    .mapping      = {},
    .aspects      = gpu::ImageAspects::Color,
    .mip_levels   = {0, 1},
    .array_layers = {0, 1},
  };

  gpu::ImageView view = gpu.device_->create_image_view(view_info).unwrap();

  info.label      = {};
  view_info.label = {};

  return ColorMsaaTexture{
    .info = info, .view_info = view_info, .image = image, .view = view};
}

static DepthStencilTexture create_depth_texture(GpuSystem & gpu,
                                                u32x2 new_extent, Str prefix,
                                                usize index)
{
  auto label =
    snformat<128>("Depth-Stencil Texture: {} [{}]"_str, prefix, index).unwrap();

  gpu::ImageInfo info{
    .label  = label,
    .type   = gpu::ImageType::Type2D,
    .format = gpu.depth_stencil_format_,
    .usage  = gpu::ImageUsage::DepthStencilAttachment |
             gpu::ImageUsage::Sampled | gpu::ImageUsage::TransferDst |
             gpu::ImageUsage::TransferSrc,
    .aspects      = gpu::ImageAspects::Depth | gpu::ImageAspects::Stencil,
    .extent       = new_extent.append(1),
    .mip_levels   = 1,
    .array_layers = 1,
    .sample_count = gpu::SampleCount::C1,
    .memory_type  = gpu::MemoryType::Unique};

  gpu::Image image = gpu.device_->create_image(info).unwrap();

  auto view_label =
    snformat<128>("Depth Texture View - Depth: {} [{}]"_str, prefix, index)
      .unwrap();

  gpu::ImageViewInfo depth_view_info{
    .label        = view_label,
    .image        = image,
    .view_type    = gpu::ImageViewType::Type2D,
    .view_format  = info.format,
    .mapping      = {},
    .aspects      = gpu::ImageAspects::Depth,
    .mip_levels   = {0, 1},
    .array_layers = {0, 1}
  };

  gpu::ImageView depth_view =
    gpu.device_->create_image_view(depth_view_info).unwrap();

  auto stencil_view_label =
    snformat<128>("Depth Texture View - Stencil: {} [{}]"_str, prefix, index)
      .unwrap();

  gpu::ImageViewInfo stencil_view_info{
    .label        = stencil_view_label,
    .image        = image,
    .view_type    = gpu::ImageViewType::Type2D,
    .view_format  = info.format,
    .mapping      = {},
    .aspects      = gpu::ImageAspects::Stencil,
    .mip_levels   = {0, 1},
    .array_layers = {0, 1}
  };

  gpu::ImageView stencil_view =
    gpu.device_->create_image_view(stencil_view_info).unwrap();

  auto texture_label =
    snformat<128>("Depth Texture Descriptor - Depth: {} [{}]"_str, prefix,
                  index)
      .unwrap();

  gpu::DescriptorSet sampled_depth_texture =
    gpu.device_
      ->create_descriptor_set(
        gpu::DescriptorSetInfo{.label            = texture_label,
                               .layout           = gpu.sampled_textures_layout_,
                               .variable_lengths = span<u32>({1})})
      .unwrap();

  gpu.device_->update_descriptor_set(gpu::DescriptorSetUpdate{
    .set           = sampled_depth_texture,
    .binding       = 0,
    .first_element = 0,
    .images        = span({gpu::ImageBinding{.image_view = depth_view}})});

  gpu::DescriptorSet storage_depth_texture =
    gpu.device_
      ->create_descriptor_set(
        gpu::DescriptorSetInfo{.label            = texture_label,
                               .layout           = gpu.storage_textures_layout_,
                               .variable_lengths = span<u32>({1})})
      .unwrap();

  gpu.device_->update_descriptor_set(gpu::DescriptorSetUpdate{
    .set           = storage_depth_texture,
    .binding       = 0,
    .first_element = 0,
    .images        = span({gpu::ImageBinding{.image_view = depth_view}})});

  info.label              = {};
  depth_view_info.label   = {};
  stencil_view_info.label = {};

  return DepthStencilTexture{.info                  = info,
                             .depth_view_info       = depth_view_info,
                             .stencil_view_info     = stencil_view_info,
                             .image                 = image,
                             .depth_view            = depth_view,
                             .stencil_view          = stencil_view,
                             .sampled_depth_texture = sampled_depth_texture,
                             .storage_depth_texture = storage_depth_texture};
}

// [ ]
void GpuSystem::recreate_framebuffers(u32x2 new_extent)
{
  idle_reclaim();
  release(fb_.color);
  release(fb_.depth_stencil);
  fb_.color_msaa.match([&](auto & c) { release(c); });

  for (auto & scratch_color : scratch_color_)
  {
    release(scratch_color);
  }

  for (auto & scratch_depth : scratch_depth_stencil_)
  {
    release(scratch_depth);
  }

  fb_.color         = create_color_texture(*this, new_extent, "Main"_str, 0);
  fb_.depth_stencil = create_depth_texture(*this, new_extent, "Main"_str, 0);
  fb_.color_msaa = create_color_msaa_texture(*this, new_extent, "Main"_str, 0);

  for (auto [i, tex] : enumerate(scratch_color_))
  {
    tex = create_color_texture(*this, new_extent, "Scratch"_str, i);
  }

  for (auto [i, tex] : enumerate(scratch_depth_stencil_))
  {
    tex = create_depth_texture(*this, new_extent, "Scratch"_str, i);
  }
}

gpu::CommandEncoder & GpuSystem::encoder()
{
  gpu::FrameContext ctx = device_->get_frame_context();
  return *ctx.encoders[ctx.ring_index];
}

u32 GpuSystem::ring_index()
{
  gpu::FrameContext ctx = device_->get_frame_context();
  return ctx.ring_index;
}

gpu::FrameId GpuSystem::frame_id()
{
  gpu::FrameContext ctx = device_->get_frame_context();
  return ctx.current;
}

gpu::FrameId GpuSystem::tail_frame_id()
{
  gpu::FrameContext ctx = device_->get_frame_context();
  return ctx.tail;
}

Sampler GpuSystem::create_sampler(gpu::SamplerInfo const & info)
{
  Option cached = sampler_cache_.try_get(info);
  if (cached)
  {
    return cached.v();
  }

  gpu::Sampler sampler = device_->create_sampler(info).unwrap();
  SamplerId    id      = alloc_sampler_id(sampler);

  Sampler entry{.id = id, .sampler = sampler};

  sampler_cache_.push(info, entry).unwrap();

  return entry;
}

TextureId GpuSystem::alloc_texture_id(gpu::ImageView view)
{
  usize i = texture_slots_.view().find_clear_bit();
  CHECK(i < texture_slots_.size(), "Out of Texture Slots");
  texture_slots_.set_bit(i);

  add_task([i, this, view]() {
    device_->update_descriptor_set(gpu::DescriptorSetUpdate{
      .set     = textures_,
      .binding = 0,
      .element = (u32) i,
      .images  = span({gpu::ImageBinding{.image_view = view}})});
  });

  return TextureId{(u32) i};
}

void GpuSystem::release_texture_id(TextureId id)
{
  u32 const i = (u32) id;
  texture_slots_.clear_bit(i);

  add_task([i, this]() {
    device_->unbind_descriptor_set(textures_, 0, Slice32{i, 1});
  });
}

SamplerId GpuSystem::alloc_sampler_id(gpu::Sampler sampler)
{
  usize i = sampler_slots_.view().find_clear_bit();
  CHECK(i < sampler_slots_.size(), "Out of Sampler Slots");
  sampler_slots_.set_bit(i);

  add_task([i, this, sampler]() {
    device_->update_descriptor_set(gpu::DescriptorSetUpdate{
      .set     = samplers_,
      .binding = 0,
      .element = (u32) i,
      .images  = span({gpu::ImageBinding{.sampler = sampler}})});
  });

  return SamplerId{(u32) i};
}

void GpuSystem::release_sampler_id(SamplerId id)
{
  u32 const i = (u32) id;
  sampler_slots_.clear_bit(i);

  add_task([i, this]() {
    device_->unbind_descriptor_set(samplers_, 0, Slice32{i, 1});
  });
}

void GpuSystem::idle_reclaim()
{
  device_->wait_idle().unwrap();
  for (auto & objects : released_objects_)
  {
    uninit_objects(*device_, objects);
    objects.clear();
  }
}

void GpuSystem::frame(gpu::Swapchain swapchain)
{
  ScopeTrace trace;
  device_->begin_frame(swapchain).unwrap();
  uninit_objects(*device_, released_objects_[ring_index()]);
  released_objects_[ring_index()].clear();

  auto & enc = encoder();

  queries_[ring_index()].begin_frame(*device_, enc);

  auto clear_color = [&](gpu::Image image) {
    enc.clear_color_image(
      image,
      gpu::Color{
    },
      span({gpu::ImageSubresourceRange{.aspects = gpu::ImageAspects::Color,
                                       .first_mip_level   = 0,
                                       .num_mip_levels    = 1,
                                       .first_array_layer = 0,
                                       .num_array_layers  = 1}}));
  };

  auto clear_depth = [&](gpu::Image image) {
    enc.clear_depth_stencil_image(
      image,
      gpu::DepthStencil{
    },
      span({gpu::ImageSubresourceRange{.aspects = gpu::ImageAspects::Depth |
                                                  gpu::ImageAspects::Stencil,
                                       .first_mip_level   = 0,
                                       .num_mip_levels    = 1,
                                       .first_array_layer = 0,
                                       .num_array_layers  = 1}}));
  };

  clear_color(fb_.color.image);
  fb_.color_msaa.match([&](auto & c) { clear_color(c.image); });
  clear_depth(fb_.depth_stencil.image);

  frame_graph_.execute(*this);

  if (swapchain != nullptr)
  {
    auto swapchain_state = device_->get_swapchain_state(swapchain).unwrap();

    swapchain_state.current_image.match([&](auto idx) {
      enc.blit_image(
        fb_.color.image, swapchain_state.images[idx],
        span({
          gpu::ImageBlit{
                         .src_layers = {.aspects           = gpu::ImageAspects::Color,
                           .mip_level         = 0,
                           .first_array_layer = 0,
                           .num_array_layers  = 1},
                         .src_area   = {{0, 0, 0}, fb_.extent()},
                         .dst_layers = {.aspects           = gpu::ImageAspects::Color,
                           .mip_level         = 0,
                           .first_array_layer = 0,
                           .num_array_layers  = 1},
                         .dst_area   = {{0, 0, 0}, swapchain_state.extent.append(1)}}
      }),
        gpu::Filter::Linear);
    });
  }

  device_->submit_frame(swapchain).unwrap();
}

}    // namespace ash
