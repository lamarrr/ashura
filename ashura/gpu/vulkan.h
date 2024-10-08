/// SPDX-License-Identifier: MIT
#pragma once
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
#define VMA_VULKAN_VERSION 1000000

#include "ashura/gpu/gpu.h"
#include "ashura/std/allocator.h"
#include "ashura/std/allocators.h"
#include "ashura/std/vec.h"
#include "vk_mem_alloc.h"
#include "vulkan/vk_enum_string_helper.h"
#include "vulkan/vulkan.h"

namespace ash
{
namespace vk
{

using gpu::Status;

constexpr char const *ENGINE_NAME    = "Ash";
constexpr u32         ENGINE_VERSION = VK_MAKE_API_VERSION(0, 0, 0, 1);
constexpr char const *CLIENT_NAME    = "Ash Client";
constexpr u32         CLIENT_VERSION = VK_MAKE_API_VERSION(0, 0, 0, 1);

constexpr u32 MAX_MEMORY_HEAP_PROPERTIES = 32;
constexpr u32 MAX_MEMORY_HEAPS           = 16;
constexpr u8  NUM_DESCRIPTOR_TYPES       = 11;

struct InstanceInterface
{
  static void destroy(gpu::Instance self);
  static Result<gpu::DeviceImpl, Status>
                      create_device(gpu::Instance self, AllocatorImpl allocator,
                                    Span<gpu::DeviceType const> preferred_types,
                                    Span<gpu::Surface const>    compatible_surfaces,
                                    u32                         buffering);
  static gpu::Backend get_backend(gpu::Instance self);
  static void         destroy_device(gpu::Instance self, gpu::Device device);
  static void         destroy_surface(gpu::Instance self, gpu::Surface surface);
};

struct DeviceInterface
{
  static gpu::DeviceProperties get_device_properties(gpu::Device self);
  static Result<gpu::FormatProperties, Status>
      get_format_properties(gpu::Device self, gpu::Format format);
  static Result<gpu::Buffer, Status> create_buffer(gpu::Device            self,
                                                   gpu::BufferDesc const &desc);
  static Result<gpu::BufferView, Status>
      create_buffer_view(gpu::Device self, gpu::BufferViewDesc const &desc);
  static Result<gpu::Image, Status> create_image(gpu::Device           self,
                                                 gpu::ImageDesc const &desc);
  static Result<gpu::ImageView, Status>
      create_image_view(gpu::Device self, gpu::ImageViewDesc const &desc);
  static Result<gpu::Sampler, Status>
      create_sampler(gpu::Device self, gpu::SamplerDesc const &desc);
  static Result<gpu::Shader, Status> create_shader(gpu::Device            self,
                                                   gpu::ShaderDesc const &desc);
  static Result<gpu::DescriptorSetLayout, Status>
      create_descriptor_set_layout(gpu::Device                         self,
                                   gpu::DescriptorSetLayoutDesc const &desc);
  static Result<gpu::DescriptorSet, Status>
      create_descriptor_set(gpu::Device self, gpu::DescriptorSetLayout layout,
                            Span<u32 const> variable_lengths);
  static Result<gpu::PipelineCache, Status>
      create_pipeline_cache(gpu::Device                   self,
                            gpu::PipelineCacheDesc const &desc);
  static Result<gpu::ComputePipeline, Status>
      create_compute_pipeline(gpu::Device                     self,
                              gpu::ComputePipelineDesc const &desc);
  static Result<gpu::GraphicsPipeline, Status>
      create_graphics_pipeline(gpu::Device                      self,
                               gpu::GraphicsPipelineDesc const &desc);
  static Result<gpu::Swapchain, Status>
      create_swapchain(gpu::Device self, gpu::Surface surface,
                       gpu::SwapchainDesc const &desc);
  static Result<gpu::TimeStampQuery, Status>
      create_timestamp_query(gpu::Device self);
  static Result<gpu::StatisticsQuery, Status>
              create_statistics_query(gpu::Device self);
  static void destroy_buffer(gpu::Device self, gpu::Buffer buffer);
  static void destroy_buffer_view(gpu::Device     self,
                                  gpu::BufferView buffer_view);
  static void destroy_image(gpu::Device self, gpu::Image image);
  static void destroy_image_view(gpu::Device self, gpu::ImageView image_view);
  static void destroy_sampler(gpu::Device self, gpu::Sampler sampler);
  static void destroy_shader(gpu::Device self, gpu::Shader shader);
  static void destroy_descriptor_set_layout(gpu::Device              self,
                                            gpu::DescriptorSetLayout layout);
  static void destroy_descriptor_set(gpu::Device self, gpu::DescriptorSet set);
  static void destroy_pipeline_cache(gpu::Device        self,
                                     gpu::PipelineCache cache);
  static void destroy_compute_pipeline(gpu::Device          self,
                                       gpu::ComputePipeline pipeline);
  static void destroy_graphics_pipeline(gpu::Device           self,
                                        gpu::GraphicsPipeline pipeline);
  static void destroy_swapchain(gpu::Device self, gpu::Swapchain swapchain);
  static void destroy_timestamp_query(gpu::Device         self,
                                      gpu::TimeStampQuery query);
  static void destroy_statistics_query(gpu::Device          self,
                                       gpu::StatisticsQuery query);
  static gpu::FrameContext      get_frame_context(gpu::Device self);
  static Result<void *, Status> map_buffer_memory(gpu::Device self,
                                                  gpu::Buffer buffer);
  static void unmap_buffer_memory(gpu::Device self, gpu::Buffer buffer);
  static Result<Void, Status>
      invalidate_mapped_buffer_memory(gpu::Device self, gpu::Buffer buffer,
                                      gpu::MemoryRange range);
  static Result<Void, Status>
      flush_mapped_buffer_memory(gpu::Device self, gpu::Buffer buffer,
                                 gpu::MemoryRange range);
  static Result<usize, Status>
      get_pipeline_cache_size(gpu::Device self, gpu::PipelineCache cache);
  static Result<usize, Status> get_pipeline_cache_data(gpu::Device        self,
                                                       gpu::PipelineCache cache,
                                                       Span<u8>           out);
  static Result<Void, Status>
              merge_pipeline_cache(gpu::Device self, gpu::PipelineCache dst,
                                   Span<gpu::PipelineCache const> srcs);
  static void update_descriptor_set(gpu::Device                     self,
                                    gpu::DescriptorSetUpdate const &update);
  static Result<Void, Status> wait_idle(gpu::Device self);
  static Result<Void, Status> wait_queue_idle(gpu::Device self);
  static Result<u32, Status>
      get_surface_formats(gpu::Device self, gpu::Surface surface,
                          Span<gpu::SurfaceFormat> formats);
  static Result<u32, Status>
      get_surface_present_modes(gpu::Device self, gpu::Surface surface,
                                Span<gpu::PresentMode> modes);
  static Result<gpu::SurfaceCapabilities, Status>
      get_surface_capabilities(gpu::Device self, gpu::Surface surface);
  static Result<gpu::SwapchainState, Status>
      get_swapchain_state(gpu::Device self, gpu::Swapchain swapchain);
  static Result<Void, Status>
      invalidate_swapchain(gpu::Device self, gpu::Swapchain swapchain,
                           gpu::SwapchainDesc const &desc);
  static Result<Void, Status> begin_frame(gpu::Device    self,
                                          gpu::Swapchain swapchain);
  static Result<Void, Status> submit_frame(gpu::Device    self,
                                           gpu::Swapchain swapchain);
  static Result<u64, Status>
      get_timestamp_query_result(gpu::Device self, gpu::TimeStampQuery query);
  static Result<gpu::PipelineStatistics, Status>
      get_statistics_query_result(gpu::Device self, gpu::StatisticsQuery query);
};

struct CommandEncoderInterface
{
  static void reset_timestamp_query(gpu::CommandEncoder self,
                                    gpu::TimeStampQuery query);
  static void reset_statistics_query(gpu::CommandEncoder  self,
                                     gpu::StatisticsQuery query);
  static void write_timestamp(gpu::CommandEncoder self,
                              gpu::TimeStampQuery query);
  static void begin_statistics(gpu::CommandEncoder  self,
                               gpu::StatisticsQuery query);
  static void end_statistics(gpu::CommandEncoder  self,
                             gpu::StatisticsQuery query);
  static void begin_debug_marker(gpu::CommandEncoder self,
                                 Span<char const> region_name, Vec4 color);
  static void end_debug_marker(gpu::CommandEncoder self);
  static void fill_buffer(gpu::CommandEncoder self, gpu::Buffer dst, u64 offset,
                          u64 size, u32 data);
  static void copy_buffer(gpu::CommandEncoder self, gpu::Buffer src,
                          gpu::Buffer dst, Span<gpu::BufferCopy const> copies);
  static void update_buffer(gpu::CommandEncoder self, Span<u8 const> src,
                            u64 dst_offset, gpu::Buffer dst);
  static void clear_color_image(gpu::CommandEncoder self, gpu::Image dst,
                                gpu::Color clear_color,
                                Span<gpu::ImageSubresourceRange const> ranges);
  static void
      clear_depth_stencil_image(gpu::CommandEncoder self, gpu::Image dst,
                                gpu::DepthStencil clear_depth_stencil,
                                Span<gpu::ImageSubresourceRange const> ranges);
  static void copy_image(gpu::CommandEncoder self, gpu::Image src,
                         gpu::Image dst, Span<gpu::ImageCopy const> copies);
  static void copy_buffer_to_image(gpu::CommandEncoder self, gpu::Buffer src,
                                   gpu::Image                       dst,
                                   Span<gpu::BufferImageCopy const> copies);
  static void blit_image(gpu::CommandEncoder self, gpu::Image src,
                         gpu::Image dst, Span<gpu::ImageBlit const> blits,
                         gpu::Filter filter);
  static void resolve_image(gpu::CommandEncoder self, gpu::Image src,
                            gpu::Image                    dst,
                            Span<gpu::ImageResolve const> resolves);
  static void begin_compute_pass(gpu::CommandEncoder self);
  static void end_compute_pass(gpu::CommandEncoder self);
  static void begin_rendering(gpu::CommandEncoder       self,
                              gpu::RenderingInfo const &info);
  static void end_rendering(gpu::CommandEncoder self);
  static void bind_compute_pipeline(gpu::CommandEncoder  self,
                                    gpu::ComputePipeline pipeline);
  static void bind_graphics_pipeline(gpu::CommandEncoder   self,
                                     gpu::GraphicsPipeline pipeline);
  static void
              bind_descriptor_sets(gpu::CommandEncoder            self,
                                   Span<gpu::DescriptorSet const> descriptor_sets,
                                   Span<u32 const>                dynamic_offsets);
  static void push_constants(gpu::CommandEncoder self,
                             Span<u8 const>      push_constants_data);
  static void dispatch(gpu::CommandEncoder self, u32 group_count_x,
                       u32 group_count_y, u32 group_count_z);
  static void dispatch_indirect(gpu::CommandEncoder self, gpu::Buffer buffer,
                                u64 offset);
  static void set_graphics_state(gpu::CommandEncoder       self,
                                 gpu::GraphicsState const &state);
  static void bind_vertex_buffers(gpu::CommandEncoder     self,
                                  Span<gpu::Buffer const> vertex_buffers,
                                  Span<u64 const>         offsets);
  static void bind_index_buffer(gpu::CommandEncoder self,
                                gpu::Buffer index_buffer, u64 offset,
                                gpu::IndexType index_type);
  static void draw(gpu::CommandEncoder self, u32 vertex_count,
                   u32 instance_count, u32 first_vertex_id,
                   u32 first_instance_id);
  static void draw_indexed(gpu::CommandEncoder self, u32 first_index,
                           u32 num_indices, i32 vertex_offset,
                           u32 first_instance_id, u32 num_instances);
  static void draw_indirect(gpu::CommandEncoder self, gpu::Buffer buffer,
                            u64 offset, u32 draw_count, u32 stride);
  static void draw_indexed_indirect(gpu::CommandEncoder self,
                                    gpu::Buffer buffer, u64 offset,
                                    u32 draw_count, u32 stride);
};

static gpu::InstanceInterface const instance_interface{
    .destroy         = InstanceInterface::destroy,
    .create_device   = InstanceInterface::create_device,
    .get_backend     = InstanceInterface::get_backend,
    .destroy_device  = InstanceInterface::destroy_device,
    .destroy_surface = InstanceInterface::destroy_surface};

static gpu::DeviceInterface const device_interface{
    .get_device_properties = DeviceInterface::get_device_properties,
    .get_format_properties = DeviceInterface::get_format_properties,
    .create_buffer         = DeviceInterface::create_buffer,
    .create_buffer_view    = DeviceInterface::create_buffer_view,
    .create_image          = DeviceInterface::create_image,
    .create_image_view     = DeviceInterface::create_image_view,
    .create_sampler        = DeviceInterface::create_sampler,
    .create_shader         = DeviceInterface::create_shader,
    .create_descriptor_set_layout =
        DeviceInterface::create_descriptor_set_layout,
    .create_descriptor_set    = DeviceInterface::create_descriptor_set,
    .create_pipeline_cache    = DeviceInterface::create_pipeline_cache,
    .create_compute_pipeline  = DeviceInterface::create_compute_pipeline,
    .create_graphics_pipeline = DeviceInterface::create_graphics_pipeline,
    .create_swapchain         = DeviceInterface::create_swapchain,
    .create_timestamp_query   = DeviceInterface::create_timestamp_query,
    .create_statistics_query  = DeviceInterface::create_statistics_query,
    .destroy_buffer           = DeviceInterface::destroy_buffer,
    .destroy_buffer_view      = DeviceInterface::destroy_buffer_view,
    .destroy_image            = DeviceInterface::destroy_image,
    .destroy_image_view       = DeviceInterface::destroy_image_view,
    .destroy_sampler          = DeviceInterface::destroy_sampler,
    .destroy_shader           = DeviceInterface::destroy_shader,
    .destroy_descriptor_set_layout =
        DeviceInterface::destroy_descriptor_set_layout,
    .destroy_descriptor_set    = DeviceInterface::destroy_descriptor_set,
    .destroy_pipeline_cache    = DeviceInterface::destroy_pipeline_cache,
    .destroy_compute_pipeline  = DeviceInterface::destroy_compute_pipeline,
    .destroy_graphics_pipeline = DeviceInterface::destroy_graphics_pipeline,
    .destroy_swapchain         = DeviceInterface::destroy_swapchain,
    .destroy_timestamp_query   = DeviceInterface::destroy_timestamp_query,
    .destroy_statistics_query  = DeviceInterface::destroy_statistics_query,
    .get_frame_context         = DeviceInterface::get_frame_context,
    .map_buffer_memory         = DeviceInterface::map_buffer_memory,
    .unmap_buffer_memory       = DeviceInterface::unmap_buffer_memory,
    .invalidate_mapped_buffer_memory =
        DeviceInterface::invalidate_mapped_buffer_memory,
    .flush_mapped_buffer_memory = DeviceInterface::flush_mapped_buffer_memory,
    .get_pipeline_cache_size    = DeviceInterface::get_pipeline_cache_size,
    .get_pipeline_cache_data    = DeviceInterface::get_pipeline_cache_data,
    .merge_pipeline_cache       = DeviceInterface::merge_pipeline_cache,
    .update_descriptor_set      = DeviceInterface::update_descriptor_set,
    .wait_idle                  = DeviceInterface::wait_idle,
    .wait_queue_idle            = DeviceInterface::wait_queue_idle,
    .get_surface_formats        = DeviceInterface::get_surface_formats,
    .get_surface_present_modes  = DeviceInterface::get_surface_present_modes,
    .get_surface_capabilities   = DeviceInterface::get_surface_capabilities,
    .get_swapchain_state        = DeviceInterface::get_swapchain_state,
    .invalidate_swapchain       = DeviceInterface::invalidate_swapchain,
    .begin_frame                = DeviceInterface::begin_frame,
    .submit_frame               = DeviceInterface::submit_frame,
    .get_timestamp_query_result = DeviceInterface::get_timestamp_query_result,
    .get_statistics_query_result =
        DeviceInterface::get_statistics_query_result};

static gpu::CommandEncoderInterface const command_encoder_interface{
    .write_timestamp    = CommandEncoderInterface::write_timestamp,
    .begin_statistics   = CommandEncoderInterface::begin_statistics,
    .end_statistics     = CommandEncoderInterface::end_statistics,
    .begin_debug_marker = CommandEncoderInterface::begin_debug_marker,
    .end_debug_marker   = CommandEncoderInterface::end_debug_marker,
    .fill_buffer        = CommandEncoderInterface::fill_buffer,
    .copy_buffer        = CommandEncoderInterface::copy_buffer,
    .update_buffer      = CommandEncoderInterface::update_buffer,
    .clear_color_image  = CommandEncoderInterface::clear_color_image,
    .clear_depth_stencil_image =
        CommandEncoderInterface::clear_depth_stencil_image,
    .copy_image             = CommandEncoderInterface::copy_image,
    .copy_buffer_to_image   = CommandEncoderInterface::copy_buffer_to_image,
    .blit_image             = CommandEncoderInterface::blit_image,
    .resolve_image          = CommandEncoderInterface::resolve_image,
    .begin_compute_pass     = CommandEncoderInterface::begin_compute_pass,
    .end_compute_pass       = CommandEncoderInterface::end_compute_pass,
    .begin_rendering        = CommandEncoderInterface::begin_rendering,
    .end_rendering          = CommandEncoderInterface::end_rendering,
    .bind_compute_pipeline  = CommandEncoderInterface::bind_compute_pipeline,
    .bind_graphics_pipeline = CommandEncoderInterface::bind_graphics_pipeline,
    .bind_descriptor_sets   = CommandEncoderInterface::bind_descriptor_sets,
    .push_constants         = CommandEncoderInterface::push_constants,
    .dispatch               = CommandEncoderInterface::dispatch,
    .dispatch_indirect      = CommandEncoderInterface::dispatch_indirect,
    .set_graphics_state     = CommandEncoderInterface::set_graphics_state,
    .bind_vertex_buffers    = CommandEncoderInterface::bind_vertex_buffers,
    .bind_index_buffer      = CommandEncoderInterface::bind_index_buffer,
    .draw                   = CommandEncoderInterface::draw,
    .draw_indexed           = CommandEncoderInterface::draw_indexed,
    .draw_indirect          = CommandEncoderInterface::draw_indirect,
    .draw_indexed_indirect  = CommandEncoderInterface::draw_indexed_indirect};

typedef VkSampler       Sampler;
typedef VkShaderModule  Shader;
typedef VkPipelineCache PipelineCache;
typedef VkSurfaceKHR    Surface;
typedef VkQueryPool     TimestampQuery;
typedef VkQueryPool     StatisticsQuery;
typedef struct Device   Device;

struct InstanceTable
{
  PFN_vkCreateInstance           CreateInstance           = nullptr;
  PFN_vkDestroyInstance          DestroyInstance          = nullptr;
  PFN_vkDestroySurfaceKHR        DestroySurfaceKHR        = nullptr;
  PFN_vkEnumeratePhysicalDevices EnumeratePhysicalDevices = nullptr;
  PFN_vkGetInstanceProcAddr      GetInstanceProcAddr      = nullptr;
  PFN_vkGetDeviceProcAddr        GetDeviceProcAddr        = nullptr;

