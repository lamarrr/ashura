

 
void Device::uninit(gpu::CommandEncoder * enc_)
{
  // [ ] fix
  auto enc = (CommandEncoder *) enc_;
  if (enc == nullptr)
  {
    return;
  }
  enc->~CommandEncoder();
  allocator->ndealloc(1, enc);
  /*
  enc->render_ctx.commands.reset();
  vk_table.DestroyCommandPool(vk_dev, enc->vk_command_pool, nullptr);
  */
}
Status Device::init_command_encoder(CommandEncoder * enc)
{
  VkCommandPoolCreateInfo command_pool_create_info{
    .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
    .pNext            = nullptr,
    .flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
    .queueFamilyIndex = queue_family};

  VkCommandPool vk_command_pool;
  VkResult      result = vk_table.CreateCommandPool(
    vk_dev, &command_pool_create_info, nullptr, &vk_command_pool);

  if (result != VK_SUCCESS)
  {
    return (Status) result;
  }

  VkCommandBufferAllocateInfo allocate_info{
    .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    .pNext              = nullptr,
    .commandPool        = vk_command_pool,
    .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    .commandBufferCount = 1};

  VkCommandBuffer vk_command_buffer;
  result =
    vk_table.AllocateCommandBuffers(vk_dev, &allocate_info, &vk_command_buffer);

  if (result != VK_SUCCESS)
  {
    vk_table.DestroyCommandPool(vk_dev, vk_command_pool, nullptr);
    return (Status) result;
  }

  set_resource_name("Frame Command Buffer"_str, vk_command_buffer,
                    VK_OBJECT_TYPE_COMMAND_BUFFER,
                    VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT);

  new (enc) CommandEncoder{};

  enc->allocator         = allocator;
  enc->dev               = this;
  enc->arg_pool          = ArenaPool{allocator};
  enc->vk_command_pool   = vk_command_pool;
  enc->vk_command_buffer = vk_command_buffer;
  enc->status            = Status::Success;
  enc->state             = CommandEncoderState::Reset;
  enc->render_ctx        = RenderPassContext{
           .arg_pool     = ArenaPool{allocator},
           .command_pool = ArenaPool{allocator},
           .commands     = Vec<Command>{enc->render_ctx.command_pool.ref()}};
  enc->compute_ctx = {};

  return Status::Success;
}

Result<Void, Status> Device::begin_frame(gpu::Swapchain swapchain_)
{
  FrameContext &   ctx          = frame_ctx;
  auto * const     swapchain    = (Swapchain *) swapchain_;
  VkFence          submit_fence = ctx.submit_fences[ctx.ring_index];
  CommandEncoder & enc          = ctx.encoders[ctx.ring_index];

  CHECK(!enc.is_recording(), "");

  VkResult result =
    vk_table.WaitForFences(vk_dev, 1, &submit_fence, VK_TRUE, U64_MAX);

  CHECK(result == VK_SUCCESS, "");

  result = vk_table.ResetFences(vk_dev, 1, &submit_fence);

  CHECK(result == VK_SUCCESS, "");

  if (swapchain != nullptr)
  {
    if (swapchain->is_out_of_date || !swapchain->is_optimal ||
        swapchain->vk_swapchain == nullptr)
    {
      // await all pending submitted operations on the device possibly using
      // the swapchain, to avoid destroying whilst in use
      result = vk_table.DeviceWaitIdle(vk_dev);
      CHECK(result == VK_SUCCESS, "");

      result = recreate_swapchain(swapchain);
      CHECK(result == VK_SUCCESS, "");
    }

    if (!swapchain->is_zero_sized)
    {
      u32 next_image;
      result = vk_table.AcquireNextImageKHR(
        vk_dev, swapchain->vk_swapchain, U64_MAX,
        ctx.acquire_semaphores[ctx.ring_index], nullptr, &next_image);

      if (result == VK_SUBOPTIMAL_KHR)
      {
        swapchain->is_optimal = false;
      }
      else
      {
        CHECK(result == VK_SUCCESS, "");
      }

      swapchain->current_image = next_image;
    }
  }

  vk_table.ResetCommandBuffer(enc.vk_command_buffer, 0);

  enc.clear_context();

  VkCommandBufferBeginInfo info{
    .sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    .pNext            = nullptr,
    .flags            = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    .pInheritanceInfo = nullptr};
  result = vk_table.BeginCommandBuffer(enc.vk_command_buffer, &info);
  CHECK(result == VK_SUCCESS, "");

  ctx.swapchain = swapchain;

  return Ok{Void{}};
}

Result<Void, Status> Device::submit_frame(gpu::Swapchain swapchain_)
{
  FrameContext &        ctx            = frame_ctx;
  auto * const          swapchain      = (Swapchain *) swapchain_;
  VkFence const         submit_fence   = ctx.submit_fences[ctx.ring_index];
  CommandEncoder &      enc            = ctx.encoders[ctx.ring_index];
  VkCommandBuffer const command_buffer = enc.vk_command_buffer;
  VkSemaphore const submit_semaphore   = ctx.submit_semaphores[ctx.ring_index];
  VkSemaphore const acquire_semaphore  = ctx.acquire_semaphores[ctx.ring_index];
  bool const was_acquired = swapchain != nullptr && !swapchain->is_zero_sized;
  bool const can_present = swapchain != nullptr && !swapchain->is_out_of_date &&
                           !swapchain->is_zero_sized;

  CHECK(swapchain == ctx.swapchain, "");

  CHECK(enc.is_recording(), "");

  if (was_acquired)
  {
    // enc.access_image(swapchain->image_impls[swapchain->current_image],
    //                  VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_ACCESS_NONE,
    //                  VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
  }

  VkResult result = vk_table.EndCommandBuffer(command_buffer);
  CHECK(result == VK_SUCCESS, "");
  CHECK(enc.status == gpu::Status::Success, "");

  VkPipelineStageFlags const wait_stages = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

  VkSubmitInfo submit_info{
    .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .pNext                = nullptr,
    .waitSemaphoreCount   = was_acquired ? 1U : 0U,
    .pWaitSemaphores      = was_acquired ? &acquire_semaphore : nullptr,
    .pWaitDstStageMask    = was_acquired ? &wait_stages : nullptr,
    .commandBufferCount   = 1,
    .pCommandBuffers      = &command_buffer,
    .signalSemaphoreCount = can_present ? 1U : 0U,
    .pSignalSemaphores    = can_present ? &submit_semaphore : nullptr};

  result = vk_table.QueueSubmit(vk_queue, 1, &submit_info, submit_fence);

  enc.state = CommandEncoderState::End;

  CHECK(result == VK_SUCCESS, "");

  // - advance frame, even if invalidation occured. frame is marked as missed
  // but has no side effect on the flow. so no need for resubmitting as previous
  // commands could have been executed.
  ctx.current_frame++;
  ctx.tail_frame =
    max(ctx.current_frame, (gpu::FrameId) ctx.buffering()) - ctx.buffering();
  ctx.ring_index = (ctx.ring_index + 1) % ctx.buffering();

  if (can_present)
  {
    VkPresentInfoKHR present_info{.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                                  .pNext = nullptr,
                                  .waitSemaphoreCount = 1,
                                  .pWaitSemaphores    = &submit_semaphore,
                                  .swapchainCount     = 1,
                                  .pSwapchains   = &swapchain->vk_swapchain,
                                  .pImageIndices = &swapchain->current_image,
                                  .pResults      = nullptr};
    result = vk_table.QueuePresentKHR(vk_queue, &present_info);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
      swapchain->is_out_of_date = true;
    }
    else if (result == VK_SUBOPTIMAL_KHR)
    {
      swapchain->is_optimal = false;
    }
    else
    {
      CHECK(result == VK_SUCCESS, "");
    }
  }

  return Ok{Void{}};
}

#define ENCODE_PRELUDE()         \
  CHECK(is_recording(), "");     \
  if (status != Status::Success) \
  {                              \
    return;                      \
  }                              \
  defer pool_reclaim_            \
  {                              \
    [&] { arg_pool.reclaim(); }  \
  }

void CommandEncoder::reset_timestamp_query(gpu::TimestampQuery query_,
                                           Slice32             range)
{
  ENCODE_PRELUDE();
  VkQueryPool const vk_pool = (VkQueryPool) query_;
  CHECK(!is_in_pass(), "");

  dev->vk_table.CmdResetQueryPool(vk_command_buffer, vk_pool, range.offset,
                                  range.span);
}

