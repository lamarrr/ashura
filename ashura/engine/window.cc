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

#define CHECK_SDL(cond_expr) CHECK(cond_expr, "SDL Error: {}", SDL_GetError())

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

  virtual Result<> get(Str mime, Vec<u8> & out) override
  {
    char mime_c_str[MAX_MIME_SIZE + 1];
    CHECK(to_c_str(mime, mime_c_str), "");
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

  virtual Result<> set(Str mime, Span<u8 const> data) override
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
    CHECK(to_c_str(mime, mime_c_str), "");

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
  SDL_Cursor *                                  cursor;

  WindowSystemImpl(AllocatorRef allocator) :
    allocator{allocator},
    listeners{allocator},
    clipboard{allocator},
    cursor{nullptr}
  {
  }

  WindowSystemImpl(WindowSystemImpl const &)             = delete;
  WindowSystemImpl(WindowSystemImpl &&)                  = delete;
  WindowSystemImpl & operator=(WindowSystemImpl const &) = delete;
  WindowSystemImpl & operator=(WindowSystemImpl &&)      = delete;
  ~WindowSystemImpl() override                           = default;

  void shutdown() override
  {
    if (cursor != nullptr)
    {
      SDL_DestroyCursor(cursor);
      cursor = nullptr;
    }
    SDL_Quit();
  }

  static inline SDL_Window * psdl(Window window)
  {
    return ((WindowImpl *) window)->win;
  }

  virtual Option<Window> create_window(gpu::Instance & instance,
                                       Str             title) override
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

    CHECK(instance.get_backend() == gpu::Backend::Vulkan, "");

    vk::Instance & vk_instance = (vk::Instance &) instance;
    VkSurfaceKHR   surface;

    CHECK_SDL(SDL_Vulkan_CreateSurface(window, vk_instance.vk_instance, nullptr,
                                       &surface));

    WindowImpl * impl;

    CHECK(allocator->nalloc(1, impl), "");

    new (impl)
      WindowImpl{allocator, window, (gpu::Surface) surface, id, instance};

    SDL_PropertiesID props_id = SDL_GetWindowProperties(window);
    CHECK(SDL_SetPointerProperty(props_id, "impl", impl), "");

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

  virtual void set_title(Window window, Str title) override
  {
    char * title_c_str;
    CHECK(allocator->nalloc(title.size() + 1, title_c_str), "");

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
    CHECK(win != nullptr, "");
    SDL_PropertiesID props_id = SDL_GetWindowProperties(win);
    WindowImpl *     impl =
      (WindowImpl *) SDL_GetPointerProperty(props_id, "impl", nullptr);
    CHECK(impl != nullptr, "");

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
          push_window_event(event.key.windowID,
                            KeyEvent{.scan_code = (ScanCode) event.key.scancode,
                                     .key_code  = to_keycode(event.key.key),
                                     .modifiers = (KeyModifiers) event.key.mod,
                                     .action    = KeyAction::Press});
          break;

        case SDL_EVENT_KEY_UP:
          push_window_event(event.key.windowID,
                            KeyEvent{.scan_code = (ScanCode) event.key.scancode,
                                     .key_code  = to_keycode(event.key.key),
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

  static constexpr KeyCode to_keycode(SDL_Keycode code)
  {
    switch (code)
    {
      case SDLK_UNKNOWN:
        return KeyCode::Unknown;
      case SDLK_RETURN:
        return KeyCode::Return;
      case SDLK_ESCAPE:
        return KeyCode::Escape;
      case SDLK_BACKSPACE:
        return KeyCode::Backspace;
      case SDLK_TAB:
        return KeyCode::Tab;
      case SDLK_SPACE:
        return KeyCode::Space;
      case SDLK_EXCLAIM:
        return KeyCode::Exclaim;
      case SDLK_DBLAPOSTROPHE:
        return KeyCode::QuoteDbl;
      case SDLK_HASH:
        return KeyCode::Hash;
      case SDLK_DOLLAR:
        return KeyCode::Dollar;
      case SDLK_PERCENT:
        return KeyCode::Percent;
      case SDLK_AMPERSAND:
        return KeyCode::Ampersand;
      case SDLK_APOSTROPHE:
        return KeyCode::Quote;
      case SDLK_LEFTPAREN:
        return KeyCode::LeftParen;
      case SDLK_RIGHTPAREN:
        return KeyCode::RightParen;
      case SDLK_ASTERISK:
        return KeyCode::Asterisk;
      case SDLK_PLUS:
        return KeyCode::Plus;
      case SDLK_COMMA:
        return KeyCode::Comma;
      case SDLK_MINUS:
        return KeyCode::Minus;
      case SDLK_PERIOD:
        return KeyCode::Period;
      case SDLK_SLASH:
        return KeyCode::Slash;
      case SDLK_0:
        return KeyCode::Num0;
      case SDLK_1:
        return KeyCode::Num1;
      case SDLK_2:
        return KeyCode::Num2;
      case SDLK_3:
        return KeyCode::Num3;
      case SDLK_4:
        return KeyCode::Num4;
      case SDLK_5:
        return KeyCode::Num5;
      case SDLK_6:
        return KeyCode::Num6;
      case SDLK_7:
        return KeyCode::Num7;
      case SDLK_8:
        return KeyCode::Num8;
      case SDLK_9:
        return KeyCode::Num9;
      case SDLK_COLON:
        return KeyCode::Colon;
      case SDLK_SEMICOLON:
        return KeyCode::SemiColon;
      case SDLK_LESS:
        return KeyCode::Less;
      case SDLK_EQUALS:
        return KeyCode::Equals;
      case SDLK_GREATER:
        return KeyCode::Greater;
      case SDLK_QUESTION:
        return KeyCode::Question;
      case SDLK_AT:
        return KeyCode::At;
      case SDLK_LEFTBRACKET:
        return KeyCode::LeftBracket;
      case SDLK_BACKSLASH:
        return KeyCode::BackSlash;
      case SDLK_RIGHTBRACKET:
        return KeyCode::RightBracket;
      case SDLK_CARET:
        return KeyCode::Caret;
      case SDLK_UNDERSCORE:
        return KeyCode::Underscore;
      case SDLK_GRAVE:
        return KeyCode::BackQuote;
      case SDLK_A:
        return KeyCode::A;
      case SDLK_B:
        return KeyCode::B;
      case SDLK_C:
        return KeyCode::C;
      case SDLK_D:
        return KeyCode::D;
      case SDLK_E:
        return KeyCode::E;
      case SDLK_F:
        return KeyCode::F;
      case SDLK_G:
        return KeyCode::G;
      case SDLK_H:
        return KeyCode::H;
      case SDLK_I:
        return KeyCode::I;
      case SDLK_J:
        return KeyCode::J;
      case SDLK_K:
        return KeyCode::K;
      case SDLK_L:
        return KeyCode::L;
      case SDLK_M:
        return KeyCode::M;
      case SDLK_N:
        return KeyCode::N;
      case SDLK_O:
        return KeyCode::O;
      case SDLK_P:
        return KeyCode::P;
      case SDLK_Q:
        return KeyCode::Q;
      case SDLK_R:
        return KeyCode::R;
      case SDLK_S:
        return KeyCode::S;
      case SDLK_T:
        return KeyCode::T;
      case SDLK_U:
        return KeyCode::U;
      case SDLK_V:
        return KeyCode::V;
      case SDLK_W:
        return KeyCode::W;
      case SDLK_X:
        return KeyCode::X;
      case SDLK_Y:
        return KeyCode::Y;
      case SDLK_Z:
        return KeyCode::Z;
      case SDLK_LEFTBRACE:
        return KeyCode::LeftBrace;
      case SDLK_PIPE:
        return KeyCode::Pipe;
      case SDLK_RIGHTBRACE:
        return KeyCode::RightBrace;
      case SDLK_TILDE:
        return KeyCode::Tilde;
      case SDLK_DELETE:
        return KeyCode::Delete;
      case SDLK_PLUSMINUS:
        return KeyCode::PlusMinus;
      case SDLK_CAPSLOCK:
        return KeyCode::CapsLock;
      case SDLK_F1:
        return KeyCode::F1;
      case SDLK_F2:
        return KeyCode::F2;
      case SDLK_F3:
        return KeyCode::F3;
      case SDLK_F4:
        return KeyCode::F4;
      case SDLK_F5:
        return KeyCode::F5;
      case SDLK_F6:
        return KeyCode::F6;
      case SDLK_F7:
        return KeyCode::F7;
      case SDLK_F8:
        return KeyCode::F8;
      case SDLK_F9:
        return KeyCode::F9;
      case SDLK_F10:
        return KeyCode::F10;
      case SDLK_F11:
        return KeyCode::F11;
      case SDLK_F12:
        return KeyCode::F12;
      case SDLK_PRINTSCREEN:
        return KeyCode::PrintScreen;
      case SDLK_SCROLLLOCK:
        return KeyCode::ScrollLock;
      case SDLK_PAUSE:
        return KeyCode::Pause;
      case SDLK_INSERT:
        return KeyCode::Insert;
      case SDLK_HOME:
        return KeyCode::Home;
      case SDLK_PAGEUP:
        return KeyCode::PageUp;
      case SDLK_END:
        return KeyCode::End;
      case SDLK_PAGEDOWN:
        return KeyCode::PageDown;
      case SDLK_RIGHT:
        return KeyCode::Right;
      case SDLK_LEFT:
        return KeyCode::Left;
      case SDLK_DOWN:
        return KeyCode::Down;
      case SDLK_UP:
        return KeyCode::Up;
      case SDLK_NUMLOCKCLEAR:
        return KeyCode::NumLockClear;
      case SDLK_KP_DIVIDE:
        return KeyCode::KpDivide;
      case SDLK_KP_MULTIPLY:
        return KeyCode::KpMultiply;
      case SDLK_KP_MINUS:
        return KeyCode::KpMinus;
      case SDLK_KP_PLUS:
        return KeyCode::KpPlus;
      case SDLK_KP_ENTER:
        return KeyCode::KpEnter;
      case SDLK_KP_1:
        return KeyCode::Kp1;
      case SDLK_KP_2:
        return KeyCode::Kp2;
      case SDLK_KP_3:
        return KeyCode::Kp3;
      case SDLK_KP_4:
        return KeyCode::Kp4;
      case SDLK_KP_5:
        return KeyCode::Kp5;
      case SDLK_KP_6:
        return KeyCode::Kp6;
      case SDLK_KP_7:
        return KeyCode::Kp7;
      case SDLK_KP_8:
        return KeyCode::Kp8;
      case SDLK_KP_9:
        return KeyCode::Kp9;
      case SDLK_KP_0:
        return KeyCode::Kp0;
      case SDLK_KP_PERIOD:
        return KeyCode::KpPeriod;
      case SDLK_APPLICATION:
        return KeyCode::Application;
      case SDLK_POWER:
        return KeyCode::Power;
      case SDLK_KP_EQUALS:
        return KeyCode::KpEquals;
      case SDLK_F13:
        return KeyCode::F13;
      case SDLK_F14:
        return KeyCode::F14;
      case SDLK_F15:
        return KeyCode::F15;
      case SDLK_F16:
        return KeyCode::F16;
      case SDLK_F17:
        return KeyCode::F17;
      case SDLK_F18:
        return KeyCode::F18;
      case SDLK_F19:
        return KeyCode::F19;
      case SDLK_F20:
        return KeyCode::F20;
      case SDLK_F21:
        return KeyCode::F21;
      case SDLK_F22:
        return KeyCode::F22;
      case SDLK_F23:
        return KeyCode::F23;
      case SDLK_F24:
        return KeyCode::F24;
      case SDLK_EXECUTE:
        return KeyCode::Execute;
      case SDLK_HELP:
        return KeyCode::Help;
      case SDLK_MENU:
        return KeyCode::Menu;
      case SDLK_SELECT:
        return KeyCode::Select;
      case SDLK_STOP:
        return KeyCode::Stop;
      case SDLK_AGAIN:
        return KeyCode::Again;
      case SDLK_UNDO:
        return KeyCode::Undo;
      case SDLK_CUT:
        return KeyCode::Cut;
      case SDLK_COPY:
        return KeyCode::Copy;
      case SDLK_PASTE:
        return KeyCode::Paste;
      case SDLK_FIND:
        return KeyCode::Find;
      case SDLK_MUTE:
        return KeyCode::Mute;
      case SDLK_VOLUMEUP:
        return KeyCode::VolumeUp;
      case SDLK_VOLUMEDOWN:
        return KeyCode::VolumeDown;
      case SDLK_KP_COMMA:
        return KeyCode::KpComma;
      case SDLK_KP_EQUALSAS400:
        return KeyCode::KpEqualsAs400;
      case SDLK_ALTERASE:
        return KeyCode::AltErase;
      case SDLK_SYSREQ:
        return KeyCode::SysReq;
      case SDLK_CANCEL:
        return KeyCode::Cancel;
      case SDLK_CLEAR:
        return KeyCode::Clear;
      case SDLK_PRIOR:
        return KeyCode::Prior;
      case SDLK_RETURN2:
        return KeyCode::Return2;
      case SDLK_SEPARATOR:
        return KeyCode::Separator;
      case SDLK_OUT:
        return KeyCode::Out;
      case SDLK_OPER:
        return KeyCode::Oper;
      case SDLK_CLEARAGAIN:
        return KeyCode::ClearAgain;
      case SDLK_CRSEL:
        return KeyCode::CrSel;
      case SDLK_EXSEL:
        return KeyCode::ExSel;
      case SDLK_KP_00:
        return KeyCode::Kp00;
      case SDLK_KP_000:
        return KeyCode::Kp000;
      case SDLK_THOUSANDSSEPARATOR:
        return KeyCode::ThousandsSeparator;
      case SDLK_DECIMALSEPARATOR:
        return KeyCode::DecimalSeparator;
      case SDLK_CURRENCYUNIT:
        return KeyCode::CurrencyUnit;
      case SDLK_CURRENCYSUBUNIT:
        return KeyCode::CurrencySubUnit;
      case SDLK_KP_LEFTPAREN:
        return KeyCode::KpLeftParen;
      case SDLK_KP_RIGHTPAREN:
        return KeyCode::KpRightParen;
      case SDLK_KP_LEFTBRACE:
        return KeyCode::KpLeftBrace;
      case SDLK_KP_RIGHTBRACE:
        return KeyCode::KpRightBrace;
      case SDLK_KP_TAB:
        return KeyCode::KpTab;
      case SDLK_KP_BACKSPACE:
        return KeyCode::KpBackSpace;
      case SDLK_KP_A:
        return KeyCode::KpA;
      case SDLK_KP_B:
        return KeyCode::KpB;
      case SDLK_KP_C:
        return KeyCode::KpC;
      case SDLK_KP_D:
        return KeyCode::KpD;
      case SDLK_KP_E:
        return KeyCode::KpE;
      case SDLK_KP_F:
        return KeyCode::KpF;
      case SDLK_KP_XOR:
        return KeyCode::KpXor;
      case SDLK_KP_POWER:
        return KeyCode::KpPower;
      case SDLK_KP_PERCENT:
        return KeyCode::KpPercent;
      case SDLK_KP_LESS:
        return KeyCode::KpLess;
      case SDLK_KP_GREATER:
        return KeyCode::KpGreater;
      case SDLK_KP_AMPERSAND:
        return KeyCode::KpAmpersand;
      case SDLK_KP_DBLAMPERSAND:
        return KeyCode::KpDblAmpersand;
      case SDLK_KP_VERTICALBAR:
        return KeyCode::KpVerticalBar;
      case SDLK_KP_DBLVERTICALBAR:
        return KeyCode::KpDblverticalBar;
      case SDLK_KP_COLON:
        return KeyCode::KpColon;
      case SDLK_KP_HASH:
        return KeyCode::KpHash;
      case SDLK_KP_SPACE:
        return KeyCode::KpSpace;
      case SDLK_KP_AT:
        return KeyCode::KpAt;
      case SDLK_KP_EXCLAM:
        return KeyCode::KpExclam;
      case SDLK_KP_MEMSTORE:
        return KeyCode::KpMemStore;
      case SDLK_KP_MEMRECALL:
        return KeyCode::KpMemRecall;
      case SDLK_KP_MEMCLEAR:
        return KeyCode::KpMemClear;
      case SDLK_KP_MEMADD:
        return KeyCode::KpMemAdd;
      case SDLK_KP_MEMSUBTRACT:
        return KeyCode::KpMemSubtract;
      case SDLK_KP_MEMMULTIPLY:
        return KeyCode::KpMemMultiply;
      case SDLK_KP_MEMDIVIDE:
        return KeyCode::KpMemDivide;
      case SDLK_KP_PLUSMINUS:
        return KeyCode::KpPlusMinus;
      case SDLK_KP_CLEAR:
        return KeyCode::KpClear;
      case SDLK_KP_CLEARENTRY:
        return KeyCode::KpClearentry;
      case SDLK_KP_BINARY:
        return KeyCode::KpBinary;
      case SDLK_KP_OCTAL:
        return KeyCode::KpOctal;
      case SDLK_KP_DECIMAL:
        return KeyCode::KpDecimal;
      case SDLK_KP_HEXADECIMAL:
        return KeyCode::KpHexaDecimal;
      case SDLK_LCTRL:
        return KeyCode::LeftCtrl;
      case SDLK_LSHIFT:
        return KeyCode::LeftShift;
      case SDLK_LALT:
        return KeyCode::LeftAlt;
      case SDLK_LGUI:
        return KeyCode::LeftGui;
      case SDLK_RCTRL:
        return KeyCode::RightCtrl;
      case SDLK_RSHIFT:
        return KeyCode::RightShift;
      case SDLK_RALT:
        return KeyCode::RightAlt;
      case SDLK_RGUI:
        return KeyCode::RightGui;
      case SDLK_MODE:
        return KeyCode::Mode;
      case SDLK_SLEEP:
        return KeyCode::Sleep;
      case SDLK_WAKE:
        return KeyCode::Wake;
      case SDLK_CHANNEL_INCREMENT:
        return KeyCode::ChannelIncrement;
      case SDLK_CHANNEL_DECREMENT:
        return KeyCode::ChannelDecrement;
      case SDLK_MEDIA_PLAY:
        return KeyCode::MediaPlay;
      case SDLK_MEDIA_PAUSE:
        return KeyCode::MediaPause;
      case SDLK_MEDIA_RECORD:
        return KeyCode::MediaRecord;
      case SDLK_MEDIA_FAST_FORWARD:
        return KeyCode::MediaFastForward;
      case SDLK_MEDIA_REWIND:
        return KeyCode::MediaRewind;
      case SDLK_MEDIA_NEXT_TRACK:
        return KeyCode::MediaNextTrack;
      case SDLK_MEDIA_PREVIOUS_TRACK:
        return KeyCode::MediaPreviousTrack;
      case SDLK_MEDIA_STOP:
        return KeyCode::MediaStop;
      case SDLK_MEDIA_EJECT:
        return KeyCode::MediaEject;
      case SDLK_MEDIA_PLAY_PAUSE:
        return KeyCode::MediaPlayPause;
      case SDLK_MEDIA_SELECT:
        return KeyCode::MediaSelect;
      case SDLK_AC_NEW:
        return KeyCode::AcNew;
      case SDLK_AC_OPEN:
        return KeyCode::AcOpen;
      case SDLK_AC_CLOSE:
        return KeyCode::AcClose;
      case SDLK_AC_EXIT:
        return KeyCode::AcExit;
      case SDLK_AC_SAVE:
        return KeyCode::AcSave;
      case SDLK_AC_PRINT:
        return KeyCode::AcPrint;
      case SDLK_AC_PROPERTIES:
        return KeyCode::AcProperties;
      case SDLK_AC_SEARCH:
        return KeyCode::AcSearch;
      case SDLK_AC_HOME:
        return KeyCode::AcHome;
      case SDLK_AC_BACK:
        return KeyCode::AcBack;
      case SDLK_AC_FORWARD:
        return KeyCode::AcForward;
      case SDLK_AC_STOP:
        return KeyCode::AcStop;
      case SDLK_AC_REFRESH:
        return KeyCode::AcRefresh;
      case SDLK_AC_BOOKMARKS:
        return KeyCode::AcBookmarks;
      case SDLK_SOFTLEFT:
        return KeyCode::SoftLeft;
      case SDLK_SOFTRIGHT:
        return KeyCode::SoftRight;
      case SDLK_CALL:
        return KeyCode::Call;
      case SDLK_ENDCALL:
        return KeyCode::EndCall;
      case SDLK_LEFT_TAB:
        return KeyCode::LeftTab;
      case SDLK_LEVEL5_SHIFT:
        return KeyCode::Level5Shift;
      case SDLK_MULTI_KEY_COMPOSE:
        return KeyCode::MultiKeyCompose;
      case SDLK_LMETA:
        return KeyCode::LMeta;
      case SDLK_RMETA:
        return KeyCode::RMeta;
      case SDLK_LHYPER:
        return KeyCode::LHyper;
      case SDLK_RHYPER:
        return KeyCode::RHyper;
      default:
        return KeyCode::Unknown;
    }
  }

  virtual void get_keyboard_state(BitSpan<u64>   scan_state,
                                  BitSpan<u64>   key_state,
                                  KeyModifiers & modifiers) override
  {
    CHECK(scan_state.size() >= NUM_SCAN_CODES, "");
    CHECK(key_state.size() >= NUM_KEY_CODES, "");
    i32          num_keys       = 0;
    bool const * sdl_scan_state = SDL_GetKeyboardState(&num_keys);
    CHECK(num_keys == NUM_SCAN_CODES, "");

    SDL_Keymod const sdl_mod = SDL_GetModState();

    scan_state.clear_all_bits();
    key_state.clear_all_bits();

    for (usize i = 0; i < NUM_SCAN_CODES; i++)
    {
      if (sdl_scan_state[i])
      {
        scan_state.set_bit(i);

        SDL_Keycode const sdl_keycode =
          SDL_GetKeyFromScancode((SDL_Scancode) i, sdl_mod, true);
        key_state.set_bit((usize) to_keycode(sdl_keycode));
      }
    }

    modifiers = (KeyModifiers) sdl_mod;
  }

  virtual void get_mouse_state(MouseButtons & state, Vec2 & pos) override
  {
    pos   = {};
    state = MouseButtons::None;

    SDL_MouseButtonFlags const flags = SDL_GetMouseState(&pos.x, &pos.y);

    if (flags & SDL_BUTTON_MASK(SDL_BUTTON_LEFT))
    {
      state |= MouseButtons::Primary;
    }

    if (flags & SDL_BUTTON_MASK(SDL_BUTTON_RIGHT))
    {
      state |= MouseButtons::Secondary;
    }

    if (flags & SDL_BUTTON_MASK(SDL_BUTTON_MIDDLE))
    {
      state |= MouseButtons::Middle;
    }

    if (flags & SDL_BUTTON_MASK(SDL_BUTTON_X1))
    {
      state |= MouseButtons::A1;
    }

    if (flags & SDL_BUTTON_MASK(SDL_BUTTON_X2))
    {
      state |= MouseButtons::A2;
    }
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

  virtual void set_text_input_area(Window window, RectU const & rect,
                                   i32 cursor_position) override
  {
    WindowImpl *   w = (WindowImpl *) window;
    SDL_Rect const sdl_rect{.x = (i32) rect.offset.x,
                            .y = (i32) rect.offset.y,
                            .w = (i32) rect.extent.x,
                            .h = (i32) rect.extent.y};

    CHECK_SDL(SDL_SetTextInputArea(w->win, &sdl_rect, cursor_position));
  }

  virtual void set_cursor(Cursor c) override
  {
    SDL_SystemCursor sdl_cursor = SDL_SYSTEM_CURSOR_DEFAULT;
    switch (c)
    {
      case Cursor::Default:
        sdl_cursor = SDL_SYSTEM_CURSOR_DEFAULT;
        break;
      case Cursor::Text:
        sdl_cursor = SDL_SYSTEM_CURSOR_TEXT;
        break;
      case Cursor::Wait:
        sdl_cursor = SDL_SYSTEM_CURSOR_WAIT;
        break;
      case Cursor::CrossHair:
        sdl_cursor = SDL_SYSTEM_CURSOR_CROSSHAIR;
        break;
      case Cursor::Progress:
        sdl_cursor = SDL_SYSTEM_CURSOR_PROGRESS;
        break;
      case Cursor::NWSEResize:
        sdl_cursor = SDL_SYSTEM_CURSOR_NWSE_RESIZE;
        break;
      case Cursor::NESWResize:
        sdl_cursor = SDL_SYSTEM_CURSOR_NESW_RESIZE;
        break;
      case Cursor::EWResize:
        sdl_cursor = SDL_SYSTEM_CURSOR_EW_RESIZE;
        break;
      case Cursor::NSResize:
        sdl_cursor = SDL_SYSTEM_CURSOR_NS_RESIZE;
        break;
      case Cursor::Move:
        sdl_cursor = SDL_SYSTEM_CURSOR_MOVE;
        break;
      case Cursor::NotAllowed:
        sdl_cursor = SDL_SYSTEM_CURSOR_NOT_ALLOWED;
        break;
      case Cursor::Pointer:
        sdl_cursor = SDL_SYSTEM_CURSOR_POINTER;
        break;
      case Cursor::NWResize:
        sdl_cursor = SDL_SYSTEM_CURSOR_NW_RESIZE;
        break;
      case Cursor::NorthResize:
        sdl_cursor = SDL_SYSTEM_CURSOR_N_RESIZE;
        break;
      case Cursor::NEResize:
        sdl_cursor = SDL_SYSTEM_CURSOR_NE_RESIZE;
        break;
      case Cursor::EastResize:
        sdl_cursor = SDL_SYSTEM_CURSOR_E_RESIZE;
        break;
      case Cursor::SEResize:
        sdl_cursor = SDL_SYSTEM_CURSOR_SE_RESIZE;
        break;
      case Cursor::SouthResize:
        sdl_cursor = SDL_SYSTEM_CURSOR_S_RESIZE;
        break;
      case Cursor::SWResize:
        sdl_cursor = SDL_SYSTEM_CURSOR_SW_RESIZE;
        break;
      case Cursor::WestResize:
        sdl_cursor = SDL_SYSTEM_CURSOR_W_RESIZE;
        break;
      default:
        break;
    }

    if (cursor != nullptr)
    {
      SDL_DestroyCursor(cursor);
      cursor = nullptr;
    }

    cursor = SDL_CreateSystemCursor(sdl_cursor);
    CHECK_SDL(cursor != nullptr);
    CHECK_SDL(SDL_SetCursor(cursor));
  }

  virtual void lock_cursor(Window window, bool lock) override
  {
    WindowImpl * w = (WindowImpl *) window;
    CHECK_SDL(SDL_SetWindowRelativeMouseMode(w->win, lock));
  }
};

Dyn<WindowSystem *> WindowSystem::create_SDL(AllocatorRef allocator)
{
  CHECK_SDL(SDL_Init(SDL_INIT_VIDEO));
  return cast<WindowSystem *>(
    dyn<WindowSystemImpl>(inplace, allocator, allocator).unwrap());
}

}    // namespace ash
