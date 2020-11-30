#include <map>

#include "stx/option.h"
#include "stx/result.h"

// clang-format off
#include "vulkan/vulkan.h"
#include "GLFW/glfw3.h"
// clang-format on

#include "vlk/gl.h"
#include "vlk/gl_debug.h"

// EXT suffix => extensions. needs to be loaded before use
// PFN prefix => pointer function

using namespace vlk;

// TODO(lamarrr):
//    glfwSetCursorEnterCallback;
//    glfwSetCursorPosCallback;
//    glfwSetDropCallback;
//    glfwSetJoystickCallback;
//    glfwSetKeyCallback;
//    glfwSetMouseButtonCallback;
//    glfwSetScrollCallback;
//    glfwSetWindowCloseCallback;
//    glfwSetWindowFocusCallback;
//    glfwSetWindowPosCallback;
//    glfwSetWindowMaximizeCallback;
//    glfwSetWindowIconifyCallback;


struct Window {
  GLFWwindow* window;
  int width;
  int height;
};

struct Application {
 public:
  Application(int window_width, int window_height)
      : window_{nullptr, window_width, window_height},
        vk_instance_{nullptr},
        surface_{nullptr},
        logical_device_{nullptr},
        debug_messenger_{},
        default_debug_messenger_create_info_{} {}

  void run() {
    init_window_();
    init_vulkan_();

    // creates and binds the window surface (back buffer) to the glfw window
    VLK_ENSURE(glfwCreateWindowSurface(vk_instance_, window_.window, nullptr,
                                       &surface_) == VK_SUCCESS,
               "Unable to Create Window Surface");

    auto physical_devices = get_physical_devices(vk_instance_);
    auto [physical_device, prop, features] = most_suitable_physical_device(
        physical_devices, [=](DevicePropFt const& device_hpf) {
          auto const& [device, properties, features] = device_hpf;
          auto queue_families = get_queue_families(device);

          // check device has graphics queue
          auto graphics_queue_family_index =
              find_queue_family(queue_families, VK_QUEUE_GRAPHICS_BIT);

          // check that any of the device's graphics queue family has
          // surface presentation support for the window surface
          auto surface_presentation_queue_family_index =
              find_surface_presentation_queue_family(device, queue_families,
                                                     surface_);

          return graphics_queue_family_index.is_some() &&
                 features.geometryShader &&
                 surface_presentation_queue_family_index.is_some() &&
                 is_swapchain_adequate(
                     get_swapchain_properties(device, surface_));
        });

    VLK_LOG("Using Physical Device: " << name_physical_device(prop))

    std::vector<VkQueueFamilyProperties> queue_families =
        get_queue_families(physical_device);
    uint32_t graphics_queue_family_index =
        find_queue_family(queue_families, VK_QUEUE_GRAPHICS_BIT)
            .expect(
                "Selected physical device does not have graphics command "
                "queue");

    // find any queue that supports surface presentation
    uint32_t surface_presentation_queue_family_index =
        find_surface_presentation_queue_family(physical_device, queue_families,
                                               surface_)
            .expect(
                "Selected physical device does not have a surface presentation "
                "command queue");

    // the vector's length is equal to the number of command
    // queues to create on each of the queue family
    std::map<uint32_t, std::vector<float>> target_queue_families;

    target_queue_families[graphics_queue_family_index].push_back(1.0f);
    auto graphics_command_queue_index =
        target_queue_families[graphics_queue_family_index].size() - 1;
    target_queue_families[surface_presentation_queue_family_index].push_back(
        1.0f);
    auto surface_presentation_command_queue_index =
        target_queue_families[surface_presentation_queue_family_index].size() -
        1;

    std::vector<VkDeviceQueueCreateInfo> command_queue_create_infos;
    std::vector<uint32_t> unique_queue_families_indexes;

    for (auto& [queue_family_index, priorities] : target_queue_families) {
      command_queue_create_infos.push_back(
          make_command_queue_create_info(queue_family_index, priorities));
      unique_queue_families_indexes.push_back(queue_family_index);
    }

    // required extensions for the device
    constexpr char const* required_logical_device_extensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    logical_device_ = create_logical_device(
        physical_device, graphics_queue_family_index,
        required_logical_device_extensions, required_validation_layers_,
        command_queue_create_infos, nullptr);

    SwapChainProperties device_swapchain_properties =
        get_swapchain_properties(physical_device, surface_);

    VkSurfaceFormatKHR surface_format =
        select_surface_formats(device_swapchain_properties.supported_formats);
    VkPresentModeKHR surface_presentation_mode =
        select_surface_presentation_mode(
            device_swapchain_properties.presentation_modes);

    // it is loaded one the logical device is created as we already specified
    // it
    vkGetDeviceQueue(logical_device_, graphics_queue_family_index,
                     graphics_command_queue_index, &graphics_command_queue_);
    VLK_ENSURE(graphics_command_queue_ != nullptr,
               "Graphics command queue not created on target device");

    vkGetDeviceQueue(logical_device_, surface_presentation_queue_family_index,
                     surface_presentation_command_queue_index,
                     &surface_presentation_command_queue_);
    VLK_ENSURE(surface_presentation_command_queue_ != nullptr,
               "Graphics command queue not created on target device");

    VkExtent2D surface_extent = select_swapchain_extent(
        window_.window, device_swapchain_properties.capabilities);

    window_swapchain_ = create_swapchain(
        logical_device_, surface_, surface_extent, surface_format,
        surface_presentation_mode, device_swapchain_properties,
        graphics_queue_family_index == surface_presentation_queue_family_index
            ? VK_SHARING_MODE_EXCLUSIVE
            : VK_SHARING_MODE_CONCURRENT,
        unique_queue_families_indexes);

    uint32_t image_count;
    VLK_ENSURE(vkGetSwapchainImagesKHR(logical_device_, window_swapchain_,
                                       &image_count, nullptr) == VK_SUCCESS,
               "Unable to get swapchain images count");

    std::vector<VkImage> swapchain_images(image_count);

    VLK_ENSURE(vkGetSwapchainImagesKHR(logical_device_, window_swapchain_,
                                       &image_count,
                                       swapchain_images.data()) == VK_SUCCESS,
               "Unable to get swapchain images");

    VkImageView image_view = create_image_view(
        logical_device_, swapchain_images[0], surface_format.format);

    active_swapchain_image_views_.push_back(image_view);

    main_loop_();
    cleanup_();

    return;
  }

