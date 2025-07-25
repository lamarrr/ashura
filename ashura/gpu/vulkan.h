/// SPDX-License-Identifier: MIT
#pragma once
#define VMA_STATIC_VULKAN_FUNCTIONS  0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
#define VMA_VULKAN_VERSION           1'000'000

#include "ashura/gpu/gpu.h"
#include "ashura/std/allocator.h"
#include "ashura/std/allocators.h"
#include "ashura/std/range.h"
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
inline constexpr u32 NUM_DESCRIPTOR_TYPES       = 11;

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

struct Access
{
  VkPipelineStageFlags stages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
  VkAccessFlags        access = VK_ACCESS_NONE;
};

/*
enum class AliasState : u8
{
  /// the resource is the active resource using the aliased memory region, i.e. no other resource that overlaps the same memory region is currently using part of the memory
  Resident  = 0,
  /// the resource is not the active resource using its allocated memory region, i.e. another resource that overlaps the same memory region is using part of the memory
  Displaced = 1
};
*/

// [ ] we can merge this hazard states and move them into the memorygroup abstraction; it will enable us track aliases? by binding id?

struct DescriptorSet;

struct Binder
{
  DescriptorSet * set     = nullptr;
  u32             binding = 0;
  u32             element = 0;
};

constexpr u32 MAX_RESOURCE_BINDERS = 16;

struct MemoryGroup;

struct MemoryInfo
{
  MemoryGroup *   memory_group  = nullptr;
  u32             group_binding = 0;
  u32             alias_binding = 0;
  gpu::MemoryType type          = gpu::MemoryType::Unique;
};

struct BufferBarrier
{
  VkPipelineStageFlags  src_stages = VK_PIPELINE_STAGE_NONE;
  VkPipelineStageFlags  dst_stages = VK_PIPELINE_STAGE_NONE;
  VkBufferMemoryBarrier barrier    = {};
};

struct ImageBarrier
{
  VkPipelineStageFlags src_stages = VK_PIPELINE_STAGE_NONE;
  VkPipelineStageFlags dst_stages = VK_PIPELINE_STAGE_NONE;
  VkImageMemoryBarrier barrier    = {};
};

struct MemoryBarrier
{
  VkPipelineStageFlags src_stages = VK_PIPELINE_STAGE_NONE;
  VkPipelineStageFlags dst_stages = VK_PIPELINE_STAGE_NONE;
  VkMemoryBarrier      barrier    = {};
};

// [ ] use an array and a slot system for the binders
struct BufferState
{
  MemoryInfo                               memory  = {};
  InplaceVec<Binder, MAX_RESOURCE_BINDERS> binders = {};
};

struct ImageState
{
  MemoryInfo                               memory  = {};
  InplaceVec<Binder, MAX_RESOURCE_BINDERS> binders = {};
};

struct Buffer
{
  gpu::BufferInfo info      = {};
  VkBuffer        vk_buffer = nullptr;
  BufferState     state     = {};

  Option<BufferBarrier> sync(Access request, u64 pass_timestamp);
};

struct BufferView
{
  gpu::BufferViewInfo info    = {};
  VkBufferView        vk_view = nullptr;
};

struct Image
{
  gpu::ImageInfo info               = {};
  bool           is_swapchain_image = false;
  VkImage        vk_image           = nullptr;
  ImageState     state              = {};

  Option<ImageBarrier> sync(gpu::ImageSubresourceRange const & range,
                            Access request, VkImageLayout layout,
                            u64 pass_timestamp);
};

// [ ] sorted insert and remove
// [ ] if new resource is binded whilst others have been placed? how will sync work?
// [ ] store list of written-to resources so we know which to be tracked?; will also help with sets if propagated with it

/// @brief An allocated block of memory that can be aliased by multiple resources.
struct MemoryGroup
{
  VmaAllocation vma_allocation = nullptr;
  u64           alignment      = 0;
  void *        map            = nullptr;
  SmallVec<u64> alias_offsets  = {};
  SmallVec<u32> alias_bindings = {};

  // [ ] reclaim the memory range used by the buffer and invalidate all the binding resources that overlap/alias the same memory range
  void reclaim(Buffer *);

