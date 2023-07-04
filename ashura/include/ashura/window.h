#pragma once

#include <utility>

#include "SDL3/SDL.h"
#include "SDL3/SDL_vulkan.h"
#include "ashura/event.h"
#include "ashura/image.h"
#include "ashura/primitives.h"
#include "ashura/sdl_utils.h"
#include "ashura/utils.h"
#include "ashura/vulkan.h"
#include "stx/enum.h"
#include "stx/fn.h"
#include "stx/string.h"
#include "stx/vec.h"

namespace ash
{

enum class WindowType : u8
{
  Normal,
  Utility,
  Tooltip,
  Popup
};

enum class WindowCreateFlags
{
  None         = 0,
  Hidden       = 1 << 0,
  NonResizable = 1 << 1,
  Borderless   = 1 << 2,
  FullScreen   = 1 << 3,
  AlwaysOnTop  = 1 << 4
};

STX_DEFINE_ENUM_BIT_OPS(WindowCreateFlags)

enum class SwapChainState : u8
{
  Ok            = 0,
  ExtentChanged = 1,        // the window's extent and surface (framebuffer) extent has changed
  Suboptimal    = 2,        // the window swapchain can still be used for presentation but is not optimal for presentation in its present state
  OutOfDate     = 4,        // the window swapchain is now out of date and needs to be changed
  All           = 7
};

STX_DEFINE_ENUM_BIT_OPS(SwapChainState)

struct Window
{
  Window(SDL_Window *iwindow) :
      window{iwindow}
  {}

  STX_MAKE_PINNED(Window)

  // IMPORTANT: window must be destructed on the same thread that created it
  ~Window()
  {
    if (surface.is_some())
    {
      // TODO(lamarrr): an unspecified lifetime reference is bound to the device here
      surface.value()->destroy();
    }

    SDL_DestroyWindow(window);
  }

  static stx::Vec<char const *> get_required_instance_extensions()
  {
    u32 ext_count;

    ASH_SDL_CHECK(SDL_Vulkan_GetInstanceExtensions(&ext_count, nullptr) == SDL_TRUE);

    stx::Vec<char const *> required_instance_extensions;

    required_instance_extensions.resize(ext_count).unwrap();

    ASH_SDL_CHECK(SDL_Vulkan_GetInstanceExtensions(&ext_count, required_instance_extensions.data()) == SDL_TRUE);

    return required_instance_extensions;
  }

  void set_title(char const *title)
  {
    ASH_SDL_CHECK(SDL_SetWindowTitle(window, title) == 0);
  }

  stx::String get_title()
  {
    return stx::string::make(stx::os_allocator, SDL_GetWindowTitle(window)).unwrap();
  }

  void maximize()
  {
    ASH_SDL_CHECK(SDL_MaximizeWindow(window) == 0);
  }

  void minimize()
  {
    ASH_SDL_CHECK(SDL_MinimizeWindow(window) == 0);
  }

  void set_size(extent size)
  {
    ASH_SDL_CHECK(SDL_SetWindowSize(window, AS(int, size.width), AS(int, size.height)) == 0);
  }

  void center()
  {
    ASH_SDL_CHECK(SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED) == 0);
  }

  extent get_size()
  {
    int w, h;
    ASH_SDL_CHECK(SDL_GetWindowSize(window, &w, &h) == 0);
    return extent{.width = AS(u32, w), .height = AS(u32, h)};
  }

  extent get_surface_size()
  {
    int w, h;
    ASH_SDL_CHECK(SDL_GetWindowSizeInPixels(window, &w, &h) == 0);
    return extent{.width = AS(u32, w), .height = AS(u32, h)};
  }

  void set_position(offseti pos)
  {
    ASH_SDL_CHECK(SDL_SetWindowPosition(window, AS(int, pos.x), AS(int, pos.y)) == 0);
  }

  offseti get_position()
  {
    int x, y;
    ASH_SDL_CHECK(SDL_GetWindowPosition(window, &x, &y) == 0);
    return offseti{.x = AS(i32, x), .y = AS(i32, y)};
  }

  void set_min_size(extent min)
  {
    ASH_SDL_CHECK(SDL_SetWindowMinimumSize(window, AS(int, min.width), AS(int, min.height)) == 0);
  }

  extent get_min_size()
  {
    int w, h;
    ASH_SDL_CHECK(SDL_GetWindowMinimumSize(window, &w, &h) == 0);
    return extent{.width = AS(u32, w), .height = AS(u32, h)};
  }

  void set_max_size(extent max)
  {
    ASH_SDL_CHECK(SDL_SetWindowMaximumSize(window, AS(int, max.width), AS(int, max.height)) == 0);
  }

  extent get_max_size()
  {
    int w, h;
    ASH_SDL_CHECK(SDL_GetWindowMaximumSize(window, &w, &h) == 0);
    return extent{.width = AS(u32, w), .height = AS(u32, h)};
  }

