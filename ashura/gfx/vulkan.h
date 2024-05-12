#pragma once
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
#define VMA_VULKAN_VERSION 1000000

#include "ashura/gfx/gfx.h"
#include "ashura/std/allocator.h"
#include "ashura/std/arena_allocator.h"
#include "ashura/std/vec.h"
#include "vk_mem_alloc.h"
#include "vulkan/vk_enum_string_helper.h"
#include "vulkan/vulkan.h"

namespace ash
{
namespace vk
{

using gfx::Status;

constexpr char const *ENGINE_NAME    = "Ash";
constexpr u32         ENGINE_VERSION = VK_MAKE_API_VERSION(0, 0, 0, 1);
constexpr char const *CLIENT_NAME    = "Ash Client";
constexpr u32         CLIENT_VERSION = VK_MAKE_API_VERSION(0, 0, 0, 1);

constexpr u32 MAX_MEMORY_HEAP_PROPERTIES = 32;
constexpr u32 MAX_MEMORY_HEAPS           = 16;
constexpr u8  NUM_DESCRIPTOR_TYPES       = 11;

struct InstanceInterface
{
  static void destroy(gfx::Instance self);
  static Result<gfx::DeviceImpl, Status>
                      create_device(gfx::Instance self, AllocatorImpl allocator,
                                    Span<gfx::DeviceType const> preferred_types,
                                    Span<gfx::Surface const>    compatible_surfaces,
                                    u32                         buffering);
  static gfx::Backend get_backend(gfx::Instance self);
  static void         destroy_device(gfx::Instance self, gfx::Device device);
  static void         destroy_surface(gfx::Instance self, gfx::Surface surface);
};

struct DeviceInterface
{
  static gfx::DeviceProperties get_device_properties(gfx::Device self);
  static Result<gfx::FormatProperties, Status>
      get_format_properties(gfx::Device self, gfx::Format format);
  static Result<gfx::Buffer, Status> create_buffer(gfx::Device            self,
                                                   gfx::BufferDesc const &desc);
  static Result<gfx::BufferView, Status>
      create_buffer_view(gfx::Device self, gfx::BufferViewDesc const &desc);
  static Result<gfx::Image, Status> create_image(gfx::Device           self,
                                                 gfx::ImageDesc const &desc);
  static Result<gfx::ImageView, Status>
      create_image_view(gfx::Device self, gfx::ImageViewDesc const &desc);
  static Result<gfx::Sampler, Status>
      create_sampler(gfx::Device self, gfx::SamplerDesc const &desc);
  static Result<gfx::Shader, Status> create_shader(gfx::Device            self,
                                                   gfx::ShaderDesc const &desc);
  static Result<gfx::DescriptorSetLayout, Status>
      create_descriptor_set_layout(gfx::Device                         self,
                                   gfx::DescriptorSetLayoutDesc const &desc);
  static Result<gfx::DescriptorSet, Status>
      create_descriptor_set(gfx::Device self, gfx::DescriptorSetLayout layout,
                            Span<u32 const> variable_lengths);
  static Result<gfx::PipelineCache, Status>
      create_pipeline_cache(gfx::Device                   self,
                            gfx::PipelineCacheDesc const &desc);
  static Result<gfx::ComputePipeline, Status>
      create_compute_pipeline(gfx::Device                     self,
                              gfx::ComputePipelineDesc const &desc);
  static Result<gfx::GraphicsPipeline, Status>
      create_graphics_pipeline(gfx::Device                      self,
                               gfx::GraphicsPipelineDesc const &desc);
  static Result<gfx::Swapchain, Status>
      create_swapchain(gfx::Device self, gfx::Surface surface,
                       gfx::SwapchainDesc const &desc);
  static Result<gfx::TimeStampQuery, Status>
      create_timestamp_query(gfx::Device self);
  static Result<gfx::StatisticsQuery, Status>
              create_statistics_query(gfx::Device self);
  static void destroy_buffer(gfx::Device self, gfx::Buffer buffer);
  static void destroy_buffer_view(gfx::Device     self,
                                  gfx::BufferView buffer_view);
  static void destroy_image(gfx::Device self, gfx::Image image);
  static void destroy_image_view(gfx::Device self, gfx::ImageView image_view);
  static void destroy_sampler(gfx::Device self, gfx::Sampler sampler);
  static void destroy_shader(gfx::Device self, gfx::Shader shader);
  static void destroy_descriptor_set_layout(gfx::Device              self,
                                            gfx::DescriptorSetLayout layout);
  static void destroy_descriptor_set(gfx::Device self, gfx::DescriptorSet set);
  static void destroy_pipeline_cache(gfx::Device        self,
                                     gfx::PipelineCache cache);
  static void destroy_compute_pipeline(gfx::Device          self,
                                       gfx::ComputePipeline pipeline);
  static void destroy_graphics_pipeline(gfx::Device           self,
                                        gfx::GraphicsPipeline pipeline);
  static void destroy_swapchain(gfx::Device self, gfx::Swapchain swapchain);
  static void destroy_timestamp_query(gfx::Device         self,
                                      gfx::TimeStampQuery query);
  static void destroy_statistics_query(gfx::Device          self,
                                       gfx::StatisticsQuery query);
  static gfx::FrameContext      get_frame_context(gfx::Device self);
  static Result<void *, Status> get_buffer_memory_map(gfx::Device self,
                                                      gfx::Buffer buffer);
  static Result<Void, Status>
      invalidate_buffer_memory_map(gfx::Device self, gfx::Buffer buffer,
                                   gfx::MemoryRange ranges);
  static Result<Void, Status> flush_buffer_memory_map(gfx::Device      self,
                                                      gfx::Buffer      buffer,
                                                      gfx::MemoryRange range);
  static Result<usize, Status>
      get_pipeline_cache_size(gfx::Device self, gfx::PipelineCache cache);
  static Result<usize, Status> get_pipeline_cache_data(gfx::Device        self,
                                                       gfx::PipelineCache cache,
                                                       Span<u8>           out);
  static Result<Void, Status>
              merge_pipeline_cache(gfx::Device self, gfx::PipelineCache dst,
                                   Span<gfx::PipelineCache const> srcs);
  static void update_descriptor_set(gfx::Device                     self,
                                    gfx::DescriptorSetUpdate const &update);
  static Result<Void, Status> wait_idle(gfx::Device self);
  static Result<Void, Status> wait_queue_idle(gfx::Device self);
  static Result<u32, Status>
      get_surface_formats(gfx::Device self, gfx::Surface surface,
                          Span<gfx::SurfaceFormat> formats);
  static Result<u32, Status>
      get_surface_present_modes(gfx::Device self, gfx::Surface surface,
                                Span<gfx::PresentMode> modes);
  static Result<gfx::SurfaceCapabilities, Status>
      get_surface_capabilities(gfx::Device self, gfx::Surface surface);
  static Result<gfx::SwapchainState, Status>
      get_swapchain_state(gfx::Device self, gfx::Swapchain swapchain);
  static Result<Void, Status>
      invalidate_swapchain(gfx::Device self, gfx::Swapchain swapchain,
                           gfx::SwapchainDesc const &desc);
  static Result<Void, Status> begin_frame(gfx::Device    self,
                                          gfx::Swapchain swapchain);
  static Result<Void, Status> submit_frame(gfx::Device    self,
                                           gfx::Swapchain swapchain);
  static Result<u64, Status>
      get_timestamp_query_result(gfx::Device self, gfx::TimeStampQuery query);
  static Result<gfx::PipelineStatistics, Status>
      get_statistics_query_result(gfx::Device self, gfx::StatisticsQuery query);
};

struct CommandEncoderInterface
{
  static void reset_timestamp_query(gfx::CommandEncoder self,
                                    gfx::TimeStampQuery query);
  static void reset_statistics_query(gfx::CommandEncoder  self,
                                     gfx::StatisticsQuery query);
  static void write_timestamp(gfx::CommandEncoder self,
                              gfx::TimeStampQuery query);
  static void begin_statistics(gfx::CommandEncoder  self,
                               gfx::StatisticsQuery query);
  static void end_statistics(gfx::CommandEncoder  self,
                             gfx::StatisticsQuery query);
  static void begin_debug_marker(gfx::CommandEncoder self,
                                 Span<char const> region_name, Vec4 color);
  static void end_debug_marker(gfx::CommandEncoder self);
  static void fill_buffer(gfx::CommandEncoder self, gfx::Buffer dst, u64 offset,
                          u64 size, u32 data);
  static void copy_buffer(gfx::CommandEncoder self, gfx::Buffer src,
                          gfx::Buffer dst, Span<gfx::BufferCopy const> copies);
  static void update_buffer(gfx::CommandEncoder self, Span<u8 const> src,
                            u64 dst_offset, gfx::Buffer dst);
  static void clear_color_image(gfx::CommandEncoder self, gfx::Image dst,
                                gfx::Color clear_color,
                                Span<gfx::ImageSubresourceRange const> ranges);
  static void
      clear_depth_stencil_image(gfx::CommandEncoder self, gfx::Image dst,
                                gfx::DepthStencil clear_depth_stencil,
                                Span<gfx::ImageSubresourceRange const> ranges);
  static void copy_image(gfx::CommandEncoder self, gfx::Image src,
                         gfx::Image dst, Span<gfx::ImageCopy const> copies);
  static void copy_buffer_to_image(gfx::CommandEncoder self, gfx::Buffer src,
                                   gfx::Image                       dst,
                                   Span<gfx::BufferImageCopy const> copies);
  static void blit_image(gfx::CommandEncoder self, gfx::Image src,
                         gfx::Image dst, Span<gfx::ImageBlit const> blits,
                         gfx::Filter filter);
  static void resolve_image(gfx::CommandEncoder self, gfx::Image src,
                            gfx::Image                    dst,
                            Span<gfx::ImageResolve const> resolves);
  static void begin_compute_pass(gfx::CommandEncoder self);
  static void end_compute_pass(gfx::CommandEncoder self);
  static void begin_rendering(gfx::CommandEncoder       self,
                              gfx::RenderingInfo const &info);
  static void end_rendering(gfx::CommandEncoder self);
  static void bind_compute_pipeline(gfx::CommandEncoder  self,
                                    gfx::ComputePipeline pipeline);
  static void bind_graphics_pipeline(gfx::CommandEncoder   self,
                                     gfx::GraphicsPipeline pipeline);
  static void
              bind_descriptor_sets(gfx::CommandEncoder            self,
                                   Span<gfx::DescriptorSet const> descriptor_sets,
                                   Span<u32 const>                dynamic_offsets);
  static void push_constants(gfx::CommandEncoder self,
                             Span<u8 const>      push_constants_data);
  static void dispatch(gfx::CommandEncoder self, u32 group_count_x,
                       u32 group_count_y, u32 group_count_z);
  static void dispatch_indirect(gfx::CommandEncoder self, gfx::Buffer buffer,
                                u64 offset);
  static void set_graphics_state(gfx::CommandEncoder       self,
                                 gfx::GraphicsState const &state);
  static void bind_vertex_buffers(gfx::CommandEncoder     self,
                                  Span<gfx::Buffer const> vertex_buffers,
                                  Span<u64 const>         offsets);
  static void bind_index_buffer(gfx::CommandEncoder self,
                                gfx::Buffer index_buffer, u64 offset,
                                gfx::IndexType index_type);
  static void draw(gfx::CommandEncoder self, u32 vertex_count,
                   u32 instance_count, u32 first_vertex_id,
                   u32 first_instance_id);
  static void draw_indexed(gfx::CommandEncoder self, u32 first_index,
                           u32 num_indices, i32 vertex_offset,
                           u32 first_instance_id, u32 num_instances);
  static void draw_indirect(gfx::CommandEncoder self, gfx::Buffer buffer,
                            u64 offset, u32 draw_count, u32 stride);
  static void draw_indexed_indirect(gfx::CommandEncoder self,
                                    gfx::Buffer buffer, u64 offset,
                                    u32 draw_count, u32 stride);
};

static gfx::InstanceInterface const instance_interface{
    .destroy         = InstanceInterface::destroy,
    .create_device   = InstanceInterface::create_device,
    .get_backend     = InstanceInterface::get_backend,
    .destroy_device  = InstanceInterface::destroy_device,
    .destroy_surface = InstanceInterface::destroy_surface};

static gfx::DeviceInterface const device_interface{
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
    .get_buffer_memory_map     = DeviceInterface::get_buffer_memory_map,
    .invalidate_buffer_memory_map =
        DeviceInterface::invalidate_buffer_memory_map,
    .flush_buffer_memory_map    = DeviceInterface::flush_buffer_memory_map,
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

static gfx::CommandEncoderInterface const command_encoder_interface{
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
  gfx::BufferDesc   desc                = {};
  VkBuffer          vk_buffer           = nullptr;
  VmaAllocation     vma_allocation      = nullptr;
  VmaAllocationInfo vma_allocation_info = {};
  void             *host_map            = nullptr;
  BufferState       state               = {};
};

struct BufferView
{
  gfx::BufferViewDesc desc    = {};
  VkBufferView        vk_view = nullptr;
};

constexpr u32 COLOR_ASPECT_IDX   = 0;
constexpr u32 DEPTH_ASPECT_IDX   = 0;
constexpr u32 STENCIL_ASPECT_IDX = 1;

struct Image
{
  gfx::ImageDesc    desc                = {};
  bool              is_swapchain_image  = false;
  VkImage           vk_image            = nullptr;
  VmaAllocation     vma_allocation      = nullptr;
  VmaAllocationInfo vma_allocation_info = {};
  ImageState        states[2]           = {};
  u32               num_aspects         = 0;
};

struct ImageView
{
  gfx::ImageViewDesc desc    = {};
  VkImageView        vk_view = nullptr;
};

struct DescriptorSetLayout
{
  gfx::DescriptorBindingDesc bindings[gfx::MAX_DESCRIPTOR_SET_BINDINGS] = {};
  VkDescriptorSetLayout      vk_layout                    = nullptr;
  u32                        sizing[NUM_DESCRIPTOR_TYPES] = {};
  u32                        num_bindings                 = 0;
  u32                        num_variable_length          = 0;
};

/// used to track stateful resource access
/// @images: only valid if `type` is a descriptor type that access images
/// @buffers: only valid if `type` is a descriptor type that access buffers
struct DescriptorBinding
{
  union
  {
    void   **resources = nullptr;
    Image  **images;
    Buffer **buffers;
  };
  u32                 count              = 0;
  gfx::DescriptorType type               = gfx::DescriptorType::Sampler;
  bool                is_variable_length = false;
  u32                 max_count          = 0;
};

struct DescriptorSet
{
  VkDescriptorSet   vk_set                                     = nullptr;
  DescriptorBinding bindings[gfx::MAX_DESCRIPTOR_SET_BINDINGS] = {};
  u32               num_bindings                               = 0;
  u32               pool                                       = 0;
};

struct DescriptorPool
{
  VkDescriptorPool vk_pool                     = nullptr;
  u32              avail[NUM_DESCRIPTOR_TYPES] = {};
};

/// @pool_size: each pool will have `pool_size` of each descriptor type
struct DescriptorHeap
{
  AllocatorImpl   allocator    = default_allocator;
  DescriptorPool *pools        = nullptr;
  u32             pool_size    = 0;
  void           *scratch      = nullptr;
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
  gfx::Format      colors[gfx::MAX_PIPELINE_COLOR_ATTACHMENTS] = {};
  gfx::Format      depth[1]                                    = {};
  gfx::Format      stencil[1]                                  = {};
  u32              num_colors                                  = 0;
  u32              num_depths                                  = 0;
  u32              num_stencils                                = 0;
};

struct Instance
{
  AllocatorImpl            allocator          = {};
  Logger                  *logger             = {};
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
    gfx::GraphicsState                       state;
    Tuple<u8 *, u32>                         push_constant;
    Tuple<u32, Buffer *, u64>                vertex_buffer;
    Tuple<Buffer *, u64, gfx::IndexType>     index_buffer;
    Tuple<u32, u32, u32, u32>                draw;
    Tuple<u32, u32, i32, u32, u32>           draw_indexed;
    Tuple<Buffer *, u64, u32, u32>           draw_indirect;
  };
};

struct RenderPassContext
{
  gfx::Offset offset     = {};
  gfx::Extent extent     = {};
  u32         num_layers = 0;
  gfx::RenderingAttachment
      color_attachments[gfx::MAX_PIPELINE_COLOR_ATTACHMENTS]          = {};
  gfx::RenderingAttachment depth_attachment[1]                        = {};
  gfx::RenderingAttachment stencil_attachment[1]                      = {};
  u32                      num_color_attachments                      = 0;
  u32                      num_depth_attachments                      = 0;
  u32                      num_stencil_attachments                    = 0;
  Vec<Command>             commands                                   = {};
  ArenaPool                command_pool                               = {};
  ArenaPool                arg_pool                                   = {};
  Buffer                  *vertex_buffers[gfx::MAX_VERTEX_ATTRIBUTES] = {};
  u32                      num_vertex_buffers                         = 0;
  Buffer                  *index_buffer                               = nullptr;
  gfx::IndexType           index_type          = gfx::IndexType::Uint16;
  u64                      index_buffer_offset = 0;
  GraphicsPipeline        *pipeline            = nullptr;
  bool                     has_state           = false;

