/// SPDX-License-Identifier: MIT
#include "ashura/engine/engine.h"
#include "ashura/engine/animation_system.h"
#include "ashura/engine/audio_system.h"
#include "ashura/engine/file_system.h"
#include "ashura/engine/font_system.h"
#include "ashura/engine/gpu_system.h"
#include "ashura/engine/image_system.h"
#include "ashura/engine/pipeline_system.h"
#include "ashura/engine/shader_system.h"
#include "ashura/engine/systems.h"
#include "ashura/engine/video_system.h"
#include "ashura/engine/view_system.h"
#include "ashura/engine/window_system.h"
#include "ashura/std/async.h"
#include "ashura/std/fs.h"
#include "ashura/std/sformat.h"
#include "ashura/std/trace.h"
#include "simdjson.h"

namespace ash
{

Result<EngineCfg> EngineCfg::parse_json(Span<u8 const> json,
                                        Allocator      allocator,
                                        Allocator      scratch_allocator)
{
  EngineCfg out{.window{.title{allocator}},
                .font_paths{allocator},
                .image_paths{allocator},
                .working_dir_path{allocator},
                .pipeline_cache_path{allocator}};

  Vec<u8> json_copy{scratch_allocator};
  if (!(json_copy.reserve_extend(json.size() + simdjson::SIMDJSON_PADDING) &&
        json_copy.append(json)))
  {
    return Err{};
  }

  simdjson::ondemand::parser parser;

  auto error = parser.iterate(json_copy.data(), json_copy.size_bytes(),
                              json_copy.capacity());

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

  auto title = cfg["window.title"].get_string().value();
  out.window.title.append(span(title)).unwrap();
  out.window.resizable   = cfg["window.resizable"].get_bool().value();
  out.window.maximized   = cfg["window.maximized"].get_bool().value();
  out.window.full_screen = cfg["window.full_screen"].get_bool().value();
  out.window.borderless  = cfg["window.borderless"].get_bool().value();
  out.window.vsync       = cfg["window.vsync"].get_bool().value();
  out.window.width       = (u32) clamp(cfg["window.width"].get_int64().value(),
                                       (i64) 0, (i64) U32_MAX);
  out.window.height      = (u32) clamp(cfg["window.height"].get_int64().value(),
                                       (i64) 0, (i64) U32_MAX);

  out.font_height =
    clamp((u32) cfg["fonts.height"].get_int64().value(), 16U, 256U);

  std::string_view pipeline_cache_path =
    cfg["cache.pipeline.path"].get_string().value();

  out.pipeline_cache_path.append(pipeline_cache_path).unwrap();

  std::string_view working_dir_path =
    cfg["paths.working_dir"].get_string().value();

  out.working_dir_path.append(working_dir_path).unwrap();

  return Ok{std::move(out)};
}

static void system_event_listener(Engine engine, SystemEvent const & event)
{
  event.match(
    [&](SystemTheme theme) {
      SystemState & s = engine->state_;
      s.theme.theme   = theme;
      s.theme.changed = true;
    },
    [](SystemEventType) {});
}

static void window_event_listener(IEngine::WindowEntry * win,
                                  WindowEvent const &    event)
{
  auto & s = win->state;

  event.match(
    [&](KeyEvent e) {
      switch (e.action)
      {
        case KeyAction::Press:
        {
          s.key.any_down = true;
          s.key.key_downs.set_bit((usize) e.key_code);
          s.key.scan_downs.set_bit((usize) e.scan_code);
          s.key.mod_downs |= e.modifiers;
        }
        break;
        case KeyAction::Release:
        {
          s.key.any_up = true;
          s.key.key_ups.set_bit((usize) e.key_code);
          s.key.scan_ups.set_bit((usize) e.scan_code);
          s.key.mod_ups |= e.modifiers;
        }
        break;
        default:
          break;
      }
    },
    [&](MouseMotionEvent e) {
      s.mouse.moved       = true;
      s.mouse.position    = e.position;
      s.mouse.translation = e.translation;
    },
    [&](MouseClickEvent e) {
      s.mouse.num_clicks[(u32) e.button] = e.clicks;
      s.mouse.position                   = e.position;
      switch (e.action)
      {
        case KeyAction::Press:
          s.mouse.downs |= static_cast<MouseButtons>(1U << (u32) e.button);
          s.mouse.any_down = true;
          break;
        case KeyAction::Release:
          s.mouse.ups |= static_cast<MouseButtons>(1U << (u32) e.button);
          s.mouse.any_up = true;
          break;
        default:
          break;
      }
    },
    [&](MouseWheelEvent e) {
      s.mouse.scrolled          = true;
      s.mouse.position          = e.position;
      s.mouse.wheel_translation = e.translation;
    },
    [&](TextInputEvent e) {
      s.key.input = true;
      s.key.text.append(e.text).unwrap();
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
          s.resized = true;
          break;
        case WindowEventType::SurfaceResized:
          s.surface_resized = true;
          break;
        case WindowEventType::Minimized:
        case WindowEventType::Maximized:
        case WindowEventType::Restored:
          break;
        case WindowEventType::MouseEnter:
          s.mouse.in = true;
          break;
        case WindowEventType::MouseLeave:
          s.mouse.out = true;
          break;
        case WindowEventType::KeyboardFocusIn:
          s.key.in = true;
          break;
        case WindowEventType::KeyboardFocusOut:
          s.key.out = true;
          break;
        case WindowEventType::CloseRequested:
          s.close_requested = true;
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
              s.drop.event = DropState::Event::Begin;
              break;
            case DropEventType::DropComplete:
              s.drop.event = DropState::Event::End;
              break;
            default:
              break;
          }
        },
        [&](DropPositionEvent e) { s.mouse.position = e.pos; },
        [&](DropFileEvent e) {
          s.drop.data.clear();
          s.drop.data.append(e.path.as_u8()).unwrap();
          s.drop.event = DropState::Event::FilePath;
        },
        [&](DropTextEvent e) {
          s.drop.data.clear();
          s.drop.data.append(e.text.as_u8()).unwrap();
          s.drop.event = DropState::Event::Bytes;
        });
    });
}