void CommandEncoder::reset_statistics_query(gpu::StatisticsQuery query_,
                                            Slice32              range)
{
  ENCODE_PRELUDE();
  VkQueryPool const vk_pool = (VkQueryPool) query_;
  CHECK(!is_in_pass(), "");

  dev->vk_table.CmdResetQueryPool(vk_command_buffer, vk_pool, range.offset,
                                  range.span);
}

void CommandEncoder::write_timestamp(gpu::TimeStampQuery query_,
                                     gpu::PipelineStages stage, u32 index)
{
  ENCODE_PRELUDE();
  CHECK(!is_in_pass(), "");
  VkQueryPool const vk_pool = (VkQueryPool) query_;
  dev->vk_table.CmdWriteTimestamp(
    vk_command_buffer, (VkPipelineStageFlagBits) stage, vk_pool, index);
}

void CommandEncoder::begin_statistics(gpu::StatisticsQuery query_, u32 index)
{
  ENCODE_PRELUDE();
  CHECK(!is_in_pass(), "");
  VkQueryPool const vk_pool = (VkQueryPool) query_;
  dev->vk_table.CmdBeginQuery(vk_command_buffer, vk_pool, index, 0);
}

void CommandEncoder::end_statistics(gpu::StatisticsQuery query_, u32 index)
{
  ENCODE_PRELUDE();
  CHECK(!is_in_pass(), "");
  VkQueryPool const vk_pool = (VkQueryPool) query_;
  dev->vk_table.CmdEndQuery(vk_command_buffer, vk_pool, index);
}

void CommandEncoder::begin_debug_marker(Str region_name, f32x4 color)
{
  ENCODE_PRELUDE();
  CHECK(!is_in_pass(), "");
  char region_name_cstr[256];
  CHECK(to_c_str(region_name, region_name_cstr), "");

  VkDebugMarkerMarkerInfoEXT info{
    .sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT,
    .pNext       = nullptr,
    .pMarkerName = region_name_cstr,
    .color       = {color.x(), color.y(), color.z(), color.w()}
  };
  dev->vk_table.CmdDebugMarkerBeginEXT(vk_command_buffer, &info);
}

void CommandEncoder::end_debug_marker()
{
  ENCODE_PRELUDE();
  CHECK(!is_in_pass(), "");
  dev->vk_table.CmdDebugMarkerEndEXT(vk_command_buffer);
}

void CommandEncoder::fill_buffer(gpu::Buffer dst_, u64 offset, u64 size,
                                 u32 data)
{
  ENCODE_PRELUDE();
  auto * const dst = (Buffer *) dst_;

  CHECK(!is_in_pass(), "");
  CHECK(has_bits(dst->usage, gpu::BufferUsage::TransferDst), "");
  CHECK(is_valid_buffer_access(dst->size, offset, size, 4), "");
  CHECK(is_aligned<u64>(4, size), "");

  // access_buffer(*dst, VK_PIPELINE_STAGE_TRANSFER_BIT,
  // VK_ACCESS_TRANSFER_WRITE_BIT);
  dev->vk_table.CmdFillBuffer(vk_command_buffer, dst->vk_buffer, offset, size,
                              data);
}

void CommandEncoder::copy_buffer(gpu::Buffer src_, gpu::Buffer dst_,
                                 Span<gpu::BufferCopy const> copies)
{
  ENCODE_PRELUDE();
  auto * const src        = (Buffer *) src_;
  auto * const dst        = (Buffer *) dst_;
  u32 const    num_copies = size32(copies);

  CHECK(!is_in_pass(), "");
  CHECK(has_bits(src->usage, gpu::BufferUsage::TransferSrc), "");
  CHECK(has_bits(dst->usage, gpu::BufferUsage::TransferDst), "");
  CHECK(num_copies > 0, "");
  for (gpu::BufferCopy const & copy : copies)
  {
    CHECK(is_valid_buffer_access(src->size, copy.src_offset, copy.size), "");
    CHECK(is_valid_buffer_access(dst->size, copy.dst_offset, copy.size), "");
  }

  VkBufferCopy * vk_copies;

  if (!arg_pool.nalloc(num_copies, vk_copies))
  {
    status = Status::OutOfHostMemory;
    return;
  }

  for (u32 i = 0; i < num_copies; i++)
  {
    gpu::BufferCopy const & copy = copies[i];
    vk_copies[i]                 = VkBufferCopy{.srcOffset = copy.src_offset,
                                                .dstOffset = copy.dst_offset,
                                                .size      = copy.size};
  }

  // access_buffer(*src, VK_PIPELINE_STAGE_TRANSFER_BIT,
  //               VK_ACCESS_TRANSFER_READ_BIT);
  // access_buffer(*dst, VK_PIPELINE_STAGE_TRANSFER_BIT,
  //               VK_ACCESS_TRANSFER_WRITE_BIT);

  dev->vk_table.CmdCopyBuffer(vk_command_buffer, src->vk_buffer, dst->vk_buffer,
                              num_copies, vk_copies);
}

void CommandEncoder::update_buffer(Span<u8 const> src, u64 dst_offset,
                                   gpu::Buffer dst_)
{
  ENCODE_PRELUDE();
  auto * const dst       = (Buffer *) dst_;
  u64 const    copy_size = src.size_bytes();

  CHECK(!is_in_pass(), "");
  CHECK(has_bits(dst->usage, gpu::BufferUsage::TransferDst), "");
  CHECK(is_valid_buffer_access(dst->size, dst_offset, copy_size, 4), "");
  CHECK(is_aligned<u64>(4, copy_size), "");
  CHECK(copy_size <= gpu::MAX_UPDATE_BUFFER_SIZE, "");

  // access_buffer(*dst, VK_PIPELINE_STAGE_TRANSFER_BIT,
  //               VK_ACCESS_TRANSFER_WRITE_BIT);

  dev->vk_table.CmdUpdateBuffer(vk_command_buffer, dst->vk_buffer, dst_offset,
                                (u64) src.size(), src.data());
}

void CommandEncoder::clear_color_image(
  gpu::Image dst_, gpu::Color value,
  Span<gpu::ImageSubresourceRange const> ranges)
{
  ENCODE_PRELUDE();
  auto * const dst        = (Image *) dst_;
  u32 const    num_ranges = size32(ranges);

  static_assert(sizeof(gpu::Color) == sizeof(VkClearColorValue));
  CHECK(!is_in_pass(), "");
  CHECK(has_bits(dst->usage, gpu::ImageUsage::TransferDst), "");
  CHECK(num_ranges > 0, "");
  for (u32 i = 0; i < num_ranges; i++)
  {
    gpu::ImageSubresourceRange const & range = ranges[i];
    CHECK(is_valid_image_access(
            dst->aspects, dst->mip_levels, dst->array_layers, range.aspects,
            range.first_mip_level, range.num_mip_levels,
            range.first_array_layer, range.num_array_layers),
          "");
  }

  VkImageSubresourceRange * vk_ranges;
  if (!arg_pool.nalloc(num_ranges, vk_ranges))
  {
    status = Status::OutOfHostMemory;
    return;
  }

  for (u32 i = 0; i < num_ranges; i++)
  {
    gpu::ImageSubresourceRange const & range = ranges[i];
    vk_ranges[i] =
      VkImageSubresourceRange{.aspectMask = (VkImageAspectFlags) range.aspects,
                              .baseMipLevel   = range.first_mip_level,
                              .levelCount     = range.num_mip_levels,
                              .baseArrayLayer = range.first_array_layer,
                              .layerCount     = range.num_array_layers};
  }

  // access_image(*dst, VK_PIPELINE_STAGE_TRANSFER_BIT,
  //              VK_ACCESS_TRANSFER_WRITE_BIT,
  //              VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

  VkClearColorValue vk_color;
  std::memcpy(&vk_color, &value, sizeof(VkClearColorValue));

  dev->vk_table.CmdClearColorImage(vk_command_buffer, dst->vk_image,
                                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                   &vk_color, num_ranges, vk_ranges);
}

