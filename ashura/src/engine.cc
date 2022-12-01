#include "ashura/engine.h"

#include "ashura/canvas.h"
#include "ashura/sdl_utils.h"
#include "ashura/shaders.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace asr {

namespace impl {

// TODO(lamarrr): take a quick look at UE log file content and structure
static stx::Rc<spdlog::logger*> make_multi_threaded_logger(
    std::string name, std::string file_path) {
  stx::Vec<spdlog::sink_ptr> sinks{stx::os_allocator};
  /* sinks
       .push(std::make_shared<spdlog::sinks::basic_file_sink_mt>(
           std::move(file_path)))
       .unwrap();
 */
  sinks.push(std::make_shared<spdlog::sinks::stdout_color_sink_mt>()).unwrap();

  return stx::rc::make_inplace<spdlog::logger>(
             stx::os_allocator, std::move(name), sinks.begin(), sinks.end())
      .unwrap();
}
}  // namespace impl

inline stx::Option<stx::Span<vk::PhyDeviceInfo const>> select_device(
    stx::Span<vk::PhyDeviceInfo const> const phy_devices,
    stx::Span<VkPhysicalDeviceType const> preferred_device_types,
    vk::Surface const& target_surface) {
  for (VkPhysicalDeviceType type : preferred_device_types) {
    if (stx::Span selected =
            phy_devices.which([&](vk::PhyDeviceInfo const& dev) -> bool {
              return dev.properties.deviceType == type &&
                     // can use shaders (fragment and vertex)
                     dev.has_geometry_shader() &&
                     // has graphics command queue for rendering commands
                     dev.has_graphics_command_queue_family() &&
                     // has data transfer command queue for uploading textures
                     // or data
                     dev.has_transfer_command_queue_family() &&
                     // can be used for presenting to a specific surface
                     any_true(
                         vk::get_surface_presentation_command_queue_support(
                             dev.phy_device, dev.family_properties,
                             target_surface.surface));
            });
        !selected.is_empty()) {
      return stx::Some(std::move(selected));
    }
  }

  return stx::None;
}