  void set_icon(ImageView image)
  {
    SDL_PixelFormatEnum fmt = SDL_PIXELFORMAT_RGBA8888;

    switch (image.format)
    {
      case ImageFormat::Rgba8888:
        fmt = SDL_PIXELFORMAT_RGBA8888;
        break;
      case ImageFormat::Bgra8888:
        fmt = SDL_PIXELFORMAT_RGBA8888;
        break;
      case ImageFormat::Rgb888:
        fmt = SDL_PIXELFORMAT_RGB888;
        break;
      default:
        ASH_PANIC("unsupported icon image format");
        break;
    }

    SDL_Surface *icon = SDL_CreateSurfaceFrom((void *) image.data.data(), AS(int, image.extent.width), AS(int, image.extent.height), AS(int, image.extent.width *nchannel_bytes(image.format)), fmt);
    ASH_SDL_CHECK(icon != nullptr);
    ASH_SDL_CHECK(SDL_SetWindowIcon(window, icon) == 0);
    SDL_DestroySurface(icon);
  }

  void make_bordered()
  {
    ASH_SDL_CHECK(SDL_SetWindowBordered(window, SDL_TRUE) == 0);
  }

  void make_borderless()
  {
    ASH_SDL_CHECK(SDL_SetWindowBordered(window, SDL_FALSE) == 0);
  }

  void show()
  {
    ASH_SDL_CHECK(SDL_ShowWindow(window) == 0);
  }

  void hide()
  {
    ASH_SDL_CHECK(SDL_HideWindow(window) == 0);
  }

  void raise()
  {
    ASH_SDL_CHECK(SDL_RaiseWindow(window) == 0);
  }

  void restore()
  {
    ASH_SDL_CHECK(SDL_RestoreWindow(window) == 0);
  }

  void request_attention(bool briefly)
  {
    ASH_SDL_CHECK(SDL_FlashWindow(window, briefly ? SDL_FLASH_BRIEFLY : SDL_FLASH_UNTIL_FOCUSED) == 0);
  }

  void make_fullscreen()
  {
    // SDL_SetWindowFullscreenMode()
    ASH_SDL_CHECK(SDL_SetWindowFullscreen(window, SDL_TRUE) == 0);
  }

  void make_windowed()
  {
    ASH_SDL_CHECK(SDL_SetWindowFullscreen(window, SDL_FALSE) == 0);
  }

  void make_resizable()
  {
    ASH_SDL_CHECK(SDL_SetWindowResizable(window, SDL_TRUE) == 0);
  }

  void make_unresizable()
  {
    ASH_SDL_CHECK(SDL_SetWindowResizable(window, SDL_FALSE) == 0);
  }

  void set_always_on_top(bool always)
  {
    ASH_SDL_CHECK(SDL_SetWindowAlwaysOnTop(window, AS(SDL_bool, always)) == 0);
  }

  void on(WindowEvents event, stx::UniqueFn<void(WindowEvents)> action)
  {
    event_listeners.general.push(std::make_pair(event, std::move(action))).unwrap();
  }

  void on_key_down(stx::UniqueFn<void(Key, KeyModifiers)> action)
  {
    event_listeners.key_down.push(std::move(action)).unwrap();
  }

  void on_key_up(stx::UniqueFn<void(Key, KeyModifiers)> action)
  {
    event_listeners.key_up.push(std::move(action)).unwrap();
  }

  void on_mouse_motion(stx::UniqueFn<void(MouseMotionEvent)> action)
  {
    event_listeners.mouse_motion.push(std::move(action)).unwrap();
  }

  void on_mouse_click(stx::UniqueFn<void(MouseClickEvent)> action)
  {
    event_listeners.mouse_click.push(std::move(action)).unwrap();
  }

  // TODO(lamarrr): on keypressed
  // TODO(lamarrr): repeat all widget events

  // attach surface to window for presentation
  void attach_surface(stx::Rc<vk::Instance *> const &instance)
  {
    this->instance = stx::Some(instance.share());

    VkSurfaceKHR surface;

    ASH_SDL_CHECK(SDL_Vulkan_CreateSurface(window, instance->instance, &surface) == SDL_TRUE, "unable to create surface for window");

    this->surface = stx::Some(stx::rc::make_unique(stx::os_allocator, vk::Surface{.surface = surface, .instance = instance->instance}).unwrap());
  }

