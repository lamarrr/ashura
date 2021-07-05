

#include "vlk/ui/widgets/row.h"

#include "gtest/gtest.h"
#include "mock_widgets.h"
#include "vlk/ui/palettes/ios.h"
#include "vlk/ui/palettes/material.h"
#include "vlk/ui/pipeline.h"
#include "vlk/ui/render_context.h"
#include "vlk/ui/tile_cache.h"
#include "vlk/ui/vulkan.h"
#include "vlk/ui/widgets/box.h"
#include "vlk/ui/widgets/image.h"
#include "vlk/ui/widgets/text.h"
#include "vlk/ui/window.h"

using namespace vlk::ui;
using namespace vlk;
// RowProps constrain

// what to work on next?
// on image loading the user needs to use a default fallback image or a provided
// one. transition?
//
//
// Add imgui and glfw for testing
//

struct App {
  WindowApi api;

  static constexpr char const* required_validation_layers[] = {
      "VK_LAYER_KHRONOS_validation"};
  static constexpr char const* required_device_extensions[] = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME};

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

  void start() {
    WindowCfg cfg{};
    cfg.maximized = false;

    Window window = Window::create(api, cfg).expect("Unable to create window");

    {
     
    }

   

    auto instance = vk::Instance::create("TestApp", VK_MAKE_VERSION(0, 0, 1),
                                         "Valkyrie", VK_MAKE_VERSION(1, 0, 0),
                                         required_instance_extensions,
                                         required_validation_layers);

    window.attach_surface(api, instance);

    auto phys_devices = vk::PhysDevice::get_all(instance);

    VkPhysicalDeviceType const device_preference[] = {
        VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU,

        VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,
        VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU, VK_PHYSICAL_DEVICE_TYPE_CPU};

    VLK_LOG("Available Physical Devices:");

    for (vk::PhysDevice const& device : phys_devices) {
      VLK_LOG("\t{}", device.format());
    }

    vk::PhysDevice phys_device =
        select_device(phys_devices, device_preference, window.handle->surface)
            .expect("Unable to find any suitable rendering device");

    VLK_LOG("Selected Physical Device: {}", phys_device.format());

    auto const& features = phys_device.info.features;

    // enable sampler anisotropy if available
    VkPhysicalDeviceFeatures required_features{};

    required_features.samplerAnisotropy = features.samplerAnisotropy;

    // we need multiple command queues, one for data transfer and one for
    // rendering
    float const priorities[] = {// priority for command queue used for
                                // presentation, rendering, data transfer
                                1.0f};

    vk::CommandQueueFamily graphic_command_queue_family =
        vk::CommandQueueFamily::get_graphics(phys_device).unwrap();

    // we can accept queue family struct here instead and thus not have to
    // perform extra manual checks
    // the user shouldn't have to touch handles
    VkDeviceQueueCreateInfo const command_queue_create_infos[] = {
        vk::make_command_queue_create_info(
            graphic_command_queue_family.info.index, priorities)};

    vk::Device device = vk::Device::create(
        phys_device, command_queue_create_infos, required_device_extensions,
        required_validation_layers, required_features);

    vk::CommandQueue graphics_command_queue =
        vk::CommandQueue::get(device, graphic_command_queue_family, 0)
            .expect("Failed to create graphics command queue");

    {
      // how do we send pixels over from SKia to Vulkan window?
      GrVkBackendContext vk_context{};

      GrVkExtensions extensions_cache{};
      vk_context.fVkExtensions = &extensions_cache;
      vk_context.fInstance = instance.handle->instance;
      vk_context.fPhysicalDevice = phys_device.info.phys_device;
      vk_context.fDevice = device.handle->device;
      vk_context.fQueue = graphics_command_queue.info.queue;
      vk_context.fGraphicsQueueIndex = graphics_command_queue.info.index;
      vk_context.fMaxAPIVersion = VK_API_VERSION_1_1;
      vk_context.fDeviceFeatures = &features;
      // vk_context.fMemoryAllocator
      vk_context.fGetProc = [](char const* proc_name, VkInstance instance,
                               VkDevice device) {
        VLK_ENSURE(instance == nullptr || device == nullptr);
        VLK_ENSURE(!(instance != nullptr && device != nullptr));
        if (device != nullptr) {
          return vkGetDeviceProcAddr(device, proc_name);
        } else {
          return vkGetInstanceProcAddr(instance, proc_name);
        }
      };

      auto direct_context = GrDirectContext::MakeVulkan(vk_context);
      VLK_ENSURE(direct_context != nullptr,
                 "Unable to create Skia Direct Vulkan Context");

      auto allocator = vk::Allocator::create(device);

      auto image = vk::Image::create(allocator, graphic_command_queue_family,
                                     VK_FORMAT_R8G8B8A8_UINT, Extent{250, 250})
                       .unwrap();

      bool quit = false;

      auto frame_budget = std::chrono::milliseconds(16);

      while (!quit) {
        // we need to get frame budget and use diff between it and the used time
        // window.publish_events();  // defer the events into the widget system
        // via the pawn process widget invalidation and events

        auto begin = std::chrono::steady_clock::now();

        while (!window.handle->tick(graphics_command_queue, direct_context)) {
        }

        auto render_end = std::chrono::steady_clock::now();
        auto total_used = std::chrono::duration_cast<std::chrono::milliseconds>(
            render_end - begin);

        Ticks no_event_ticks{0};
        static constexpr auto sleep_interval = std::chrono::milliseconds(1);

        while (frame_budget > total_used) {
          bool got_event = false;

          window.handle->api.poll_event().match(
              [&](SDL_Event event) {
                if (event.type == SDL_QUIT) quit = true;

                if (event.type == SDL_WINDOWEVENT) {
                  if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    window.handle->surface_extent_dirty = true;
                    window.handle->extent_dirty = true;
                  }
                }

                if (event.type == SDL_MOUSEBUTTONDOWN) {
                  if (event.button.clicks == 2) {
                    VLK_LOG("Double click");
                  }
                }

                got_event = true;
              },
              []() {});

          if (got_event) {
            no_event_ticks.reset();
          } else {
            no_event_ticks++;
          }

          if (no_event_ticks >= Ticks{64}) {
            std::this_thread::sleep_for(sleep_interval);
          }

          total_used = std::chrono::duration_cast<std::chrono::milliseconds>(
              std::chrono::steady_clock::now() - begin);
        }
      }
    }
  }
};