  void reclaim(Image *);
};

struct ImageView
{
  gpu::ImageViewInfo info    = {};
  VkImageView        vk_view = nullptr;
};

struct DescriptorSetLayout
{
  VkDescriptorSetLayout vk_layout = nullptr;

  InplaceVec<gpu::DescriptorBindingInfo, gpu::MAX_DESCRIPTOR_SET_BINDINGS>
    bindings = {};

  Array<u32, NUM_DESCRIPTOR_TYPES> sizing = {};

  u32 num_variable_length = 0;
};

using SyncResources =
  Enum<None, Vec<Tuple<Buffer *, u32>>, Vec<Tuple<BufferView *, u32>>,
       Vec<Tuple<ImageView *, u32>>>;

/// used to track stateful resource access
/// @param images only valid if `type` is a descriptor type that access images
/// @param param buffers: only valid if `type` is a descriptor type that access
/// buffers
struct DescriptorBinding
{
  SyncResources       sync_resources = none;
  gpu::DescriptorType type           = gpu::DescriptorType::Sampler;

  u32 size() const
  {
    return sync_resources.match(
      [](None) { return (u32) 0; }, [](auto & v) { return size32(v); },
      [](auto & v) { return size32(v); }, [](auto & v) { return size32(v); });
  }
};

struct DescriptorSet
{
  VkDescriptorSet vk_set = nullptr;

  VkDescriptorPool vk_pool = nullptr;

  InplaceVec<DescriptorBinding, gpu::MAX_DESCRIPTOR_SET_BINDINGS> bindings = {};

  /*
  {

  // [ ] fix
  // [ ] needs binding system; store binding slots in resources and let the binders allocate from it
  // [ ] remove old binding; if non-null
  // [ ] add new binding
  // if same slot continue; if not bind
  // [ ] if(buffer == binding.sync_resources[el + i])  buffer might already be added, if it is the same as the present slot, we need to remove it? it can be binded-to multiple times
  // [ ] alternate method?

    BufferView *& current = sync_resources[v2][binder.element];

    // remove old binder
    if (current != nullptr)
    {
      auto loc = find(current->state.binders.view(), binder, obj::byte_eq)
                   .as_slice_of(current->state.binders.view());
      if (!loc.is_empty())
      {
        current->state.binders.erase(loc);
      }
    }

    // update binder
    current = next;
    current->state.binders.push(binder).unwrap();
  }*/

  /*{
    ImageView *& current = sync_resources[v1][binder.element];

    if (current != nullptr)
    {
      auto loc =
        find(current->state.aspects[state].binders.view(), binder, obj::byte_eq)
          .as_slice_of(current->state.aspects[state].binders.view());
      if (!loc.is_empty())
      {
        current->state.aspects[state].binders.erase(loc);
      }
    }

    current = next;
    current->state.aspects[state].binders.push(binder).unwrap();
  }*.*/

  void update_link(Buffer * next, u32 binding, u32 element)
  {
  }

  void update_link(BufferView * next, u32 binding, u32 element)
  {
  }

  void update_link(ImageView * next, u32 binding, u32 element)
  {
  }

  static void remove_binder(InplaceVec<Binder, MAX_RESOURCE_BINDERS> & binders,
                            DescriptorSet *                            set)
  {
    auto old = binders;
    binders.clear();
    // [ ] this sucks
    for (auto const & binder : old)
    {
      if (binder.set == set)
      {
        continue;
      }
      binders.push(binder).unwrap();
    }
  }
};

struct ComputePipeline
{
  VkPipeline vk_pipeline = nullptr;

  VkPipelineLayout vk_layout = nullptr;

  u32 push_constants_size = 0;

  u32 num_sets = 0;
};

struct GraphicsPipeline
{
  VkPipeline vk_pipeline = nullptr;

  VkPipelineLayout vk_layout = nullptr;

  u32 push_constants_size = 0;

  u32 num_sets = 0;

  InplaceVec<gpu::Format, gpu::MAX_PIPELINE_COLOR_ATTACHMENTS> color_fmts = {};

