/// SPDX-License-Identifier: MIT
#include "ashura/engine/gpu_system.h"
#include "ashura/std/range.h"
#include "ashura/std/sformat.h"
#include "ashura/std/str.h"
#include "ashura/std/trace.h"

namespace ash
{

u32x3 ColorTexture::extent() const
{
  return info.extent;
}

void ColorTexture::uninit(gpu::Device device)
{
  device->uninit(sampled_texture);
  device->uninit(storage_texture);
  device->uninit(input_attachment);
  device->uninit(view);
  device->uninit(image);
}

gpu::SampleCount ColorMsaaTexture::sample_count() const
{
  return info.sample_count;
}

u32x3 ColorMsaaTexture::extent() const
{
  return info.extent;
}

void ColorMsaaTexture::uninit(gpu::Device device)
{
  device->uninit(view);
  device->uninit(image);
}

u32x3 DepthStencilTexture::extent() const
{
  return info.extent;
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

u32x3 Framebuffer::extent() const
{
  return color.extent();
}

void Framebuffer::uninit(gpu::Device device)
{
  color.uninit(device);
  color_msaa.match([&](auto & c) { c.uninit(device); });
  depth_stencil.uninit(device);
}

void GpuBuffer::uninit(gpu::Device device)
{
  device->uninit(uniform_buffer);
  device->uninit(read_storage_buffer);
  device->uninit(read_write_storage_buffer);
  device->uninit(buffer);
}

GpuBuffer GpuBuffer::create(GpuSys sys, u64 capacity, gpu::BufferUsage usage,
                            Str label, Allocator scratch)
{
  auto buffer_label =
    sformat(scratch, "{} / {}"_str, label, "Buffer"_str).unwrap();
  auto buffer =
    sys->dev_
      ->create_buffer(gpu::BufferInfo{.label       = buffer_label,
                                      .size        = capacity,
                                      .usage       = usage,
                                      .memory_type = gpu::MemoryType::Unique,
                                      .host_mapped = true})
      .unwrap();

  auto make_set = [&](Str component, gpu::DescriptorSetLayout layout) {
    auto set_label = sformat(scratch, "{} / {}"_str, label, component).unwrap();
    auto set =
      sys->dev_
        ->create_descriptor_set(gpu::DescriptorSetInfo{
          .label = set_label, .layout = layout, .variable_lengths = {}})
        .unwrap();

    sys->dev_->update_descriptor_set(gpu::DescriptorSetUpdate{
      .set           = set,
      .binding       = 0,
      .first_element = 0,
      .buffers       = span(
        {gpu::BufferBinding{.buffer = buffer, .range{0, gpu::WHOLE_SIZE}}}
                )
    });

    return set;
  };

  auto uniform_buffer =
    make_set("Uniform Buffer", sys->descriptors_layout_.uniform_buffer);
  auto read_storage_buffer = make_set(
    "Read StorageBuffer", sys->descriptors_layout_.read_storage_buffer);
  auto read_write_storage_buffer =
    make_set("Read/Write StorageBuffer",
             sys->descriptors_layout_.read_write_storage_buffer);

  return GpuBuffer{.capacity                  = capacity,
                   .usage                     = usage,
                   .buffer                    = buffer,
                   .uniform_buffer            = uniform_buffer,
                   .read_storage_buffer       = read_storage_buffer,
                   .read_write_storage_buffer = read_write_storage_buffer};
}

void GpuQueries::uninit(gpu::Device device)
{
  device->uninit(timestamps);
  device->uninit(statistics);
}

u32 GpuQueries::timestamps_capacity() const
{
  return size32(cpu_timestamps);
}

u32 GpuQueries::statistics_capacity() const
{
  return size32(cpu_statistics);
}

GpuQueries GpuQueries::create(Allocator allocator, gpu::Device device,
                              Span<char const> label, u32 timestamps_capacity,
                              u32 statistics_capacity, Allocator scratch)
{
  CHECK(timestamps_capacity > 0, "");
  CHECK(statistics_capacity > 0, "");

  auto timestamp_label =
    sformat(scratch, "{} / TimestampQuery"_str, label).unwrap();
  auto timestamps = device
                      ->create_timestamp_query(gpu::TimestampQueryInfo{
                        .label = timestamp_label, .count = timestamps_capacity})
                      .unwrap();

  Vec<u64> cpu_timestamps{allocator};
  cpu_timestamps.resize_uninit(timestamps_capacity).unwrap();

  auto statistics_label =
    sformat(scratch, "{} / StatisticsQuery"_str, label).unwrap();
  auto statistics =
    device
      ->create_statistics_query(gpu::StatisticsQueryInfo{
        .label = statistics_label, .count = statistics_capacity})
      .unwrap();

  Vec<gpu::PipelineStatistics> cpu_statistics{allocator};
  cpu_statistics.resize_uninit(statistics_capacity).unwrap();

  return GpuQueries{.timestamps     = timestamps,
                    .statistics     = statistics,
                    .cpu_timestamps = std::move(cpu_timestamps),
                    .cpu_statistics = std::move(cpu_statistics)};
}

void GpuDescriptorsLayout::uninit(gpu::Device device)
{
  device->uninit(samplers);
  device->uninit(sampled_textures);
  device->uninit(storage_textures);
  device->uninit(uniform_buffer);
  device->uninit(read_storage_buffer);
  device->uninit(read_write_storage_buffer);
  device->uninit(uniform_buffers);
  device->uninit(read_storage_buffers);
  device->uninit(read_write_storage_buffers);
  device->uninit(input_attachments);
}

GpuDescriptorsLayout GpuDescriptorsLayout::create(gpu::Device device, Str label,
                                                  GpuSysCfg const & cfg,
                                                  Allocator         scratch)
{
  auto tag = [&](Str component) {
    return sformat(scratch, "{} / {}"_str, label, component).unwrap();
  };

  auto samplers_label = tag("Samplers"_str);
  auto samplers =
    device
      ->create_descriptor_set_layout(gpu::DescriptorSetLayoutInfo{
        .label    = samplers_label,
        .bindings = span(
          {gpu::DescriptorBindingInfo{.type  = gpu::DescriptorType::Sampler,
                                      .count = cfg.bindless_samplers_capacity,
                                      .is_variable_length = true}}
            )
  })
      .unwrap();

  auto sampled_textures_label = tag("Sampled Textures"_str);
  auto sampled_textures =
    device
      ->create_descriptor_set_layout(gpu::DescriptorSetLayoutInfo{
        .label    = sampled_textures_label,
        .bindings = span({gpu::DescriptorBindingInfo{
          .type               = gpu::DescriptorType::SampledImage,
          .count              = cfg.bindless_sampled_textures_capacity,
          .is_variable_length = true}}
          )
  })
      .unwrap();

  auto storage_textures_label = tag("Storage Textures"_str);
  auto storage_textures =
    device
      ->create_descriptor_set_layout(gpu::DescriptorSetLayoutInfo{
        .label    = storage_textures_label,
        .bindings = span({gpu::DescriptorBindingInfo{
          .type               = gpu::DescriptorType::StorageImage,
          .count              = cfg.bindless_storage_textures_capacity,
          .is_variable_length = true}}
          )
  })
      .unwrap();

  auto uniform_buffer_label = tag("Uniform Buffer"_str);
  auto uniform_buffer =
    device
      ->create_descriptor_set_layout(gpu::DescriptorSetLayoutInfo{
        .label    = uniform_buffer_label,
        .bindings = span({gpu::DescriptorBindingInfo{
          .type               = gpu::DescriptorType::DynUniformBuffer,
          .count              = 1,
          .is_variable_length = false}}
          )
  })
      .unwrap();

  auto read_storage_buffer_label = tag("Read Storage Buffer"_str);
  auto read_storage_buffer =
    device
      ->create_descriptor_set_layout(gpu::DescriptorSetLayoutInfo{
        .label    = read_storage_buffer_label,
        .bindings = span({gpu::DescriptorBindingInfo{
          .type               = gpu::DescriptorType::DynReadStorageBuffer,
          .count              = 1,
          .is_variable_length = false}}
          )
  })
      .unwrap();

  auto read_write_storage_buffer_label = tag("Read/Write Storage Buffer"_str);
  auto read_write_storage_buffer =
    device
      ->create_descriptor_set_layout(gpu::DescriptorSetLayoutInfo{
        .label    = read_write_storage_buffer_label,
        .bindings = span({gpu::DescriptorBindingInfo{
          .type               = gpu::DescriptorType::DynRWStorageBuffer,
          .count              = 1,
          .is_variable_length = false}}
          )
  })
      .unwrap();

  auto uniform_buffers_label = tag("Uniform Buffers"_str);
  auto uniform_buffers =
    device
      ->create_descriptor_set_layout(gpu::DescriptorSetLayoutInfo{
        .label    = uniform_buffers_label,
        .bindings = span({gpu::DescriptorBindingInfo{
          .type               = gpu::DescriptorType::UniformBuffer,
          .count              = cfg.bindless_uniform_buffers_capacity,
          .is_variable_length = true}}
          )
  })
      .unwrap();

  auto read_storage_buffers_label = tag("Read Storage Buffers"_str);
  auto read_storage_buffers =
    device
      ->create_descriptor_set_layout(gpu::DescriptorSetLayoutInfo{
        .label    = read_storage_buffers_label,
        .bindings = span({gpu::DescriptorBindingInfo{
          .type               = gpu::DescriptorType::ReadStorageBuffer,
          .count              = cfg.bindless_read_storage_buffers_capacity,
          .is_variable_length = true}}
          )
  })
      .unwrap();

  auto read_write_storage_buffers_label = tag("Read/Write Storage Buffers"_str);
  auto read_write_storage_buffers =
    device
      ->create_descriptor_set_layout(gpu::DescriptorSetLayoutInfo{
        .label    = read_write_storage_buffers_label,
        .bindings = span({gpu::DescriptorBindingInfo{
          .type  = gpu::DescriptorType::RWStorageBuffer,
          .count = cfg.bindless_read_write_storage_buffers_capacity,
          .is_variable_length = true}}
          )
  })
      .unwrap();

  auto input_attachments_label = tag("Input Attachments"_str);
  auto input_attachments =
    device
      ->create_descriptor_set_layout(gpu::DescriptorSetLayoutInfo{
        .label    = input_attachments_label,
        .bindings = span({gpu::DescriptorBindingInfo{
          .type               = gpu::DescriptorType::InputAttachment,
          .count              = cfg.bindless_input_attachments_capacity,
          .is_variable_length = true}}
          )
  })
      .unwrap();

  return GpuDescriptorsLayout{
    .samplers                      = samplers,
    .samplers_capacity             = cfg.bindless_samplers_capacity,
    .sampled_textures              = sampled_textures,
    .sampled_textures_capacity     = cfg.bindless_sampled_textures_capacity,
    .storage_textures              = storage_textures,
    .storage_textures_capacity     = cfg.bindless_storage_textures_capacity,
    .uniform_buffer                = uniform_buffer,
    .read_storage_buffer           = read_storage_buffer,
    .read_write_storage_buffer     = read_write_storage_buffer,
    .uniform_buffers               = uniform_buffers,
    .uniform_buffer_capacity       = cfg.bindless_uniform_buffers_capacity,
    .read_storage_buffers          = read_storage_buffers,
    .read_storage_buffers_capacity = cfg.bindless_read_storage_buffers_capacity,
    .read_write_storage_buffers    = read_write_storage_buffers,
    .read_write_storage_buffers_capacity =
      cfg.bindless_read_write_storage_buffers_capacity,
    .input_attachments          = input_attachments,
    .input_attachments_capacity = cfg.bindless_input_attachments_capacity};
}

void GpuDescriptors::uninit(gpu::Device device)
{
  device->uninit(samplers);
  device->uninit(sampled_textures);
}

GpuDescriptors GpuDescriptors::create(GpuSys sys, Str label, Allocator scratch)
{
  auto tag = [&](Str component) {
    return sformat(scratch, "{} / {}"_str, label, component).unwrap();
  };

  auto samplers_label = tag("Samplers"_str);
  auto samplers =
    sys->dev_
      ->create_descriptor_set(gpu::DescriptorSetInfo{
        .label            = samplers_label,
        .layout           = sys->descriptors_layout_.samplers,
        .variable_lengths = span({sys->descriptors_layout_.samplers_capacity})})
      .unwrap();

  auto sampled_textures_label = tag("Sampled Textures"_str);
  auto sampled_textures =
    sys->dev_
      ->create_descriptor_set(gpu::DescriptorSetInfo{
        .label  = sampled_textures_label,
        .layout = sys->descriptors_layout_.sampled_textures,
        .variable_lengths =
          span({sys->descriptors_layout_.sampled_textures_capacity})})
      .unwrap();

  SparseVec<> samplers_map{sys->allocator_};
  samplers_map.reserve(sys->descriptors_layout_.samplers_capacity).unwrap();

  SparseVec<> sampled_textures_map{sys->allocator_};
  sampled_textures_map
    .reserve(sys->descriptors_layout_.sampled_textures_capacity)
    .unwrap();

  return GpuDescriptors{
    .samplers          = samplers,
    .samplers_capacity = sys->descriptors_layout_.samplers_capacity,
    .samplers_map      = std::move(samplers_map),
    .sampled_textures  = sampled_textures,
    .sampled_textures_capacity =
      sys->descriptors_layout_.sampled_textures_capacity,
    .sampled_textures_map = std::move(sampled_textures_map)};
}

void IGpuFramePlan::uninit()
{
}

void IGpuFramePlan::set_target(GpuFrameTargetInfo target)
{
  CHECK(state_ == GpuFramePlanState::Recording, "");
  target_ = target;
}

void IGpuFramePlan::reserve_scratch_buffers(Span<u64 const> sizes)
{
  CHECK(state_ == GpuFramePlanState::Recording, "");
  scratch_buffer_sizes_.resize(max(scratch_buffer_sizes_.size(), sizes.size()))
    .unwrap();

  for (auto [size, target] : zip(scratch_buffer_sizes_, sizes))
  {
    size = max(size, target);
  }
}

void IGpuFramePlan::reserve_scratch_textures(u32 num_scratch_textures)
{
  CHECK(state_ == GpuFramePlanState::Recording, "");
  num_scratch_textures_ = max(num_scratch_textures_, num_scratch_textures);
}

void IGpuFramePlan::add_preframe_task(GpuFrameTask && task)
{
  CHECK(state_ == GpuFramePlanState::Recording, "");
  pre_frame_tasks_.push(std::move(task)).unwrap();
}

void IGpuFramePlan::add_postframe_task(GpuFrameTask && task)
{
  CHECK(state_ == GpuFramePlanState::Recording, "");
  post_frame_tasks_.push(std::move(task)).unwrap();
}

void IGpuFramePlan::add_pass(GpuPass && pass)
{
  CHECK(state_ == GpuFramePlanState::Recording, "");
  passes_.push(std::move(pass)).unwrap();
}

BufferId IGpuFramePlan::push_cpu(Span<u8 const> data)
{
  CHECK(state_ == GpuFramePlanState::Recording, "");
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

GpuBufferId IGpuFramePlan::push_gpu(Span<u8 const> data)
{
  CHECK(state_ == GpuFramePlanState::Recording, "");
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

GpuSys IGpuFramePlan::sys() const
{
  return sys_;
}

gpu::Device IGpuFramePlan::device() const
{
  return sys_->dev_;
}

void IGpuFramePlan::begin()
{
  CHECK(state_ == GpuFramePlanState::Reset, "");
  state_            = GpuFramePlanState::Recording;
  submission_stage_ = semaphore_->stage();
}

void IGpuFramePlan::end()
{
  CHECK(state_ == GpuFramePlanState::Recording, "");
  state_ = GpuFramePlanState::Recorded;
}

void IGpuFramePlan::reset()
{
  CHECK(state_ != GpuFramePlanState::Submitted, "");
  // these buffers are expected to be very large so we reset them on every frame when they aren't being used
  // Target at least 75% utilization
  pre_frame_tasks_.shrink_clear().unwrap();
  post_frame_tasks_.shrink_clear().unwrap();
  frame_completed_tasks_.shrink_clear().unwrap();
  gpu_buffer_data_.shrink_clear().unwrap();
  gpu_buffer_entries_.shrink_clear().unwrap();
  cpu_buffer_data_.shrink_clear().unwrap();
  cpu_buffer_entries_.shrink_clear().unwrap();
  scratch_buffer_sizes_.shrink_clear().unwrap();
  num_scratch_textures_ = 0;
  passes_.shrink_clear().unwrap();
  target_ = {};
  arena_.reclaim();
  state_ = GpuFramePlanState::Reset;
}

bool IGpuFramePlan::await(nanoseconds timeout)
{
  return semaphore_->await(submission_stage_, timeout);
}

void TextureUnion::uninit(gpu::Device device)
{
  color.uninit(device);
  depth_stencil.uninit(device);
  device->uninit(alias);
}

TextureUnion TextureUnion::create(GpuSys sys, u32x2 target_size,
                                  gpu::Format color_format,
                                  gpu::Format depth_stencil_format, Str label,
                                  Allocator scratch)
{
  // [ ] MSAA scratch and target textures
  auto tag = [&](Str component) {
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
                                   .memory_type  = gpu::MemoryType::Aliased};

  auto color_image = sys->dev_->create_image(color_info).unwrap();

  color_info.label = {};

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

  auto color_image_view =
    sys->dev_->create_image_view(color_view_info).unwrap();

  color_view_info.label = {};

  auto color_sampled_texture_label = tag("Color Sampled Texture"_str);
  auto color_sampled_texture =
    sys->dev_
      ->create_descriptor_set(gpu::DescriptorSetInfo{
        .label            = color_sampled_texture_label,
        .layout           = sys->descriptors_layout_.sampled_textures,
        .variable_lengths = span({1U})})
      .unwrap();

  sys->dev_->update_descriptor_set(gpu::DescriptorSetUpdate{
    .set           = color_sampled_texture,
    .binding       = 0,
    .first_element = 0,
    .images        = span({gpu::ImageBinding{.image_view = color_image_view}}),
    .texel_buffers = {},
    .buffers       = {}});

  auto color_storage_texture_label = tag("Color Storage Texture"_str);
  auto color_storage_texture =
    sys->dev_
      ->create_descriptor_set(gpu::DescriptorSetInfo{
        .label            = color_storage_texture_label,
        .layout           = sys->descriptors_layout_.storage_textures,
        .variable_lengths = span({1U})})
      .unwrap();

  sys->dev_->update_descriptor_set(gpu::DescriptorSetUpdate{
    .set           = color_storage_texture,
    .binding       = 0,
    .first_element = 0,
    .images        = span({gpu::ImageBinding{.image_view = color_image_view}}),
    .texel_buffers = {},
    .buffers       = {}});

  auto color_input_attachment_label = tag("Color Input Attachment"_str);
  auto color_input_attachment =
    sys->dev_
      ->create_descriptor_set(gpu::DescriptorSetInfo{
        .label            = color_input_attachment_label,
        .layout           = sys->descriptors_layout_.input_attachments,
        .variable_lengths = span({1U})})
      .unwrap();

  sys->dev_->update_descriptor_set(gpu::DescriptorSetUpdate{
    .set           = color_input_attachment,
    .binding       = 0,
    .first_element = 0,
    .images        = span({gpu::ImageBinding{.image_view = color_image_view}}),
    .texel_buffers = {},
    .buffers       = {}});

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
     .usage        = DepthStencilTexture::USAGE,
     .aspects      = gpu::ImageAspects::Depth | gpu::ImageAspects::Stencil,
     .extent       = target_size.append(1),
     .mip_levels   = 1,
     .array_layers = 1,
     .sample_count = gpu::SampleCount::C1,
     .memory_type  = gpu::MemoryType::Aliased};

  auto depth_stencil_image =
    sys->dev_->create_image(depth_stencil_info).unwrap();

  depth_stencil_info.label = {};

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

  auto depth_image_view =
    sys->dev_->create_image_view(depth_view_info).unwrap();

  depth_view_info.label = {};

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
    sys->dev_->create_image_view(stencil_view_info).unwrap();

  stencil_view_info.label = {};

  auto depth_sampled_texture_label = tag("Depth Sampled Texture"_str);
  auto depth_sampled_texture =
    sys->dev_
      ->create_descriptor_set(gpu::DescriptorSetInfo{
        .label            = depth_sampled_texture_label,
        .layout           = sys->descriptors_layout_.sampled_textures,
        .variable_lengths = span({1U})})
      .unwrap();

  sys->dev_->update_descriptor_set(gpu::DescriptorSetUpdate{
    .set           = depth_sampled_texture,
    .binding       = 0,
    .first_element = 0,
    .images        = span({gpu::ImageBinding{.image_view = depth_image_view}}),
    .texel_buffers = {},
    .buffers       = {}});

  auto depth_storage_texture_label = tag("Depth Storage Texture"_str);
  auto depth_storage_texture =
    sys->dev_
      ->create_descriptor_set(gpu::DescriptorSetInfo{
        .label            = depth_storage_texture_label,
        .layout           = sys->descriptors_layout_.storage_textures,
        .variable_lengths = span({1U})})
      .unwrap();

  sys->dev_->update_descriptor_set(gpu::DescriptorSetUpdate{
    .set           = depth_storage_texture,
    .binding       = 0,
    .first_element = 0,
    .images        = span({gpu::ImageBinding{.image_view = depth_image_view}}),
    .texel_buffers = {},
    .buffers       = {}});

  auto depth_input_attachment_label = tag("Depth Input Attachment"_str);
  auto depth_input_attachment =
    sys->dev_
      ->create_descriptor_set(gpu::DescriptorSetInfo{
        .label            = depth_input_attachment_label,
        .layout           = sys->descriptors_layout_.input_attachments,
        .variable_lengths = span({1U})})
      .unwrap();

  sys->dev_->update_descriptor_set(gpu::DescriptorSetUpdate{
    .set           = depth_input_attachment,
    .binding       = 0,
    .first_element = 0,
    .images        = span({gpu::ImageBinding{.image_view = depth_image_view}}),
    .texel_buffers = {},
    .buffers       = {}});

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

  auto alias_label = tag("Alias"_str);
  auto alias       = sys->dev_
                 ->create_alias(gpu::AliasInfo{
                   .label     = alias_label,
                   .resources = span<Enum<gpu::Buffer, gpu::Image>>(
                     {color_image, depth_stencil_image})})
                 .unwrap();

  return TextureUnion{
    .color = color, .depth_stencil = depth_stencil, .alias = alias};
}

void ScratchTextures::uninit(gpu::Device device)
{
  for (auto & scratch : textures)
  {
    scratch.uninit(device);
  }
  textures.clear();
}

ScratchTextures ScratchTextures::create(GpuSys sys, u32 num_scratch,
                                        u32x2       target_size,
                                        gpu::Format color_format,
                                        gpu::Format depth_stencil_format,
                                        Str label, Allocator allocator,
                                        Allocator scratch)
{
  Vec<TextureUnion> textures{allocator};

  for (auto i : range(num_scratch))
  {
    auto union_label = sformat(scratch, "{} / {}"_str, label, i).unwrap();
    auto union_texture =
      TextureUnion::create(sys, target_size, color_format, depth_stencil_format,
                           union_label, scratch);
    textures.push(union_texture).unwrap();
  }

  return ScratchTextures{.textures = std::move(textures)};
}

void ScratchBuffers::uninit(gpu::Device device)
{
  for (auto buffer : buffers)
  {
    buffer.uninit(device);
  }
}

ScratchBuffers ScratchBuffers::create(GpuSys sys, Span<u64 const> sizes,
                                      Str label, Allocator allocator,
                                      Allocator scratch)
{
  Vec<GpuBuffer> buffers{allocator};
  for (auto [i, size] : enumerate(sizes))
  {
    auto tag    = sformat(scratch, "{} / Buffer {}"_str, label, i).unwrap();
    auto buffer = GpuBuffer::create(sys, size, GpuBuffer::USAGE, tag, scratch);
    buffers.push(buffer).unwrap();
  }
  return ScratchBuffers{.buffers = std::move(buffers)};
}

ColorTexture create_target_texture(GpuSys sys, u32x2 target_size,
                                   gpu::Format color_format, Str label,
                                   Allocator scratch)
{
  auto tag = [&](Str component) {
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

  auto color_image = sys->dev_->create_image(color_info).unwrap();

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

  auto color_image_view =
    sys->dev_->create_image_view(color_view_info).unwrap();

  auto sampled_color_texture_label = tag("Sampled Color Texture"_str);
  auto sampled_color_texture =
    sys->dev_
      ->create_descriptor_set(gpu::DescriptorSetInfo{
        .label            = sampled_color_texture_label,
        .layout           = sys->descriptors_layout_.sampled_textures,
        .variable_lengths = span({1U})})
      .unwrap();

  sys->dev_->update_descriptor_set(gpu::DescriptorSetUpdate{
    .set           = sampled_color_texture,
    .binding       = 0,
    .first_element = 0,
    .images        = span({gpu::ImageBinding{.image_view = color_image_view}}),
    .texel_buffers = {},
    .buffers       = {}});

  auto storage_color_texture_label = tag("Storage Color Texture"_str);
  auto storage_color_texture =
    sys->dev_
      ->create_descriptor_set(gpu::DescriptorSetInfo{
        .label            = storage_color_texture_label,
        .layout           = sys->descriptors_layout_.storage_textures,
        .variable_lengths = span({1U})})
      .unwrap();

  sys->dev_->update_descriptor_set(gpu::DescriptorSetUpdate{
    .set           = storage_color_texture,
    .binding       = 0,
    .first_element = 0,
    .images        = span({gpu::ImageBinding{.image_view = color_image_view}}),
    .texel_buffers = {},
    .buffers       = {}});

  auto input_attachment_label = tag("Input Attachment"_str);
  auto input_attachment =
    sys->dev_
      ->create_descriptor_set(gpu::DescriptorSetInfo{
        .label            = input_attachment_label,
        .layout           = sys->descriptors_layout_.input_attachments,
        .variable_lengths = span({1U})})
      .unwrap();

  sys->dev_->update_descriptor_set(gpu::DescriptorSetUpdate{
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

void GpuFrameResources::uninit(gpu::Device device)
{
  buffer.uninit(device);
  target.uninit(device);
  scratch_buffers.uninit(device);
  scratch_textures.uninit(device);
  queries.uninit(device);
}

void grow_buffer(GpuSys sys, Str label, GpuBuffer & buffer, u64 next_capacity,
                 Allocator scratch)
{
  if (buffer.capacity < next_capacity)
  {
    buffer.uninit(sys->dev_);
    buffer =
      GpuBuffer::create(sys, next_capacity, buffer.usage, label, scratch);
  }
  else if (buffer.capacity > HalfGrowth::grow(next_capacity))
  {
    // Target at least 75% utilization
    buffer.uninit(sys->dev_);
    buffer =
      GpuBuffer::create(sys, next_capacity, buffer.usage, label, scratch);
  }
}

void ScratchBuffers::grow(GpuSys sys, Span<u64 const> sizes, Str label,
                          Allocator allocator, Allocator scratch)
{
  if (buffers.size() != sizes.size())
  {
    uninit(sys->dev_);
    *this = create(sys, sizes, label, allocator, scratch);
    return;
  }

  for (auto [buffer, size] : zip(buffers, sizes))
  {
    grow_buffer(sys, label, buffer, size, scratch);
  }
}

gpu::Device IGpuFrame::dev() const
{
  return dev_;
}

GpuSys IGpuFrame::sys() const
{
  return sys_;
}

gpu::Swapchain IGpuFrame::swapchain() const
{
  return sys_->swapchain_;
}

ColorTexture IGpuFrame::target() const
{
  return resources_.target;
}

gpu::DescriptorSet IGpuFrame::sampled_textures() const
{
  return sys_->descriptors_.sampled_textures;
}

gpu::DescriptorSet IGpuFrame::samplers() const
{
  return sys_->descriptors_.samplers;
}

gpu::CommandEncoder IGpuFrame::command_encoder() const
{
  return command_encoder_;
}

gpu::CommandBuffer IGpuFrame::command_buffer() const
{
  return command_buffer_;
}

void IGpuFrame::uninit()
{
  resources_.uninit(dev_);
  dev_->uninit(command_encoder_);
  dev_->uninit(command_buffer_);
}

Option<Tuple<gpu::TimestampQuery, u32>> IGpuFrame::allocate_timestamp()
{
  CHECK(state_ == GpuFrameState::Recording, "");
  if (next_timestamp_ >= resources_.queries.cpu_timestamps.size())
  {
    return none;
  }

  auto idx = next_timestamp_++;

  return Tuple{resources_.queries.timestamps, idx};
}

Option<Tuple<gpu::StatisticsQuery, u32>> IGpuFrame::allocate_statistics()
{
  CHECK(state_ == GpuFrameState::Recording, "");
  if (next_statistics_ >= resources_.queries.cpu_statistics.size())
  {
    return none;
  }

  auto idx = next_statistics_++;

  return Tuple{resources_.queries.statistics, idx};
}

void IGpuFrame::get_scratch_textures(Span<TextureUnion> textures)
{
  CHECK(state_ == GpuFrameState::Recording, "");
  CHECK(textures.size() <= resources_.scratch_textures.textures.size(), "");
  for (auto & tex : textures)
  {
    tex = resources_.scratch_textures.textures[next_scratch_texture_];
    next_scratch_texture_++;
    next_scratch_texture_ =
      next_scratch_texture_ % resources_.scratch_textures.textures.size();
  }
}

void IGpuFrame::get_scratch_buffers(Span<GpuBuffer> buffers)
{
  CHECK(state_ == GpuFrameState::Recording, "");
  CHECK(buffers.size() <= resources_.scratch_buffers.buffers.size(), "");
  for (auto [tgt, src] : zip(buffers, resources_.scratch_buffers.buffers))
  {
    tgt = src;
  }
}

GpuBufferSpan IGpuFrame::get(GpuBufferId id)
{
  CHECK(state_ == GpuFrameState::Recording, "");
  Slice64 slice = current_plan_->gpu_buffer_entries_.get((usize) id);
  return {resources_.buffer, slice};
}

Span<u8 const> IGpuFrame::get(BufferId id)
{
  CHECK(state_ == GpuFrameState::Recording, "");
  CHECK(current_plan_ != nullptr, "");
  auto slice = current_plan_->cpu_buffer_entries_.get((usize) id);
  return current_plan_->cpu_buffer_data_.view().slice(slice);
}

void IGpuFrame::begin()
{
  CHECK(state_ == GpuFrameState::Reset, "");
  state_            = GpuFrameState::Recording;
  submission_stage_ = semaphore_->stage();
}

void IGpuFrame::cmd(GpuFramePlan plan)
{
  CHECK(state_ == GpuFrameState::Recording, "");
  CHECK(plan != nullptr, "");
  CHECK(plan->state_ == GpuFramePlanState::Recorded, "");
  current_plan_         = plan;
  current_plan_->state_ = GpuFramePlanState::Submitted;
}

void IGpuFrame::end()
{
  CHECK(state_ == GpuFrameState::Recording, "");
  CHECK(current_plan_ != nullptr, "");
  state_ = GpuFrameState::Recorded;
}

void IGpuFrame::submit()
{
  CHECK(state_ == GpuFrameState::Recorded, "");
  char              scratch_buffer_[1_KB];
  FallbackAllocator scratch{Arena::from(scratch_buffer_), allocator_};

  // [ ] collect time and statistics traces

  {
    auto label = sformat(scratch, "GpuFrame {} / Buffer"_str, id_).unwrap();
    CHECK(current_plan_->gpu_buffer_data_.size() <= cfg_.max_buffer_size, "");
    auto size = clamp(current_plan_->gpu_buffer_data_.size(),
                      cfg_.min_buffer_size, cfg_.max_buffer_size);
    grow_buffer(sys_, label, resources_.buffer, size, scratch);
    mem::copy(current_plan_->gpu_buffer_data_.view(),
              dev_->get_memory_map(resources_.buffer.buffer).unwrap());
  }

  if (target_info_ != current_plan_->target_)
  {
    resources_.target.uninit(dev_);
    auto label = sformat(scratch, "GpuFrame {} / Target"_str, id_).unwrap();
    resources_.target = create_target_texture(
      sys_, target_info_.extent, target_info_.color_format, label, scratch);
  }

  {
    auto label =
      sformat(scratch, "GpuFrame {} / Scratch Buffers"_str, id_).unwrap();
    for (auto s : current_plan_->scratch_buffer_sizes_)
    {
      CHECK(s <= cfg_.max_scratch_buffer_size, "");
    }

    CHECK(current_plan_->scratch_buffer_sizes_.size() <=
            cfg_.max_scratch_buffers,
          "");

    Vec<u64> sizes{scratch};

    for (auto s : current_plan_->scratch_buffer_sizes_)
    {
      sizes
        .push(
          clamp(s, cfg_.min_scratch_buffer_size, cfg_.max_scratch_buffer_size))
        .unwrap();
    }

    resources_.scratch_buffers.grow(sys_, sizes, label, allocator_, scratch);
  }

  CHECK(current_plan_->num_scratch_textures_ <= cfg_.max_scratch_textures, "");

  auto num_scratch_textures =
    clamp(current_plan_->num_scratch_textures_, cfg_.min_scratch_textures,
          cfg_.max_scratch_textures);

  if (target_info_ != current_plan_->target_ ||
      resources_.scratch_textures.textures.size() != num_scratch_textures)
  {
    resources_.scratch_textures.uninit(dev_);
    auto label =
      sformat(scratch, "GpuFrame {} / Scratch Textures"_str, id_).unwrap();
    resources_.scratch_textures = ScratchTextures::create(
      sys_, num_scratch_textures, target_info_.extent,
      target_info_.color_format, target_info_.depth_stencil_format, label,
      allocator_, scratch);
  }

  target_info_ = current_plan_->target_;

  if (sys_->cfg_.frame_timestamps_capacity !=
        resources_.queries.timestamps_capacity() ||
      sys_->cfg_.frame_statistics_capacity !=
        resources_.queries.statistics_capacity())
  {
    auto label = sformat(scratch, "GpuFrame {} / Queries"_str, id_).unwrap();
    resources_.queries.uninit(dev_);
    resources_.queries = GpuQueries::create(
      allocator_, dev_, label, sys_->cfg_.frame_timestamps_capacity,
      sys_->cfg_.frame_statistics_capacity, scratch);
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
    pass(this);
  }

  command_encoder_->end().unwrap();
  current_plan_->semaphore_->increment(1);

  command_buffer_->begin();
  command_buffer_->record(command_encoder_);
  command_buffer_->end().unwrap();

  scope_frame_id_ = dev_->submit(command_buffer_, sys_->queue_scope_).unwrap();

  for (auto & task : current_plan_->post_frame_tasks_)
  {
    task();
  }

  state_ = GpuFrameState::Submitted;
}

bool IGpuFrame::try_complete(nanoseconds timeout)
{
  CHECK(state_ == GpuFrameState::Submitted, "");

  if (!dev_->await_queue_scope_frame(sys_->queue_scope_, scope_frame_id_,
                                     timeout))
  {
    return false;
  }

  dev_
    ->get_timestamp_query_result(resources_.queries.timestamps, 0,
                                 resources_.queries.cpu_timestamps)
    .unwrap();
  dev_
    ->get_statistics_query_result(resources_.queries.statistics, 0,
                                  resources_.queries.cpu_statistics)
    .unwrap();

  for (auto & task : current_plan_->frame_completed_tasks_)
  {
    task();
  }

  current_plan_->state_ = GpuFramePlanState::Executed;
  state_                = GpuFrameState::Completed;
  semaphore_->increment(1);

  return true;
}

void IGpuFrame::reset()
{
  CHECK(state_ != GpuFrameState::Submitted, "");
  next_scratch_texture_ = 0;
  next_scratch_texture_ = 0;
  next_statistics_      = 0;
  command_encoder_->reset();
  command_buffer_->reset();
  current_plan_ = nullptr;
}

bool IGpuFrame::await(nanoseconds timeout)
{
  return semaphore_->await(submission_stage_, timeout);
}

static Option<gpu::Format> select_color_format(gpu::Device             dev,
                                               Span<gpu::Format const> formats)
{
  for (auto fmt : formats)
  {
    gpu::FormatProperties props = dev->get_format_properties(fmt).unwrap();
    if (has_bits(props.optimal_tiling_features, ColorTexture::FEATURES))
    {
      return fmt;
    }
  }

  return none;
}

static Option<gpu::Format>
  select_depth_stencil_format(gpu::Device dev, Span<gpu::Format const> formats)
{
  for (auto fmt : formats)
  {
    gpu::FormatProperties props = dev->get_format_properties(fmt).unwrap();
    if (has_bits(props.optimal_tiling_features, DepthStencilTexture::FEATURES))
    {
      return fmt;
    }
  }

  return none;
}

gpu::Swapchain create_surface_swapchain(
  gpu::Device device, Str label, gpu::Surface surface, u32 buffering,
  u32x2 initial_extent, Span<gpu::SurfaceFormat const> preferred_formats,
  Span<gpu::PresentMode const> preferred_present_modes,
  gpu::CompositeAlpha composite_alpha, Allocator scratch)
{
  CHECK(!initial_extent.any_zero(), "");

  Vec<gpu::SurfaceFormat> formats{scratch};
  device->get_surface_formats(surface, formats).unwrap();
  Vec<gpu::PresentMode> present_modes{scratch};
  device->get_surface_present_modes(surface, present_modes).unwrap();

  auto selected_format =
    find(preferred_formats, formats.view(),
         [](gpu::SurfaceFormat pref, Span<gpu::SurfaceFormat const> formats) {
           return !find(formats, pref, bit_eq).is_empty();
         });

  CHECK(!selected_format.is_empty(), "");

  auto selected_present_mode =
    find(preferred_present_modes, present_modes.view(),
         [](gpu::PresentMode pref, Span<gpu::PresentMode const> modes) {
           return !find(modes, pref).is_empty();
         });

  CHECK(!selected_present_mode.is_empty(), "");

  auto capabilities = device->get_surface_capabilities(surface).unwrap();

  CHECK(has_bits(capabilities.image_usage, gpu::ImageUsage::TransferDst |
                                             gpu::ImageUsage::ColorAttachment),
        "");

  return device
    ->create_swapchain(gpu::SwapchainInfo{
      .label   = label,
      .surface = surface,
      .format  = selected_format[0],
      .usage = gpu::ImageUsage::TransferDst | gpu::ImageUsage::ColorAttachment,
      .preferred_buffering = buffering,
      .present_mode        = selected_present_mode[0],
      .preferred_extent    = initial_extent,
      .composite_alpha     = composite_alpha})
    .unwrap();
}

void IGpuSys::uninit(Vec<u8> & cache)
{
  auto drain_semaphore = scheduler_->get_drain_semaphore(thread_id_);
  CHECK(drain_semaphore->complete(0), "");
  drain_semaphore->await(1ULL, nanoseconds::max());
  dev_->await_idle().unwrap();
  dev_->get_pipeline_cache_data(pipeline_cache_, cache).unwrap();

  for (auto & frame : frames_)
  {
    frame->uninit();
  }

  for (auto & plan : plans_)
  {
    plan.uninit();
  }

  descriptors_.uninit(dev_);
  for (auto [info, sampler] : sampler_cache_)
  {
    dev_->uninit(sampler.v1);
  }
  dev_->uninit(queue_scope_);
  dev_->uninit(swapchain_);
  descriptors_layout_.uninit(dev_);
  for (auto view : default_image_views_)
  {
    dev_->uninit(view);
  }
  dev_->uninit(default_image_);
  dev_->uninit(pipeline_cache_);
}

void create_default_samplers(GpuSys sys, Allocator scratch)
{
  constexpr Tuple<Str, gpu::BorderColor> colors[] = {
    {"FloatTransparentBlack"_str, gpu::BorderColor::FloatTransparentBlack},
    {"IntTransparentBlack"_str,   gpu::BorderColor::IntTransparentBlack  },
    {"FloatOpaqueBlack"_str,      gpu::BorderColor::FloatOpaqueBlack     },
    {"IntOpaqueBlack"_str,        gpu::BorderColor::IntOpaqueBlack       },
    {"FloatOpaqueueWhite"_str,    gpu::BorderColor::FloatOpaqueueWhite   },
    {"IntOpaqueueWhite"_str,      gpu::BorderColor::IntOpaqueueWhite     }
  };

  constexpr Tuple<Str, gpu::SamplerAddressMode> address_modes[] = {
    {"Repeat"_str,            gpu::SamplerAddressMode::Repeat           },
    {"MirroredRepeat"_str,    gpu::SamplerAddressMode::MirroredRepeat   },
    {"ClampToEdge"_str,       gpu::SamplerAddressMode::ClampToEdge      },
    {"ClampToBorder"_str,     gpu::SamplerAddressMode::ClampToBorder    },
    {"MirrorClampToEdge"_str, gpu::SamplerAddressMode::MirrorClampToEdge}
  };

  constexpr Tuple<Str, gpu::Filter, gpu::SamplerMipMapMode> mip_map_modes[] = {
    {"Linear"_str,  gpu::Filter::Linear,  gpu::SamplerMipMapMode::Linear },
    {"Nearest"_str, gpu::Filter::Nearest, gpu::SamplerMipMapMode::Nearest}
  };

  for (auto [mip_map_mode_name, filter, mip_map_mode] : mip_map_modes)
  {
    for (auto [address_mode_name, adress_mode] : address_modes)
    {
      for (auto [color_name, color] : colors)
      {
        auto label = sformat(scratch, "/ Sampler: {} + {} + {}"_str,
                             mip_map_mode_name, address_mode_name, color_name)
                       .unwrap();
        [[maybe_unused]] auto id = sys->create_cached_sampler(
          gpu::SamplerInfo{.label                    = label,
                           .mag_filter               = filter,
                           .min_filter               = filter,
                           .mip_map_mode             = mip_map_mode,
                           .address_mode_u           = adress_mode,
                           .address_mode_v           = adress_mode,
                           .address_mode_w           = adress_mode,
                           .mip_lod_bias             = 0,
                           .anisotropy_enable        = false,
                           .max_anisotropy           = 1.0,
                           .compare_enable           = false,
                           .compare_op               = gpu::CompareOp::Never,
                           .min_lod                  = 0,
                           .max_lod                  = 0,
                           .border_color             = color,
                           .unnormalized_coordinates = false});
      }
    }
  }
}

void create_default_textures(GpuSys sys)
{
  gpu::Image default_image =
    sys->dev_
      ->create_image(gpu::ImageInfo{
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

  static constexpr Array<Tuple<Str, TextureId, gpu::ComponentMapping>,
                         NUM_DEFAULT_TEXTURES>
    mappings{
      {{"Transparent Texture"_str,
        TextureId::Transparent,
        {.r = gpu::ComponentSwizzle::Zero,
         .g = gpu::ComponentSwizzle::Zero,
         .b = gpu::ComponentSwizzle::Zero,
         .a = gpu::ComponentSwizzle::Zero}},
       {"RedTransparent Texture"_str,
        TextureId::RedTransparent,
        {.r = gpu::ComponentSwizzle::One,
         .g = gpu::ComponentSwizzle::Zero,
         .b = gpu::ComponentSwizzle::Zero,
         .a = gpu::ComponentSwizzle::Zero}},
       {"GreenTransparent Texture"_str,
        TextureId::GreenTransparent,
        {.r = gpu::ComponentSwizzle::Zero,
         .g = gpu::ComponentSwizzle::One,
         .b = gpu::ComponentSwizzle::Zero,
         .a = gpu::ComponentSwizzle::Zero}},
       {"BlueTransparent Texture"_str,
        TextureId::BlueTransparent,
        {.r = gpu::ComponentSwizzle::Zero,
         .g = gpu::ComponentSwizzle::Zero,
         .b = gpu::ComponentSwizzle::One,
         .a = gpu::ComponentSwizzle::Zero}},
       {"YellowTransparent Texture"_str,
        TextureId::YellowTransparent,
        {.r = gpu::ComponentSwizzle::One,
         .g = gpu::ComponentSwizzle::One,
         .b = gpu::ComponentSwizzle::Zero,
         .a = gpu::ComponentSwizzle::Zero}},
       {"MagentaTransparent Texture"_str,
        TextureId::MagentaTransparent,
        {.r = gpu::ComponentSwizzle::One,
         .g = gpu::ComponentSwizzle::Zero,
         .b = gpu::ComponentSwizzle::One,
         .a = gpu::ComponentSwizzle::Zero}},
       {"CyanTransparent Texture"_str,
        TextureId::CyanTransparent,
        {.r = gpu::ComponentSwizzle::Zero,
         .g = gpu::ComponentSwizzle::One,
         .b = gpu::ComponentSwizzle::One,
         .a = gpu::ComponentSwizzle::Zero}},
       {"WhiteTransparent Texture"_str,
        TextureId::WhiteTransparent,
        {.r = gpu::ComponentSwizzle::One,
         .g = gpu::ComponentSwizzle::One,
         .b = gpu::ComponentSwizzle::One,
         .a = gpu::ComponentSwizzle::Zero}},
       {"Black Texture"_str,
        TextureId::Black,
        {.r = gpu::ComponentSwizzle::Zero,
         .g = gpu::ComponentSwizzle::Zero,
         .b = gpu::ComponentSwizzle::Zero,
         .a = gpu::ComponentSwizzle::One}},
       {"Red Texture"_str,
        TextureId::Red,
        {.r = gpu::ComponentSwizzle::One,
         .g = gpu::ComponentSwizzle::Zero,
         .b = gpu::ComponentSwizzle::Zero,
         .a = gpu::ComponentSwizzle::One}},
       {"Green Texture"_str,
        TextureId::Green,
        {.r = gpu::ComponentSwizzle::Zero,
         .g = gpu::ComponentSwizzle::One,
         .b = gpu::ComponentSwizzle::Zero,
         .a = gpu::ComponentSwizzle::One}},
       {"Blue Texture"_str,
        TextureId::Blue,
        {.r = gpu::ComponentSwizzle::Zero,
         .g = gpu::ComponentSwizzle::Zero,
         .b = gpu::ComponentSwizzle::One,
         .a = gpu::ComponentSwizzle::One}},
       {"Yellow Texture"_str,
        TextureId::Yellow,
        {.r = gpu::ComponentSwizzle::One,
         .g = gpu::ComponentSwizzle::One,
         .b = gpu::ComponentSwizzle::Zero,
         .a = gpu::ComponentSwizzle::One}},
       {"Magenta Texture"_str,
        TextureId::Magenta,
        {.r = gpu::ComponentSwizzle::One,
         .g = gpu::ComponentSwizzle::Zero,
         .b = gpu::ComponentSwizzle::One,
         .a = gpu::ComponentSwizzle::One}},
       {"Cyan Texture"_str,
        TextureId::Cyan,
        {.r = gpu::ComponentSwizzle::Zero,
         .g = gpu::ComponentSwizzle::One,
         .b = gpu::ComponentSwizzle::One,
         .a = gpu::ComponentSwizzle::One}},
       {"White Texture"_str,
        TextureId::White,
        {.r = gpu::ComponentSwizzle::One,
         .g = gpu::ComponentSwizzle::One,
         .b = gpu::ComponentSwizzle::One,
         .a = gpu::ComponentSwizzle::One}}}
  };

  Array<gpu::ImageView, NUM_DEFAULT_TEXTURES> default_image_views;

  for (auto [mapping, view] : zip(mappings, default_image_views))
  {
    view = sys->dev_
             ->create_image_view(gpu::ImageViewInfo{
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

    CHECK(mapping.v1 == sys->alloc_texture_id(view), "");
  }

  sys->default_image_       = default_image;
  sys->default_image_views_ = default_image_views;
}

void IGpuSys::init(Allocator allocator, gpu::Device device,
                   Span<u8 const> pipeline_cache_data, gpu::Surface surface,
                   GpuSysPreferences const & preferences, Scheduler scheduler,
                   ThreadId thread_id)
{
  char              scratch_buffer_[1_KB];
  FallbackAllocator scratch{Arena::from(scratch_buffer_), allocator_};

  // [ ] use timeline semaphore

  CHECK(preferences.buffering > 0, "");
  CHECK(preferences.buffering <= MAX_BUFFERING, "");
  CHECK(!preferences.initial_extent.any_zero(), "");

  allocator_ = allocator;
  dev_       = device;
  surface_   = surface;
  props_     = device->get_properties();
  pipeline_cache_ =
    dev_
      ->create_pipeline_cache(gpu::PipelineCacheInfo{
        .label = "/ PipelineCache"_str, .initial_data = pipeline_cache_data})
      .unwrap();
  buffering_ = preferences.buffering;

  color_format_ =
    select_color_format(dev_, preferences.color_formats)
      .unwrap("Device doesn't support any preferred color format"_str);

  depth_stencil_format_ =
    select_depth_stencil_format(dev_, preferences.depth_stencil_formats)
      .unwrap("Device doesn't support any preferred depth-stencil formats"_str);

  trace("Selected color format: {}"_str, color_format_);

  trace("Selected depth stencil format: {}"_str, depth_stencil_format_);

  descriptors_layout_ = GpuDescriptorsLayout::create(
    dev_, "/ DescriptorsLayout"_str, cfg_, scratch);

  swapchain_ = create_surface_swapchain(
    dev_, "/ Swapchain"_str, surface_, buffering_, preferences.initial_extent,
    preferences.swapchain_formats, preferences.swapchain_present_modes,
    preferences.swapchain_composite_alpha, scratch);

  queue_scope_ = dev_
                   ->create_queue_scope(gpu::QueueScopeInfo{
                     .label = "/ QueueScope"_str, .buffering = buffering_})
                   .unwrap();

  sampler_cache_ = SamplerCache{allocator_};
  descriptors_   = GpuDescriptors::create(this, "/ Descriptors", scratch);

  auto frames = Vec<Dyn<GpuFrame>>::make(buffering_, allocator_).unwrap();

  for (auto i : range(buffering_))
  {
    // start as signaled semaphore
    auto semaphore = dyn<ISemaphore>(inplace, allocator_, 1ULL).unwrap();

    auto encoder_label =
      sformat(scratch, "/ GpuFrame / CommandEncoder {}"_str, i).unwrap();

    auto encoder = dev_
                     ->create_command_encoder(
                       gpu::CommandEncoderInfo{.label = encoder_label})
                     .unwrap();

    auto buffer_label =
      sformat(scratch, "/ GpuFrame / CommandBuffer {}"_str, i).unwrap();

    auto buffer =
      dev_->create_command_buffer(gpu::CommandBufferInfo{.label = buffer_label})
        .unwrap();

    auto frame = dyn<IGpuFrame>(inplace, allocator_, allocator_, dev_, this, i,
                                std::move(semaphore), encoder, buffer)
                   .unwrap();

    frames.push(std::move(frame)).unwrap();
  }

  frames_ = std::move(frames);

  auto plans = Vec<Dyn<GpuFramePlan>>::make(buffering_, allocator_).unwrap();

  for (auto _ : range(buffering_))
  {
    // start as signaled semaphore
    auto semaphore = dyn<ISemaphore>(inplace, allocator_, 1ULL).unwrap();
    auto plan      = dyn<IGpuFramePlan>(inplace, allocator_, allocator_, this,
                                        std::move(semaphore))
                  .unwrap();
    plans.push(std::move(plan)).unwrap();
  }

  plans_       = std::move(plans);
  scheduler_   = scheduler;
  thread_id_   = thread_id;
  initialized_ = true;

  create_default_textures(this);
  create_default_samplers(this, scratch);
}

SamplerId IGpuSys::create_cached_sampler(gpu::SamplerInfo const & info_)
{
  CHECK(initialized_, "");
  LockGuard guard{resources_lock_};

  auto info  = info_;
  info.label = {};

  auto found = sampler_cache_.try_get(info);

  if (found)
  {
    return found.v().v0;
  }

  CHECK(descriptors_.samplers_map.size() < descriptors_.samplers_capacity,
        "Ran out of sampler descriptor slots");

  auto sampler = dev_->create_sampler(info_).unwrap();

  auto id = static_cast<SamplerId>(descriptors_.samplers_map.push().unwrap());

  sampler_cache_.push(info, Tuple{id, sampler}).unwrap();

  plan()->add_preframe_task([device   = this->dev_,
                             samplers = descriptors_.samplers,
                             index    = static_cast<u32>(id), sampler] {
    device->update_descriptor_set(gpu::DescriptorSetUpdate{
      .set           = samplers,
      .binding       = 0,
      .first_element = index,
      .images        = span({gpu::ImageBinding{.sampler = sampler}}),
      .texel_buffers = {},
      .buffers       = {}});
  });

  return id;
}

TextureId IGpuSys::alloc_texture_id(gpu::ImageView view)
{
  CHECK(initialized_, "");

  LockGuard guard{resources_lock_};

  CHECK(descriptors_.sampled_textures_map.size() <
          descriptors_.sampled_textures_capacity,
        "Ran out of sampled texture descriptor slots");

  auto id =
    static_cast<TextureId>(descriptors_.sampled_textures_map.push().unwrap());

  plan()->add_preframe_task([device   = this->dev_,
                             textures = descriptors_.sampled_textures,
                             index    = static_cast<u32>(id), view] {
    device->update_descriptor_set(gpu::DescriptorSetUpdate{
      .set           = textures,
      .binding       = 0,
      .first_element = index,
      .images        = span({gpu::ImageBinding{.image_view = view}}),
      .texel_buffers = {},
      .buffers       = {}});
  });

  return id;
}

void IGpuSys::release_texture_id(TextureId id)
{
  CHECK(initialized_, "");

  LockGuard guard{resources_lock_};

  descriptors_.sampled_textures_map.erase(id);

  plan()->add_preframe_task([device   = this->dev_,
                             textures = descriptors_.sampled_textures,
                             index    = static_cast<u32>(id)] {
    device->update_descriptor_set(
      gpu::DescriptorSetUpdate{.set           = textures,
                               .binding       = 0,
                               .first_element = index,
                               .images        = span({gpu::ImageBinding{}}),
                               .texel_buffers = {},
                               .buffers       = {}});
  });
}

gpu::Device IGpuSys::device()
{
  CHECK(initialized_, "");
  return dev_;
}

Allocator IGpuSys::allocator() const
{
  return allocator_;
}

GpuFramePlan IGpuSys::plan()
{
  CHECK(initialized_, "");
  return plans_[frame_ring_index_].get();
}

gpu::Format IGpuSys::color_format() const
{
  return color_format_;
}

gpu::Format IGpuSys::depth_stencil_format() const
{
  return depth_stencil_format_;
}

gpu::SampleCount IGpuSys::sample_count() const
{
  return sample_count_;
}

gpu::PipelineCache IGpuSys::pipeline_cache() const
{
  return pipeline_cache_;
}

void IGpuSys::submit_frame()
{
  CHECK(initialized_, "");

  auto * frame = frames_[frame_ring_index_].get();
  auto * plan  = plans_[frame_ring_index_].get();

  scheduler_->once(
    [frame, plan] {
      frame->await(nanoseconds::max());
      frame->reset();
      frame->begin();
      frame->cmd(plan);
      frame->end();
      frame->submit();

      // [ ] complete????; will need to be in order
    },
    Ready{}, thread_id_);

  frame_ring_index_ = (frame_ring_index_ + 1) % buffering_;

  // wait on the next frame plan
  plans_[frame_ring_index_]->await(nanoseconds::max());
}

// [ ] move to scene construction
/*
void GpuSys::frame(gpu::Swapchain swapchain)
{

  ScopeTrace trace;
  dev_->begin_frame(swapchain).unwrap();
  uninit_objects(*dev_, released_objects_[ring_index()]);
  released_objects_[ring_index()].clear();

  auto & enc = encoder();

  queries_[ring_index()].begin_frame(*dev_, enc);

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
    auto swapchain_state = dev_->get_swapchain_state(swapchain).unwrap();

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

  dev_->submit_frame(swapchain).unwrap();
}
   */

}    // namespace ash