void CommandEncoder::clear_depth_stencil_image(
  gpu::Image dst_, gpu::DepthStencil value,
  Span<gpu::ImageSubresourceRange const> ranges)
{
  ENCODE_PRELUDE();
  auto * const dst        = (Image *) dst_;
  u32 const    num_ranges = size32(ranges);

  static_assert(sizeof(gpu::DepthStencil) == sizeof(VkClearDepthStencilValue));
  CHECK(!is_in_pass(), "");
  CHECK(num_ranges > 0, "");
  CHECK(has_bits(dst->usage, gpu::ImageUsage::TransferDst), "");
  for (u32 i = 0; i < num_ranges; i++)
  {
    gpu::ImageSubresourceRange const & range = ranges[i];
    CHECK(is_valid_image_access(
            dst->aspects, dst->mip_levels, dst->array_layers, range.aspects,
            range.first_mip_level, range.num_mip_levels,
            range.first_array_layer, range.num_array_layers),
          "");
  }

  VkImageSubresourceRange * vk_ranges;
  if (!arg_pool.nalloc(num_ranges, vk_ranges))
  {
    status = Status::OutOfHostMemory;
    return;
  }

  for (u32 i = 0; i < num_ranges; i++)
  {
    gpu::ImageSubresourceRange const & range = ranges[i];
    vk_ranges[i] =
      VkImageSubresourceRange{.aspectMask = (VkImageAspectFlags) range.aspects,
                              .baseMipLevel   = range.first_mip_level,
                              .levelCount     = range.num_mip_levels,
                              .baseArrayLayer = range.first_array_layer,
                              .layerCount     = range.num_array_layers};
  }

  // access_image(*dst, VK_PIPELINE_STAGE_TRANSFER_BIT,
  //              VK_ACCESS_TRANSFER_WRITE_BIT,
  //              VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

  VkClearDepthStencilValue vk_depth_stencil;
  std::memcpy(&vk_depth_stencil, &value, sizeof(gpu::DepthStencil));

  dev->vk_table.CmdClearDepthStencilImage(
    vk_command_buffer, dst->vk_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    &vk_depth_stencil, num_ranges, vk_ranges);
}

void CommandEncoder::copy_image(gpu::Image src_, gpu::Image dst_,
                                Span<gpu::ImageCopy const> copies)
{
  ENCODE_PRELUDE();
  auto * const src        = (Image *) src_;
  auto * const dst        = (Image *) dst_;
  u32 const    num_copies = size32(copies);

  CHECK(!is_in_pass(), "");
  CHECK(num_copies > 0, "");
  CHECK(has_bits(src->usage, gpu::ImageUsage::TransferSrc), "");
  CHECK(has_bits(dst->usage, gpu::ImageUsage::TransferDst), "");
  for (u32 i = 0; i < num_copies; i++)
  {
    gpu::ImageCopy const & copy = copies[i];

    CHECK(is_valid_image_access(src->aspects, src->mip_levels,
                                src->array_layers, copy.src_layers.aspects,
                                copy.src_layers.mip_level, 1,
                                copy.src_layers.first_array_layer,
                                copy.src_layers.num_array_layers),
          "");
    CHECK(is_valid_image_access(dst->aspects, dst->mip_levels,
                                dst->array_layers, copy.dst_layers.aspects,
                                copy.dst_layers.mip_level, 1,
                                copy.dst_layers.first_array_layer,
                                copy.dst_layers.num_array_layers),
          "");

    auto src_extent = src->extent.mip(copy.src_layers.mip_level);
    auto dst_extent = dst->extent.mip(copy.dst_layers.mip_level);
    CHECK(copy.src_area.extent.x() > 0, "");
    CHECK(copy.src_area.extent.y() > 0, "");
    CHECK(copy.src_area.extent.z() > 0, "");
    CHECK(copy.src_area.offset.x() <= src_extent.x(), "");
    CHECK(copy.src_area.offset.y() <= src_extent.y(), "");
    CHECK(copy.src_area.offset.z() <= src_extent.z(), "");
    CHECK(copy.src_area.end().x() <= src_extent.x(), "");
    CHECK(copy.src_area.end().y() <= src_extent.y(), "");
    CHECK(copy.src_area.end().z() <= src_extent.z(), "");
    CHECK(copy.dst_offset.x() <= dst_extent.x(), "");
    CHECK(copy.dst_offset.y() <= dst_extent.y(), "");
    CHECK(copy.dst_offset.z() <= dst_extent.z(), "");
    CHECK((copy.dst_offset.x() + copy.src_area.extent.x()) <= dst_extent.x(),
          "");
    CHECK((copy.dst_offset.y() + copy.src_area.extent.y()) <= dst_extent.y(),
          "");
    CHECK((copy.dst_offset.z() + copy.src_area.extent.z()) <= dst_extent.z(),
          "");
  }

  VkImageCopy * vk_copies;
  if (!arg_pool.nalloc(num_copies, vk_copies))
  {
    status = Status::OutOfHostMemory;
    return;
  }

  for (u32 i = 0; i < num_copies; i++)
  {
    gpu::ImageCopy const &   copy = copies[i];
    VkImageSubresourceLayers src_subresource{
      .aspectMask     = (VkImageAspectFlags) copy.src_layers.aspects,
      .mipLevel       = copy.src_layers.mip_level,
      .baseArrayLayer = copy.src_layers.first_array_layer,
      .layerCount     = copy.src_layers.num_array_layers};
    VkOffset3D               src_offset{(i32) copy.src_area.offset.x(),
                          (i32) copy.src_area.offset.y(),
                          (i32) copy.src_area.offset.z()};
    VkImageSubresourceLayers dst_subresource{
      .aspectMask     = (VkImageAspectFlags) copy.dst_layers.aspects,
      .mipLevel       = copy.dst_layers.mip_level,
      .baseArrayLayer = copy.dst_layers.first_array_layer,
      .layerCount     = copy.dst_layers.num_array_layers};
    VkOffset3D dst_offset{(i32) copy.dst_offset.x(), (i32) copy.dst_offset.y(),
                          (i32) copy.dst_offset.z()};
    VkExtent3D extent{copy.src_area.extent.x(), copy.src_area.extent.y(),
                      copy.src_area.extent.z()};

    vk_copies[i] = VkImageCopy{.srcSubresource = src_subresource,
                               .srcOffset      = src_offset,
                               .dstSubresource = dst_subresource,
                               .dstOffset      = dst_offset,
                               .extent         = extent};
  }

  // access_image(*src, VK_PIPELINE_STAGE_TRANSFER_BIT,
  //              VK_ACCESS_TRANSFER_READ_BIT,
  //              VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
  // access_image(*dst, VK_PIPELINE_STAGE_TRANSFER_BIT,
  //              VK_ACCESS_TRANSFER_WRITE_BIT,
  //              VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

  dev->vk_table.CmdCopyImage(
    vk_command_buffer, src->vk_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
    dst->vk_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, num_copies, vk_copies);
}

void CommandEncoder::copy_buffer_to_image(
  gpu::Buffer src_, gpu::Image dst_, Span<gpu::BufferImageCopy const> copies)
{
  ENCODE_PRELUDE();
  auto * const src        = (Buffer *) src_;
  auto * const dst        = (Image *) dst_;
  u32 const    num_copies = size32(copies);

  CHECK(!is_in_pass(), "");
  CHECK(num_copies > 0, "");
  CHECK(has_bits(src->usage, gpu::BufferUsage::TransferSrc), "");
  CHECK(has_bits(dst->usage, gpu::ImageUsage::TransferDst), "");
  for (u32 i = 0; i < num_copies; i++)
  {
    gpu::BufferImageCopy const & copy = copies[i];
    CHECK(
      is_valid_buffer_access(src->size, copy.buffer_offset, gpu::WHOLE_SIZE),
      "");

    CHECK(is_valid_image_access(dst->aspects, dst->mip_levels,
                                dst->array_layers, copy.image_layers.aspects,
                                copy.image_layers.mip_level, 1,
                                copy.image_layers.first_array_layer,
                                copy.image_layers.num_array_layers),
          "");

    CHECK(copy.image_area.extent.x() > 0, "");
    CHECK(copy.image_area.extent.y() > 0, "");
    CHECK(copy.image_area.extent.z() > 0, "");
    auto dst_extent = dst->extent.mip(copy.image_layers.mip_level);
    CHECK(copy.image_area.extent.x() <= dst_extent.x(), "");
    CHECK(copy.image_area.extent.y() <= dst_extent.y(), "");
    CHECK(copy.image_area.extent.z() <= dst_extent.z(), "");
    CHECK(copy.image_area.end().x() <= dst_extent.x(), "");
    CHECK(copy.image_area.end().y() <= dst_extent.y(), "");
    CHECK(copy.image_area.end().z() <= dst_extent.z(), "");
  }

  VkBufferImageCopy * vk_copies;
  if (!arg_pool.nalloc(num_copies, vk_copies))
  {
    status = Status::OutOfHostMemory;
    return;
  }

  for (u32 i = 0; i < num_copies; i++)
  {
    gpu::BufferImageCopy const & copy = copies[i];
    VkImageSubresourceLayers     image_subresource{
          .aspectMask     = (VkImageAspectFlags) copy.image_layers.aspects,
          .mipLevel       = copy.image_layers.mip_level,
          .baseArrayLayer = copy.image_layers.first_array_layer,
          .layerCount     = copy.image_layers.num_array_layers};
    vk_copies[i] = VkBufferImageCopy{
      .bufferOffset      = copy.buffer_offset,
      .bufferRowLength   = copy.buffer_row_length,
      .bufferImageHeight = copy.buffer_image_height,
      .imageSubresource  = image_subresource,
      .imageOffset       = VkOffset3D{(i32) copy.image_area.offset.x(),
                                      (i32) copy.image_area.offset.y(),
                                      (i32) copy.image_area.offset.z()},
      .imageExtent =
        VkExtent3D{copy.image_area.extent.x(),       copy.image_area.extent.y(),
                                      copy.image_area.extent.z()      }
    };
  }

  // access_buffer(*src, VK_PIPELINE_STAGE_TRANSFER_BIT,
  //               VK_ACCESS_TRANSFER_READ_BIT);
  // access_image(*dst, VK_PIPELINE_STAGE_TRANSFER_BIT,
  //              VK_ACCESS_TRANSFER_WRITE_BIT,
  //              VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

  dev->vk_table.CmdCopyBufferToImage(
    vk_command_buffer, src->vk_buffer, dst->vk_image,
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, num_copies, vk_copies);
}

