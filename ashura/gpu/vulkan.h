/// SPDX-License-Identifier: MIT
#pragma once
#define VMA_STATIC_VULKAN_FUNCTIONS  0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
#define VMA_VULKAN_VERSION           1'000'000

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

inline constexpr char const * ENGINE_NAME    = "Ash";
inline constexpr u32          ENGINE_VERSION = VK_MAKE_API_VERSION(0, 0, 0, 1);
inline constexpr char const * CLIENT_NAME    = "Ash Client";
inline constexpr u32          CLIENT_VERSION = VK_MAKE_API_VERSION(0, 0, 0, 1);

inline constexpr u32 MAX_MEMORY_HEAP_PROPERTIES = 32;
inline constexpr u32 MAX_MEMORY_HEAPS           = 16;
inline constexpr u8  NUM_DESCRIPTOR_TYPES       = 11;

typedef VkSampler             Sampler;
typedef VkShaderModule        Shader;
typedef VkPipelineCache       PipelineCache;
typedef VkSurfaceKHR          Surface;
typedef VkQueryPool           TimestampQuery;
typedef VkQueryPool           StatisticsQuery;
typedef struct Instance       Instance;
typedef struct Device         Device;
typedef struct CommandEncoder CommandEncoder;

#define ASH_DEF_VKPFN(name) PFN_vk##name name = nullptr

struct InstanceTable
{
  ASH_DEF_VKPFN(CreateInstance);
  ASH_DEF_VKPFN(DestroyInstance);
  ASH_DEF_VKPFN(DestroySurfaceKHR);
  ASH_DEF_VKPFN(EnumeratePhysicalDevices);
  ASH_DEF_VKPFN(GetInstanceProcAddr);
  ASH_DEF_VKPFN(GetDeviceProcAddr);

  ASH_DEF_VKPFN(CreateDevice);
  ASH_DEF_VKPFN(EnumerateDeviceExtensionProperties);
  ASH_DEF_VKPFN(EnumerateDeviceLayerProperties);
  ASH_DEF_VKPFN(GetPhysicalDeviceFeatures);
  ASH_DEF_VKPFN(GetPhysicalDeviceFormatProperties);
  ASH_DEF_VKPFN(GetPhysicalDeviceImageFormatProperties);
  ASH_DEF_VKPFN(GetPhysicalDeviceMemoryProperties);
  ASH_DEF_VKPFN(GetPhysicalDeviceProperties);
  ASH_DEF_VKPFN(GetPhysicalDeviceQueueFamilyProperties);
  ASH_DEF_VKPFN(GetPhysicalDeviceSparseImageFormatProperties);

  ASH_DEF_VKPFN(GetPhysicalDeviceSurfaceSupportKHR);
  ASH_DEF_VKPFN(GetPhysicalDeviceSurfaceCapabilitiesKHR);
  ASH_DEF_VKPFN(GetPhysicalDeviceSurfaceFormatsKHR);
  ASH_DEF_VKPFN(GetPhysicalDeviceSurfacePresentModesKHR);

  ASH_DEF_VKPFN(CreateDebugUtilsMessengerEXT);
  ASH_DEF_VKPFN(DestroyDebugUtilsMessengerEXT);
  ASH_DEF_VKPFN(SetDebugUtilsObjectNameEXT);
};

