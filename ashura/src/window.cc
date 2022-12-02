
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
      "unable to get number of window's required Vulkan instance extensions");

  required_instance_extensions.resize(ext_count).unwrap();

  ASR_SDL_ENSURE(
      SDL_Vulkan_GetInstanceExtensions(
          window_, &ext_count, required_instance_extensions.data()) == SDL_TRUE,
      "unable to get window's required Vulkan instance extensions");

  return required_instance_extensions;
}

void Window::attach_surface(stx::Rc<vk::Instance*> const& instance) {
  VkSurfaceKHR surface;

  ASR_SDL_ENSURE(SDL_Vulkan_CreateSurface(window_, instance.handle->instance,
                                          &surface) == SDL_TRUE,
                 "unable to create surface for window");

  surface_ =
      stx::Some(stx::rc::make_unique_inplace<vk::Surface>(
                    stx::os_allocator, instance.share(), surface, stx::None)
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
void Window::recreate_swapchain(stx::Rc<vk::CommandQueue*> const& queue) {
  // if cause of change in swapchain is a change in extent, then mark
  // layout as dirty, otherwise maintain pipeline state
  int width = 0, height = 0;
  SDL_GetWindowSize(window_, &width, &height);
  window_extent_ = Extent{u32_clamp(width), u32_clamp(height)};

  int surface_width = 0, surface_height = 0;
  SDL_Vulkan_GetDrawableSize(window_, &surface_width, &surface_height);
  surface_extent_ = Extent{u32_clamp(surface_width), u32_clamp(surface_height)};

  ASR_LOG_TRACE("resizing window to logical({}, {}), physical({}, {})", width,
                height, surface_width, surface_height);

  VkSurfaceFormatKHR preferred_formats[] = {
      {VK_FORMAT_R8G8B8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
      {VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
      {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
      {VK_FORMAT_R16G16B16A16_SFLOAT, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR}};

  VkPresentModeKHR preferred_present_modes[] = {
      VK_PRESENT_MODE_FIFO_RELAXED_KHR, VK_PRESENT_MODE_FIFO_KHR,
      VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR};

  VkSampleCountFlagBits msaa_sample_count =
      queue.handle->device.handle->phy_device.handle->get_max_sample_count();

  // TODO(lamarr): check VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR
  surface_.value().handle->change_swapchain(
      queue, preferred_formats, preferred_present_modes,
      VkExtent2D{.width = surface_extent_.w, .height = surface_extent_.h},
      VkExtent2D{.width = window_extent_.w, .height = window_extent_.h},
      msaa_sample_count, VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR);
}

std::pair<WindowSwapchainDiff, u32> Window::acquire_image() {
  ASR_ENSURE(surface_.is_some(),
             "trying to present backing store without having surface attached");

  vk::SwapChain& swapchain = *surface_.value().handle->swapchain.value().handle;

  VkSemaphore semaphore =
      swapchain.image_acquisition_semaphores[swapchain.next_frame_flight_index];

  VkFence fence =
      swapchain.image_acquisition_fences[swapchain.next_frame_flight_index];

  VkDevice device = swapchain.queue.handle->device.handle->device;

  auto [next_swapchain_image_index, acquire_result] =
      vk::acquire_next_swapchain_image(device, swapchain.swapchain, semaphore,
                                       fence);

  if (acquire_result == VK_SUBOPTIMAL_KHR) {
    return std::make_pair(WindowSwapchainDiff::Suboptimal,
                          next_swapchain_image_index);
  } else if (acquire_result == VK_ERROR_OUT_OF_DATE_KHR) {
    return std::make_pair(WindowSwapchainDiff::OutOfDate,
                          next_swapchain_image_index);
  } else if (acquire_result == VK_SUCCESS) {
    return std::make_pair(WindowSwapchainDiff::None,
                          next_swapchain_image_index);
  } else {
    ASR_PANIC("failed to acquire image from swapchain", acquire_result);
  }
}

WindowSwapchainDiff Window::present(u32 next_swapchain_image_index) {
  ASR_ENSURE(surface_.is_some(),
             "trying to present backing store without having surface attached");

  // we submit multiple render commands (operating on the swapchain images) to
  // the GPU to prevent having to force a sync with the GPU (await_fence) when
  // it could be doing useful work.
  vk::SwapChain& swapchain = *surface_.value().handle->swapchain.value().handle;

  // we don't need to wait on presentation
  //
  // if v-sync is enabled (VK_PRESENT_MODE_FIFO_KHR) the GPU driver *can*
  // delay the process so we don't submit more frames than the display's
  // refresh rate can keep up with and we thus save power.
  //
  VkResult present_result =
      vk::present(swapchain.queue.handle->info.queue, stx::Span<VkSemaphore>{},
                  stx::Span{&swapchain.swapchain, 1},
                  stx::Span{&next_swapchain_image_index, 1});

  if (present_result == VK_SUBOPTIMAL_KHR) {
    return WindowSwapchainDiff::Suboptimal;
  } else if (present_result == VK_ERROR_OUT_OF_DATE_KHR) {
    return WindowSwapchainDiff::OutOfDate;
  } else if (present_result == VK_SUCCESS) {
    return WindowSwapchainDiff::None;
  } else {
    ASR_PANIC("failed to present swapchain image", present_result);
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
  ASR_SDL_ENSURE(window != nullptr, "unable to create window");

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
