#pragma once

#include "SDL3/SDL_Keycode.h"
#include "ashura/primitives.h"
#include "stx/allocator.h"
#include "stx/enum.h"
#include "stx/fn.h"
#include "stx/vec.h"

namespace ash
{

using WindowID      = u32;
using MouseID       = u32;
using AudioDeviceID = u32;

enum class WindowEvents : u32
{
  None             = 0,
  Shown            = 1,
  Hidden           = 1 << 1,
  Exposed          = 1 << 2,
  Moved            = 1 << 3,
  Resized          = 1 << 4,        // window size changed by user
  PixelSizeChanged = 1 << 5,        // window size changed by user or via window API
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

enum class KeyAction : u8
{
  Press,
  Release,
};

struct MouseMotionEvent
{
  MouseID mouse_id = 0;
  Vec2    position;
  Vec2    translation;
};

struct MouseClickEvent
{
  MouseID     mouse_id = 0;
  Vec2        position;
  u32         clicks = 0;
  MouseButton button = MouseButton::None;
  KeyAction   action = KeyAction::Press;
};

struct MouseWheelEvent
{
  MouseID mouse_id = 0;
  Vec2    position;
  Vec2    translation;
};

using Key = SDL_Keycode;

constexpr Key UNKNOWN_Key    = SDLK_UNKNOWN;
constexpr Key RETURN_Key     = SDLK_RETURN;
constexpr Key ESCAPE_Key     = SDLK_ESCAPE;
constexpr Key BACKSPACE_Key  = SDLK_BACKSPACE;
constexpr Key TAB_Key        = SDLK_TAB;
constexpr Key SPACE_Key      = SDLK_SPACE;
constexpr Key EXCLAIM_Key    = SDLK_EXCLAIM;
constexpr Key QUOTEDBL_Key   = SDLK_QUOTEDBL;
constexpr Key HASH_Key       = SDLK_HASH;
constexpr Key PERCENT_Key    = SDLK_PERCENT;
constexpr Key DOLLAR_Key     = SDLK_DOLLAR;
constexpr Key AMPERSAND_Key  = SDLK_AMPERSAND;
constexpr Key QUOTE_Key      = SDLK_QUOTE;
constexpr Key LEFTPAREN_Key  = SDLK_LEFTPAREN;
constexpr Key RIGHTPAREN_Key = SDLK_RIGHTPAREN;
constexpr Key ASTERISK_Key   = SDLK_ASTERISK;
constexpr Key PLUS_Key       = SDLK_PLUS;
constexpr Key COMMA_Key      = SDLK_COMMA;
constexpr Key MINUS_Key      = SDLK_MINUS;
constexpr Key PERIOD_Key     = SDLK_PERIOD;
constexpr Key SLASH_Key      = SDLK_SLASH;
constexpr Key NUM_0_Key      = SDLK_0;
constexpr Key NUM_1_Key      = SDLK_1;
constexpr Key NUM_2_Key      = SDLK_2;
constexpr Key NUM_3_Key      = SDLK_3;
constexpr Key NUM_4_Key      = SDLK_4;
constexpr Key NUM_5_Key      = SDLK_5;
constexpr Key NUM_6_Key      = SDLK_6;
constexpr Key NUM_7_Key      = SDLK_7;
constexpr Key NUM_8_Key      = SDLK_8;
constexpr Key NUM_9_Key      = SDLK_9;
constexpr Key COLON_Key      = SDLK_COLON;
constexpr Key SEMICOLON_Key  = SDLK_SEMICOLON;
constexpr Key LESS_Key       = SDLK_LESS;
constexpr Key EQUALS_Key     = SDLK_EQUALS;
constexpr Key GREATER_Key    = SDLK_GREATER;
constexpr Key QUESTION_Key   = SDLK_QUESTION;
constexpr Key AT_Key         = SDLK_AT;

constexpr Key LEFTBRACKET_Key  = SDLK_LEFTBRACKET;
constexpr Key BACKSLASH_Key    = SDLK_BACKSLASH;
constexpr Key RIGHTBRACKET_Key = SDLK_RIGHTBRACKET;
constexpr Key CARET_Key        = SDLK_CARET;
constexpr Key UNDERSCORE_Key   = SDLK_UNDERSCORE;
constexpr Key BACKQUOTE_Key    = SDLK_BACKQUOTE;
constexpr Key a_Key            = SDLK_a;
constexpr Key b_Key            = SDLK_b;
constexpr Key c_Key            = SDLK_c;
constexpr Key d_Key            = SDLK_d;
constexpr Key e_Key            = SDLK_e;
constexpr Key f_Key            = SDLK_f;
constexpr Key g_Key            = SDLK_g;
constexpr Key h_Key            = SDLK_h;
constexpr Key i_Key            = SDLK_i;
constexpr Key j_Key            = SDLK_j;
constexpr Key k_Key            = SDLK_k;
constexpr Key l_Key            = SDLK_l;
constexpr Key m_Key            = SDLK_m;
constexpr Key n_Key            = SDLK_n;
constexpr Key o_Key            = SDLK_o;
constexpr Key p_Key            = SDLK_p;
constexpr Key q_Key            = SDLK_q;
constexpr Key r_Key            = SDLK_r;
constexpr Key s_Key            = SDLK_s;
constexpr Key t_Key            = SDLK_t;
constexpr Key u_Key            = SDLK_u;
constexpr Key v_Key            = SDLK_v;
constexpr Key w_Key            = SDLK_w;
constexpr Key x_Key            = SDLK_x;
constexpr Key y_Key            = SDLK_y;
constexpr Key z_Key            = SDLK_z;

constexpr Key CAPSLOCK_Key = SDLK_CAPSLOCK;

constexpr Key F1_Key  = SDLK_F1;
constexpr Key F2_Key  = SDLK_F2;
constexpr Key F3_Key  = SDLK_F3;
constexpr Key F4_Key  = SDLK_F4;
constexpr Key F5_Key  = SDLK_F5;
constexpr Key F6_Key  = SDLK_F6;
constexpr Key F7_Key  = SDLK_F7;
constexpr Key F8_Key  = SDLK_F8;
constexpr Key F9_Key  = SDLK_F9;
constexpr Key F10_Key = SDLK_F10;
constexpr Key F11_Key = SDLK_F11;
constexpr Key F12_Key = SDLK_F12;

constexpr Key PRINTSCREEN_Key = SDLK_PRINTSCREEN;
constexpr Key SCROLLLOCK_Key  = SDLK_SCROLLLOCK;
constexpr Key PAUSE_Key       = SDLK_PAUSE;
constexpr Key INSERT_Key      = SDLK_INSERT;
constexpr Key HOME_Key        = SDLK_HOME;
constexpr Key PAGEUP_Key      = SDLK_PAGEUP;
constexpr Key DELETE_Key      = SDLK_DELETE;
constexpr Key END_Key         = SDLK_END;
constexpr Key PAGEDOWN_Key    = SDLK_PAGEDOWN;
constexpr Key RIGHT_Key       = SDLK_RIGHT;
constexpr Key LEFT_Key        = SDLK_LEFT;
constexpr Key DOWN_Key        = SDLK_DOWN;
constexpr Key UP_Key          = SDLK_UP;

constexpr Key NUMLOCKCLEAR_Key = SDLK_NUMLOCKCLEAR;
constexpr Key KP_DIVIDE_Key    = SDLK_KP_DIVIDE;
constexpr Key KP_MULTIPLY_Key  = SDLK_KP_MULTIPLY;
constexpr Key KP_MINUS_Key     = SDLK_KP_MINUS;
constexpr Key KP_PLUS_Key      = SDLK_KP_PLUS;
constexpr Key KP_ENTER_Key     = SDLK_KP_ENTER;
constexpr Key KP_1_Key         = SDLK_KP_1;
constexpr Key KP_2_Key         = SDLK_KP_2;
constexpr Key KP_3_Key         = SDLK_KP_3;
constexpr Key KP_4_Key         = SDLK_KP_4;
constexpr Key KP_5_Key         = SDLK_KP_5;
constexpr Key KP_6_Key         = SDLK_KP_6;
constexpr Key KP_7_Key         = SDLK_KP_7;
constexpr Key KP_8_Key         = SDLK_KP_8;
constexpr Key KP_9_Key         = SDLK_KP_9;
constexpr Key KP_0_Key         = SDLK_KP_0;
constexpr Key KP_PERIOD_Key    = SDLK_KP_PERIOD;

constexpr Key APPLICATION_Key    = SDLK_APPLICATION;
constexpr Key POWER_Key          = SDLK_POWER;
constexpr Key KP_EQUALS_Key      = SDLK_KP_EQUALS;
constexpr Key F13_Key            = SDLK_F13;
constexpr Key F14_Key            = SDLK_F14;
constexpr Key F15_Key            = SDLK_F15;
constexpr Key F16_Key            = SDLK_F16;
constexpr Key F17_Key            = SDLK_F17;
constexpr Key F18_Key            = SDLK_F18;
constexpr Key F19_Key            = SDLK_F19;
constexpr Key F20_Key            = SDLK_F20;
constexpr Key F21_Key            = SDLK_F21;
constexpr Key F22_Key            = SDLK_F22;
constexpr Key F23_Key            = SDLK_F23;
constexpr Key F24_Key            = SDLK_F24;
constexpr Key EXECUTE_Key        = SDLK_EXECUTE;
constexpr Key HELP_Key           = SDLK_HELP;
constexpr Key MENU_Key           = SDLK_MENU;
constexpr Key SELECT_Key         = SDLK_SELECT;
constexpr Key STOP_Key           = SDLK_STOP;
constexpr Key AGAIN_Key          = SDLK_AGAIN;
constexpr Key UNDO_Key           = SDLK_UNDO;
constexpr Key CUT_Key            = SDLK_CUT;
constexpr Key COPY_Key           = SDLK_COPY;
constexpr Key PASTE_Key          = SDLK_PASTE;
constexpr Key FIND_Key           = SDLK_FIND;
constexpr Key MUTE_Key           = SDLK_MUTE;
constexpr Key VOLUMEUP_Key       = SDLK_VOLUMEUP;
constexpr Key VOLUMEDOWN_Key     = SDLK_VOLUMEDOWN;
constexpr Key KP_COMMA_Key       = SDLK_KP_COMMA;
constexpr Key KP_EQUALSAS400_Key = SDLK_KP_EQUALSAS400;

constexpr Key ALTERASE_Key   = SDLK_ALTERASE;
constexpr Key SYSREQ_Key     = SDLK_SYSREQ;
constexpr Key CANCEL_Key     = SDLK_CANCEL;
constexpr Key CLEAR_Key      = SDLK_CLEAR;
constexpr Key PRIOR_Key      = SDLK_PRIOR;
constexpr Key RETURN2_Key    = SDLK_RETURN2;
constexpr Key SEPARATOR_Key  = SDLK_SEPARATOR;
constexpr Key OUT_Key        = SDLK_OUT;
constexpr Key OPER_Key       = SDLK_OPER;
constexpr Key CLEARAGAIN_Key = SDLK_CLEARAGAIN;
constexpr Key CRSEL_Key      = SDLK_CRSEL;
constexpr Key EXSEL_Key      = SDLK_EXSEL;

constexpr Key KP_00_Key              = SDLK_KP_00;
constexpr Key KP_000_Key             = SDLK_KP_000;
constexpr Key THOUSANDSSEPARATOR_Key = SDLK_THOUSANDSSEPARATOR;
constexpr Key DECIMALSEPARATOR_Key   = SDLK_DECIMALSEPARATOR;
constexpr Key CURRENCYUNIT_Key       = SDLK_CURRENCYUNIT;
constexpr Key CURRENCYSUBUNIT_Key    = SDLK_CURRENCYSUBUNIT;
constexpr Key KP_LEFTPAREN_Key       = SDLK_KP_LEFTPAREN;
constexpr Key KP_RIGHTPAREN_Key      = SDLK_KP_RIGHTPAREN;
constexpr Key KP_LEFTBRACE_Key       = SDLK_KP_LEFTBRACE;
constexpr Key KP_RIGHTBRACE_Key      = SDLK_KP_RIGHTBRACE;
constexpr Key KP_TAB_Key             = SDLK_KP_TAB;
constexpr Key KP_BACKSPACE_Key       = SDLK_KP_BACKSPACE;
constexpr Key KP_A_Key               = SDLK_KP_A;
constexpr Key KP_B_Key               = SDLK_KP_B;
constexpr Key KP_C_Key               = SDLK_KP_C;
constexpr Key KP_D_Key               = SDLK_KP_D;
constexpr Key KP_E_Key               = SDLK_KP_E;
constexpr Key KP_F_Key               = SDLK_KP_F;
constexpr Key KP_XOR_Key             = SDLK_KP_XOR;
constexpr Key KP_POWER_Key           = SDLK_KP_POWER;
constexpr Key KP_PERCENT_Key         = SDLK_KP_PERCENT;
constexpr Key KP_LESS_Key            = SDLK_KP_LESS;
constexpr Key KP_GREATER_Key         = SDLK_KP_GREATER;
constexpr Key KP_AMPERSAND_Key       = SDLK_KP_AMPERSAND;
constexpr Key KP_DBLAMPERSAND_Key    = SDLK_KP_DBLAMPERSAND;
constexpr Key KP_VERTICALBAR_Key     = SDLK_KP_VERTICALBAR;
constexpr Key KP_DBLVERTICALBAR_Key  = SDLK_KP_DBLVERTICALBAR;
constexpr Key KP_COLON_Key           = SDLK_KP_COLON;
constexpr Key KP_HASH_Key            = SDLK_KP_HASH;
constexpr Key KP_SPACE_Key           = SDLK_KP_SPACE;
constexpr Key KP_AT_Key              = SDLK_KP_AT;
constexpr Key KP_EXCLAM_Key          = SDLK_KP_EXCLAM;
constexpr Key KP_MEMSTORE_Key        = SDLK_KP_MEMSTORE;
constexpr Key KP_MEMRECALL_Key       = SDLK_KP_MEMRECALL;
constexpr Key KP_MEMCLEAR_Key        = SDLK_KP_MEMCLEAR;
constexpr Key KP_MEMADD_Key          = SDLK_KP_MEMADD;
constexpr Key KP_MEMSUBTRACT_Key     = SDLK_KP_MEMSUBTRACT;
constexpr Key KP_MEMMULTIPLY_Key     = SDLK_KP_MEMMULTIPLY;
constexpr Key KP_MEMDIVIDE_Key       = SDLK_KP_MEMDIVIDE;
constexpr Key KP_PLUSMINUS_Key       = SDLK_KP_PLUSMINUS;
constexpr Key KP_CLEAR_Key           = SDLK_KP_CLEAR;
constexpr Key KP_CLEARENTRY_Key      = SDLK_KP_CLEARENTRY;
constexpr Key KP_BINARY_Key          = SDLK_KP_BINARY;
constexpr Key KP_OCTAL_Key           = SDLK_KP_OCTAL;
constexpr Key KP_DECIMAL_Key         = SDLK_KP_DECIMAL;
constexpr Key KP_HEXADECIMAL_Key     = SDLK_KP_HEXADECIMAL;

constexpr Key LCTRL_Key  = SDLK_LCTRL;
constexpr Key LSHIFT_Key = SDLK_LSHIFT;
constexpr Key LALT_Key   = SDLK_LALT;
constexpr Key LGUI_Key   = SDLK_LGUI;
constexpr Key RCTRL_Key  = SDLK_RCTRL;
constexpr Key RSHIFT_Key = SDLK_RSHIFT;
constexpr Key RALT_Key   = SDLK_RALT;
constexpr Key RGUI_Key   = SDLK_RGUI;

constexpr Key MODE_Key = SDLK_MODE;

constexpr Key AUDIONEXT_Key    = SDLK_AUDIONEXT;
constexpr Key AUDIOPREV_Key    = SDLK_AUDIOPREV;
constexpr Key AUDIOSTOP_Key    = SDLK_AUDIOSTOP;
constexpr Key AUDIOPLAY_Key    = SDLK_AUDIOPLAY;
constexpr Key AUDIOMUTE_Key    = SDLK_AUDIOMUTE;
constexpr Key MEDIASELECT_Key  = SDLK_MEDIASELECT;
constexpr Key WWW_Key          = SDLK_WWW;
constexpr Key MAIL_Key         = SDLK_MAIL;
constexpr Key CALCULATOR_Key   = SDLK_CALCULATOR;
constexpr Key COMPUTER_Key     = SDLK_COMPUTER;
constexpr Key AC_SEARCH_Key    = SDLK_AC_SEARCH;
constexpr Key AC_HOME_Key      = SDLK_AC_HOME;
constexpr Key AC_BACK_Key      = SDLK_AC_BACK;
constexpr Key AC_FORWARD_Key   = SDLK_AC_FORWARD;
constexpr Key AC_STOP_Key      = SDLK_AC_STOP;
constexpr Key AC_REFRESH_Key   = SDLK_AC_REFRESH;
constexpr Key AC_BOOKMARKS_Key = SDLK_AC_BOOKMARKS;

constexpr Key BRIGHTNESSDOWN_Key = SDLK_BRIGHTNESSDOWN;
constexpr Key BRIGHTNESSUP_Key   = SDLK_BRIGHTNESSUP;
constexpr Key DISPLAYSWITCH_Key  = SDLK_DISPLAYSWITCH;
constexpr Key KBDILLUMTOGGLE_Key = SDLK_KBDILLUMTOGGLE;
constexpr Key KBDILLUMDOWN_Key   = SDLK_KBDILLUMDOWN;
constexpr Key KBDILLUMUP_Key     = SDLK_KBDILLUMUP;
constexpr Key EJECT_Key          = SDLK_EJECT;
constexpr Key SLEEP_Key          = SDLK_SLEEP;
constexpr Key APP1_Key           = SDLK_APP1;
constexpr Key APP2_Key           = SDLK_APP2;

constexpr Key AUDIOREWIND_Key      = SDLK_AUDIOREWIND;
constexpr Key AUDIOFASTFORWARD_Key = SDLK_AUDIOFASTFORWARD;

constexpr Key SOFTLEFT_Key  = SDLK_SOFTLEFT;
constexpr Key SOFTRIGHT_Key = SDLK_SOFTRIGHT;
constexpr Key CALL_Key      = SDLK_CALL;
constexpr Key ENDCALL_Key   = SDLK_ENDCALL;

struct ClipBoardEvent;        // TODO(lamarrr): on_update only

struct DeviceOrientationEvent;        // TODO(lamarrr)

struct PointerLock;        // TODO(lamarrr)

struct KeyEvent
{
  Key          key       = UNKNOWN_Key;
  KeyModifiers modifiers = KeyModifiers::None;
  KeyAction    action    = KeyAction::Press;
};

struct AudioDeviceEvent
{
  AudioDeviceID device_id  = 0;
  bool          is_capture = false;
};

struct WindowEventListeners
{
  stx::Vec<std::pair<WindowEvents, stx::UniqueFn<void(WindowEvents)>>> general;
  stx::Vec<stx::UniqueFn<void(MouseClickEvent)>>                       mouse_click;
  stx::Vec<stx::UniqueFn<void(MouseMotionEvent)>>                      mouse_motion;
  stx::Vec<stx::UniqueFn<void(MouseWheelEvent)>>                       mouse_wheel;
  stx::Vec<stx::UniqueFn<void(KeyEvent)>>                              key;
};

struct GlobalEventListeners
{
  stx::Vec<stx::UniqueFn<void(AudioDeviceEvent)>> audio_event;
  stx::Vec<stx::UniqueFn<void()>>                 system_theme;
};

}        // namespace ash
