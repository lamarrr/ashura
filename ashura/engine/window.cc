/// SPDX-License-Identifier: MIT
#include "ashura/engine/window.h"
#include "SDL3/SDL.h"
#include "SDL3/SDL_vulkan.h"
#include "ashura/gpu/vulkan.h"
#include "ashura/std/error.h"
#include "ashura/std/vec.h"
#include <cstring>

namespace ash
{

#define CHECK_SDL(cond_expr) CHECK(cond_expr, "SDL Error: ", SDL_GetError())

struct WindowImpl
{
  SDL_Window *                                  win       = nullptr;
  gpu::Surface                                  surface   = nullptr;
  SDL_WindowID                                  id        = 0;
  SparseVec<Vec<Fn<void(WindowEvent const &)>>> listeners = {};
  gpu::Instance *                               instance  = nullptr;
  Fn<WindowRegion(Vec2U)> hit_test = [](Vec2U) { return WindowRegion::Normal; };

  WindowImpl(AllocatorRef allocator, SDL_Window * window, gpu::Surface surface,
             SDL_WindowID id, gpu::Instance & instance) :
    win{window},
    surface{surface},
    id{id},
    listeners{allocator},
    instance{&instance}
  {
  }
};

struct ClipBoardImpl : ClipBoard
{
  static constexpr usize MAX_MIME_SIZE = 256;
  Vec<u8>                local_;

  ClipBoardImpl(AllocatorRef allocator) : local_{allocator}
  {
  }

  ClipBoardImpl(ClipBoardImpl const &)             = delete;
  ClipBoardImpl(ClipBoardImpl &&)                  = delete;
  ClipBoardImpl & operator=(ClipBoardImpl const &) = delete;
  ClipBoardImpl & operator=(ClipBoardImpl &&)      = delete;
  ~ClipBoardImpl() override                        = default;

  virtual Result<> get(Span<char const> mime, Vec<u8> & out) override
  {
    char mime_c_str[MAX_MIME_SIZE + 1];
    CHECK(to_c_str(mime, mime_c_str));
    usize  mime_data_len;
    void * data = SDL_GetClipboardData(mime_c_str, &mime_data_len);
    if (data == nullptr)
    {
      return Err{};
    }
    defer data_{[&] { SDL_free(data); }};

    out.extend(Span<u8 const>{reinterpret_cast<u8 *>(data), mime_data_len})
      .unwrap();
    return Ok{};
  }

  static void const * get_callback(void * pimpl, char const * mime_type,
                                   usize * size)
  {
    if (mime_type == nullptr || pimpl == nullptr)
    {
      *size = 0;
      return nullptr;
    }
    ClipBoardImpl & clipboard = *reinterpret_cast<ClipBoardImpl *>(pimpl);
    *size                     = clipboard.local_.size();
    return clipboard.local_.data();
  }

  static void cleanup_callback(void * pimpl)
  {
    ClipBoardImpl & clipboard = *reinterpret_cast<ClipBoardImpl *>(pimpl);
    clipboard.local_.clear();
  }

  virtual Result<> set(Span<char const> mime, Span<u8 const> data) override
  {
    if (data.is_empty() || mime.is_empty())
    {
      if (SDL_ClearClipboardData())
      {
        return Ok{};
      }
      return Err{};
    }

    char mime_c_str[MAX_MIME_SIZE + 1];
    CHECK(to_c_str(mime, mime_c_str));

    char const * mime_types[] = {mime_c_str};

    local_.extend(data).unwrap();

    bool failed =
      SDL_SetClipboardData(get_callback, cleanup_callback, this, mime_types, 1);

    if (failed)
    {
      return Err{};
    }

    return Ok{};
  }
};

struct WindowSystemImpl : WindowSystem
{
  AllocatorRef                                  allocator;
  SparseVec<Vec<Fn<void(SystemEvent const &)>>> listeners;
  ClipBoardImpl                                 clipboard;

  WindowSystemImpl(AllocatorRef allocator) :
    allocator{allocator},
    listeners{allocator},
    clipboard{allocator}
  {
  }

