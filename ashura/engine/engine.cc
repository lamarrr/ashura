/// SPDX-License-Identifier: MIT
#include "ashura/engine/engine.h"
#include "ashura/std/async.h"
#include "ashura/std/fs.h"
#include "simdjson.h"

namespace ash
{

ASH_C_LINKAGE ASH_DLL_EXPORT Engine * engine = nullptr;

EngineCfg EngineCfg::parse(AllocatorImpl allocator, Span<u8 const> json)
{
  EngineCfg                    out;
  simdjson::ondemand::parser   parser;
  simdjson::padded_string      str{json.as_char().data(), json.size()};
  simdjson::ondemand::document doc = parser.iterate(str);

  auto cfg = doc.get_object().value();

  std::string_view version = cfg["version"].get_string().value();
  CHECK(version == "0.0.1");

  out.gpu.validation = cfg["gpu.validation"].get_bool().value();
  out.gpu.vsync      = cfg["gpu.vsync"].get_bool().value();

  auto gpu_prefs = cfg["gpu.preferences"].get_array().value();
  CHECK(gpu_prefs.count_elements().value() <= 5);

  for (auto pref : gpu_prefs)
  {
    std::string_view s = pref.get_string().value();
    if (s == "dgpu")
    {
      out.gpu.preferences.push(gpu::DeviceType::DiscreteGpu).unwrap();
    }
    else if (s == "vgpu")
    {
      out.gpu.preferences.push(gpu::DeviceType::VirtualGpu).unwrap();
    }
    else if (s == "igpu")
    {
      out.gpu.preferences.push(gpu::DeviceType::IntegratedGpu).unwrap();
    }
    else if (s == "other")
    {
      out.gpu.preferences.push(gpu::DeviceType::Other).unwrap();
    }
    else if (s == "cpu")
    {
      out.gpu.preferences.push(gpu::DeviceType::Cpu).unwrap();
    }
    else
    {
      CHECK(false);
    }
  }

  out.gpu.hdr = cfg["gpu.hdr"].get_bool().value();
  out.gpu.buffering =
    (u32) clamp(cfg["gpu.buffering"].get_int64().value(), (i64) 0, (i64) 4);

  switch (cfg["gpu.msaa.level"].get_int64().value())
  {
    case 1:
      out.gpu.msaa_level = gpu::SampleCount::C1;
      break;
    case 2:
      out.gpu.msaa_level = gpu::SampleCount::C2;
      break;
    case 4:
      out.gpu.msaa_level = gpu::SampleCount::C4;
      break;
    case 8:
      out.gpu.msaa_level = gpu::SampleCount::C8;
      break;
    case 16:
      out.gpu.msaa_level = gpu::SampleCount::C16;
      break;
    default:
      out.gpu.msaa_level = gpu::SampleCount::C4;
      break;
  }

  out.window.resizable   = cfg["window.resizable"].get_bool().value();
  out.window.maximized   = cfg["window.maximized"].get_bool().value();
  out.window.full_screen = cfg["window.full_screen"].get_bool().value();
  out.window.borderless  = cfg["window.borderless"].get_bool().value();
  out.window.width       = (u32) clamp(cfg["window.width"].get_int64().value(),
                                       (i64) 0, (i64) U32_MAX);
  out.window.height      = (u32) clamp(cfg["window.height"].get_int64().value(),
                                       (i64) 0, (i64) U32_MAX);

  auto shaders = cfg["shaders"].get_object().value();
  for (auto entry : shaders)
  {
    std::string_view id   = entry.escaped_key().value();
    std::string_view path = entry.value().get_string().value();
    out.shaders
      .insert(vec(span(id), allocator).unwrap(),
              vec(span(path), allocator).unwrap())
      .unwrap();
  }

  auto fonts = cfg["fonts"].get_object().value();
  for (auto entry : fonts)
  {
    std::string_view id   = entry.escaped_key().value();
    std::string_view path = entry.value().get_string().value();
    out.fonts
      .insert(vec(span(id), allocator).unwrap(),
              vec(span(path), allocator).unwrap())
      .unwrap();
  }

  std::string_view default_font = cfg["fonts.default"].get_string().value();
  out.default_font              = vec<char>(default_font, allocator).unwrap();

  // check that it is a valid entry
  fonts[default_font].get_string().value();

  auto images = cfg["images"].get_object().value();
  for (auto entry : images)
  {
    std::string_view id   = entry.escaped_key().value();
    std::string_view path = entry.value().get_string().value();
    out.images
      .insert(vec(span(id), allocator).unwrap(),
              vec(span(path), allocator).unwrap())
      .unwrap();
  }

  return out;
}

void Engine::init(AllocatorImpl allocator, void * app,
                  Span<char const> config_path, Span<char const> assets_dir)
{
  if (logger == nullptr)
  {
    abort();
  }
  CHECK(scheduler != nullptr);
  CHECK(engine == nullptr);

  logger->trace("Initializing Window System");

  WindowSystem::init();

  logger->trace("Loading Engine config file");

  Vec<u8> json{allocator};

  read_file(config_path, json).unwrap();

  EngineCfg cfg = EngineCfg::parse(allocator, json);

  logger->trace("Initializing Engine");

  Dyn<gpu::Instance *> instance =
    gpu::create_vulkan_instance(allocator, cfg.gpu.validation).unwrap();

  gpu::Device * device =
    instance->create_device(allocator, cfg.gpu.preferences, 2).unwrap();

  GpuSystem gpu = GpuSystem::create(allocator, device, cfg.gpu.hdr,
                                    cfg.gpu.buffering, cfg.gpu.msaa_level,
                                    Vec2U{cfg.window.width, cfg.window.height});

  logger->trace("Initializing Renderer");

  Renderer renderer = Renderer::create(allocator);

  Canvas canvas{allocator};

  ViewSystem view_system{allocator};

  logger->trace("Creating Root Window");

  Window window =
    window_system->create_window(*instance, "Ashura"_str).unwrap();

  if (cfg.window.maximized)
  {
    window_system->maximize(window);
  }
  else
  {
    window_system->set_size(window, Vec2U{cfg.window.width, cfg.window.height});
  }

  if (cfg.window.full_screen)
  {
    window_system->make_fullscreen(window);
  }
  else
  {
    window_system->make_windowed(window);
  }

  if (cfg.window.borderless)
  {
    window_system->make_borderless(window);
  }
  else
  {
    window_system->make_bordered(window);
  }

  if (cfg.window.resizable)
  {
    window_system->make_resizable(window);
  }
  else
  {
    window_system->make_unresizable(window);
  }

  gpu::Surface surface = window_system->get_surface(window);

  logger->trace("Initializing GPU Context");

  alignas(Engine) static u8 storage[sizeof(Engine)] = {};

  engine = new (storage)
    Engine{allocator,
           app,
           std::move(instance),
           device,
           window,
           window_system->get_clipboard(),
           surface,
           cfg.gpu.vsync ? gpu::PresentMode::Fifo : gpu::PresentMode::Immediate,
           std::move(gpu),
           std::move(renderer),
           std::move(canvas),
           std::move(view_system)};

  window_system->listen(fn(
    engine, +[](Engine * engine, SystemEvent const & event) {
      event.match(
        [&](SystemTheme theme) {
          InputState & f = engine->input_buffer;
          f.theme        = theme;
        },
        [](SystemEventType) {});
    }));

  window_system->listen(
    window, fn(
              engine, +[](Engine * engine, WindowEvent const & event) {
                InputState & f = engine->input_buffer;

                event.match(
                  [&](KeyEvent e) {
                    switch (e.action)
                    {
                      case KeyAction::Press:
                      {
                        f.key.any_down = true;
                        set_bit(f.key.downs, (u32) e.key_code);
                        set_bit(f.key.scan_downs, (u32) e.scan_code);
                        f.key.modifiers |= e.modifiers;
                      }
                      break;
                      case KeyAction::Release:
                      {
                        f.key.any_up = true;
                        set_bit(f.key.ups, (u32) e.key_code);
                        set_bit(f.key.scan_ups, (u32) e.scan_code);
                        f.key.modifiers |= e.modifiers;
                      }
                      break;
                      default:
                        break;
                    }
                  },
                  [&](MouseMotionEvent e) {
                    f.mouse.moved    = true;
                    f.mouse.position = e.position;
                    f.mouse.translation += e.translation;
                  },
                  [&](MouseClickEvent e) {
                    f.mouse.num_clicks[(u32) e.button] = e.clicks;
                    f.mouse.position                   = e.position;
                    switch (e.action)
                    {
                      case KeyAction::Press:
                        set_bit(f.mouse.downs, (u32) e.button);
                        f.mouse.any_down = true;
                        break;
                      case KeyAction::Release:
                        set_bit(f.mouse.ups, (u32) e.button);
                        f.mouse.any_up = true;
                        break;
                      default:
                        break;
                    }
                  },
                  [&](MouseWheelEvent e) {
                    f.mouse.wheel_scrolled = true;
                    f.mouse.position       = e.position;
                    f.mouse.translation += e.translation;
                  },
                  [&](TextInputEvent e) {
                    f.text_input = true;
                    f.text.extend(e.text).unwrap();
                  },
                  [&](WindowEventType e) {
                    switch (e)
                    {
                      case WindowEventType::Shown:
                      case WindowEventType::Hidden:
                      case WindowEventType::Exposed:
                      case WindowEventType::Moved:
                        break;
                      case WindowEventType::Resized:
                        f.resized = true;
                        break;
                      case WindowEventType::SurfaceResized:
                        f.surface_resized = true;
                        break;
                      case WindowEventType::Minimized:
                      case WindowEventType::Maximized:
                      case WindowEventType::Restored:
                        break;
                      case WindowEventType::MouseEnter:
                        f.mouse.in      = true;
                        f.mouse_focused = true;
                        break;
                      case WindowEventType::MouseLeave:
                        f.mouse.out     = true;
                        f.mouse_focused = false;
                        break;
                      case WindowEventType::KeyboardFocusIn:
                        f.key.in      = true;
                        f.key_focused = true;
                        break;
                      case WindowEventType::KeyboardFocusOut:
                        f.key.out     = true;
                        f.key_focused = false;
                        break;
                      case WindowEventType::CloseRequested:
                        f.close_requested = true;
                        break;
                      case WindowEventType::Occluded:
                      case WindowEventType::EnterFullScreen:
                      case WindowEventType::LeaveFullScreen:
                      case WindowEventType::Destroyed:
                        break;
                      default:
                        break;
                    }
                  },
                  [&](DropEvent const & e) {
                    e.match(
                      [&](DropEventType e) {
                        switch (e)
                        {
                          case DropEventType::DropBegin:
                            break;
                          case DropEventType::DropComplete:
                            f.dropped = true;
                            break;
                          default:
                            break;
                        }
                      },
                      [&](DropPositionEvent e) {
                        f.drop_hovering  = true;
                        f.mouse.position = e.pos;
                      },
                      [&](DropFileEvent e) {
                        f.drop_data.clear();
                        f.drop_data.extend(e.path.as_u8()).unwrap();
                        f.drop_type = DropType::FilePath;
                      },
                      [&](DropTextEvent e) {
                        f.drop_data.clear();
                        f.drop_data.extend(e.text.as_u8()).unwrap();
                        f.drop_type = DropType::Bytes;
                      });
                  });
              }));

  engine->device->begin_frame(nullptr).unwrap();


  for (auto const & [id, path] : cfg.shaders)
  {
    // [ ]
  }

  for (auto const & [id, path] : cfg.fonts)
  {
    // [ ]
  }

  while (!semaphores)
  {
    scheduler->execute_main_thread_loop(1ms, 2ms);
  }

  engine->default_font_name = vec<char>(cfg.default_font, allocator).unwrap();
  engine->default_font = engine->assets.fonts[engine->default_font_name].get();

  engine->renderer.acquire(engine->gpu, engine->assets);

  engine->device->submit_frame(nullptr).unwrap();

  logger->trace("Engine Initialized");
}

void Engine::uninit()
{
  CHECK(engine != nullptr);
  logger->trace("Uninitializing Engine");
  engine->~Engine();
  engine = nullptr;
  logger->trace("Engine Uninitialized");
}

Engine::~Engine()
{
  device->wait_idle().unwrap();
  canvas.reset();

  // [ ] shutdown systems?

  renderer.release(gpu, assets);

  gpu.uninit();

  device->uninit(swapchain);
  window_system->uninit_window(window);
  logger->trace("Uninitializing Window System");
  WindowSystem::uninit();
  instance->uninit(device);
}

void Engine::recreate_swapchain_()
{
  gpu::SurfaceCapabilities capabilities =
    device->get_surface_capabilities(surface).unwrap();
  CHECK(has_bits(capabilities.image_usage, gpu::ImageUsage::TransferDst |
                                             gpu::ImageUsage::ColorAttachment));

  Vec<gpu::SurfaceFormat> formats{allocator};
  device->get_surface_formats(surface, formats).unwrap();

  Vec<gpu::PresentMode> present_modes{allocator};
  device->get_surface_present_modes(surface, present_modes).unwrap();

  Vec2U surface_extent = window_system->get_surface_size(window);
  surface_extent.x     = max(surface_extent.x, 1U);
  surface_extent.y     = max(surface_extent.y, 1U);

  gpu::ColorSpace preferred_color_spaces[] = {
    gpu::ColorSpace::DCI_P3_NONLINEAR,
    gpu::ColorSpace::DISPLAY_P3_NONLINEAR,
    gpu::ColorSpace::DISPLAY_P3_LINEAR,
    gpu::ColorSpace::ADOBERGB_LINEAR,
    gpu::ColorSpace::ADOBERGB_NONLINEAR,
    gpu::ColorSpace::SRGB_NONLINEAR,
    gpu::ColorSpace::EXTENDED_SRGB_LINEAR,
    gpu::ColorSpace::EXTENDED_SRGB_NONLINEAR,
    gpu::ColorSpace::DOLBYVISION,
    gpu::ColorSpace::HDR10_ST2084,
    gpu::ColorSpace::HDR10_HLG,
    gpu::ColorSpace::BT709_LINEAR,
    gpu::ColorSpace::BT709_NONLINEAR,
    gpu::ColorSpace::BT2020_LINEAR,
    gpu::ColorSpace::PASS_THROUGH};

  gpu::PresentMode preferred_present_modes[] = {
    present_mode_preference, gpu::PresentMode::Immediate,
    gpu::PresentMode::Mailbox, gpu::PresentMode::Fifo,
    gpu::PresentMode::FifoRelaxed};

  bool               found_format = false;
  gpu::SurfaceFormat format;

  for (gpu::ColorSpace cp : preferred_color_spaces)
  {
    Span sel = find_if(formats.view(), [&](gpu::SurfaceFormat a) {
      return a.color_space == cp;
    });
    if (!sel.is_empty())
    {
      found_format = true;
      format       = sel[0];
      break;
    }
  }

  CHECK(found_format);

  gpu::PresentMode present_mode       = gpu::PresentMode::Immediate;
  bool             found_present_mode = false;

  for (gpu::PresentMode pm : preferred_present_modes)
  {
    if (!find(present_modes.view(), pm).is_empty())
    {
      found_present_mode = true;
      present_mode       = pm;
      break;
    }
  }

  CHECK(found_present_mode);

  gpu::CompositeAlpha alpha             = gpu::CompositeAlpha::None;
  gpu::CompositeAlpha alpha_spec        = gpu::CompositeAlpha::Opaque;
  gpu::CompositeAlpha preferred_alpha[] = {alpha_spec,
                                           gpu::CompositeAlpha::Opaque,
                                           gpu::CompositeAlpha::Inherit,
                                           gpu::CompositeAlpha::Inherit,
                                           gpu::CompositeAlpha::PreMultiplied,
                                           gpu::CompositeAlpha::PostMultiplied};
  for (gpu::CompositeAlpha a : preferred_alpha)
  {
    if (has_bits(capabilities.composite_alpha, a))
    {
      alpha = a;
      break;
    }
  }

  gpu::SwapchainInfo info{.label  = "Window Swapchain"_str,
                          .format = format,
                          .usage  = gpu::ImageUsage::TransferDst |
                                   gpu::ImageUsage::ColorAttachment,
                          .preferred_buffering = gpu.buffering,
                          .present_mode        = present_mode,
                          .preferred_extent    = surface_extent,
                          .composite_alpha     = alpha};

  if (swapchain == nullptr)
  {
    swapchain = device->create_swapchain(surface, info).unwrap();
  }
  else
  {
    device->invalidate_swapchain(swapchain, info).unwrap();
  }
}

void Engine::run(View & view, Fn<void(InputState const &)> loop)
{
  logger->trace("Starting Engine Run Loop");

  if (swapchain == nullptr)
  {
    recreate_swapchain_();
  }

  time_point timestamp = steady_clock::now();

  while (true)
  {
    time_point const  current_time = steady_clock::now();
    nanoseconds const timedelta    = current_time - timestamp;
    timestamp                      = current_time;

    input_buffer.clear();
    input_buffer.stamp(timestamp, timedelta);

    input_buffer.mouse.position =
      window_system->get_mouse_state(input_buffer.mouse.states);
    window_system->get_keyboard_state(input_buffer.key.states);

    window_system->poll_events();

    // [ ] proper cooperative shutdown is needed
    if (input_buffer.close_requested)
    {
      break;
    }

    if (input_buffer.resized || input_buffer.surface_resized)
    {
      Vec2U const surface_size = window_system->get_surface_size(window);
      sys->gpu.recreate_framebuffers(surface_size);
    }

    sys->gpu.begin_frame(swapchain);

    // [ ] wrong! use window extent?
    // [ ] propert use of window extent and surface size. then store sizing.
    // [ ] this will be needed for high density displays, i,e. apple devices
    input_buffer.viewport_extent = as_vec2(sys->gpu.fb.extent());

    canvas.begin_recording(sys->gpu.fb.viewport().extent, gpu.fb.extent());

    view_system.tick(input_buffer, view, canvas, loop);

    canvas.end_recording();

    renderer.begin_frame(sys->gpu.fb, canvas);
    renderer.render_frame(sys->gpu.fb, canvas);
    renderer.end_frame(sys->gpu.fb, canvas);
    sys->gpu.submit_frame(swapchain);
  }

  logger->trace("Ended Engine Run Loop");
}

}    // namespace ash