Engine::Engine(AppConfig const& cfg) {
  stx::Vec<char const*> required_device_extensions{stx::os_allocator};

  required_device_extensions.push(VK_KHR_SWAPCHAIN_EXTENSION_NAME).unwrap();

  stx::Vec<char const*> required_validation_layers{stx::os_allocator};

  if (cfg.enable_validation_layers)
    required_validation_layers.push("VK_LAYER_KHRONOS_validation").unwrap();

  logger = stx::Some(
      impl::make_multi_threaded_logger("ashura", cfg.log_file.c_str()));

  auto& xlogger = *logger.value().handle;

  xlogger.info("Initializing Window API");

  window_api =
      stx::Some(stx::rc::make_inplace<WindowApi>(stx::os_allocator).unwrap());

  xlogger.info("Initialized Window API");
  xlogger.info("Creating root window");

  window = stx::Some(
      create_window(window_api.value().share(), cfg.window_config.copy()));

  xlogger.info("Created root window");

  stx::Vec window_required_instance_extensions =
      window.value().handle->get_required_instance_extensions();

  // TODO(lamarrr): check for validation layers requirement
  stx::Rc<vk::Instance*> vk_instance = vk::create_instance(
      cfg.name.c_str(), VK_MAKE_VERSION(0, 0, 1), cfg.name.c_str(),
      VK_MAKE_VERSION(cfg.version.major, cfg.version.minor, cfg.version.patch),
      window_required_instance_extensions, required_validation_layers);

  window.value().handle->attach_surface(vk_instance.share());

  stx::Vec phy_devices = vk::get_all_devices(vk_instance);

  VkPhysicalDeviceType const device_preference[] = {
      VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU,
      VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU,
      VK_PHYSICAL_DEVICE_TYPE_CPU};

  xlogger.info("Available Physical Devices:");

  for (vk::PhyDeviceInfo const& device : phy_devices) {
    xlogger.info("\t{}", vk::format(device));
    // TODO(lamarrr): log graphics families on devices and other properties
  }

  stx::Rc<vk::PhyDeviceInfo*> phy_device =
      stx::rc::make(
          stx::os_allocator,
          select_device(phy_devices, device_preference,
                        *window.value().handle->surface_.value().handle)
              .expect("Unable to find any suitable rendering device")[0]
              .copy())
          .unwrap();

  xlogger.info("Selected Physical Device: {}", vk::format(*phy_device.handle));

  // we might need multiple command queues, one for data transfer and one for
  // rendering
  f32 queue_priorities[] = {// priority for command queue used for
                            // presentation, rendering, data transfer
                            1.0f};

  stx::Rc graphics_command_queue_family =
      stx::rc::make(stx::os_allocator,
                    vk::get_graphics_command_queue(phy_device)
                        .expect("Unable to get graphics command queue"))
          .unwrap();

  // we can accept queue family struct here instead and thus not have to
  // perform extra manual checks
  // the user shouldn't have to touch handles
  VkDeviceQueueCreateInfo command_queue_create_infos[] = {
      VkDeviceQueueCreateInfo{
          .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
          .pNext = nullptr,
          .flags = 0,
          .queueFamilyIndex = graphics_command_queue_family.handle->index,
          .queueCount = AS_U32(std::size(queue_priorities)),
          .pQueuePriorities = queue_priorities}};

  VkPhysicalDeviceFeatures required_features{};

  required_features.samplerAnisotropy = VK_TRUE;

  stx::Rc<vk::Device*> device = vk::create_device(
      phy_device, command_queue_create_infos, required_device_extensions,
      required_validation_layers, required_features);

  stx::Rc<vk::CommandQueue*> xqueue =
      stx::rc::make_inplace<vk::CommandQueue>(
          stx::os_allocator,
          vk::get_command_queue(device, *graphics_command_queue_family.handle,
                                0)
              .expect("Failed to create graphics command queue"))
          .unwrap();

  queue = stx::Some(xqueue.share());

  window.value().handle->recreate_swapchain(xqueue);

  canvas_context = stx::Some(stx::rc::make_inplace<gfx::CanvasContext>(
                                 stx::os_allocator, xqueue.share())
                                 .unwrap());

  canvas_context.value().handle->recording_context.on_swapchain_changed(
      *window.value()
           .handle->surface_.value()
           .handle->swapchain.value()
           .handle);

  window.value().handle->on(
      WindowEvent::Resized,
      stx::fn::rc::make_unique_functor(stx::os_allocator, []() {
        ASR_LOG("resized");
      }).unwrap());

  window.value().handle->mouse_motion_listener =
      stx::fn::rc::make_unique_static(
          [](MouseMotionEvent const&) { ASR_LOG("mouse motion detected"); });

  u32 const transparent_image_data[1] = {0x00000000};
  auto transparent_image =
      vk::upload_rgba_image(xqueue, 1, 1, transparent_image_data);

  auto sampler = vk::create_image_sampler(transparent_image);

  canvas = stx::Some(gfx::Canvas{{0.0f, 0.0f}, sampler});

  VkDevice dev = queue.value().handle->device.handle->device;

  VkAttachmentDescription color_attachment{
      .flags = 0,
      .format = window.value()
                    .handle->surface_.value()
                    .handle->swapchain.value()
                    .handle->color_format.format,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR};

  VkAttachmentDescription attachments[] = {color_attachment};

  VkAttachmentReference color_attachment_reference{
      .attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

  VkSubpassDescription subpass{
      .flags = 0,
      .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
      .inputAttachmentCount = 0,
      .pInputAttachments = nullptr,
      .colorAttachmentCount = 1,
      .pColorAttachments = &color_attachment_reference,
      .pResolveAttachments = nullptr,
      .pDepthStencilAttachment = nullptr,
      .preserveAttachmentCount = 0,
      .pPreserveAttachments = nullptr};

  VkSubpassDependency dependency{
      .srcSubpass = VK_SUBPASS_EXTERNAL,
      .dstSubpass = 0,
      .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
      .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
      .srcAccessMask = 0,
      .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
      .dependencyFlags = 0};

  VkRenderPassCreateInfo render_pass_create_info{
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .attachmentCount = AS_U32(std::size(attachments)),
      .pAttachments = attachments,
      .subpassCount = 1,
      .pSubpasses = &subpass,
      .dependencyCount = 1,
      .pDependencies = &dependency};

  VkRenderPass render_pass;

  ASR_VK_CHECK(
      vkCreateRenderPass(dev, &render_pass_create_info, nullptr, &render_pass));

  VkFramebuffer framebuffer;

  auto [image, memory, view] = vk::create_image(
      queue.value(), 200, 200, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      window.value()
          .handle->surface_.value()
          .handle->swapchain.value()
          .handle->color_format.format);

  VkImageView framebuffer_attachments[] = {view};

  VkFramebufferCreateInfo create_info{
      .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .renderPass = render_pass,
      .attachmentCount = AS_U32(std::size(framebuffer_attachments)),
      .pAttachments = framebuffer_attachments,
      .width = 200,
      .height = 200,
      .layers = 1};

  ASR_VK_CHECK(vkCreateFramebuffer(dev, &create_info, nullptr, &framebuffer));

  VkShaderModule clip_vertex_shader;
  {
    VkShaderModuleCreateInfo create_info{
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .codeSize = sizeof(gfx::clip_vertex_shader_code),
        .pCode = gfx::clip_vertex_shader_code};

    ASR_VK_CHECK(
        vkCreateShaderModule(dev, &create_info, nullptr, &clip_vertex_shader));
  }

  VkShaderModule clip_fragment_shader;

  {
    VkShaderModuleCreateInfo create_info{
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .codeSize = sizeof(gfx::clip_fragment_shader_code),
        .pCode = gfx::clip_fragment_shader_code};

    ASR_VK_CHECK(vkCreateShaderModule(dev, &create_info, nullptr,
                                      &clip_fragment_shader));
  }

  VkPipelineShaderStageCreateInfo vert_shader_stage{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .stage = VK_SHADER_STAGE_VERTEX_BIT,
      .module = clip_vertex_shader,
      .pName = "main",
      .pSpecializationInfo = nullptr};

  VkPipelineShaderStageCreateInfo frag_shader_stage{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
      .module = clip_fragment_shader,
      .pName = "main",
      .pSpecializationInfo = nullptr};

  VkPipelineShaderStageCreateInfo stages[] = {vert_shader_stage,
                                              frag_shader_stage};

  VkPipelineLayout layout;

  VkPipelineLayoutCreateInfo layout_create_info{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .setLayoutCount = 0,
      .pSetLayouts = nullptr,
      .pushConstantRangeCount = 0,
      .pPushConstantRanges = nullptr};

  ASR_VK_CHECK(
      vkCreatePipelineLayout(dev, &layout_create_info, nullptr, &layout));

  VkPipelineColorBlendAttachmentState color_blend_attachment_states[] = {
      {.blendEnable = VK_FALSE,
       .srcColorBlendFactor = VK_BLEND_FACTOR_ZERO,
       .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
       .colorBlendOp = VK_BLEND_OP_ADD,
       .srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
       .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
       .alphaBlendOp = VK_BLEND_OP_ADD,
       .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                         VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT}};

  VkPipelineColorBlendStateCreateInfo color_blend_state{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .logicOpEnable = VK_FALSE,
      .logicOp = VK_LOGIC_OP_COPY,
      .attachmentCount = 1,
      .pAttachments = color_blend_attachment_states,
      .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f}};

  VkPipelineInputAssemblyStateCreateInfo input_assembly_state{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
      .primitiveRestartEnable = VK_FALSE};

  VkPipelineDepthStencilStateCreateInfo depth_stencil_state{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .depthTestEnable = VK_FALSE,
      .depthWriteEnable = VK_FALSE,
      .depthCompareOp = VK_COMPARE_OP_NEVER,
      .depthBoundsTestEnable = VK_FALSE,
      .stencilTestEnable = VK_FALSE,
      .front = VkStencilOpState{.failOp = VK_STENCIL_OP_KEEP,
                                .passOp = VK_STENCIL_OP_KEEP,
                                .depthFailOp = VK_STENCIL_OP_KEEP,
                                .compareOp = VK_COMPARE_OP_NEVER,
                                .compareMask = 0,
                                .writeMask = 0,
                                .reference = 0},
      .back = VkStencilOpState{.failOp = VK_STENCIL_OP_KEEP,
                               .passOp = VK_STENCIL_OP_KEEP,
                               .depthFailOp = VK_STENCIL_OP_KEEP,
                               .compareOp = VK_COMPARE_OP_NEVER,
                               .compareMask = 0,
                               .writeMask = 0,
                               .reference = 0},
      .minDepthBounds = 0.0f,
      .maxDepthBounds = 0.0f};

  VkPipelineMultisampleStateCreateInfo multisample_state{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
      .sampleShadingEnable = VK_FALSE,
      .minSampleShading = 0.0f,
      .pSampleMask = nullptr,
      .alphaToCoverageEnable = VK_FALSE,
      .alphaToOneEnable = VK_FALSE};

  VkPipelineRasterizationStateCreateInfo rasterization_state{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .depthClampEnable = VK_FALSE,
      .rasterizerDiscardEnable = VK_FALSE,
      .polygonMode = VK_POLYGON_MODE_FILL,
      .cullMode = VK_CULL_MODE_BACK_BIT,
      .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
      .depthBiasEnable = VK_FALSE,
      .depthBiasConstantFactor = 0.0f,
      .depthBiasClamp = 0.0f,
      .depthBiasSlopeFactor = 0.0f,
      .lineWidth = 1.0f};

  VkVertexInputBindingDescription vertex_binding_descriptions[] = {
      {.binding = 0,
       .stride = sizeof(vec2),
       .inputRate = VK_VERTEX_INPUT_RATE_VERTEX}};

  VkVertexInputAttributeDescription vertex_attribute_desc[] = {
      {.location = 0,
       .binding = 0,
       .format = VK_FORMAT_R32G32_SFLOAT,
       .offset = 0}};

  VkPipelineVertexInputStateCreateInfo vertex_input_state{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .vertexBindingDescriptionCount =
          AS_U32(std::size(vertex_binding_descriptions)),
      .pVertexBindingDescriptions = vertex_binding_descriptions,
      .vertexAttributeDescriptionCount =
          AS_U32(std::size(vertex_attribute_desc)),
      .pVertexAttributeDescriptions = vertex_attribute_desc};

  VkPipelineViewportStateCreateInfo viewport_state{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .viewportCount = 1,
      .pViewports = nullptr,
      .scissorCount = 1,
      .pScissors = nullptr};

  VkDynamicState dynamic_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR
                                     };

  VkPipelineDynamicStateCreateInfo dynamic_state{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .dynamicStateCount = AS_U32(std::size(dynamic_states)),
      .pDynamicStates = dynamic_states};

  VkGraphicsPipelineCreateInfo pipeline_create_info{
      .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .stageCount = AS_U32(std::size(stages)),
      .pStages = stages,
      .pVertexInputState = &vertex_input_state,
      .pInputAssemblyState = &input_assembly_state,
      .pTessellationState = nullptr,
      .pViewportState = &viewport_state,
      .pRasterizationState = &rasterization_state,
      .pMultisampleState = &multisample_state,
      .pDepthStencilState = &depth_stencil_state,
      .pColorBlendState = &color_blend_state,
      .pDynamicState = &dynamic_state,
      .layout = layout,
      .renderPass = render_pass,
      .subpass = 0,
      .basePipelineHandle = VK_NULL_HANDLE,
      .basePipelineIndex = 0};

  VkPipeline pipeline;

  ASR_VK_CHECK(vkCreateGraphicsPipelines(
      dev, VK_NULL_HANDLE, 1, &pipeline_create_info, nullptr, &pipeline));

  auto const& memory_properties = phy_device.handle->memory_properties;
  u32 queue_families[] = {queue.value().handle->info.family.index};

  auto create_buffer = [&queue_families, dev, &memory_properties](
                           usize size, void* data, VkBufferUsageFlags usage) {
    VkBuffer buffer;
    VkBufferCreateInfo create_info{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .size = size,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = std::size(queue_families),
        .pQueueFamilyIndices = queue_families};

    ASR_VK_CHECK(vkCreateBuffer(dev, &create_info, nullptr, &buffer));

    VkDeviceMemory memory;

    VkMemoryRequirements memory_requirements;

    vkGetBufferMemoryRequirements(dev, buffer, &memory_requirements);

    u32 memory_type_index =
        vk::find_suitable_memory_type(memory_properties, memory_requirements,
                                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
            .unwrap();

    VkMemoryAllocateInfo alloc_info{
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = nullptr,
        .allocationSize = memory_requirements.size,
        .memoryTypeIndex = memory_type_index};

    ASR_VK_CHECK(vkAllocateMemory(dev, &alloc_info, nullptr, &memory));

    ASR_VK_CHECK(vkBindBufferMemory(dev, buffer, memory, 0));

    void* memory_map;

    ASR_VK_CHECK(vkMapMemory(dev, memory, 0, VK_WHOLE_SIZE, 0, &memory_map));

    std::memcpy(memory_map, data, size);

    VkMappedMemoryRange range{
        .sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
        .pNext = nullptr,
        .memory = memory,
        .offset = 0,
        .size = VK_WHOLE_SIZE,
    };

    ASR_VK_CHECK(vkFlushMappedMemoryRanges(dev, 1, &range));

    return buffer;
  };

  vec2 vertices[] = {{0, 0}, {200, 0}, {100, 200}};

  VkBuffer vertex_buffer = create_buffer(sizeof(vertices), vertices,
                                         VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

  u32 indices[] = {0, 1, 2};

  VkBuffer index_buffer =
      create_buffer(sizeof(indices), indices, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

  VkCommandPoolCreateInfo command_pool_create_info{
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .pNext = nullptr,
      .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
      .queueFamilyIndex = queue_families[0]};

  VkCommandPool command_pool;

  ASR_VK_CHECK(vkCreateCommandPool(dev, &command_pool_create_info, nullptr,
                                   &command_pool));

  VkCommandBufferAllocateInfo command_buffer_allocate_info{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .pNext = nullptr,
      .commandPool = command_pool,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = 1};

  VkCommandBuffer command_buffer;

  ASR_VK_CHECK(vkAllocateCommandBuffers(dev, &command_buffer_allocate_info,
                                        &command_buffer));

  VkFence fence;

  VkFenceCreateInfo fence_create_info{
      .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
      .pNext = nullptr,
      .flags = VK_FENCE_CREATE_SIGNALED_BIT};

  ASR_VK_CHECK(vkCreateFence(dev, &fence_create_info, nullptr, &fence));

  while (true) {
    VkCommandBufferBeginInfo command_buffer_begin_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr};

    ASR_VK_CHECK(
        vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info));

    VkClearValue clear_values[] = {
        {.color = VkClearColorValue{{0.0f, 0.0f, 0.0f, 0.0f}}}};

    VkRenderPassBeginInfo render_pass_begin_info{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .pNext = nullptr,
        .renderPass = render_pass,
        .framebuffer = framebuffer,
        .renderArea = VkRect2D{.offset = {0, 0}, .extent = {200, 200}},
        .clearValueCount = AS_U32(std::size(clear_values)),
        .pClearValues = clear_values};

    vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info,
                         VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      pipeline);

    VkViewport viewport{.x = 0.0f,
                        .y = 0.0f,
                        .width = 200,
                        .height = 200,
                        .minDepth = 0.0f,
                        .maxDepth = 1.0f};

    vkCmdSetViewport(command_buffer, 0, 1, &viewport);

    VkRect2D scissor{.offset = {0, 0}, .extent = {200, 200}};

    vkCmdSetScissor(command_buffer, 0, 1, &scissor);

    VkDeviceSize offset = 0;

    vkCmdBindVertexBuffers(command_buffer, 0, 1, &vertex_buffer, &offset);

    vkCmdBindIndexBuffer(command_buffer, index_buffer, 0, VK_INDEX_TYPE_UINT32);

    vkCmdDrawIndexed(command_buffer, std ::size(indices), 1, 0, 0, 0);

    vkCmdEndRenderPass(command_buffer);

    ASR_VK_CHECK(vkEndCommandBuffer(command_buffer));

    VkPipelineStageFlags wait_stages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    VkSubmitInfo submit_info{.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                             .pNext = nullptr,
                             .waitSemaphoreCount = 0,
                             .pWaitSemaphores = nullptr,
                             .pWaitDstStageMask = wait_stages,
                             .commandBufferCount = 1,
                             .pCommandBuffers = &command_buffer,
                             .signalSemaphoreCount = 0,
                             .pSignalSemaphores = nullptr};

    ASR_VK_CHECK(vkResetFences(dev, 1, &fence));

    ASR_VK_CHECK(vkQueueSubmit(queue.value().handle->info.queue, 1,
                               &submit_info, fence));

    ASR_VK_CHECK(vkWaitForFences(dev, 1, &fence, VK_TRUE, stx::u64_max));

    ASR_VK_CHECK(vkResetCommandBuffer(command_buffer, 0));

    WindowSwapchainDiff swapchain_diff = WindowSwapchainDiff::None;

    do {
      if (swapchain_diff != WindowSwapchainDiff::None) {
        window.value().handle->recreate_swapchain(queue.value());
        canvas_context.value().handle->recording_context.on_swapchain_changed(
            *window.value()
                 .handle->surface_.value()
                 .handle->swapchain.value()
                 .handle);
      }

      auto [diff, next_swapchain_image_index] =
          window.value().handle->acquire_image();

      swapchain_diff = diff;

      if (swapchain_diff != WindowSwapchainDiff::None) {
        continue;
      }

      static constexpr u64 TIMEOUT = AS_U64(
          std::chrono::duration_cast<std::chrono::nanoseconds>(1min).count());

      swapchain_diff =
          window.value().handle->present(next_swapchain_image_index);

      vk::SwapChain& swapchain = *window.value()
                                      .handle->surface_.value()
                                      .handle->swapchain.value()
                                      .handle;
      // the frame semaphores and synchronization primitives are still used even
      // if an error is returned
      swapchain.next_frame_flight_index =
          (swapchain.next_frame_flight_index + 1) %
          vk::SwapChain::MAX_FRAMES_INFLIGHT;

    } while (swapchain_diff != WindowSwapchainDiff::None);

    // poll events to make the window not be marked as unresponsive.
    // we also poll events from SDL's event queue until there are none left.
    //
    // any missed event should be rolled over to the next tick()
    do {
    } while (window_api.value().handle->poll_events());
  }
};