struct DeviceTable
{
  // DEVICE OBJECT FUNCTIONS
  ASH_DEF_VKPFN(AllocateCommandBuffers);
  ASH_DEF_VKPFN(AllocateDescriptorSets);
  ASH_DEF_VKPFN(AllocateMemory);
  ASH_DEF_VKPFN(BindBufferMemory);
  ASH_DEF_VKPFN(BindImageMemory);
  ASH_DEF_VKPFN(CreateBuffer);
  ASH_DEF_VKPFN(CreateBufferView);
  ASH_DEF_VKPFN(CreateCommandPool);
  ASH_DEF_VKPFN(CreateComputePipelines);
  ASH_DEF_VKPFN(CreateDescriptorPool);
  ASH_DEF_VKPFN(CreateDescriptorSetLayout);
  ASH_DEF_VKPFN(CreateEvent);
  ASH_DEF_VKPFN(CreateFence);
  ASH_DEF_VKPFN(CreateGraphicsPipelines);
  ASH_DEF_VKPFN(CreateImage);
  ASH_DEF_VKPFN(CreateImageView);
  ASH_DEF_VKPFN(CreatePipelineCache);
  ASH_DEF_VKPFN(CreatePipelineLayout);
  ASH_DEF_VKPFN(CreateQueryPool);
  ASH_DEF_VKPFN(CreateSampler);
  ASH_DEF_VKPFN(CreateSemaphore);
  ASH_DEF_VKPFN(CreateShaderModule);
  ASH_DEF_VKPFN(DestroyBuffer);
  ASH_DEF_VKPFN(DestroyBufferView);
  ASH_DEF_VKPFN(DestroyCommandPool);
  ASH_DEF_VKPFN(DestroyDescriptorPool);
  ASH_DEF_VKPFN(DestroyDescriptorSetLayout);
  ASH_DEF_VKPFN(DestroyDevice);
  ASH_DEF_VKPFN(DestroyEvent);
  ASH_DEF_VKPFN(DestroyFence);
  ASH_DEF_VKPFN(DestroyImage);
  ASH_DEF_VKPFN(DestroyImageView);
  ASH_DEF_VKPFN(DestroyPipeline);
  ASH_DEF_VKPFN(DestroyPipelineCache);
  ASH_DEF_VKPFN(DestroyPipelineLayout);
  ASH_DEF_VKPFN(DestroyQueryPool);
  ASH_DEF_VKPFN(DestroySampler);
  ASH_DEF_VKPFN(DestroySemaphore);
  ASH_DEF_VKPFN(DestroyShaderModule);
  ASH_DEF_VKPFN(DeviceWaitIdle);
  ASH_DEF_VKPFN(FlushMappedMemoryRanges);
  ASH_DEF_VKPFN(FreeCommandBuffers);
  ASH_DEF_VKPFN(FreeDescriptorSets);
  ASH_DEF_VKPFN(FreeMemory);
  ASH_DEF_VKPFN(GetBufferMemoryRequirements);
  ASH_DEF_VKPFN(GetDeviceMemoryCommitment);
  ASH_DEF_VKPFN(GetDeviceQueue);
  ASH_DEF_VKPFN(GetEventStatus);
  ASH_DEF_VKPFN(GetFenceStatus);
  ASH_DEF_VKPFN(GetImageMemoryRequirements);
  ASH_DEF_VKPFN(GetImageSubresourceLayout);
  ASH_DEF_VKPFN(GetPipelineCacheData);
  ASH_DEF_VKPFN(GetQueryPoolResults);
  ASH_DEF_VKPFN(InvalidateMappedMemoryRanges);
  ASH_DEF_VKPFN(MapMemory);
  ASH_DEF_VKPFN(MergePipelineCaches);
  ASH_DEF_VKPFN(ResetCommandPool);
  ASH_DEF_VKPFN(ResetDescriptorPool);
  ASH_DEF_VKPFN(ResetEvent);
  ASH_DEF_VKPFN(ResetFences);
  ASH_DEF_VKPFN(SetEvent);
  ASH_DEF_VKPFN(UpdateDescriptorSets);
  ASH_DEF_VKPFN(UnmapMemory);
  ASH_DEF_VKPFN(WaitForFences);

  ASH_DEF_VKPFN(QueueSubmit);
  ASH_DEF_VKPFN(QueueWaitIdle);