  PFN_vkCreateDevice                       CreateDevice = nullptr;
  PFN_vkEnumerateDeviceExtensionProperties EnumerateDeviceExtensionProperties =
      nullptr;
  PFN_vkEnumerateDeviceLayerProperties EnumerateDeviceLayerProperties = nullptr;
  PFN_vkGetPhysicalDeviceFeatures      GetPhysicalDeviceFeatures      = nullptr;
  PFN_vkGetPhysicalDeviceFormatProperties GetPhysicalDeviceFormatProperties =
      nullptr;
  PFN_vkGetPhysicalDeviceImageFormatProperties
      GetPhysicalDeviceImageFormatProperties = nullptr;
  PFN_vkGetPhysicalDeviceMemoryProperties GetPhysicalDeviceMemoryProperties =
      nullptr;
  PFN_vkGetPhysicalDeviceProperties GetPhysicalDeviceProperties = nullptr;
  PFN_vkGetPhysicalDeviceQueueFamilyProperties
      GetPhysicalDeviceQueueFamilyProperties = nullptr;
  PFN_vkGetPhysicalDeviceSparseImageFormatProperties
      GetPhysicalDeviceSparseImageFormatProperties = nullptr;

  PFN_vkGetPhysicalDeviceSurfaceSupportKHR GetPhysicalDeviceSurfaceSupportKHR =
      nullptr;
  PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR
      GetPhysicalDeviceSurfaceCapabilitiesKHR = nullptr;
  PFN_vkGetPhysicalDeviceSurfaceFormatsKHR GetPhysicalDeviceSurfaceFormatsKHR =
      nullptr;
  PFN_vkGetPhysicalDeviceSurfacePresentModesKHR
      GetPhysicalDeviceSurfacePresentModesKHR = nullptr;

