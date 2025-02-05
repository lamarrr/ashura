/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/text.h"
#include "ashura/std/enum.h"
#include "ashura/std/math.h"
#include "ashura/std/result.h"
#include "ashura/std/time.h"
#include "ashura/std/types.h"
#include "ashura/std/vec.h"

namespace ash
{

enum class SystemTheme : u32
{
  Unknown = 0,
  Light   = 1,
  Dark    = 2
};

enum class KeyAction : u32
{
  Press   = 0,
  Release = 1
};

enum class KeyModifiers : u32
{
  None       = 0x0000,
  LeftShift  = 0x0001,
  RightShift = 0x0002,
  LeftCtrl   = 0x0004,
  RightCtrl  = 0x0008,
  LeftAlt    = 0x0010,
  RightAlt   = 0x0020,
  LeftWin    = 0x0040,
  RightWin   = 0x0080,
  Num        = 0x0100,
  Caps       = 0x0200,
  AltGr      = 0x0400,
  ScrollLock = 0x0800,
  Ctrl       = 0x1000,
  Shift      = 0x2000,
  Alt        = 0x4000,
  Gui        = 0x8000,
  All        = 0xFFFF
};

ASH_BIT_ENUM_OPS(KeyModifiers)

/// Scan Codes vs Key Codes:
/// https://learn.microsoft.com/en-us/windows/win32/inputdev/about-keyboard-input?redirectedfrom=MSDN#_win32_Keyboard_Input_Model
enum class ScanCode : u32
{
  None               = 0,
  Return             = '\r',
  Escape             = '\x1B',
  Backspace          = '\b',
  Tab                = '\t',
  Space              = ' ',
  Exclaim            = '!',
  QuoteDbl           = '"',
  Hash               = '#',
  Percent            = '%',
  Dollar             = '$',
  Ampersand          = '&',
  Quote              = '\'',
  LeftParen          = '(',
  RightParen         = ')',
  Asterisk           = '*',
  Plus               = '+',
  Comma              = ',',
  Minus              = '-',
  Period             = '.',
  Slash              = '/',
  Num0               = '0',
  Num1               = '1',
  Num2               = '2',
  Num3               = '3',
  Num4               = '4',
  Num5               = '5',
  Num6               = '6',
  Num7               = '7',
  Num8               = '8',
  Num9               = '9',
  Colon              = ':',
  SemiColon          = ';',
  Less               = '<',
  Equals             = '=',
  Greater            = '>',
  Question           = '?',
  At                 = '@',
  LeftBracket        = '[',
  BackSlash          = '\\',
  RightBracket       = ']',
  Caret              = '^',
  Underscore         = '_',
  BackQuote          = '`',
  A                  = 'a',
  B                  = 'b',
  C                  = 'c',
  D                  = 'd',
  E                  = 'e',
  F                  = 'f',
  G                  = 'g',
  H                  = 'h',
  I                  = 'i',
  J                  = 'j',
  K                  = 'k',
  L                  = 'l',
  M                  = 'm',
  N                  = 'n',
  O                  = 'o',
  P                  = 'p',
  Q                  = 'q',
  R                  = 'r',
  S                  = 's',
  T                  = 't',
  U                  = 'u',
  V                  = 'v',
  W                  = 'w',
  X                  = 'x',
  Y                  = 'y',
  Z                  = 'z',
  CapsLock           = 57,
  F1                 = 58,
  F2                 = 59,
  F3                 = 60,
  F4                 = 61,
  F5                 = 62,
  F6                 = 63,
  F7                 = 64,
  F8                 = 65,
  F9                 = 66,
  F10                = 67,
  F11                = 68,
  F12                = 69,
  PrintScreen        = 70,
  ScrollLock         = 71,
  Pause              = 72,
  Insert             = 73,
  Home               = 74,
  PageUp             = 75,
  Delete             = 76,
  End                = 77,
  PageDown           = 78,
  Right              = 79,
  Left               = 80,
  Down               = 81,
  Up                 = 82,
  NumLockClear       = 83,
  KP_Divide          = 84,
  KP_Multiply        = 85,
  KP_Minus           = 86,
  KP_Plus            = 87,
  KP_Enter           = 88,
  KP_1               = 89,
  KP_2               = 90,
  KP_3               = 91,
  KP_4               = 92,
  KP_5               = 93,
  KP_6               = 94,
  KP_7               = 95,
  KP_8               = 96,
  KP_9               = 97,
  KP_0               = 98,
  KP_Period          = 99,
  Application        = 101,
  Power              = 102,
  KP_Equals          = 103,
  F13                = 104,
  F14                = 105,
  F15                = 106,
  F16                = 107,
  F17                = 108,
  F18                = 109,
  F19                = 110,
  F20                = 111,
  F21                = 112,
  F22                = 113,
  F23                = 114,
  F24                = 115,
  Execute            = 116,
  Help               = 117,
  Menu               = 118,
  Select             = 119,
  Stop               = 120,
  Again              = 121,
  Undo               = 122,
  Cut                = 123,
  Copy               = 124,
  Paste              = 125,
  Find               = 126,
  Mute               = 127,
  VolumeUp           = 128,
  VolumeDown         = 129,
  KP_Comma           = 133,
  KP_EqualsAs400     = 134,
  AltErase           = 153,
  SysReq             = 154,
  Cancel             = 155,
  Clear              = 156,
  Prior              = 157,
  Return2            = 158,
  Separator          = 159,
  Out                = 160,
  Oper               = 161,
  ClearAgain         = 162,
  CrSel              = 163,
  ExSel              = 164,
  KP_00              = 176,
  KP_000             = 177,
  ThousandsSeparator = 178,
  DecimalSeparator   = 179,
  CurrencyUnit       = 180,
  CurrencySubunit    = 181,
  KP_LeftParen       = 182,
  KP_RightParen      = 183,
  KP_LeftBrace       = 184,
  KP_RightBrace      = 185,
  KP_Tab             = 186,
  KP_BackSpace       = 187,
  KP_A               = 188,
  KP_B               = 189,
  KP_C               = 190,
  KP_D               = 191,
  KP_E               = 192,
  KP_F               = 193,
  KP_Xor             = 194,
  KP_Power           = 195,
  KP_Percent         = 196,
  KP_Less            = 197,
  KP_Greater         = 198,
  KP_Ampersand       = 199,
  KP_DblAmpersanD    = 200,
  KP_VerticalBar     = 201,
  KP_DblVerticalBar  = 202,
  KP_Colon           = 203,
  KP_Hash            = 204,
  KP_Space           = 205,
  KP_At              = 206,
  KP_Exclam          = 207,
  KP_MemStore        = 208,
  KP_MemRecall       = 209,
  KP_MemClear        = 210,
  KP_MemAdd          = 211,
  KP_MemSubtract     = 212,
  KP_MemMultiply     = 213,
  KP_MemDivide       = 214,
  KP_PlusMinus       = 215,
  KP_Clear           = 216,
  KP_Clearentry      = 217,
  KP_Binary          = 218,
  KP_Octal           = 219,
  KP_Decimal         = 220,
  KP_HexaDecimal     = 221,
  LCtrl              = 224,
  LShift             = 225,
  LAlt               = 226,
  LGui               = 227,
  RCtrl              = 228,
  RShift             = 229,
  RAlt               = 230,
  RGui               = 231,
  Mode               = 257,
  AudioNext          = 258,
  AudioPrev          = 259,
  AudioStop          = 260,
  AudioPlay          = 261,
  AudioMute          = 262,
  MediaSelect        = 263,
  WWW                = 264,
  Mail               = 265,
  Calculator         = 266,
  Computer           = 267,
  AC_Search          = 268,
  AC_Home            = 269,
  AC_Back            = 270,
  AC_Forward         = 271,
  AC_Stop            = 272,
  AC_Refresh         = 273,
  AC_Bookmarks       = 274,
  BrightnessDown     = 275,
  BrightnessUp       = 276,
  DisplaySwitch      = 277,
  KbdillumToggle     = 278,
  KbdillumDown       = 279,
  KbdillumUp         = 280,
  Eject              = 281,
  Sleep              = 282,
  App1               = 283,
  App2               = 284,
  AudioRewind        = 285,
  AudioFastForward   = 286,
  SoftLeft           = 287,
  SoftRight          = 288,
  Call               = 289,
  EndCall            = 290
};

enum class KeyCode : u32
{
  None               = 0,
  Return             = '\r',
  Escape             = '\x1B',
  Backspace          = '\b',
  Tab                = '\t',
  Space              = ' ',
  Exclaim            = '!',
  QuoteDbl           = '"',
  Hash               = '#',
  Percent            = '%',
  Dollar             = '$',
  Ampersand          = '&',
  Quote              = '\'',
  LeftParen          = '(',
  RightParen         = ')',
  Asterisk           = '*',
  Plus               = '+',
  Comma              = ',',
  Minus              = '-',
  Period             = '.',
  Slash              = '/',
  Num0               = '0',
  Num1               = '1',
  Num2               = '2',
  Num3               = '3',
  Num4               = '4',
  Num5               = '5',
  Num6               = '6',
  Num7               = '7',
  Num8               = '8',
  Num9               = '9',
  Colon              = ':',
  SemiColon          = ';',
  Less               = '<',
  Equals             = '=',
  Greater            = '>',
  Question           = '?',
  At                 = '@',
  LeftBracket        = '[',
  BackSlash          = '\\',
  RightBracket       = ']',
  Caret              = '^',
  Underscore         = '_',
  BackQuote          = '`',
  A                  = 'a',
  B                  = 'b',
  C                  = 'c',
  D                  = 'd',
  E                  = 'e',
  F                  = 'f',
  G                  = 'g',
  H                  = 'h',
  I                  = 'i',
  J                  = 'j',
  K                  = 'k',
  L                  = 'l',
  M                  = 'm',
  N                  = 'n',
  O                  = 'o',
  P                  = 'p',
  Q                  = 'q',
  R                  = 'r',
  S                  = 's',
  T                  = 't',
  U                  = 'u',
  V                  = 'v',
  W                  = 'w',
  X                  = 'x',
  Y                  = 'y',
  Z                  = 'z',
  CapsLock           = 57,
  F1                 = 58,
  F2                 = 59,
  F3                 = 60,
  F4                 = 61,
  F5                 = 62,
  F6                 = 63,
  F7                 = 64,
  F8                 = 65,
  F9                 = 66,
  F10                = 67,
  F11                = 68,
  F12                = 69,
  PrintScreen        = 70,
  ScrollLock         = 71,
  Pause              = 72,
  Insert             = 73,
  Home               = 74,
  PageUp             = 75,
  Delete             = 76,
  End                = 77,
  PageDown           = 78,
  Right              = 79,
  Left               = 80,
  Down               = 81,
  Up                 = 82,
  NumLockClear       = 83,
  KP_Divide          = 84,
  KP_Multiply        = 85,
  KP_Minus           = 86,
  KP_Plus            = 87,
  KP_Enter           = 88,
  KP_1               = 89,
  KP_2               = 90,
  KP_3               = 91,
  KP_4               = 92,
  KP_5               = 93,
  KP_6               = 94,
  KP_7               = 95,
  KP_8               = 96,
  KP_9               = 97,
  KP_0               = 98,
  KP_Period          = 99,
  Application        = 101,
  Power              = 102,
  KP_Equals          = 103,
  F13                = 104,
  F14                = 105,
  F15                = 106,
  F16                = 107,
  F17                = 108,
  F18                = 109,
  F19                = 110,
  F20                = 111,
  F21                = 112,
  F22                = 113,
  F23                = 114,
  F24                = 115,
  Execute            = 116,
  Help               = 117,
  Menu               = 118,
  Select             = 119,
  Stop               = 120,
  Again              = 121,
  Undo               = 122,
  Cut                = 123,
  Copy               = 124,
  Paste              = 125,
  Find               = 126,
  Mute               = 127,
  VolumeUp           = 128,
  VolumeDown         = 129,
  KP_Comma           = 133,
  KP_EqualsAs400     = 134,
  AltErase           = 153,
  SysReq             = 154,
  Cancel             = 155,
  Clear              = 156,
  Prior              = 157,
  Return2            = 158,
  Separator          = 159,
  Out                = 160,
  Oper               = 161,
  ClearAgain         = 162,
  CrSel              = 163,
  ExSel              = 164,
  KP_00              = 176,
  KP_000             = 177,
  ThousandsSeparator = 178,
  DecimalSeparator   = 179,
  CurrencyUnit       = 180,
  CurrencySubunit    = 181,
  KP_LeftParen       = 182,
  KP_RightParen      = 183,
  KP_LeftBrace       = 184,
  KP_RightBrace      = 185,
  KP_Tab             = 186,
  KP_BackSpace       = 187,
  KP_A               = 188,
  KP_B               = 189,
  KP_C               = 190,
  KP_D               = 191,
  KP_E               = 192,
  KP_F               = 193,
  KP_Xor             = 194,
  KP_Power           = 195,
  KP_Percent         = 196,
  KP_Less            = 197,
  KP_Greater         = 198,
  KP_Ampersand       = 199,
  KP_DblAmpersanD    = 200,
  KP_VerticalBar     = 201,
  KP_DblVerticalBar  = 202,
  KP_Colon           = 203,
  KP_Hash            = 204,
  KP_Space           = 205,
  KP_At              = 206,
  KP_Exclam          = 207,
  KP_MemStore        = 208,
  KP_MemRecall       = 209,
  KP_MemClear        = 210,
  KP_MemAdd          = 211,
  KP_MemSubtract     = 212,
  KP_MemMultiply     = 213,
  KP_MemDivide       = 214,
  KP_PlusMinus       = 215,
  KP_Clear           = 216,
  KP_Clearentry      = 217,
  KP_Binary          = 218,
  KP_Octal           = 219,
  KP_Decimal         = 220,
  KP_HexaDecimal     = 221,
  LCtrl              = 224,
  LShift             = 225,
  LAlt               = 226,
  LGui               = 227,
  RCtrl              = 228,
  RShift             = 229,
  RAlt               = 230,
  RGui               = 231,
  Mode               = 257,
  AudioNext          = 258,
  AudioPrev          = 259,
  AudioStop          = 260,
  AudioPlay          = 261,
  AudioMute          = 262,
  MediaSelect        = 263,
  WWW                = 264,
  Mail               = 265,
  Calculator         = 266,
  Computer           = 267,
  AC_Search          = 268,
  AC_Home            = 269,
  AC_Back            = 270,
  AC_Forward         = 271,
  AC_Stop            = 272,
  AC_Refresh         = 273,
  AC_Bookmarks       = 274,
  BrightnessDown     = 275,
  BrightnessUp       = 276,
  DisplaySwitch      = 277,
  KbdillumToggle     = 278,
  KbdillumDown       = 279,
  KbdillumUp         = 280,
  Eject              = 281,
  Sleep              = 282,
  App1               = 283,
  App2               = 284,
  AudioRewind        = 285,
  AudioFastForward   = 286,
  SoftLeft           = 287,
  SoftRight          = 288,
  Call               = 289,
  EndCall            = 290
};

inline constexpr usize NUM_KEYS = 512;

enum class MouseButton : u32
{
  Primary   = 0,
  Secondary = 1,
  Middle    = 2,
  A1        = 3,
  A2        = 4
};

inline constexpr usize NUM_MOUSE_BUTTONS = 5;

struct KeyEvent
{
  ScanCode     scan_code = ScanCode::None;
  KeyCode      key_code  = KeyCode::None;
  KeyModifiers modifiers = KeyModifiers::None;
  KeyAction    action    = KeyAction::Press;
};

struct MouseMotionEvent
{
  Vec2 position    = {};
  Vec2 translation = {};
};

struct MouseClickEvent
{
  Vec2        position = {};
  u32         clicks   = 0;
  MouseButton button   = MouseButton::Primary;
  KeyAction   action   = KeyAction::Press;
};

struct MouseWheelEvent
{
  Vec2 position    = {};
  Vec2 translation = {};
};

enum class WindowEventType : u32
{
  Shown            = 0,
  Hidden           = 1,
  Exposed          = 2,
  Moved            = 3,
  Resized          = 4,
  SurfaceResized   = 5,
  Minimized        = 6,
  Maximized        = 7,
  Restored         = 8,
  MouseEnter       = 9,
  MouseLeave       = 10,
  KeyboardFocusIn  = 11,
  KeyboardFocusOut = 12,
  CloseRequested   = 13,
  Occluded         = 14,
  EnterFullScreen  = 15,
  LeaveFullScreen  = 16,
  Destroyed        = 17
};

struct TextInputEvent
{
  Span<c8 const> text{};
};

enum class DropEventType : u32
{
  DropBegin    = 0,
  DropComplete = 1
};

struct DropPositionEvent
{
  Vec2 pos{};
};

struct DropFileEvent
{
  Span<char const> path{};
};

struct DropTextEvent
{
  Span<c8 const> text{};
};

using DropEvent =
  Enum<DropEventType, DropPositionEvent, DropFileEvent, DropTextEvent>;

using WindowEvent =
  Enum<KeyEvent, MouseMotionEvent, MouseClickEvent, MouseWheelEvent,
       TextInputEvent, WindowEventType, DropEvent>;

enum class SystemEventType : u32
{
  KeymapChanged            = 0,
  AudioDeviceAdded         = 1,
  AudioDeviceRemoved       = 2,
  AudioDeviceFormatChanged = 3,
  DisplayReoriented        = 4,
  DisplayAdded             = 5,
  DisplayRemoved           = 6,
  DisplayMoved             = 7,
  CameraAdded              = 8,
  CameraRemoved            = 9,
  CameraApproved           = 10,
  CameraDenied             = 11
};

using SystemEvent = Enum<SystemTheme, SystemEventType>;

enum class TextInputType : u32
{
  Text                  = 0,
  Number                = 1,
  Name                  = 2,
  Email                 = 3,
  Username              = 4,
  PasswordHidden        = 5,
  PasswordVisible       = 6,
  NumberPasswordHidden  = 7,
  NumberPasswordVisible = 8
};

enum class TextCapitalization : u32
{
  None      = 0,
  Sentences = 1,
  Words     = 2,
  Letters   = 3
};

struct TextInputInfo
{
  TextInputType type = TextInputType::Text;