  WindowSystemImpl(WindowSystemImpl const &)             = delete;
  WindowSystemImpl(WindowSystemImpl &&)                  = delete;
  WindowSystemImpl & operator=(WindowSystemImpl const &) = delete;
  WindowSystemImpl & operator=(WindowSystemImpl &&)      = delete;
  ~WindowSystemImpl() override                           = default;

  void shutdown() override
  {
    SDL_Quit();
  }

  static inline SDL_Window * psdl(Window window)
  {
    return ((WindowImpl *) window)->win;
  }

  virtual Option<Window> create_window(gpu::Instance &  instance,
                                       Span<char const> title) override
  {
    char * title_c_str;
    if (!allocator->nalloc(title.size() + 1, title_c_str))
    {
      return none;
    }

    defer title_c_str_{
      [&] { allocator->ndealloc(title.size() + 1, title_c_str); }};

    mem::copy(title, title_c_str);
    title_c_str[title.size()] = 0;

    SDL_Window * window = SDL_CreateWindow(
      title_c_str, 1'920, 1'080, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    CHECK_SDL(window != nullptr);
    SDL_WindowID id = SDL_GetWindowID(window);
    CHECK_SDL(id != 0);

    CHECK(instance.get_backend() == gpu::Backend::Vulkan);

    vk::Instance & vk_instance = (vk::Instance &) instance;
    VkSurfaceKHR   surface;

    CHECK_SDL(SDL_Vulkan_CreateSurface(window, vk_instance.vk_instance, nullptr,
                                       &surface));

    WindowImpl * impl;

    CHECK(allocator->nalloc(1, impl));

    new (impl)
      WindowImpl{allocator, window, (gpu::Surface) surface, id, instance};

    SDL_PropertiesID props_id = SDL_GetWindowProperties(window);
    CHECK(SDL_SetPointerProperty(props_id, "impl", impl));

    return (Window) impl;
  }

  virtual void uninit_window(Window window) override
  {
    if (window != nullptr)
    {
      WindowImpl * win = (WindowImpl *) window;
      win->instance->uninit(win->surface);
      SDL_DestroyWindow(win->win);
      win->~WindowImpl();
      allocator->ndealloc(1, win);
    }
  }

  virtual void set_title(Window window, Span<char const> title) override
  {
    char * title_c_str;
    CHECK(allocator->nalloc(title.size() + 1, title_c_str));

    defer title_c_str_{
      [&] { allocator->ndealloc(title.size() + 1, title_c_str); }};

    mem::copy(title, title_c_str);
    title_c_str[title.size()] = 0;

    CHECK_SDL(SDL_SetWindowTitle(psdl(window), title_c_str));
  }

  virtual char const * get_title(Window window) override
  {
    char const * title = SDL_GetWindowTitle(psdl(window));
    CHECK_SDL(title != nullptr);
    return title;
  }

  virtual void maximize(Window window) override
  {
    CHECK_SDL(SDL_MaximizeWindow(psdl(window)));
  }

  virtual void minimize(Window window) override
  {
    CHECK_SDL(SDL_MinimizeWindow(psdl(window)));
  }

  virtual void set_extent(Window window, Vec2U extent) override
  {
    CHECK_SDL(SDL_SetWindowSize(psdl(window), static_cast<i32>(extent.x),
                                static_cast<i32>(extent.y)));
  }

  virtual void center(Window window) override
  {
    CHECK_SDL(SDL_SetWindowPosition(psdl(window), SDL_WINDOWPOS_CENTERED,
                                    SDL_WINDOWPOS_CENTERED));
  }

  virtual Vec2U get_extent(Window window) override
  {
    i32 width, height;
    CHECK_SDL(SDL_GetWindowSize(psdl(window), &width, &height));
    return Vec2U{static_cast<u32>(width), static_cast<u32>(height)};
  }

  virtual Vec2U get_surface_extent(Window window) override
  {
    i32 width, height;
    CHECK_SDL(SDL_GetWindowSizeInPixels(psdl(window), &width, &height));
    return Vec2U{static_cast<u32>(width), static_cast<u32>(height)};
  }

  virtual void set_position(Window window, Vec2I pos) override
  {
    CHECK_SDL(SDL_SetWindowPosition(psdl(window), pos.x, pos.y));
  }

  virtual Vec2I get_position(Window window) override
  {
    i32 x, y;
    CHECK_SDL(SDL_GetWindowPosition(psdl(window), &x, &y));
    return Vec2I{x, y};
  }

  virtual void set_min_extent(Window window, Vec2U min) override
  {
    CHECK_SDL(SDL_SetWindowMinimumSize(psdl(window), static_cast<i32>(min.x),
                                       static_cast<i32>(min.y)));
  }

  virtual Vec2U get_min_extent(Window window) override
  {
    i32 width, height;
    CHECK_SDL(SDL_GetWindowMinimumSize(psdl(window), &width, &height));
    return Vec2U{static_cast<u32>(width), static_cast<u32>(height)};
  }

  virtual void set_max_extent(Window window, Vec2U max) override
  {
    CHECK_SDL(SDL_SetWindowMaximumSize(psdl(window), static_cast<i32>(max.x),
                                       static_cast<i32>(max.y)));
  }

  virtual Vec2U get_max_extent(Window window) override
  {
    i32 width, height;
    CHECK_SDL(SDL_GetWindowMaximumSize(psdl(window), &width, &height));
    return Vec2U{static_cast<u32>(width), static_cast<u32>(height)};
  }

  virtual void set_icon(Window window, ImageSpan<u8 const, 4> image,
                        gpu::Format format) override
  {
    SDL_PixelFormat fmt = SDL_PIXELFORMAT_RGBA8888;

    switch (format)
    {
      case gpu::Format::R8G8B8A8_UNORM:
        fmt = SDL_PIXELFORMAT_RGBA8888;
        break;
      case gpu::Format::B8G8R8A8_UNORM:
        fmt = SDL_PIXELFORMAT_BGRA8888;
        break;
      default:
        CHECK(false, "unsupported image format");
    }

    SDL_Surface * icon = SDL_CreateSurfaceFrom(
      static_cast<i32>(image.extent.x), static_cast<i32>(image.extent.y), fmt,
      (void *) image.channels.data(), static_cast<i32>(image.pitch()));
    CHECK_SDL(icon != nullptr);
    CHECK_SDL(SDL_SetWindowIcon(psdl(window), icon));
    SDL_DestroySurface(icon);
  }

  virtual void make_bordered(Window window) override
  {
    CHECK_SDL(SDL_SetWindowBordered(psdl(window), true));
  }

  virtual void make_borderless(Window window) override
  {
    CHECK_SDL(SDL_SetWindowBordered(psdl(window), false));
  }

  virtual void show(Window window) override
  {
    CHECK_SDL(SDL_ShowWindow(psdl(window)));
  }

  virtual void hide(Window window) override
  {
    CHECK_SDL(SDL_HideWindow(psdl(window)));
  }

  virtual void raise(Window window) override
  {
    CHECK_SDL(SDL_RaiseWindow(psdl(window)));
  }

  virtual void restore(Window window) override
  {
    CHECK_SDL(SDL_RestoreWindow(psdl(window)));
  }

  virtual void request_attention(Window window, bool briefly) override
  {
    CHECK_SDL(SDL_FlashWindow(psdl(window), briefly ? SDL_FLASH_BRIEFLY :
                                                      SDL_FLASH_UNTIL_FOCUSED));
  }

  virtual void make_fullscreen(Window window) override
  {
    CHECK_SDL(SDL_SetWindowFullscreen(psdl(window), true));
  }

  virtual void make_windowed(Window window) override
  {
    CHECK_SDL(SDL_SetWindowFullscreen(psdl(window), false));
  }

  virtual void make_resizable(Window window) override
  {
    CHECK_SDL(SDL_SetWindowResizable(psdl(window), true));
  }

  virtual void make_unresizable(Window window) override
  {
    CHECK_SDL(SDL_SetWindowResizable(psdl(window), false));
  }

  virtual u64 listen(Fn<void(SystemEvent const &)> callback) override
  {
    return listeners.push(callback).unwrap();
  }

  virtual u64 listen(Window                        window,
                     Fn<void(WindowEvent const &)> callback) override
  {
    WindowImpl * pwin = (WindowImpl *) window;
    return pwin->listeners.push(callback).unwrap();
  }

  virtual void unlisten(Window window, u64 listener) override
  {
    WindowImpl * pwin = (WindowImpl *) window;
    pwin->listeners.erase(listener);
  }

  static SDL_HitTestResult sdl_hit_test(SDL_Window *, SDL_Point const * area,
                                        void * data)
  {
    WindowImpl * win    = (WindowImpl *) data;
    WindowRegion region = win->hit_test(Vec2U{(u32) area->x, (u32) area->y});
    switch (region)
    {
      case WindowRegion::Normal:
        return SDL_HITTEST_NORMAL;
      case WindowRegion::Draggable:
        return SDL_HITTEST_DRAGGABLE;
      case WindowRegion::ResizeTopLeft:
        return SDL_HITTEST_RESIZE_TOPLEFT;
      case WindowRegion::ResizeTop:
        return SDL_HITTEST_RESIZE_TOP;
      case WindowRegion::ResizeTopRight:
        return SDL_HITTEST_RESIZE_TOPRIGHT;
      case WindowRegion::ResizeRight:
        return SDL_HITTEST_RESIZE_RIGHT;
      case WindowRegion::ResizeBottomRight:
        return SDL_HITTEST_RESIZE_BOTTOMRIGHT;
      case WindowRegion::ResizeBottom:
        return SDL_HITTEST_RESIZE_BOTTOM;
      case WindowRegion::ResizeBottomLeft:
        return SDL_HITTEST_RESIZE_BOTTOMLEFT;
      case WindowRegion::ResizeLeft:
        return SDL_HITTEST_RESIZE_LEFT;
      default:
        return SDL_HITTEST_NORMAL;
    }
  }

  virtual Result<> set_hit_test(Window                  window,
                                Fn<WindowRegion(Vec2U)> hit) override
  {
    WindowImpl * pwin = (WindowImpl *) window;
    pwin->hit_test    = hit;
    if (SDL_SetWindowHitTest(pwin->win, sdl_hit_test, pwin) != 0)
    {
      return Err{};
    }

    return Ok{};
  }

  virtual gpu::Surface get_surface(Window window) override
  {
    WindowImpl * pwin = (WindowImpl *) window;
    return pwin->surface;
  }

  void push_window_event(SDL_WindowID window_id, WindowEvent const & event)
  {
    SDL_Window * win = SDL_GetWindowFromID(window_id);
    CHECK(win != nullptr);
    SDL_PropertiesID props_id = SDL_GetWindowProperties(win);
    WindowImpl *     impl =
      (WindowImpl *) SDL_GetPointerProperty(props_id, "impl", nullptr);
    CHECK(impl != nullptr);

    for (auto const & listener : impl->listeners.dense.v0)
    {
      listener(event);
    }
  }

  void push_system_event(SystemEvent const & event)
  {
    for (auto const & listener : listeners.dense.v0)
    {
      listener(event);
    }
  }

  virtual SystemTheme get_theme() override
  {
    SDL_SystemTheme theme = SDL_GetSystemTheme();
    switch (theme)
    {
      case SDL_SystemTheme::SDL_SYSTEM_THEME_DARK:
        return SystemTheme::Dark;
      case SDL_SystemTheme::SDL_SYSTEM_THEME_LIGHT:
        return SystemTheme::Light;
      case SDL_SystemTheme::SDL_SYSTEM_THEME_UNKNOWN:
        return SystemTheme::Unknown;
      default:
        CHECK_UNREACHABLE();
    }
  }

  virtual void poll_events() override
  {
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
      switch (event.type)
      {
        case SDL_EVENT_WINDOW_SHOWN:
          push_window_event(event.window.windowID, WindowEventType::Shown);
          break;
        case SDL_EVENT_WINDOW_HIDDEN:
          push_window_event(event.window.windowID, WindowEventType::Hidden);
          break;
        case SDL_EVENT_WINDOW_EXPOSED:
          push_window_event(event.window.windowID, WindowEventType::Exposed);
          break;
        case SDL_EVENT_WINDOW_MOVED:
          push_window_event(event.window.windowID, WindowEventType::Moved);
          break;
        case SDL_EVENT_WINDOW_RESIZED:
          push_window_event(event.window.windowID, WindowEventType::Resized);
          break;
        case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
          push_window_event(event.window.windowID,
                            WindowEventType::SurfaceResized);
          break;
        case SDL_EVENT_WINDOW_MINIMIZED:
          push_window_event(event.window.windowID, WindowEventType::Minimized);
          break;
        case SDL_EVENT_WINDOW_MAXIMIZED:
          push_window_event(event.window.windowID, WindowEventType::Maximized);
          break;
        case SDL_EVENT_WINDOW_RESTORED:
          push_window_event(event.window.windowID, WindowEventType::Restored);
          break;
        case SDL_EVENT_WINDOW_MOUSE_ENTER:
          push_window_event(event.window.windowID, WindowEventType::MouseEnter);
          break;
        case SDL_EVENT_WINDOW_MOUSE_LEAVE:
          push_window_event(event.window.windowID, WindowEventType::MouseLeave);
          break;
        case SDL_EVENT_WINDOW_FOCUS_GAINED:
          push_window_event(event.window.windowID,
                            WindowEventType::KeyboardFocusIn);
          break;
        case SDL_EVENT_WINDOW_FOCUS_LOST:
          push_window_event(event.window.windowID,
                            WindowEventType::KeyboardFocusOut);
          break;
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
          push_window_event(event.window.windowID,
                            WindowEventType::CloseRequested);
          break;
        case SDL_EVENT_WINDOW_OCCLUDED:
          push_window_event(event.window.windowID, WindowEventType::Occluded);
          break;
        case SDL_EVENT_WINDOW_ENTER_FULLSCREEN:
          push_window_event(event.window.windowID,
                            WindowEventType::EnterFullScreen);
          break;
        case SDL_EVENT_WINDOW_LEAVE_FULLSCREEN:
          push_window_event(event.window.windowID,
                            WindowEventType::LeaveFullScreen);
          break;
        case SDL_EVENT_WINDOW_DESTROYED:
          push_window_event(event.window.windowID, WindowEventType::Destroyed);
          break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP:
        {
          MouseClickEvent mouse_event{
            .position{event.button.x, event.button.y},
            .clicks = event.button.clicks
          };
          switch (event.button.button)
          {
            case SDL_BUTTON_LEFT:
              mouse_event.button = MouseButton::Primary;
              break;
            case SDL_BUTTON_RIGHT:
              mouse_event.button = MouseButton::Secondary;
              break;
            case SDL_BUTTON_MIDDLE:
              mouse_event.button = MouseButton::Middle;
              break;
            case SDL_BUTTON_X1:
              mouse_event.button = MouseButton::A1;
              break;
            case SDL_BUTTON_X2:
              mouse_event.button = MouseButton::A2;
              break;
            default:
              CHECK_UNREACHABLE();
          }

          switch (event.type)
          {
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
              mouse_event.action = KeyAction::Press;
              break;

            case SDL_EVENT_MOUSE_BUTTON_UP:
              mouse_event.action = KeyAction::Release;
              break;

            default:
              break;
          }

          push_window_event(event.button.windowID, mouse_event);
          break;
        }

        case SDL_EVENT_MOUSE_MOTION:
          push_window_event(
            event.motion.windowID,
            MouseMotionEvent{
              .position{event.motion.x,    event.motion.y   },
              .translation{event.motion.xrel, event.motion.yrel}
          });
          break;

        case SDL_EVENT_MOUSE_WHEEL:
          push_window_event(
            event.wheel.windowID,
            MouseWheelEvent{
              .position{event.wheel.mouse_x, event.wheel.mouse_y},
              .translation{event.wheel.x,       event.wheel.y      }
          });
          break;

        case SDL_EVENT_KEY_DOWN:
          push_window_event(
            event.key.windowID,
            KeyEvent{.scan_code = (ScanCode) event.key.scancode,
                     .key_code =
                       (KeyCode) ((u32) event.key.key & ~SDLK_SCANCODE_MASK),
                     .modifiers = (KeyModifiers) event.key.mod,
                     .action    = KeyAction::Press});
          break;

        case SDL_EVENT_KEY_UP:
          push_window_event(
            event.key.windowID,
            KeyEvent{.scan_code = (ScanCode) event.key.scancode,
                     .key_code =
                       (KeyCode) ((u32) event.key.key & ~SDLK_SCANCODE_MASK),
                     .modifiers = (KeyModifiers) event.key.mod,
                     .action    = KeyAction::Release});
          break;

        case SDL_EVENT_TEXT_INPUT:
        {
          char const * text = event.text.text;
          usize const  size = (text == nullptr) ? 0 : std::strlen(text);
          push_window_event(event.text.windowID,
                            TextInputEvent{
                              .text{(c8 const *) text, size}
          });
        }
        break;

        case SDL_EVENT_DROP_BEGIN:
          push_window_event(event.drop.windowID,
                            DropEvent{DropEventType::DropBegin});
          break;

        case SDL_EVENT_DROP_COMPLETE:
          push_window_event(event.drop.windowID,
                            DropEvent{DropEventType::DropComplete});
          break;

        case SDL_EVENT_DROP_POSITION:
          push_window_event(
            event.drop.windowID,
            DropEvent{DropPositionEvent{.pos{event.drop.x, event.drop.y}}});
          break;

        case SDL_EVENT_DROP_FILE:
        {
          char const * text = event.drop.data;
          usize const  size = (text == nullptr) ? 0 : std::strlen(text);
          push_window_event(event.drop.windowID,
                            DropEvent{DropFileEvent{.path{text, size}}});
        }
        break;

        case SDL_EVENT_DROP_TEXT:
        {
          c8 const *  text = reinterpret_cast<c8 const *>(event.drop.data);
          usize const size =
            (event.drop.data == nullptr) ? 0 : std::strlen(event.drop.data);
          push_window_event(event.drop.windowID,
                            DropEvent{DropTextEvent{.text{text, size}}});
        }
        break;

        case SDL_EVENT_SYSTEM_THEME_CHANGED:
          push_system_event(get_theme());
          break;

        case SDL_EVENT_KEYMAP_CHANGED:
          push_system_event(SystemEventType::KeymapChanged);
          break;

        case SDL_EVENT_AUDIO_DEVICE_ADDED:
          push_system_event(SystemEventType::AudioDeviceAdded);
          break;

        case SDL_EVENT_AUDIO_DEVICE_REMOVED:
          push_system_event(SystemEventType::AudioDeviceRemoved);
          break;

        case SDL_EVENT_AUDIO_DEVICE_FORMAT_CHANGED:
          push_system_event(SystemEventType::AudioDeviceFormatChanged);
          break;

        case SDL_EVENT_DISPLAY_ORIENTATION:
          push_system_event(SystemEventType::DisplayReoriented);
          break;

        case SDL_EVENT_DISPLAY_ADDED:
          push_system_event(SystemEventType::DisplayAdded);
          break;

        case SDL_EVENT_DISPLAY_REMOVED:
          push_system_event(SystemEventType::DisplayRemoved);
          break;

        case SDL_EVENT_DISPLAY_MOVED:
          push_system_event(SystemEventType::DisplayMoved);
          break;

        case SDL_EVENT_FINGER_DOWN:
        case SDL_EVENT_FINGER_UP:
        case SDL_EVENT_FINGER_MOTION:
          break;

        default:
          break;
      }
    }
  }