 private:
  void init_window_() {
    VLK_ENSURE(glfwInit() == GLFW_TRUE, "Unable to initialize GLFW");
    glfwWindowHint(
        GLFW_CLIENT_API,
        GLFW_NO_API);  // not an OpenGL app, do not create OpenGL context
    glfwWindowHint(GLFW_RESIZABLE,
                   GLFW_FALSE);  // requires handling the framebuffer size

    // width and height here refer to the screen coordinates and not the actual
    // pixels
    window_.window = glfwCreateWindow(window_.width, window_.height, "Valkyrie",
                                      nullptr, nullptr);
    VLK_ENSURE(window_.window != nullptr, "Window creation failed");
  }

  void init_vulkan_() {
#if VK_DEBUG
    default_debug_messenger_create_info_ = make_debug_messenger_create_info();
#endif

    vk_instance_ = create_vk_instance(&default_debug_messenger_create_info_,
                                      required_validation_layers_)
                       .expect("Unable to create Vulkan Instance");

#if VLK_DEBUG
    debug_messenger_ = create_install_debug_messenger(vk_instance_, nullptr);
#endif
  }

  void main_loop_() {
    while (!glfwWindowShouldClose(window_.window)) {
      glfwPollEvents();
    }
  }

  void cleanup_() {
    while (!active_swapchain_image_views_.empty()) {
      VkImageView image_view = active_swapchain_image_views_.back();
      vkDestroyImageView(logical_device_, image_view, nullptr);
      active_swapchain_image_views_.pop_back();
    }

    vkDestroySwapchainKHR(logical_device_, window_swapchain_, nullptr);
    vkDestroySurfaceKHR(vk_instance_, surface_, nullptr);

    vkDestroyDevice(logical_device_, nullptr);

#if VLK_DEBUG
    destroy_debug_messenger(vk_instance_, debug_messenger_, nullptr);
#endif

    vkDestroyInstance(vk_instance_, nullptr);

    glfwDestroyWindow(window_.window);
    glfwTerminate();
  }

  Window window_;

  VkInstance vk_instance_;

  // creation only needs the vulkan instance
  VkSurfaceKHR surface_;

  VkDevice logical_device_;

  // automatically cleaned on destruction of the logical device
  VkQueue graphics_command_queue_;
  VkQueue surface_presentation_command_queue_;

  VkSwapchainKHR window_swapchain_;

  std::vector<VkImageView> active_swapchain_image_views_;

  // only used in debug mode
  VkDebugUtilsMessengerEXT debug_messenger_;
  VkDebugUtilsMessengerCreateInfoEXT default_debug_messenger_create_info_;
  static constexpr char const* required_validation_layers_[] = {
      "VK_LAYER_KHRONOS_validation"};
};

int main() {
  Application app{1920, 1080};

  app.run();
}
