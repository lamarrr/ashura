/// SPDX-License-Identifier: MIT
#include "ashura/engine/gpu_system.hpp"
#include "ashura/std/range.hpp"
#include "ashura/std/sformat.hpp"
#include "ashura/std/trace.hpp"

namespace ash
{

u32x3 ColorImage::extent() const
{
    return info.extent;
}

void ColorImage::uninit(gpu::Device d)
{
    d->uninit(sampled_textures);
    d->uninit(storage_textures);
    d->uninit(input_attachments);
    d->uninit(view);
    d->uninit(image);
}

gpu::SampleCount ColorMsaaImage::sample_count() const
{
    return info.sample_count;
}

u32x3 ColorMsaaImage::extent() const
{
    return info.extent;
}

void ColorMsaaImage::uninit(gpu::Device d)
{
    d->uninit(view);
    d->uninit(image);
}

u32x3 DepthStencilImage::extent() const
{
    return info.extent;
}

void DepthStencilImage::uninit(gpu::Device d)
{
    d->uninit(depth_sampled_textures);
    d->uninit(depth_input_attachments);
    d->uninit(depth_view);
    d->uninit(stencil_view);
    d->uninit(image);
}

u32x3 Framebuffer::extent() const
{
    return color.extent();
}

void Framebuffer::uninit(gpu::Device d)
{
    color.uninit(d);
    color_msaa.match([&](auto & c) { c.uninit(d); });
    depth_stencil.match([&](auto & s) { s.uninit(d); });
}

void GpuBuffer::uninit(gpu::Device d)
{
    d->uninit(uniform_buffer);
    d->uninit(read_storage_buffer);
    d->uninit(read_write_storage_buffer);
    d->uninit(buffer);
}

GpuBuffer GpuBuffer::create(GpuSys sys, u64 capacity, gpu::BufferUsage usage, Str label)
{
    ScratchScope scratch{sys->allocator_};
    auto &       d = *sys->dev_;

    auto buffer_label = sformat(scratch, "{} / {}"_s, label, "Buffer"_s).unwrap();
    auto buffer =
      d.create_buffer(gpu::BufferInfo{.label       = buffer_label,
                                      .size        = capacity,
                                      .usage       = usage,
                                      .memory_type = gpu::MemoryType::Unique,
                                      .host_mapped = true})
        .unwrap();

    auto device_address = d.get_device_address(buffer);

    auto make_set = [&](Str component, gpu::DescriptorSetLayout layout, u64 max_size) {
        auto set_label = sformat(scratch, "{} / {}"_s, label, component).unwrap();
        auto set = d
                     .create_descriptor_set(gpu::DescriptorSetInfo{
                       .label = set_label, .layout = layout, .variable_lengths = {}})
                     .unwrap();

        d.update_descriptor_set(gpu::DescriptorSetUpdate{
          .set           = set,
          .binding       = 0,
          .first_element = 0,
          .buffers =
            span({gpu::BufferBinding{.buffer = buffer, .range{0, max_size}}}
            )
        });

        return set;
    };

    auto props = d.get_properties();

    // uniform buffer has a low limit on most systems, so we need to cap it
    auto uniform_buffer =
      make_set("Uniform Buffer"_s, sys->descriptors_layout_.uniform_buffer,
               min(capacity, (u64) props.limits.uniform_buffer_range));

    auto read_storage_buffer =
      make_set("Read StorageBuffer"_s, sys->descriptors_layout_.read_storage_buffer,
               gpu::WHOLE_SIZE);

    auto read_write_storage_buffer =
      make_set("Read/Write StorageBuffer"_s,
               sys->descriptors_layout_.read_write_storage_buffer, gpu::WHOLE_SIZE);

    return GpuBuffer{.capacity                  = capacity,
                     .usage                     = usage,
                     .buffer                    = buffer,
                     .device_address            = device_address,
                     .uniform_buffer            = uniform_buffer,
                     .read_storage_buffer       = read_storage_buffer,
                     .read_write_storage_buffer = read_write_storage_buffer};
}

void GpuQueries::uninit(gpu::Device d)
{
    d->uninit(timestamps);
    d->uninit(statistics);
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
                              u32 statistics_capacity)
{
    ScratchScope scratch{allocator};
    ASH_CHECK(timestamps_capacity > 0, "");
    ASH_CHECK(statistics_capacity > 0, "");

    auto timestamp_label = sformat(scratch, "{} / TimestampQuery"_s, label).unwrap();
    auto timestamps      = device
                             ->create_timestamp_query(gpu::TimestampQueryInfo{
                               .label = timestamp_label, .count = timestamps_capacity})
                             .unwrap();

    Vec<u64> cpu_timestamps{allocator};
    cpu_timestamps.resize_uninit(timestamps_capacity).unwrap();

    auto statistics_label = sformat(scratch, "{} / StatisticsQuery"_s, label).unwrap();
    auto statistics = device
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

void GpuDescriptorsLayout::uninit(gpu::Device d)
{
    d->uninit(samplers);
    d->uninit(sampled_textures);
    d->uninit(storage_textures);
    d->uninit(uniform_buffer);
    d->uninit(read_storage_buffer);
    d->uninit(read_write_storage_buffer);
    d->uninit(uniform_buffers);
    d->uninit(read_storage_buffers);
    d->uninit(read_write_storage_buffers);
    d->uninit(input_attachments);
}

GpuDescriptorsLayout GpuDescriptorsLayout::create(Allocator   allocator,
                                                  gpu::Device device, Str label,
                                                  GpuSysCfg const & cfg)
{
    ScratchScope scratch{allocator};
    auto         tag = [&](Str component) {
        return sformat(scratch, "{} / {}"_s, label, component).unwrap();
    };

    auto samplers_label = tag("Samplers"_s);
    auto samplers =
      device
        ->create_descriptor_set_layout(gpu::DescriptorSetLayoutInfo{
          .label = samplers_label,
          .bindings =
            span({gpu::DescriptorBindingInfo{.type  = gpu::DescriptorType::Sampler,
                                             .count = cfg.bindless_samplers_capacity,
                                             .is_variable_length = true}}
            )
    })
        .unwrap();

    auto sampled_textures_label = tag("Sampled Textures"_s);
    auto sampled_textures =
      device
        ->create_descriptor_set_layout(gpu::DescriptorSetLayoutInfo{
          .label    = sampled_textures_label,
          .bindings = span(
            {gpu::DescriptorBindingInfo{.type  = gpu::DescriptorType::SampledImage,
                                        .count = cfg.bindless_sampled_textures_capacity,
                                        .is_variable_length = true}}
              )
    })
        .unwrap();

    auto storage_textures_label = tag("Storage Textures"_s);
    auto storage_textures =
      device
        ->create_descriptor_set_layout(gpu::DescriptorSetLayoutInfo{
          .label    = storage_textures_label,
          .bindings = span(
            {gpu::DescriptorBindingInfo{.type  = gpu::DescriptorType::StorageImage,
                                        .count = cfg.bindless_storage_textures_capacity,
                                        .is_variable_length = true}}
              )
    })
        .unwrap();

    auto uniform_buffer_label = tag("Uniform Buffer"_s);
    auto uniform_buffer =
      device
        ->create_descriptor_set_layout(gpu::DescriptorSetLayoutInfo{
          .label    = uniform_buffer_label,
          .bindings = span(
            {gpu::DescriptorBindingInfo{.type  = gpu::DescriptorType::DynUniformBuffer,
                                        .count = 1,
                                        .is_variable_length = false}}
              )
    })
        .unwrap();

    auto read_storage_buffer_label = tag("Read Storage Buffer"_s);
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

    auto read_write_storage_buffer_label = tag("Read/Write Storage Buffer"_s);
    auto read_write_storage_buffer =
      device
        ->create_descriptor_set_layout(gpu::DescriptorSetLayoutInfo{
          .label    = read_write_storage_buffer_label,
          .bindings = span(
            {gpu::DescriptorBindingInfo{.type = gpu::DescriptorType::DynRWStorageBuffer,
                                        .count              = 1,
                                        .is_variable_length = false}}
              )
    })
        .unwrap();

    auto uniform_texel_buffers_label = tag("Uniform Texel Buffers"_s);
    auto uniform_texel_buffers =
      device
        ->create_descriptor_set_layout(gpu::DescriptorSetLayoutInfo{
          .label    = uniform_texel_buffers_label,
          .bindings = span({gpu::DescriptorBindingInfo{
            .type               = gpu::DescriptorType::UniformTexelBuffer,
            .count              = cfg.bindless_uniform_texel_buffers_capacity,
            .is_variable_length = true}}
            )
    })
        .unwrap();

    auto storage_texel_buffers_label = tag("Storage Texel Buffers"_s);
    auto storage_texel_buffers =
      device
        ->create_descriptor_set_layout(gpu::DescriptorSetLayoutInfo{
          .label    = storage_texel_buffers_label,
          .bindings = span({gpu::DescriptorBindingInfo{
            .type               = gpu::DescriptorType::UniformTexelBuffer,
            .count              = cfg.bindless_uniform_texel_buffers_capacity,
            .is_variable_length = true}}
            )
    })
        .unwrap();

    auto uniform_buffers_label = tag("Uniform Buffers"_s);
    auto uniform_buffers =
      device
        ->create_descriptor_set_layout(gpu::DescriptorSetLayoutInfo{
          .label    = uniform_buffers_label,
          .bindings = span(
            {gpu::DescriptorBindingInfo{.type  = gpu::DescriptorType::UniformBuffer,
                                        .count = cfg.bindless_uniform_buffers_capacity,
                                        .is_variable_length = true}}
              )
    })
        .unwrap();

    auto read_storage_buffers_label = tag("Read Storage Buffers"_s);
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

    auto read_write_storage_buffers_label = tag("Read/Write Storage Buffers"_s);
    auto read_write_storage_buffers =
      device
        ->create_descriptor_set_layout(gpu::DescriptorSetLayoutInfo{
          .label    = read_write_storage_buffers_label,
          .bindings = span({gpu::DescriptorBindingInfo{
            .type               = gpu::DescriptorType::RWStorageBuffer,
            .count              = cfg.bindless_read_write_storage_buffers_capacity,
            .is_variable_length = true}}
            )
    })
        .unwrap();

    auto input_attachments_label = tag("Input Attachments"_s);
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
      .samplers                       = samplers,
      .samplers_capacity              = cfg.bindless_samplers_capacity,
      .sampled_textures               = sampled_textures,
      .sampled_textures_capacity      = cfg.bindless_sampled_textures_capacity,
      .storage_textures               = storage_textures,
      .storage_textures_capacity      = cfg.bindless_storage_textures_capacity,
      .uniform_texel_buffers          = uniform_texel_buffers,
      .uniform_texel_buffers_capacity = cfg.bindless_uniform_texel_buffers_capacity,
      .storage_texel_buffers          = storage_texel_buffers,
      .storage_texel_buffers_capacity = cfg.bindless_storage_texel_buffers_capacity,
      .uniform_buffer                 = uniform_buffer,
      .read_storage_buffer            = read_storage_buffer,
      .read_write_storage_buffer      = read_write_storage_buffer,
      .uniform_buffers                = uniform_buffers,
      .uniform_buffer_capacity        = cfg.bindless_uniform_buffers_capacity,
      .read_storage_buffers           = read_storage_buffers,
      .read_storage_buffers_capacity  = cfg.bindless_read_storage_buffers_capacity,
      .read_write_storage_buffers     = read_write_storage_buffers,
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

GpuDescriptors GpuDescriptors::create(GpuSys sys, Str label)
{
    ScratchScope scratch{sys->allocator_};
    auto         tag = [&](Str component) {
        return sformat(scratch, "{} / {}"_s, label, component).unwrap();
    };

    auto samplers_label = tag("Samplers"_s);
    auto samplers =
      sys->dev_
        ->create_descriptor_set(gpu::DescriptorSetInfo{
          .label            = samplers_label,
          .layout           = sys->descriptors_layout_.samplers,
          .variable_lengths = span({sys->descriptors_layout_.samplers_capacity})})
        .unwrap();

    auto sampled_textures_label = tag("Sampled Textures"_s);
    auto sampled_textures =
      sys->dev_
        ->create_descriptor_set(gpu::DescriptorSetInfo{
          .label  = sampled_textures_label,
          .layout = sys->descriptors_layout_.sampled_textures,
          .variable_lengths =
            span({sys->descriptors_layout_.sampled_textures_capacity})})
        .unwrap();

    BitVec<u64> samplers_slots{sys->allocator_};
    samplers_slots.resize(sys->descriptors_layout_.samplers_capacity).unwrap();

    BitVec<u64> sampled_textures_slots{sys->allocator_};
    sampled_textures_slots.resize(sys->descriptors_layout_.sampled_textures_capacity)
      .unwrap();

    return GpuDescriptors{.samplers = samplers,
                          .samplers_slots{std::move(samplers_slots)},
                          .sampled_textures = sampled_textures,
                          .sampled_textures_slots{std::move(sampled_textures_slots)}};
}

void IGpuFramePlan::uninit()
{
}

void IGpuFramePlan::set_target(GpuFrameTargetInfo target)
{
    ASH_CHECK(state_ == GpuFramePlanState::Recording, "");
    target_ = target;
}

void IGpuFramePlan::reserve_scratch_buffers(Span<u64 const> sizes)
{
    ASH_CHECK(state_ == GpuFramePlanState::Recording, "");
    scratch_buffer_sizes_.resize(max(scratch_buffer_sizes_.size(), sizes.size()))
      .unwrap();

    for (auto [size, target] : zip(scratch_buffer_sizes_, sizes))
    {
        size = max(size, target);
    }
}

void IGpuFramePlan::reserve_scratch_images(u32 num_scratch_images)
{
    ASH_CHECK(state_ == GpuFramePlanState::Recording, "");
    target_.v().num_scratch_images =
      max(target_.v().num_scratch_images, num_scratch_images);
}

void IGpuFramePlan::add_preframe_task(GpuFrameTask && task)
{
    ASH_CHECK(state_ == GpuFramePlanState::Recording, "");
    pre_frame_tasks_.push(std::move(task)).unwrap();
}

void IGpuFramePlan::add_postframe_task(GpuFrameTask && task)
{
    ASH_CHECK(state_ == GpuFramePlanState::Recording, "");
    post_frame_tasks_.push(std::move(task)).unwrap();
}

void IGpuFramePlan::add_pass(GpuPass && pass)
{
    ASH_CHECK(state_ == GpuFramePlanState::Recording, "");
    passes_.push(std::move(pass)).unwrap();
}

CpuBufferId IGpuFramePlan::push_cpu(Span<u8 const> data)
{
    ASH_CHECK(state_ == GpuFramePlanState::Recording, "");
    auto offset = cpu_buffer_data_.size();
    auto size   = data.size();
    cpu_buffer_data_.append(data).unwrap();
    ASH_CHECK(cpu_buffer_data_.size() <= U32_MAX, "");
    auto aligned_size = align_offset_up<usize>(SIMD_ALIGNMENT, cpu_buffer_data_.size());
    cpu_buffer_data_.resize_uninit(aligned_size).unwrap();
    auto idx = cpu_buffer_entries_.size();
    cpu_buffer_entries_.push(offset, size).unwrap();
    return CpuBufferId{(u32) idx};
}

GpuBufferId IGpuFramePlan::push_gpu(Span<u8 const> data)
{
    ASH_CHECK(state_ == GpuFramePlanState::Recording, "");
    auto offset = gpu_buffer_data_.size();
    auto size   = data.size();
    gpu_buffer_data_.append(data).unwrap();
    ASH_CHECK(gpu_buffer_data_.size() <= U32_MAX, "");
    auto aligned_size =
      align_offset_up<usize>(gpu::BUFFER_OFFSET_ALIGNMENT, gpu_buffer_data_.size());
    gpu_buffer_data_.resize_uninit(aligned_size).unwrap();
    auto idx = gpu_buffer_entries_.size();
    gpu_buffer_entries_.push(offset, size).unwrap();
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
    ASH_CHECK(state_ == GpuFramePlanState::Reset, "");
    state_ = GpuFramePlanState::Recording;
}

void IGpuFramePlan::end()
{
    ASH_CHECK(state_ == GpuFramePlanState::Recording, "");
    state_ = GpuFramePlanState::Recorded;
}

void IGpuFramePlan::reset()
{
    ASH_CHECK(state_ != GpuFramePlanState::Submitted, "");
    // these buffers are expected to be very large so we reset them on every frame
    // when they aren't being used Target at least 75% utilization
    pre_frame_tasks_.reset();
    post_frame_tasks_.reset();
    frame_completed_tasks_.reset();
    gpu_buffer_data_.reset();
    gpu_buffer_entries_.reset();
    cpu_buffer_data_.reset();
    cpu_buffer_entries_.reset();
    scratch_buffer_sizes_.reset();
    passes_.reset();
    target_ = none;
    arena_.reclaim();
    state_ = GpuFramePlanState::Reset;
}

constexpr u32 IN_USE     = 1U;
constexpr u32 NOT_IN_USE = 0U;

void IGpuFramePlan::await()
{
    if (wait_token_->load(std::memory_order_acquire) != IN_USE)
    {
        return;
    }
    wait_token_->os_wait(IN_USE, std::memory_order_acquire);
}

TexelBufferUnion::View TexelBufferUnion::interpret(gpu::Format format) const
{
    auto view = find(views.view(), format,
                     [&](auto & view, auto format) { return view.format == format; });

    ASH_CHECK(!view.is_empty(), "");

    return view[0];
}

void TexelBufferUnion::uninit(gpu::Device d)
{
    for (auto & view : views)
    {
        d->uninit(view.uniform_texel_buffers);
        d->uninit(view.storage_texel_buffers);
        d->uninit(view.view);
    }
    d->uninit(buffer);
}

void TexelBufferUnion::init_views(GpuSys sys)
{
    ScratchScope scratch{sys->allocator_};

    auto & d = *sys->dev_;

    auto itag = [&](Str component, u32 i) {
        return sformat(scratch, "{} / {} / {}"_s, label, component, i).unwrap();
    };

    decltype(TexelBufferUnion::views) views{};
    views.resize_uninit(size32(formats)).unwrap();

    for (auto [i, format, view] : enumerate(formats, views))
    {
        auto buffer_view_tag = itag("View"_s, i);
        auto buffer_view =
          d.create_buffer_view(gpu::BufferViewInfo{.label  = buffer_view_tag,
                                                   .buffer = buffer,
                                                   .format = format,
                                                   .slice  = Slice64::all()})
            .unwrap();

        auto uniform_tag =
          sformat(scratch, "{} / {}"_s, buffer_view_tag, "Uniform"_s).unwrap();

        auto uniform_texel_buffers =
          d
            .create_descriptor_set(gpu::DescriptorSetInfo{
              .label            = uniform_tag,
              .layout           = sys->descriptors_layout().uniform_texel_buffers,
              .variable_lengths = span({1U})})
            .unwrap();

        auto storage_tag =
          sformat(scratch, "{} / {}"_s, buffer_view_tag, "Storage"_s).unwrap();

        auto storage_texel_buffers =
          d
            .create_descriptor_set(gpu::DescriptorSetInfo{
              .label            = storage_tag,
              .layout           = sys->descriptors_layout().storage_texel_buffers,
              .variable_lengths = span({1U})})
            .unwrap();

        view = View{.view                  = buffer_view,
                    .format                = format,
                    .uniform_texel_buffers = uniform_texel_buffers,
                    .storage_texel_buffers = storage_texel_buffers};
    }

    for (auto [i, view] : enumerate(views))
    {
        d.update_descriptor_set(
          gpu::DescriptorSetUpdate{.set           = view.uniform_texel_buffers,
                                   .binding       = 0,
                                   .first_element = 0,
                                   .images        = {},
                                   .texel_buffers = span({view.view}),
                                   .buffers       = {}});

        d.update_descriptor_set(
          gpu::DescriptorSetUpdate{.set           = view.storage_texel_buffers,
                                   .binding       = 0,
                                   .first_element = 0,
                                   .images        = {},
                                   .texel_buffers = span({view.view}),
                                   .buffers       = {}});
    }

    this->views = std::move(views);
}

TexelBufferUnion TexelBufferUnion::create(GpuSys sys, u32x2 target_extent,
                                          u32 sample_count, u32x2 tile_texel_count,
                                          Span<gpu::Format const> in_formats,
                                          Str                     in_label)
{
    ScratchScope scratch{sys->allocator_};
    ASH_CHECK(!tile_texel_count.any_zero(), "");
    ASH_CHECK(is_pow2(tile_texel_count.x()) && is_pow2(tile_texel_count.y()), "");
    ASH_CHECK(sample_count == 1, "");

    for (auto format : in_formats)
    {
        ASH_CHECK(!find(span(ALL_FORMATS), format).is_empty(), "");
    }

    auto tag = [&](Str component) {
        return sformat(scratch, "{} / {}"_s, in_label, component).unwrap();
    };

    auto buffer_tag = tag("Texel Buffer"_s);

    auto tiled_size = u32x2{align_offset_up(tile_texel_count.x(), target_extent.x()),
                            align_offset_up(tile_texel_count.y(), target_extent.y())};

    auto tile_texel_count_log2 = log2(tile_texel_count);
    auto tile_count            = tiled_size >> tile_texel_count_log2;

    auto & d = *sys->dev_;

    auto buffer =
      d.create_buffer(gpu::BufferInfo{.label = buffer_tag,
                                      .size = sizeof(f32x4) * tiled_size.product<u64>(),
                                      .usage       = TexelBufferUnion::USAGE,
                                      .memory_type = gpu::MemoryType::Aliased,
                                      .host_mapped = false})
        .unwrap();

    auto label   = vec::copy(sys->allocator_, in_label).unwrap();
    auto formats = vec::copy(sys->allocator_, in_formats).unwrap();

    return TexelBufferUnion{.buffer           = buffer,
                            .tile_texel_count = tile_texel_count,
                            .tile_count       = tile_count,
                            .extent           = target_extent,
                            .sample_count     = sample_count,
                            .views            = {},
                            .label            = std::move(label),
                            .formats          = std::move(formats)};
}

void ImageUnion::uninit(gpu::Device device)
{
    color.uninit(device);
    depth_stencil.uninit(device);
    texel.uninit(device);
    device->uninit(alias);
}

void ImageUnion::init_views(GpuSys sys)
{
    ScratchScope scratch{sys->allocator_};

    auto & d = *sys->dev_;

    auto tag = [&](Str component) {
        return sformat(scratch, "{} / {}"_s, label, component).unwrap();
    };

    auto color_view_label = tag("Color Image View"_s);

    auto color_view_info = gpu::ImageViewInfo{
      .label        = color_view_label,
      .image        = color.image,
      .view_type    = gpu::ImageViewType::Type2D,
      .view_format  = color.info.format,
      .mapping      = {},
      .aspects      = gpu::ImageAspects::Color,
      .mip_levels   = {0, 1},
      .array_layers = {0, 1}
    };

    auto color_image_view = d.create_image_view(color_view_info).unwrap();

    color_view_info.label = {};

    auto color_sampled_texture_label = tag("Color Sampled Texture"_s);
    auto color_sampled_texture =
      d.create_descriptor_set(
         gpu::DescriptorSetInfo{.label  = color_sampled_texture_label,
                                .layout = sys->descriptors_layout_.sampled_textures,
                                .variable_lengths = span({1U})})
        .unwrap();

    auto color_storage_texture_label = tag("Color Storage Texture"_s);
    auto color_storage_texture =
      d.create_descriptor_set(
         gpu::DescriptorSetInfo{.label  = color_storage_texture_label,
                                .layout = sys->descriptors_layout_.storage_textures,
                                .variable_lengths = span({1U})})
        .unwrap();

    auto color_input_attachment_label = tag("Color Input Attachment"_s);
    auto color_input_attachment =
      d.create_descriptor_set(
         gpu::DescriptorSetInfo{.label  = color_input_attachment_label,
                                .layout = sys->descriptors_layout_.input_attachments,
                                .variable_lengths = span({1U})})
        .unwrap();

    color.view_info         = color_view_info;
    color.view              = color_image_view;
    color.sampled_textures  = color_sampled_texture;
    color.storage_textures  = color_storage_texture;
    color.input_attachments = color_input_attachment;

    auto depth_stencil_label = tag("Depth Stencil Image"_s);

    auto depth_view_label = tag("Depth Image View"_s);
    auto depth_view_info  = gpu::ImageViewInfo{
      .label        = depth_view_label,
      .image        = depth_stencil.image,
      .view_type    = gpu::ImageViewType::Type2D,
      .view_format  = depth_stencil.info.format,
      .mapping      = {},
      .aspects      = gpu::ImageAspects::Depth,
      .mip_levels   = {0, 1},
      .array_layers = {0, 1}
    };

    auto depth_image_view = d.create_image_view(depth_view_info).unwrap();

    depth_view_info.label = {};

    auto stencil_view_label = tag("Stencil Image View"_s);
    auto stencil_view_info  = gpu::ImageViewInfo{
      .label        = stencil_view_label,
      .image        = depth_stencil.image,
      .view_type    = gpu::ImageViewType::Type2D,
      .view_format  = depth_stencil.info.format,
      .mapping      = {},
      .aspects      = gpu::ImageAspects::Stencil,
      .mip_levels   = {0, 1},
      .array_layers = {0, 1}
    };

    auto stencil_image_view = d.create_image_view(stencil_view_info).unwrap();

    stencil_view_info.label = {};

    auto depth_sampled_texture_label = tag("Depth Sampled Texture"_s);
    auto depth_sampled_texture =
      d.create_descriptor_set(
         gpu::DescriptorSetInfo{.label  = depth_sampled_texture_label,
                                .layout = sys->descriptors_layout_.sampled_textures,
                                .variable_lengths = span({1U})})
        .unwrap();

    auto depth_input_attachment_label = tag("Depth Input Attachment"_s);
    auto depth_input_attachment =
      d.create_descriptor_set(
         gpu::DescriptorSetInfo{.label  = depth_input_attachment_label,
                                .layout = sys->descriptors_layout_.input_attachments,
                                .variable_lengths = span({1U})})
        .unwrap();

    depth_stencil.depth_view              = depth_image_view;
    depth_stencil.depth_view_info         = depth_view_info;
    depth_stencil.stencil_view            = stencil_image_view;
    depth_stencil.stencil_view_info       = stencil_view_info;
    depth_stencil.depth_sampled_textures  = depth_sampled_texture;
    depth_stencil.depth_input_attachments = depth_input_attachment;

    Tuple<gpu::DescriptorSet, gpu::ImageView> bindings[] = {
      {color.sampled_textures,                color.view              },
      {color.storage_textures,                color.view              },
      {color.input_attachments,               color.view              },
      {depth_stencil.depth_sampled_textures,  depth_stencil.depth_view},
      {depth_stencil.depth_input_attachments, depth_stencil.depth_view}
    };

    for (auto [set, view] : bindings)
    {
        d.update_descriptor_set(gpu::DescriptorSetUpdate{
          .set           = set,
          .binding       = 0,
          .first_element = 0,
          .images        = span({gpu::ImageBinding{.image_view = view}}),
          .texel_buffers = {},
          .buffers       = {}});
    }

    texel.init_views(sys);
}

ImageUnion ImageUnion::create(GpuSys sys, u32x2 target_extent, gpu::Format color_format,
                              gpu::Format depth_stencil_format, Str in_label)
{
    ScratchScope scratch{sys->allocator_};
    // TODO: MSAA scratch and target textures
    auto         tag = [&](Str component) {
        return sformat(scratch, "{} / {}"_s, in_label, component).unwrap();
    };

    auto color_label = tag("Color Image"_s);

    auto color_info = gpu::ImageInfo{.label        = color_label,
                                     .type         = gpu::ImageType::Type2D,
                                     .format       = color_format,
                                     .usage        = ColorImage::USAGE,
                                     .aspects      = gpu::ImageAspects::Color,
                                     .extent       = target_extent.append(1),
                                     .mip_levels   = 1,
                                     .array_layers = 1,
                                     .sample_count = gpu::SampleCount::C1,
                                     .memory_type  = gpu::MemoryType::Aliased};

    auto & d           = *sys->dev_;
    auto   color_image = d.create_image(color_info).unwrap();

    color_info.label = {};

    auto color = ColorImage{.info              = color_info,
                            .view_info         = {},
                            .image             = color_image,
                            .view              = nullptr,
                            .sampled_textures  = nullptr,
                            .storage_textures  = nullptr,
                            .input_attachments = nullptr};

    auto depth_stencil_label = tag("Depth Stencil Image"_s);
    auto depth_stencil_info =
      gpu::ImageInfo{.label   = depth_stencil_label,
                     .type    = gpu::ImageType::Type2D,
                     .format  = depth_stencil_format,
                     .usage   = DepthStencilImage::USAGE,
                     .aspects = gpu::ImageAspects::Depth | gpu::ImageAspects::Stencil,
                     .extent  = target_extent.append(1),
                     .mip_levels   = 1,
                     .array_layers = 1,
                     .sample_count = gpu::SampleCount::C1,
                     .memory_type  = gpu::MemoryType::Aliased};

    auto depth_stencil_image = d.create_image(depth_stencil_info).unwrap();

    depth_stencil_info.label = {};

    auto depth_stencil = DepthStencilImage{.info              = depth_stencil_info,
                                           .depth_view_info   = {},
                                           .stencil_view_info = {},
                                           .image             = depth_stencil_image,
                                           .depth_view        = nullptr,
                                           .stencil_view      = nullptr,
                                           .depth_sampled_textures  = nullptr,
                                           .depth_input_attachments = nullptr};

    // only enable support for the 32-bit formats to minimize memory waste.
    // enabling support for f32x4 formats for example would consume 4x the memory
    // in the case where we only use RGBA8 formats, which is the most common case.
    static constexpr gpu::Format FORMATS[] = {
      gpu::Format::R8_UNORM,       gpu::Format::R8_SNORM,
      gpu::Format::R8_UINT,        gpu::Format::R8_SINT,
      gpu::Format::R8G8B8A8_UNORM, gpu::Format::R8G8B8A8_SNORM,
      gpu::Format::R8G8B8A8_UINT,  gpu::Format::R8G8B8A8_SINT,
      gpu::Format::R16_UINT,       gpu::Format::R16_SINT,
      gpu::Format::R16_SFLOAT,     gpu::Format::R16G16_UINT,
      gpu::Format::R16G16_SINT,    gpu::Format::R16G16_SFLOAT,
      gpu::Format::R32_UINT,       gpu::Format::R32_SINT,
      gpu::Format::R32_SFLOAT};

    static constexpr u32x2 TILE_TEXEL_COUNT = u32x2{32, 32};

    auto texel_union = TexelBufferUnion::create(sys, target_extent, 1, TILE_TEXEL_COUNT,
                                                FORMATS, in_label);

    auto alias_label = tag("Alias"_s);
    auto alias =
      d.create_alias(
         gpu::AliasInfo{.label     = alias_label,
                        .resources = span<Enum<gpu::Buffer, gpu::Image>>(
                          {color_image, depth_stencil_image, texel_union.buffer})})
        .unwrap();

    auto res = ImageUnion{.color         = color,
                          .depth_stencil = depth_stencil,
                          .texel         = std::move(texel_union),
                          .alias         = alias};

    res.init_views(sys);

    return res;
}

void ScratchImages::uninit(gpu::Device device)
{
    for (auto & scratch : images)
    {
        scratch.uninit(device);
    }
    images.clear();
}

ScratchImages ScratchImages::create(GpuSys sys, u32 num_scratch, u32x2 target_extent,
                                    gpu::Format color_format,
                                    gpu::Format depth_stencil_format, Str label,
                                    Allocator allocator)
{
    ScratchScope    scratch{sys->allocator_};
    Vec<ImageUnion> images{allocator};

    for (auto i : range(num_scratch))
    {
        auto union_label = sformat(scratch, "{} / {}"_s, label, i).unwrap();
        auto union_image = ImageUnion::create(sys, target_extent, color_format,
                                              depth_stencil_format, union_label);
        images.push(std::move(union_image)).unwrap();
    }

    return ScratchImages{.images = std::move(images)};
}

void ScratchBuffers::uninit(gpu::Device device)
{
    for (auto buffer : buffers)
    {
        buffer.uninit(device);
    }
}

ScratchBuffers ScratchBuffers::create(GpuSys sys, Span<u64 const> sizes, Str label,
                                      Allocator allocator)
{
    ScratchScope   scratch{sys->allocator_};
    Vec<GpuBuffer> buffers{allocator};
    for (auto [i, size] : enumerate(sizes))
    {
        auto tag    = sformat(scratch, "{} / Buffer {}"_s, label, i).unwrap();
        auto buffer = GpuBuffer::create(sys, size, GpuBuffer::USAGE, tag);
        buffers.push(buffer).unwrap();
    }
    return ScratchBuffers{.buffers = std::move(buffers)};
}

void GpuFrameResources::uninit(gpu::Device device)
{
    buffer.uninit(device);
    scratch_buffers.uninit(device);
    scratch_images.uninit(device);
    queries.uninit(device);
}

static void grow_buffer(GpuSys sys, Str label, GpuBuffer & buffer, u64 next_capacity)
{
    if (buffer.capacity < next_capacity)
    {
        buffer.uninit(sys->dev_);
        buffer = GpuBuffer::create(sys, next_capacity, buffer.usage, label);
    }
    else if (buffer.capacity > HalfGrowth::grow(next_capacity))
    {
        // Target at least 75% utilization
        buffer.uninit(sys->dev_);
        buffer = GpuBuffer::create(sys, next_capacity, buffer.usage, label);
    }
}

// TODO: ok to recreate every frame
void ScratchBuffers::grow(GpuSys sys, Span<u64 const> sizes, Str label,
                          Allocator allocator)
{
    if (buffers.size() != sizes.size())
    {
        uninit(sys->dev_);
        *this = create(sys, sizes, label, allocator);
        return;
    }

    for (auto [buffer, size] : zip(buffers, sizes))
    {
        grow_buffer(sys, label, buffer, size);
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
    ASH_CHECK(state_ == GpuFrameState::Recording, "");
    if (next_timestamp_ >= resources_.queries.cpu_timestamps.size())
    {
        return none;
    }

    auto idx = next_timestamp_++;

    return Tuple{resources_.queries.timestamps, idx};
}

Option<Tuple<gpu::StatisticsQuery, u32>> IGpuFrame::allocate_statistics()
{
    ASH_CHECK(state_ == GpuFrameState::Recording, "");
    if (next_statistics_ >= resources_.queries.cpu_statistics.size())
    {
        return none;
    }

    auto idx = next_statistics_++;

    return Tuple{resources_.queries.statistics, idx};
}

Span<ImageUnion const> IGpuFrame::get_scratch_images() const
{
    ASH_CHECK(state_ == GpuFrameState::Recording || state_ == GpuFrameState::Recorded,
              "");
    return resources_.scratch_images.images;
}

Span<GpuBuffer const> IGpuFrame::get_scratch_buffers() const
{
    ASH_CHECK(state_ == GpuFrameState::Recording || state_ == GpuFrameState::Recorded,
              "");
    return resources_.scratch_buffers.buffers;
}

GpuBufferSpan IGpuFrame::get(GpuBufferId id)
{
    ASH_CHECK(state_ == GpuFrameState::Recording || state_ == GpuFrameState::Recorded,
              "");
    Slice64 slice = current_plan_->gpu_buffer_entries_.get((usize) id);
    return GpuBufferSpan{.buffer = resources_.buffer, .slice = slice};
}

Span<u8> IGpuFrame::get(CpuBufferId id)
{
    ASH_CHECK(state_ == GpuFrameState::Recording || state_ == GpuFrameState::Recorded,
              "");
    ASH_CHECK(current_plan_ != nullptr, "");
    auto slice = current_plan_->cpu_buffer_entries_.get((usize) id);
    return current_plan_->cpu_buffer_data_.view().slice(slice);
}

gpu::DescriptorSet IGpuFrame::get(TextureSet tex)
{
    return tex.match(
      [&](ScratchTexture s) {
          auto & img = get_scratch_images()[s.image];

          switch (s.type)
          {
              case ash::ScratchTexureType::SampledColor:
                  return img.color.sampled_textures;
              case ash::ScratchTexureType::StorageColor:
                  return img.color.storage_textures;
              case ash::ScratchTexureType::InputAttachmentColor:
                  return img.color.input_attachments;
              case ash::ScratchTexureType::SampledDepthStencil:
                  return img.depth_stencil.depth_sampled_textures;
              case ash::ScratchTexureType::InputAttachmentDepthStencil:
                  return img.depth_stencil.depth_input_attachments;
              default:
                  ASH_UNREACHABLE;
          }
      },
      [&](SampledTextures) { return sys_->sampled_textures(); });
}

void IGpuFrame::begin()
{
    ASH_CHECK(state_ == GpuFrameState::Reset, "");
    state_ = GpuFrameState::Recording;
}

void IGpuFrame::set_plan(GpuFramePlan plan)
{
    ASH_CHECK(state_ == GpuFrameState::Recording, "");
    ASH_CHECK(plan != nullptr, "");
    ASH_CHECK(plan->state_ == GpuFramePlanState::Recorded, "");
    current_plan_                         = plan;
    current_plan_->state_                 = GpuFramePlanState::Submitted;
    current_plan_->frame_completed_tasks_ = std::move(plan->frame_completed_tasks_);
}

void IGpuFrame::end()
{
    ASH_CHECK(state_ == GpuFrameState::Recording, "");
    ASH_CHECK(current_plan_ != nullptr, "");
    state_ = GpuFrameState::Recorded;
}

void IGpuFrame::submit()
{
    tracing::ScopeTrace trace;
    ASH_CHECK(state_ == GpuFrameState::Recorded, "");

    {
        ScratchScope scratch{allocator_};
        auto         label = sformat(scratch, "GpuFrame {} / Buffer"_s, id_).unwrap();
        ASH_CHECK(current_plan_->gpu_buffer_data_.size() <= cfg_.max_buffer_size, "");
        auto size = clamp(current_plan_->gpu_buffer_data_.size(), cfg_.min_buffer_size,
                          cfg_.max_buffer_size);
        grow_buffer(sys_, label, resources_.buffer, size);
        mem::copy(current_plan_->gpu_buffer_data_.view(),
                  dev_->get_memory_map(resources_.buffer.buffer).unwrap());
    }

    {
        ScratchScope scratch{allocator_};

        auto label = sformat(scratch, "GpuFrame {} / Scratch Buffers"_s, id_).unwrap();
        for (auto s : current_plan_->scratch_buffer_sizes_)
        {
            ASH_CHECK(s <= cfg_.max_scratch_buffer_size, "");
        }

        ASH_CHECK(
          current_plan_->scratch_buffer_sizes_.size() <= cfg_.max_scratch_buffers, "");

        Vec<u64> sizes{scratch};

        for (auto s : current_plan_->scratch_buffer_sizes_)
        {
            sizes
              .push(
                clamp(s, cfg_.min_scratch_buffer_size, cfg_.max_scratch_buffer_size))
              .unwrap();
        }

        resources_.scratch_buffers.grow(sys_, sizes, label, allocator_);
    }

    current_plan_->target_.match([&](auto & target) {
        ASH_CHECK(target.num_scratch_images <= cfg_.max_scratch_images, "");
        ASH_CHECK(target.color_format != gpu::Format::Undefined, "");
        ASH_CHECK(target.depth_stencil_format != gpu::Format::Undefined, "");
        ASH_CHECK(!target.extent.any_zero(), "");
    });

    if (target_info_ != current_plan_->target_)
    {
        ScratchScope scratch{allocator_};
        resources_.scratch_images.uninit(dev_);

        current_plan_->target_.match([&](auto & target) {
            auto num_scratch_images =
              clamp(target.num_scratch_images, cfg_.min_scratch_images,
                    cfg_.max_scratch_images);
            auto label =
              sformat(scratch, "GpuFrame {} / Scratch Images"_s, id_).unwrap();
            resources_.scratch_images = ScratchImages::create(
              sys_, num_scratch_images, target.extent, target.color_format,
              target.depth_stencil_format, label, allocator_);
        });
    }

    target_info_ = current_plan_->target_;

    if (sys_->cfg_.frame_timestamps_capacity !=
          resources_.queries.timestamps_capacity() ||
        sys_->cfg_.frame_statistics_capacity !=
          resources_.queries.statistics_capacity())
    {
        ScratchScope scratch{allocator_};
        auto         label = sformat(scratch, "GpuFrame {} / Queries"_s, id_).unwrap();
        resources_.queries.uninit(dev_);
        resources_.queries = GpuQueries::create(allocator_, dev_, label,
                                                sys_->cfg_.frame_timestamps_capacity,
                                                sys_->cfg_.frame_statistics_capacity);
    }

    for (auto & task : current_plan_->pre_frame_tasks_)
    {
        task(this);
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
        pass(this, this->command_encoder());
    }

    command_encoder_->end().unwrap();

    command_buffer_->begin();
    command_buffer_->record(command_encoder_);
    command_buffer_->end().unwrap();

    scope_frame_id_ = dev_->submit(command_buffer_, sys_->queue_scope_).unwrap();

    for (auto & task : current_plan_->post_frame_tasks_)
    {
        task(this);
    }

    current_plan_->state_ = GpuFramePlanState::Executed;
    current_plan_         = nullptr;
    state_                = GpuFrameState::Submitted;
}

void IGpuFrame::complete()
{
    tracing::ScopeTrace trace;
    ASH_CHECK(state_ == GpuFrameState::Submitted, "");

    dev_
      ->await_queue_scope_frame(sys_->queue_scope_, scope_frame_id_, nanoseconds::max())
      .unwrap();

    dev_
      ->get_timestamp_query_result(
        resources_.queries.timestamps, 0,
        resources_.queries.cpu_timestamps.view().slice(0, next_timestamp_))
      .unwrap();
    dev_
      ->get_statistics_query_result(
        resources_.queries.statistics, 0,
        resources_.queries.cpu_statistics.view().slice(0, next_statistics_))
      .unwrap();

    for (auto & task : frame_completed_tasks_)
    {
        task(this);
    }

    state_ = GpuFrameState::Completed;
    wait_token_->store(NOT_IN_USE, std::memory_order_release);
    wait_token_->os_notify();
    frame_completed_tasks_ = Vec<GpuFrameTask>{noop_allocator};
}

void IGpuFrame::reset()
{
    ASH_CHECK(state_ != GpuFrameState::Submitted, "");
    next_statistics_ = 0;
    command_encoder_->reset();
    command_buffer_->reset();
    current_plan_          = nullptr;
    frame_completed_tasks_ = Vec<GpuFrameTask>{noop_allocator};
    state_                 = GpuFrameState::Reset;
}

void IGpuFrame::await()
{
    if (wait_token_->load(std::memory_order_acquire) != IN_USE)
    {
        return;
    }
    wait_token_->os_wait(IN_USE, std::memory_order_acquire);
}

static Option<gpu::Format> select_color_format(gpu::Device             dev,
                                               Span<gpu::Format const> formats)
{
    for (auto fmt : formats)
    {
        gpu::FormatProperties props = dev->get_format_properties(fmt).unwrap();
        if (has_bits(props.optimal_tiling_features, ColorImage::FEATURES))
        {
            return fmt;
        }
    }

    return none;
}

static Option<gpu::Format> select_depth_stencil_format(gpu::Device             dev,
                                                       Span<gpu::Format const> formats)
{
    for (auto fmt : formats)
    {
        gpu::FormatProperties props = dev->get_format_properties(fmt).unwrap();
        if (has_bits(props.optimal_tiling_features, DepthStencilImage::FEATURES))
        {
            return fmt;
        }
    }

    return none;
}

void IGpuSys::shutdown(Vec<u8> & cache)
{
    scheduler_->await_thread_shutdown(thread_.v());
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

    descriptors_layout_.uninit(dev_);
    for (auto view : default_image_views_)
    {
        dev_->uninit(view);
    }
    dev_->uninit(default_image_);
    dev_->uninit(pipeline_cache_);
}

static void create_default_samplers(GpuSys sys)
{
    constexpr Tuple<Str, gpu::BorderColor> colors[] = {
      {"FloatTransparentBlack"_s, gpu::BorderColor::FloatTransparentBlack},
      {"IntTransparentBlack"_s,   gpu::BorderColor::IntTransparentBlack  },
      {"FloatOpaqueBlack"_s,      gpu::BorderColor::FloatOpaqueBlack     },
      {"IntOpaqueBlack"_s,        gpu::BorderColor::IntOpaqueBlack       },
      {"FloatOpaqueueWhite"_s,    gpu::BorderColor::FloatOpaqueueWhite   },
      {"IntOpaqueueWhite"_s,      gpu::BorderColor::IntOpaqueueWhite     }
    };

    constexpr Tuple<Str, gpu::SamplerAddressMode> address_modes[] = {
      {"Repeat"_s,            gpu::SamplerAddressMode::Repeat           },
      {"MirroredRepeat"_s,    gpu::SamplerAddressMode::MirroredRepeat   },
      {"ClampToEdge"_s,       gpu::SamplerAddressMode::ClampToEdge      },
      {"ClampToBorder"_s,     gpu::SamplerAddressMode::ClampToBorder    },
      {"MirrorClampToEdge"_s, gpu::SamplerAddressMode::MirrorClampToEdge}
    };

    constexpr Tuple<Str, gpu::Filter, gpu::SamplerMipMapMode> mip_map_modes[] = {
      {"Linear"_s,  gpu::Filter::Linear,  gpu::SamplerMipMapMode::Linear },
      {"Nearest"_s, gpu::Filter::Nearest, gpu::SamplerMipMapMode::Nearest}
    };

    for (auto [mip_map_mode_name, filter, mip_map_mode] : mip_map_modes)
    {
        for (auto [address_mode_name, adress_mode] : address_modes)
        {
            for (auto [color_name, color] : colors)
            {
                ScratchScope scratch{sys->allocator_};

                auto label = sformat(scratch, "Sampler: {} + {} + {}"_s,
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

static void create_default_textures(GpuSys sys)
{
    gpu::Image default_image =
      sys->dev_
        ->create_image(gpu::ImageInfo{
          .label        = "Default Image"_s,
          .type         = gpu::ImageType::Type2D,
          .format       = gpu::Format::B8G8R8A8_UNORM,
          .usage        = gpu::ImageUsage::Sampled | gpu::ImageUsage::TransferDst |
                          gpu::ImageUsage::Storage | gpu::ImageUsage::Storage,
          .aspects      = gpu::ImageAspects::Color,
          .extent       = {1, 1, 1},
          .mip_levels   = 1,
          .array_layers = 1,
          .sample_count = gpu::SampleCount::C1,
          .memory_type  = gpu::MemoryType::Unique
    })
        .unwrap();

    static constexpr Array<Tuple<Str, TextureIndex, gpu::ComponentMapping>,
                           NUM_DEFAULT_TEXTURES>
      mappings{
        {{"White Texture"_s,
          TextureIndex::White,
          {.r = gpu::ComponentSwizzle::One,
           .g = gpu::ComponentSwizzle::One,
           .b = gpu::ComponentSwizzle::One,
           .a = gpu::ComponentSwizzle::One}},
         {"Transparent Texture"_s,
          TextureIndex::Transparent,
          {.r = gpu::ComponentSwizzle::Zero,
           .g = gpu::ComponentSwizzle::Zero,
           .b = gpu::ComponentSwizzle::Zero,
           .a = gpu::ComponentSwizzle::Zero}},
         {"RedTransparent Texture"_s,
          TextureIndex::RedTransparent,
          {.r = gpu::ComponentSwizzle::One,
           .g = gpu::ComponentSwizzle::Zero,
           .b = gpu::ComponentSwizzle::Zero,
           .a = gpu::ComponentSwizzle::Zero}},
         {"GreenTransparent Texture"_s,
          TextureIndex::GreenTransparent,
          {.r = gpu::ComponentSwizzle::Zero,
           .g = gpu::ComponentSwizzle::One,
           .b = gpu::ComponentSwizzle::Zero,
           .a = gpu::ComponentSwizzle::Zero}},
         {"BlueTransparent Texture"_s,
          TextureIndex::BlueTransparent,
          {.r = gpu::ComponentSwizzle::Zero,
           .g = gpu::ComponentSwizzle::Zero,
           .b = gpu::ComponentSwizzle::One,
           .a = gpu::ComponentSwizzle::Zero}},
         {"YellowTransparent Texture"_s,
          TextureIndex::YellowTransparent,
          {.r = gpu::ComponentSwizzle::One,
           .g = gpu::ComponentSwizzle::One,
           .b = gpu::ComponentSwizzle::Zero,
           .a = gpu::ComponentSwizzle::Zero}},
         {"MagentaTransparent Texture"_s,
          TextureIndex::MagentaTransparent,
          {.r = gpu::ComponentSwizzle::One,
           .g = gpu::ComponentSwizzle::Zero,
           .b = gpu::ComponentSwizzle::One,
           .a = gpu::ComponentSwizzle::Zero}},
         {"CyanTransparent Texture"_s,
          TextureIndex::CyanTransparent,
          {.r = gpu::ComponentSwizzle::Zero,
           .g = gpu::ComponentSwizzle::One,
           .b = gpu::ComponentSwizzle::One,
           .a = gpu::ComponentSwizzle::Zero}},
         {"WhiteTransparent Texture"_s,
          TextureIndex::WhiteTransparent,
          {.r = gpu::ComponentSwizzle::One,
           .g = gpu::ComponentSwizzle::One,
           .b = gpu::ComponentSwizzle::One,
           .a = gpu::ComponentSwizzle::Zero}},
         {"Black Texture"_s,
          TextureIndex::Black,
          {.r = gpu::ComponentSwizzle::Zero,
           .g = gpu::ComponentSwizzle::Zero,
           .b = gpu::ComponentSwizzle::Zero,
           .a = gpu::ComponentSwizzle::One}},
         {"Red Texture"_s,
          TextureIndex::Red,
          {.r = gpu::ComponentSwizzle::One,
           .g = gpu::ComponentSwizzle::Zero,
           .b = gpu::ComponentSwizzle::Zero,
           .a = gpu::ComponentSwizzle::One}},
         {"Green Texture"_s,
          TextureIndex::Green,
          {.r = gpu::ComponentSwizzle::Zero,
           .g = gpu::ComponentSwizzle::One,
           .b = gpu::ComponentSwizzle::Zero,
           .a = gpu::ComponentSwizzle::One}},
         {"Blue Texture"_s,
          TextureIndex::Blue,
          {.r = gpu::ComponentSwizzle::Zero,
           .g = gpu::ComponentSwizzle::Zero,
           .b = gpu::ComponentSwizzle::One,
           .a = gpu::ComponentSwizzle::One}},
         {"Yellow Texture"_s,
          TextureIndex::Yellow,
          {.r = gpu::ComponentSwizzle::One,
           .g = gpu::ComponentSwizzle::One,
           .b = gpu::ComponentSwizzle::Zero,
           .a = gpu::ComponentSwizzle::One}},
         {"Magenta Texture"_s,
          TextureIndex::Magenta,
          {.r = gpu::ComponentSwizzle::One,
           .g = gpu::ComponentSwizzle::Zero,
           .b = gpu::ComponentSwizzle::One,
           .a = gpu::ComponentSwizzle::One}},
         {"Cyan Texture"_s,
          TextureIndex::Cyan,
          {.r = gpu::ComponentSwizzle::Zero,
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

        auto idx = sys->alloc_texture_index(view);
        ASH_CHECK(mapping.v1 == idx, "");
    }

    sys->default_image_       = default_image;
    sys->default_image_views_ = default_image_views;
}

void IGpuSys::init(Allocator allocator, gpu::Device device,
                   Span<u8 const>            pipeline_cache_data,
                   GpuSysPreferences const & preferences, Scheduler scheduler,
                   Thread thread)
{
    ASH_CHECK(preferences.buffering > 0, "");
    ASH_CHECK(preferences.buffering <= MAX_BUFFERING, "");

    allocator_ = allocator;
    dev_       = device;

    props_ = device->get_properties();
    pipeline_cache_ =
      dev_
        ->create_pipeline_cache(gpu::PipelineCacheInfo{
          .label = "PipelineCache"_s, .initial_data = pipeline_cache_data})
        .unwrap();
    buffering_ = preferences.buffering;

    color_format_ = select_color_format(dev_, preferences.color_formats)
                      .unwrap("Device doesn't support any preferred color format"_s);

    depth_stencil_format_ =
      select_depth_stencil_format(dev_, preferences.depth_stencil_formats)
        .unwrap("Device doesn't support any preferred depth-stencil formats"_s);

    sample_count_ = gpu::SampleCount::C1;

    trace("Selected color format: {}"_s, color_format_);

    trace("Selected depth stencil format: {}"_s, depth_stencil_format_);

    descriptors_layout_ =
      GpuDescriptorsLayout::create(allocator_, dev_, "DescriptorsLayout"_s, cfg_);

    queue_scope_ = dev_
                     ->create_queue_scope(gpu::QueueScopeInfo{
                       .label = "QueueScope"_s, .buffering = buffering_ * 4})
                     .unwrap();

    sampler_cache_ = SamplerCache{allocator_};
    descriptors_   = GpuDescriptors::create(this, "Descriptors"_s);

    auto frames = Vec<Dyn<GpuFrame>>::make(buffering_, allocator_).unwrap();

    for (auto i : range(buffering_))
    {
        ScratchScope scratch{allocator_};

        // start as signaled wait token
        auto wait_token = dyn<IWaitToken>(inplace, allocator_, NOT_IN_USE).unwrap();

        auto encoder_label =
          sformat(scratch, "GpuFrame {} / CommandEncoder"_s, i).unwrap();

        auto encoder =
          dev_->create_command_encoder(gpu::CommandEncoderInfo{.label = encoder_label})
            .unwrap();

        auto buffer_label =
          sformat(scratch, "GpuFrame {} / CommandBuffer"_s, i).unwrap();

        auto buffer =
          dev_->create_command_buffer(gpu::CommandBufferInfo{.label = buffer_label})
            .unwrap();

        auto frame = dyn<IGpuFrame>(inplace, allocator_, allocator_, dev_, this, i,
                                    std::move(wait_token), encoder, buffer)
                       .unwrap();

        frames.push(std::move(frame)).unwrap();
    }

    frames_ = std::move(frames);

    auto plans = Vec<Dyn<GpuFramePlan>>::make(buffering_, allocator_).unwrap();

    for (auto _ : range(buffering_))
    {
        auto wait_token = dyn<IWaitToken>(inplace, allocator_, NOT_IN_USE).unwrap();
        auto plan       = dyn<IGpuFramePlan>(inplace, allocator_, allocator_, this,
                                             std::move(wait_token))
                            .unwrap();
        plans.push(std::move(plan)).unwrap();
    }

    plans_                = std::move(plans);
    scheduler_            = scheduler;
    thread_               = thread;
    initialized_          = true;
    num_frames_in_flight_ = buffering_;

    auto * plan = current_plan();
    plan->begin();
    create_default_textures(this);
    create_default_samplers(this);
    plan->end();
    submit_frame();
}

SamplerIndex IGpuSys::create_cached_sampler(gpu::SamplerInfo const & info_)
{
    ASH_CHECK(initialized_, "");
    LockGuard guard{resources_lock_};

    auto info  = info_;
    info.label = {};

    auto found = sampler_cache_.try_get(info);

    if (found)
    {
        return found.v().v0;
    }

    auto index = descriptors_.samplers_slots.view().find_clear_bit();

    ASH_CHECK(index < descriptors_.samplers_slots.size(),
              "Ran out of sampler descriptor slots");

    descriptors_.samplers_slots.set_bit(index);

    auto sampler_index = static_cast<SamplerIndex>(index);

    auto sampler = dev_->create_sampler(info_).unwrap();

    sampler_cache_.push(info, Tuple{sampler_index, sampler}).unwrap();

    current_plan()->add_preframe_task(
      [device = this->dev_, samplers = descriptors_.samplers,
       index = static_cast<u32>(index), sampler](GpuFrame) {
          device->update_descriptor_set(gpu::DescriptorSetUpdate{
            .set           = samplers,
            .binding       = 0,
            .first_element = index,
            .images        = span({gpu::ImageBinding{.sampler = sampler}}),
            .texel_buffers = {},
            .buffers       = {}});
      });

    return sampler_index;
}

TextureIndex IGpuSys::alloc_texture_index(gpu::ImageView view)
{
    ASH_CHECK(initialized_, "");

    LockGuard guard{resources_lock_};

    auto index = descriptors_.sampled_textures_slots.view().find_clear_bit();

    ASH_CHECK(index < descriptors_.sampled_textures_slots.size(),
              "Ran out of sampled texture descriptor slots");

    descriptors_.sampled_textures_slots.set_bit(index);

    current_plan()->add_preframe_task(
      [device = this->dev_, textures = descriptors_.sampled_textures,
       index = static_cast<u32>(index), view](GpuFrame) {
          device->update_descriptor_set(gpu::DescriptorSetUpdate{
            .set           = textures,
            .binding       = 0,
            .first_element = index,
            .images        = span({gpu::ImageBinding{.image_view = view}}),
            .texel_buffers = {},
            .buffers       = {}});
      });

    return static_cast<TextureIndex>(index);
}

void IGpuSys::release_texture_index(TextureIndex index)
{
    ASH_CHECK(initialized_, "");

    LockGuard guard{resources_lock_};

    descriptors_.sampled_textures_slots.clear_bit((usize) index);

    current_plan()->add_preframe_task([device   = this->dev_,
                                       textures = descriptors_.sampled_textures,
                                       index    = static_cast<u32>(index)](GpuFrame) {
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
    ASH_CHECK(initialized_, "");
    return dev_;
}

Allocator IGpuSys::allocator() const
{
    return allocator_;
}

GpuFramePlan IGpuSys::current_plan()
{
    ASH_CHECK(initialized_, "");
    return plans_[frame_ring_index()].get();
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

GpuDescriptorsLayout const & IGpuSys::descriptors_layout() const
{
    return descriptors_layout_;
}

gpu::DescriptorSet IGpuSys::samplers() const
{
    return descriptors_.samplers;
}

gpu::DescriptorSet IGpuSys::sampled_textures() const
{
    return descriptors_.sampled_textures;
}

void IGpuSys::submit_frame()
{
    ASH_CHECK(initialized_, "");

    auto * frame = frames_[frame_ring_index()].get();
    auto * plan  = plans_[frame_ring_index()].get();

    plan->wait_token_->store(IN_USE, std::memory_order_release);
    plan->wait_token_->os_notify();

    // tasks are executed in submission order
    scheduler_->once(
      thread_.v(),
      [frame, plan] mutable {
          // wait on the frame to be available
          frame->await();
          // submit the frame
          frame->reset();
          frame->begin();
          frame->set_plan(plan);
          frame->end();
          frame->submit();
          plan->wait_token_->store(NOT_IN_USE, std::memory_order_release);
          plan->wait_token_->os_notify();
          frame->complete();
      },
      ready);

    frame_index_++;

    // wait on the next frame plan
    plans_[frame_ring_index()]->await();
}

u64 IGpuSys::frame_index() const
{
    return frame_index_;
}

u32 IGpuSys::frame_ring_index() const
{
    return static_cast<u32>(frame_index_ % (u64) num_frames_in_flight_);
}

}    // namespace ash