void CommandEncoder::blit_image(gpu::Image src_, gpu::Image dst_,
                                Span<gpu::ImageBlit const> blits,
                                gpu::Filter                filter)
{
  ENCODE_PRELUDE();
  auto * const src       = (Image *) src_;
  auto * const dst       = (Image *) dst_;
  u32 const    num_blits = size32(blits);

  CHECK(!is_in_pass(), "");    // [ ] not needed
  CHECK(num_blits > 0, "");
  // [ ] pass_id?
  CHECK(has_bits(src->usage, gpu::ImageUsage::TransferSrc), "");
  CHECK(has_bits(dst->usage, gpu::ImageUsage::TransferDst), "");
  for (u32 i = 0; i < num_blits; i++)
  {
    gpu::ImageBlit const & blit = blits[i];

    CHECK(is_valid_image_access(src->aspects, src->mip_levels,
                                src->array_layers, blit.src_layers.aspects,
                                blit.src_layers.mip_level, 1,
                                blit.src_layers.first_array_layer,
                                blit.src_layers.num_array_layers),
          "");

    CHECK(is_valid_image_access(dst->aspects, dst->mip_levels,
                                dst->array_layers, blit.dst_layers.aspects,
                                blit.dst_layers.mip_level, 1,
                                blit.dst_layers.first_array_layer,
                                blit.dst_layers.num_array_layers),
          "");

    auto src_extent = src->extent.mip(blit.src_layers.mip_level);
    auto dst_extent = dst->extent.mip(blit.dst_layers.mip_level);
    CHECK(blit.src_area.offset.x() <= src_extent.x(), "");
    CHECK(blit.src_area.offset.y() <= src_extent.y(), "");
    CHECK(blit.src_area.offset.z() <= src_extent.z(), "");
    CHECK(blit.src_area.end().x() <= src_extent.x(), "");
    CHECK(blit.src_area.end().y() <= src_extent.y(), "");
    CHECK(blit.src_area.end().z() <= src_extent.z(), "");
    CHECK(blit.dst_area.offset.x() <= dst_extent.x(), "");
    CHECK(blit.dst_area.offset.y() <= dst_extent.y(), "");
    CHECK(blit.dst_area.offset.z() <= dst_extent.z(), "");
    CHECK(blit.dst_area.end().x() <= dst_extent.x(), "");
    CHECK(blit.dst_area.end().y() <= dst_extent.y(), "");
    CHECK(blit.dst_area.end().z() <= dst_extent.z(), "");
  }

  VkImageBlit * vk_blits;
  if (!arg_pool.nalloc(num_blits, vk_blits))
  {
    status = Status::OutOfHostMemory;
    return;
  }

  for (u32 i = 0; i < num_blits; i++)
  {
    gpu::ImageBlit const &   blit = blits[i];
    VkImageSubresourceLayers src_subresource{
      .aspectMask     = (VkImageAspectFlags) blit.src_layers.aspects,
      .mipLevel       = blit.src_layers.mip_level,
      .baseArrayLayer = blit.src_layers.first_array_layer,
      .layerCount     = blit.src_layers.num_array_layers};
    VkImageSubresourceLayers dst_subresource{
      .aspectMask     = (VkImageAspectFlags) blit.dst_layers.aspects,
      .mipLevel       = blit.dst_layers.mip_level,
      .baseArrayLayer = blit.dst_layers.first_array_layer,
      .layerCount     = blit.dst_layers.num_array_layers};
    vk_blits[i] = VkImageBlit{
      .srcSubresource = src_subresource,
      .srcOffsets     = {VkOffset3D{(i32) blit.src_area.offset.x(),
                                (i32) blit.src_area.offset.y(),
                                (i32) blit.src_area.offset.z()},
                         VkOffset3D{(i32) blit.src_area.end().x(),
                                (i32) blit.src_area.end().y(),
                                (i32) blit.src_area.end().z()}},
      .dstSubresource = dst_subresource,
      .dstOffsets     = {VkOffset3D{(i32) blit.dst_area.offset.x(),
                                (i32) blit.dst_area.offset.y(),
                                (i32) blit.dst_area.offset.z()},
                         VkOffset3D{(i32) blit.dst_area.end().x(),
                                (i32) blit.dst_area.end().y(),
                                (i32) blit.dst_area.end().z()}}
    };
  }

  // access_image(*src, VK_PIPELINE_STAGE_TRANSFER_BIT,
  //              VK_ACCESS_TRANSFER_READ_BIT,
  //              VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
  // access_image(*dst, VK_PIPELINE_STAGE_TRANSFER_BIT,
  //              VK_ACCESS_TRANSFER_WRITE_BIT,
  //              VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  dev->vk_table.CmdBlitImage(
    vk_command_buffer, src->vk_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
    dst->vk_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, num_blits, vk_blits,
    (VkFilter) filter);
}

void CommandEncoder::resolve_image(gpu::Image src_, gpu::Image dst_,
                                   Span<gpu::ImageResolve const> resolves)
{
  ENCODE_PRELUDE();
  auto * const src          = (Image *) src_;
  auto * const dst          = (Image *) dst_;
  u32 const    num_resolves = size32(resolves);

  CHECK(!is_in_pass(), "");
  CHECK(num_resolves > 0, "");
  CHECK(has_bits(src->usage, gpu::ImageUsage::TransferSrc), "");
  CHECK(has_bits(dst->usage, gpu::ImageUsage::TransferDst), "");
  CHECK(has_bits(dst->sample_count, gpu::SampleCount::C1), "");

  for (u32 i = 0; i < num_resolves; i++)
  {
    gpu::ImageResolve const & resolve = resolves[i];

    CHECK(is_valid_image_access(src->aspects, src->mip_levels,
                                src->array_layers, resolve.src_layers.aspects,
                                resolve.src_layers.mip_level, 1,
                                resolve.src_layers.first_array_layer,
                                resolve.src_layers.num_array_layers),
          "");
    CHECK(is_valid_image_access(dst->aspects, dst->mip_levels,
                                dst->array_layers, resolve.dst_layers.aspects,
                                resolve.dst_layers.mip_level, 1,
                                resolve.dst_layers.first_array_layer,
                                resolve.dst_layers.num_array_layers),
          "");

    auto src_extent = src->extent.mip(resolve.src_layers.mip_level);
    auto dst_extent = dst->extent.mip(resolve.dst_layers.mip_level);
    CHECK(resolve.src_area.extent.x() > 0, "");
    CHECK(resolve.src_area.extent.y() > 0, "");
    CHECK(resolve.src_area.extent.z() > 0, "");
    CHECK(resolve.src_area.offset.x() <= src_extent.x(), "");
    CHECK(resolve.src_area.offset.y() <= src_extent.y(), "");
    CHECK(resolve.src_area.offset.z() <= src_extent.z(), "");
    CHECK(resolve.src_area.end().x() <= src_extent.x(), "");
    CHECK(resolve.src_area.end().y() <= src_extent.y(), "");
    CHECK(resolve.src_area.end().z() <= src_extent.z(), "");
    CHECK(resolve.dst_offset.x() <= dst_extent.x(), "");
    CHECK(resolve.dst_offset.y() <= dst_extent.y(), "");
    CHECK(resolve.dst_offset.z() <= dst_extent.z(), "");
    CHECK((resolve.dst_offset.x() + resolve.src_area.extent.x()) <=
            dst_extent.x(),
          "");
    CHECK((resolve.dst_offset.y() + resolve.src_area.extent.y()) <=
            dst_extent.y(),
          "");
    CHECK((resolve.dst_offset.z() + resolve.src_area.extent.z()) <=
            dst_extent.z(),
          "");
  }

  VkImageResolve * vk_resolves;
  if (!arg_pool.nalloc<VkImageResolve>(num_resolves, vk_resolves))
  {
    status = Status::OutOfHostMemory;
    return;
  }

  for (u32 i = 0; i < num_resolves; i++)
  {
    gpu::ImageResolve const & resolve = resolves[i];
    VkImageSubresourceLayers  src_subresource{
       .aspectMask     = (VkImageAspectFlags) resolve.src_layers.aspects,
       .mipLevel       = resolve.src_layers.mip_level,
       .baseArrayLayer = resolve.src_layers.first_array_layer,
       .layerCount     = resolve.src_layers.num_array_layers};
    VkOffset3D               src_offset{(i32) resolve.src_area.offset.x(),
                          (i32) resolve.src_area.offset.y(),
                          (i32) resolve.src_area.offset.z()};
    VkImageSubresourceLayers dst_subresource{
      .aspectMask     = (VkImageAspectFlags) resolve.dst_layers.aspects,
      .mipLevel       = resolve.dst_layers.mip_level,
      .baseArrayLayer = resolve.dst_layers.first_array_layer,
      .layerCount     = resolve.dst_layers.num_array_layers};
    VkOffset3D dst_offset{(i32) resolve.dst_offset.x(),
                          (i32) resolve.dst_offset.y(),
                          (i32) resolve.dst_offset.z()};
    VkExtent3D extent{resolve.src_area.extent.x(), resolve.src_area.extent.y(),
                      resolve.src_area.extent.z()};

    vk_resolves[i] = VkImageResolve{.srcSubresource = src_subresource,
                                    .srcOffset      = src_offset,
                                    .dstSubresource = dst_subresource,
                                    .dstOffset      = dst_offset,
                                    .extent         = extent};
  }

  // access_image(*src, VK_PIPELINE_STAGE_TRANSFER_BIT,
  //              VK_ACCESS_TRANSFER_READ_BIT,
  //              VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
  // access_image(*dst, VK_PIPELINE_STAGE_TRANSFER_BIT,
  //              VK_ACCESS_TRANSFER_WRITE_BIT,
  //              VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

  dev->vk_table.CmdResolveImage(
    vk_command_buffer, src->vk_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
    dst->vk_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, num_resolves,
    vk_resolves);
}