  // COMMAND BUFFER OBJECT FUNCTIONS
  ASH_DEF_VKPFN(BeginCommandBuffer);
  ASH_DEF_VKPFN(CmdBeginQuery);
  ASH_DEF_VKPFN(CmdBindDescriptorSets);
  ASH_DEF_VKPFN(CmdBindIndexBuffer);
  ASH_DEF_VKPFN(CmdBindPipeline);
  ASH_DEF_VKPFN(CmdBindVertexBuffers);
  ASH_DEF_VKPFN(CmdBlitImage);
  ASH_DEF_VKPFN(CmdClearAttachments);
  ASH_DEF_VKPFN(CmdClearColorImage);
  ASH_DEF_VKPFN(CmdClearDepthStencilImage);
  ASH_DEF_VKPFN(CmdCopyBuffer);
  ASH_DEF_VKPFN(CmdCopyBufferToImage);
  ASH_DEF_VKPFN(CmdCopyImage);
  ASH_DEF_VKPFN(CmdCopyImageToBuffer);
  ASH_DEF_VKPFN(CmdCopyQueryPoolResults);
  ASH_DEF_VKPFN(CmdDispatch);
  ASH_DEF_VKPFN(CmdDispatchIndirect);
  ASH_DEF_VKPFN(CmdDraw);
  ASH_DEF_VKPFN(CmdDrawIndexed);
  ASH_DEF_VKPFN(CmdDrawIndexedIndirect);
  ASH_DEF_VKPFN(CmdDrawIndirect);
  ASH_DEF_VKPFN(CmdEndQuery);
  ASH_DEF_VKPFN(CmdFillBuffer);
  ASH_DEF_VKPFN(CmdPipelineBarrier);
  ASH_DEF_VKPFN(CmdPushConstants);
  ASH_DEF_VKPFN(CmdResetEvent);
  ASH_DEF_VKPFN(CmdResetQueryPool);
  ASH_DEF_VKPFN(CmdResolveImage);
  ASH_DEF_VKPFN(CmdSetBlendConstants);
  ASH_DEF_VKPFN(CmdSetDepthBias);
  ASH_DEF_VKPFN(CmdSetDepthBounds);
  ASH_DEF_VKPFN(CmdSetEvent);
  ASH_DEF_VKPFN(CmdSetLineWidth);
  ASH_DEF_VKPFN(CmdSetScissor);
  ASH_DEF_VKPFN(CmdSetStencilCompareMask);
  ASH_DEF_VKPFN(CmdSetStencilReference);
  ASH_DEF_VKPFN(CmdSetStencilWriteMask);
  ASH_DEF_VKPFN(CmdSetViewport);
  ASH_DEF_VKPFN(CmdUpdateBuffer);
  ASH_DEF_VKPFN(CmdWaitEvents);
  ASH_DEF_VKPFN(CmdWriteTimestamp);
  ASH_DEF_VKPFN(EndCommandBuffer);
  ASH_DEF_VKPFN(ResetCommandBuffer);

  ASH_DEF_VKPFN(CmdSetStencilOpEXT);
  ASH_DEF_VKPFN(CmdSetStencilTestEnableEXT);
  ASH_DEF_VKPFN(CmdSetCullModeEXT);
  ASH_DEF_VKPFN(CmdSetFrontFaceEXT);
  ASH_DEF_VKPFN(CmdSetPrimitiveTopologyEXT);
  ASH_DEF_VKPFN(CmdSetDepthBoundsTestEnableEXT);
  ASH_DEF_VKPFN(CmdSetDepthCompareOpEXT);
  ASH_DEF_VKPFN(CmdSetDepthTestEnableEXT);
  ASH_DEF_VKPFN(CmdSetDepthWriteEnableEXT);

  ASH_DEF_VKPFN(CmdBeginRenderingKHR);
  ASH_DEF_VKPFN(CmdEndRenderingKHR);

  ASH_DEF_VKPFN(CreateSwapchainKHR);
  ASH_DEF_VKPFN(DestroySwapchainKHR);
  ASH_DEF_VKPFN(GetSwapchainImagesKHR);
  ASH_DEF_VKPFN(AcquireNextImageKHR);
  ASH_DEF_VKPFN(QueuePresentKHR);

