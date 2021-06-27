#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "stx/span.h"
#include "vlk/ui/input_pawn.h"
#include "vlk/ui/primitives.h"
#include "vlk/ui/vulkan_helpers.h"
#include "vulkan/vulkan.h"

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

#if !defined(SK_VULKAN)
#define SK_VULKAN
#endif

#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/vk/GrVkBackendContext.h"
#include "include/gpu/vk/GrVkExtensions.h"
#include "src/gpu/vk/GrVkRenderTarget.h"
#include "vk_mem_alloc.h"
#include "vlk/ui/trace.h"

#define VLK_DISABLE_COPY(target_type)       \
  target_type(target_type const&) = delete; \
  target_type& operator=(target_type const&) = delete;

#define VLK_DEFAULT_MOVE(target_type)   \
  target_type(target_type&&) = default; \
  target_type& operator=(target_type&&) = default;

#define VLK_MAKE_HANDLE(target_type) \
  VLK_DISABLE_COPY(target_type)      \
  VLK_DEFAULT_MOVE(target_type)

namespace vlk {
using namespace ui;
namespace vk {

struct WindowDesiredConfig {
  Extent extent = Extent{1920, 1080};
  std::string title = "Valkyrie App";
  bool resizable = true;
  bool maximized = false;
  VkCompositeAlphaFlagBitsKHR composite_alpha =
      VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  // TODO(lamarrr): log and format presentation modes
  VkPresentModeKHR present_mode[4] = {
      VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_FIFO_KHR,
      VK_PRESENT_MODE_FIFO_RELAXED_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR};
};

struct InstanceHandle {
  VkInstance instance = nullptr;
  stx::Option<VkDebugUtilsMessengerEXT> debug_messenger = stx::None;

  VLK_MAKE_HANDLE(InstanceHandle)

  ~InstanceHandle() {
    if (debug_messenger.is_some()) {
      destroy_debug_messenger(instance, debug_messenger.value(), nullptr);
    }
    if (instance != nullptr) vkDestroyInstance(instance, nullptr);
  }
};

struct Instance {
  static Instance create(std::string const& app_name, uint32_t app_version,
                         std::string const& engine_name,
                         uint32_t engine_version,
                         stx::Span<char const* const> required_extensions = {},
                         stx::Span<char const* const> validation_layers = {}) {
    auto handle = std::shared_ptr<InstanceHandle>(new InstanceHandle{});
    auto [instance, messenger] = create_vulkan_instance(
        required_extensions, validation_layers,
        make_debug_messenger_create_info(), app_name.c_str(), app_version,
        engine_name.c_str(), engine_version);

    // validation layers are extensions and might not be supported so we still
    // need to check for support
    handle->instance = instance;
    if (messenger != nullptr) {
      handle->debug_messenger = stx::Some(std::move(messenger));
    }
    return Instance{std::move(handle)};
  }

  std::shared_ptr<InstanceHandle const> handle;
};

struct PhysDeviceHandle {
  VkPhysicalDevice phys_device = nullptr;
  VkPhysicalDeviceProperties properties{};
  VkPhysicalDeviceFeatures features{};
  std::vector<VkQueueFamilyProperties> family_properties;
  Instance instance;
};

struct PhysDevice {
  static std::vector<PhysDevice> get_all(Instance const& instance) {
    uint32_t devices_count = 0;

    VLK_MUST_SUCCEED(vkEnumeratePhysicalDevices(instance.handle->instance,
                                                &devices_count, nullptr),
                     "Unable to get physical devices");

    VLK_ENSURE(devices_count != 0, "No Physical Device Found");

    std::vector<VkPhysicalDevice> physical_devices(devices_count);
    VLK_MUST_SUCCEED(
        vkEnumeratePhysicalDevices(instance.handle->instance, &devices_count,
                                   physical_devices.data()),
        "Unable to get physical devices");

    std::vector<PhysDevice> devices;

    for (VkPhysicalDevice device : physical_devices) {
      VkPhysicalDeviceProperties device_properties;
      VkPhysicalDeviceFeatures device_features;

      vkGetPhysicalDeviceProperties(device, &device_properties);
      vkGetPhysicalDeviceFeatures(device, &device_features);

      auto phys_device = PhysDevice{
          PhysDeviceHandle{device, device_properties, device_features,
                           get_queue_families(device), instance}};

      devices.push_back(std::move(phys_device));
    }

    return std::move(devices);
  }

  std::string_view format_type() const {
    switch (handle.properties.deviceType) {
      case VK_PHYSICAL_DEVICE_TYPE_CPU:
        return "CPU";
      case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
        return "dGPU";
      case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
        return "iGPU";
      case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
        return "vGPU";
      case VK_PHYSICAL_DEVICE_TYPE_OTHER:
        return "other";
      default:
        return "unknown";
    }
  }

  std::string format() const {
    auto const& properties = handle.properties;
    return fmt::format("Device(name: '{}', ID: {}, type: {}) ",
                       properties.deviceName, properties.deviceID,
                       format_type());
  }

  bool has_geometry_shader() const {
    auto const& features = handle.features;
    return features.geometryShader;
  }

  bool has_transfer_command_queue_family() const {
    return std::any_of(handle.family_properties.begin(),
                       handle.family_properties.end(),
                       [](VkQueueFamilyProperties const& prop) -> bool {
                         return prop.queueFlags & VK_QUEUE_TRANSFER_BIT;
                       });
  }

  bool has_graphics_command_queue_family() const {
    return std::any_of(handle.family_properties.begin(),
                       handle.family_properties.end(),
                       [](VkQueueFamilyProperties const& prop) -> bool {
                         return prop.queueFlags & VK_QUEUE_GRAPHICS_BIT;
                       });
  }

  std::string format_features() const {
    return fmt::format("Geometry Shader: {}", has_geometry_shader());
  }

  PhysDeviceHandle handle;
};

struct QueueInfo {
  uint32_t family_index = 0;
  VkQueue raw_handle = nullptr;
  float priority = 0.0f;
  uint32_t create_index = 0;
};

struct DeviceHandle {
  VkDevice device = nullptr;
  PhysDevice phys_device;
  std::vector<QueueInfo> command_queues;

