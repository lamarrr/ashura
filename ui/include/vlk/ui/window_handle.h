#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "SDL_vulkan.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/gpu/GrBackendSemaphore.h"
#include "vlk/primitives.h"
#include "vlk/ui/sdl_utils.h"
#include "vlk/ui/vk_render_context.h"
#include "vlk/ui/vulkan.h"
#include "vlk/ui/window_api_handle.h"
#include "vlk/ui/window_event_queue.h"
#include "vlk/utils.h"

namespace vlk {
namespace ui {

// skia doesn't support all surface formats, hence we have to provide formats or
// color types for it to convert to
struct WindowSurfaceFormat {
  VkFormat vk_format{};
  VkColorSpaceKHR vk_color_space{};
  SkColorType sk_color{};
  sk_sp<SkColorSpace> sk_color_space = nullptr;
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
struct WindowSwapChainHandle {
  static constexpr VkImageUsageFlags images_usage =
      VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
      VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

  static constexpr VkImageTiling images_tiling = VK_IMAGE_TILING_OPTIMAL;

  static constexpr VkSharingMode images_sharing_mode =
      VK_SHARING_MODE_EXCLUSIVE;

  static constexpr VkImageLayout images_initial_layout =
      VK_IMAGE_LAYOUT_UNDEFINED;

  static constexpr VkCompositeAlphaFlagBitsKHR composite_alpha =
      VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

  // TODO(lamarrr): log and format presentation modes
  static constexpr VkPresentModeKHR present_modes[4] = {
      VK_PRESENT_MODE_FIFO_KHR, VK_PRESENT_MODE_MAILBOX_KHR,
      VK_PRESENT_MODE_FIFO_RELAXED_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR};

  // actually holds the images of the surface and used to present to the render
  // target image. when resizing is needed, the swapchain is destroyed and
  // recreated with the desired extents.
  VkSwapchainKHR swapchain = nullptr;
  WindowSurfaceFormat format;
  VkPresentModeKHR present_mode{};
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
  std::vector<VkImage> images;

  // the image views pointing to a part of a whole texture (images in the
  // swapchain)
  std::vector<VkImageView> image_views;

  // the rendering semaphores correspond to the frame indexes and not the
  // swapchain images
  std::vector<VkSemaphore> rendering_semaphores;

  std::vector<VkSemaphore> image_acquisition_semaphores;

  std::shared_ptr<VkRenderContext> vk_render_context = nullptr;

  /// IMPORTANT: placed in this position so Skia can destroy its command queue
  /// before the logical device contained by `graphics_command_queue` is
  /// destroyed
  // but surfaces are bound to the contexts
  std::vector<sk_sp<SkSurface>> skia_surfaces;

  VLK_MAKE_HANDLE(WindowSwapChainHandle)

  ~WindowSwapChainHandle() {
    if (swapchain != nullptr) {
      // await idleness of the semaphores device, so we can destroy the
      // semaphore and images whislt not in use.
      // any part of the device could be using the semaphore
      VLK_MUST_SUCCEED(
          vkDeviceWaitIdle(vk_render_context->graphics_command_queue.info.device
                               .handle->device),
          "Unable to await device idleness");

      for (VkSemaphore semaphore : rendering_semaphores) {
        vkDestroySemaphore(vk_render_context->graphics_command_queue.info.device
                               .handle->device,
                           semaphore, nullptr);
      }

      for (VkSemaphore semaphore : image_acquisition_semaphores) {
        vkDestroySemaphore(vk_render_context->graphics_command_queue.info.device
                               .handle->device,
                           semaphore, nullptr);
      }

      for (VkImageView image_view : image_views) {
        vkDestroyImageView(vk_render_context->graphics_command_queue.info.device
                               .handle->device,
                           image_view, nullptr);
      }

      // swapchain image is automatically deleted along with the swapchain
      vkDestroySwapchainKHR(
          vk_render_context->graphics_command_queue.info.device.handle->device,
          swapchain, nullptr);
    }

    swapchain = nullptr;
  }
};

struct WindowSurfaceHandle {
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
  std::unique_ptr<WindowSwapChainHandle> swapchain_handle;

  vk::Instance instance;

  VLK_MAKE_HANDLE(WindowSurfaceHandle)