  PFN_vkCreateDebugUtilsMessengerEXT  CreateDebugUtilsMessengerEXT  = nullptr;
  PFN_vkDestroyDebugUtilsMessengerEXT DestroyDebugUtilsMessengerEXT = nullptr;
  PFN_vkSetDebugUtilsObjectNameEXT    SetDebugUtilsObjectNameEXT    = nullptr;
};

struct DeviceTable
{
  // DEVICE OBJECT FUNCTIONS
  PFN_vkAllocateCommandBuffers       AllocateCommandBuffers       = nullptr;
  PFN_vkAllocateDescriptorSets       AllocateDescriptorSets       = nullptr;
  PFN_vkAllocateMemory               AllocateMemory               = nullptr;
  PFN_vkBindBufferMemory             BindBufferMemory             = nullptr;
  PFN_vkBindImageMemory              BindImageMemory              = nullptr;
  PFN_vkCreateBuffer                 CreateBuffer                 = nullptr;
  PFN_vkCreateBufferView             CreateBufferView             = nullptr;
  PFN_vkCreateCommandPool            CreateCommandPool            = nullptr;
  PFN_vkCreateComputePipelines       CreateComputePipelines       = nullptr;
  PFN_vkCreateDescriptorPool         CreateDescriptorPool         = nullptr;
  PFN_vkCreateDescriptorSetLayout    CreateDescriptorSetLayout    = nullptr;
  PFN_vkCreateEvent                  CreateEvent                  = nullptr;
  PFN_vkCreateFence                  CreateFence                  = nullptr;
  PFN_vkCreateGraphicsPipelines      CreateGraphicsPipelines      = nullptr;
  PFN_vkCreateImage                  CreateImage                  = nullptr;
  PFN_vkCreateImageView              CreateImageView              = nullptr;
  PFN_vkCreatePipelineCache          CreatePipelineCache          = nullptr;
  PFN_vkCreatePipelineLayout         CreatePipelineLayout         = nullptr;
  PFN_vkCreateQueryPool              CreateQueryPool              = nullptr;
  PFN_vkCreateSampler                CreateSampler                = nullptr;
  PFN_vkCreateSemaphore              CreateSemaphore              = nullptr;
  PFN_vkCreateShaderModule           CreateShaderModule           = nullptr;
  PFN_vkDestroyBuffer                DestroyBuffer                = nullptr;
  PFN_vkDestroyBufferView            DestroyBufferView            = nullptr;
  PFN_vkDestroyCommandPool           DestroyCommandPool           = nullptr;
  PFN_vkDestroyDescriptorPool        DestroyDescriptorPool        = nullptr;
  PFN_vkDestroyDescriptorSetLayout   DestroyDescriptorSetLayout   = nullptr;
  PFN_vkDestroyDevice                DestroyDevice                = nullptr;
  PFN_vkDestroyEvent                 DestroyEvent                 = nullptr;
  PFN_vkDestroyFence                 DestroyFence                 = nullptr;
  PFN_vkDestroyImage                 DestroyImage                 = nullptr;
  PFN_vkDestroyImageView             DestroyImageView             = nullptr;
  PFN_vkDestroyPipeline              DestroyPipeline              = nullptr;
  PFN_vkDestroyPipelineCache         DestroyPipelineCache         = nullptr;
  PFN_vkDestroyPipelineLayout        DestroyPipelineLayout        = nullptr;
  PFN_vkDestroyQueryPool             DestroyQueryPool             = nullptr;
  PFN_vkDestroySampler               DestroySampler               = nullptr;
  PFN_vkDestroySemaphore             DestroySemaphore             = nullptr;
  PFN_vkDestroyShaderModule          DestroyShaderModule          = nullptr;
  PFN_vkDeviceWaitIdle               DeviceWaitIdle               = nullptr;
  PFN_vkFlushMappedMemoryRanges      FlushMappedMemoryRanges      = nullptr;
  PFN_vkFreeCommandBuffers           FreeCommandBuffers           = nullptr;
  PFN_vkFreeDescriptorSets           FreeDescriptorSets           = nullptr;
  PFN_vkFreeMemory                   FreeMemory                   = nullptr;
  PFN_vkGetBufferMemoryRequirements  GetBufferMemoryRequirements  = nullptr;
  PFN_vkGetDeviceMemoryCommitment    GetDeviceMemoryCommitment    = nullptr;
  PFN_vkGetDeviceQueue               GetDeviceQueue               = nullptr;
  PFN_vkGetEventStatus               GetEventStatus               = nullptr;
  PFN_vkGetFenceStatus               GetFenceStatus               = nullptr;
  PFN_vkGetImageMemoryRequirements   GetImageMemoryRequirements   = nullptr;
  PFN_vkGetImageSubresourceLayout    GetImageSubresourceLayout    = nullptr;
  PFN_vkGetPipelineCacheData         GetPipelineCacheData         = nullptr;
  PFN_vkGetQueryPoolResults          GetQueryPoolResults          = nullptr;
  PFN_vkInvalidateMappedMemoryRanges InvalidateMappedMemoryRanges = nullptr;
  PFN_vkMapMemory                    MapMemory                    = nullptr;
  PFN_vkMergePipelineCaches          MergePipelineCaches          = nullptr;
  PFN_vkResetCommandPool             ResetCommandPool             = nullptr;
  PFN_vkResetDescriptorPool          ResetDescriptorPool          = nullptr;
  PFN_vkResetEvent                   ResetEvent                   = nullptr;
  PFN_vkResetFences                  ResetFences                  = nullptr;
  PFN_vkSetEvent                     SetEvent                     = nullptr;
  PFN_vkUpdateDescriptorSets         UpdateDescriptorSets         = nullptr;
  PFN_vkUnmapMemory                  UnmapMemory                  = nullptr;
  PFN_vkWaitForFences                WaitForFences                = nullptr;