  VLK_MAKE_HANDLE(DeviceHandle)

  ~DeviceHandle() {
    if (device != nullptr) vkDestroyDevice(device, nullptr);
  }
};

struct Device {
  static Device create(
      PhysDevice const& phys_device,
      stx::Span<VkDeviceQueueCreateInfo const> command_queue_create_info,
      stx::Span<char const* const> required_extensions = {},
      stx::Span<char const* const> required_validation_layers = {},
      VkPhysicalDeviceFeatures required_features = {}) {
    auto handle = std::shared_ptr<DeviceHandle>(new DeviceHandle{});
    handle->phys_device = phys_device;
    handle->device = create_logical_device(
        phys_device.handle.phys_device, required_extensions,
        required_validation_layers, command_queue_create_info, nullptr,
        required_features);

    for (size_t i = 0; i < command_queue_create_info.size(); i++) {
      auto create_info = command_queue_create_info[i];
      auto command_queue_family_index = create_info.queueFamilyIndex;
      auto queue_count = create_info.queueCount;
      VLK_ENSURE(command_queue_family_index <
                 phys_device.handle.family_properties.size());

      for (uint32_t queue_index = 0; queue_index < queue_count; queue_index++) {
        float priority = create_info.pQueuePriorities[i];
        VkQueue command_queue = get_command_queue(
            handle->device, command_queue_family_index, queue_index);
        handle->command_queues.push_back(QueueInfo{
            command_queue_family_index, command_queue, priority, queue_index});
      }
    }

    return std::move(Device{std::move(handle)});
  }

  std::shared_ptr<DeviceHandle> handle;
};

struct CommandQueueFamilyHandle {
  // automatically destroyed once the device is destroyed
  uint32_t index = 0;
  PhysDevice device;
};

struct CommandQueueFamily {
  // can also be used for transfer
  static stx::Option<CommandQueueFamily> get_graphics(
      PhysDevice const& phys_device) {
    auto pos = std::find_if(phys_device.handle.family_properties.begin(),
                            phys_device.handle.family_properties.end(),
                            [](VkQueueFamilyProperties const& prop) -> bool {
                              return prop.queueFlags & VK_QUEUE_GRAPHICS_BIT;
                            });

    if (pos == phys_device.handle.family_properties.end()) {
      return stx::None;
    }

    CommandQueueFamilyHandle handle;
    handle.index = pos - phys_device.handle.family_properties.begin();
    handle.device = phys_device;

    return stx::Some(CommandQueueFamily{std::move(handle)});
  }

  CommandQueueFamilyHandle handle;
};

struct CommandQueueHandle {
  // automatically destroyed once the device is destroyed
  VkQueue queue = nullptr;
  uint32_t index = 0;
  float priority = 0.0f;
  CommandQueueFamily family;
  Device device;
};

struct CommandQueue {
  static stx::Option<CommandQueue> get(Device const& device,
                                       CommandQueueFamily const& family,
                                       uint32_t command_queue_create_index) {
    // We shouldn't have to perform checks?
    VLK_ENSURE(device.handle->phys_device.handle.phys_device ==
               family.handle.device.handle.phys_device);

    auto pos = std::find_if(
        device.handle->command_queues.begin(),
        device.handle->command_queues.end(), [&](QueueInfo const& info) {
          return info.family_index == family.handle.index &&
                 info.create_index == command_queue_create_index;
        });

    if (pos == device.handle->command_queues.end()) {
      return stx::None;
    }

    auto handle = CommandQueueHandle{};

    handle.queue = pos->raw_handle;
    handle.family = CommandQueueFamily{CommandQueueFamilyHandle{
        pos->family_index, device.handle->phys_device}};
    handle.index = pos->create_index;
    handle.priority = pos->priority;
    handle.device = device;

    return stx::Some(CommandQueue{std::move(handle)});
  }

  CommandQueueHandle handle;
};

// skia doesn't support all surface formats, hence we have to provide formats or
// color types for it to convert to
struct WindowSurfaceFormat {
  VkFormat vk_format{};
  VkColorSpaceKHR vk_color_space{};
  SkColorType sk_color{};
  sk_sp<SkColorSpace> sk_color_space = nullptr;
};

// swapchains can not be headless, nor exist independently of the surface they
// originated from, its lifetime thus depends on the surface. the surface can
// and should be able to destroy and create it at will (which would be
// impossible with ref-counting)
// we thus can't hold a reference to the swapchain nor its images nor image
// views outside the object.
//
//
// Swapchains handle the presentation and update logic of the images to the
// window surface
//
//
//
// NOTE: all arguments to create a swapchain for a window surface are
// preferences, meaning another available argument will be used if the suggested
// ones are not supported. Thus do not assume your arguments are final.
struct WindowSwapChainHandle {
  // actually holds the images of the surface and used to present to the render
  // target image. when resizing is needed, the swapchain is destroyed and
  // recreated with the desired extents.
  VkSwapchainKHR swapchain = nullptr;
  WindowSurfaceFormat format{};
  VkPresentModeKHR present_mode{};
  Extent extent;

  static constexpr VkImageUsageFlags images_usage =
      VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
      VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

  static constexpr VkImageTiling images_tiling = VK_IMAGE_TILING_OPTIMAL;

  static constexpr VkSharingMode images_sharing_mode =
      VK_SHARING_MODE_EXCLUSIVE;

  static constexpr VkImageLayout images_initial_layout =
      VK_IMAGE_LAYOUT_UNDEFINED;

  uint32_t frame_flight_index = 0;

  // the images in the swapchain
  std::vector<VkImage> images;

  // the image views pointing to a part of a whole texture (images i the
  // swapchain)
  std::vector<VkImageView> image_views;

  // the rendering semaphores correspond to the frame indexes and not the
  // swapchain images
  std::vector<VkSemaphore> rendering_semaphores;

  std::vector<VkSemaphore> image_acquisition_semaphores;

  CommandQueue graphics_command_queue;