  ASH_DEF_VKPFN(DebugMarkerSetObjectTagEXT);
  ASH_DEF_VKPFN(DebugMarkerSetObjectNameEXT);

  ASH_DEF_VKPFN(CmdDebugMarkerBeginEXT);
  ASH_DEF_VKPFN(CmdDebugMarkerEndEXT);
  ASH_DEF_VKPFN(CmdDebugMarkerInsertEXT);
};

#undef ASH_DEF_VKPFN

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
  gpu::BufferInfo info           = {};
  VkBuffer        vk_buffer      = nullptr;
  VmaAllocation   vma_allocation = nullptr;
  BufferState     state          = {};
};

struct BufferView
{
  gpu::BufferViewInfo info    = {};
  VkBufferView        vk_view = nullptr;
};

inline constexpr u32 COLOR_ASPECT_IDX   = 0;
inline constexpr u32 DEPTH_ASPECT_IDX   = 0;
inline constexpr u32 STENCIL_ASPECT_IDX = 1;

struct Image
{
  gpu::ImageInfo    info                = {};
  bool              is_swapchain_image  = false;
  VkImage           vk_image            = nullptr;
  VmaAllocation     vma_allocation      = nullptr;
  VmaAllocationInfo vma_allocation_info = {};
  ImageState        states[2]           = {};
  u32               num_aspects         = 0;
};

struct ImageView
{
  gpu::ImageViewInfo info    = {};
  VkImageView        vk_view = nullptr;
};

struct DescriptorSetLayout
{
  gpu::DescriptorBindingInfo bindings[gpu::MAX_DESCRIPTOR_SET_BINDINGS] = {};
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
    void **   sync_resources = nullptr;
    Image **  images;
    Buffer ** buffers;
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
  AllocatorImpl    allocator    = {};
  DescriptorPool * pools        = nullptr;
  u32              pool_size    = 0;
  u8 *             scratch      = nullptr;
  u32              num_pools    = 0;
  usize            scratch_size = 0;
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
  gpu::SampleCount sample_count = gpu::SampleCount::C1;
};

struct Instance final : gpu::Instance
{
  AllocatorImpl            allocator          = {};
  InstanceTable            vk_table           = {};
  VkInstance               vk_instance        = nullptr;
  VkDebugUtilsMessengerEXT vk_debug_messenger = nullptr;
  bool                     validation_enabled = false;

  explicit Instance() = default;

  Instance(Instance const &)             = delete;
  Instance & operator=(Instance const &) = delete;
  Instance(Instance &&)                  = delete;
  Instance & operator=(Instance &&)      = delete;
  virtual ~Instance() override;

  virtual Result<gpu::Device *, Status>
    create_device(AllocatorImpl               allocator,
                  Span<gpu::DeviceType const> preferred_types,
                  u32                         buffering) override;

  virtual gpu::Backend get_backend() override;

  virtual void uninit(gpu::Device * device) override;

  virtual void uninit(gpu::Surface surface) override;
};

struct PhysicalDevice
{
  VkPhysicalDevice                 vk_phy_dev           = nullptr;
  VkPhysicalDeviceFeatures         vk_features          = {};
  VkPhysicalDeviceProperties       vk_properties        = {};
  VkPhysicalDeviceMemoryProperties vk_memory_properties = {};
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
  gpu::SwapchainInfo  info            = {};
  bool                is_out_of_date  = true;
  bool                is_optimal      = false;
  bool                is_zero_sized   = false;
  gpu::SurfaceFormat  format          = {};
  gpu::ImageUsage     usage           = gpu::ImageUsage::None;
  gpu::PresentMode    present_mode    = gpu::PresentMode::Immediate;
  Vec2U               extent          = {};
  gpu::CompositeAlpha composite_alpha = gpu::CompositeAlpha::None;
  Image               image_impls[gpu::MAX_SWAPCHAIN_IMAGES] = {};
  gpu::Image          images[gpu::MAX_SWAPCHAIN_IMAGES]      = {};
  VkImage             vk_images[gpu::MAX_SWAPCHAIN_IMAGES]   = {};
  u32                 num_images                             = 0;
  u32                 current_image                          = 0;
  VkSwapchainKHR      vk_swapchain                           = nullptr;
  VkSurfaceKHR        vk_surface                             = nullptr;
};

