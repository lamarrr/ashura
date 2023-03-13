
#include "ashura/window.h"

#include <thread>

#include "SDL3/SDL.h"
#include "SDL3/SDL_vulkan.h"
#include "ashura/sdl_utils.h"

namespace ash
{

void Window::attach_surface(stx::Rc<vk::Instance *> const &instance)
{
  this->instance = stx::Some(instance.share());

  VkSurfaceKHR surface;

  ASH_SDL_CHECK(SDL_Vulkan_CreateSurface(window, instance->instance,
                                         &surface) == SDL_TRUE,
                "unable to create surface for window");

  this->surface = stx::Some(
      stx::rc::make_unique(
          stx::os_allocator,
          vk::Surface{.surface = surface, .instance = instance->instance})
          .unwrap());
}

void Window::recreate_swapchain(stx::Rc<vk::CommandQueue *> const &queue,
                                u32                                max_nframes_in_flight,
                                spdlog::logger                    &logger)
{
  // if cause of change in swapchain is a change in extent, then mark
  // layout as dirty, otherwise maintain pipeline state
  int width, height;
  SDL_GetWindowSize(window, &width, &height);
  window_extent = extent{AS(u32, width), AS(u32, height)};

  int surface_width, surface_height;
  SDL_GetWindowSizeInPixels(window, &surface_width, &surface_height);
  surface_extent = extent{AS(u32, surface_width), AS(u32, surface_height)};

  VkSurfaceFormatKHR preferred_formats[] = {
      {VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
      {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
      {VK_FORMAT_R16G16B16A16_SFLOAT, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR}};

  /*
  VK_COLOR_SPACE_SRGB_NONLINEAR_KHR = 0,
      VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT = 1000104001,
      VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT = 1000104002,
      VK_COLOR_SPACE_DISPLAY_P3_LINEAR_EXT = 1000104003,
      VK_COLOR_SPACE_DCI_P3_NONLINEAR_EXT = 1000104004,
      VK_COLOR_SPACE_BT709_LINEAR_EXT = 1000104005,
      VK_COLOR_SPACE_BT709_NONLINEAR_EXT = 1000104006,
      VK_COLOR_SPACE_BT2020_LINEAR_EXT = 1000104007,
      VK_COLOR_SPACE_HDR10_ST2084_EXT = 1000104008,
      VK_COLOR_SPACE_DOLBYVISION_EXT = 1000104009,
      VK_COLOR_SPACE_HDR10_HLG_EXT = 1000104010,
      VK_COLOR_SPACE_ADOBERGB_LINEAR_EXT = 1000104011,
      VK_COLOR_SPACE_ADOBERGB_NONLINEAR_EXT = 1000104012,
      VK_COLOR_SPACE_PASS_THROUGH_EXT = 1000104013,
      VK_COLOR_SPACE_EXTENDED_SRGB_NONLINEAR_EXT = 1000104014,
      VK_COLOR_SPACE_DISPLAY_NATIVE_AMD = 1000213000,
      VK_COLORSPACE_SRGB_NONLINEAR_KHR = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
      VK_COLOR_SPACE_DCI_P3_LINEAR_EXT = VK_COLOR_SPACE_DISPLAY_P3_LINEAR_EXT,
      VK_COLOR_SPACE_MAX_ENUM_KHR = 0x7FFFFFFF
  */

  VkPresentModeKHR preferred_present_modes[] = {
      VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_FIFO_RELAXED_KHR,
      VK_PRESENT_MODE_FIFO_KHR, VK_PRESENT_MODE_MAILBOX_KHR};

  VkSampleCountFlagBits msaa_sample_count =
      queue->device->phy_dev->get_max_sample_count();

  surface.value()->change_swapchain(
      queue, max_nframes_in_flight, preferred_formats, preferred_present_modes,
      VkExtent2D{.width  = surface_extent.width,
                 .height = surface_extent.height},
      VkExtent2D{.width = window_extent.width, .height = window_extent.height},
      msaa_sample_count, VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR, logger);
}

std::pair<SwapChainState, u32> Window::acquire_image()
{
  ASH_CHECK(surface.is_some(),
            "trying to present to swapchain without having surface attached");
  ASH_CHECK(surface.value()->swapchain.is_some(),
            "trying to present to swapchain without having one");

  vk::SwapChain &swapchain = surface.value()->swapchain.value();

  VkSemaphore semaphore =
      swapchain.image_acquisition_semaphores[swapchain.frame];

  VkFence fence = VK_NULL_HANDLE;

  u32 swapchain_image_index = 0;

  VkResult result = vkAcquireNextImageKHR(swapchain.dev, swapchain.swapchain,
                                          UI_COMMAND_TIMEOUT, semaphore, fence,
                                          &swapchain_image_index);

  ASH_CHECK(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR ||
                result == VK_ERROR_OUT_OF_DATE_KHR,
            "failed to acquire swapchain image");

  if (result == VK_SUBOPTIMAL_KHR)
  {
    return std::make_pair(SwapChainState::Suboptimal, swapchain_image_index);
  }
  else if (result == VK_ERROR_OUT_OF_DATE_KHR)
  {
    return std::make_pair(SwapChainState::OutOfDate, swapchain_image_index);
  }
  else if (result == VK_SUCCESS)
  {
    return std::make_pair(SwapChainState::Ok, swapchain_image_index);
  }
  else
  {
    ASH_PANIC("failed to acquire swapchain image", result);
  }
}

SwapChainState Window::present(VkQueue queue, u32 swapchain_image_index)
{
  ASH_CHECK(surface.is_some(),
            "trying to present to swapchain without having surface attached");
  ASH_CHECK(surface.value()->swapchain.is_some(),
            "trying to present to swapchain without having one");

  // we submit multiple render commands (operating on the swapchain images) to
  // the GPU to prevent having to force a sync with the GPU (await_fence) when
  // it could be doing useful work.
  vk::SwapChain &swapchain = surface.value()->swapchain.value();

  // we don't need to wait on presentation
  //
  // if v-sync is enabled (VK_PRESENT_MODE_FIFO_KHR) the GPU driver *can*
  // delay the process so we don't submit more frames than the display's
  // refresh rate can keep up with and we thus save power.
  //
  VkPresentInfoKHR present_info{
      .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
      .pNext              = nullptr,
      .waitSemaphoreCount = 1,
      .pWaitSemaphores    = &swapchain.render_semaphores[swapchain.frame],
      .swapchainCount     = 1,
      .pSwapchains        = &swapchain.swapchain,
      .pImageIndices      = &swapchain_image_index,
      .pResults           = nullptr};

  VkResult result = vkQueuePresentKHR(queue, &present_info);

  ASH_CHECK(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR ||
                result == VK_ERROR_OUT_OF_DATE_KHR,
            "failed to present swapchain image");

  if (result == VK_SUBOPTIMAL_KHR)
  {
    return SwapChainState::Suboptimal;
  }
  else if (result == VK_ERROR_OUT_OF_DATE_KHR)
  {
    return SwapChainState::OutOfDate;
  }
  else if (result == VK_SUCCESS)
  {
    return SwapChainState::Ok;
  }
  else
  {
    ASH_PANIC("failed to present swapchain image", result);
  }
}

stx::Rc<Window *> create_window(stx::Rc<WindowApi *> api, WindowConfig cfg)
{
  // width and height here refer to the screen coordinates and not the
  // actual pixel coordinates (cc: Device Pixel Ratio)

  int window_flags = SDL_WINDOW_VULKAN;

  if (cfg.type_hint == WindowTypeHint::Normal)
  {
  }
  else if (cfg.type_hint == WindowTypeHint::Popup)
  {
    window_flags |= SDL_WINDOW_POPUP_MENU;
  }
  else if (cfg.type_hint == WindowTypeHint::Tooltip)
  {
    window_flags |= SDL_WINDOW_TOOLTIP;
  }
  else if (cfg.type_hint == WindowTypeHint::Utility)
  {
    window_flags |= SDL_WINDOW_UTILITY;
  }

  if (cfg.hidden)
  {
    window_flags |= SDL_WINDOW_HIDDEN;
  }

  if (cfg.resizable)
  {
    window_flags |= SDL_WINDOW_RESIZABLE;
  }

  if (cfg.borderless)
  {
    window_flags |= SDL_WINDOW_BORDERLESS;
  }

  if (cfg.fullscreen)
  {
    window_flags |= SDL_WINDOW_FULLSCREEN;
  }

  if (cfg.always_on_top)
  {
    window_flags |= SDL_WINDOW_ALWAYS_ON_TOP;
  }

  SDL_Window *window = SDL_CreateWindow(
      cfg.title.c_str(), AS(i32, std::max<u32>(cfg.extent.width, 1)),
      AS(i32, std::max<u32>(cfg.extent.height, 1)), window_flags);

  // window creation shouldn't fail reliably, if it fails,
  // there's no point in the program proceeding
  ASH_SDL_CHECK(window != nullptr, "unable to create window");

  cfg.min_extent.copy().match(
      [&](extent min_extent) {
        SDL_SetWindowMinimumSize(window, AS(i32, min_extent.width),
                                 AS(i32, min_extent.height));
      },
      []() {});

  cfg.max_extent.copy().match(
      [&](extent max_extent) {
        SDL_SetWindowMaximumSize(window, AS(i32, max_extent.width),
                                 AS(i32, max_extent.height));
      },
      []() {});

  SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

  u32 window_id = SDL_GetWindowID(window);

  stx::Rc<Window *> win =
      stx::rc::make_inplace<Window>(stx::os_allocator, std::move(api), window,
                                    WindowID{window_id}, cfg.extent, cfg.extent,
                                    std::move(cfg), std::this_thread::get_id())
          .unwrap();

  win->api->add_window_info(win->id, win.handle);

  return win;
}

}        // namespace ash
