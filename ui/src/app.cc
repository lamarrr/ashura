#include "vlk/ui/app.h"

#include "include/core/SkGraphics.h"
#include "include/gpu/vk/GrVkBackendContext.h"
#include "include/gpu/vk/GrVkExtensions.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
#include "vlk/ui/pipeline.h"
#include "vlk/ui/vk_render_context.h"
#include "vlk/ui/vulkan.h"
#include "vlk/ui/window.h"
#include "vlk/ui/window_handle.h"

namespace vlk {
namespace ui {

namespace impl {
// take a quick look at UE log file content and structure
// Valkyrie.App
static std::unique_ptr<spdlog::logger> make_single_threaded_logger(
    std::string name, std::string file_path) {
  std::vector<spdlog::sink_ptr> sinks;
  sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_st>(
      std::move(file_path)));
  sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_st>());
  return std::make_unique<spdlog::logger>(std::move(name), sinks.begin(),
                                          sinks.end());
}
}  // namespace impl

static stx::Option<vk::PhysDevice> select_device(
    stx::Span<vk::PhysDevice const> const physical_devices,
    stx::Span<VkPhysicalDeviceType const> preferred_device_types,
    WindowSurface const& target_surface) {
  for (auto type : preferred_device_types) {
    auto selected_device_it = std::find_if(
        physical_devices.begin(), physical_devices.end(),
        [&](vk::PhysDevice const& dev) -> bool {
          return dev.info.properties.deviceType == type &&
                 // can use shaders (fragment and vertex)
                 dev.has_geometry_shader() &&
                 // has graphics command queue for rendering commands
                 dev.has_graphics_command_queue_family() &&
                 // has data transfer command queue for uploading textures
                 // or data
                 dev.has_transfer_command_queue_family() &&
                 // can be used for presenting to a specific surface
                 any_true(vk::get_surface_presentation_command_queue_support(
                     dev.info.phys_device, dev.info.family_properties,
                     target_surface.handle->surface));
        });
    if (selected_device_it != physical_devices.end()) {
      return stx::Some(vk::PhysDevice{*selected_device_it});
    }
  }

  return stx::None;
}

static constexpr char const* const REQUIRED_DEVICE_EXTENSIONS[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME};

static constexpr char const* REQUIRED_VALIDATION_LAYERS[] = {
    "VK_LAYER_KHRONOS_validation"};

App::~App() = default;

void App::init() {
  logger = impl::make_single_threaded_logger("App", "log.txt");

  logger->info("Initializing Window API");

  window_api = WindowApi::init();

  logger->info("Initialized Window API");
  logger->info("Creating root window");

  window = Window::create(window_api, cfg.window_cfg);

  logger->info("Created root window");

  std::vector window_required_instance_extensions =
      window.handle->get_required_instance_extensions();

  // TODO(lamarrr): check for validation layers requirement
  vk::Instance vk_instance = vk::Instance::create(
      cfg.name, VK_MAKE_VERSION(0, 0, 1), engine_cfg.name,
      VK_MAKE_VERSION(engine_cfg.version.major, engine_cfg.version.minor,
                      engine_cfg.version.patch),
      window_required_instance_extensions, REQUIRED_VALIDATION_LAYERS);

  window.attach_surface(vk_instance);

  std::vector phys_devices = vk::PhysDevice::get_all(vk_instance);

  VkPhysicalDeviceType const device_preference[] = {
      VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU,
      VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU,
      VK_PHYSICAL_DEVICE_TYPE_CPU};

  logger->info("Available Physical Devices:");

  for (vk::PhysDevice const& device : phys_devices) {
    logger->info("\t{}", vk::format(device));
    // TODO(lamarrr): log graphics families on devices and other properties
  }

  vk::PhysDevice phys_device =
      select_device(phys_devices, device_preference, window.handle->surface)
          .expect("Unable to find any suitable rendering device");

  logger->info("Selected Physical Device: {}", vk::format(phys_device));

  // we might need multiple command queues, one for data transfer and one for
  // rendering
  float const queue_priorities[] = {// priority for command queue used for
                                    // presentation, rendering, data transfer
                                    1.0f};

  vk::CommandQueueFamily graphic_command_queue_family =
      vk::CommandQueueFamily::get_graphics(phys_device)
          .expect("Unable to get graphics command queue");

  // we can accept queue family struct here instead and thus not have to
  // perform extra manual checks
  // the user shouldn't have to touch handles
  VkDeviceQueueCreateInfo const command_queue_create_infos[] = {
      vk::make_command_queue_create_info(
          graphic_command_queue_family.info.index, queue_priorities)};

  VkPhysicalDeviceFeatures required_features{};

  vk::Device device = vk::Device::create(
      phys_device, command_queue_create_infos, REQUIRED_DEVICE_EXTENSIONS,
      REQUIRED_VALIDATION_LAYERS, required_features);

  vk::CommandQueue vk_graphics_command_queue = vk::CommandQueue{
      vk::CommandQueue::get(device, graphic_command_queue_family, 0)
          .expect("Failed to create graphics command queue")};

  GrVkBackendContext sk_vk_context{};

  GrVkExtensions sk_extensions_cache{};
  sk_vk_context.fVkExtensions = &sk_extensions_cache;
  sk_vk_context.fInstance = vk_instance.handle->instance;
  sk_vk_context.fPhysicalDevice = phys_device.info.phys_device;
  sk_vk_context.fDevice = device.handle->device;
  sk_vk_context.fQueue = vk_graphics_command_queue.info.queue;
  sk_vk_context.fGraphicsQueueIndex = vk_graphics_command_queue.info.index;
  sk_vk_context.fMaxAPIVersion = VK_API_VERSION_1_1;
  sk_vk_context.fDeviceFeatures = &phys_device.info.features;
  // TODO(lamarrr): vk_context.fMemoryAllocator, see: GrVkAMDMemoryAllocator
  sk_vk_context.fGetProc = [](char const* proc_name, VkInstance instance,
                              VkDevice device) {
    VLK_ENSURE(instance == nullptr || device == nullptr);
    VLK_ENSURE(!(instance != nullptr && device != nullptr));
    if (device != nullptr) {
      // get process address
      return vkGetDeviceProcAddr(device, proc_name);
    } else {
      return vkGetInstanceProcAddr(instance, proc_name);
    }
  };

  // TODO(lamarrr): we need to take care of lifetime interactions here:
  // - direct context, render context and the likes, so we can also pass to
  // swapchain and the likes and not have to enforce ordering of struct members
  // -
  // -

  sk_sp<GrDirectContext> sk_direct_context =
      GrDirectContext::MakeVulkan(sk_vk_context);

  VLK_ENSURE(sk_direct_context != nullptr,
             "Unable to create Skia GPU Vulkan Context");
  // NOTE: we use a swapchain-independent color space, color type and color
  // info. we don't want to have to reconstruct the pipeline if for instance the
  // monitor is changed
  RenderContext render_context{
      stx::Some(std::move(sk_direct_context)),
  };

  vk_render_context = std::shared_ptr<VkRenderContext>{new VkRenderContext{
      std::move(vk_graphics_command_queue), std::move(render_context)}};

  window.handle->recreate_swapchain(vk_render_context);

  // TODO(lamarrr): initial extent?, RenderContext
  pipeline = std::unique_ptr<Pipeline>{
      new Pipeline{*root_widget, vk_render_context->render_context}};
}

