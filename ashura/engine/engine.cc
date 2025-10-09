/// SPDX-License-Identifier: MIT
#include "ashura/engine/engine.h"
#include "ashura/std/async.h"
#include "ashura/std/fs.h"
#include "ashura/std/trace.h"
#include "simdjson.h"

namespace ash
{

Result<EngineCfg> EngineCfg::parse(Allocator allocator, Vec<u8> & json)
{
  EngineCfg out{.shaders{allocator},
                .fonts{allocator},
                .images{allocator},
                .pipeline_cache{allocator}};

  json.reserve(json.size() + simdjson::SIMDJSON_PADDING).unwrap();

  simdjson::ondemand::parser parser;

  auto error = parser.iterate(json.data(), json.size_bytes(), json.capacity());

  if (error.error() != simdjson::SUCCESS)
  {
    return Err{};
  }

  auto & doc = error.value();

  // [ ] check valid schema
  auto cfg = doc.get_object().value();

  std::string_view version = cfg["version"].get_string().value();
  CHECK(version == "0.0.1", "");

  out.gpu.validation = cfg["gpu.validation"].get_bool().value();
  out.gpu.vsync      = cfg["gpu.vsync"].get_bool().value();

  auto gpu_prefs = cfg["gpu.preferences"].get_array().value();
  CHECK(gpu_prefs.count_elements().value() <= 5, "");

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
      CHECK(false, "");
    }
  }

  out.gpu.hdr = cfg["gpu.hdr"].get_bool().value();
  out.gpu.buffering =
    (u32) clamp(cfg["gpu.buffering"].get_int64().value(), (i64) 1, (i64) 4);

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

  if (auto fps = cfg["gpu.max_fps"].get_int64();
      fps.error() == simdjson::error_code::SUCCESS)
  {
    out.gpu.max_fps = fps.value();
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
      .push(vec(allocator, span(id)).unwrap(),
            vec(allocator, span(path)).unwrap())
      .unwrap();
  }

  auto fonts = cfg["fonts"].get_object().value();
  for (auto entry : fonts)
  {
    std::string_view id   = entry.escaped_key().value();
    std::string_view path = entry.value().get_string().value();
    out.fonts
      .push(vec(allocator, span(id)).unwrap(),
            vec(allocator, span(path)).unwrap())
      .unwrap();
  }

  out.font_height =
    clamp((u32) cfg["fonts.height"].get_int64().value(), 16U, 256U);

  auto images = cfg["images"].get_object().value();
  for (auto entry : images)
  {
    std::string_view id   = entry.escaped_key().value();
    std::string_view path = entry.value().get_string().value();
    out.images
      .push(vec(allocator, span(id)).unwrap(),
            vec(allocator, span(path)).unwrap())
      .unwrap();
  }

  std::string_view pipeline_cache_path =
    cfg["cache.pipeline.path"].get_string().value();

  out.pipeline_cache.extend(pipeline_cache_path).unwrap();

  return Ok{std::move(out)};
}

static void window_event_listener(Engine * engine, WindowEvent const & event)
{
  InputState & f = engine->input_state;

  event.match(
    [&](KeyEvent e) {
      switch (e.action)
      {
        case KeyAction::Press:
        {
          f.key.any_down = true;
          f.key.key_downs.set_bit((usize) e.key_code);
          f.key.scan_downs.set_bit((usize) e.scan_code);
          f.key.mod_downs |= e.modifiers;
        }
        break;
        case KeyAction::Release:
        {
          f.key.any_up = true;
          f.key.key_ups.set_bit((usize) e.key_code);
          f.key.scan_ups.set_bit((usize) e.scan_code);
          f.key.mod_ups |= e.modifiers;
        }
        break;
        default:
          break;
      }
    },
    [&](MouseMotionEvent e) {
      f.mouse.moved       = true;
      f.mouse.position    = e.position;
      f.mouse.translation = e.translation;
    },
    [&](MouseClickEvent e) {
      f.mouse.num_clicks[(u32) e.button] = e.clicks;
      f.mouse.position                   = e.position;
      switch (e.action)
      {
        case KeyAction::Press:
          f.mouse.downs |= static_cast<MouseButtons>(1U << (u32) e.button);
          f.mouse.any_down = true;
          break;
        case KeyAction::Release:
          f.mouse.ups |= static_cast<MouseButtons>(1U << (u32) e.button);
          f.mouse.any_up = true;
          break;
        default:
          break;
      }
    },
    [&](MouseWheelEvent e) {
      f.mouse.scrolled          = true;
      f.mouse.position          = e.position;
      f.mouse.wheel_translation = e.translation;
    },
    [&](TextInputEvent e) {
      f.key.input = true;
      f.key.text.extend(e.text).unwrap();
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
          f.window.resized = true;
          break;
        case WindowEventType::SurfaceResized:
          f.window.surface_resized = true;
          break;
        case WindowEventType::Minimized:
        case WindowEventType::Maximized:
        case WindowEventType::Restored:
          break;
        case WindowEventType::MouseEnter:
          f.mouse.in = true;
          break;
        case WindowEventType::MouseLeave:
          f.mouse.out = true;
          break;
        case WindowEventType::KeyboardFocusIn:
          f.key.in = true;
          break;
        case WindowEventType::KeyboardFocusOut:
          f.key.out = true;
          break;
        case WindowEventType::CloseRequested:
          f.window.close_requested = true;
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
              f.drop.event = DropState::Event::Begin;
              break;
            case DropEventType::DropComplete:
              f.drop.event = DropState::Event::End;
              break;
            default:
              break;
          }
        },
        [&](DropPositionEvent e) { f.mouse.position = e.pos; },
        [&](DropFileEvent e) {
          f.drop.data.clear();
          f.drop.data.extend(e.path.as_u8()).unwrap();
          f.drop.event = DropState::Event::FilePath;
        },
        [&](DropTextEvent e) {
          f.drop.data.clear();
          f.drop.data.extend(e.text.as_u8()).unwrap();
          f.drop.event = DropState::Event::Bytes;
        });
    });
}

