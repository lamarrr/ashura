#include <map>

#include "stx/option.h"
#include "stx/result.h"

// clang-format off
#include "vulkan/vulkan.h"
#include "GLFW/glfw3.h"
// clang-format on

#include "vlk/gl.h"
#include "vlk/gl_debug.h"
#include "vlk/shader.h"
#include "vlk/config.h"

// EXT suffix => extensions. needs to be loaded before use
// PFN prefix => pointer function

using namespace vlk;

struct Window {
  GLFWwindow* window;
};

struct WindowConfig {
  int desired_width;
  int desired_height;
  bool resizable;
};

struct [[nodiscard]] Application {
 public:
  Application(WindowConfig const& window_config)
      : window_{nullptr},
        window_config_{window_config},
        vulkan_instance_{nullptr},
        surface_{nullptr},
        logical_device_{nullptr},
        debug_messenger_{},
        default_debug_messenger_create_info_{} {}

  void run() {
    init_window_();
    init_vulkan_();

    // creates and binds the window surface (back buffer) to the glfw window
    VLK_MUST_SUCCEED(glfwCreateWindowSurface(vulkan_instance_, window_.window,
                                             nullptr, &surface_),
                     "Unable to Create Window Surface");

    auto physical_devices = get_physical_devices(vulkan_instance_);
    auto [physical_device, prop, features] = most_suitable_physical_device(
        physical_devices,
        [surface_ = this->surface_](DevicePropFt const& device_hpf) -> bool {
          auto const& [device, properties, features] = device_hpf;
          auto queue_families = get_queue_families(device);

          // check device has graphics queue
          auto graphics_queue_support =
              get_command_queue_support(queue_families, VK_QUEUE_GRAPHICS_BIT);

          // check that any of the device's graphics queue family has
          // surface presentation support for the window surface
          auto surface_presentation_queue_support =
              get_surface_presentation_command_queue_support(
                  device, queue_families, surface_);

          auto swapchain_properties =
              get_swapchain_properties(device, surface_);

          return any_true(graphics_queue_support) &&
                 any_true(surface_presentation_queue_support) &&
                 features.geometryShader &&
                 is_swapchain_adequate(swapchain_properties);
        });

    VLK_LOG("Using Physical Device: " << name_physical_device(prop))

    std::vector<VkQueueFamilyProperties> queue_families =
        get_queue_families(physical_device);
    std::vector<bool> graphics_queue_support =
        get_command_queue_support(queue_families, VK_QUEUE_GRAPHICS_BIT);

    // find any queue that supports surface presentation
    std::vector<bool> surface_presentation_queue_support =
        get_surface_presentation_command_queue_support(
            physical_device, queue_families, surface_);

    auto graphics_queue_family_index =
        std::find(graphics_queue_support.begin(), graphics_queue_support.end(),
                  true) -
        graphics_queue_support.begin();

    auto surface_presentation_queue_family_index =
        std::find(surface_presentation_queue_support.begin(),
                  surface_presentation_queue_support.end(), true) -
        surface_presentation_queue_support.begin();

    // the vector's length is equal to the number of command
    // queues to create on each of the queue family
    std::map<uint32_t, std::vector<float>> target_queue_families;

    // TODO(lamarrr): ensure size must not exceed queue family's queueCount
    target_queue_families[graphics_queue_family_index].push_back(1.0f);
    auto graphics_command_queue_index =
        target_queue_families[graphics_queue_family_index].size() - 1;

    // trying to make sure we don't create more than one command queue per queue
    // family
    auto surface_presentation_command_queue_index =
        graphics_command_queue_index;
    if (surface_presentation_queue_family_index !=
        graphics_queue_family_index) {
      target_queue_families[surface_presentation_queue_family_index].push_back(
          1.0f);
      surface_presentation_command_queue_index =
          target_queue_families[surface_presentation_queue_family_index]
              .size() -
          1;
    }

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
        physical_device, required_logical_device_extensions,
        required_validation_layers_, command_queue_create_infos, nullptr);

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
            ? VK_SHARING_MODE_EXCLUSIVE  // command queue on same queue family
                                         // can share resources
            : VK_SHARING_MODE_CONCURRENT,
        unique_queue_families_indexes);

    uint32_t image_count;
    VLK_MUST_SUCCEED(vkGetSwapchainImagesKHR(logical_device_, window_swapchain_,
                                             &image_count, nullptr),
                     "Unable to get swapchain images count");

    std::vector<VkImage> swapchain_images(image_count);

    VLK_MUST_SUCCEED(
        vkGetSwapchainImagesKHR(logical_device_, window_swapchain_,
                                &image_count, swapchain_images.data()),
        "Unable to get swapchain images");

    for (auto swapchain_image : swapchain_images) {
      swapchain_image_views_.push_back(create_image_view(
          logical_device_, swapchain_image, surface_format.format));
    }

    // pipeline creation

    std::basic_string<uint32_t> const vert_shader_binary =
        load_spirv_binary(config::kSpirvBinariesPath / "triangle.vert.spv")
            .expect("Unable to load vertex shader binary");

    std::basic_string<uint32_t> const frag_shader_binary =
        load_spirv_binary(config::kSpirvBinariesPath / "triangle.frag.spv")
            .expect("Unable to load fragment shader binary");

    auto vert_shader_module =
        create_shader_module(logical_device_, vert_shader_binary);  // destroy

    auto frag_shader_module =
        create_shader_module(logical_device_, frag_shader_binary);  // destroy

    auto vert_shader_pipeline_shader_stage_create_info =
        make_pipeline_shader_stage_create_info(vert_shader_module, "main",
                                               VK_SHADER_STAGE_VERTEX_BIT);

    auto frag_shader_pipeline_shader_stage_create_info =
        make_pipeline_shader_stage_create_info(frag_shader_module, "main",
                                               VK_SHADER_STAGE_FRAGMENT_BIT);
    std::vector<VkPipelineShaderStageCreateInfo> shader_stages_create_info = {
        vert_shader_pipeline_shader_stage_create_info,
        frag_shader_pipeline_shader_stage_create_info};

    VkViewport const viewports[] = {
        make_viewport(0, 0, surface_extent.width, surface_extent.height)};
    VkRect2D const scissors[] = {
        VkRect2D{{0, 0}, {surface_extent.width, surface_extent.height}}};

    // the position of each element corresponds to the indexes of the active
    // framebuffers
    VkPipelineColorBlendAttachmentState const attachments_states[] = {
        make_pipeline_color_blend_attachment_state()};
    VkAttachmentDescription const attachments_descriptions[] = {
        make_attachment_description(surface_format.format)};

    VkAttachmentReference attachments_references[1] = {};
    attachments_references[0].attachment = 0;
    attachments_references[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription const subpasses_descriptions[] = {
        make_subpass_description(attachments_references)};

    VkDynamicState const dynamic_states[] = {VK_DYNAMIC_STATE_VIEWPORT,
                                             VK_DYNAMIC_STATE_LINE_WIDTH};
    auto pipeline_dynamic_state = make_pipeline_dynamic_state(dynamic_states);

    auto pipeline_layout = create_pipeline_layout(logical_device_);

    auto render_pass = create_render_pass(
        logical_device_, attachments_descriptions, subpasses_descriptions);

    auto graphics_pipeline = create_graphics_pipeline(
        logical_device_, pipeline_layout, render_pass,
        shader_stages_create_info,
        make_pipeline_vertex_input_state_create_info({}, {}),
        make_pipeline_input_assembly_state_create_info(),
        make_pipeline_viewport_state_create_info(viewports, scissors),
        make_pipeline_rasterization_create_info(),
        make_pipeline_multisample_state_create_info(),
        make_pipeline_depth_stencil_state_create_info(),
        make_pipeline_color_blend_state_create_info(attachments_states),
        pipeline_dynamic_state);

    std::vector<VkFramebuffer> swapchain_fraamebuffers;

    for (size_t i = 0; i < swapchain_image_views_.size(); i++) {
      VkImageView const attachments[] = {swapchain_image_views_[i]};
      auto frame_buffer = create_frame_buffer(logical_device_, render_pass,
                                              attachments, surface_extent);
      swapchain_fraamebuffers.push_back(frame_buffer);
    }

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
    window_.window = glfwCreateWindow(window_config_.desired_width,
                                      window_config_.desired_height, "Valkyrie",
                                      nullptr, nullptr);
    VLK_ENSURE(window_.window != nullptr, "Window creation failed");
  }

  void init_vulkan_() {
#if VK_DEBUG
    default_debug_messenger_create_info_ = make_debug_messenger_create_info();
#endif

    // get list of extensions required for vulkan interfacing with the window
    // system
    uint32_t glfw_req_extensions_count = 0;
    char const** glfw_req_extensions_names;

    glfw_req_extensions_names =
        glfwGetRequiredInstanceExtensions(&glfw_req_extensions_count);

    VLK_LOG("Required GLFW Extensions:");
    for (size_t i = 0; i < glfw_req_extensions_count; i++) {
      VLK_LOG("\t" << glfw_req_extensions_names[i]);
    }

    std::vector<char const*> required_extensions;
    // TODO(lamarrr): deduction guides
    for (auto extension : stx::Span<char const* const>(
             glfw_req_extensions_names, glfw_req_extensions_count)) {
      required_extensions.push_back(extension);
    }

#if VLK_DEBUG
    required_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
    vulkan_instance_ =
        create_vulkan_instance(required_extensions, required_validation_layers_,
                               &default_debug_messenger_create_info_);

#if VLK_DEBUG
    debug_messenger_ =
        create_install_debug_messenger(vulkan_instance_, nullptr);
#endif
  }

  void main_loop_() {
    while (!glfwWindowShouldClose(window_.window)) {
      glfwPollEvents();
    }
  }

  void cleanup_() {
    for (auto image_view : swapchain_image_views_) {
      vkDestroyImageView(logical_device_, image_view, nullptr);
    }

    vkDestroySwapchainKHR(logical_device_, window_swapchain_, nullptr);
    vkDestroySurfaceKHR(vulkan_instance_, surface_, nullptr);

    vkDestroyDevice(logical_device_, nullptr);

#if VLK_DEBUG
    destroy_debug_messenger(vulkan_instance_, debug_messenger_, nullptr);
#endif

    vkDestroyInstance(vulkan_instance_, nullptr);

    glfwDestroyWindow(window_.window);
    glfwTerminate();
  }

  Window window_;
  WindowConfig window_config_;

  VkInstance vulkan_instance_;

  // creation only needs the vulkan instance, a.k.a. backbuffer
  VkSurfaceKHR surface_;

  VkDevice logical_device_;

  // automatically cleaned on destruction of the logical device
  VkQueue graphics_command_queue_;
  VkQueue surface_presentation_command_queue_;

  VkSwapchainKHR window_swapchain_;

  std::vector<VkImageView> swapchain_image_views_;

  // only used in debug mode
  VkDebugUtilsMessengerEXT debug_messenger_;
  VkDebugUtilsMessengerCreateInfoEXT default_debug_messenger_create_info_;
  static constexpr char const* required_validation_layers_[] = {
      "VK_LAYER_KHRONOS_validation"};
};

int main() {
  Application app{WindowConfig{
      .desired_width = 1920, .desired_height = 1080, .resizable = true}};
  app.run();
}
