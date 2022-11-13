#pragma once

#include <memory>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "ashura/primitives.h"
#include "ashura/utils.h"
#include "ashura/vulkan.h"
#include "stx/vec.h"

namespace asr {

enum class WindowSwapchainDiff : u8 {
  None = 0,
  // the window's extent and surface (framebuffer) extent has changed
  Extent = 1,
  // the window swapchain can still be used for presentation but is not optimal
  // for presentation in its present state
  Suboptimal = 2,
  // the window swapchain is now out of date and needs to be changed
  OutOfDate = 4,
  All = Extent | Suboptimal | OutOfDate
};

STX_DEFINE_ENUM_BIT_OPS(WindowSwapchainDiff)

// choose a specific swapchain format available on the surface
inline VkSurfaceFormatKHR select_swapchain_surface_formats(
    stx::Span<VkSurfaceFormatKHR const> formats,
    stx::Span<VkSurfaceFormatKHR const> preferred_formats) {
  ASR_ENSURE(!formats.is_empty(),
             "No window surface format supported by physical device");

  for (VkSurfaceFormatKHR preferred_format : preferred_formats) {
    if (!formats
             .which([&](VkSurfaceFormatKHR format) {
               return preferred_format.colorSpace == format.colorSpace &&
                      preferred_format.format == format.format;
             })
             .is_empty())
      return preferred_format;
  }

  ASR_PANIC("Unable to find any of the preferred swapchain surface formats");
}

inline VkPresentModeKHR select_swapchain_presentation_mode(
    stx::Span<VkPresentModeKHR const> available_presentation_modes,
    stx::Span<VkPresentModeKHR const> preferred_present_modes) noexcept {
  /// - VK_PRESENT_MODE_IMMEDIATE_KHR: Images submitted by your application
  /// are transferred to the screen right away, which may result in tearing.
  ///
  /// - VK_PRESENT_MODE_FIFO_KHR: The swap chain is a queue where the
  /// display takes an image from the front of the queue when the display is
  /// refreshed and the program inserts rendered images at the back of the
  /// queue. If the queue is full then the program has to wait. This is most
  /// similar to vertical sync as found in modern games. The moment that the
  /// display is refreshed is known as "vertical blank" (v-sync).
  ///
  /// - VK_PRESENT_MODE_FIFO_RELAXED_KHR: This mode only differs
  /// from the previous one if the application is late and the queue was
  /// empty at the last vertical blank. Instead of waiting for the next
  /// vertical blank, the image is transferred right away when it finally
  /// arrives. This may result in visible tearing.
  ///
  /// - VK_PRESENT_MODE_MAILBOX_KHR: This is another variation of the
  /// second mode. Instead of blocking the application when the queue is
  /// full, the images that are already queued are simply replaced with the
  /// newer ones. This mode can be used to implement triple buffering, which
  /// allows you to avoid tearing with significantly less latency issues
  /// than standard vertical sync that uses double buffering.

  ASR_ENSURE(!available_presentation_modes.is_empty(),
             "No surface presentation mode available");

  for (auto const& preferred_present_mode : preferred_present_modes) {
    if (!available_presentation_modes.find(preferred_present_mode).is_empty())
      return preferred_present_mode;
  }

  ASR_PANIC("Unable to find any of the preferred presentation modes");
}

/// Swapchains handle the presentation and update logic of the images to the
/// window surface.
///
///
///
/// NOTE: all arguments to create a swapchain for a window surface are
/// preferences, meaning another available argument will be used if the
/// suggested ones are not supported. Thus do not assume your arguments are
/// final.
///
///
/// swapchains can not be headless, nor exist independently of the surface they
/// originated from, its lifetime thus depends on the surface. the surface can
/// and should be able to destroy and create it at will (which would be
/// impossible to do correctly with ref-counting, since we are not holding a
/// reference to the surface) we thus can't hold a reference to the swapchain,
/// its images, nor its image views outside itself (the swapchain object).
///
struct WindowSwapChain {
  static constexpr VkImageUsageFlags IMAGES_USAGE =
      VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
      VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

  static constexpr VkImageTiling IMAGES_TILING = VK_IMAGE_TILING_OPTIMAL;

  static constexpr VkSharingMode IMAGES_SHARING_MODE =
      VK_SHARING_MODE_EXCLUSIVE;

  static constexpr VkImageLayout IMAGES_INITIAL_LAYOUT =
      VK_IMAGE_LAYOUT_UNDEFINED;