Dyn<Engine *> Engine::create(Allocator allocator, Str config_path,
                             Str working_dir)
{
  Dyn<Logger *> logger =
    dyn<Logger>(inplace, default_allocator, span<LogSink *>({&stdio_sink}))
      .unwrap();
  hook_logger(logger);
  trace("Initializing Engine, config_path: {} and working dir: {} "_str,
        config_path, working_dir);

  trace("Loading Engine config file"_str);

  Vec<u8> json{allocator};

  read_file(config_path, json).unwrap("Error opening config file"_str);

  EngineCfg cfg = EngineCfg::parse(allocator, json).unwrap();

  trace("Initializing Core Systems"_str);

  FileSystem file_sys{allocator};

  Dyn<gpu::Instance *> instance =
    gpu::create_vulkan_instance(allocator, cfg.gpu.validation).unwrap();

  gpu::Device * device =
    instance->create_device(allocator, cfg.gpu.preferences, cfg.gpu.buffering)
      .unwrap();

  trace("Loading Pipeline cache from disk"_str);

  Vec<u8> pipeline_cache{allocator};
  read_file(cfg.pipeline_cache, pipeline_cache)
    .match([](Void) {},
           [](IoErr err) {
             CHECK(err == IoErr::InvalidFileOrDir, "Io Error Occured");
           });

  GpuSystem gpu_sys = GpuSystem::create(
    allocator, *device, pipeline_cache, cfg.gpu.hdr, cfg.gpu.buffering,
    cfg.gpu.msaa_level, u32x2{cfg.window.width, cfg.window.height});

  ImageSystem image_sys{allocator};

  Dyn<FontSystem *> font_sys = FontSystem::create(allocator);

  ShaderSystem shader_sys{allocator};

  Dyn<WindowSystem *> window_sys = WindowSystem::create_SDL(allocator);

  trace("Creating Root Window"_str);

  Window window = window_sys->create_window(*instance, "Ashura"_str).unwrap();

  if (cfg.window.maximized)
  {
    window_sys->maximize(window);
  }
  else
  {
    window_sys->set_extent(window, u32x2{cfg.window.width, cfg.window.height});
  }

  if (cfg.window.full_screen)
  {
    window_sys->make_fullscreen(window);
  }
  else
  {
    window_sys->make_windowed(window);
  }

  if (cfg.window.borderless)
  {
    window_sys->make_borderless(window);
  }
  else
  {
    window_sys->make_bordered(window);
  }

  if (cfg.window.resizable)
  {
    window_sys->make_resizable(window);
  }
  else
  {
    window_sys->make_unresizable(window);
  }

  ClipBoard & clipboard = window_sys->get_clipboard();

  gpu::Surface surface = window_sys->get_surface(window);

  gpu::PresentMode present_mode =
    cfg.gpu.vsync ? gpu::PresentMode::Fifo : gpu::PresentMode::Immediate;

  Renderer renderer = Renderer::create(allocator);

  Canvas canvas{allocator};

  ui::System ui_sys{allocator};

  Vec<char> working_dir_copy = vec<char>(allocator, working_dir).unwrap();

  u32 const hardware_concurrency = std::thread::hardware_concurrency();

  u32 const num_worker_threads = max(hardware_concurrency, 2U) - 1U;

  nanoseconds const max_thread_sleep = 5ms;

  Vec<nanoseconds> worker_thread_sleep{allocator};

  for (u32 i = 0; i < num_worker_threads; i++)
  {
    worker_thread_sleep.push(max_thread_sleep).unwrap();
  }

  Dyn<Scheduler *> scheduler = Scheduler::create(
    allocator, std::this_thread::get_id(), {}, worker_thread_sleep);

  trace("All Core Systems Initialized"_str);

  nanoseconds min_frame_interval = 0ns;

  if (cfg.gpu.max_fps.is_some())
  {
    f64 const max_fpns          = cfg.gpu.max_fps.v() * (1 / 1'000'000'000.0);
    f64 const min_frame_time_ns = 1 / max_fpns;
    min_frame_interval = nanoseconds{(nanoseconds::rep) min_frame_time_ns};
  }

  Dyn<Engine *> engine =
    dyn<Engine>(inplace, allocator, allocator, std::move(logger),
                std::move(scheduler), std::move(file_sys), std::move(instance),
                *device, std::move(gpu_sys), std::move(image_sys),
                std::move(font_sys), std::move(shader_sys),
                std::move(window_sys), window, clipboard, surface, present_mode,
                std::move(renderer), std::move(canvas), std::move(ui_sys),
                std::move(working_dir_copy), std::move(cfg.pipeline_cache),
                min_frame_interval)
      .unwrap();

  hook_engine(engine);

  engine->engage_(cfg);

  return engine;
}

void Engine::engage_(EngineCfg const & cfg)
{
  window_sys->listen({this, [](Engine * engine, SystemEvent const & event) {
                        event.match(
                          [&](SystemTheme theme) {
                            InputState & f  = engine->input_state;
                            f.theme.theme   = theme;
                            f.theme.changed = true;
                          },
                          [](SystemEventType) {});
                      }});

  window_sys->listen(window, {this, window_event_listener});

  Vec<AnyFuture> futures{allocator};

  Vec<char> resolved_path{allocator};

  for (auto & [label, path] : cfg.shaders)
  {
    resolved_path.clear();
    path_join(working_dir, path, resolved_path).unwrap();
    trace("Loading shader: {} from : {}"_str, label, resolved_path);
    futures.push(shader_sys.load_from_path(std::move(label), resolved_path))
      .unwrap();
  }

  for (auto & [label, path] : cfg.fonts)
  {
    resolved_path.clear();
    path_join(working_dir, path, resolved_path).unwrap();
    trace("Loading font: {} from: {}"_str, label, resolved_path);
    futures
      .push(font_sys->load_from_path(std::move(label), resolved_path,
                                     cfg.font_height, 0))
      .unwrap();
  }

  for (auto & [label, path] : cfg.images)
  {
    resolved_path.clear();
    path_join(working_dir, path, resolved_path).unwrap();
    trace("Loading image: {}  from: {}"_str, label, resolved_path);
    futures.push(image_sys.load_from_path(std::move(label), resolved_path))
      .unwrap();
  }

  trace("Waiting for resources"_str);
  while (!await_futures(futures, 0ns))
  {
    gpu_sys.frame(nullptr);
    scheduler->run_main_loop(1ms, 1ms);
  }

  trace("All resources loaded"_str);

  renderer.acquire();
}

void Engine::shutdown()
{
  trace("Shutting down engine"_str);

  scheduler->shutdown();

  device->wait_idle().unwrap();

  renderer.release();
  device->uninit(swapchain);
  swapchain = nullptr;

  window_sys->uninit_window(window);
  window = nullptr;
  window_sys->shutdown();

  shader_sys.shutdown();
  font_sys->shutdown();
  image_sys.shutdown();

  Vec<u8> pipeline_cache{allocator};

  gpu_sys.shutdown(pipeline_cache);

  if (!pipeline_cache.is_empty())
  {
    write_to_file(pipeline_cache_path, pipeline_cache, false)
      .match(
        [&](Void) {
          trace("Saved pipeline cache to: {}"_str, pipeline_cache_path);
        },
        [&](IoErr err) {
          error("Error {} writing pipeline cache to {}"_str, err,
                pipeline_cache_path);
        });
  }

  canvas.reset();

  window_sys->shutdown();
  shader_sys.shutdown();
  font_sys->shutdown();
  image_sys.shutdown();

  instance->uninit(device.ptr());

  trace("Engine Uninitialized"_str);
}

time_point Engine::get_inputs_(time_point prev_frame_end)
{
  ScopeTrace poll_trace{
    {"frame.event_poll"_str, 0}
  };
  input_state.clear();

  auto const frame_start = steady_clock::now();
  auto const timedelta   = frame_start - prev_frame_end;

  input_state.stamp(frame_start, timedelta);
  window_sys->poll_events();

  input_state.window.surface_extent = window_sys->get_surface_extent(window);
  input_state.window.extent         = window_sys->get_extent(window);

  input_state.theme.theme = window_sys->get_theme();

  auto [mouse_btns, mouse_pos, mouse_window] = window_sys->get_mouse_state();
  input_state.mouse.focused                  = (mouse_window == window);
  input_state.mouse.position =
    mouse_pos - 0.5F * static_cast<f32x2>(input_state.window.extent);
  input_state.mouse.states = mouse_btns;

  auto [kb_mods, kb_window] = window_sys->get_keyboard_state(
    input_state.key.scan_states.view(), input_state.key.key_states.view());

  input_state.key.focused    = (kb_window == window);
  input_state.key.mod_states = kb_mods;

  return frame_start;
}

void Engine::run(ui::View & view, Fn<void(ui::Ctx const &)> loop)
{
  trace("Starting Engine Run Loop"_str);

  if (swapchain == nullptr)
  {
    recreate_swapchain_();
  }

  bool                  running            = true;
  Option<Cursor>        cursor             = Cursor::Default;
  Option<TextInputInfo> current_input_info = none;
  time_point            frame_end          = steady_clock::now();

  window_sys->set_cursor(cursor);

  auto spread = f32x2::splat(4.5);

  while (running)
  {
    ScopeTrace frame_trace{{"frame"_str}};

    auto const frame_start = get_inputs_(frame_end);

    if (input_state.window.resized || input_state.window.surface_resized)
    {
      gpu_sys.recreate_framebuffers(input_state.window.surface_extent);
    }

    ScopeTrace record_trace{{"frame.record"_str}};

    canvas.begin_recording(
      1 + GpuSystem::NUM_SCRATCH_COLOR_TEXTURES,
      1 + GpuSystem::NUM_SCRATCH_DEPTH_TEXTURES,
      gpu::Viewport{
        .offset{0, 0},
        .extent    = static_cast<f32x2>(input_state.window.surface_extent),
        .min_depth = 0,
        .max_depth = 1
    },
      static_cast<f32x2>(input_state.window.extent),
      input_state.window.surface_extent);

    running = ui_sys.tick(input_state, view, canvas, loop);

    {
      /* canvas.squircle(ShapeInfo{
        .area{{0, 0}, {200, 200}},
        .corner_radii = f32x4::splat(5),
        .tint         = ColorGradient{ios::DARK_GREEN}
      });*/

      /*      if (input_state.mouse.down(MouseButton::A1))
      {
        spread = spread + 1;
      }
      else if (input_state.mouse.down(MouseButton::A2))
      {
        spread = spread - 1;
      }*/

      // [ ] always-on borderless windows
      // [ ] ** add grain effect with uv shift; additive blend of the noise with the region: https://www.shadertoy.com/view/DdcfzH
      canvas.blur({
        .area{.center = input_state.mouse.position.unwrap_or(),
              .extent{875, 450}},
        .transform    = translate3d(vec3(f32x2::splat(200), 0.0F)),
        .corner_radii = f32x4::splat(25),
        .thickness    = spread,
        .tint         = f32x4::splat(0.75F)
      });
      // [ ] fix feathering
      canvas.rrect({
        .area{.center = input_state.mouse.position.unwrap_or(),
              .extent{875, 450}},
        .transform    = translate3d(f32x2::splat(200).append(0.0F)).to_mat(),
        .corner_radii = f32x4::splat(100),
        .stroke       = 0.0F,
        .thickness    = f32x2::splat(5.F),
        .tint         = mdc::GRAY_500,
        .feathering   = 15
      });
    }

    auto current_cursor = ui_sys.cursor;

    if (current_cursor != cursor)
    {
      cursor = current_cursor;
      window_sys->set_cursor(current_cursor);
    }

    auto input_info = ui_sys.text_input();

    if (input_info != current_input_info)
    {
      window_sys->set_text_input(window, input_info);
      current_input_info = input_info;
    }

    canvas.end_recording();

    renderer.render_canvas(gpu_sys.frame_graph_, canvas, gpu_sys.fb_,
                           gpu_sys.scratch_color_,
                           gpu_sys.scratch_depth_stencil_);
    gpu_sys.frame(swapchain);

    frame_end             = steady_clock::now();
    auto const frame_time = frame_end - frame_start;

    if (frame_time < min_frame_interval)
    {
      auto const sleep_dur = min_frame_interval - frame_time;
      std::this_thread::sleep_for(sleep_dur);
    }
  }

  trace("Ended Engine Run Loop");
}

Storage<Systems> systems_storage;

void hook_engine(Engine * instance)
{
  if (instance == nullptr)
  {
    ash::logger    = nullptr;
    ash::engine    = nullptr;
    ash::scheduler = nullptr;
    ash::sys       = nullptr;
    return;
  }

  ash::logger    = instance->logger;
  ash::engine    = instance;
  ash::scheduler = instance->scheduler;
  ash::sys = new (systems_storage.storage_) Systems{instance->get_systems()};
}

}    // namespace ash

ash::Engine * ::ash::engine = nullptr;