  Option<gpu::Format> depth_fmt = none;

  Option<gpu::Format> stencil_fmt = none;

  gpu::SampleCount sample_count = gpu::SampleCount::C1;
};

struct Instance final : gpu::Instance
{
  AllocatorRef allocator = {};

  InstanceTable vk_table = {};

  VkInstance vk_instance = nullptr;

  VkDebugUtilsMessengerEXT vk_debug_messenger = nullptr;

  bool validation_enabled = false;

  explicit Instance() = default;

  Instance(Instance const &)             = delete;
  Instance & operator=(Instance const &) = delete;
  Instance(Instance &&)                  = delete;
  Instance & operator=(Instance &&)      = delete;
  virtual ~Instance() override;

  virtual Result<gpu::Device *, Status>
    create_device(AllocatorRef                allocator,
                  Span<gpu::DeviceType const> preferred_types,
                  u32                         buffering) override;

  virtual gpu::Backend get_backend() override;

  virtual void uninit(gpu::Device * device) override;

  virtual void uninit(gpu::Surface surface) override;
};

struct PhysicalDevice
{
  VkPhysicalDevice vk_phy_dev = nullptr;

  VkPhysicalDeviceFeatures vk_features = {};

  VkPhysicalDeviceProperties vk_properties = {};

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
  gpu::SwapchainInfo info = {};

  bool is_out_of_date = true;

  bool is_optimal = false;

  bool is_zero_sized = false;

  gpu::SurfaceFormat format = {};

  gpu::ImageUsage usage = gpu::ImageUsage::None;

  gpu::PresentMode present_mode = gpu::PresentMode::Immediate;

  u32x2 extent = {};

  gpu::CompositeAlpha composite_alpha = gpu::CompositeAlpha::None;

  InplaceVec<Image, gpu::MAX_SWAPCHAIN_IMAGES> image_impls = {};

  InplaceVec<gpu::Image, gpu::MAX_SWAPCHAIN_IMAGES> images = {};

  InplaceVec<VkImage, gpu::MAX_SWAPCHAIN_IMAGES> vk_images = {};

  u32 current_image = 0;

  VkSwapchainKHR vk_swapchain = nullptr;

  VkSurfaceKHR vk_surface = nullptr;
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
  PinVec<DescriptorSet *> sets            = {};
  PinVec<u32>             dynamic_offsets = {};
};

struct CmdBindGraphicsPipeline
{
  GraphicsPipeline * pipeline = nullptr;
};

struct CmdPushConstants
{
  PinVec<u8> constant = {};
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

  u32 num_layers = 0;

  InplaceVec<gpu::RenderingAttachment, gpu::MAX_PIPELINE_COLOR_ATTACHMENTS>
    color_attachments = {};

  Option<gpu::RenderingAttachment> depth_attachment = none;

  Option<gpu::RenderingAttachment> stencil_attachment = none;

  ArenaPool arg_pool = {};

  ArenaPool command_pool = {};

  Vec<Command> commands = {};

  InplaceVec<Buffer *, gpu::MAX_VERTEX_ATTRIBUTES> vertex_buffers = {};

  Buffer * index_buffer = nullptr;

  gpu::IndexType index_type = gpu::IndexType::Uint16;

  u64 index_buffer_offset = 0;

  GraphicsPipeline * pipeline = nullptr;

  bool has_state = false;

  void clear()
  {
    render_area = {};
    num_layers  = 0;
    color_attachments.clear();
    depth_attachment   = none;
    stencil_attachment = none;
    commands.reset();
    command_pool.reclaim();
    arg_pool.reclaim();
    vertex_buffers.clear();
    index_buffer        = nullptr;
    index_buffer_offset = 0;
    pipeline            = nullptr;
    has_state           = false;
  }
};

struct ComputePassContext
{
  InplaceVec<DescriptorSet *, gpu::MAX_PIPELINE_DESCRIPTOR_SETS> sets = {};
  ComputePipeline * pipeline                                          = nullptr;

  void clear()
  {
    sets.clear();
    pipeline = nullptr;
  }
};