enum class CommandEncoderState : u16
{
  Reset       = 0,
  Begin       = 1,
  RenderPass  = 2,
  ComputePass = 3,
  End         = 4
};

struct CmdBindDescriptorSets
{
  DescriptorSet ** sets                = nullptr;
  u32              count               = 0;
  u32 *            dynamic_offsets     = nullptr;
  u32              num_dynamic_offsets = 0;
};

struct CmdBindGraphicsPipeline
{
  GraphicsPipeline * pipeline = nullptr;
};

struct CmdPushConstants
{
  u8 * data = nullptr;
  u32  size = 0;
};

// can be split up into subcommands, this alone takes 112 bytes
struct CmdSetGraphicsState
{
  gpu::GraphicsState state{};
};

struct CmdBindVertexBuffer
{
  u32      binding = 0;
  Buffer * buffer  = nullptr;
  u64      offset  = 0;
};

struct CmdBindIndexBuffer
{
  Buffer *       buffer     = nullptr;
  u64            offset     = 0;
  gpu::IndexType index_type = gpu::IndexType::Uint32;
};

struct CmdDraw
{
  u32 vertex_count   = 0;
  u32 instance_count = 0;
  u32 first_vertex   = 0;
  u32 first_instance = 0;
};

struct CmdDrawIndexed
{
  u32 first_index    = 0;
  u32 num_indices    = 0;
  i32 vertex_offset  = 0;
  u32 first_instance = 0;
  u32 num_instances  = 0;
};

struct CmdDrawIndirect
{
  Buffer * buffer     = nullptr;
  u64      offset     = 0;
  u32      draw_count = 0;
  u32      stride     = 0;
};

struct CmdDrawIndexedIndirect
{
  Buffer * buffer     = nullptr;
  u64      offset     = 0;
  u32      draw_count = 0;
  u32      stride     = 0;
};

using Command =
  Enum<CmdBindDescriptorSets, CmdBindGraphicsPipeline, CmdPushConstants,
       CmdSetGraphicsState, CmdBindVertexBuffer, CmdBindIndexBuffer, CmdDraw,
       CmdDrawIndexed, CmdDrawIndirect, CmdDrawIndexedIndirect>;

struct RenderPassContext
{
  RectU render_area = {};
  u32   num_layers  = 0;
  gpu::RenderingAttachment
    color_attachments[gpu::MAX_PIPELINE_COLOR_ATTACHMENTS]            = {};
  gpu::RenderingAttachment depth_attachment[1]                        = {};
  gpu::RenderingAttachment stencil_attachment[1]                      = {};
  u32                      num_color_attachments                      = 0;
  u32                      num_depth_attachments                      = 0;
  u32                      num_stencil_attachments                    = 0;
  ArenaPool                arg_pool                                   = {};
  ArenaPool                command_pool                               = {};
  Vec<Command>             commands                                   = {};
  Buffer *                 vertex_buffers[gpu::MAX_VERTEX_ATTRIBUTES] = {};
  u32                      num_vertex_buffers                         = 0;
  Buffer *                 index_buffer                               = nullptr;
  gpu::IndexType           index_type          = gpu::IndexType::Uint16;
  u64                      index_buffer_offset = 0;
  GraphicsPipeline *       pipeline            = nullptr;
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
  DescriptorSet *   sets[gpu::MAX_PIPELINE_DESCRIPTOR_SETS] = {};
  u32               num_sets                                = 0;
  ComputePipeline * pipeline                                = nullptr;

  void reset()
  {
    num_sets = 0;
    pipeline = nullptr;
  }
};