  // placed after graphics_command_queue to ensure it is deleted after it
  sk_sp<GrDirectContext> direct_context = nullptr;

  // NOTE: placed in this position so Skia can destroy its command queue
  std::vector<sk_sp<SkSurface>> skia_surfaces;

  VLK_MAKE_HANDLE(WindowSwapChainHandle)

  void destroy() {
    if (swapchain != nullptr) {
      // await idleness of the semaphores device, so we can destroy the
      // semaphore and images whislt not in use
      VLK_MUST_SUCCEED(
          vkDeviceWaitIdle(graphics_command_queue.handle.device.handle->device),
          "Unable to await device idleness");

      for (VkSemaphore semaphore : rendering_semaphores) {
        vkDestroySemaphore(graphics_command_queue.handle.device.handle->device,
                           semaphore, nullptr);
      }

      for (VkSemaphore semaphore : image_acquisition_semaphores) {
        vkDestroySemaphore(graphics_command_queue.handle.device.handle->device,
                           semaphore, nullptr);
      }

      for (VkImageView image_view : image_views) {
        vkDestroyImageView(graphics_command_queue.handle.device.handle->device,
                           image_view, nullptr);
      }
      // swapchain image is automatically deleted along with the swapchain
      vkDestroySwapchainKHR(graphics_command_queue.handle.device.handle->device,
                            swapchain, nullptr);
    }
  }
};

// choose a specific swapchain format available on the surface
inline WindowSurfaceFormat select_swapchain_surface_formats(
    stx::Span<VkSurfaceFormatKHR const> formats,
    stx::Span<WindowSurfaceFormat const> preferred_formats) noexcept {
  VLK_ENSURE(formats.size() != 0,
             "No window surface format supported by physical device");

  for (auto const& preferred_format : preferred_formats) {
    auto pos = std::find_if(
        formats.begin(), formats.end(), [&](VkSurfaceFormatKHR const& format) {
          return format.format == preferred_format.vk_format &&
                 format.colorSpace == preferred_format.vk_color_space;
        });

    if (pos != formats.end()) return preferred_format;
  }

  VLK_PANIC("Unable to find any of the preferred swapchain surface formats");
}

struct WindowSurfaceHandle {
  // only a pointer to metadata, does not contain data itself, resilient to
  // resizing
  VkSurfaceKHR surface = nullptr;

  // empty until change_swapchain is called
  WindowSwapChainHandle swapchain_handle;

  Instance instance;

  VLK_MAKE_HANDLE(WindowSurfaceHandle)

  void change_swapchain(
      CommandQueue const& command_queue,
      stx::Span<WindowSurfaceFormat const> preferred_formats,
      stx::Span<VkPresentModeKHR const> preferred_present_modes, Extent extent,
      VkCompositeAlphaFlagBitsKHR alpha_compositing,
      sk_sp<GrDirectContext> const& direct_context) {
    swapchain_handle.destroy();

    WindowSwapChainHandle new_handle{};

    new_handle.graphics_command_queue = command_queue;

    new_handle.direct_context = direct_context;

    auto const& phys_device =
        command_queue.handle.device.handle->phys_device.handle.phys_device;
    auto const& device = command_queue.handle.device.handle->device;

    // the properties change every time we need to create a swapchain so we must
    // query for this every time
    vlk::SwapChainProperties properties =
        get_swapchain_properties(phys_device, surface);

    VLK_LOG("Device Supported Formats:");
    for (VkSurfaceFormatKHR const& format : properties.supported_formats) {
      VLK_LOG("Format: {}, Color Space: {}", vlk::format(format.format),
              (int)(format.colorSpace));
    }

    // swapchain formats are device-dependent
    new_handle.format = select_swapchain_surface_formats(
        properties.supported_formats, preferred_formats);
    // log selections
    // swapchain presentation modes are device-dependent
    new_handle.present_mode = select_swapchain_presentation_mode(
        properties.presentation_modes, preferred_present_modes);

    uint32_t accessing_families[1] = {command_queue.handle.family.handle.index};

    auto [new_swapchain, actual_extent] = create_swapchain(
        device, surface, VkExtent2D{extent.width, extent.height},
        VkSurfaceFormatKHR{new_handle.format.vk_format,
                           new_handle.format.vk_color_space},
        new_handle.present_mode, properties,
        // not thread-safe since GPUs typically have one graphics queue
        WindowSwapChainHandle::images_sharing_mode, accessing_families,
        // render target image
        WindowSwapChainHandle::images_usage, alpha_compositing,
        // we don't care about the color of pixels that are obscured, for
        // example because another window is in front of them. Unless you really
        // need to be able to read these pixels back and get predictable
        // results, you'll get the best performance by enabling clipping.
        true);

    new_handle.swapchain = new_swapchain;
    new_handle.extent = Extent{actual_extent.width, actual_extent.height};
    new_handle.images = get_swapchain_images(device, new_handle.swapchain);

    for (VkImage image : new_handle.images) {
      VkImageView image_view = create_image_view(
          device, image, new_handle.format.vk_format, VK_IMAGE_VIEW_TYPE_2D,
          // use image view as color buffer (can be used as depth buffer)
          VK_IMAGE_ASPECT_COLOR_BIT, make_default_component_mapping());
      new_handle.image_views.push_back(image_view);
    }

    for (VkImage image : new_handle.images) {
      GrVkImageInfo image_info{};
      image_info.fImage = image;
      image_info.fImageTiling = WindowSwapChainHandle::images_tiling;
      image_info.fImageLayout = WindowSwapChainHandle::images_initial_layout;
      image_info.fFormat = new_handle.format.vk_format;
      image_info.fImageUsageFlags = WindowSwapChainHandle::images_usage;
      image_info.fSampleCount = 1;  // VK_SAMPLE_COUNT_1
      image_info.fLevelCount = 1;   // mip levels
      image_info.fSharingMode = WindowSwapChainHandle::images_sharing_mode;

      GrBackendRenderTarget backend_render_target(
          new_handle.extent.width, new_handle.extent.height, 1, image_info);
      SkSurfaceProps props{};

      auto sk_surface = SkSurface::MakeFromBackendRenderTarget(
          new_handle.direct_context.get(),   // context
          backend_render_target,             // backend render target
          kTopLeft_GrSurfaceOrigin,          // origin
          new_handle.format.sk_color,        // color type
          new_handle.format.sk_color_space,  // color space
          &props);                           // surface properties

      VLK_ENSURE(sk_surface != nullptr);
      new_handle.skia_surfaces.push_back(std::move(sk_surface));
    }

    for (size_t i = 0; i < new_handle.images.size(); i++) {
      new_handle.rendering_semaphores.push_back(create_semaphore(device));
      new_handle.image_acquisition_semaphores.push_back(
          create_semaphore(device));
    }

    swapchain_handle = std::move(new_handle);
  }