namespace experimental
{

enum class ImageId : u16
{
  Undefined = U16_MAX
};

enum class BufferId : u16
{
  Undefined = U16_MAX
};

enum class DescriptorSetId : u16
{
  Undefined = U16_MAX
};

enum class AliasId : u16
{
  Undefined = U16_MAX
};

enum class HazardType : u8
{
  None            = 0,
  Write           = 1,
  Reads           = 2,
  ReadsAfterWrite = 3
};

struct BufferHazard
{
  u32        pass  = 0;
  HazardType type  = HazardType::None;
  Access     reads = {};
  Access     write = {};
};

struct ImageHazard
{
  u32           pass   = 0;
  HazardType    type   = HazardType::None;
  Access        reads  = {};
  Access        write  = {};
  VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
};

struct ImageAccess
{
  VkPipelineStageFlags stages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
  VkAccessFlags        access = VK_ACCESS_NONE;
  VkImageLayout        layout = VK_IMAGE_LAYOUT_UNDEFINED;
};

struct BufferAccess
{
  VkPipelineStageFlags stages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
  VkAccessFlags        access = VK_ACCESS_NONE;
};

struct AliasAccess
{
};

/// @brief Multiple aliases can be binded to the same memory alias and read from it at the same time.
/// but they can't write to it at the same time.
// [ ] will this tracking work?
struct AliasHazard
{
  u32                           pass   = 0;
  Enum<None, BufferId, ImageId> reader = none;
  Enum<None, BufferId, ImageId> writer = none;
};

// [ ] fix alias tracking
// [ ] how to track memory aliases
// [ ] how to track descriptor set bindings

/// @brief Descriptor set hazard state
struct DescriptorSetHazard
{
  u32  pass       = 0;
  // [ ] if an hazard has occured on one of its bindings' elements
  bool has_hazard = false;
};

/*
/// @brief resources need timeline ids as their ids will change across frames when resources are removed
constexpr u16 MAX_TIMELINES = 16;

struct ImageIds
{
  ImageId timelines[MAX_TIMELINES] = {};
  ImageId id = ImageId::Undefined;
};

struct BufferIds
{
  BufferId timelines[MAX_TIMELINES] = {};
  BufferId id = BufferId::Undefined;
};

struct DescriptorSetIds
{
  // DescriptorSetId timelines[MAX_TIMELINES] = {};
  DescriptorSetId id = DescriptorSetId::Undefined;
};
*/

/// @brief Per-encoder resource state heap
struct ResourceStateHeap
{
  CoreSparseMap<u16, Vec<VkImageLayout>> image_states     = {};
  CoreSparseMap<u16, Vec<AliasId>>       image_alias_map  = {};
  CoreSparseMap<u16, Vec<AliasId>>       buffer_alias_map = {};
  // memory group
};

/// @brief Per-encoder resource hazard state heap
struct ResourceHazardHeap
{
  template <typename... T>
  using Map = CoreSparseMap<u16, BitVec<u64>, Vec<T>...>;

  Map<Enum<ImageId, BufferId>> alias_state            = {};
  Map<ImageHazard>             image_hazards          = {};
  Map<BufferHazard>            buffer_hazards         = {};
  Map<AliasHazard>             alias_hazards          = {};
  Map<DescriptorSetHazard>     descriptor_set_hazards = {};

  void mark(ImageId, ImageAccess const &);
  void mark(BufferId, BufferAccess const &);
  void mark(AliasId, AliasAccess const &);
  void mark(DescriptorSetId);
};

struct AccessTimeline
{
  struct Pass
  {
    Slice32 descriptor_sets           = {};
    Slice32 vertex_buffers            = {};
    Slice32 index_buffers             = {};
    Slice32 indirect_buffers          = {};
    Slice32 color_attachments         = {};
    Slice32 depth_stencil_attachments = {};
    Slice32 transfer_src_images       = {};
    Slice32 transfer_dst_images       = {};
    Slice32 transfer_src_buffers      = {};
    Slice32 transfer_dst_buffers      = {};
  };

  // [ ] alias?