struct CommandEncoder final : gpu::CommandEncoder
{
  AllocatorImpl       allocator         = {};
  Device *            dev               = nullptr;
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

  void validate_render_pass_compatible(gpu::GraphicsPipeline pipeline);

  void access_image_aspect(Image & image, VkPipelineStageFlags stages,
                           VkAccessFlags access, VkImageLayout layout,
                           gpu::ImageAspects aspects, u32 aspect_index);

  void access_buffer(Buffer & buffer, VkPipelineStageFlags stages,
                     VkAccessFlags access);

  void access_image_all_aspects(Image & image, VkPipelineStageFlags stages,
                                VkAccessFlags access, VkImageLayout layout);

  void access_image_depth(Image & image, VkPipelineStageFlags stages,
                          VkAccessFlags access, VkImageLayout layout)
  {
    access_image_aspect(image, stages, access, layout, gpu::ImageAspects::Depth,
                        DEPTH_ASPECT_IDX);
  }

  void access_image_stencil(Image & image, VkPipelineStageFlags stages,
                            VkAccessFlags access, VkImageLayout layout)
  {
    access_image_aspect(image, stages, access, layout,
                        gpu::ImageAspects::Stencil, STENCIL_ASPECT_IDX);
  }

  void access_compute_bindings(DescriptorSet const & set);

  void access_graphics_bindings(DescriptorSet const & set);

  void reset_context()
  {
    state = CommandEncoderState::Begin;
    render_ctx.reset();
    compute_ctx.reset();
  }

  virtual void reset_timestamp_query(gpu::TimeStampQuery query) override;

  virtual void reset_statistics_query(gpu::StatisticsQuery query) override;

  virtual void write_timestamp(gpu::TimeStampQuery query) override;

  virtual void begin_statistics(gpu::StatisticsQuery query) override;

  virtual void end_statistics(gpu::StatisticsQuery query) override;

  virtual void begin_debug_marker(Span<char const> region_name,
                                  Vec4             color) override;

  virtual void end_debug_marker() override;

  virtual void fill_buffer(gpu::Buffer dst, u64 offset, u64 size,
                           u32 data) override;

  virtual void copy_buffer(gpu::Buffer src, gpu::Buffer dst,
                           Span<gpu::BufferCopy const> copies) override;

  virtual void update_buffer(Span<u8 const> src, u64 dst_offset,
                             gpu::Buffer dst) override;

  virtual void
    clear_color_image(gpu::Image dst, gpu::Color value,
                      Span<gpu::ImageSubresourceRange const> ranges) override;

  virtual void clear_depth_stencil_image(
    gpu::Image dst, gpu::DepthStencil value,
    Span<gpu::ImageSubresourceRange const> ranges) override;

  virtual void copy_image(gpu::Image src, gpu::Image dst,
                          Span<gpu::ImageCopy const> copies) override;

  virtual void
    copy_buffer_to_image(gpu::Buffer src, gpu::Image dst,
                         Span<gpu::BufferImageCopy const> copies) override;

  virtual void blit_image(gpu::Image src, gpu::Image dst,
                          Span<gpu::ImageBlit const> blits,
                          gpu::Filter                filter) override;

  virtual void resolve_image(gpu::Image src, gpu::Image dst,
                             Span<gpu::ImageResolve const> resolves) override;

  virtual void begin_compute_pass() override;

  virtual void end_compute_pass() override;

  virtual void begin_rendering(gpu::RenderingInfo const & info) override;

  virtual void end_rendering() override;

  virtual void bind_compute_pipeline(gpu::ComputePipeline pipeline) override;

  virtual void bind_graphics_pipeline(gpu::GraphicsPipeline pipeline) override;

  virtual void
    bind_descriptor_sets(Span<gpu::DescriptorSet const> descriptor_sets,
                         Span<u32 const> dynamic_offsets) override;

  virtual void push_constants(Span<u8 const> push_constants_data) override;

  virtual void dispatch(u32 group_count_x, u32 group_count_y,
                        u32 group_count_z) override;