Dyn<Engine> IEngine::create(Allocator allocator, EngineCfg const & cfg,
                            Callbacks callbacks, WindowLoop loop)
{
  tracing::ScopeTrace _;

  Dyn<Logger> logger =
    dyn<ILogger>(inplace, default_allocator, span<ILogSink *>({&stdio_sink}))
      .unwrap();
  hook_logger(logger);

  trace("Initializing Engine Core Systems"_str);
  trace("Loading Graphics Pipeline Cache From {}"_str,
        cfg.pipeline_cache_path.view());

  Vec<u8> pipeline_cache{allocator};
  read_file(cfg.pipeline_cache_path, pipeline_cache, allocator)
    .match([](Void) {},
           [](IoErr err) {
             CHECK(err == IoErr::InvalidFileOrDir, "Io Error Occured");
           });

  constexpr Str const dedicated_thread_names[] = {
    "GpuThread"_str,
    "AudioThread"_str,
    "VideoThread"_str,
  };
  constexpr DedicatedThread                  gpu_thread   = DedicatedThread{0};
  [[maybe_unused]] constexpr DedicatedThread audio_thread = DedicatedThread{1};
  [[maybe_unused]] constexpr DedicatedThread video_thread = DedicatedThread{2};

  u32 const num_dedicated_threads =
    size(dedicated_thread_names);    // gpu/render, audio, video,
  u32 const hardware_concurrency = std::thread::hardware_concurrency();
  u32 const min_worker_threads   = 1;
  u32 const total_concurrency = max(num_dedicated_threads + min_worker_threads +
                                      // main thread
                                      1,
                                    hardware_concurrency);
  u32 const num_worker_threads =
    total_concurrency - (num_dedicated_threads + 1);

  Vec<SchedulerThreadInfo> dedicated_thread_infos{allocator};

  for (auto [i, name] : enumerate(dedicated_thread_names))
  {
    dedicated_thread_infos.push(SchedulerThreadInfo{.name = name}).unwrap();
  }

  Vec<StrVec>              worker_thread_names{allocator};
  Vec<SchedulerThreadInfo> worker_thread_infos{allocator};

  for (auto i : range(num_worker_threads))
  {
    worker_thread_names.push(sformat(allocator, "WorkerThread {}", i).unwrap())
      .unwrap();
    worker_thread_infos
      .push(SchedulerThreadInfo{.name = worker_thread_names.last()})
      .unwrap();
  }

  Dyn<Scheduler> scheduler = IScheduler::create(
    SchedulerInfo{.allocator         = allocator,
                  .dedicated_threads = dedicated_thread_infos,
                  .worker_threads    = worker_thread_infos,
                  .main_thread_id    = std::this_thread::get_id()});

  ash::sys.sched = scheduler;

  Dyn<FileSys> file_sys = dyn<IFileSys>(inplace, allocator, allocator).unwrap();

  ash::sys.file = file_sys;

  Dyn<WindowSys> window_sys = IWindowSys::create_SDL(allocator);

  ash::sys.win = window_sys;

  Dyn<gpu::Instance> gpu_instance =
    gpu::create_vulkan_instance(allocator, cfg.gpu.validation).unwrap();

  gpu::Device gpu_device =
    gpu_instance->create_device(allocator, cfg.gpu.preferences).unwrap();

  Dyn<GpuSys> gpu_sys = dyn<IGpuSys>(inplace, allocator).unwrap();

  ash::sys.gpu = gpu_sys;

  auto gpu_pref =
    GpuSysPreferences{.buffering             = cfg.gpu.buffering,
                      .cfg                   = GpuSysCfg{},
                      .color_formats         = ColorImage::SDR_FORMATS,
                      .depth_stencil_formats = DepthStencilImage::FORMATS};

  gpu_sys->init(allocator, gpu_device, pipeline_cache.view(), gpu_pref,
                scheduler.get(), gpu_thread);

  Dyn<ImageSys> image_sys =
    dyn<IImageSys>(inplace, allocator, allocator, gpu_sys.get(), file_sys.get())
      .unwrap();

  ash::sys.image = image_sys;

  Dyn<FontSys> font_sys = IFontSys::create(allocator);

  ash::sys.font = font_sys;

  Dyn<ShaderSys> shader_sys =
    dyn<IShaderSys>(inplace, allocator, gpu_sys.get(), file_sys.get()).unwrap();

  shader_sys->init(allocator);

  ash::sys.shader = shader_sys;

  Dyn<PipelineSys> pipeline_sys =
    dyn<IPipelineSys>(inplace, allocator, gpu_sys.get()).unwrap();

  ash::sys.pipeline = pipeline_sys;

  pipeline_sys->init(allocator);

  Dyn<ViewSys> view_sys = dyn<IViewSys>(inplace, allocator, allocator).unwrap();

  trace("All Core Systems Initialized"_str);

  Dyn<Engine> engine = dyn<IEngine>(inplace, allocator).unwrap();

  engine->allocator_    = allocator;
  engine->sys_          = Systems{.logger   = std::move(logger),
                                  .sched    = std::move(scheduler),
                                  .file     = std::move(file_sys),
                                  .gpu      = std::move(gpu_sys),
                                  .image    = std::move(image_sys),
                                  .font     = std::move(font_sys),
                                  .shader   = std::move(shader_sys),
                                  .win      = std::move(window_sys),
                                  .pipeline = std::move(pipeline_sys),
                                  .audio{},
                                  .video{},
                                  .animation{}};
  engine->gpu_instance_ = std::move(gpu_instance);
  engine->gpu_device_   = std::move(gpu_device);
  engine->buffering_    = cfg.gpu.buffering;
  engine->state_        = SystemState{allocator};
  engine->paths_        = Paths{
           .working_dir = vec::copy(allocator, cfg.working_dir_path.view()).unwrap(),
           .pipeline_cache =
      vec::copy(allocator, cfg.pipeline_cache_path.view()).unwrap()};
  engine->callbacks_ = std::move(callbacks);

  hook_engine(engine);

  engine->sys_.win->listen({engine.get(), system_event_listener});
  trace("Creating Root Window"_str);

  engine->window_ = engine->add_window_(cfg.window, std::move(loop));

  return engine;
}