  void change_swapchain(
      std::shared_ptr<VkRenderContext> const& vk_render_context,
      stx::Span<WindowSurfaceFormat const> preferred_formats,
      stx::Span<VkPresentModeKHR const> preferred_present_modes, Extent extent,
      VkCompositeAlphaFlagBitsKHR alpha_compositing) {
    VLK_ENSURE(vk_render_context->graphics_command_queue.info.device.handle
                       ->phys_device.info.instance.handle->instance ==
                   instance.handle->instance,
               "Provided command queue and target surface do not belong on the "
               "same Vulkan instance");

    swapchain_handle = nullptr;

    std::unique_ptr<WindowSwapChainHandle> new_handle{
        new WindowSwapChainHandle{}};

    new_handle->vk_render_context = vk_render_context;

    vk::Device const& device_object =
        vk_render_context->graphics_command_queue.info.device;
    VkPhysicalDevice phys_device =
        device_object.handle->phys_device.info.phys_device;
    VkDevice device = device_object.handle->device;

    // the properties change every time we need to create a swapchain so we must
    // query for this every time
    vk::SwapChainProperties properties =
        vk::get_swapchain_properties(phys_device, surface);

    VLK_LOG("Device Supported Surface Formats:");
    for (VkSurfaceFormatKHR const& format : properties.supported_formats) {
      VLK_LOG("\tFormat: {}, Color Space: {}", vk::format(format.format),
              vk::format(format.colorSpace));
    }

    // swapchain formats are device-dependent
    new_handle->format = select_swapchain_surface_formats(
        properties.supported_formats, preferred_formats);
    // log selections
    // swapchain presentation modes are device-dependent
    new_handle->present_mode = select_swapchain_presentation_mode(
        properties.presentation_modes, preferred_present_modes);

    uint32_t accessing_families[1] = {
        new_handle->vk_render_context->graphics_command_queue.info.family.info
            .index};

    auto [new_swapchain, actual_extent] = vk::create_swapchain(
        device, surface, VkExtent2D{extent.width, extent.height},
        VkSurfaceFormatKHR{new_handle->format.vk_format,
                           new_handle->format.vk_color_space},
        new_handle->present_mode, properties,
        // not thread-safe since GPUs typically have one graphics queue
        WindowSwapChainHandle::images_sharing_mode, accessing_families,
        // render target image
        WindowSwapChainHandle::images_usage, alpha_compositing,
        // we don't care about the color of pixels that are obscured, for
        // example because another window is in front of them. Unless you really
        // need to be able to read these pixels back and get predictable
        // results, you'll get the best performance by enabling clipping.
        true);

    new_handle->swapchain = new_swapchain;
    new_handle->extent = Extent{actual_extent.width, actual_extent.height};
    new_handle->images =
        vk::get_swapchain_images(device, new_handle->swapchain);

    for (VkImage image : new_handle->images) {
      VkImageView image_view = vk::create_image_view(
          device, image, new_handle->format.vk_format, VK_IMAGE_VIEW_TYPE_2D,
          // use image view as color buffer (can be used as depth buffer)
          VK_IMAGE_ASPECT_COLOR_BIT, vk::make_default_component_mapping());
      new_handle->image_views.push_back(image_view);
    }

    for (VkImage image : new_handle->images) {
      GrVkImageInfo image_info{};
      image_info.fImage = image;
      image_info.fImageTiling = WindowSwapChainHandle::images_tiling;
      image_info.fImageLayout = WindowSwapChainHandle::images_initial_layout;
      image_info.fFormat = new_handle->format.vk_format;
      image_info.fImageUsageFlags = WindowSwapChainHandle::images_usage;
      image_info.fSampleCount = 1;  // VK_SAMPLE_COUNT_1
      image_info.fLevelCount = 1;   // mip levels
      image_info.fSharingMode = WindowSwapChainHandle::images_sharing_mode;

      GrBackendRenderTarget backend_render_target(
          new_handle->extent.width, new_handle->extent.height, 1, image_info);
      SkSurfaceProps props{};

      auto sk_surface = SkSurface::MakeFromBackendRenderTarget(
          new_handle->vk_render_context->render_context.get_direct_context()
              .unwrap()
              .get(),                         // context
          backend_render_target,              // backend render target
          kTopLeft_GrSurfaceOrigin,           // origin
          new_handle->format.sk_color,        // color type
          new_handle->format.sk_color_space,  // color space
          &props);                            // surface properties

      VLK_ENSURE(sk_surface != nullptr);
      new_handle->skia_surfaces.push_back(std::move(sk_surface));
    }

    for (size_t i = 0; i < new_handle->images.size(); i++) {
      new_handle->rendering_semaphores.push_back(vk::create_semaphore(device));
      new_handle->image_acquisition_semaphores.push_back(
          vk::create_semaphore(device));
    }

    swapchain_handle = std::move(new_handle);
  }