  void resize();

  ~WindowSurfaceHandle() {
    swapchain_handle.destroy();
    if (surface != nullptr) {
      vkDestroySurfaceKHR(instance.handle->instance, surface, nullptr);
    }
  }

  static VkPresentModeKHR select_swapchain_presentation_mode(
      stx::Span<VkPresentModeKHR const> available_presentation_modes,
      stx::Span<VkPresentModeKHR const> preferred_present_modes) noexcept {
    /*
   - VK_PRESENT_MODE_IMMEDIATE_KHR: Images submitted by your application are
   transferred to the screen right away, which may result in tearing.

   - VK_PRESENT_MODE_FIFO_KHR: The swap chain is a queue where the display takes
   an image from the front of the queue when the display is refreshed and the
   program inserts rendered images at the back of the queue. If the queue is
   full then the program has to wait. This is most similar to vertical sync as
   found in modern games. The moment that the display is refreshed is known as
   "vertical blank".

   - VK_PRESENT_MODE_FIFO_RELAXED_KHR: This mode only differs
   from the previous one if the application is late and the queue was empty at
   the last vertical blank. Instead of waiting for the next vertical blank, the
   image is transferred right away when it finally arrives. This may result in
   visible tearing.

   - VK_PRESENT_MODE_MAILBOX_KHR: This is another variation of the
   second mode. Instead of blocking the application when the queue is full, the
   images that are already queued are simply replaced with the newer ones. This
   mode can be used to implement triple buffering, which allows you to avoid
   tearing with significantly less latency issues than standard vertical sync
   that uses double buffering.
   */

    VLK_ENSURE(available_presentation_modes.size() != 0,
               "No surface presentation mode available");

    for (auto const& preferred_present_mode : preferred_present_modes) {
      auto it =
          std::find(available_presentation_modes.begin(),
                    available_presentation_modes.end(), preferred_present_mode);
      if (it != available_presentation_modes.end()) return *it;
    }

    VLK_PANIC("Unable to find any of the preferred presentation modes");
  }
};

struct WindowSurface {
  std::shared_ptr<WindowSurfaceHandle> handle = nullptr;
};

struct AllocatorHandle {
  VmaAllocator allocator = nullptr;
  Device device;

  VLK_MAKE_HANDLE(AllocatorHandle)

  ~AllocatorHandle() {
    if (allocator != nullptr) {
      vmaDestroyAllocator(allocator);
    }
  }
};

struct Allocator {
  static Allocator create(Device const& device) {
    VmaAllocatorCreateInfo info{};
    info.vulkanApiVersion =
        device.handle->phys_device.handle.properties.apiVersion;
    info.device = device.handle->device;
    info.physicalDevice = device.handle->phys_device.handle.phys_device;
    info.instance = device.handle->phys_device.handle.instance.handle->instance;

    auto handle = std::shared_ptr<AllocatorHandle>(new AllocatorHandle{});

    handle->device = device;

    VLK_MUST_SUCCEED(vmaCreateAllocator(&info, &handle->allocator),
                     "Unable to create allocator");

    return Allocator{std::move(handle)};
  }

  std::shared_ptr<AllocatorHandle> handle;
};

struct ImageHandle {
  VkImage image = nullptr;
  uint32_t queue_family = 0;
  VmaAllocation allocation = nullptr;
  Extent extent;

  Allocator allocator;

  VLK_MAKE_HANDLE(ImageHandle)

  ~ImageHandle() {
    if (image != nullptr) {
      vmaDestroyImage(allocator.handle->allocator, image, allocation);
    }
  }
};

// 2d Image
struct Image {
  // TODO(lamarrr): check and allow zero-sized image ?
  static stx::Option<Image> create(Allocator const& allocator,
                                   CommandQueueFamily const& family,
                                   VkFormat format, Extent extent) {
    if (!extent.visible()) return stx::None;

    auto const& phys_device =
        allocator.handle->device.handle->phys_device.handle;

    VkImageCreateInfo info{};

    // should we store the queue family?
    info.arrayLayers = 1;

    info.extent;
    info.extent.width = extent.width;
    info.extent.height = extent.height;
    info.extent.depth = 1;

    info.flags;

    info.format = format;
    info.imageType = VK_IMAGE_TYPE_2D;

    info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    info.mipLevels = 1;
    info.pNext = nullptr;
    info.pQueueFamilyIndices = &family.handle.index;
    info.queueFamilyIndexCount = 1;
    info.samples = VK_SAMPLE_COUNT_1_BIT;
    info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    info.tiling = VK_IMAGE_TILING_OPTIMAL;
    info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    VmaAllocationCreateInfo alloc_info{};

    auto handle = std::shared_ptr<ImageHandle>(new ImageHandle{});
    handle->allocator = allocator;
    handle->queue_family = family.handle.index;
    handle->extent = extent;

    VkResult result =
        vmaCreateImage(allocator.handle->allocator, &info, &alloc_info,
                       &handle->image, &handle->allocation, nullptr);

    if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
      return stx::None;
    }

    if (result == VK_SUCCESS) {
      return stx::Some(Image{std::move(handle)});
    }

    VLK_MUST_SUCCEED(result, "Unable to create image on device");
  }

  std::shared_ptr<ImageHandle> handle;
};

struct ImageViewHandle {
  VkImageView view = nullptr;
  Image image;