Dyn<IEngine::WindowEntry *> IEngine::add_window_(EngineCfg::Window const & cfg,
                                                 WindowLoop                loop)
{
  auto entry    = dyn<WindowEntry>(inplace, allocator_).unwrap();
  entry->engine = this;

  auto window = &sys_.win->create_window(gpu_instance_, cfg.title).unwrap();

  entry->win      = window;
  entry->state    = WindowState{allocator_};
  entry->surface  = sys_.win->get_surface(window);
  entry->view_sys = dyn<IViewSys>(inplace, allocator_, allocator_).unwrap();
  entry->canvas   = dyn<ICanvas>(inplace, allocator_, allocator_).unwrap();
  entry->loop     = std::move(loop);
  entry->present_mode_preference =
    cfg.vsync ? gpu::PresentMode::Fifo : gpu::PresentMode::Immediate;

  sys_.win->listen(window, {entry.get(), window_event_listener});

  if (cfg.maximized)
  {
    sys_.win->maximize(window);
  }
  else
  {
    sys_.win->set_extent(window, u32x2{cfg.width, cfg.height});
  }

  if (cfg.full_screen)
  {
    sys_.win->make_fullscreen(window);
  }
  else
  {
    sys_.win->make_windowed(window);
  }

  if (cfg.borderless)
  {
    sys_.win->make_borderless(window);
  }
  else
  {
    sys_.win->make_bordered(window);
  }

  if (cfg.resizable)
  {
    sys_.win->make_resizable(window);
  }
  else
  {
    sys_.win->make_unresizable(window);
  }

  return entry;
}