// TODO(lamarrr): handle should_quit
void App::tick() {
  auto frame_budget = frequency_to_period(present_refresh_rate_hz);
  auto begin = std::chrono::steady_clock::now();
  auto total_used = std::chrono::steady_clock::duration(0);

  // TODO(lamarrr): add actual tick
  BackingStoreDiff backing_store_diff = BackingStoreDiff::None;

  {
    VLK_TRACE(trace_context, "Swapchain", "Pipeline Tick");
    backing_store_diff = pipeline->tick({});
  }

  // only try to present if the pipeline has new changes or window was
  // resized
  if (backing_store_diff != BackingStoreDiff::None || window_extent_changed) {
    // TODO(lamarrr): make presentation happen after recreation, for the first
    // iteration. and remove the created swapchain in the init method

    // only try to recreate swapchain if the present swapchain can't be used for
    // presentation
    if (window_extent_changed) {
      VLK_TRACE(trace_context, "Swapchain", "Recreation");
      window.handle->recreate_swapchain(vk_render_context);
    }

    // TODO(lamarrr): we don't need another backing store on the pipeline side
    WindowSwapchainDiff swapchain_diff = window.handle->present_backing_store(
        pipeline->tile_cache.backing_store_cache.get_surface_ref());

    while (swapchain_diff != WindowSwapchainDiff::None) {
      {
        VLK_TRACE(trace_context, "Swapchain", "Recreation");
        window.handle->recreate_swapchain(vk_render_context);
      }

      {
        VLK_TRACE(trace_context, "Swapchain", "Presentation");
        swapchain_diff = window.handle->present_backing_store(
            pipeline->tile_cache.backing_store_cache.get_surface_ref());
      }
    }

    // we need to update viewport in case it changed
    pipeline->viewport.resize(
        window.handle->extent,
        pipeline->viewport.get_unresolved_widgets_allocation());

    // resize event could have been caused by a display or DPR-setting change.
    // we thus need to update the DPR accordingly
    pipeline->tile_cache.update_dpr(
        dpr_from_extents(window.handle->extent, window.handle->surface_extent));

    SDL_DisplayMode display_mode{};
    VLK_SDL_ENSURE(
        SDL_GetWindowDisplayMode(window.handle->window, &display_mode) == 0,
        "Unable to get window display mode");

    // TODO(lamarrr): log refresh rate
    present_refresh_rate_hz = static_cast<uint32_t>(display_mode.refresh_rate);

    window_extent_changed = false;

    auto render_end = std::chrono::steady_clock::now();

    total_used = render_end - begin;
    // we can save power on minimized and not have to update the buffer
  }

  // poll events to make the window not be marked as unresponsive.
  // we also poll events from SDL's event queue until there are none left.
  //
  // any missed event should be rolled over to the next tick()
  do {
  } while (window_api.poll_events());

  total_used = std::chrono::steady_clock::now() - begin;

  if (total_used < frame_budget) {
    std::this_thread::sleep_for(frame_budget - total_used);
  }

  pipeline->dispatch_events(window.handle->event_queue.mouse_button_events,
                            window.handle->event_queue.window_events);

  window_extent_changed = any_eq(window.handle->event_queue.window_events,
                                 WindowEvent::SizeChanged);

  if (any_eq(window.handle->event_queue.window_events, WindowEvent::Close)) {
    std::exit(0);
  }

  window.handle->event_queue.clear();
}

}  // namespace ui
}  // namespace vlk