  Vec<DescriptorSetId> descriptor_sets           = {};
  Vec<BufferId>        vertex_buffers            = {};
  Vec<BufferId>        index_buffers             = {};
  Vec<BufferId>        indirect_buffers          = {};
  Vec<ImageId>         color_attachments         = {};
  Vec<ImageId>         depth_stencil_attachments = {};
  Vec<ImageId>         transfer_src_images       = {};
  Vec<ImageId>         transfer_dst_images       = {};
  Vec<BufferId>        transfer_src_buffers      = {};
  Vec<BufferId>        transfer_dst_buffers      = {};
  Vec<Pass>            passes                    = {};

  void begin_pass();
  void end_pass();
  void access_descriptor_set(DescriptorSetId);
  void access_vertex_buffer(BufferId);
  void access_index_buffer(BufferId);
  void access_indirect_buffer(BufferId);
  void access_color_image(ImageId);
  void access_depth_stencil_image(ImageId);
  void access_transfer_src_image(ImageId);
  void access_transfer_dst_image(ImageId);
  void access_transfer_src_buffer(ImageId);
  void access_transfer_dst_buffer(ImageId);
};

struct TransferCmd
{
};

struct TransferPass
{
};

struct ComputeCmd
{
};

struct ComputePass
{
  Slice32 commands;
};

struct RenderCmd
{
};

struct RenderPass
{
  Slice32 commands;
};

}    // namespace experimental

struct AccessEncoder
{
  struct RenderPass
  {
    // these will determine the expected state and layout of each item
    // DescriptorSetLayout * layout;
    // DescriptorSet * set;

    // { set bindings: sampler }
    // { set bindings: sampledimage, combinedimage, texelbuffer, read-only storagebuffer, input attachments }
    // {  Set{ set bindings: storage image, storage texel buffer, read-write storage buffer } }
    // { output attachments: color, depth, stencil }
    // { output resolve attachments: color, depth, stencil }
    //
    //
    // needs to emit barriers in-between render passes as they might overlap write access
  };

  struct ComputePass
  {
    // Set { read set bindings }
    // Set { write set bindings }
    //
    // issue barrier immediately after render pass
  };

  struct TransferPass
  {
    // Set { read resources }
    // Set { write resources }
  };
};

struct CommandEncoder final : gpu::CommandEncoder
{
  AllocatorRef        allocator         = {};
  Device *            dev               = nullptr;
  ArenaPool           arg_pool          = {};
  VkCommandPool       vk_command_pool   = nullptr;
  VkCommandBuffer     vk_command_buffer = nullptr;
  Status              status            = Status::Success;
  CommandEncoderState state             = CommandEncoderState::Reset;
  RenderPassContext   render_ctx        = {};
  ComputePassContext  compute_ctx       = {};
  u64                 pass_timestamp    = 1;

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

  void access_image(Image & image, VkPipelineStageFlags stages,
                    VkAccessFlags access, VkImageLayout layout,
                    u64 pass_timestamp);

  void access_buffer(Buffer & buffer, VkPipelineStageFlags stages,
                     VkAccessFlags access, u64 pass_timestamp);

  void insert_barrier(ImageBarrier const & barrier);

  void insert_barrier(BufferBarrier const & barrier);

  void insert_barrier(MemoryBarrier const & barrier);

  void access_compute_bindings(DescriptorSet const & set, u64 pass_timestamp);

  void access_graphics_bindings(DescriptorSet const & set, u64 pass_timestamp);

  void clear_context()
  {
    state = CommandEncoderState::Begin;
    render_ctx.clear();
    compute_ctx.clear();
  }

  virtual void reset_timestamp_query(gpu::TimeStampQuery query,
                                     Slice32             range) override;

  virtual void reset_statistics_query(gpu::StatisticsQuery query,
                                      Slice32              range) override;

  virtual void write_timestamp(gpu::TimeStampQuery query,
                               gpu::PipelineStages stage, u32 index) override;

  virtual void begin_statistics(gpu::StatisticsQuery query, u32 index) override;

  virtual void end_statistics(gpu::StatisticsQuery query, u32 index) override;