  virtual ClipBoard & get_clipboard() override
  {
    return clipboard;
  }

  virtual void get_keyboard_state(BitSpan<u64> state) override
  {
    CHECK(state.size() >= NUM_KEYS);
    i32          num_keys   = 0;
    bool const * key_states = SDL_GetKeyboardState(&num_keys);
    CHECK(num_keys == NUM_KEYS);

    for (usize i = 0; i < NUM_KEYS; i++)
    {
      state.set(i, key_states[i]);
    }
  }

  virtual Vec2 get_mouse_state(BitSpan<u64> state) override
  {
    CHECK(state.size() >= NUM_MOUSE_BUTTONS);
    Vec2                       pos;
    SDL_MouseButtonFlags const flags = SDL_GetMouseState(&pos.x, &pos.y);

    state.set((usize) MouseButton::Primary,
              flags & SDL_BUTTON_MASK(SDL_BUTTON_LEFT));
    state.set((usize) MouseButton::Secondary,
              flags & SDL_BUTTON_MASK(SDL_BUTTON_RIGHT));
    state.set((usize) MouseButton::Middle,
              flags & SDL_BUTTON_MASK(SDL_BUTTON_MIDDLE));
    state.set((usize) MouseButton::A1, flags & SDL_BUTTON_MASK(SDL_BUTTON_X1));
    state.set((usize) MouseButton::A2, flags & SDL_BUTTON_MASK(SDL_BUTTON_X2));

    return pos;
  }