  virtual void dispatch_indirect(gpu::Buffer buffer, u64 offset) override;

  virtual void set_graphics_state(gpu::GraphicsState const & state) override;

  virtual void bind_vertex_buffers(Span<gpu::Buffer const> vertex_buffers,
                                   Span<u64 const>         offsets) override;

  virtual void bind_index_buffer(gpu::Buffer index_buffer, u64 offset,
                                 gpu::IndexType index_type) override;

  virtual void draw(u32 vertex_count, u32 instance_count, u32 first_vertex,
                    u32 first_instance) override;

  virtual void draw_indexed(u32 first_index, u32 num_indices, i32 vertex_offset,
                            u32 first_instance, u32 num_instances) override;

  virtual void draw_indirect(gpu::Buffer buffer, u64 offset, u32 draw_count,
                             u32 stride) override;

  virtual void draw_indexed_indirect(gpu::Buffer buffer, u64 offset,
                                     u32 draw_count, u32 stride) override;
};

struct FrameContext
{
  gpu::FrameId          tail_frame                          = 0;
  gpu::FrameId          current_frame                       = 0;
  u32                   ring_index                          = 0;
  u32                   buffering                           = 0;
  CommandEncoder        encs[gpu::MAX_FRAME_BUFFERING]      = {};
  gpu::CommandEncoder * encs_impl[gpu::MAX_FRAME_BUFFERING] = {};
  VkSemaphore           acquire_s[gpu::MAX_FRAME_BUFFERING] = {};
  VkFence               submit_f[gpu::MAX_FRAME_BUFFERING]  = {};
  VkSemaphore           submit_s[gpu::MAX_FRAME_BUFFERING]  = {};
  Swapchain *           swapchain                           = nullptr;
};

struct Device final : gpu::Device
{
  AllocatorImpl      allocator       = {};
  Instance *         instance        = nullptr;
  PhysicalDevice     phy_dev         = {};
  DeviceTable        vk_table        = {};
  VmaVulkanFunctions vma_table       = {};
  VkDevice           vk_dev          = nullptr;
  u32                queue_family    = 0;
  VkQueue            vk_queue        = nullptr;
  VmaAllocator       vma_allocator   = nullptr;
  FrameContext       frame_ctx       = {};
  DescriptorHeap     descriptor_heap = {};

  void set_resource_name(Span<char const> label, void const * resource,
                         VkObjectType               type,
                         VkDebugReportObjectTypeEXT debug_type);

  void uninit(DescriptorHeap * heap);

  VkResult recreate_swapchain(Swapchain * swapchain);

  Status init_command_encoder(CommandEncoder * enc);

  void uninit(CommandEncoder * enc);

  Status init_frame_context(u32 buffering);

  void uninit();

  virtual gpu::DeviceProperties get_device_properties() override;

  virtual Result<gpu::FormatProperties, Status>
    get_format_properties(gpu::Format format) override;

  virtual Result<gpu::Buffer, Status>
    create_buffer(gpu::BufferInfo const & info) override;

  virtual Result<gpu::BufferView, Status>
    create_buffer_view(gpu::BufferViewInfo const & info) override;

  virtual Result<gpu::Image, Status>
    create_image(gpu::ImageInfo const & info) override;

  virtual Result<gpu::ImageView, Status>
    create_image_view(gpu::ImageViewInfo const & info) override;

  virtual Result<gpu::Sampler, Status>
    create_sampler(gpu::SamplerInfo const & info) override;

  virtual Result<gpu::Shader, Status>
    create_shader(gpu::ShaderInfo const & info) override;

  virtual Result<gpu::DescriptorSetLayout, Status> create_descriptor_set_layout(
    gpu::DescriptorSetLayoutInfo const & info) override;

  virtual Result<gpu::DescriptorSet, Status>
    create_descriptor_set(gpu::DescriptorSetInfo const & info) override;