  PFN_vkQueueSubmit   QueueSubmit   = nullptr;
  PFN_vkQueueWaitIdle QueueWaitIdle = nullptr;

  // COMMAND BUFFER OBJECT FUNCTIONS
  PFN_vkBeginCommandBuffer        BeginCommandBuffer        = nullptr;
  PFN_vkCmdBeginQuery             CmdBeginQuery             = nullptr;
  PFN_vkCmdBindDescriptorSets     CmdBindDescriptorSets     = nullptr;
  PFN_vkCmdBindIndexBuffer        CmdBindIndexBuffer        = nullptr;
  PFN_vkCmdBindPipeline           CmdBindPipeline           = nullptr;
  PFN_vkCmdBindVertexBuffers      CmdBindVertexBuffers      = nullptr;
  PFN_vkCmdBlitImage              CmdBlitImage              = nullptr;
  PFN_vkCmdClearAttachments       CmdClearAttachments       = nullptr;
  PFN_vkCmdClearColorImage        CmdClearColorImage        = nullptr;
  PFN_vkCmdClearDepthStencilImage CmdClearDepthStencilImage = nullptr;
  PFN_vkCmdCopyBuffer             CmdCopyBuffer             = nullptr;
  PFN_vkCmdCopyBufferToImage      CmdCopyBufferToImage      = nullptr;
  PFN_vkCmdCopyImage              CmdCopyImage              = nullptr;
  PFN_vkCmdCopyImageToBuffer      CmdCopyImageToBuffer      = nullptr;
  PFN_vkCmdCopyQueryPoolResults   CmdCopyQueryPoolResults   = nullptr;
  PFN_vkCmdDispatch               CmdDispatch               = nullptr;
  PFN_vkCmdDispatchIndirect       CmdDispatchIndirect       = nullptr;
  PFN_vkCmdDraw                   CmdDraw                   = nullptr;
  PFN_vkCmdDrawIndexed            CmdDrawIndexed            = nullptr;
  PFN_vkCmdDrawIndexedIndirect    CmdDrawIndexedIndirect    = nullptr;
  PFN_vkCmdDrawIndirect           CmdDrawIndirect           = nullptr;
  PFN_vkCmdEndQuery               CmdEndQuery               = nullptr;
  PFN_vkCmdFillBuffer             CmdFillBuffer             = nullptr;
  PFN_vkCmdPipelineBarrier        CmdPipelineBarrier        = nullptr;
  PFN_vkCmdPushConstants          CmdPushConstants          = nullptr;
  PFN_vkCmdResetEvent             CmdResetEvent             = nullptr;
  PFN_vkCmdResetQueryPool         CmdResetQueryPool         = nullptr;
  PFN_vkCmdResolveImage           CmdResolveImage           = nullptr;
  PFN_vkCmdSetBlendConstants      CmdSetBlendConstants      = nullptr;
  PFN_vkCmdSetDepthBias           CmdSetDepthBias           = nullptr;
  PFN_vkCmdSetDepthBounds         CmdSetDepthBounds         = nullptr;
  PFN_vkCmdSetEvent               CmdSetEvent               = nullptr;
  PFN_vkCmdSetLineWidth           CmdSetLineWidth           = nullptr;
  PFN_vkCmdSetScissor             CmdSetScissor             = nullptr;
  PFN_vkCmdSetStencilCompareMask  CmdSetStencilCompareMask  = nullptr;
  PFN_vkCmdSetStencilReference    CmdSetStencilReference    = nullptr;
  PFN_vkCmdSetStencilWriteMask    CmdSetStencilWriteMask    = nullptr;
  PFN_vkCmdSetViewport            CmdSetViewport            = nullptr;
  PFN_vkCmdUpdateBuffer           CmdUpdateBuffer           = nullptr;
  PFN_vkCmdWaitEvents             CmdWaitEvents             = nullptr;
  PFN_vkCmdWriteTimestamp         CmdWriteTimestamp         = nullptr;
  PFN_vkEndCommandBuffer          EndCommandBuffer          = nullptr;
  PFN_vkResetCommandBuffer        ResetCommandBuffer        = nullptr;