  bool multiline = false;

  /// @brief can receive `Tab` key as input
  bool esc_input = false;

  /// @brief can receive `Esc` key as input
  bool tab_input = false;

  TextCapitalization cap = TextCapitalization::None;

  bool autocorrect = false;
};

/// @param Normal region is normal and has no special properties
/// @param Draggable region can drag entire window
/// @param ResizeTopLeft region can resize top left window
/// @param ResizeTop region can resize top window
/// @param ResizeTopRight region can resize top right window
/// @param ResizeRight region can resize right window
/// @param ResizeBottomRight region can resize bottom right window
/// @param ResizeBottom region can resize bottom window
/// @param ResizeBottomLeft region can resize bottom left window
/// @param ResizeLeft region can resize left window
enum class WindowRegion : u32
{
  Normal            = 0,
  Draggable         = 1,
  ResizeTopLeft     = 2,
  ResizeTop         = 3,
  ResizeTopRight    = 4,
  ResizeRight       = 5,
  ResizeBottomRight = 6,
  ResizeBottom      = 7,
  ResizeBottomLeft  = 8,
  ResizeLeft        = 9
};

/// @param Default Default cursor. Usually an arrow
/// @param None No cursor (disable cursor).
/// @param Text Text selection. Usually an I-beam
/// @param Wait Wait. Usually an hourglass or watch or spinning ball.
/// @param CrossHair
/// @param Progress Program is busy but still interactive. Usually it's WAIT
/// with an arrow.
/// @param NWSEResize Double arrow pointing northwest and
/// southeast.
/// @param NESWResize  Double arrow pointing northeast and
/// southwest.
/// @param EWResize  Double arrow pointing west and east.
/// @param NSResize  Double arrow pointing north and south.
/// @param Move Four pointed arrow pointing north, south, east, and west.
/// @param NotAllowed Not permitted. Usually a slashed circle or crossbones.
/// @param Pointer Pointer that indicates a link. Usually a pointing hand.
/// @param NWResize Window resize top-left
/// @param NorthResize Window resize top
/// @param NEResize  Window resize top-right
/// @param EastResize Window resize right
/// @param SEResize resize bottom-right
/// @param SouthResize  Window resize bottom
/// @param SWResize Window resize bottom-left
/// @param WestResize  Window resize left
enum class Cursor : u32
{
  Default     = 0,
  Text        = 1,
  Wait        = 2,
  CrossHair   = 3,
  Progress    = 4,
  NWSEResize  = 5,
  NESWResize  = 6,
  EWResize    = 7,
  NSResize    = 8,
  Move        = 9,
  NotAllowed  = 10,
  Pointer     = 11,
  NWResize    = 12,
  NorthResize = 13,
  NEResize    = 14,
  EastResize  = 15,
  SEResize    = 16,
  SouthResize = 17,
  SWResize    = 18,
  WestResize  = 19
};

constexpr u32 NUM_CURSOR_TYPES = 20;

/// @brief default charset is ASCII
inline constexpr char const MIME_TEXT_PLAIN[]    = "text/plain";
inline constexpr char const MIME_TEXT_UTF8[]     = "text/plain;charset=UTF-8";
inline constexpr char const MIME_TEXT_CSS[]      = "text/css";
inline constexpr char const MIME_TEXT_CSV[]      = "text/csv";
inline constexpr char const MIME_TEXT_HTML[]     = "text/html";
inline constexpr char const MIME_TEXT_JS[]       = "text/javascript";
inline constexpr char const MIME_TEXT_MARKDOWN[] = "text/markdown";

inline constexpr char const MIME_IMAGE_AVIF[] = "image/avif";
inline constexpr char const MIME_IMAGE_BMP[]  = "image/bmp";
inline constexpr char const MIME_IMAGE_HEIF[] = "image/heif";
inline constexpr char const MIME_IMAGE_JPEG[] = "image/jpeg";
inline constexpr char const MIME_IMAGE_PNG[]  = "image/png";
inline constexpr char const MIME_IMAGE_SVG[]  = "image/svg+xml";
inline constexpr char const MIME_IMAGE_WEBP[] = "image/webp";

inline constexpr char const MIME_VIDEO_AV1[]      = "video/AV1";
inline constexpr char const MIME_VIDEO_H264[]     = "video/H264";
inline constexpr char const MIME_VIDEO_H265[]     = "video/H265";
inline constexpr char const MIME_VIDEO_H266[]     = "video/H266";
inline constexpr char const MIME_VIDEO_MATROSKA[] = "video/matroska";
inline constexpr char const MIME_VIDEO_MP4[]      = "video/mp4";
inline constexpr char const MIME_VIDEO_RAW[]      = "video/raw";
inline constexpr char const MIME_VIDEO_VP8[]      = "video/VP8";
inline constexpr char const MIME_VIDEO_VP9[]      = "video/VP9";

inline constexpr char const MIME_MODEL_GLTF_BINARY[] = "model/gltf+binary";
inline constexpr char const MIME_MODEL_GLTF_JSON[]   = "model/gltf+json";
inline constexpr char const MIME_MODEL_MESH[]        = "model/mesh";
inline constexpr char const MIME_MODEL_MTL[]         = "model/mtl";
inline constexpr char const MIME_MODEL_OBJ[]         = "model/obj";
inline constexpr char const MIME_MODEL_STL[]         = "model/stl";

inline constexpr char const MIME_FONT_OTF[]   = "font/otf";
inline constexpr char const MIME_FONT_SFNT[]  = "font/sfnt";
inline constexpr char const MIME_FONT_TTF[]   = "font/ttf";
inline constexpr char const MIME_FONT_WOFF[]  = "font/woff";
inline constexpr char const MIME_FONT_WOFF2[] = "font/woff2";

struct ClipBoard
{
  ClipBoard()                              = default;
  ClipBoard(ClipBoard const &)             = default;
  ClipBoard(ClipBoard &&)                  = default;
  ClipBoard & operator=(ClipBoard const &) = default;
  ClipBoard & operator=(ClipBoard &&)      = default;
  virtual ~ClipBoard()                     = default;