  VLK_MAKE_HANDLE(ImageViewHandle)

  ~ImageViewHandle() {
    if (view != nullptr) {
      vkDestroyImageView(image.handle->allocator.handle->device.handle->device,
                         view, nullptr);
    }
  }
};

struct ImageView {
  ImageViewHandle handle;
};

struct WindowApiHandle {
  WindowApiHandle() {
    VLK_ENSURE(glfwInit() == GLFW_TRUE, "Unable to initialize GLFW");
  }

  // needs to be deleted for all handle types
  VLK_MAKE_HANDLE(WindowApiHandle)

  ~WindowApiHandle() { glfwTerminate(); }
};

// not thread-safe, only one instance should be possible
// TODO(lamarrr): setup loggers, this needs a window api logger
struct WindowApi {
  std::shared_ptr<WindowApiHandle const> handle;

  WindowApi() {
    handle = std::shared_ptr<WindowApiHandle const>(new WindowApiHandle{});
    glfwSetErrorCallback(error_callback);
  }

  stx::Span<char const* const> get_required_instance_extensions() const {
    uint32_t size = 0;
    char const** extensions = glfwGetRequiredInstanceExtensions(&size);
    return stx::Span<char const*>(extensions, size);
  }

  // this isn't thread-safe, we need to sync it
  // timeout
  template <typename Rep, typename Period>
  void await_events(std::chrono::duration<Rep, Period> timeout) const {
    glfwWaitEventsTimeout(
        timeout.count() *
        (std::chrono::seconds(1).count() /
         (std::chrono::duration<Rep, Period>{1}.count() * 1.0)));
  }

  void await_events() const { glfwWaitEvents(); }

  static void error_callback(int error_code, char const* error_description) {
    VLK_ERR("Error: {} (code : {})", error_description,
            glfw_error_to_str(error_code));
  }

  static constexpr std::string_view glfw_error_to_str(int error_code) {
    switch (error_code) {
      VLK_ERRNUM_CASE(GLFW_NO_ERROR)
      VLK_ERRNUM_CASE(GLFW_NOT_INITIALIZED)
      VLK_ERRNUM_CASE(GLFW_NO_CURRENT_CONTEXT)
      VLK_ERRNUM_CASE(GLFW_INVALID_ENUM)
      VLK_ERRNUM_CASE(GLFW_INVALID_VALUE)
      VLK_ERRNUM_CASE(GLFW_OUT_OF_MEMORY)
      VLK_ERRNUM_CASE(GLFW_API_UNAVAILABLE)
      VLK_ERRNUM_CASE(GLFW_VERSION_UNAVAILABLE)
      VLK_ERRNUM_CASE(GLFW_PLATFORM_ERROR)
      VLK_ERRNUM_CASE(GLFW_FORMAT_UNAVAILABLE)
      VLK_ERRNUM_CASE(GLFW_NO_WINDOW_CONTEXT)
      default:
        return "Unidentified Error";
    }
  }
};

struct WindowEventDispatchQueue {
  std::vector<MouseEvent> mouse_events;

  // used for detecting double-clicks
  std::vector<std::chrono::nanoseconds> mouse_event_timestamps;
  std::vector<KeyboardEvent> keyboard_events;

  void add_raw(MouseEvent event) { mouse_events.push_back(event); }
  void add_raw(KeyboardEvent event) { keyboard_events.push_back(event); }

  void clear() {
    mouse_events.clear();
    keyboard_events.clear();
  }
};

struct WindowHandle {
  GLFWwindow* window = nullptr;
  bool extent_dirty = true;
  Extent extent = Extent{0, 0};
  bool surface_extent_dirty = true;
  Extent surface_extent = Extent{0, 0};
  WindowSurface surface;
  WindowApi api;
  WindowDesiredConfig cfg;
  WindowEventDispatchQueue event_queue;
  std::vector<InputPawn*> input_pawns;

  VLK_MAKE_HANDLE(WindowHandle)

  void resize(Extent new_extent) {
    extent_dirty = true;
    extent = new_extent;
  }

  void resize_surface(Extent new_extent) {
    surface_extent_dirty = true;
    surface_extent = new_extent;
  }

  void maximize() {}

  ~WindowHandle() {
    if (window != nullptr) {
      glfwDestroyWindow(window);
    }
  }

