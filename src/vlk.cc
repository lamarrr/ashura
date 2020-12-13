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

struct WindowConfig {
  // in pixels
  int desired_width;
  int desired_height;
  bool resizable;
};

struct Window {
  GLFWwindow* window;
  // in pixels
  VkExtent2D surface_extent;
};

constexpr uint64_t kWaitTimeout =
    static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(
                              std::chrono::seconds(45))
                              .count());

struct Application;

static void application_window_resize_callback(GLFWwindow* window, int width,
                                               int height);

struct [[nodiscard]] Application {
 public:
  Application(WindowConfig const& window_config)
      : window_{},
        window_config_{window_config},
        clear_values_{VkClearValue{1.0f, 1.0f, 1.0f, 1.0f}},
        vulkan_instance_{nullptr},
        surface_{nullptr},
        physical_device_{nullptr},
        logical_device_{nullptr},
        debug_messenger_{},
        max_frames_in_flight_{2},
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

    physical_device_ = physical_device;

    VLK_LOG("Using Physical Device: " << name_physical_device(prop))

    std::vector<VkQueueFamilyProperties> queue_families =
        get_queue_families(physical_device_);
    std::vector<bool> graphics_queue_support =
        get_command_queue_support(queue_families, VK_QUEUE_GRAPHICS_BIT);

    // find any queue that supports surface presentation
    std::vector<bool> surface_presentation_queue_support =
        get_surface_presentation_command_queue_support(
            physical_device_, queue_families, surface_);

    graphics_queue_family_index_ =
        std::find(graphics_queue_support.begin(), graphics_queue_support.end(),
                  true) -
        graphics_queue_support.begin();

    surface_presentation_queue_family_index_ =
        std::find(surface_presentation_queue_support.begin(),
                  surface_presentation_queue_support.end(), true) -
        surface_presentation_queue_support.begin();

    // the vector's length is equal to the number of command
    // queues to create on each of the queue family
    std::map<uint32_t, std::vector<float>> target_queue_families;

    // TODO(lamarrr): ensure size must not exceed queue family's queueCount
    target_queue_families[graphics_queue_family_index_].push_back(1.0f);
    graphics_command_queue_index_ =
        target_queue_families[graphics_queue_family_index_].size() - 1;

    // trying to make sure we don't create more than one command queue per queue
    // family
    surface_presentation_command_queue_index_ = graphics_command_queue_index_;
    if (surface_presentation_queue_family_index_ !=
        graphics_queue_family_index_) {
      target_queue_families[surface_presentation_queue_family_index_].push_back(
          1.0f);
      surface_presentation_command_queue_index_ =
          target_queue_families[surface_presentation_queue_family_index_]
              .size() -
          1;
    }

    std::vector<VkDeviceQueueCreateInfo> command_queue_create_infos;

    for (auto& [queue_family_index, priorities] : target_queue_families) {
      command_queue_create_infos.push_back(
          make_command_queue_create_info(queue_family_index, priorities));
      unique_queue_families_indexes_.push_back(queue_family_index);
    }

    // required extensions for the device
    constexpr char const* required_logical_device_extensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    logical_device_ = create_logical_device(
        physical_device_, required_logical_device_extensions,
        required_validation_layers_, command_queue_create_infos, nullptr);

    /*========== Command Queue Fetching ==========*/

    // it is already added onto the create_info of the logical device
    graphics_command_queue_ =
        get_command_queue(logical_device_, graphics_queue_family_index_,
                          graphics_command_queue_index_);

    surface_presentation_command_queue_ = get_command_queue(
        logical_device_, surface_presentation_queue_family_index_,
        surface_presentation_command_queue_index_);

    /*========== Shader Loading ==========*/

    std::basic_string<uint32_t> const vert_shader_binary =
        load_spirv_binary(config::kSpirvBinariesPath / "triangle.vert.spv")
            .expect("Unable to load vertex shader binary");

    std::basic_string<uint32_t> const frag_shader_binary =
        load_spirv_binary(config::kSpirvBinariesPath / "triangle.frag.spv")
            .expect("Unable to load fragment shader binary");

    vert_shader_module_ =
        create_shader_module(logical_device_, vert_shader_binary);  // destroy