  virtual Result<> get(Span<char const> mime, Vec<u8> & out)
  {
    (void) mime;
    (void) out;
    return Err{};
  }

  virtual Result<> set(Span<char const> mime, Span<u8 const> data)
  {
    (void) mime;
    (void) data;
    return Err{};
  }

  Result<> get_text(Vec<u8> & out)
  {
    return get(MIME_TEXT_UTF8, out);
  }

  Result<> set_text(Span<u8 const> text)
  {
    return set(MIME_TEXT_UTF8, text);
  }
};

enum class DropType : u32
{
  None     = 0,
  FilePath = 1,
  Bytes    = 2
};

struct InputState
{
  struct Mouse
  {
    /// @brief did the mouse enter the window on this frame?
    bool in = false;

    /// @brief did the mouse leave the window on this frame?
    bool out = false;

    /// @brief did the mouse move on this frame?
    bool moved = false;

    /// @brief did the mouse wheel get scrolled on this frame?
    bool wheel_scrolled = false;

    /// @brief is any of the keys pressed on this frame
    bool any_down = false;

    /// @brief is any of the keys released on this frame
    bool any_up = false;

    /// @brief which mouse buttons were pressed on this frame
    Bits<u64, NUM_MOUSE_BUTTONS> downs{};

    /// @brief which mouse buttons were released on this frame
    Bits<u64, NUM_MOUSE_BUTTONS> ups{};

