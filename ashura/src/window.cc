
#include "ashura/window.h"

#include <thread>

#include "SDL.h"
#include "SDL_vulkan.h"
#include "ashura/sdl_utils.h"

namespace ash {

stx::Vec<char const*> Window::get_required_instance_extensions() const {
  u32 ext_count = 0;
  stx::Vec<char const*> required_instance_extensions{stx::os_allocator};

  ASH_SDL_CHECK(
      SDL_Vulkan_GetInstanceExtensions(window_, &ext_count, nullptr) ==
          SDL_TRUE,
      "unable to get number of window's required Vulkan instance extensions");

  required_instance_extensions.resize(ext_count).unwrap();

  ASH_SDL_CHECK(
      SDL_Vulkan_GetInstanceExtensions(
          window_, &ext_count, required_instance_extensions.data()) == SDL_TRUE,
      "unable to get window's required Vulkan instance extensions");

  return required_instance_extensions;
}

void Window::attach_surface(stx::Rc<vk::Instance*> const& instance) {
  VkSurfaceKHR surface;

  ASH_SDL_CHECK(SDL_Vulkan_CreateSurface(window_, instance->instance,
                                         &surface) == SDL_TRUE,
                "unable to create surface for window");

  surface_ = stx::Some(stx::rc::make_unique_inplace<vk::Surface>(
                           stx::os_allocator, instance.share(), surface)
                           .unwrap());
}

void Window::recreate_swapchain(stx::Rc<vk::CommandQueue*> const& queue) {
  // if cause of change in swapchain is a change in extent, then mark
  // layout as dirty, otherwise maintain pipeline state
  int width = 0, height = 0;
  SDL_GetWindowSize(window_, &width, &height);
  window_extent_ = extent{AS_U32(width), AS_U32(height)};

  int surface_width = 0, surface_height = 0;
  SDL_Vulkan_GetDrawableSize(window_, &surface_width, &surface_height);
  surface_extent_ = extent{AS_U32(surface_width), AS_U32(surface_height)};

  VkSurfaceFormatKHR preferred_formats[] = {
      {VK_FORMAT_R8G8B8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
      {VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
      {VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
      {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
      {VK_FORMAT_R16G16B16A16_SFLOAT, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR}};

  VkPresentModeKHR preferred_present_modes[] = {
      VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_FIFO_RELAXED_KHR,
      VK_PRESENT_MODE_FIFO_KHR, VK_PRESENT_MODE_MAILBOX_KHR};

  VkSampleCountFlagBits msaa_sample_count =
      queue->device->phy_device->get_max_sample_count();

  surface_.value()->change_swapchain(
      queue, preferred_formats, preferred_present_modes,
      VkExtent2D{.width = surface_extent_.width,
                 .height = surface_extent_.height},
      VkExtent2D{.width = window_extent_.width,
                 .height = window_extent_.height},
      msaa_sample_count, VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR);

  ASH_LOG(
      "recreated swapchain for logical/window/viewport extent: [{}, {}], "
      "physical/surface extent: [{}, {}]",
      width, height, surface_width, surface_height);
}

std::pair<WindowSwapchainDiff, u32> Window::acquire_image() {
  ASH_CHECK(surface_.is_some(),
            "trying to present to swapchain without having surface attached");
  ASH_CHECK(surface_.value()->swapchain.is_some(),
            "trying to present to swapchain without having one");

  vk::SwapChain& swapchain = surface_.value()->swapchain.value();

  VkDevice dev = swapchain.queue->device->device;

  VkSemaphore semaphore =
      swapchain.image_acquisition_semaphores[swapchain.frame];

  VkFence fence = VK_NULL_HANDLE;

  u32 swapchain_image_index = 0;

  VkResult result =
      vkAcquireNextImageKHR(dev, swapchain.swapchain, COMMAND_TIMEOUT,
                            semaphore, fence, &swapchain_image_index);

  ASH_CHECK(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR ||
                result == VK_ERROR_OUT_OF_DATE_KHR,
            "failed to acquire swapchain image");

  if (result == VK_SUBOPTIMAL_KHR) {
    return std::make_pair(WindowSwapchainDiff::Suboptimal,
                          swapchain_image_index);
  } else if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    return std::make_pair(WindowSwapchainDiff::OutOfDate,
                          swapchain_image_index);
  } else if (result == VK_SUCCESS) {
    return std::make_pair(WindowSwapchainDiff::None, swapchain_image_index);
  } else {
    ASH_PANIC("failed to acquire swapchain image", result);
  }
}

WindowSwapchainDiff Window::present(u32 swapchain_image_index) {
  ASH_CHECK(surface_.is_some(),
            "trying to present to swapchain without having surface attached");
  ASH_CHECK(surface_.value()->swapchain.is_some(),
            "trying to present to swapchain without having one");

  // we submit multiple render commands (operating on the swapchain images) to
  // the GPU to prevent having to force a sync with the GPU (await_fence) when
  // it could be doing useful work.
  vk::SwapChain& swapchain = surface_.value()->swapchain.value();

  // we don't need to wait on presentation
  //
  // if v-sync is enabled (VK_PRESENT_MODE_FIFO_KHR) the GPU driver *can*
  // delay the process so we don't submit more frames than the display's
  // refresh rate can keep up with and we thus save power.
  //
  VkPresentInfoKHR present_info{
      .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
      .pNext = nullptr,
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = &swapchain.render_semaphores[swapchain.frame],
      .swapchainCount = 1,
      .pSwapchains = &swapchain.swapchain,
      .pImageIndices = &swapchain_image_index,
      .pResults = nullptr};

  VkResult result =
      vkQueuePresentKHR(swapchain.queue->info.queue, &present_info);

  ASH_CHECK(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR ||
                result == VK_ERROR_OUT_OF_DATE_KHR,
            "failed to present swapchain image");

  if (result == VK_SUBOPTIMAL_KHR) {
    return WindowSwapchainDiff::Suboptimal;
  } else if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    return WindowSwapchainDiff::OutOfDate;
  } else if (result == VK_SUCCESS) {
    return WindowSwapchainDiff::None;
  } else {
    ASH_PANIC("failed to present swapchain image", result);
  }
}

stx::Rc<Window*> create_window(stx::Rc<WindowApi*> api, WindowConfig cfg) {
  // width and height here refer to the screen coordinates and not the
  // actual pixel coordinates (cc: Device Pixel Ratio)

  int window_flags = SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_VULKAN;

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

  SDL_Window* window = SDL_CreateWindow(
      cfg.title.c_str(), 0, 0, AS_I32(std::max<u32>(cfg.extent.width, 1)),
      AS_I32(std::max<u32>(cfg.extent.height, 1)), window_flags);

  // window creation shouldn't fail reliably, if it fails,
  // there's no point in the program proceeding
  ASH_SDL_CHECK(window != nullptr, "unable to create window");

  cfg.min_extent.copy().match(
      [&](extent min_extent) {
        SDL_SetWindowMinimumSize(window, AS_I32(min_extent.width),
                                 AS_I32(min_extent.height));
      },
      []() {});

  cfg.max_extent.copy().match(
      [&](extent max_extent) {
        SDL_SetWindowMaximumSize(window, AS_I32(max_extent.width),
                                 AS_I32(max_extent.height));
      },
      []() {});

  SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

  u32 window_id = SDL_GetWindowID(window);

  stx::Rc<Window*> win =
      stx::rc::make_inplace<Window>(stx::os_allocator, std::move(api), window,
                                    WindowID{window_id}, cfg.extent, cfg.extent,
                                    std::move(cfg), std::this_thread::get_id())
          .unwrap();

  win->api_->add_window_info(win->id_, win.handle);

  return win;
}

}  // namespace ash