  bool tick(CommandQueue const& queue, sk_sp<GrDirectContext> const& context) {
    if (extent_dirty || surface_extent_dirty) {
      VLK_LOG("Changing swapchain");

      {
        int width = 0, height = 0;
        glfwGetWindowSize(window, &width, &height);
        resize(Extent{static_cast<uint32_t>(width),
                      static_cast<uint32_t>(height)});
      }

      {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        resize_surface(Extent{static_cast<uint32_t>(width),
                              static_cast<uint32_t>(height)});
      }

      WindowSurfaceFormat preferred_formats[] = {
          {VK_FORMAT_R8G8B8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
           kRGBA_8888_SkColorType, SkColorSpace::MakeSRGB()},
          {VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
           kRGBA_8888_SkColorType, SkColorSpace::MakeSRGB()},
          {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
           kBGRA_8888_SkColorType, SkColorSpace::MakeSRGB()},
          {VK_FORMAT_R16G16B16A16_SFLOAT, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
           kRGBA_F16_SkColorType, SkColorSpace::MakeSRGBLinear()}};

      {
        VLK_TRACE(Render, "Swapchain Recreation");

        surface.handle->change_swapchain(queue, preferred_formats,
                                         cfg.present_mode, surface_extent,
                                         cfg.composite_alpha, context);
      }

      extent_dirty = false;
      surface_extent_dirty = false;
    }

    // We submit multiple render commands (operating on the swapchain images) to
    // the GPU to prevent having to force a sync with the GPU (await_fence) when
    // it could be doing useful work.
    VkDevice device = surface.handle->swapchain_handle.graphics_command_queue
                          .handle.device.handle->device;

    // TODO(lamarrr): document reason for difference in indexes
    VkSemaphore image_acquisition_semaphore =
        surface.handle->swapchain_handle.image_acquisition_semaphores
            [surface.handle->swapchain_handle.frame_flight_index];

    auto [next_image, acquire_result] = acquire_next_swapchain_image(
        device, surface.handle->swapchain_handle.swapchain,
        image_acquisition_semaphore, nullptr);

    if (acquire_result == VK_SUBOPTIMAL_KHR ||
        acquire_result == VK_ERROR_OUT_OF_DATE_KHR) {
      extent_dirty = true;
      surface_extent_dirty = true;

      return false;
    }

    auto& sk_surface =
        surface.handle->swapchain_handle.skia_surfaces[next_image];

    // if the previously submitted images from the previous swapchain image
    // rendering iteration is not done yet, then perform an expensive
    // GPU-CPU synchronization

    // Since GrDirectContext has and retains internal state, wont that affect
    // this?
    sk_surface->flushAndSubmit(true);

    GrBackendSemaphore gr_image_acquisition_semaphore{};
    gr_image_acquisition_semaphore.initVulkan(image_acquisition_semaphore);

    VLK_ENSURE(sk_surface->wait(1, &gr_image_acquisition_semaphore, false));

    auto canvas = sk_surface->getCanvas();
    canvas->clear(SK_ColorTRANSPARENT);
    SkFont font;
    SkPaint paint;

    font.setSubpixel(true);
    font.setSize(20);
    paint.setColor(SK_ColorYELLOW);

    constexpr char const text[] = "hello bitch";
    canvas->drawSimpleText(text, std::size(text) - 1, SkTextEncoding::kUTF8,
                           200, 200, font, paint);

    // rendering
    VkSemaphore rendering_semaphore =
        surface.handle->swapchain_handle.rendering_semaphores
            [surface.handle->swapchain_handle.frame_flight_index];

    GrBackendSemaphore gr_rendering_semaphore{};
    gr_rendering_semaphore.initVulkan(rendering_semaphore);

    GrFlushInfo flush_info{};
    flush_info.fNumSemaphores = 1;
    flush_info.fSignalSemaphores = &gr_rendering_semaphore;

    GrBackendSurfaceMutableState target_presentation_surface_state{
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        surface.handle->swapchain_handle.graphics_command_queue.handle.family
            .handle.index};

    VLK_ENSURE(
        sk_surface->flush(flush_info, &target_presentation_surface_state) ==
        GrSemaphoresSubmitted::kYes);

    VLK_ENSURE(surface.handle->swapchain_handle.direct_context->submit(false));

    // presentation
    // TODO(lamarrr): find a way to coalesce direct context and command queue
    // (into SkiaContext?)
    // (we don't need to wait on presentation)
    auto present_result =
        present(queue.handle.queue,
                stx::Span<VkSemaphore const>(&rendering_semaphore, 1),
                stx::Span<VkSwapchainKHR const>(
                    &surface.handle->swapchain_handle.swapchain, 1),
                stx::Span<uint32_t const>(&next_image, 1));

    if (acquire_result == VK_SUBOPTIMAL_KHR ||
        acquire_result == VK_ERROR_OUT_OF_DATE_KHR) {
      extent_dirty = true;
      surface_extent_dirty = true;

      return false;
    }

    surface.handle->swapchain_handle.frame_flight_index =
        (surface.handle->swapchain_handle.frame_flight_index + 1) %
        surface.handle->swapchain_handle.images.size();
    VLK_LOG("Rendered frame {}",
            surface.handle->swapchain_handle.frame_flight_index);

    return true;
  }
};

struct Window {
  std::shared_ptr<WindowHandle> handle;

  /* static void monitor_callback(GLFWmonitor* monitor, int event) {
     GLFW_CONNECTED;
     GLFW_DISCONNECTED;
   }
   */

  static stx::Option<Window> create(WindowApi const& api,
                                    Instance const& instance,
                                    WindowDesiredConfig const& cfg) {
    glfwWindowHint(GLFW_CLIENT_API,
                   GLFW_NO_API);  // not an OpenGL app, do not create or use
                                  // OpenGL context

    glfwWindowHint(
        GLFW_RESIZABLE,
        cfg.resizable);  // requires handling the framebuffer/surface resizing

    // TODO(lamarrr): missing initializers
    std::shared_ptr<WindowHandle> handle{new WindowHandle{}};

    // width and height here refer to the screen coordinates and not the
    // actual pixel coordinates (cc: Device Pixel Ratio)

    GLFWwindow* window = glfwCreateWindow(
        static_cast<int>(std::clamp<uint32_t>(cfg.extent.width, 0, i32_max)),
        static_cast<int>(std::clamp<uint32_t>(cfg.extent.height, 0, i32_max)),
        cfg.title.c_str(), /* monitor */ nullptr,
        /* window to share context with */ nullptr);

    if (window == nullptr) return stx::None;

    handle->window = window;

    glfwSetWindowUserPointer(handle->window, handle.get());

    glfwSetWindowAttrib(handle->window, GLFW_RESIZABLE, cfg.resizable);

    glfwSetWindowSizeCallback(handle->window, Window::resize_callback);
    glfwSetFramebufferSizeCallback(handle->window,
                                   Window::surface_resize_callback);
    glfwSetMouseButtonCallback(handle->window, Window::mouse_button_callback);

    // glfwGetWindowMonitor
    // we might need to handle disconnection/connection of monitors
    GLFWmonitor* primary_monitor = glfwGetPrimaryMonitor();
    VLK_ENSURE(primary_monitor != nullptr, "No monitors found");
    GLFWvidmode const* primary_monitor_video_mode =
        glfwGetVideoMode(primary_monitor);
    VLK_ENSURE(primary_monitor_video_mode != nullptr,
               "Unable to get monitor video mode");

    // makes the window into windowed mode if primary_monitor is not null
    // otherwise into non-windowed mode
    // setting should otherwise only be used during monitor setting
    glfwSetWindowMonitor(window, primary_monitor, 0, 0, 1920, 1080,
                         primary_monitor_video_mode->refreshRate);

    // might need to be updated
    VLK_LOG("Monitor is at {}hz", primary_monitor_video_mode->refreshRate);

    handle->cfg = cfg;

    WindowSurface surface;
    surface.handle =
        std::shared_ptr<WindowSurfaceHandle>(new WindowSurfaceHandle{});
    surface.handle->instance = instance;

    // creates and binds the window surface (back buffer, render target
    // surface) to the glfw window
    VLK_MUST_SUCCEED(
        glfwCreateWindowSurface(instance.handle->instance, handle->window,
                                nullptr, &surface.handle->surface),
        "Unable to Create Window Surface");

    handle->surface = std::move(surface);

    return stx::Some(Window{std::move(handle)});
  }