    /// @brief the current state of each mouse button
    Bits<u64, NUM_MOUSE_BUTTONS> states{};

    /// @brief number of times the mouse was clicked so far
    u32 num_clicks[NUM_MOUSE_BUTTONS]{};

    /// @brief the position of the mouse on this frame
    Vec2 position = {};

    /// @brief translation of the mouse on this frame
    Vec2 translation = {};

    /// @brief translation of the mouse wheel on this frame
    Vec2 wheel_translation = {};

    void clear()
    {
      in             = false;
      out            = false;
      moved          = false;
      wheel_scrolled = false;
      any_down       = false;
      any_up         = false;
      fill(downs, 0ULL);
      fill(ups, 0ULL);
      fill(states, 0ULL);
      fill(num_clicks, 0ULL);
      position          = {};
      translation       = {};
      wheel_translation = {};
    }
  };

  struct Keyboard
  {
    /// @brief did the window gain keyboard focus on this frame?
    bool in = false;

    /// @brief did the window lose keyboard focus on this frame?
    bool out = false;

    /// @brief is any of the keys pressed on this frame
    bool any_down = false;

    /// @brief is any of the keys released on this frame
    bool any_up = false;

    /// @brief bit mask of all the keys that were pressed on this frame
    Bits<u64, NUM_KEYS> downs{};