  virtual Result<gpu::PipelineCache, Status>
    create_pipeline_cache(gpu::PipelineCacheInfo const & info) override;

  virtual Result<gpu::ComputePipeline, Status>
    create_compute_pipeline(gpu::ComputePipelineInfo const & info) override;

  virtual Result<gpu::GraphicsPipeline, Status>
    create_graphics_pipeline(gpu::GraphicsPipelineInfo const & info) override;

  virtual Result<gpu::Swapchain, Status>
    create_swapchain(gpu::Surface               surface,
                     gpu::SwapchainInfo const & info) override;

  virtual Result<gpu::TimeStampQuery, Status> create_timestamp_query() override;

  virtual Result<gpu::StatisticsQuery, Status>
    create_statistics_query() override;

  virtual void uninit(gpu::Buffer buffer) override;

  virtual void uninit(gpu::BufferView buffer_view) override;

  virtual void uninit(gpu::Image image) override;

  virtual void uninit(gpu::ImageView image_view) override;

  virtual void uninit(gpu::Sampler sampler) override;

  virtual void uninit(gpu::Shader shader) override;

  virtual void uninit(gpu::DescriptorSetLayout layout) override;

  virtual void uninit(gpu::DescriptorSet set) override;

  virtual void uninit(gpu::PipelineCache cache) override;

  virtual void uninit(gpu::ComputePipeline pipeline) override;

  virtual void uninit(gpu::GraphicsPipeline pipeline) override;

  virtual void uninit(gpu::Swapchain swapchain) override;

  virtual void uninit(gpu::TimeStampQuery query) override;

  virtual void uninit(gpu::StatisticsQuery query) override;

  virtual gpu::FrameContext get_frame_context() override;

  virtual Result<void *, Status> map_buffer_memory(gpu::Buffer buffer) override;

  virtual void unmap_buffer_memory(gpu::Buffer buffer) override;

  virtual Result<Void, Status>
    invalidate_mapped_buffer_memory(gpu::Buffer      buffer,
                                    gpu::MemoryRange range) override;

  virtual Result<Void, Status>
    flush_mapped_buffer_memory(gpu::Buffer      buffer,
                               gpu::MemoryRange range) override;

  virtual Result<usize, Status>
    get_pipeline_cache_size(gpu::PipelineCache cache) override;

  virtual Result<Void, Status> get_pipeline_cache_data(gpu::PipelineCache cache,
                                                       Vec<u8> & out) override;

  virtual Result<Void, Status>
    merge_pipeline_cache(gpu::PipelineCache             dst,
                         Span<gpu::PipelineCache const> srcs) override;

  virtual void
    update_descriptor_set(gpu::DescriptorSetUpdate const & update) override;

  virtual Result<Void, Status> wait_idle() override;

  virtual Result<Void, Status> wait_queue_idle() override;

  virtual Result<Void, Status>
    get_surface_formats(gpu::Surface              surface,
                        Vec<gpu::SurfaceFormat> & formats) override;

  virtual Result<Void, Status>
    get_surface_present_modes(gpu::Surface            surface,
                              Vec<gpu::PresentMode> & modes) override;

  virtual Result<gpu::SurfaceCapabilities, Status>
    get_surface_capabilities(gpu::Surface surface) override;

  virtual Result<gpu::SwapchainState, Status>
    get_swapchain_state(gpu::Swapchain swapchain) override;

  virtual Result<Void, Status>
    invalidate_swapchain(gpu::Swapchain             swapchain,
                         gpu::SwapchainInfo const & info) override;

  virtual Result<Void, Status> begin_frame(gpu::Swapchain swapchain) override;

  virtual Result<Void, Status> submit_frame(gpu::Swapchain swapchain) override;

  virtual Result<u64, Status>
    get_timestamp_query_result(gpu::TimeStampQuery query) override;

  virtual Result<gpu::PipelineStatistics, Status>
    get_statistics_query_result(gpu::StatisticsQuery query) override;
};

}    // namespace vk
}    // namespace ash