void IEngine::remove_window_(IEngine::WindowEntry & entry)
{
  sys_.gpu->await_idle();
  entry.swapchain.match([&](gpu::ISwapchain & sc) { gpu_device_->uninit(&sc); },
                        []() {});
  sys_.win->uninit_window(entry.win);
}

void IEngine::poll_inputs_(time_point prev_frame_end, time_point frame_start)
{
  tracing::ScopeTrace _;

  auto const timedelta = frame_start - prev_frame_end;

  state_.clear();

  {
    window_->state.clear();
  }

  state_.stamp(frame_start, timedelta);
  state_.theme.theme = sys_.win->get_theme();
  sys_.win->poll_events();

  auto [mouse_btns, mouse_pos, mouse_window] = sys_.win->get_mouse_state();

  Bits<u64, NUM_KEY_CODES>  key_states;
  Bits<u64, NUM_SCAN_CODES> scan_states;

  auto [kb_mods, kb_window] =
    sys_.win->get_keyboard_state(scan_states.view(), key_states.view());

  {
    auto & s         = window_->state;
    s.extent         = sys_.win->get_extent(window_->win);
    s.surface_extent = sys_.win->get_surface_extent(window_->win);

    if (mouse_window.ptr() == window_->win)
    {
      s.mouse.focused  = true;
      s.mouse.position = mouse_pos - 0.5F * static_cast<f32x2>(s.extent);
      s.mouse.states   = mouse_btns;
    }

    if (kb_window.ptr() == window_->win)
    {
      s.key.focused     = true;
      s.key.mod_states  = kb_mods;
      s.key.scan_states = scan_states;
      s.key.key_states  = key_states;
    }
  }
}

void IEngine::shutdown()
{
  tracing::ScopeTrace _;

  trace("Shutting down engine"_str);
  callbacks_.pre_shutdown(this);

  scheduler->shutdown();

  gpu_device_->await_idle().unwrap();

  remove_window_(*window_);

  sys_.pipeline->shutdown();
  sys_.win->shutdown();
  sys_.shader->shutdown();
  sys_.font->shutdown();
  sys_.image->shutdown();

  Vec<u8> pipeline_cache{allocator_};

  sys_.gpu->shutdown(pipeline_cache);

  if (!pipeline_cache.is_empty())
  {
    write_to_file(paths_.pipeline_cache, pipeline_cache, false, allocator_)
      .match(
        [&](Void) {
          trace("Saved pipeline cache to: {}"_str, paths_.pipeline_cache);
        },
        [&](IoErr err) {
          error("Error {} writing pipeline cache to {}"_str, err,
                paths_.pipeline_cache);
        });
  }

  sys_.file->shutdown();
  sys_.sched->shutdown();

  gpu_instance_->uninit(gpu_device_);

  callbacks_.post_shutdown(this);

  trace("Engine Uninitialized"_str);
}

Option<gpu::SwapchainInfo>
  IEngine::create_swapchain_info_(WindowEntry const & w)
{
  gpu::SurfaceCapabilities capabilities =
    gpu_device_->get_surface_capabilities(w.surface).unwrap();
  CHECK(has_bits(capabilities.image_usage, gpu::ImageUsage::TransferDst |
                                             gpu::ImageUsage::ColorAttachment),
        "");

  Vec<gpu::SurfaceFormat> formats{allocator_};
  gpu_device_->get_surface_formats(w.surface, formats).unwrap();

  Vec<gpu::PresentMode> present_modes{allocator_};
  gpu_device_->get_surface_present_modes(w.surface, present_modes).unwrap();

  u32x2 surface_extent = sys_.win->get_surface_extent(w.win);

  if (surface_extent.any_zero())
  {
    return none;
  }

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
    w.present_mode_preference, gpu::PresentMode::Immediate,
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

  CHECK(found_format, "");

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

  CHECK(found_present_mode, "");

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

  return gpu::SwapchainInfo{.label   = "Window Swapchain"_str,
                            .surface = w.surface,
                            .format  = format,
                            .usage   = gpu::ImageUsage::TransferDst |
                                     gpu::ImageUsage::ColorAttachment,
                            .preferred_buffering = buffering_,
                            .present_mode        = present_mode,
                            .preferred_extent    = surface_extent,
                            .composite_alpha     = alpha};
}