    /// @brief bit mask of all the keys that were released on this frame
    Bits<u64, NUM_KEYS> ups{};

    /// @brief bit mask of all the key states
    Bits<u64, NUM_KEYS> states{};

    /// @brief bit mask of all the keys that were pressed on this frame, indexed using the scancode
    Bits<u64, NUM_KEYS> scan_downs{};

    /// @brief bit mask of all the keys that were released on this frame, indexed using the scancode
    Bits<u64, NUM_KEYS> scan_ups{};

    /// @brief bit mask of all the key states, indexed using the scancode
    Bits<u64, NUM_KEYS> scan_states{};

    /// @brief hold state of the key modifiers on this frame
    KeyModifiers modifiers = KeyModifiers::None;

    void clear()
    {
      in       = false;
      out      = false;
      any_down = false;
      any_up   = false;
      fill(downs, 0ULL);
      fill(ups, 0ULL);
      fill(states, 0ULL);
      fill(scan_downs, 0ULL);
      fill(scan_ups, 0ULL);
      fill(scan_states, 0ULL);
      modifiers = KeyModifiers::None;
    }
  };

  /// @brief timestamp of current frame
  time_point timestamp = {};

  /// @brief time elapsed between previous and current frame
  nanoseconds timedelta = {};

  /// @brief the current theme gotten from the window manager
  SystemTheme theme = SystemTheme::Unknown;