  void publish_events() {
    for (InputPawn* pawn : handle->input_pawns) {
      InputPawnSystemProxy::system_tick(*pawn, handle->event_queue.mouse_events,
                                        handle->event_queue.keyboard_events);
    }
  }

  static void resize_callback(GLFWwindow* window, int width, int height) {
    auto handle = static_cast<WindowHandle*>(glfwGetWindowUserPointer(window));
    handle->resize(
        Extent{static_cast<uint32_t>(width), static_cast<uint32_t>(height)});
  }

  static void surface_resize_callback(GLFWwindow* window, int width,
                                      int height) {
    auto handle = static_cast<WindowHandle*>(glfwGetWindowUserPointer(window));
    handle->resize_surface(
        Extent{static_cast<uint32_t>(width), static_cast<uint32_t>(height)});
  }

  static void cursor_position_callback(GLFWwindow* window, double x, double y) {
    // measured in screen coordinates
  }

  static void mouse_button_callback(GLFWwindow* window, int button, int action,
                                    int modifier) {
    auto handle = static_cast<WindowHandle*>(glfwGetWindowUserPointer(window));

    MouseEvent event;

    switch (button) {
      case GLFW_MOUSE_BUTTON_LEFT:
        event.button = MouseButton::Primary;
        break;
      case GLFW_MOUSE_BUTTON_RIGHT:
        event.button = MouseButton::Secondary;
        break;
      case GLFW_MOUSE_BUTTON_MIDDLE:
        event.button = MouseButton::Middle;
        break;
      case GLFW_MOUSE_BUTTON_4:
        event.button = MouseButton::A1;
        break;
      case GLFW_MOUSE_BUTTON_5:
        event.button = MouseButton::A2;
        break;
      case GLFW_MOUSE_BUTTON_6:
        event.button = MouseButton::A3;
        break;
      case GLFW_MOUSE_BUTTON_7:
        event.button = MouseButton::A4;
        break;
      case GLFW_MOUSE_BUTTON_8:
        event.button = MouseButton::A5;
        break;
      default:
        VLK_PANIC("Unimplemented mouse button", static_cast<int>(button));
    }

    switch (action) {
      case GLFW_PRESS:
        event.action = MouseAction::Press;
        break;
      case GLFW_RELEASE:
        event.action = MouseAction::Release;
        break;
      default:
        VLK_PANIC("Unimplemented mouse action", static_cast<int>(action));
    }

    if (modifier & GLFW_MOD_SHIFT) {
      event.modifiers |= MouseModifier::Shift;
    }

    if (modifier & GLFW_MOD_CONTROL) {
      event.modifiers |= MouseModifier::Ctrl;
    }

    if (modifier & GLFW_MOD_ALT) {
      event.modifiers |= MouseModifier::Alt;
    }

    if (modifier & GLFW_MOD_SUPER) {
      event.modifiers |= MouseModifier::Super;
    }

    if (modifier & GLFW_MOD_CAPS_LOCK) {
      event.modifiers |= MouseModifier::CapsLock;
    }

    if (modifier & GLFW_MOD_NUM_LOCK) {
      event.modifiers |= MouseModifier::NumLock;
    }

    handle->event_queue.add_raw(event);
  }

  static void scroll_callback(GLFWwindow* window, double x, double y) {}

  bool should_close() const { return glfwWindowShouldClose(handle->window); }

  void request_close() const { glfwSetWindowShouldClose(handle->window, true); }

  void request_attention() const { glfwRequestWindowAttention(handle->window); }

 private:
  /*
clipboardstring get/set utf8
  glfwSetWindowSizeLimits
  glfwSetWindowPosCallback
  glfwsetscrollcallback
  glfwSetWindowCloseCallback
  glfwSetWindowRefreshCallback
  glfwSetWindowFocusCallback
  glfwSetWindowIconifyCallback
  glfwSetWindowMaximizeCallback
  glfwSetWindowContentScaleCallback


  */
  void fn() {
    glfwSetCursorPosCallback(handle->window, cursor_position_callback);
    glfwSetScrollCallback(handle->window, scroll_callback);
  }
};

enum class LogTarget : uint8_t { File = 1, Std = 2 };

VLK_DEFINE_ENUM_BIT_OPS(LogTarget);

struct AppOptions {
  // log options
  // redirect some to file
  LogTarget window_log = LogTarget::File;
};

struct App {
  WindowApi api;

  static constexpr char const* required_validation_layers[] = {
      "VK_LAYER_KHRONOS_validation"};
  static constexpr char const* required_device_extensions[] = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME};

  static stx::Option<PhysDevice> select_device(
      stx::Span<PhysDevice const> const physical_devices,
      stx::Span<VkPhysicalDeviceType const> preferred_device_types,
      WindowSurface const& target_surface) {
    for (auto type : preferred_device_types) {
      auto selected_device_it = std::find_if(
          physical_devices.begin(), physical_devices.end(),
          [&](PhysDevice const& dev) -> bool {
            return dev.handle.properties.deviceType == type &&
                   // can use shaders (fragment and vertex)
                   dev.has_geometry_shader() &&
                   // has graphics command queue for rendering commands
                   dev.has_graphics_command_queue_family() &&
                   // has data transfer command queue for uploading textures
                   // or data
                   dev.has_transfer_command_queue_family() &&
                   // can be used for presenting to a specific surface
                   any_true(get_surface_presentation_command_queue_support(
                       dev.handle.phys_device, dev.handle.family_properties,
                       target_surface.handle->surface));
          });
      if (selected_device_it != physical_devices.end()) {
        return stx::Some(PhysDevice{*selected_device_it});
      }
    }

    return stx::None;
  }