TEST(RowTest, BasicTest) {
  RenderContext context;

  App app;
  app.start();

  constexpr Color color_list[] = {ios::DarkPurple, ios::DarkRed,
                                  ios::DarkIndigo, ios::DarkMint,
                                  ios::DarkTeal};

  // TODO(lamarrr): why isn't this wrapping to the next line? it actually is
  // wrapping but the height allotted is wrong? or do we need to scroll the root
  // view on viewport scroll?

  MockView view{{new Row{
      [&](size_t i) -> Widget* {
        if (i >= 8) return nullptr;

        if (i == 0)
          return new Text{
              {InlineText{"Apparently we had reached a great height in the "
                          "atmosphere, for "
                          "the sky was a dead black, and the stars had ceased "
                          "to twinkle. "
                          "By the same illusion which lifts the horizon of the "
                          "sea to the "
                          "level of the spectator on a hillside, the sable "
                          "cloud beneath "
                          "was dished out, and the car seemed to float in the "
                          "middle of an "
                          "immense dark sphere, whose upper half was strewn "
                          "with silver. "},
               InlineText{"Looking down into the dark gulf below, I could "
                          "see a ruddy "
                          "light streaming through a rift in the clouds.",
                          TextProps{}.color(ios::LightRed)}},
              ParagraphProps{}
                  .font_size(25.0f)
                  .color(ios::DarkGray6)
                  .font(SystemFont{"SF Pro"})};

        if (i == 1) {
          return new Image{ImageProps{
              FileImageSource{"/home/lamar/Pictures/E0U2xTYVcAE1-gl.jpeg"}}
                               .extent(700, 700)
                               .aspect_ratio(3, 1)
                               .border_radius(BorderRadius::all(50))};
        }

        if (i == 2) {
          return new Image{
              ImageProps{FileImageSource{"/home/lamar/Pictures/crow.PNG"}}
                  .extent(500, 500)
                  .aspect_ratio(3, 2)
                  .border_radius(BorderRadius::all(50))};
        }

        if (i == 3) {
          return new Image{
              ImageProps{FileImageSource{"/home/lamar/Pictures/IMG_0079.JPG"}}
                  .extent(500, 500)
                  .aspect_ratio(2, 1)
                  .border_radius(BorderRadius::all(20))};
        }

        if (i == 4) {
          return new Image{ImageProps{
              MemoryImageSource{ImageInfo{Extent{2, 2}, ImageFormat::RGB},
                                {255, 0, 0, 0, 255, 0, 0, 0, 255, 255, 0, 0}}}
                               .extent(500, 500)
                               .aspect_ratio(2, 1)
                               .border_radius(BorderRadius::all(20))};
        }

        return new Box(
            new Box(new Text{"Aa Type of A Box (" + std::to_string(i) + ")",
                             TextProps{}
                                 .font_size(25.0f)
                                 .color(colors::White)
                                 .font(SystemFont{"SF Pro"})},
                    BoxProps{}
                        .padding(Padding::all(15))
                        .border_radius(BorderRadius::all(20))
                        .color(color_list[i % std::size(color_list)])),
            BoxProps{}
                .image(FileImageSource{
                    "/home/lamar/Pictures/E0U20cZUYAEaJqL.jpeg"})
                .padding(Padding::all(50))
                .border(Border::all(ios::DarkPurple, 20))
                .border_radius(BorderRadius::all(50)));
      },
      RowProps{}.main_align(MainAlign::SpaceBetween)}}};

  Extent screen_extent{2000, 1000};

  Pipeline pipeline{view};

  pipeline.viewport.resize(screen_extent);

  for (size_t i = 0; i < 1'00; i++) {
    constexpr float mul = 1 / 50.0f;
    pipeline.tick(std::chrono::nanoseconds(0));
    pipeline.tile_cache.scroll_backing_store(IOffset{0, mul * i * 0});
    // pipeline.tile_cache.backing_store.save_pixels_to_file("./ui_output_row_"
    // +
    //                                                      std::to_string(i));
    // VLK_LOG("written tick: {}", i);
  }
}s


  uint32_t uneventful_polls = 0;

  while (total_used < frame_budget) {
    if (window_api.poll_events()) {
      uneventful_polls = 0;
    } else {
      uneventful_polls++;
    }

    backoff_spin_sleep(uneventful_polls, std::chrono::milliseconds(1));
    total_used = std::chrono::steady_clock::now() - begin;
  }