  virtual void begin_debug_marker(Str region_name, f32x4 color) override;

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

struct CommandBuffer final : gpu::CommandBuffer
{
  Device *            dev               = nullptr;
  VkCommandPool       vk_command_pool   = nullptr;
  VkCommandBuffer     vk_command_buffer = nullptr;
  Status              status            = Status::Success;
  CommandEncoderState state             = CommandEncoderState::Reset;

  virtual void reset_timestamp_query(gpu::TimeStampQuery query,
                                     Slice32             range) override;

  virtual void reset_statistics_query(gpu::StatisticsQuery query,
                                      Slice32              range) override;

  virtual void write_timestamp(gpu::TimeStampQuery query,
                               gpu::PipelineStages stage, u32 index) override;

  virtual void begin_statistics(gpu::StatisticsQuery query, u32 index) override;

  virtual void end_statistics(gpu::StatisticsQuery query, u32 index) override;

  virtual void begin_debug_marker(Str region_name, f32x4 color) override;

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
  gpu::FrameId tail_frame = 0;

  gpu::FrameId current_frame = 0;

  u32 ring_index = 0;

  InplaceVec<CommandEncoder, gpu::MAX_FRAME_BUFFERING> encoders = {};

  InplaceVec<gpu::CommandEncoder *, gpu::MAX_FRAME_BUFFERING> encoders_impl =
    {};

  InplaceVec<VkSemaphore, gpu::MAX_FRAME_BUFFERING> acquire_semaphores = {};

  InplaceVec<VkFence, gpu::MAX_FRAME_BUFFERING> submit_fences = {};

  InplaceVec<VkSemaphore, gpu::MAX_FRAME_BUFFERING> submit_semaphores = {};

  Swapchain * swapchain = nullptr;

  u32 buffering() const
  {
    return encoders.size();
  }
};

struct Device final : gpu::Device
{
  AllocatorRef       allocator      = {};
  Instance *         instance       = nullptr;
  PhysicalDevice     phy_dev        = {};
  DeviceTable        vk_table       = {};
  VmaVulkanFunctions vma_table      = {};
  VkDevice           vk_dev         = nullptr;
  u32                queue_family   = 0;
  VkQueue            vk_queue       = nullptr;
  VmaAllocator       vma_allocator  = nullptr;
  FrameContext       frame_ctx      = {};
  Vec<u8>            scratch        = {};
  u64                pass_timestamp = 1;

  void set_resource_name(Str label, void const * resource, VkObjectType type,
                         VkDebugReportObjectTypeEXT debug_type);

  VkResult recreate_swapchain(Swapchain * swapchain);

  Status init_command_encoder(CommandEncoder * enc);

  void uninit(CommandEncoder * enc);

  Status init_frame_context(u32 buffering);

  void uninit();

  virtual gpu::DeviceProperties get_properties() override;

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

  virtual Result<gpu::MemoryGroup, Status>
    create_memory_group(gpu::MemoryGroupInfo const & info) override;

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

  virtual Result<gpu::TimeStampQuery, Status>
    create_timestamp_query(u32 count) override;

  virtual Result<gpu::StatisticsQuery, Status>
    create_statistics_query(u32 count) override;

  virtual void uninit(gpu::Buffer buffer) override;

  virtual void uninit(gpu::BufferView buffer_view) override;

  virtual void uninit(gpu::Image image) override;

  virtual void uninit(gpu::ImageView image_view) override;

  virtual void uninit(gpu::MemoryGroup memory_group) override;

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

  virtual Result<Span<u8>, Status> get_memory_map(gpu::Buffer buffer) override;

  virtual Result<Void, Status> invalidate_mapped_memory(gpu::Buffer buffer,
                                                        Slice64 range) override;

  virtual Result<Void, Status> flush_mapped_memory(gpu::Buffer buffer,
                                                   Slice64     range) override;

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

  virtual Result<Void, Status>
    get_timestamp_query_result(gpu::TimeStampQuery query, Slice32 range,
                               Vec<u64> & timestamps) override;

  virtual Result<Void, Status> get_statistics_query_result(
    gpu::StatisticsQuery query, Slice32 range,
    Vec<gpu::PipelineStatistics> & statistics) override;
};

}    // namespace vk
}    // namespace ash