void CommandEncoder::begin_compute_pass()
{
  ENCODE_PRELUDE();
  CHECK(!is_in_pass(), "");

  state = CommandEncoderState::ComputePass;
}

void CommandEncoder::end_compute_pass()
{
  ENCODE_PRELUDE();
  CHECK(is_in_compute_pass(), "");

  clear_context();
}

void CommandEncoder::begin_rendering(gpu::RenderingInfo const & info)
{
  ENCODE_PRELUDE();

  CHECK(!is_in_pass(), "");
  CHECK(info.color_attachments.size() <= gpu::MAX_PIPELINE_COLOR_ATTACHMENTS,
        "");
  CHECK(info.render_area.extent.x() > 0, "");
  CHECK(info.render_area.extent.y() > 0, "");
  CHECK(info.num_layers > 0, "");

  for (gpu::RenderingAttachment const & attachment : info.color_attachments)
  {
    validate_attachment(attachment, gpu::ImageAspects::Color,
                        gpu::ImageUsage::ColorAttachment);
  }

  info.depth_attachment.match([](auto & depth) {
    validate_attachment(depth, gpu::ImageAspects::Depth,
                        gpu::ImageUsage::DepthStencilAttachment);
  });

  info.stencil_attachment.match([](auto & stencil) {
    validate_attachment(stencil, gpu::ImageAspects::Stencil,
                        gpu::ImageUsage::DepthStencilAttachment);
  });

  clear_context();
  render_ctx.color_attachments.extend(info.color_attachments).unwrap();
  render_ctx.depth_attachment   = info.depth_attachment;
  render_ctx.stencil_attachment = info.stencil_attachment;
  state                         = CommandEncoderState::RenderPass;
  render_ctx.render_area        = info.render_area;
  render_ctx.num_layers         = info.num_layers;
}