  static constexpr VkCompositeAlphaFlagBitsKHR COMPOSITE_ALPHA =
      VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

  // TODO(lamarrr): log and format presentation modes
  static constexpr VkPresentModeKHR PRESENT_MODES[4] = {
      VK_PRESENT_MODE_FIFO_RELAXED_KHR, VK_PRESENT_MODE_FIFO_KHR,
      VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR};

  // actually holds the images of the surface and used to present to the render
  // target image. when resizing is needed, the swapchain is destroyed and
  // recreated with the desired extents.
  VkSwapchainKHR swapchain = VK_NULL_HANDLE;
  VkSurfaceFormatKHR format{VK_FORMAT_R8G8B8A8_SRGB,
                            VK_COLORSPACE_SRGB_NONLINEAR_KHR};
  VkPresentModeKHR present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
  Extent extent;

  /// IMPORTANT: this is different from the image index obtained via
  /// `vkAcquireNextImageKHR`. this index is used for referencing semaphores
  /// used for submitting and querying rendering operations. this value is
  /// always increasing and wrapping, unlike the index obtained from
  /// `vkAcquireNextImageKHR` which depends on the presentation mode being used
  /// (determines how the images are used, in what order and whether they
  /// repeat).
  uint32_t frame_flight_index = 0;

  // the images in the swapchain
  stx::Vec<VkImage> images;

  // the image views pointing to a part of a whole texture (images in the
  // swapchain)
  stx::Vec<VkImageView> image_views;

  // the rendering semaphores correspond to the frame indexes and not the
  // swapchain images
  stx::Vec<VkSemaphore> rendering_semaphores;

  stx::Vec<VkSemaphore> image_acquisition_semaphores;

  stx::Rc<vkh::Device*> device;

  stx::Rc<vkh::CommandQueueFamilyInfo*> accessing_family;

  WindowSwapChain(VkSwapchainKHR aswapchain, VkSurfaceFormatKHR aformat,
                  VkPresentModeKHR apresent_mode, Extent aextent,
                  uint32_t aframe_flight_index, stx::Vec<VkImage> aimages,
                  stx::Vec<VkImageView> aimage_views,
                  stx::Vec<VkSemaphore> arendering_semaphores,
                  stx::Vec<VkSemaphore> aimage_acquisition_semaphores,
                  stx::Rc<vkh::Device*> adevice,
                  stx::Rc<vkh::CommandQueueFamilyInfo*> aaccessing_family)
      : swapchain{aswapchain},
        format{aformat},
        present_mode{apresent_mode},
        extent{aextent},
        frame_flight_index{aframe_flight_index},
        images{std::move(aimages)},
        image_views{std::move(aimage_views)},
        rendering_semaphores{std::move(arendering_semaphores)},
        image_acquisition_semaphores{std::move(aimage_acquisition_semaphores)},
        device{std::move(adevice)},
        accessing_family{std::move(aaccessing_family)} {}

  STX_MAKE_PINNED(WindowSwapChain)

  ~WindowSwapChain() {
    VkDevice dev = device.handle->device;

    // await idleness of the semaphores device, so we can destroy the
    // semaphore and images whislt not in use.
    // any part of the device could be using the semaphore
    ASR_MUST_SUCCEED(vkDeviceWaitIdle(dev), "Unable to await device idleness");

    for (VkSemaphore semaphore : rendering_semaphores) {
      vkDestroySemaphore(dev, semaphore, nullptr);
    }

    for (VkSemaphore semaphore : image_acquisition_semaphores) {
      vkDestroySemaphore(dev, semaphore, nullptr);
    }

    for (VkImageView image_view : image_views) {
      vkDestroyImageView(dev, image_view, nullptr);
    }

    // swapchain image is automatically deleted along with the swapchain
    vkDestroySwapchainKHR(dev, swapchain, nullptr);
  }
};

struct WindowSurface {
  // only a pointer to metadata, does not contain data itself, resilient to
  // resizing
  VkSurfaceKHR surface = VK_NULL_HANDLE;

  // empty and invalid until change_swapchain is called.
  // not ref-counted since it solely belongs to this surface and the surface can
  // create and destroy it upon request.
  //
  // also, we need to be certain it is non-existent and not referring to any
  // resources when destroyed, not just by calling a method to destroy its
  // resources.
  //
  stx::Option<stx::Unique<WindowSwapChain*>> swapchain;