  PFN_vkCmdSetStencilOpEXT             CmdSetStencilOpEXT             = nullptr;
  PFN_vkCmdSetStencilTestEnableEXT     CmdSetStencilTestEnableEXT     = nullptr;
  PFN_vkCmdSetCullModeEXT              CmdSetCullModeEXT              = nullptr;
  PFN_vkCmdSetFrontFaceEXT             CmdSetFrontFaceEXT             = nullptr;
  PFN_vkCmdSetPrimitiveTopologyEXT     CmdSetPrimitiveTopologyEXT     = nullptr;
  PFN_vkCmdSetDepthBoundsTestEnableEXT CmdSetDepthBoundsTestEnableEXT = nullptr;
  PFN_vkCmdSetDepthCompareOpEXT        CmdSetDepthCompareOpEXT        = nullptr;
  PFN_vkCmdSetDepthTestEnableEXT       CmdSetDepthTestEnableEXT       = nullptr;
  PFN_vkCmdSetDepthWriteEnableEXT      CmdSetDepthWriteEnableEXT      = nullptr;

  PFN_vkCmdBeginRenderingKHR CmdBeginRenderingKHR = nullptr;
  PFN_vkCmdEndRenderingKHR   CmdEndRenderingKHR   = nullptr;

  PFN_vkCreateSwapchainKHR    CreateSwapchainKHR    = nullptr;
  PFN_vkDestroySwapchainKHR   DestroySwapchainKHR   = nullptr;
  PFN_vkGetSwapchainImagesKHR GetSwapchainImagesKHR = nullptr;
  PFN_vkAcquireNextImageKHR   AcquireNextImageKHR   = nullptr;
  PFN_vkQueuePresentKHR       QueuePresentKHR       = nullptr;