void CommandEncoder::end_rendering()
{
  ENCODE_PRELUDE();
  RenderPassContext & ctx = render_ctx;
  DeviceTable const * t   = &dev->vk_table;

  CHECK(is_in_render_pass(), "");

  // synchronization pass: bindings
  for (Command const & cmd : render_ctx.commands)
  {
    cmd.match(
      [&](CmdBindDescriptorSets const & c) {
        for (auto set : c.sets)
        {
          // access_graphics_bindings(*set, ctx.pass_timestamp);
        }
      },
      [&](CmdBindGraphicsPipeline const &) {}, [&](CmdPushConstants const &) {},
      [&](CmdSetGraphicsState const &) {},
      [&](CmdBindVertexBuffer const & c) {
        access_buffer(*c.buffer, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                      VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT, ctx.pass_timestamp);
      },
      [&](CmdBindIndexBuffer const & c) {
        access_buffer(*c.buffer, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                      VK_ACCESS_INDEX_READ_BIT, ctx.pass_timestamp);
      },
      [&](CmdDraw const &) {}, [&](CmdDrawIndexed const &) {},
      [&](CmdDrawIndirect const & c) {
        access_buffer(*c.buffer, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
                      VK_ACCESS_INDIRECT_COMMAND_READ_BIT, ctx.pass_timestamp);
      },
      [&](CmdDrawIndexedIndirect const & c) {
        access_buffer(*c.buffer, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
                      VK_ACCESS_INDIRECT_COMMAND_READ_BIT, ctx.pass_timestamp);
      });
  }

  // synchronization pass: attachments
  {
    InplaceVec<VkRenderingAttachmentInfoKHR,
               gpu::MAX_PIPELINE_COLOR_ATTACHMENTS>
      vk_color_attachments{};

    constexpr VkPipelineStageFlags RESOLVE_STAGE =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    constexpr VkAccessFlags RESOLVE_COLOR_SRC_ACCESS =
      VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;

    constexpr VkAccessFlags RESOLVE_COLOR_DST_ACCESS =
      VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
      VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    constexpr VkAccessFlags RESOLVE_DEPTH_STENCIL_SRC_ACCESS =
      VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    constexpr VkAccessFlags RESOLVE_DEPTH_STENCIL_DST_ACCESS =
      VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
      VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    constexpr VkImageLayout RESOLVE_COLOR_LAYOUT =
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    constexpr VkImageLayout RESOLVE_DEPTH_STENCIL_LAYOUT =
      VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    for (auto const & attachment : ctx.color_attachments)
    {
      VkAccessFlags        access     = color_attachment_access(attachment);
      VkImageView          vk_view    = nullptr;
      VkImageView          vk_resolve = nullptr;
      VkPipelineStageFlags stages =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
      VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
      VkClearValue  clear_value{.color{.uint32{
        attachment.clear.color.u32.x(), attachment.clear.color.u32.y(),
        attachment.clear.color.u32.z(), attachment.clear.color.u32.w()}}};

      if (attachment.resolve_mode != gpu::ResolveModes::None)
      {
        access |= RESOLVE_COLOR_SRC_ACCESS;
        stages |= RESOLVE_STAGE;
      }

      if (attachment.view != nullptr)
      {
        ImageView * view = (ImageView *) attachment.view;
        vk_view          = view->vk_view;

        access_image(*IMAGE_FROM_VIEW(view), stages, access, layout);

        if (attachment.resolve_mode != gpu::ResolveModes::None)
        {
          ImageView * resolve = (ImageView *) attachment.resolve;
          vk_resolve          = resolve->vk_view;
          access_image(*IMAGE_FROM_VIEW(resolve), RESOLVE_STAGE,
                       RESOLVE_COLOR_DST_ACCESS, RESOLVE_COLOR_LAYOUT);
        }
      }

      vk_color_attachments
        .push(VkRenderingAttachmentInfoKHR{
          .sType              = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
          .pNext              = nullptr,
          .imageView          = vk_view,
          .imageLayout        = layout,
          .resolveMode        = (VkResolveModeFlagBits) attachment.resolve_mode,
          .resolveImageView   = vk_resolve,
          .resolveImageLayout = RESOLVE_COLOR_LAYOUT,
          .loadOp             = (VkAttachmentLoadOp) attachment.load_op,
          .storeOp            = (VkAttachmentStoreOp) attachment.store_op,
          .clearValue         = clear_value})
        .unwrap();
    }

    auto vk_depth_attachment = ctx.depth_attachment.map([&](auto & attachment) {
      VkAccessFlags access = depth_stencil_attachment_access(attachment) |
                             RESOLVE_DEPTH_STENCIL_SRC_ACCESS;
      VkPipelineStageFlags stages = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                                    VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT |
                                    RESOLVE_STAGE;

      VkImageView   vk_view    = nullptr;
      VkImageView   vk_resolve = nullptr;
      VkImageLayout layout =
        has_write_access(access) ?
          VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL :
          VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

      VkClearValue clear_value{
        .depthStencil{.depth = attachment.clear.depth_stencil.depth}};

      if (attachment.view != nullptr)
      {
        ImageView * view = (ImageView *) attachment.view;
        vk_view          = view->vk_view;

        access_image(*IMAGE_FROM_VIEW(view), stages, access, layout);

        if (attachment.resolve_mode != gpu::ResolveModes::None)
        {
          ImageView * resolve = (ImageView *) attachment.resolve;
          vk_resolve          = resolve->vk_view;
          access_image(*IMAGE_FROM_VIEW(resolve), RESOLVE_STAGE,
                       RESOLVE_DEPTH_STENCIL_DST_ACCESS |
                         RESOLVE_DEPTH_STENCIL_SRC_ACCESS,
                       RESOLVE_DEPTH_STENCIL_LAYOUT);
        }
      }

      return VkRenderingAttachmentInfoKHR{
        .sType              = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
        .pNext              = nullptr,
        .imageView          = vk_view,
        .imageLayout        = layout,
        .resolveMode        = (VkResolveModeFlagBits) attachment.resolve_mode,
        .resolveImageView   = vk_resolve,
        .resolveImageLayout = RESOLVE_DEPTH_STENCIL_LAYOUT,
        .loadOp             = (VkAttachmentLoadOp) attachment.load_op,
        .storeOp            = (VkAttachmentStoreOp) attachment.store_op,
        .clearValue         = clear_value};
    });

    auto vk_stencil_attachment =
      ctx.stencil_attachment.map([&](auto & attachment) {
        VkAccessFlags access = depth_stencil_attachment_access(attachment) |
                               RESOLVE_DEPTH_STENCIL_SRC_ACCESS;
        VkImageView   vk_view    = nullptr;
        VkImageView   vk_resolve = nullptr;
        VkImageLayout layout =
          has_write_access(access) ?
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL :
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        VkPipelineStageFlags stages =
          VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
          VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT | RESOLVE_STAGE;

        VkClearValue clear_value{
          .depthStencil{.stencil = attachment.clear.depth_stencil.stencil}};

        if (attachment.view != nullptr)
        {
          ImageView * view = (ImageView *) attachment.view;
          vk_view          = view->vk_view;

          access_image(*IMAGE_FROM_VIEW(view), stages, access, layout);

          if (attachment.resolve_mode != gpu::ResolveModes::None)
          {
            ImageView * resolve = (ImageView *) attachment.resolve;
            vk_resolve          = resolve->vk_view;
            access_image(*IMAGE_FROM_VIEW(resolve), RESOLVE_STAGE,
                         RESOLVE_DEPTH_STENCIL_DST_ACCESS,
                         RESOLVE_DEPTH_STENCIL_LAYOUT);
          }
        }

        return VkRenderingAttachmentInfoKHR{
          .sType              = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
          .pNext              = nullptr,
          .imageView          = vk_view,
          .imageLayout        = layout,
          .resolveMode        = (VkResolveModeFlagBits) attachment.resolve_mode,
          .resolveImageView   = vk_resolve,
          .resolveImageLayout = RESOLVE_DEPTH_STENCIL_LAYOUT,
          .loadOp             = (VkAttachmentLoadOp) attachment.load_op,
          .storeOp            = (VkAttachmentStoreOp) attachment.store_op,
          .clearValue         = clear_value};
      });

    VkRenderingInfoKHR begin_info{
      .sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
      .pNext = nullptr,
      .flags = 0,
      .renderArea =
        VkRect2D{.offset = VkOffset2D{.x = (i32) ctx.render_area.offset.x(),
                                      .y = (i32) ctx.render_area.offset.y()},
                 .extent = VkExtent2D{.width  = ctx.render_area.extent.x(),
                                      .height = ctx.render_area.extent.y()}},
      .layerCount           = ctx.num_layers,
      .viewMask             = 0,
      .colorAttachmentCount = size32(vk_color_attachments),
      .pColorAttachments    = vk_color_attachments.data(),
      .pDepthAttachment     = vk_depth_attachment.as_ptr().unwrap_or(nullptr),
      .pStencilAttachment = vk_stencil_attachment.as_ptr().unwrap_or(nullptr)
    };

    t->CmdBeginRenderingKHR(vk_command_buffer, &begin_info);
  }

  GraphicsPipeline * pipeline = nullptr;

  for (Command const & cmd : ctx.commands)
  {
    cmd.match(
      [&](CmdBindDescriptorSets const & c) {
        InplaceVec<VkDescriptorSet, gpu::MAX_PIPELINE_DESCRIPTOR_SETS> vk_sets;

        for (auto & set : c.sets)
        {
          vk_sets.push(set->vk_set).unwrap();
        }

        t->CmdBindDescriptorSets(
          vk_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
          pipeline->vk_layout, 0, size32(vk_sets), vk_sets.data(),
          size32(c.dynamic_offsets), c.dynamic_offsets.data());
      },
      [&](CmdBindGraphicsPipeline const & c) {
        pipeline = c.pipeline;
        t->CmdBindPipeline(vk_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                           pipeline->vk_pipeline);
      },
      [&](CmdPushConstants const & c) {
        t->CmdPushConstants(
          vk_command_buffer, pipeline->vk_layout, VK_SHADER_STAGE_ALL, 0,
          size32(c.constant.view().as_u8()), c.constant.view().as_u8().data());
      },
      [&](CmdSetGraphicsState const & c) {
        gpu::GraphicsState const & s = c.state;

        VkRect2D vk_scissor{
          .offset =
            VkOffset2D{(i32) s.scissor.offset.x(), (i32) s.scissor.offset.y()},
          .extent = VkExtent2D{s.scissor.extent.x(),       s.scissor.extent.y()      }
        };
        t->CmdSetScissor(vk_command_buffer, 0, 1, &vk_scissor);

        VkViewport vk_viewport{.x        = s.viewport.offset.x(),
                               .y        = s.viewport.offset.y(),
                               .width    = s.viewport.extent.x(),
                               .height   = s.viewport.extent.y(),
                               .minDepth = s.viewport.min_depth,
                               .maxDepth = s.viewport.max_depth};
        t->CmdSetViewport(vk_command_buffer, 0, 1, &vk_viewport);

        f32 vk_constant[4] = {s.blend_constant.x(), s.blend_constant.y(),
                              s.blend_constant.z(), s.blend_constant.w()};
        t->CmdSetBlendConstants(vk_command_buffer, vk_constant);

        t->CmdSetStencilTestEnableEXT(vk_command_buffer, s.stencil_test_enable);

        t->CmdSetStencilReference(vk_command_buffer, VK_STENCIL_FACE_FRONT_BIT,
                                  s.front_face_stencil.reference);
        t->CmdSetStencilCompareMask(vk_command_buffer,
                                    VK_STENCIL_FACE_FRONT_BIT,
                                    s.front_face_stencil.compare_mask);
        t->CmdSetStencilWriteMask(vk_command_buffer, VK_STENCIL_FACE_FRONT_BIT,
                                  s.front_face_stencil.write_mask);
        t->CmdSetStencilOpEXT(vk_command_buffer, VK_STENCIL_FACE_FRONT_BIT,
                              (VkStencilOp) s.front_face_stencil.fail_op,
                              (VkStencilOp) s.front_face_stencil.pass_op,
                              (VkStencilOp) s.front_face_stencil.depth_fail_op,
                              (VkCompareOp) s.front_face_stencil.compare_op);

        t->CmdSetStencilReference(vk_command_buffer, VK_STENCIL_FACE_BACK_BIT,
                                  s.back_face_stencil.reference);
        t->CmdSetStencilCompareMask(vk_command_buffer, VK_STENCIL_FACE_BACK_BIT,
                                    s.back_face_stencil.compare_mask);
        t->CmdSetStencilWriteMask(vk_command_buffer, VK_STENCIL_FACE_BACK_BIT,
                                  s.back_face_stencil.write_mask);
        t->CmdSetStencilOpEXT(vk_command_buffer, VK_STENCIL_FACE_BACK_BIT,
                              (VkStencilOp) s.back_face_stencil.fail_op,
                              (VkStencilOp) s.back_face_stencil.pass_op,
                              (VkStencilOp) s.back_face_stencil.depth_fail_op,
                              (VkCompareOp) s.back_face_stencil.compare_op);
        t->CmdSetCullModeEXT(vk_command_buffer, (VkCullModeFlags) s.cull_mode);
        t->CmdSetFrontFaceEXT(vk_command_buffer, (VkFrontFace) s.front_face);
        t->CmdSetDepthTestEnableEXT(vk_command_buffer, s.depth_test_enable);
        t->CmdSetDepthCompareOpEXT(vk_command_buffer,
                                   (VkCompareOp) s.depth_compare_op);
        t->CmdSetDepthWriteEnableEXT(vk_command_buffer, s.depth_write_enable);
        t->CmdSetDepthBoundsTestEnableEXT(vk_command_buffer,
                                          s.depth_bounds_test_enable);
      },
      [&](CmdBindVertexBuffer const & c) {
        t->CmdBindVertexBuffers(vk_command_buffer, c.binding, 1,
                                &c.buffer->vk_buffer, &c.offset);
      },
      [&](CmdBindIndexBuffer const & c) {
        t->CmdBindIndexBuffer(vk_command_buffer, c.buffer->vk_buffer, c.offset,
                              (VkIndexType) c.index_type);
      },
      [&](CmdDraw const & c) {
        t->CmdDraw(vk_command_buffer, c.vertex_count, c.instance_count,
                   c.first_vertex, c.first_instance);
      },
      [&](CmdDrawIndexed const & c) {
        t->CmdDrawIndexed(vk_command_buffer, c.num_indices, c.num_instances,
                          c.first_index, c.vertex_offset, c.first_instance);
      },
      [&](CmdDrawIndirect const & c) {
        t->CmdDrawIndirect(vk_command_buffer, c.buffer->vk_buffer, c.offset,
                           c.draw_count, c.stride);
      },
      [&](CmdDrawIndexedIndirect const & c) {
        t->CmdDrawIndexedIndirect(vk_command_buffer, c.buffer->vk_buffer,
                                  c.offset, c.draw_count, c.stride);
      });
  }

  t->CmdEndRenderingKHR(vk_command_buffer);
  clear_context();
}