void IEngine::run()
{
  tracing::ScopeTrace _;
  trace("Starting Engine Run Loop"_str);

  bool                  running            = true;
  Option<Cursor>        cursor             = Cursor::Default;
  Option<TextInputInfo> current_input_info = none;
  time_point            frame_end          = steady_clock::now();

  sys_.win->set_cursor(cursor);

  while (running)
  {
    tracing::ScopeTrace frame_trace{"frame"_str};

    auto const frame_start = steady_clock::now();
    poll_inputs_(frame_end, frame_start);
    auto * plan = sys_.gpu->current_plan();

    plan->await(nanoseconds::max());
    plan->reset();
    plan->begin();

    u32x2 required_framebuffer_extent = window_->state.surface_extent;

    plan->set_target(GpuFrameTargetInfo{.extent = required_framebuffer_extent,
                                        .color_format         = {},
                                        .depth_stencil_format = {}});

    {
      if (window_->swapchain.is_none())
      {
        create_swapchain_info_(*window_).match([&](gpu::SwapchainInfo info) {
          window_->swapchain = *gpu_device_->create_swapchain(info).unwrap();
        });
      }
      else
      {
        // if swapchain extent is 0, defer creation until first resize event
        if ((window_->state.resized || window_->state.surface_resized) &&
            !(window_->state.extent.any_zero() ||
              window_->state.surface_extent.any_zero()))
        {
          create_swapchain_info_(*window_).match([&](gpu::SwapchainInfo info) {
            plan->add_preframe_task([dev       = plan->device(),
                                     swapchain = &window_->swapchain.v(),
                                     info] {
              dev->mark_swapchain_out_of_date(swapchain, info).unwrap();
            });
          });
        }
      }
    }

    {
      plan->add_preframe_task(
        [swapchain = &window_->swapchain.v(), dev = plan->device()] {
          dev->acquire_next(swapchain).unwrap();
        });
    }

    {
      tracing::ScopeTrace record_trace{"frame.record"_str};

      auto & w = *window_;

      // [ ] framebuffer extent would be larger than surface extent since we render multiple windows into one
      // image
      w.canvas->begin(
        gpu::Viewport{
          .offset{0, 0},
          .extent    = w.state.surface_extent.to<f32>(),
          .min_depth = 0,
          .max_depth = 1
      },
        w.state.extent.to<f32>(), w.state.surface_extent);
      w.view_sys->tick(state_, w.state, w.canvas, w.loop);

      if (w.state.extent.all_nonzero() && w.state.surface_extent.all_nonzero())
      {
        plan->add_pass([swapchain = &w.swapchain.v()](GpuFrame            frame,
                                                      gpu::CommandEncoder enc) {
          auto * dev   = frame->dev();
          auto   state = dev->get_swapchain_state(swapchain).unwrap();

          gpu::ImageCopy const copies[] = {
            {.src_layers = {.aspects   = gpu::ImageAspects::Color,
                            .mip_level = 0,
                            .array_layers{0, 1}},
             .src_area   = {{0, 0, 0}, state.extent.append(1)},
             .dst_layers = {.aspects   = gpu::ImageAspects::Color,
                            .mip_level = 0,
                            .array_layers{0, 1}}}
          };

          state.current_image.match([&](u32 i) {
            auto image = frame->get_scratch_images()[0];
            enc->copy_image(image.color.image, state.images[i], copies);
            enc->present(swapchain);
          });
        });
      }

      w.canvas->end();
      w.canvas->execute(plan);
      w.canvas->reset();
    }

    // [ ] always-on borderless windows

    {
      if (window_->state.mouse.focused)
      {
        sys_.win->set_cursor(window_->view_sys->cursor);
      }

      if (window_->state.key.focused)
      {
        auto text = window_->view_sys->text_input();
        if (text != current_input_info)
        {
          sys_.win->set_text_input(window_->win, text);
          current_input_info = text;
        }
      }
    }

    plan->end();
    sys_.gpu->submit_frame();

    frame_end = steady_clock::now();

    sys_.sched->run_main_loop(milliseconds{10}, nanoseconds{500});
  }

  trace("Ended Engine Run Loop");
}

void hook_engine(Engine instance)
{
  ash::engine = instance;
}

}    // namespace ash

::ash::Engine ::ash::engine = nullptr;