  void reset()
  {
    offset                  = {};
    extent                  = {};
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
  DescriptorSet   *sets[gfx::MAX_PIPELINE_DESCRIPTOR_SETS] = {};
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
  Logger             *logger            = nullptr;
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

/// @is_optimal: false when vulkan returns that the surface is suboptimal or
/// the description is updated by the user
///
/// @is_out_of_date: can't present anymore
/// @is_optimal: recommended but not necessary to resize
/// @is_zero_sized: swapchain is not receiving presentation requests, because
/// the surface requested a zero sized image extent
struct Swapchain
{
  gfx::SwapchainDesc  desc            = {};
  bool                is_out_of_date  = true;
  bool                is_optimal      = false;
  bool                is_zero_sized   = false;
  gfx::SurfaceFormat  format          = {};
  gfx::ImageUsage     usage           = gfx::ImageUsage::None;
  gfx::PresentMode    present_mode    = gfx::PresentMode::Immediate;
  gfx::Extent         extent          = {};
  gfx::CompositeAlpha composite_alpha = gfx::CompositeAlpha::None;
  Image               image_impls[gfx::MAX_SWAPCHAIN_IMAGES] = {};
  gfx::Image          images[gfx::MAX_SWAPCHAIN_IMAGES]      = {};
  VkImage             vk_images[gfx::MAX_SWAPCHAIN_IMAGES]   = {};
  u32                 num_images                             = 0;
  u32                 current_image                          = 0;
  VkSwapchainKHR      vk_swapchain                           = nullptr;
  VkSurfaceKHR        vk_surface                             = nullptr;
};

struct FrameContext
{
  gfx::FrameId            tail_frame                          = 0;
  gfx::FrameId            current_frame                       = 0;
  u32                     ring_index                          = 0;
  u32                     buffering                           = 0;
  CommandEncoder          encs[gfx::MAX_FRAME_BUFFERING]      = {};
  gfx::CommandEncoderImpl encs_impl[gfx::MAX_FRAME_BUFFERING] = {};
  VkSemaphore             acquire_s[gfx::MAX_FRAME_BUFFERING] = {};
  VkFence                 submit_f[gfx::MAX_FRAME_BUFFERING]  = {};
  VkSemaphore             submit_s[gfx::MAX_FRAME_BUFFERING]  = {};
};

struct Device
{
  AllocatorImpl      allocator       = {};
  Logger            *logger          = nullptr;
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
