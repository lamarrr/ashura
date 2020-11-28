

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

struct Window {
  GLFWwindow* window;
  int width;
  int height;
};

struct Application {
 public:
  enum class Error {
    UnableToCreateVulkanInstance,
    // required glfw extensions not loaded to vulkan
    GlfwExtensionNotPresent
  };

  Application(int width, int height)
      : window_{nullptr, width, height},
        vk_instance_{nullptr},
#if VLK_DEBUG
        debug_messenger_{},
        default_debug_messenger_create_info_ {
  }
#endif
  {}

  void run() {
    init_window_();
    init_vulkan_();

    VLK_ENSURE(glfwCreateWindowSurface(vk_instance_, window_.window, nullptr,
                                       &surface_) == VK_SUCCESS,
               "Unable to Create Window Surface");

    auto physical_devices = get_physical_devices(vk_instance_);
    auto [physical_device, prop, features] = most_suitable_physical_device(
        vk_instance_, physical_devices, [=](DevicePropFt const& device_hpf) {
          auto const& [device, properties, features] = device_hpf;
          auto queue_families = get_queue_families(device);

          // check device has graphics queue
          auto queue_family_index =
              find_queue_family(device, queue_families, VK_QUEUE_GRAPHICS_BIT);
          if (queue_family_index.is_none()) return false;

          // check that the device's graphics queue has presentation queue
          // support for the window surface
          VkBool32 surface_presentation_queue_supported;
          vkGetPhysicalDeviceSurfaceSupportKHR(
              device, queue_family_index.clone().unwrap(), surface_,
              &surface_presentation_queue_supported);

          return features.geometryShader &&
                 surface_presentation_queue_supported &&
                 is_swapchain_adequate(
                     get_swapchain_properties(device, surface_));
        });

    VLK_LOG("Using Physical Device: " << name_physical_device(prop))

    std::vector<VkQueueFamilyProperties> queue_families =
        get_queue_families(physical_device);
    uint32_t graphics_queue_family_index =
        find_queue_family(physical_device, queue_families,
                          VK_QUEUE_GRAPHICS_BIT)
            .expect(
                "Selected physical device does not have graphics command "
                "queue");

    constexpr char const* required_logical_device_extensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    logical_device_ =
        create_logical_device(physical_device, graphics_queue_family_index,
                              required_logical_device_extensions,
                              required_validation_layers_, nullptr);

    // it is loaded one the logical device is created as we already specified
    // it
    vkGetDeviceQueue(logical_device_, graphics_queue_family_index, 0,
                     &graphics_queue_);

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
  VkDevice logical_device_;

  // automatically cleaned on destruction of the logical device
  VkQueue graphics_queue_;

  // creation only needs the vulkan instance
  VkSurfaceKHR surface_;

#if VLK_DEBUG
  VkDebugUtilsMessengerEXT debug_messenger_;
  VkDebugUtilsMessengerCreateInfoEXT default_debug_messenger_create_info_;
  static constexpr char const* required_validation_layers_[] = {
      "VK_LAYER_KHRONOS_validation"};
#endif
};

int main() {
  Application app{1920, 1080};

  app.run();
}
