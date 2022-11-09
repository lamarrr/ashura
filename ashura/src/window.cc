
#include "ashura/window.h"

#include <thread>

#include "SDL.h"
#include "SDL_vulkan.h"
#include "ashura/sdl_utils.h"

namespace asr {

stx::Vec<char const*> Window::get_required_instance_extensions() const {
  uint32_t ext_count = 0;
  stx::Vec<char const*> required_instance_extensions{stx::os_allocator};

  ASR_SDL_ENSURE(
      SDL_Vulkan_GetInstanceExtensions(window_, &ext_count, nullptr) ==
          SDL_TRUE,
      "Unable to get number of window's required Vulkan instance extensions");

  required_instance_extensions.resize(ext_count).unwrap();

  ASR_SDL_ENSURE(
      SDL_Vulkan_GetInstanceExtensions(
          window_, &ext_count, required_instance_extensions.data()) == SDL_TRUE,
      "Unable to get window's required Vulkan instance extensions");

  return required_instance_extensions;
}

void Window::attach_surface(stx::Rc<vkh::Instance*> instance) {
  VkSurfaceKHR surface;

  ASR_SDL_ENSURE(SDL_Vulkan_CreateSurface(window_, instance.handle->instance,
                                          &surface) == SDL_TRUE,
                 "Unable to create surface for window");

  surface_ =
      stx::Some(stx::rc::make_unique_inplace<WindowSurface>(
                    stx::os_allocator, surface, stx::None, std::move(instance))
                    .unwrap());
}

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
void Window::recreate_swapchain(
    stx::Rc<vkh::Device*> const& device,
    stx::Rc<vkh::CommandQueueFamilyInfo*> const& family) {
  // if cause of change in swapchain is a change in extent, then mark
  // layout as dirty, otherwise maintain pipeline state
  int width = 0, height = 0;
  SDL_GetWindowSize(window_, &width, &height);
  extent_ = Extent{u32_clamp(width), u32_clamp(height)};

  int surface_width = 0, surface_height = 0;
  SDL_Vulkan_GetDrawableSize(window_, &surface_width, &surface_height);
  surface_extent_ = Extent{u32_clamp(surface_width), u32_clamp(surface_height)};

  ASR_LOG("Resizing window to logical({},{}), physical({},{})", width, height,
          surface_width, surface_height);

  VkSurfaceFormatKHR preferred_formats[] = {
      {VK_FORMAT_R8G8B8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
      {VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
      {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
      {VK_FORMAT_R16G16B16A16_SFLOAT, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR}};

  surface_.value().handle->change_swapchain(
      device, family, preferred_formats, WindowSwapChain::PRESENT_MODES,
      surface_extent_, WindowSwapChain ::COMPOSITE_ALPHA);
}

// TODO(lamarrr): can't we render to the swapchain images directly?
WindowSwapchainDiff Window::present_backing_store() {
  ASR_ENSURE(surface_.is_some(),
             "trying to present backing store without having surface attached");

  WindowSwapchainDiff diff = WindowSwapchainDiff::None;

  // We submit multiple render commands (operating on the swapchain images) to
  // the GPU to prevent having to force a sync with the GPU (await_fence) when
  // it could be doing useful work.
  VkDevice device =
      surface_.value().handle->swapchain.value().handle->device.handle->device;
  auto& swapchain = *surface_.value().handle->swapchain.value().handle;
  VkSemaphore image_acquisition_semaphore =
      swapchain.image_acquisition_semaphores[swapchain.frame_flight_index];

  auto [next_swapchain_image_index, acquire_result] =
      vk::acquire_next_swapchain_image(device, swapchain.swapchain,
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

  /*
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

    ASR_ENSURE(surface.handle->swapchain_handle->vk_render_context->render_context
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
  */

  VkResult present_result = VK_SUBOPTIMAL_KHR;
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

stx::Rc<Window*> create_window(stx::Rc<WindowApi*> api, WindowConfig cfg) {
  // width and height here refer to the screen coordinates and not the
  // actual pixel coordinates (cc: Device Pixel Ratio)

  auto window_flags = SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_VULKAN;

  if (cfg.type_hint == WindowTypeHint::Normal) {
  } else if (cfg.type_hint == WindowTypeHint::Popup) {
    window_flags |= SDL_WINDOW_POPUP_MENU;
  } else if (cfg.type_hint == WindowTypeHint::Tooltip) {
    window_flags |= SDL_WINDOW_TOOLTIP;
  } else if (cfg.type_hint == WindowTypeHint::Utility) {
    window_flags |= SDL_WINDOW_UTILITY;
  }

  if (cfg.hidden) {
    window_flags |= SDL_WINDOW_HIDDEN;
  } else {
    window_flags |= SDL_WINDOW_SHOWN;
  }

  if (cfg.resizable) {
    window_flags |= SDL_WINDOW_RESIZABLE;
  }

  if (cfg.borderless) {
    window_flags |= SDL_WINDOW_BORDERLESS;
  }

  if (cfg.fullscreen) {
    window_flags |= SDL_WINDOW_FULLSCREEN;
  }

  if (cfg.always_on_top) {
    window_flags |= SDL_WINDOW_ALWAYS_ON_TOP;
  }

  SDL_Window* window =
      SDL_CreateWindow(cfg.title.c_str(), 0, 0, i32_clamp(cfg.extent.w),
                       i32_clamp(cfg.extent.h), window_flags);

  // window creation shouldn't fail reliably, if it fails,
  // there's no point in the program proceeding
  ASR_SDL_ENSURE(window != nullptr, "Unable to create window");

  cfg.min_extent.copy().match(
      [&](Extent min_extent) {
        SDL_SetWindowMinimumSize(window, i32_clamp(min_extent.w),
                                 i32_clamp(min_extent.h));
      },
      []() {});

  cfg.max_extent.copy().match(
      [&](Extent max_extent) {
        SDL_SetWindowMaximumSize(window, i32_clamp(max_extent.w),
                                 i32_clamp(max_extent.h));
      },
      []() {});

  SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

  uint32_t window_id = SDL_GetWindowID(window);

  stx::Rc<Window*> win =
      stx::rc::make_inplace<Window>(stx::os_allocator, std::move(api), window,
                                    WindowID{window_id}, cfg.extent, cfg.extent,
                                    std::move(cfg), std::this_thread::get_id())
          .unwrap();

  api.handle->add_window_info(win.handle->id_, win.handle);

  return win;
}

}  // namespace asr