    frag_shader_module_ =
        create_shader_module(logical_device_, frag_shader_binary);  // destroy

    /*=====================================*/

    create_swapchain_();

    create_image_views_();

    create_pipeline_();

    create_framebuffers_();

    create_command_pool_();

    allocate_command_buffers_();

    record_command_buffers_();

    create_synchronization_objects_();

    swapchain_dirty_ = false;

    main_loop_();

    cleanup_();

    return;
  }

 private:
  void create_swapchain_() {
    device_swapchain_properties_ =
        get_swapchain_properties(physical_device_, surface_);

    device_surface_format_ =
        select_surface_formats(device_swapchain_properties_.supported_formats);
    device_surface_presentation_mode_ = select_surface_presentation_mode(
        device_swapchain_properties_.presentation_modes);

    window_.surface_extent = select_swapchain_extent(
        window_.window, device_swapchain_properties_.capabilities);

    window_swapchain_ = create_swapchain(
        logical_device_, surface_, window_.surface_extent,
        device_surface_format_, device_surface_presentation_mode_,
        device_swapchain_properties_,
        graphics_queue_family_index_ == surface_presentation_queue_family_index_
            ? VK_SHARING_MODE_EXCLUSIVE  // command queue on same queue family
                                         // can share resources
            : VK_SHARING_MODE_CONCURRENT,
        unique_queue_families_indexes_);
  }

  void destroy_swapchain_() {
    vkDestroySwapchainKHR(logical_device_, window_swapchain_, nullptr);
  }

  void create_image_views_() {
    swapchain_image_views_.clear();
    std::vector<VkImage> swapchain_images =
        get_swapchain_images(logical_device_, window_swapchain_);

    for (auto swapchain_image : swapchain_images) {
      swapchain_image_views_.push_back(create_image_view(
          logical_device_, swapchain_image, device_surface_format_.format));
    }
  }

  void destroy_image_views_() {
    for (auto image_view : swapchain_image_views_) {
      vkDestroyImageView(logical_device_, image_view, nullptr);
    }
  }

  void create_pipeline_() {
    auto vert_shader_pipeline_shader_stage_create_info =
        make_pipeline_shader_stage_create_info(vert_shader_module_, "main",
                                               VK_SHADER_STAGE_VERTEX_BIT);

    auto frag_shader_pipeline_shader_stage_create_info =
        make_pipeline_shader_stage_create_info(frag_shader_module_, "main",
                                               VK_SHADER_STAGE_FRAGMENT_BIT);

    std::vector<VkPipelineShaderStageCreateInfo> shader_stages_create_info = {
        vert_shader_pipeline_shader_stage_create_info,
        frag_shader_pipeline_shader_stage_create_info};

    VkViewport const viewports[] = {get_viewport_()};  // dynamic pipeline state
    VkRect2D const scissors[] = {get_scissor_()};      // dynamic pipeline state

    // the position of each element corresponds to the indexes of the active
    // framebuffers
    VkPipelineColorBlendAttachmentState const attachments_states[] = {
        make_pipeline_color_blend_attachment_state()};
    VkAttachmentDescription const attachments_descriptions[] = {
        make_attachment_description(device_surface_format_.format)};

    VkAttachmentReference attachments_references[1] = {};
    attachments_references[0].attachment = 0;
    attachments_references[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription const subpasses_descriptions[] = {
        make_subpass_description(attachments_references)};

    // will be set on command buffer recording
    VkDynamicState const dynamic_states[] = {VK_DYNAMIC_STATE_VIEWPORT,
                                             VK_DYNAMIC_STATE_SCISSOR,
                                             VK_DYNAMIC_STATE_LINE_WIDTH};
    auto pipeline_dynamic_state = make_pipeline_dynamic_state(dynamic_states);

    pipeline_layout_ = create_pipeline_layout(logical_device_);

    VkSubpassDependency const subpass_dependencies[] = {
        make_subpass_dependency()};

    render_pass_ =
        create_render_pass(logical_device_, attachments_descriptions,
                           subpasses_descriptions, subpass_dependencies);

    graphics_pipeline_ = create_graphics_pipeline(
        logical_device_, pipeline_layout_, render_pass_,
        shader_stages_create_info,
        make_pipeline_vertex_input_state_create_info({}, {}),
        make_pipeline_input_assembly_state_create_info(),
        make_pipeline_viewport_state_create_info(viewports, scissors),
        make_pipeline_rasterization_create_info(),
        make_pipeline_multisample_state_create_info(),
        make_pipeline_depth_stencil_state_create_info(),
        make_pipeline_color_blend_state_create_info(attachments_states),
        pipeline_dynamic_state);
  }

  void destroy_pipeline_() {
    vkDestroyPipelineLayout(logical_device_, pipeline_layout_, nullptr);
    vkDestroyRenderPass(logical_device_, render_pass_, nullptr);
    vkDestroyPipeline(logical_device_, graphics_pipeline_, nullptr);
  }

  void create_framebuffers_() {
    swapchain_framebuffers_.clear();
    for (auto image_view : swapchain_image_views_) {
      VkImageView const attachments[] = {image_view};
      auto frame_buffer = create_frame_buffer(
          logical_device_, render_pass_, attachments, window_.surface_extent);
      swapchain_framebuffers_.push_back(frame_buffer);
    }
  }

  void destroy_framebuffers_() {
    for (auto frame_buffer : swapchain_framebuffers_) {
      vkDestroyFramebuffer(logical_device_, frame_buffer, nullptr);
    }
  }

  void create_command_pool_() {
    graphics_command_pool_ =
        create_command_pool(logical_device_, graphics_queue_family_index_);
  }

  void allocate_command_buffers_() {
    graphics_command_buffers_.clear();
    graphics_command_buffers_.resize(swapchain_framebuffers_.size());
    allocate_command_buffers(logical_device_, graphics_command_pool_,
                             graphics_command_buffers_);
  }

  void record_command_buffers_() {
    VkViewport const viewports[] = {get_viewport_()};
    VkRect2D const scissors[] = {get_scissor_()};
    for (size_t i = 0; i < swapchain_framebuffers_.size(); i++) {
      begin_command_buffer_recording(graphics_command_buffers_[i]);
      auto render_area = VkRect2D{
          {0, 0},
          {window_.surface_extent.width, window_.surface_extent.height}};
      cmd::begin_render_pass(render_pass_, graphics_command_buffers_[i],
                             swapchain_framebuffers_[i], render_area,
                             clear_values_);
      cmd::bind_pipeline(graphics_pipeline_, graphics_command_buffers_[i]);
      cmd::set_viewports(graphics_command_buffers_[i], viewports);
      cmd::set_scissors(graphics_command_buffers_[i], scissors);
      cmd::set_line_width(graphics_command_buffers_[i], 1.0f);
      cmd::draw(graphics_command_buffers_[i], 3, 1, 0, 0);
      cmd::end_render_pass(graphics_command_buffers_[i]);
      end_command_buffer_recording(graphics_command_buffers_[i]);
    }
  }

  void destroy_command_pool_() {
    vkDestroyCommandPool(logical_device_, graphics_command_pool_, nullptr);
  }

  void create_synchronization_objects_() {
    image_available_semaphores_.clear();
    rendering_finished_semaphores_.clear();
    in_flight_fences_.clear();

    for (uint32_t i = 0; i < max_frames_in_flight_; i++) {
      image_available_semaphores_.push_back(create_semaphore(logical_device_));
      rendering_finished_semaphores_.push_back(
          create_semaphore(logical_device_));
      in_flight_fences_.push_back(create_fence(logical_device_, true));
    }
  }

  void destroy_synchronization_objects_() {
    for (auto semaphore : image_available_semaphores_) {
      vkDestroySemaphore(logical_device_, semaphore, nullptr);
    }

    for (auto semaphore : rendering_finished_semaphores_) {
      vkDestroySemaphore(logical_device_, semaphore, nullptr);
    }

    for (auto fence : in_flight_fences_) {
      vkDestroyFence(logical_device_, fence, nullptr);
    }
  }

  VkViewport get_viewport_() {
    return make_viewport(0, 0, window_.surface_extent.width,
                         window_.surface_extent.height);
  }

  VkRect2D get_scissor_() {
    return make_scissor(0, 0, window_.surface_extent.width,
                        window_.surface_extent.height);
  }

  void init_window_() {
    VLK_ENSURE(glfwInit() == GLFW_TRUE, "Unable to initialize GLFW");
    glfwWindowHint(
        GLFW_CLIENT_API,
        GLFW_NO_API);  // not an OpenGL app, do not create OpenGL context

    glfwWindowHint(
        GLFW_RESIZABLE,
        window_config_.resizable);  // requires handling the framebuffer size

    // width and height here refer to the screen coordinates and not the actual
    // pixels
    window_.window = glfwCreateWindow(window_config_.desired_width,
                                      window_config_.desired_height, "Valkyrie",
                                      nullptr, nullptr);

    VLK_ENSURE(window_.window != nullptr, "Window creation failed");

    glfwSetWindowUserPointer(window_.window, this);
    glfwSetWindowSizeCallback(window_.window,
                              application_window_resize_callback);
  }

  void init_vulkan_() {
    default_debug_messenger_create_info_ = make_debug_messenger_create_info();

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

  void draw_frame_(uint32_t frame_flight_index) {
    // - Acquire an image from the swap chain
    // - Execute the command buffer with that image as attachment in the
    // framebuffer
    // - Return the image to the swap chain for presentation
    //
    // Each of these events is set in motion using a single function call, but
    // they are executed asynchronously. The function calls will return before
    // the operations are actually finished and the **order of execution** is
    // also undefined. That is unfortunate, because each of the operations
    // depends on the previous one finishing.
    //
    // Fences are mainly designed to synchronize your application itself with
    // rendering operation, whereas semaphores are used to synchronize
    // operations within or across command queues

    // wait for the image using the present flight synchronization values to
    // finish

    VLK_MUST_SUCCEED(
        vkWaitForFences(logical_device_, 1,
                        in_flight_fences_.data() + frame_flight_index, VK_TRUE,
                        kWaitTimeout),
        "Error occured waiting for synchronization fences");

    VLK_MUST_SUCCEED(
        vkResetFences(logical_device_, 1,
                      in_flight_fences_.data() + frame_flight_index),
        "Error occured resetting synchronization fence");

    uint32_t swapchain_image_index;

    auto image_acquire_result = vkAcquireNextImageKHR(
        logical_device_, window_swapchain_, kWaitTimeout,
        /* notify */ image_available_semaphores_[frame_flight_index],
        VK_NULL_HANDLE, &swapchain_image_index);
    VLK_ENSURE(image_acquire_result == VK_SUCCESS ||
                   image_acquire_result == VK_ERROR_OUT_OF_DATE_KHR,
               "Unable to acquire swapchain image");

    if (image_acquire_result == VK_ERROR_OUT_OF_DATE_KHR) {
      swapchain_dirty_ = true;
      return;
    }

    {
      VkSemaphore const await_semaphores[] = {
          image_available_semaphores_[frame_flight_index]};
      VkPipelineStageFlags const await_stages[] = {
          VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
      VkSemaphore const notify_semaphores[] = {
          rendering_finished_semaphores_[frame_flight_index]};
      submit_buffer(graphics_command_queue_,
                    graphics_command_buffers_[swapchain_image_index],
                    await_semaphores, await_stages, notify_semaphores,
                    in_flight_fences_[frame_flight_index]);
    }

    {
      VkSwapchainKHR const swapchains[] = {window_swapchain_};
      uint32_t const swapchain_image_indexes[] = {swapchain_image_index};
      VkSemaphore const await_semaphores[] = {
          rendering_finished_semaphores_[frame_flight_index]};

      if (present_to_swapchains(surface_presentation_command_queue_,
                                await_semaphores, swapchains,
                                swapchain_image_indexes) != VK_SUCCESS) {
        swapchain_dirty_ = true;
        return;
      }
    }
  }

  void recreate_swapchain_() {
    vkDeviceWaitIdle(logical_device_);  // to ensure we don't modify any
                                        // resource while in use
    int width, height;
    glfwGetFramebufferSize(window_.window, &width, &height);
    window_.surface_extent.width = width;
    window_.surface_extent.height = height;

    destroy_swapchain_();
    destroy_image_views_();
    destroy_pipeline_();
    destroy_framebuffers_();
    destroy_command_pool_();
    destroy_synchronization_objects_();

    create_swapchain_();
    create_image_views_();
    create_pipeline_();
    create_framebuffers_();
    create_command_pool_();
    allocate_command_buffers_();
    record_command_buffers_();
    create_synchronization_objects_();
  }

  void main_loop_() {
    uint32_t frame_flight_index = 0;
    while (!glfwWindowShouldClose(window_.window)) {
      glfwPollEvents();

      if (swapchain_dirty_) {
        recreate_swapchain_();
        swapchain_dirty_ = false;
        continue;
      }

      draw_frame_(frame_flight_index);
      frame_flight_index = (frame_flight_index + 1) % max_frames_in_flight_;
    }

    vkDeviceWaitIdle(logical_device_);  // do this before cleaning up resources
  }

  void cleanup_() {
    destroy_swapchain_();
    destroy_image_views_();
    destroy_pipeline_();
    destroy_framebuffers_();
    destroy_command_pool_();
    destroy_synchronization_objects_();

    /*==================*/
    vkDestroyShaderModule(logical_device_, frag_shader_module_, nullptr);
    vkDestroyShaderModule(logical_device_, vert_shader_module_, nullptr);

    vkDestroySurfaceKHR(vulkan_instance_, surface_, nullptr);

    vkDestroyDevice(logical_device_, nullptr);

#if VLK_DEBUG
    destroy_debug_messenger(vulkan_instance_, debug_messenger_, nullptr);
#endif

    vkDestroyInstance(vulkan_instance_, nullptr);

    glfwDestroyWindow(window_.window);
    glfwTerminate();
  }

  friend void application_window_resize_callback(GLFWwindow*, int, int);

  Window window_;
  WindowConfig window_config_;

  std::vector<VkClearValue> clear_values_;

  VkInstance vulkan_instance_;

  // creation only needs the vulkan instance, a.k.a. backbuffer
  VkSurfaceKHR surface_;

  VkPhysicalDevice physical_device_;
  SwapChainProperties device_swapchain_properties_;
  VkSurfaceFormatKHR device_surface_format_;
  VkPresentModeKHR device_surface_presentation_mode_;

  VkDevice logical_device_;

  uint32_t graphics_queue_family_index_;
  uint32_t surface_presentation_queue_family_index_;

  std::vector<uint32_t> unique_queue_families_indexes_;

  uint32_t graphics_command_queue_index_;
  uint32_t surface_presentation_command_queue_index_;

  VkSwapchainKHR window_swapchain_;
  bool swapchain_dirty_;

  VkShaderModule vert_shader_module_;
  VkShaderModule frag_shader_module_;

  VkRenderPass render_pass_;

  VkPipeline graphics_pipeline_;
  VkPipelineLayout pipeline_layout_;

  std::vector<VkFramebuffer> swapchain_framebuffers_;

  VkCommandPool graphics_command_pool_;

  // automatically cleaned on destruction of the logical device
  VkQueue graphics_command_queue_;
  VkQueue surface_presentation_command_queue_;

  std::vector<VkCommandBuffer> graphics_command_buffers_;

  // one for each frame in flight
  std::vector<VkSemaphore> image_available_semaphores_;
  std::vector<VkSemaphore> rendering_finished_semaphores_;

  std::vector<VkFence> in_flight_fences_;
  // i.e. maximum number of frames to be processed per loop
  uint32_t max_frames_in_flight_;

  std::vector<VkImageView> swapchain_image_views_;

  // only used in debug mode
  VkDebugUtilsMessengerEXT debug_messenger_;
  VkDebugUtilsMessengerCreateInfoEXT default_debug_messenger_create_info_;
  static constexpr char const* required_validation_layers_[] = {
      "VK_LAYER_KHRONOS_validation"};
};

static void application_window_resize_callback(GLFWwindow* window, int, int) {
  auto app = static_cast<Application*>(glfwGetWindowUserPointer(window));
  app->swapchain_dirty_ = true;
}

int main() {
  WindowConfig window_config{};
  window_config.desired_width = 1920;
  window_config.desired_height = 1080;
  window_config.resizable = true;
  Application app{window_config};
  app.run();
}