  void recreate_swapchain(stx::Rc<vk::CommandQueue *> const &queue, u32 max_nframes_in_flight)        // TODO(lamarrr): use manual logger
  {
    // if cause of change in swapchain is a change in extent, then mark
    // layout as dirty, otherwise maintain pipeline state
    int width, height;
    ASH_SDL_CHECK(SDL_GetWindowSize(window, &width, &height) == 0);

    int surface_width, surface_height;
    ASH_SDL_CHECK(SDL_GetWindowSizeInPixels(window, &surface_width, &surface_height) == 0);

    VkSurfaceFormatKHR preferred_formats[] = {{VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
                                              {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
                                              {VK_FORMAT_R16G16B16A16_SFLOAT, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR}};

    // VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    // VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT;
    // VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT;
    // VK_COLOR_SPACE_DISPLAY_P3_LINEAR_EXT;
    // VK_COLOR_SPACE_DCI_P3_NONLINEAR_EXT;
    // VK_COLOR_SPACE_HDR10_ST2084_EXT;
    // VK_COLOR_SPACE_HDR10_HLG_EXT;
    // VK_COLOR_SPACE_ADOBERGB_LINEAR_EXT;
    // VK_COLOR_SPACE_ADOBERGB_NONLINEAR_EXT;
    // VK_COLOR_SPACE_EXTENDED_SRGB_NONLINEAR_EXT;
    // VK_COLOR_SPACE_DCI_P3_LINEAR_EXT;

    VkPresentModeKHR preferred_present_modes[] = {VK_PRESENT_MODE_IMMEDIATE_KHR,VK_PRESENT_MODE_FIFO_KHR,  VK_PRESENT_MODE_FIFO_RELAXED_KHR,
                                                  VK_PRESENT_MODE_MAILBOX_KHR};

    VkSampleCountFlagBits msaa_sample_count = queue->device->phy_dev->get_max_sample_count();

    surface.value()->change_swapchain(queue, max_nframes_in_flight, preferred_formats, preferred_present_modes,
                                      VkExtent2D{.width = AS(u32, surface_width), .height = AS(u32, surface_height)},
                                      VkExtent2D{.width = AS(u32, width), .height = AS(u32, height)}, msaa_sample_count,
                                      VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR);
  }

  std::pair<SwapChainState, u32> acquire_image()
  {
    ASH_CHECK(surface.is_some(), "trying to present to swapchain without having surface attached");
    ASH_CHECK(surface.value()->swapchain.is_some(), "trying to present to swapchain without having one");

    vk::SwapChain &swapchain = surface.value()->swapchain.value();

    VkSemaphore semaphore = swapchain.image_acquisition_semaphores[swapchain.frame];

    VkFence fence = VK_NULL_HANDLE;

    u32 swapchain_image_index = 0;

    VkResult result = vkAcquireNextImageKHR(swapchain.dev, swapchain.swapchain, VULKAN_TIMEOUT, semaphore, fence, &swapchain_image_index);

    ASH_CHECK(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR, "failed to acquire swapchain image");

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

  SwapChainState present(VkQueue queue, u32 swapchain_image_index)
  {
    ASH_CHECK(surface.is_some(), "trying to present to swapchain without having surface attached");
    ASH_CHECK(surface.value()->swapchain.is_some(), "trying to present to swapchain without having one");

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
    VkPresentInfoKHR present_info{.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                                  .pNext              = nullptr,
                                  .waitSemaphoreCount = 1,
                                  .pWaitSemaphores    = &swapchain.render_semaphores[swapchain.frame],
                                  .swapchainCount     = 1,
                                  .pSwapchains        = &swapchain.swapchain,
                                  .pImageIndices      = &swapchain_image_index,
                                  .pResults           = nullptr};

    VkResult result = vkQueuePresentKHR(queue, &present_info);

    ASH_CHECK(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR, "failed to present swapchain image");

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

  // void enable_hit_testing();
  //
  // SDL_HITTEST_NORMAL
  //
  // region is normal and has no special properties
  //
  // SDL_HITTEST_DRAGGABLE
  //
  // region can drag entire window
  //
  // SDL_HITTEST_RESIZE_TOPLEFT
  //
  // region can resize top left window
  //
  // SDL_HITTEST_RESIZE_TOP
  //
  // region can resize top window
  //
  // SDL_HITTEST_RESIZE_TOPRIGHT
  //
  // region can resize top right window
  //
  // SDL_HITTEST_RESIZE_RIGHT
  //
  // region can resize right window
  //
  // SDL_HITTEST_RESIZE_BOTTOMRIGHT
  //
  // region can resize bottom right window
  //
  // SDL_HITTEST_RESIZE_BOTTOM
  //
  // region can resize bottom window
  //
  // SDL_HITTEST_RESIZE_BOTTOMLEFT
  //
  // region can resize bottom left window
  //
  // SDL_HITTEST_RESIZE_LEFT
  //
  // region can resize left window
  //

  SDL_Window                             *window = nullptr;
  stx::Option<stx::Unique<vk::Surface *>> surface;
  stx::Option<stx::Rc<vk::Instance *>>    instance;
  WindowEventListeners                    event_listeners;
};

}        // namespace ash