  /// @brief the preferred text direction of the host system
  TextDirection direction = TextDirection::LeftToRight;

  /// @brief current window mouse focus state
  bool mouse_focused = false;

  /// @brief current window keyboard focus state
  bool key_focused = false;

  /// @brief windows' current frame mouse state
  Mouse mouse{};

  /// @brief windows' current frame keyboard state
  Keyboard key{};

  /// @brief extent of the viewport the windows' views are in
  Vec2U window_extent = {};

  /// @brief then windows' backing surface extent
  Vec2U surface_extent = {};

  /// @brief current drop data type
  DropType drop_type = DropType::None;

  /// @brief drag data associated with the current drag operation (if any, otherwise empty)
  Vec<u8> drop_data{};

  /// @brief if a text input came in
  bool text_input = false;

  /// @brief current text input data from the IME or keyboard
  Vec<c8> text{};

  /// @brief is the application requested to close
  bool close_requested = false;

  /// @brief did a window resize happen
  bool resized = true;

  /// @brief did a window surface resize happen
  bool surface_resized = true;

  bool dropped = false;

  bool drop_hovering = false;

  Cursor cursor = Cursor::Default;

  explicit InputState(AllocatorRef allocator) :
    drop_data{allocator},
    text{allocator}
  {
  }

  InputState(InputState const &)             = delete;
  InputState & operator=(InputState const &) = delete;
  InputState(InputState &&)                  = default;
  InputState & operator=(InputState &&)      = default;
  ~InputState()                              = default;