  PFN_vkDebugMarkerSetObjectTagEXT  DebugMarkerSetObjectTagEXT  = nullptr;
  PFN_vkDebugMarkerSetObjectNameEXT DebugMarkerSetObjectNameEXT = nullptr;

  PFN_vkCmdDebugMarkerBeginEXT  CmdDebugMarkerBeginEXT  = nullptr;
  PFN_vkCmdDebugMarkerEndEXT    CmdDebugMarkerEndEXT    = nullptr;
  PFN_vkCmdDebugMarkerInsertEXT CmdDebugMarkerInsertEXT = nullptr;
};

struct BufferAccess
{
  VkPipelineStageFlags stages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
  VkAccessFlags        access = VK_ACCESS_NONE;
};

struct ImageAccess
{
  VkPipelineStageFlags stages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
  VkAccessFlags        access = VK_ACCESS_NONE;
  VkImageLayout        layout = VK_IMAGE_LAYOUT_UNDEFINED;
};

enum class AccessSequence : u8
{
  None           = 0,
  Reads          = 1,
  Write          = 2,
  ReadAfterWrite = 3
};

struct BufferState
{
  BufferAccess   access[2] = {};
  AccessSequence sequence  = AccessSequence::None;
};

struct ImageState
{
  ImageAccess    access[2] = {};
  AccessSequence sequence  = AccessSequence::None;
};

struct Buffer
{
  gpu::BufferDesc desc           = {};
  VkBuffer        vk_buffer      = nullptr;
  VmaAllocation   vma_allocation = nullptr;
  BufferState     state          = {};
};

struct BufferView
{
  gpu::BufferViewDesc desc    = {};
  VkBufferView        vk_view = nullptr;
};

constexpr u32 COLOR_ASPECT_IDX   = 0;
constexpr u32 DEPTH_ASPECT_IDX   = 0;
constexpr u32 STENCIL_ASPECT_IDX = 1;

struct Image
{
  gpu::ImageDesc    desc                = {};
  bool              is_swapchain_image  = false;
  VkImage           vk_image            = nullptr;
  VmaAllocation     vma_allocation      = nullptr;
  VmaAllocationInfo vma_allocation_info = {};
  ImageState        states[2]           = {};
  u32               num_aspects         = 0;
};

struct ImageView
{
  gpu::ImageViewDesc desc    = {};
  VkImageView        vk_view = nullptr;
};

struct DescriptorSetLayout
{
  gpu::DescriptorBindingDesc bindings[gpu::MAX_DESCRIPTOR_SET_BINDINGS] = {};
  VkDescriptorSetLayout      vk_layout                    = nullptr;
  u32                        sizing[NUM_DESCRIPTOR_TYPES] = {};
  u32                        num_bindings                 = 0;
  u32                        num_variable_length          = 0;
};

/// used to track stateful resource access
/// @param images only valid if `type` is a descriptor type that access images
/// @param param buffers: only valid if `type` is a descriptor type that access
/// buffers
struct DescriptorBinding
{
  union
  {
    void   **sync_resources = nullptr;
    Image  **images;
    Buffer **buffers;
  };
  u32                 count              = 0;
  gpu::DescriptorType type               = gpu::DescriptorType::Sampler;
  bool                is_variable_length = false;
  u32                 max_count          = 0;
};

struct DescriptorSet
{
  VkDescriptorSet   vk_set                                     = nullptr;
  DescriptorBinding bindings[gpu::MAX_DESCRIPTOR_SET_BINDINGS] = {};
  u32               num_bindings                               = 0;
  u32               pool                                       = 0;
};

struct DescriptorPool
{
  VkDescriptorPool vk_pool                     = nullptr;
  u32              avail[NUM_DESCRIPTOR_TYPES] = {};
};

/// @param pool_size each pool will have `pool_size` of each descriptor type
struct DescriptorHeap
{
  AllocatorImpl   allocator    = default_allocator;
  DescriptorPool *pools        = nullptr;
  u32             pool_size    = 0;
  u8             *scratch      = nullptr;
  u32             num_pools    = 0;
  usize           scratch_size = 0;
};

struct ComputePipeline
{
  VkPipeline       vk_pipeline         = nullptr;
  VkPipelineLayout vk_layout           = nullptr;
  u32              push_constants_size = 0;
  u32              num_sets            = 0;
};

struct GraphicsPipeline
{
  VkPipeline       vk_pipeline                                 = nullptr;
  VkPipelineLayout vk_layout                                   = nullptr;
  u32              push_constants_size                         = 0;
  u32              num_sets                                    = 0;
  gpu::Format      colors[gpu::MAX_PIPELINE_COLOR_ATTACHMENTS] = {};
  gpu::Format      depth[1]                                    = {};
  gpu::Format      stencil[1]                                  = {};
  u32              num_colors                                  = 0;
  u32              num_depths                                  = 0;
  u32              num_stencils                                = 0;
};

struct Instance
{
  AllocatorImpl            allocator          = {};
  InstanceTable            vk_table           = {};
  VkInstance               vk_instance        = nullptr;
  VkDebugUtilsMessengerEXT vk_debug_messenger = nullptr;
  bool                     validation_enabled = false;
};

struct PhysicalDevice
{
  VkPhysicalDevice                 vk_phy_dev           = nullptr;
  VkPhysicalDeviceFeatures         vk_features          = {};
  VkPhysicalDeviceProperties       vk_properties        = {};
  VkPhysicalDeviceMemoryProperties vk_memory_properties = {};
};

enum class CommandEncoderState : u16
{
  Reset       = 0,
  Begin       = 1,
  RenderPass  = 2,
  ComputePass = 3,
  End         = 4
};

enum class CommandType : u8
{
  None                = 0,
  BindDescriptorSets  = 1,
  BindPipeline        = 2,
  PushConstants       = 3,
  SetGraphicsState    = 4,
  BindVertexBuffer    = 5,
  BindIndexBuffer     = 6,
  Draw                = 7,
  DrawIndexed         = 8,
  DrawIndirect        = 9,
  DrawIndexedIndirect = 10
};

struct Command
{
  CommandType type = CommandType::None;
  union
  {
    char                                     none_ = 0;
    Tuple<DescriptorSet **, u32, u32 *, u32> set;
    GraphicsPipeline                        *pipeline;
    gpu::GraphicsState                       state;
    Tuple<u8 *, u32>                         push_constant;
    Tuple<u32, Buffer *, u64>                vertex_buffer;
    Tuple<Buffer *, u64, gpu::IndexType>     index_buffer;
    Tuple<u32, u32, u32, u32>                draw;
    Tuple<u32, u32, i32, u32, u32>           draw_indexed;
    Tuple<Buffer *, u64, u32, u32>           draw_indirect;
  };
};

struct RenderPassContext
{
  gpu::Rect render_area = {};
  u32       num_layers  = 0;
  gpu::RenderingAttachment
      color_attachments[gpu::MAX_PIPELINE_COLOR_ATTACHMENTS]          = {};
  gpu::RenderingAttachment depth_attachment[1]                        = {};
  gpu::RenderingAttachment stencil_attachment[1]                      = {};
  u32                      num_color_attachments                      = 0;
  u32                      num_depth_attachments                      = 0;
  u32                      num_stencil_attachments                    = 0;
  Vec<Command>             commands                                   = {};
  ArenaPool                command_pool                               = {};
  ArenaPool                arg_pool                                   = {};
  Buffer                  *vertex_buffers[gpu::MAX_VERTEX_ATTRIBUTES] = {};
  u32                      num_vertex_buffers                         = 0;
  Buffer                  *index_buffer                               = nullptr;
  gpu::IndexType           index_type          = gpu::IndexType::Uint16;
  u64                      index_buffer_offset = 0;
  GraphicsPipeline        *pipeline            = nullptr;
  bool                     has_state           = false;

