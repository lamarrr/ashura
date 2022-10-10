#pragma once

#include <memory>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "SDL_vulkan.h"
#include "asura/primitives.h"
#include "asura/sdl_utils.h"
#include "asura/utils.h"
#include "asura/vulkan.h"
#include "asura/window_event_queue.h"
#include "stx/vec.h"

namespace asr {

enum class WindowSwapchainDiff : uint8_t {
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

STX_DEFINE_ENUM_BIT_OPS(WindowSwapchainDiff);

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
  VkSwapchainKHR swapchain = nullptr;
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

  stx::Option<stx::Rc<vk::Device*>> device;

  ASR_MAKE_HANDLE(WindowSwapChain)

  ~WindowSwapChain() {
    if (swapchain != nullptr) {
      VkDevice dev = device.value().handle->device;

      // await idleness of the semaphores device, so we can destroy the
      // semaphore and images whislt not in use.
      // any part of the device could be using the semaphore
      ASR_MUST_SUCCEED(vkDeviceWaitIdle(dev),
                       "Unable to await device idleness");

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
  }
};

struct WindowSurface {
  // only a pointer to metadata, does not contain data itself, resilient to
  // resizing
  VkSurfaceKHR surface = nullptr;

  // empty and invalid until change_swapchain is called.
  // not ref-counted since it solely belongs to this surface and the surface can
  // create and destroy it upon request.
  //
  // also, we need to be certain it is non-existent and not referring to any
  // resources when destroyed, not just by calling a method to destroy its
  // resources.
  //
  stx::Option<stx::Unique<WindowSwapChain*>> swapchain;

  stx::Option<stx::Rc<vk::Instance*>> instance;

  ASR_MAKE_HANDLE(WindowSurface)

  ~WindowSurface() {
    // we need to ensure the swapchain is destroyed before the surface (if not
    // already destroyed)
    swapchain = stx::None;

    if (surface != nullptr) {
      vkDestroySurfaceKHR(instance.value().handle->instance, surface, nullptr);
    }
  }

  void change_swapchain(
      vk::CommandQueue const& queue,
      stx::Span<VkSurfaceFormatKHR const> preferred_formats,
      stx::Span<VkPresentModeKHR const> preferred_present_modes, Extent extent,
      VkCompositeAlphaFlagBitsKHR alpha_compositing) {
    swapchain = stx::None;  // probably don't want to have two existing at once

    stx::Unique new_swapchain =
        stx::rc::make_unique_inplace<WindowSwapChain>(stx::os_allocator)
            .unwrap();

    VkPhysicalDevice phys_device =
        queue.info.device.value().handle->phys_device.info.phys_device;

    VkDevice device = queue.info.device.value().handle->device;

    // the properties change every time we need to create a swapchain so we must
    // query for this every time
    vk::SwapChainProperties properties =
        vk::get_swapchain_properties(phys_device, surface);

    ASR_LOG("Device Supported Surface Formats:");
    for (VkSurfaceFormatKHR const& format : properties.supported_formats) {
      ASR_LOG("\tFormat: {}, Color Space: {}", vk::format(format.format),
              vk::format(format.colorSpace));
    }

    // swapchain formats are device-dependent
    new_swapchain.handle->format = select_swapchain_surface_formats(
        properties.supported_formats, preferred_formats);
    // log selections
    // swapchain presentation modes are device-dependent
    new_swapchain.handle->present_mode = select_swapchain_presentation_mode(
        properties.presentation_modes, preferred_present_modes);

    uint32_t accessing_families[1] = {queue.info.family.info.index};

    auto [new_swapchain_r, actual_extent] = vk::create_swapchain(
        device, surface, VkExtent2D{extent.width, extent.height},
        new_swapchain.handle->format, new_swapchain.handle->present_mode,
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

    new_swapchain.handle->swapchain = new_swapchain_r;
    new_swapchain.handle->extent =
        Extent{actual_extent.width, actual_extent.height};
    new_swapchain.handle->images =
        vk::get_swapchain_images(device, new_swapchain.handle->swapchain);

    for (VkImage image : new_swapchain.handle->images) {
      VkImageView image_view = vk::create_image_view(
          device, image, new_swapchain.handle->format.format,
          VK_IMAGE_VIEW_TYPE_2D,
          // use image view as color buffer (can be used as depth buffer)
          VK_IMAGE_ASPECT_COLOR_BIT, vk::make_default_component_mapping());
      new_swapchain.handle->image_views.push_inplace(image_view).unwrap();
    }

    for (size_t i = 0; i < new_swapchain.handle->images.size(); i++) {
      new_swapchain.handle->rendering_semaphores
          .push(vk::create_semaphore(device))
          .unwrap();
      new_swapchain.handle->image_acquisition_semaphores
          .push(vk::create_semaphore(device))
          .unwrap();
    }

    swapchain = stx::Some(std::move(new_swapchain));
  }
};

struct WindowHandle {
  // SDL_Window* window = nullptr;
  // WindowID id{};
  // WindowSurface surface;
  // WindowApi api;
  // Extent extent;
  // Extent surface_extent;
  // WindowEventQueue event_queue;
  // WindowCfg cfg;
  // std::thread::id init_thread_id{};