  stx::Rc<vkh::Instance*> instance;

  WindowSurface(VkSurfaceKHR asurface,
                stx::Option<stx::Unique<WindowSwapChain*>> aswapchain,
                stx::Rc<vkh::Instance*> ainstance)
      : surface{asurface},
        swapchain{std::move(aswapchain)},
        instance{std::move(ainstance)} {}

  STX_MAKE_PINNED(WindowSurface)

  ~WindowSurface() {
    // we need to ensure the swapchain is destroyed before the surface (if not
    // already destroyed)
    swapchain = stx::None;

    vkDestroySurfaceKHR(instance.handle->instance, surface, nullptr);
  }

  void change_swapchain(
      stx::Rc<vkh::Device*> const& dev,
      stx::Rc<vkh::CommandQueueFamilyInfo*> const& family,
      stx::Span<VkSurfaceFormatKHR const> preferred_formats,
      stx::Span<VkPresentModeKHR const> preferred_present_modes, Extent extent,
      VkCompositeAlphaFlagBitsKHR alpha_compositing) {
    ASR_ENSURE(dev.handle->phy_device.handle->phy_device ==
               family.handle->phy_device.handle->phy_device);

    swapchain = stx::None;  // probably don't want to have two existing at once

    VkPhysicalDevice phy_device = dev.handle->phy_device.handle->phy_device;
    VkDevice device = dev.handle->device;

    // the properties change every time we need to create a swapchain so we must
    // query for this every time
    vk::SwapChainProperties properties =
        vk::get_swapchain_properties(phy_device, surface);

    ASR_LOG("Device Supported Surface Formats:");
    for (VkSurfaceFormatKHR const& format : properties.supported_formats) {
      ASR_LOG("\tFormat: {}, Color Space: {}", vk::format(format.format),
              vk::format(format.colorSpace));
    }

    // swapchain formats are device-dependent
    VkSurfaceFormatKHR format = select_swapchain_surface_formats(
        properties.supported_formats, preferred_formats);
    // TODO(lamarrr): log selections
    // swapchain presentation modes are device-dependent
    VkPresentModeKHR present_mode = select_swapchain_presentation_mode(
        properties.presentation_modes, preferred_present_modes);

    uint32_t accessing_families[] = {family.handle->index};

    auto [new_swapchain_r, actual_extent] = vk::create_swapchain(
        device, surface, VkExtent2D{extent.w, extent.h}, format, present_mode,
        properties,
        // not thread-safe since GPUs typically have one graphics queue
        WindowSwapChain::IMAGES_SHARING_MODE, accessing_families,
        // render target image
        WindowSwapChain::IMAGES_USAGE, alpha_compositing,
        // we don't care about the color of pixels that are obscured, for
        // example because another window is in front of them. Unless you really
        // need to be able to read these pixels back and get predictable
        // results, you'll get the best performance by enabling clipping.
        true);

    swapchain =
        stx::Some(stx::rc::make_unique_inplace<WindowSwapChain>(
                      stx::os_allocator, new_swapchain_r, format, present_mode,
                      Extent{actual_extent.width, actual_extent.height},
                      static_cast<uint32_t>(0),
                      vk::get_swapchain_images(device, new_swapchain_r),
                      stx::Vec<VkImageView>{stx::os_allocator},
                      stx::Vec<VkSemaphore>{stx::os_allocator},
                      stx::Vec<VkSemaphore>{stx::os_allocator}, dev.share(),
                      family.share())
                      .unwrap());

    auto& swapchain_r = *swapchain.value().handle;

    for (VkImage image : swapchain_r.images) {
      VkImageView image_view = vk::create_image_view(
          device, image, format.format, VK_IMAGE_VIEW_TYPE_2D,
          // use image view as color buffer (can be used as depth buffer)
          VK_IMAGE_ASPECT_COLOR_BIT,
          VkComponentMapping{// how to map the image color components
                             .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                             .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                             .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                             .a = VK_COMPONENT_SWIZZLE_IDENTITY});
      swapchain_r.image_views.push_inplace(image_view).unwrap();
    }

    for (usize i = 0; i < swapchain_r.images.size(); i++) {
      swapchain_r.rendering_semaphores.push(vk::create_semaphore(device))
          .unwrap();
      swapchain_r.image_acquisition_semaphores
          .push(vk::create_semaphore(device))
          .unwrap();
    }
  }
};

}  // namespace asr
