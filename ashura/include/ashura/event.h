#pragma once

#include "SDL3/SDL_keycode.h"
#include "ashura/primitives.h"
#include "stx/enum.h"

namespace ash
{

using window_id = i32;

enum class WindowEvents : u32
{
  None    = 0,
  Shown   = 1,
  Hidden  = 1 << 1,
  Exposed = 1 << 2,
  Moved   = 1 << 3,
  /// window size changed by user
  Resized = 1 << 4,
  /// window size changed by user or via window API
  PixelSizeChanged = 1 << 5,
  Minimized        = 1 << 6,
  Maximized        = 1 << 7,
  Restored         = 1 << 8,
  MouseEnter       = 1 << 9,
  MouseLeave       = 1 << 10,
  FocusGained      = 1 << 11,
  FocusLost        = 1 << 12,
  CloseRequested   = 1 << 13,
  TakeFocus        = 1 << 14,
  All              = (1 << 15) - 1
};

STX_DEFINE_ENUM_BIT_OPS(WindowEvents)

enum class MouseButton : u8
{
  None      = 0,
  Primary   = 1,
  Secondary = 2,
  Middle    = 4,
  A1        = 8,
  A2        = 16,
  A3        = 32,
  A4        = 64,
  A5        = 128
};

STX_DEFINE_ENUM_BIT_OPS(MouseButton)

enum class KeyModifiers : u16
{
  None       = SDL_KMOD_NONE,
  LeftShift  = SDL_KMOD_LSHIFT,
  RightShift = SDL_KMOD_RSHIFT,
  LeftCtrl   = SDL_KMOD_LCTRL,
  RightCtrl  = SDL_KMOD_RCTRL,
  LeftAlt    = SDL_KMOD_LALT,
  RightAlt   = SDL_KMOD_RALT,
  LeftWin    = SDL_KMOD_LGUI,
  RightWin   = SDL_KMOD_RGUI,
  Num        = SDL_KMOD_NUM,
  Caps       = SDL_KMOD_CAPS,
  AltGr      = SDL_KMOD_MODE,
  ScrollLock = SDL_KMOD_SCROLL,
  Ctrl       = SDL_KMOD_CTRL,
  Shift      = SDL_KMOD_SHIFT,
  Alt        = SDL_KMOD_ALT,
  Gui        = SDL_KMOD_GUI
};

STX_DEFINE_ENUM_BIT_OPS(KeyModifiers)

using MouseID = u32;

enum class MouseAction : u8
{
  Press,
  Release,
};

struct MouseMotionEvent
{
  MouseID mouse_id = 0;
  vec2    position;
  vec2    translation;
};

struct MouseClickEvent
{
  MouseID     mouse_id = 0;
  vec2        position;
  u32         clicks = 0;
  MouseButton button = MouseButton::None;
  MouseAction action = MouseAction::Press;
};

struct MouseWheelEvent
{
  MouseID mouse_id = 0;
  vec2    position;
  vec2    translation;
};

using Key = SDL_Keycode;

namespace keys
{
constexpr Key UNKNOWN    = SDLK_UNKNOWN;
constexpr Key RETURN     = SDLK_RETURN;
constexpr Key ESCAPE     = SDLK_ESCAPE;
constexpr Key BACKSPACE  = SDLK_BACKSPACE;
constexpr Key TAB        = SDLK_TAB;
constexpr Key SPACE      = SDLK_SPACE;
constexpr Key EXCLAIM    = SDLK_EXCLAIM;
constexpr Key QUOTEDBL   = SDLK_QUOTEDBL;
constexpr Key HASH       = SDLK_HASH;
constexpr Key PERCENT    = SDLK_PERCENT;
constexpr Key DOLLAR     = SDLK_DOLLAR;
constexpr Key AMPERSAND  = SDLK_AMPERSAND;
constexpr Key QUOTE      = SDLK_QUOTE;
constexpr Key LEFTPAREN  = SDLK_LEFTPAREN;
constexpr Key RIGHTPAREN = SDLK_RIGHTPAREN;
constexpr Key ASTERISK   = SDLK_ASTERISK;
constexpr Key PLUS       = SDLK_PLUS;
constexpr Key COMMA      = SDLK_COMMA;
constexpr Key MINUS      = SDLK_MINUS;
constexpr Key PERIOD     = SDLK_PERIOD;
constexpr Key SLASH      = SDLK_SLASH;
constexpr Key NUM_0      = SDLK_0;
constexpr Key NUM_1      = SDLK_1;
constexpr Key NUM_2      = SDLK_2;
constexpr Key NUM_3      = SDLK_3;
constexpr Key NUM_4      = SDLK_4;
constexpr Key NUM_5      = SDLK_5;
constexpr Key NUM_6      = SDLK_6;
constexpr Key NUM_7      = SDLK_7;
constexpr Key NUM_8      = SDLK_8;
constexpr Key NUM_9      = SDLK_9;
constexpr Key COLON      = SDLK_COLON;
constexpr Key SEMICOLON  = SDLK_SEMICOLON;
constexpr Key LESS       = SDLK_LESS;
constexpr Key EQUALS     = SDLK_EQUALS;
constexpr Key GREATER    = SDLK_GREATER;
constexpr Key QUESTION   = SDLK_QUESTION;
constexpr Key AT         = SDLK_AT;

constexpr Key LEFTBRACKET  = SDLK_LEFTBRACKET;
constexpr Key BACKSLASH    = SDLK_BACKSLASH;
constexpr Key RIGHTBRACKET = SDLK_RIGHTBRACKET;
constexpr Key CARET        = SDLK_CARET;
constexpr Key UNDERSCORE   = SDLK_UNDERSCORE;
constexpr Key BACKQUOTE    = SDLK_BACKQUOTE;
constexpr Key a            = SDLK_a;
constexpr Key b            = SDLK_b;
constexpr Key c            = SDLK_c;
constexpr Key d            = SDLK_d;
constexpr Key e            = SDLK_e;
constexpr Key f            = SDLK_f;
constexpr Key g            = SDLK_g;
constexpr Key h            = SDLK_h;
constexpr Key i            = SDLK_i;
constexpr Key j            = SDLK_j;
constexpr Key k            = SDLK_k;
constexpr Key l            = SDLK_l;
constexpr Key m            = SDLK_m;
constexpr Key n            = SDLK_n;
constexpr Key o            = SDLK_o;
constexpr Key p            = SDLK_p;
constexpr Key q            = SDLK_q;
constexpr Key r            = SDLK_r;
constexpr Key s            = SDLK_s;
constexpr Key t            = SDLK_t;
constexpr Key u            = SDLK_u;
constexpr Key v            = SDLK_v;
constexpr Key w            = SDLK_w;
constexpr Key x            = SDLK_x;
constexpr Key y            = SDLK_y;
constexpr Key z            = SDLK_z;

constexpr Key CAPSLOCK = SDLK_CAPSLOCK;

constexpr Key F1  = SDLK_F1;
constexpr Key F2  = SDLK_F2;
constexpr Key F3  = SDLK_F3;
constexpr Key F4  = SDLK_F4;
constexpr Key F5  = SDLK_F5;
constexpr Key F6  = SDLK_F6;
constexpr Key F7  = SDLK_F7;
constexpr Key F8  = SDLK_F8;
constexpr Key F9  = SDLK_F9;
constexpr Key F10 = SDLK_F10;
constexpr Key F11 = SDLK_F11;
constexpr Key F12 = SDLK_F12;

constexpr Key PRINTSCREEN = SDLK_PRINTSCREEN;
constexpr Key SCROLLLOCK  = SDLK_SCROLLLOCK;
constexpr Key PAUSE       = SDLK_PAUSE;
constexpr Key INSERT      = SDLK_INSERT;
constexpr Key HOME        = SDLK_HOME;
constexpr Key PAGEUP      = SDLK_PAGEUP;
constexpr Key DELETE      = SDLK_DELETE;
constexpr Key END         = SDLK_END;
constexpr Key PAGEDOWN    = SDLK_PAGEDOWN;
constexpr Key RIGHT       = SDLK_RIGHT;
constexpr Key LEFT        = SDLK_LEFT;
constexpr Key DOWN        = SDLK_DOWN;
constexpr Key UP          = SDLK_UP;

constexpr Key NUMLOCKCLEAR = SDLK_NUMLOCKCLEAR;
constexpr Key KP_DIVIDE    = SDLK_KP_DIVIDE;
constexpr Key KP_MULTIPLY  = SDLK_KP_MULTIPLY;
constexpr Key KP_MINUS     = SDLK_KP_MINUS;
constexpr Key KP_PLUS      = SDLK_KP_PLUS;
constexpr Key KP_ENTER     = SDLK_KP_ENTER;
constexpr Key KP_1         = SDLK_KP_1;
constexpr Key KP_2         = SDLK_KP_2;
constexpr Key KP_3         = SDLK_KP_3;
constexpr Key KP_4         = SDLK_KP_4;
constexpr Key KP_5         = SDLK_KP_5;
constexpr Key KP_6         = SDLK_KP_6;
constexpr Key KP_7         = SDLK_KP_7;
constexpr Key KP_8         = SDLK_KP_8;
constexpr Key KP_9         = SDLK_KP_9;
constexpr Key KP_0         = SDLK_KP_0;
constexpr Key KP_PERIOD    = SDLK_KP_PERIOD;

constexpr Key APPLICATION    = SDLK_APPLICATION;
constexpr Key POWER          = SDLK_POWER;
constexpr Key KP_EQUALS      = SDLK_KP_EQUALS;
constexpr Key F13            = SDLK_F13;
constexpr Key F14            = SDLK_F14;
constexpr Key F15            = SDLK_F15;
constexpr Key F16            = SDLK_F16;
constexpr Key F17            = SDLK_F17;
constexpr Key F18            = SDLK_F18;
constexpr Key F19            = SDLK_F19;
constexpr Key F20            = SDLK_F20;
constexpr Key F21            = SDLK_F21;
constexpr Key F22            = SDLK_F22;
constexpr Key F23            = SDLK_F23;
constexpr Key F24            = SDLK_F24;
constexpr Key EXECUTE        = SDLK_EXECUTE;
constexpr Key HELP           = SDLK_HELP;
constexpr Key MENU           = SDLK_MENU;
constexpr Key SELECT         = SDLK_SELECT;
constexpr Key STOP           = SDLK_STOP;
constexpr Key AGAIN          = SDLK_AGAIN;
constexpr Key UNDO           = SDLK_UNDO;
constexpr Key CUT            = SDLK_CUT;
constexpr Key COPY           = SDLK_COPY;
constexpr Key PASTE          = SDLK_PASTE;
constexpr Key FIND           = SDLK_FIND;
constexpr Key MUTE           = SDLK_MUTE;
constexpr Key VOLUMEUP       = SDLK_VOLUMEUP;
constexpr Key VOLUMEDOWN     = SDLK_VOLUMEDOWN;
constexpr Key KP_COMMA       = SDLK_KP_COMMA;
constexpr Key KP_EQUALSAS400 = SDLK_KP_EQUALSAS400;

constexpr Key ALTERASE   = SDLK_ALTERASE;
constexpr Key SYSREQ     = SDLK_SYSREQ;
constexpr Key CANCEL     = SDLK_CANCEL;
constexpr Key CLEAR      = SDLK_CLEAR;
constexpr Key PRIOR      = SDLK_PRIOR;
constexpr Key RETURN2    = SDLK_RETURN2;
constexpr Key SEPARATOR  = SDLK_SEPARATOR;
constexpr Key OUT        = SDLK_OUT;
constexpr Key OPER       = SDLK_OPER;
constexpr Key CLEARAGAIN = SDLK_CLEARAGAIN;
constexpr Key CRSEL      = SDLK_CRSEL;
constexpr Key EXSEL      = SDLK_EXSEL;

constexpr Key KP_00              = SDLK_KP_00;
constexpr Key KP_000             = SDLK_KP_000;
constexpr Key THOUSANDSSEPARATOR = SDLK_THOUSANDSSEPARATOR;
constexpr Key DECIMALSEPARATOR   = SDLK_DECIMALSEPARATOR;
constexpr Key CURRENCYUNIT       = SDLK_CURRENCYUNIT;
constexpr Key CURRENCYSUBUNIT    = SDLK_CURRENCYSUBUNIT;
constexpr Key KP_LEFTPAREN       = SDLK_KP_LEFTPAREN;
constexpr Key KP_RIGHTPAREN      = SDLK_KP_RIGHTPAREN;
constexpr Key KP_LEFTBRACE       = SDLK_KP_LEFTBRACE;
constexpr Key KP_RIGHTBRACE      = SDLK_KP_RIGHTBRACE;
constexpr Key KP_TAB             = SDLK_KP_TAB;
constexpr Key KP_BACKSPACE       = SDLK_KP_BACKSPACE;
constexpr Key KP_A               = SDLK_KP_A;
constexpr Key KP_B               = SDLK_KP_B;
constexpr Key KP_C               = SDLK_KP_C;
constexpr Key KP_D               = SDLK_KP_D;
constexpr Key KP_E               = SDLK_KP_E;
constexpr Key KP_F               = SDLK_KP_F;
constexpr Key KP_XOR             = SDLK_KP_XOR;
constexpr Key KP_POWER           = SDLK_KP_POWER;
constexpr Key KP_PERCENT         = SDLK_KP_PERCENT;
constexpr Key KP_LESS            = SDLK_KP_LESS;
constexpr Key KP_GREATER         = SDLK_KP_GREATER;
constexpr Key KP_AMPERSAND       = SDLK_KP_AMPERSAND;
constexpr Key KP_DBLAMPERSAND    = SDLK_KP_DBLAMPERSAND;
constexpr Key KP_VERTICALBAR     = SDLK_KP_VERTICALBAR;
constexpr Key KP_DBLVERTICALBAR  = SDLK_KP_DBLVERTICALBAR;
constexpr Key KP_COLON           = SDLK_KP_COLON;
constexpr Key KP_HASH            = SDLK_KP_HASH;
constexpr Key KP_SPACE           = SDLK_KP_SPACE;
constexpr Key KP_AT              = SDLK_KP_AT;
constexpr Key KP_EXCLAM          = SDLK_KP_EXCLAM;
constexpr Key KP_MEMSTORE        = SDLK_KP_MEMSTORE;
constexpr Key KP_MEMRECALL       = SDLK_KP_MEMRECALL;
constexpr Key KP_MEMCLEAR        = SDLK_KP_MEMCLEAR;
constexpr Key KP_MEMADD          = SDLK_KP_MEMADD;
constexpr Key KP_MEMSUBTRACT     = SDLK_KP_MEMSUBTRACT;
constexpr Key KP_MEMMULTIPLY     = SDLK_KP_MEMMULTIPLY;
constexpr Key KP_MEMDIVIDE       = SDLK_KP_MEMDIVIDE;
constexpr Key KP_PLUSMINUS       = SDLK_KP_PLUSMINUS;
constexpr Key KP_CLEAR           = SDLK_KP_CLEAR;
constexpr Key KP_CLEARENTRY      = SDLK_KP_CLEARENTRY;
constexpr Key KP_BINARY          = SDLK_KP_BINARY;
constexpr Key KP_OCTAL           = SDLK_KP_OCTAL;
constexpr Key KP_DECIMAL         = SDLK_KP_DECIMAL;
constexpr Key KP_HEXADECIMAL     = SDLK_KP_HEXADECIMAL;

constexpr Key LCTRL  = SDLK_LCTRL;
constexpr Key LSHIFT = SDLK_LSHIFT;
constexpr Key LALT   = SDLK_LALT;
constexpr Key LGUI   = SDLK_LGUI;
constexpr Key RCTRL  = SDLK_RCTRL;
constexpr Key RSHIFT = SDLK_RSHIFT;
constexpr Key RALT   = SDLK_RALT;
constexpr Key RGUI   = SDLK_RGUI;

constexpr Key MODE = SDLK_MODE;

constexpr Key AUDIONEXT    = SDLK_AUDIONEXT;
constexpr Key AUDIOPREV    = SDLK_AUDIOPREV;
constexpr Key AUDIOSTOP    = SDLK_AUDIOSTOP;
constexpr Key AUDIOPLAY    = SDLK_AUDIOPLAY;
constexpr Key AUDIOMUTE    = SDLK_AUDIOMUTE;
constexpr Key MEDIASELECT  = SDLK_MEDIASELECT;
constexpr Key WWW          = SDLK_WWW;
constexpr Key MAIL         = SDLK_MAIL;
constexpr Key CALCULATOR   = SDLK_CALCULATOR;
constexpr Key COMPUTER     = SDLK_COMPUTER;
constexpr Key AC_SEARCH    = SDLK_AC_SEARCH;
constexpr Key AC_HOME      = SDLK_AC_HOME;
constexpr Key AC_BACK      = SDLK_AC_BACK;
constexpr Key AC_FORWARD   = SDLK_AC_FORWARD;
constexpr Key AC_STOP      = SDLK_AC_STOP;
constexpr Key AC_REFRESH   = SDLK_AC_REFRESH;
constexpr Key AC_BOOKMARKS = SDLK_AC_BOOKMARKS;

constexpr Key BRIGHTNESSDOWN = SDLK_BRIGHTNESSDOWN;
constexpr Key BRIGHTNESSUP   = SDLK_BRIGHTNESSUP;
constexpr Key DISPLAYSWITCH  = SDLK_DISPLAYSWITCH;
constexpr Key KBDILLUMTOGGLE = SDLK_KBDILLUMTOGGLE;
constexpr Key KBDILLUMDOWN   = SDLK_KBDILLUMDOWN;
constexpr Key KBDILLUMUP     = SDLK_KBDILLUMUP;
constexpr Key EJECT          = SDLK_EJECT;
constexpr Key SLEEP          = SDLK_SLEEP;
constexpr Key APP1           = SDLK_APP1;
constexpr Key APP2           = SDLK_APP2;

constexpr Key AUDIOREWIND      = SDLK_AUDIOREWIND;
constexpr Key AUDIOFASTFORWARD = SDLK_AUDIOFASTFORWARD;

constexpr Key SOFTLEFT  = SDLK_SOFTLEFT;
constexpr Key SOFTRIGHT = SDLK_SOFTRIGHT;
constexpr Key CALL      = SDLK_CALL;
constexpr Key ENDCALL   = SDLK_ENDCALL;
};        // namespace keys

struct ClipBoardEvent;        // TODO(lamarrr): on_update only

struct DeviceOrientationEvent;        // TODO(lamarrr)

struct PointerLock;        // TODO(lamarrr)

struct AudioDeviceEvent
{
  u32  device_id  = 0;
  bool is_capture = false;
};

struct WindowEventListeners
{
  stx::Vec<std::pair<WindowEvents, stx::UniqueFn<void(WindowEvents)>>> general{stx::os_allocator};
  stx::Vec<stx::UniqueFn<void(MouseClickEvent)>>                       mouse_click{stx::os_allocator};
  stx::Vec<stx::UniqueFn<void(MouseMotionEvent)>>                      mouse_motion{stx::os_allocator};
  stx::Vec<stx::UniqueFn<void(MouseWheelEvent)>>                       mouse_wheel{stx::os_allocator};
  stx::Vec<stx::UniqueFn<void(Key, KeyModifiers)>>                     key_down{stx::os_allocator};
  stx::Vec<stx::UniqueFn<void(Key, KeyModifiers)>>                     key_up{stx::os_allocator};
};

struct GlobalEventListeners
{
  stx::Vec<stx::UniqueFn<void(AudioDeviceEvent)>> audio_event{stx::os_allocator};
  stx::Vec<stx::UniqueFn<void()>>                 system_theme{stx::os_allocator};
};

}        // namespace ash