void CommandEncoder::bind_compute_pipeline(gpu::ComputePipeline pipeline)
{
  ENCODE_PRELUDE();
  ComputePassContext & ctx = compute_ctx;

  CHECK(is_in_compute_pass(), "");

  state        = CommandEncoderState::ComputePass;
  ctx.pipeline = (ComputePipeline *) pipeline;

  dev->vk_table.CmdBindPipeline(vk_command_buffer,
                                VK_PIPELINE_BIND_POINT_COMPUTE,
                                ctx.pipeline->vk_pipeline);
}

void CommandEncoder::validate_render_pass_compatible(
  gpu::GraphicsPipeline pipeline_)
{
  RenderPassContext const & ctx      = render_ctx;
  GraphicsPipeline *        pipeline = (GraphicsPipeline *) pipeline_;

  CHECK(pipeline->color_fmts.size() == ctx.color_attachments.size(), "");
  CHECK(!(pipeline->depth_fmt.is_none() && ctx.depth_attachment.is_some()), "");
  CHECK(!(pipeline->stencil_fmt.is_none() && ctx.depth_attachment.is_some()),
        "");

  for (auto [pipeline_fmt, attachment] :
       zip(pipeline->color_fmts, ctx.color_attachments))
  {
    if (pipeline_fmt != gpu::Format::Undefined)
    {
      CHECK(attachment.view != nullptr, "");
      CHECK(pipeline_fmt == IMAGE_FROM_VIEW(attachment.view)->format, "");
      CHECK(pipeline->sample_count ==
              IMAGE_FROM_VIEW(attachment.view)->sample_count,
            "");
    }
  }

  ctx.depth_attachment.match([&](auto & attachment) {
    CHECK(attachment.view != nullptr, "");
    CHECK(pipeline->depth_fmt == IMAGE_FROM_VIEW(attachment.view)->format, "");
  });

  ctx.stencil_attachment.match([&](auto & attachment) {
    CHECK(attachment.view != nullptr, "");
    CHECK(pipeline->stencil_fmt == IMAGE_FROM_VIEW(attachment.view)->format,
          "");
  });
}

void CommandEncoder::bind_graphics_pipeline(gpu::GraphicsPipeline pipeline_)
{
  ENCODE_PRELUDE();
  RenderPassContext & ctx      = render_ctx;
  GraphicsPipeline *  pipeline = (GraphicsPipeline *) pipeline_;

  CHECK(is_in_render_pass(), "");
  CHECK(pipeline != nullptr, "");
  validate_render_pass_compatible(pipeline_);
  ctx.pipeline = pipeline;
  if (!ctx.commands.push(CmdBindGraphicsPipeline{.pipeline = pipeline}))
  {
    status = Status::OutOfHostMemory;
    return;
  }
}

void CommandEncoder::bind_descriptor_sets(
  Span<gpu::DescriptorSet const> descriptor_sets,
  Span<u32 const>                dynamic_offsets)
{
  ENCODE_PRELUDE();

  CHECK(is_in_pass(), "");
  CHECK(size32(descriptor_sets) <= gpu::MAX_PIPELINE_DESCRIPTOR_SETS, "");
  CHECK(size32(dynamic_offsets) <= (gpu::MAX_PIPELINE_DYNAMIC_STORAGE_BUFFERS +
                                    gpu::MAX_PIPELINE_DYNAMIC_UNIFORM_BUFFERS),
        "");

  for (u32 offset : dynamic_offsets)
  {
    CHECK(is_aligned<u64>(gpu::BUFFER_OFFSET_ALIGNMENT, offset), "");
  }

  if (is_in_compute_pass())
  {
    CHECK(compute_ctx.pipeline != nullptr, "");
    CHECK(compute_ctx.pipeline->num_sets == descriptor_sets.size(), "");

    compute_ctx.sets.clear();
    for (auto set : descriptor_sets)
    {
      compute_ctx.sets.push((DescriptorSet *) set).unwrap();
    }

    InplaceVec<VkDescriptorSet, gpu::MAX_PIPELINE_DESCRIPTOR_SETS> vk_sets;
    for (auto & set : descriptor_sets)
    {
      vk_sets.push(((DescriptorSet *) set)->vk_set).unwrap();
    }

    dev->vk_table.CmdBindDescriptorSets(
      vk_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE,
      compute_ctx.pipeline->vk_layout, 0, size32(vk_sets), vk_sets.data(),
      size32(dynamic_offsets), dynamic_offsets.data());
  }
  else if (is_in_render_pass())
  {
    CHECK(render_ctx.pipeline != nullptr, "");
    CHECK(render_ctx.pipeline->num_sets == descriptor_sets.size(), "");

    auto sets =
      PinVec<DescriptorSet *>::make(size(descriptor_sets), render_ctx.arg_pool)
        .unwrap();
    auto offsets =
      PinVec<u32>::make(size(dynamic_offsets), render_ctx.arg_pool).unwrap();

    sets.extend(descriptor_sets.reinterpret<DescriptorSet * const>()).unwrap();
    offsets.extend(dynamic_offsets).unwrap();

    if (!render_ctx.commands.push(CmdBindDescriptorSets{
          .sets = std::move(sets), .dynamic_offsets = std::move(offsets)}))
    {
      status = Status::OutOfHostMemory;
      return;
    }
  }
}

void CommandEncoder::push_constants(Span<u8 const> push_constants_data)
{
  ENCODE_PRELUDE();
  CHECK(push_constants_data.size_bytes() <= gpu::MAX_PUSH_CONSTANTS_SIZE, "");
  u32 const push_constants_size = (u32) push_constants_data.size_bytes();
  CHECK(is_aligned(4U, push_constants_size), "");
  CHECK(is_in_pass(), "");

  if (is_in_compute_pass())
  {
    CHECK(compute_ctx.pipeline != nullptr, "");
    CHECK(push_constants_size == compute_ctx.pipeline->push_constants_size, "");
    dev->vk_table.CmdPushConstants(
      vk_command_buffer, compute_ctx.pipeline->vk_layout, VK_SHADER_STAGE_ALL,
      0, compute_ctx.pipeline->push_constants_size, push_constants_data.data());
  }
  else if (is_in_render_pass())
  {
    // [ ] are the commands destroyed correctly?
    CHECK(render_ctx.pipeline != nullptr, "");
    CHECK(push_constants_size == render_ctx.pipeline->push_constants_size, "");

    auto constant =
      PinVec<u8>::make(push_constants_size, render_ctx.arg_pool).unwrap();
    constant.extend(push_constants_data).unwrap();

    if (!render_ctx.commands.push(
          CmdPushConstants{.constant = std::move(constant)}))
    {
      status = Status::OutOfHostMemory;
      return;
    }
  }
}