  void stamp(time_point time, nanoseconds delta)
  {
    timestamp = time;
    timedelta = delta;
  }

  void clear()
  {
    mouse.clear();
    key.clear();
    text_input = false;
    text.clear();
    resized         = false;
    surface_resized = false;

    // if the there was a data drop on the last frame clear the buffer
    if (dropped)
    {
      drop_data.clear();
      drop_type = DropType::None;
    }

    dropped       = false;
    drop_hovering = false;
  }

  void clone_to(InputState & dst) const
  {
    dst.clear();
    dst.timestamp      = timestamp;
    dst.timedelta      = timedelta;
    dst.theme          = theme;
    dst.direction      = direction;
    dst.mouse_focused  = mouse_focused;
    dst.key_focused    = key_focused;
    dst.mouse          = mouse;
    dst.key            = key;
    dst.window_extent  = window_extent;
    dst.surface_extent = surface_extent;
    dst.drop_type      = drop_type;
    dst.drop_data.extend(drop_data).unwrap();
    dst.text_input = text_input;
    dst.text.extend(text).unwrap();
    dst.close_requested = close_requested;
    dst.resized         = resized;
    dst.surface_resized = resized;
    dst.dropped         = dropped;
    dst.drop_hovering   = drop_hovering;
    dst.cursor          = cursor;
  }

  constexpr bool key_down(KeyCode k) const
  {
    return get_bit(key.downs, (usize) k);
  }

  constexpr bool key_up(KeyCode k) const
  {
    return get_bit(key.ups, (usize) k);
  }

  constexpr bool key_state(KeyCode k) const
  {
    return get_bit(key.states, (usize) k);
  }

  constexpr bool key_down(ScanCode k) const
  {
    return get_bit(key.scan_downs, (usize) k);
  }

  constexpr bool key_up(ScanCode k) const
  {
    return get_bit(key.scan_ups, (usize) k);
  }

  constexpr bool key_state(ScanCode k) const
  {
    return get_bit(key.scan_states, (usize) k);
  }

  constexpr bool mouse_down(MouseButton btn) const
  {
    return get_bit(mouse.downs, (u32) btn);
  }

  constexpr bool mouse_up(MouseButton btn) const
  {
    return get_bit(mouse.ups, (u32) btn);
  }

  constexpr bool mouse_state(MouseButton btn) const
  {
    return get_bit(mouse.states, (u32) btn);
  }
};

}    // namespace ash