  virtual void start_text_input(Window                window,
                                TextInputInfo const & info) override
  {
    SDL_PropertiesID props = SDL_CreateProperties();
    CHECK_SDL(props != 0);

    SDL_TextInputType  type = SDL_TEXTINPUT_TYPE_TEXT;
    SDL_Capitalization cap  = SDL_CAPITALIZE_NONE;

    switch (info.type)
    {
      case TextInputType::Text:
        type = SDL_TEXTINPUT_TYPE_TEXT;
        break;
      case TextInputType::Number:
        type = SDL_TEXTINPUT_TYPE_NUMBER;
        break;
      case TextInputType::Name:
        type = SDL_TEXTINPUT_TYPE_TEXT_NAME;
        break;
      case TextInputType::Email:
        type = SDL_TEXTINPUT_TYPE_TEXT_EMAIL;
        break;
      case TextInputType::Username:
        type = SDL_TEXTINPUT_TYPE_TEXT_USERNAME;
        break;
      case TextInputType::PasswordHidden:
        type = SDL_TEXTINPUT_TYPE_TEXT_PASSWORD_HIDDEN;
        break;
      case TextInputType::PasswordVisible:
        type = SDL_TEXTINPUT_TYPE_TEXT_PASSWORD_VISIBLE;
        break;
      case TextInputType::NumberPasswordHidden:
        type = SDL_TEXTINPUT_TYPE_NUMBER_PASSWORD_HIDDEN;
        break;
      case TextInputType::NumberPasswordVisible:
        type = SDL_TEXTINPUT_TYPE_NUMBER_PASSWORD_VISIBLE;
        break;
      default:
        CHECK_UNREACHABLE();
    }

    switch (info.cap)
    {
      case TextCapitalization::None:
        cap = SDL_CAPITALIZE_NONE;
        break;
      case TextCapitalization::Sentences:
        cap = SDL_CAPITALIZE_SENTENCES;
        break;
      case TextCapitalization::Words:
        cap = SDL_CAPITALIZE_WORDS;
        break;
      case TextCapitalization::Letters:
        cap = SDL_CAPITALIZE_LETTERS;
        break;
      default:
        CHECK_UNREACHABLE();
    }

    CHECK_SDL(
      SDL_SetNumberProperty(props, SDL_PROP_TEXTINPUT_TYPE_NUMBER, type));

    CHECK_SDL(SDL_SetNumberProperty(
      props, SDL_PROP_TEXTINPUT_CAPITALIZATION_NUMBER, cap));

    CHECK_SDL(SDL_SetBooleanProperty(
      props, SDL_PROP_TEXTINPUT_MULTILINE_BOOLEAN, info.multiline));

    CHECK_SDL(SDL_SetBooleanProperty(
      props, SDL_PROP_TEXTINPUT_AUTOCORRECT_BOOLEAN, info.autocorrect));

    WindowImpl * w = (WindowImpl *) window;

    CHECK_SDL(SDL_StartTextInputWithProperties(w->win, props));
  }

  virtual void end_text_input(Window window) override
  {
    WindowImpl * w = (WindowImpl *) window;
    CHECK_SDL(SDL_StopTextInput(w->win));
  }
};

Dyn<WindowSystem *> WindowSystem::create_SDL(AllocatorRef allocator)
{
  CHECK_SDL(SDL_Init(SDL_INIT_VIDEO));
  return cast<WindowSystem *>(
    dyn<WindowSystemImpl>(inplace, allocator, allocator).unwrap());
}

}    // namespace ash