  ASR_MAKE_HANDLE(WindowHandle)

  //
  //
  // process and dispatch events
  // notify of window resize, minimize, and maximize
  // notify of pipeline render and layout dirtiness
  //
  //
  // poll events for polling budget
  //
  //
  // if resize event comes in (this should be the only event expected by the
  // window once it is all cleaned up and widgets idle):
  //    - recreate swapchain
  //    - notify widget pipeline of resize event
  //    - forward backing store to swapchain
  //
  // if swapchain needs recreation:
  //    - recreate swapchain
  //    - forward backing store to swapchain
  //
  // if forwarding backing store to swapchain:
  //    - if error occured during swapchain presentation: i.e. it becomes
  //    non-optimal or out of date, go to swapchain recreation
  //
  //
  //
  // the event queue should be cleared after publishing the eventas

  // TODO(lamarrr): do these need to be passed in everytime?
  void recreate_swapchain() {
    // if cause of change in swapchain is a change in extent, then mark
    // layout as dirty, otherwise maintain pipeline state
    int width = 0, height = 0;
    SDL_GetWindowSize(window, &width, &height);
    extent =
        Extent{static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

    int surface_width = 0, surface_height = 0;
    SDL_Vulkan_GetDrawableSize(window, &surface_width, &surface_height);
    surface_extent = Extent{static_cast<uint32_t>(surface_width),
                            static_cast<uint32_t>(surface_height)};

    ASR_LOG("Resizing window to logical({},{}), physical({},{})", width, height,
            surface_width, surface_height);

    WindowSurfaceFormat preferred_formats[] = {
        {VK_FORMAT_R8G8B8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
         kRGBA_8888_SkColorType, SkColorSpace::MakeSRGB()},
        {VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
         kRGBA_8888_SkColorType, SkColorSpace::MakeSRGB()},
        {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
         kBGRA_8888_SkColorType, SkColorSpace::MakeSRGB()},
        {VK_FORMAT_R16G16B16A16_SFLOAT, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
         kRGBA_F16_SkColorType, SkColorSpace::MakeSRGBLinear()}};

    surface.handle->change_swapchain(vk_render_context, preferred_formats,
                                     WindowSwapChainHandle::present_modes,
                                     surface_extent,
                                     WindowSwapChainHandle::composite_alpha);
  }

  WindowSwapchainDiff present_backing_store(
      SkSurface& backing_store_sk_surface) {
    WindowSwapchainDiff diff = WindowSwapchainDiff::None;

    // We submit multiple render commands (operating on the swapchain images) to
    // the GPU to prevent having to force a sync with the GPU (await_fence) when
    // it could be doing useful work.
    VkDevice device = surface.handle->swapchain_handle->vk_render_context
                          ->graphics_command_queue.info.device.handle->device;

    VkSemaphore image_acquisition_semaphore =
        surface.handle->swapchain_handle->image_acquisition_semaphores
            [surface.handle->swapchain_handle->frame_flight_index];

    auto [next_swapchain_image_index, acquire_result] =
        vk::acquire_next_swapchain_image(
            device, surface.handle->swapchain_handle->swapchain,
            image_acquisition_semaphore, nullptr);

    if (acquire_result == VK_SUBOPTIMAL_KHR) {
      diff |= WindowSwapchainDiff::Suboptimal;
      return diff;
    } else if (acquire_result == VK_ERROR_OUT_OF_DATE_KHR) {
      diff |= WindowSwapchainDiff::OutOfDate;
      return diff;
    } else if (acquire_result != VK_SUCCESS) {
      ASR_PANIC("Unable to acquire image from swapchain", acquire_result);
    }

    sk_sp<SkSurface>& sk_surface =
        surface.handle->swapchain_handle
            ->skia_surfaces[next_swapchain_image_index];

    // if the previously submitted images from the previous swapchain image
    // rendering iteration is not done yet, then perform an expensive
    // GPU-CPU synchronization

    // Ques(lamarrr): Since GrDirectContext has and retains internal state,
    // wont that affect this command in any way?
    sk_surface->flushAndSubmit(true);

    GrBackendSemaphore gr_image_acquisition_semaphore{};
    gr_image_acquisition_semaphore.initVulkan(image_acquisition_semaphore);

    ASR_ENSURE(sk_surface->wait(1, &gr_image_acquisition_semaphore, false));

    // now just push the pixels to the sk_surface
    SkCanvas* canvas = sk_surface->getCanvas();

    // we need to clear the image, as the image on the swapcahins could be
    // re-used.
    // BONUS: the subsequent operations will be optimized since we are not
    // reading back the previous pixels on the image
    //
    canvas->clear(SK_ColorTRANSPARENT);

    // TODO(lamarrr): ensure the pipeline is constructed to use the same
    // format or something? we can't construct render context before creating
    // window and swapchain we also need to change pipeline render context if
    // for example, the swapchain format changes and conversion is not
    // supported? or does skia manage to somehow convert them?
    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc);

    backing_store_sk_surface.draw(canvas, 0, 0, &paint);

    // rendering
    VkSemaphore rendering_semaphore =
        surface.handle->swapchain_handle->rendering_semaphores
            [surface.handle->swapchain_handle->frame_flight_index];

    GrBackendSemaphore gr_rendering_semaphore{};
    gr_rendering_semaphore.initVulkan(rendering_semaphore);

    GrFlushInfo flush_info{};
    flush_info.fNumSemaphores = 1;
    flush_info.fSignalSemaphores = &gr_rendering_semaphore;

    GrBackendSurfaceMutableState target_presentation_surface_state{
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        surface.handle->swapchain_handle->vk_render_context
            ->graphics_command_queue.info.family.info.index};

    ASR_ENSURE(
        sk_surface->flush(flush_info, &target_presentation_surface_state) ==
        GrSemaphoresSubmitted::kYes);

    ASR_ENSURE(
        surface.handle->swapchain_handle->vk_render_context->render_context
            .get_direct_context()
            .unwrap()
            ->submit(false));

    // cmd.drawIndexed();

    // presentation (we don't need to wait on presentation)
    //
    // if v-sync is enabled (VK_PRESENT_MODE_FIFO_KHR) the GPU driver *can*
    // delay the process so we don't submit more frames than the display's
    // refresh rate can keep up with and we thus save power.
    //
    auto present_result =
        vk::present(surface.handle->swapchain_handle->vk_render_context
                        ->graphics_command_queue.info.queue,
                    stx::Span<VkSemaphore const>(&rendering_semaphore, 1),
                    stx::Span<VkSwapchainKHR const>(
                        &surface.handle->swapchain_handle->swapchain, 1),
                    stx::Span<uint32_t const>(&next_swapchain_image_index, 1));

    // the frame semaphores and synchronization primitives are still used even
    // if an error is returned
    surface.handle->swapchain_handle->frame_flight_index =
        (surface.handle->swapchain_handle->frame_flight_index + 1) %
        surface.handle->swapchain_handle->images.size();

    if (present_result == VK_SUBOPTIMAL_KHR) {
      diff |= WindowSwapchainDiff::Suboptimal;
      return diff;
    } else if (present_result == VK_ERROR_OUT_OF_DATE_KHR) {
      diff |= WindowSwapchainDiff::OutOfDate;
      return diff;
    } else if (present_result == VK_SUCCESS) {
      return diff;
    } else {
      ASR_PANIC("Unable to present swapchain image", present_result);
    }
  }
};

}  // namespace asr