  void reset()
  {
    render_area             = {};
    num_layers              = 0;
    num_color_attachments   = 0;
    num_depth_attachments   = 0;
    num_stencil_attachments = 0;
    commands.reset();
    command_pool.reset();
    arg_pool.reset();
    num_vertex_buffers  = 0;
    index_buffer        = nullptr;
    index_buffer_offset = 0;
    pipeline            = nullptr;
    has_state           = false;
  }
};

struct ComputePassContext
{
  DescriptorSet   *sets[gpu::MAX_PIPELINE_DESCRIPTOR_SETS] = {};
  u32              num_sets                                = 0;
  ComputePipeline *pipeline                                = nullptr;

  void reset()
  {
    num_sets = 0;
    pipeline = nullptr;
  }
};

struct CommandEncoder
{
  AllocatorImpl       allocator         = {};
  Device             *dev               = nullptr;
  ArenaPool           arg_pool          = {};
  VkCommandPool       vk_command_pool   = nullptr;
  VkCommandBuffer     vk_command_buffer = nullptr;
  Status              status            = Status::Success;
  CommandEncoderState state             = CommandEncoderState::Reset;
  RenderPassContext   render_ctx        = {};
  ComputePassContext  compute_ctx       = {};

  bool is_in_render_pass() const
  {
    return state == CommandEncoderState::RenderPass;
  }