  ~WindowSurfaceHandle() {
    // we need to ensure the swapchain is destroyed before the surface (if not
    // already destroyed)
    swapchain_handle = nullptr;

    if (surface != nullptr) {
      vkDestroySurfaceKHR(instance.handle->instance, surface, nullptr);
    }
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

    /// - VK_PRESENT_MODE_MAILBOX_KHR: This is another variation of the
    /// second mode. Instead of blocking the application when the queue is
    /// full, the images that are already queued are simply replaced with the
    /// newer ones. This mode can be used to implement triple buffering, which
    /// allows you to avoid tearing with significantly less latency issues
    /// than standard vertical sync that uses double buffering.

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
  std::shared_ptr<WindowSurfaceHandle> handle;
};

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

enum class WindowContentDirtiness : uint8_t {
  None = 0,
  Layout = 1,
  RePresent = 2,
  All = Layout | RePresent
};

STX_DEFINE_ENUM_BIT_OPS(WindowContentDirtiness);

constexpr WindowContentDirtiness map_diff(WindowSwapchainDiff diff) {
  WindowContentDirtiness dirtiness = WindowContentDirtiness::None;

  if ((diff & WindowSwapchainDiff::Extent) != WindowSwapchainDiff::None) {
    dirtiness |=
        WindowContentDirtiness::Layout | WindowContentDirtiness::RePresent;
  }

  if ((diff & WindowSwapchainDiff::Suboptimal) != WindowSwapchainDiff::None) {
    dirtiness |= WindowContentDirtiness::RePresent;
  }

  if ((diff & WindowSwapchainDiff::OutOfDate) != WindowSwapchainDiff::None) {
    dirtiness |= WindowContentDirtiness::RePresent;
  }

  return dirtiness;
}

struct WindowHandle {
  SDL_Window* window = nullptr;
  WindowID id{};
  WindowSurface surface;
  WindowApi api;
  Extent extent;
  Extent surface_extent;
  WindowEventQueue event_queue;
  WindowCfg cfg;

  VLK_MAKE_HANDLE(WindowHandle)

  ~WindowHandle() {
    if (window != nullptr) {
      api.handle->remove_window_info(id);
      SDL_DestroyWindow(window);
    }
  }

  std::vector<char const*> get_required_instance_extensions() const {
    uint32_t ext_count = 0;
    std::vector<char const*> required_instance_extensions;

    VLK_SDL_ENSURE(
        SDL_Vulkan_GetInstanceExtensions(window, &ext_count, nullptr) ==
            SDL_TRUE,
        "Unable to get number of window's required Vulkan instance extensions");

    required_instance_extensions.resize(ext_count);

    VLK_SDL_ENSURE(
        SDL_Vulkan_GetInstanceExtensions(window, &ext_count,
                                         required_instance_extensions.data()) ==
            SDL_TRUE,
        "Unable to get window's required Vulkan instance extensions");

    return required_instance_extensions;
  }

  // needs:
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
  void recreate_swapchain(
      std::shared_ptr<VkRenderContext> const& vk_render_context) {
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

    VLK_LOG("Resizing window to logical({},{}), physical({},{})", width, height,
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
      VLK_PANIC("Unable to acquire image from swapchain", acquire_result);
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

    VLK_ENSURE(sk_surface->wait(1, &gr_image_acquisition_semaphore, false));

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

    VLK_ENSURE(
        sk_surface->flush(flush_info, &target_presentation_surface_state) ==
        GrSemaphoresSubmitted::kYes);

    VLK_ENSURE(
        surface.handle->swapchain_handle->vk_render_context->render_context
            .get_direct_context()
            .unwrap()
            ->submit(false));

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
      VLK_PANIC("Unable to present swapchain image", present_result);
    }
  }
};

}  // namespace ui
}  // namespace vlk