void CommandEncoder::dispatch(u32 group_count_x, u32 group_count_y,
                              u32 group_count_z)
{
  ENCODE_PRELUDE();
  ComputePassContext & ctx = compute_ctx;

  CHECK(is_in_compute_pass(), "");

  CHECK(ctx.pipeline != nullptr, "");
  CHECK(group_count_x <=
          dev->phy_dev.vk_properties.limits.maxComputeWorkGroupCount[0],
        "");
  CHECK(group_count_y <=
          dev->phy_dev.vk_properties.limits.maxComputeWorkGroupCount[1],
        "");
  CHECK(group_count_z <=
          dev->phy_dev.vk_properties.limits.maxComputeWorkGroupCount[2],
        "");

  for (auto set : ctx.sets)
  {
    access_compute_bindings(*set);
  }

  dev->vk_table.CmdDispatch(vk_command_buffer, group_count_x, group_count_y,
                            group_count_z);
}

void CommandEncoder::dispatch_indirect(gpu::Buffer buffer_, u64 offset)
{
  ENCODE_PRELUDE();
  ComputePassContext & ctx    = compute_ctx;
  auto * const         buffer = (Buffer *) buffer_;

  CHECK(is_in_compute_pass(), "");
  CHECK(ctx.pipeline != nullptr, "");
  CHECK(has_bits(buffer->usage, gpu::BufferUsage::IndirectBuffer), "");
  CHECK(is_valid_buffer_access(buffer->size, offset,
                               sizeof(gpu::DispatchCommand), 4),
        "");

  for (auto set : ctx.sets)
  {
    access_compute_bindings(*set);
  }

  dev->vk_table.CmdDispatchIndirect(vk_command_buffer, buffer->vk_buffer,
                                    offset);
}

void CommandEncoder::set_graphics_state(gpu::GraphicsState const & state)
{
  ENCODE_PRELUDE();
  RenderPassContext & ctx = render_ctx;

  CHECK(is_in_render_pass(), "");
  CHECK(state.viewport.min_depth >= 0.0F, "");
  CHECK(state.viewport.min_depth <= 1.0F, "");
  CHECK(state.viewport.max_depth >= 0.0F, "");
  CHECK(state.viewport.max_depth <= 1.0F, "");
  ctx.has_state = true;

  if (!ctx.commands.push(CmdSetGraphicsState{.state = state}))
  {
    status = Status::OutOfHostMemory;
    return;
  }
}

void CommandEncoder::bind_vertex_buffers(Span<gpu::Buffer const> vertex_buffers,
                                         Span<u64 const>         offsets)
{
  ENCODE_PRELUDE();
  RenderPassContext & ctx = render_ctx;

  CHECK(is_in_render_pass(), "");
  u32 const num_vertex_buffers = size32(vertex_buffers);
  CHECK(num_vertex_buffers > 0, "");
  CHECK(num_vertex_buffers <= gpu::MAX_VERTEX_ATTRIBUTES, "");
  CHECK(offsets.size() == vertex_buffers.size(), "");
  for (u32 i = 0; i < num_vertex_buffers; i++)
  {
    u64 const    offset = offsets[i];
    auto * const buffer = (Buffer *) vertex_buffers[i];
    CHECK(offset < buffer->size, "");
    CHECK(has_bits(buffer->usage, gpu::BufferUsage::VertexBuffer), "");
  }

  for (u32 i = 0; i < num_vertex_buffers; i++)
  {
    if (!ctx.commands.push(
          CmdBindVertexBuffer{.binding = i,
                              .buffer  = (Buffer *) vertex_buffers[i],
                              .offset  = offsets[i]}))
    {
      status = Status::OutOfHostMemory;
      return;
    }
  }
}

void CommandEncoder::bind_index_buffer(gpu::Buffer index_buffer_, u64 offset,
                                       gpu::IndexType index_type)
{
  ENCODE_PRELUDE();
  RenderPassContext & ctx          = render_ctx;
  auto * const        index_buffer = (Buffer *) index_buffer_;
  u64 const           index_size   = index_type_size(index_type);

  CHECK(is_in_render_pass(), "");
  CHECK(offset < index_buffer->size, "");
  CHECK(is_aligned(index_size, offset), "");
  CHECK(has_bits(index_buffer->usage, gpu::BufferUsage::IndexBuffer), "");

  ctx.index_buffer        = index_buffer;
  ctx.index_type          = index_type;
  ctx.index_buffer_offset = offset;
  if (!ctx.commands.push(CmdBindIndexBuffer{
        .buffer = index_buffer, .offset = offset, .index_type = index_type}))
  {
    status = Status::OutOfHostMemory;
    return;
  }
}

void CommandEncoder::draw(u32 vertex_count, u32 instance_count,
                          u32 first_vertex, u32 first_instance)
{
  ENCODE_PRELUDE();
  RenderPassContext & ctx = render_ctx;

  CHECK(is_in_render_pass(), "");
  CHECK(ctx.pipeline != nullptr, "");
  CHECK(ctx.has_state, "");

  if (!ctx.commands.push(CmdDraw{.vertex_count   = vertex_count,
                                 .instance_count = instance_count,
                                 .first_vertex   = first_vertex,
                                 .first_instance = first_instance}))
  {
    status = Status::OutOfHostMemory;
    return;
  }
}

void CommandEncoder::draw_indexed(u32 first_index, u32 num_indices,
                                  i32 vertex_offset, u32 first_instance,
                                  u32 num_instances)
{
  ENCODE_PRELUDE();
  RenderPassContext & ctx = render_ctx;

  CHECK(is_in_render_pass(), "");
  CHECK(ctx.pipeline != nullptr, "");
  CHECK(ctx.index_buffer != nullptr, "");
  u64 const index_size = index_type_size(ctx.index_type);
  CHECK((ctx.index_buffer_offset + first_index * index_size) <
          ctx.index_buffer->size,
        "");
  CHECK((ctx.index_buffer_offset + (first_index + num_indices) * index_size) <=
          ctx.index_buffer->size,
        "");
  CHECK(ctx.has_state, "");

  if (!ctx.commands.push(CmdDrawIndexed{.first_index    = first_index,
                                        .num_indices    = num_indices,
                                        .vertex_offset  = vertex_offset,
                                        .first_instance = first_instance,
                                        .num_instances  = num_instances}))
  {
    status = Status::OutOfHostMemory;
    return;
  }
}

void CommandEncoder::draw_indirect(gpu::Buffer buffer_, u64 offset,
                                   u32 draw_count, u32 stride)
{
  ENCODE_PRELUDE();
  RenderPassContext & ctx    = render_ctx;
  auto * const        buffer = (Buffer *) buffer_;

  CHECK(is_in_render_pass(), "");
  CHECK(ctx.pipeline != nullptr, "");
  CHECK(has_bits(buffer->usage, gpu::BufferUsage::IndirectBuffer), "");
  CHECK(offset < buffer->size, "");
  CHECK((offset + (u64) draw_count * stride) <= buffer->size, "");
  CHECK(is_aligned(4U, stride), "");
  CHECK(stride >= sizeof(gpu::DrawCommand), "");
  CHECK(ctx.has_state, "");

  if (!ctx.commands.push(CmdDrawIndirect{.buffer     = buffer,
                                         .offset     = offset,
                                         .draw_count = draw_count,
                                         .stride     = stride}))
  {
    status = Status::OutOfHostMemory;
    return;
  }
}

void CommandEncoder::draw_indexed_indirect(gpu::Buffer buffer_, u64 offset,
                                           u32 draw_count, u32 stride)
{
  ENCODE_PRELUDE();
  RenderPassContext & ctx    = render_ctx;
  auto * const        buffer = (Buffer *) buffer_;

  CHECK(is_in_render_pass(), "");
  CHECK(ctx.pipeline != nullptr, "");
  CHECK(ctx.index_buffer != nullptr, "");
  CHECK(has_bits(buffer->usage, gpu::BufferUsage::IndirectBuffer), "");
  CHECK(offset < buffer->size, "");
  CHECK((offset + (u64) draw_count * stride) <= buffer->size, "");
  CHECK(is_aligned(4U, stride), "");
  CHECK(stride >= sizeof(gpu::DrawIndexedCommand), "");
  CHECK(ctx.has_state, "");

  if (!ctx.commands.push(CmdDrawIndexedIndirect{.buffer     = buffer,
                                                .offset     = offset,
                                                .draw_count = draw_count,
                                                .stride     = stride}))
  {
    status = Status::OutOfHostMemory;
    return;
  }
}