  void start() {
    auto const required_instance_extensions =
        api.get_required_instance_extensions();
    char const* const required_device_extensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    auto instance =
        Instance::create("TestApp", VK_MAKE_VERSION(0, 0, 1), "Valkyrie",
                         VK_MAKE_VERSION(1, 0, 0), required_instance_extensions,
                         required_validation_layers);

    auto phys_devices = PhysDevice::get_all(instance);

    VkPhysicalDeviceType const device_preference[] = {
        VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,
        VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU,
        VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU,
        VK_PHYSICAL_DEVICE_TYPE_CPU,
    };

    WindowDesiredConfig cfg{};
    cfg.maximized = false;

    Window window =
        Window::create(api, instance, cfg).expect("Unable to create window");

    VLK_LOG("Available Physical Devices:");

    for (PhysDevice const& device : phys_devices) {
      VLK_LOG("\t{}", device.format());
    }

    PhysDevice phys_device =
        select_device(phys_devices, device_preference, window.handle->surface)
            .expect("Unable to find any suitable rendering device");

    VLK_LOG("Selected Physical Device: {}", phys_device.format());

    auto const& features = phys_device.handle.features;

    // enable sampler anisotropy if available
    VkPhysicalDeviceFeatures required_features{};

    required_features.samplerAnisotropy = features.samplerAnisotropy;

    // we need multiple command queues, one for data transfer and one for
    // rendering
    float const priorities[] = {// priority for command queue used for
                                // presentation, rendering, data transfer
                                1.0f};
    uint32_t command_queue_index = 0;

    CommandQueueFamily graphic_command_queue_family =
        CommandQueueFamily::get_graphics(phys_device).unwrap();

    // we can accept queue family struct here instead and thus not have to
    // perform extra manual checks
    // the user shouldn't have to touch handles
    VkDeviceQueueCreateInfo const command_queue_create_infos[] = {
        make_command_queue_create_info(
            graphic_command_queue_family.handle.index, priorities)};

    Device device = Device::create(
        phys_device, command_queue_create_infos, required_device_extensions,
        required_validation_layers, required_features);

    CommandQueue graphics_command_queue =
        CommandQueue::get(device, graphic_command_queue_family, 0)
            .expect("Failed to create graphics command queue");

    {
      // how do we send pixels over from SKia to Vulkan window?
      GrVkBackendContext vk_context{};

      GrVkExtensions extensions_cache{};
      vk_context.fVkExtensions = &extensions_cache;
      vk_context.fInstance = instance.handle->instance;
      vk_context.fPhysicalDevice = phys_device.handle.phys_device;
      vk_context.fDevice = device.handle->device;
      vk_context.fQueue = graphics_command_queue.handle.queue;
      vk_context.fGraphicsQueueIndex = graphics_command_queue.handle.index;
      vk_context.fMaxAPIVersion = VK_API_VERSION_1_1;
      vk_context.fDeviceFeatures = &features;
      // vk_context.fMemoryAllocator
      vk_context.fGetProc = [](char const* proc_name, VkInstance instance,
                               VkDevice device) {
        VLK_ENSURE(instance == nullptr || device == nullptr);
        VLK_ENSURE(!(instance != nullptr && device != nullptr));
        if (device != nullptr) {
          return vkGetDeviceProcAddr(device, proc_name);
        } else {
          return vkGetInstanceProcAddr(instance, proc_name);
        }
      };

      auto direct_context = GrDirectContext::MakeVulkan(vk_context);
      VLK_ENSURE(direct_context != nullptr,
                 "Unable to create Skia Direct Vulkan Context");

      auto allocator = Allocator::create(device);

      auto image = Image::create(allocator, graphic_command_queue_family,
                                 VK_FORMAT_R8G8B8A8_UINT, Extent{250, 250})
                       .unwrap();

      auto prev_tick_tp = std::chrono::steady_clock::now();
      while (!window.should_close()) {
        auto present_tick_tp = std::chrono::steady_clock::now();
        auto interval = present_tick_tp - prev_tick_tp;
        prev_tick_tp = present_tick_tp;
        // we need to get frame budget and use diff between it and the used time
        window.publish_events();  // defer the events into the widget system via
                                  // the pawn
                                  // process widget invalidation and events
        while (!window.handle->tick(graphics_command_queue, direct_context)) {
        }
        window.handle->api.await_events();
      }
    }
  }
};

/*


ENSURE(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) == 0, "Unable to init SDL");
  auto sdl_window = SDL_CreateWindow(
      "SDL Vulkan Sample", 0, 0, 1920, 1080,
      SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

  ENSURE(sdl_window != nullptr, "Unable to init SDL window");

  uint32_t extensions_count = 0;
  std::vector<char const *> extensions;
  ENSURE(SDL_Vulkan_GetInstanceExtensions(sdl_window, &extensions_count,
                                          nullptr) == SDL_TRUE,
         "unex");

  extensions.resize(extensions_count);

  ENSURE(SDL_Vulkan_GetInstanceExtensions(sdl_window, &extensions_count,
                                          extensions.data()) == SDL_TRUE,
         "unex");

  SDL_Event event{};
  while (true) {
    ENSURE(SDL_WaitEvent(&event) == 1, "Wait");
    if (event.type == SDL_QUIT) break;
    if (event.type == SDL_MOUSEBUTTONUP || event.type == SDL_MOUSEBUTTONDOWN) {
      if (event.button.clicks == 2) {
        std::cout << "double click" << std::endl;
        SDL_FlashWindow(sdl_window, 10);
      }
    }
    if (event.type == SDL_MOUSEMOTION) {
      std::cout << "x: " << event.motion.x << ", y: " << event.motion.y
                << std::endl;
    }
  }

  SDL_DestroyWindow(sdl_window);

  SDL_Quit();



*/




}  // namespace vk
}  // namespace vlk