void Engine::tick(std::chrono::nanoseconds interval) {
  window.value().handle->tick(interval);

  auto draw_content = [&]() {
    VkExtent2D window_extent = window.value()
                                   .handle->surface_.value()
                                   .handle->swapchain.value()
                                   .handle->window_extent;

    canvas.value().restart(
        vec2{AS_F32(window_extent.width), AS_F32(window_extent.height)});

    canvas.value().brush.color = colors::CYAN;
    canvas.value().clear();
  };

  draw_content();
  // only try to present if the pipeline has new changes or window was
  // resized

  // TODO(lamarrr): make presentation happen after recreation, for the first
  // iteration. and remove the created swapchain in the init method

  // only try to recreate swapchain if the present swapchain can't be used for
  // presentation

  WindowSwapchainDiff swapchain_diff = WindowSwapchainDiff::None;

  do {
    if (swapchain_diff != WindowSwapchainDiff::None) {
      window.value().handle->recreate_swapchain(queue.value());
      canvas_context.value().handle->recording_context.on_swapchain_changed(
          *window.value()
               .handle->surface_.value()
               .handle->swapchain.value()
               .handle);

      draw_content();
    }

    vk::SwapChain& swapchain = *window.value()
                                    .handle->surface_.value()
                                    .handle->swapchain.value()
                                    .handle;

    ASR_VK_CHECK(vkResetFences(
        swapchain.queue.handle->device.handle->device, 1,
        &swapchain
             .image_acquisition_fences[swapchain.next_frame_flight_index]));

    auto [diff, next_swapchain_image_index] =
        window.value().handle->acquire_image();

    swapchain_diff = diff;

    if (swapchain_diff != WindowSwapchainDiff::None) {
      continue;
    }

    static constexpr u64 TIMEOUT = AS_U64(
        std::chrono::duration_cast<std::chrono::nanoseconds>(1min).count());

    ASR_VK_CHECK(vkWaitForFences(
        swapchain.queue.handle->device.handle->device, 1,
        &swapchain.image_acquisition_fences[swapchain.next_frame_flight_index],
        VK_TRUE, TIMEOUT));

    ASR_VK_CHECK(vkResetFences(
        swapchain.queue.handle->device.handle->device, 1,
        &swapchain
             .image_acquisition_fences[swapchain.next_frame_flight_index]));

    canvas_context.value().handle->submit(*window.value()
                                               .handle->surface_.value()
                                               .handle->swapchain.value()
                                               .handle,
                                          next_swapchain_image_index,
                                          canvas.value().draw_list);

    swapchain_diff = window.value().handle->present(next_swapchain_image_index);

    // the frame semaphores and synchronization primitives are still used even
    // if an error is returned
    swapchain.next_frame_flight_index =
        (swapchain.next_frame_flight_index + 1) %
        vk::SwapChain::MAX_FRAMES_INFLIGHT;

  } while (swapchain_diff != WindowSwapchainDiff::None);

  // poll events to make the window not be marked as unresponsive.
  // we also poll events from SDL's event queue until there are none left.
  //
  // any missed event should be rolled over to the next tick()
  do {
  } while (window_api.value().handle->poll_events());

  //   pipeline->dispatch_events(
  //   window->handle.handle->event_queue.mouse_button_events,
  //   window->handle.handle->event_queue.window_events);

  //   bool window_extent_changed =
  //   any_eq(window.value().handle->, WindowEvent::SizeChanged);

  // actually forward
  //   if (any_eq(window->handle.handle->event_queue.window_events,
  //  WindowEvent::Close)) {
  // std::exit(0);
  //   }

  // TODO(lamarrr): ???
  //   window->handle.handle->event_queue.clear();
}

}  // namespace asr