  bool is_in_compute_pass() const
  {
    return state == CommandEncoderState::ComputePass;
  }

  bool is_in_pass() const
  {
    return is_in_render_pass() || is_in_compute_pass();
  }

  bool is_recording() const
  {
    return state == CommandEncoderState::Begin ||
           state == CommandEncoderState::RenderPass ||
           state == CommandEncoderState::ComputePass;
  }

  void reset_context()
  {
    state = CommandEncoderState::Begin;
    render_ctx.reset();
    compute_ctx.reset();
  }
};

/// @param is_optimal false when vulkan returns that the surface is suboptimal
/// or the description is updated by the user
///
/// @param is_out_of_date can't present anymore
/// @param is_optimal recommended but not necessary to resize
/// @param is_zero_sized swapchain is not receiving presentation requests,
/// because the surface requested a zero sized image extent
struct Swapchain
{
  gpu::SwapchainDesc  desc            = {};
  bool                is_out_of_date  = true;
  bool                is_optimal      = false;
  bool                is_zero_sized   = false;
  gpu::SurfaceFormat  format          = {};
  gpu::ImageUsage     usage           = gpu::ImageUsage::None;
  gpu::PresentMode    present_mode    = gpu::PresentMode::Immediate;
  gpu::Extent         extent          = {};
  gpu::CompositeAlpha composite_alpha = gpu::CompositeAlpha::None;
  Image               image_impls[gpu::MAX_SWAPCHAIN_IMAGES] = {};
  gpu::Image          images[gpu::MAX_SWAPCHAIN_IMAGES]      = {};
  VkImage             vk_images[gpu::MAX_SWAPCHAIN_IMAGES]   = {};
  u32                 num_images                             = 0;
  u32                 current_image                          = 0;
  VkSwapchainKHR      vk_swapchain                           = nullptr;
  VkSurfaceKHR        vk_surface                             = nullptr;
};

struct FrameContext
{
  gpu::FrameId            tail_frame                          = 0;
  gpu::FrameId            current_frame                       = 0;
  u32                     ring_index                          = 0;
  u32                     buffering                           = 0;
  CommandEncoder          encs[gpu::MAX_FRAME_BUFFERING]      = {};
  gpu::CommandEncoderImpl encs_impl[gpu::MAX_FRAME_BUFFERING] = {};
  VkSemaphore             acquire_s[gpu::MAX_FRAME_BUFFERING] = {};
  VkFence                 submit_f[gpu::MAX_FRAME_BUFFERING]  = {};
  VkSemaphore             submit_s[gpu::MAX_FRAME_BUFFERING]  = {};
};

struct Device
{
  AllocatorImpl      allocator       = {};
  Instance          *instance        = nullptr;
  PhysicalDevice     phy_dev         = {};
  DeviceTable        vk_table        = {};
  VmaVulkanFunctions vma_table       = {};
  VkDevice           vk_dev          = nullptr;
  u32                queue_family    = 0;
  VkQueue            vk_queue        = nullptr;
  VmaAllocator       vma_allocator   = nullptr;
  FrameContext       frame_ctx       = {};
  DescriptorHeap     descriptor_heap = {};
};

}        // namespace vk
}        // namespace ash
