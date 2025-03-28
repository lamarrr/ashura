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
  LeftShift  = 0x0001U,    /// the left Shift key is down.
  RightShift = 0x0002U,    /// the right Shift key is down.
  Level5     = 0x0004U,    /// the Level 5 Shift key is down.
  LeftCtrl   = 0x0040U,    /// the left Ctrl (Control) key is down.
  RightCtrl  = 0x0080U,    /// the right Ctrl (Control) key is down.
  LeftAlt    = 0x0100U,    /// the left Alt key is down.
  RightAlt   = 0x0200U,    /// the right Alt key is down.
  LeftGui    = 0x0400U,    /// the left GUI key (often the Windows key) is down.
  RightGui = 0x0800U,    /// the right GUI key (often the Windows key) is down.
  Num      = 0x1000U,    /// the Num Lock key (may be located on an
                         /// extended keypad) is down.
  Caps     = 0x2000U,    /// the Caps Lock key is down.
  AltGr    = 0x4000U,    /// the !AltGr/Mode key is down.
  ScrollLock = 0x8000U,    /// the Scroll Lock key is down.
  All        = 0xFFFFU
};

ASH_BIT_ENUM_OPS(KeyModifiers)

/// Scan Codes vs Key Codes:
/// https://learn.microsoft.com/en-us/windows/win32/inputdev/about-keyboard-input?redirectedfrom=MSDN#_win32_Keyboard_Input_Model

///
///@brief scancode is the physical representation of a key on the keyboard,
/// independent of language and keyboard mapping.
///
///
/// The values in this enumeration are based on the USB usage page standard:
/// https://usb.org/sites/default/files/hut1_5.pdf
enum class ScanCode : u32
{
  Unknown = 0U,
  A       = 4U,
  B       = 5U,
  C       = 6U,
  D       = 7U,
  E       = 8U,
  F       = 9U,
  G       = 10U,
  H       = 11U,
  I       = 12U,
  J       = 13U,
  K       = 14U,
  L       = 15U,
  M       = 16U,
  N       = 17U,
  O       = 18U,
  P       = 19U,
  Q       = 20U,
  R       = 21U,
  S       = 22U,
  T       = 23U,
  U       = 24U,
  V       = 25U,
  W       = 26U,
  X       = 27U,
  Y       = 28U,
  Z       = 29U,

  Num1 = 30U,
  Num2 = 31U,
  Num3 = 32U,
  Num4 = 33U,
  Num5 = 34U,
  Num6 = 35U,
  Num7 = 36U,
  Num8 = 37U,
  Num9 = 38U,
  Num0 = 39U,

  Return    = 40U,
  Escape    = 41U,
  Backspace = 42U,
  Tab       = 43U,
  Space     = 44U,

  Minus        = 45U,
  Equals       = 46U,
  LeftBracket  = 47U,
  RightBracket = 48U,
  Backslash    = 49U,
  Nonushash    = 50U,
  SemiColon    = 51U,
  Apostrophe   = 52U,
  Grave        = 53U,
  Comma        = 54U,
  Period       = 55U,
  Slash        = 56U,

  CAPSLOCK = 57U,

  F1  = 58U,
  F2  = 59U,
  F3  = 60U,
  F4  = 61U,
  F5  = 62U,
  F6  = 63U,
  F7  = 64U,
  F8  = 65U,
  F9  = 66U,
  F10 = 67U,
  F11 = 68U,
  F12 = 69U,

  PrintScreen = 70U,
  ScrollLock  = 71U,
  Pause       = 72U,
  Insert      = 73U,
  Home        = 74U,
  Pageup      = 75U,
  Delete      = 76U,
  End         = 77U,
  PageDown    = 78U,
  Right       = 79U,
  Left        = 80U,
  Down        = 81U,
  Up          = 82U,

  NumlockClear = 83U,
  KpDivide     = 84U,
  KpMultiply   = 85U,
  KpMinus      = 86U,
  KpPlus       = 87U,
  KpEnter      = 88U,
  Kp1          = 89U,
  Kp2          = 90U,
  Kp3          = 91U,
  Kp4          = 92U,
  Kp5          = 93U,
  Kp6          = 94U,
  Kp7          = 95U,
  Kp8          = 96U,
  Kp9          = 97U,
  Kp0          = 98U,
  KpPeriod     = 99U,

  NonusBackslash = 100U,
  Application    = 101U,
  Power          = 102U,
  KpEquals       = 103U,
  F13            = 104U,
  F14            = 105U,
  F15            = 106U,
  F16            = 107U,
  F17            = 108U,
  F18            = 109U,
  F19            = 110U,
  F20            = 111U,
  F21            = 112U,
  F22            = 113U,
  F23            = 114U,
  F24            = 115U,
  Execute        = 116U,
  Help           = 117U,
  Menu           = 118U,
  Select         = 119U,
  Stop           = 120U,
  Again          = 121U,
  Undo           = 122U,
  Cut            = 123U,
  Copy           = 124U,
  Paste          = 125U,
  Find           = 126U,
  Mute           = 127U,
  VolumeUp       = 128U,
  VolumeDown     = 129U,

  KpComma       = 133U,
  KpEqualsAs400 = 134U,

  International1 = 135U,
  International2 = 136U,
  International3 = 137U,
  International4 = 138U,
  International5 = 139U,
  International6 = 140U,
  International7 = 141U,
  International8 = 142U,
  International9 = 143U,
  Lang1          = 144U,
  Lang2          = 145U,
  Lang3          = 146U,
  Lang4          = 147U,
  Lang5          = 148U,
  Lang6          = 149U,
  Lang7          = 150U,
  Lang8          = 151U,
  Lang9          = 152U,

  AltErase   = 153U,
  SysReq     = 154U,
  Cancel     = 155U,
  Clear      = 156U,
  Prior      = 157U,
  Return2    = 158U,
  Separator  = 159U,
  Out        = 160U,
  Oper       = 161U,
  ClearAgain = 162U,
  Crsel      = 163U,
  Exsel      = 164U,

  Kp00               = 176U,
  Kp000              = 177U,
  ThousandsSeparator = 178U,
  DecimalSeparator   = 179U,
  CurrencyUnit       = 180U,
  CurrencySubunit    = 181U,
  KpLeftParen        = 182U,
  KpRightParen       = 183U,
  KpLeftBrace        = 184U,
  KpRightBrace       = 185U,
  KpTab              = 186U,
  KpBackSpace        = 187U,
  KpA                = 188U,
  KpB                = 189U,
  KpC                = 190U,
  KpD                = 191U,
  KpE                = 192U,
  KpF                = 193U,
  KpXor              = 194U,
  KpPower            = 195U,
  KpPercent          = 196U,
  KpLess             = 197U,
  KpGreater          = 198U,
  KpAmpersand        = 199U,
  KpDblAmpersand     = 200U,
  KpVerticalBar      = 201U,
  KpDblverticalBar   = 202U,
  KpColon            = 203U,
  KpHash             = 204U,
  KpSpace            = 205U,
  KpAt               = 206U,
  KpExclam           = 207U,
  KpMemStore         = 208U,
  KpMemRecall        = 209U,
  KpMemClear         = 210U,
  KpMemAdd           = 211U,
  KpMemSubtract      = 212U,
  KpMemMultiply      = 213U,
  KpMemDivide        = 214U,
  KpPlusMinus        = 215U,
  KpClear            = 216U,
  KpClearEntry       = 217U,
  KpBinary           = 218U,
  KpOctal            = 219U,
  KpDecimal          = 220U,
  KpHexadecimal      = 221U,

  LeftCtrl   = 224U,
  LeftShift  = 225U,
  LeftAlt    = 226U,
  LeftGui    = 227U,
  RightCtrl  = 228U,
  RightShift = 229U,
  RAlt       = 230U,
  RGui       = 231U,

  Mode = 257U,

  Sleep = 258U,
  Wake  = 259U,

  ChannelIncrement = 260U,
  ChannelDecrement = 261U,

  MediaPlay          = 262U,
  MediaPause         = 263U,
  MediaRecord        = 264U,
  MediaFastForward   = 265U,
  MediaRewind        = 266U,
  MediaNextTrack     = 267U,
  MediaPreviousTrack = 268U,
  MediaStop          = 269U,
  MediaEject         = 270U,
  MediaPlayPause     = 271U,
  MediaSelect        = 272U,

  ACNew        = 273U,
  ACOpen       = 274U,
  ACClose      = 275U,
  ACExit       = 276U,
  ACSave       = 277U,
  ACPrint      = 278U,
  ACProperties = 279U,

  ACSearch    = 280U,
  ACHome      = 281U,
  ACBack      = 282U,
  ACForward   = 283U,
  ACStop      = 284U,
  ACRefresh   = 285U,
  ACBookmarks = 286U,

  SoftLeft  = 287U,
  SoftRight = 288U,
  Call      = 289U,
  EndCall   = 290U,

  Reserved = 400U
};

inline constexpr u32 NUM_SCAN_CODES = 512;

/// @brief Values of this type are used to represent keyboard keys using the current
/// layout of the keyboard. These values include Unicode values representing
/// the unmodified character that would be generated by pressing the key, or an
/// `SDLK_*` constant for those keys that do not generate characters.
///
/// A special exception is the number keys at the top of the keyboard which map
/// to SDLK_0...SDLK_9 on AZERTY layouts.
///
/// Keys with the `SDLK_EXTENDED_MASK` bit set do not map to a scancode or
/// unicode code point.
enum class KeyCode : u32
{
  Unknown = 0,

  Backspace = '\b',
  Tab       = '\t',
  Return    = '\r',
  Escape    = '\x1B',

  Space      = ' ',
  Exclaim    = '!',
  QuoteDbl   = '"',
  Hash       = '#',
  Dollar     = '$',
  Percent    = '%',
  Ampersand  = '&',
  Quote      = '\'',
  LeftParen  = '(',
  RightParen = ')',
  Asterisk   = '*',
  Plus       = '+',
  Comma      = ',',
  Minus      = '-',
  Period     = '.',
  Slash      = '/',

  Num0      = '0',
  Num1      = '1',
  Num2      = '2',
  Num3      = '3',
  Num4      = '4',
  Num5      = '5',
  Num6      = '6',
  Num7      = '7',
  Num8      = '8',
  Num9      = '9',
  Colon     = ':',
  SemiColon = ';',
  Less      = '<',
  Equals    = '=',
  Greater   = '>',
  Question  = '?',
  At        = '@',

  LeftBracket  = '[',
  BackSlash    = '\\',
  RightBracket = ']',
  Caret        = '^',
  Underscore   = '_',
  BackQuote    = '`',

  A = 'a',
  B = 'b',
  C = 'c',
  D = 'd',
  E = 'e',
  F = 'f',
  G = 'g',
  H = 'h',
  I = 'i',
  J = 'j',
  K = 'k',
  L = 'l',
  M = 'm',
  N = 'n',
  O = 'o',
  P = 'p',
  Q = 'q',
  R = 'r',
  S = 's',
  T = 't',
  U = 'u',
  V = 'v',
  W = 'w',
  X = 'x',
  Y = 'y',
  Z = 'z',

  LeftBrace  = '{',
  Pipe       = '|',
  RightBrace = '}',
  Tilde      = '~',
  Delete     = 0x7FU,

  PlusMinus = 0xB1U,

  CapsLock           = 256,
  F1                 = 257,
  F2                 = 258,
  F3                 = 259,
  F4                 = 260,
  F5                 = 261,
  F6                 = 262,
  F7                 = 263,
  F8                 = 264,
  F9                 = 265,
  F10                = 266,
  F11                = 267,
  F12                = 268,
  PrintScreen        = 269,
  ScrollLock         = 270,
  Pause              = 271,
  Insert             = 272,
  Home               = 273,
  PageUp             = 274,
  End                = 275,
  PageDown           = 276,
  Right              = 277,
  Left               = 278,
  Down               = 279,
  Up                 = 280,
  NumLockClear       = 281,
  KpDivide           = 282,
  KpMultiply         = 283,
  KpMinus            = 284,
  KpPlus             = 285,
  KpEnter            = 286,
  Kp1                = 287,
  Kp2                = 288,
  Kp3                = 289,
  Kp4                = 290,
  Kp5                = 291,
  Kp6                = 292,
  Kp7                = 293,
  Kp8                = 294,
  Kp9                = 295,
  Kp0                = 296,
  KpPeriod           = 297,
  Application        = 298,
  Power              = 299,
  KpEquals           = 300,
  F13                = 301,
  F14                = 302,
  F15                = 303,
  F16                = 304,
  F17                = 305,
  F18                = 306,
  F19                = 307,
  F20                = 308,
  F21                = 309,
  F22                = 310,
  F23                = 311,
  F24                = 312,
  Execute            = 313,
  Help               = 314,
  Menu               = 315,
  Select             = 316,
  Stop               = 317,
  Again              = 318,
  Undo               = 319,
  Cut                = 320,
  Copy               = 321,
  Paste              = 322,
  Find               = 323,
  Mute               = 324,
  VolumeUp           = 325,
  VolumeDown         = 326,
  KpComma            = 327,
  KpEqualsAs400      = 328,
  AltErase           = 329,
  SysReq             = 330,
  Cancel             = 331,
  Clear              = 332,
  Prior              = 333,
  Return2            = 334,
  Separator          = 335,
  Out                = 336,
  Oper               = 337,
  ClearAgain         = 338,
  CrSel              = 339,
  ExSel              = 340,
  Kp00               = 341,
  Kp000              = 342,
  ThousandsSeparator = 343,
  DecimalSeparator   = 344,
  CurrencyUnit       = 345,
  CurrencySubUnit    = 346,
  KpLeftParen        = 347,
  KpRightParen       = 348,
  KpLeftBrace        = 349,
  KpRightBrace       = 350,
  KpTab              = 351,
  KpBackSpace        = 352,
  KpA                = 353,
  KpB                = 354,
  KpC                = 355,
  KpD                = 356,
  KpE                = 357,
  KpF                = 358,
  KpXor              = 359,
  KpPower            = 360,
  KpPercent          = 361,
  KpLess             = 362,
  KpGreater          = 363,
  KpAmpersand        = 364,
  KpDblAmpersand     = 365,
  KpVerticalBar      = 366,
  KpDblverticalBar   = 367,
  KpColon            = 368,
  KpHash             = 369,
  KpSpace            = 370,
  KpAt               = 371,
  KpExclam           = 372,
  KpMemStore         = 373,
  KpMemRecall        = 374,
  KpMemClear         = 375,
  KpMemAdd           = 376,
  KpMemSubtract      = 377,
  KpMemMultiply      = 378,
  KpMemDivide        = 379,
  KpPlusMinus        = 380,
  KpClear            = 381,
  KpClearentry       = 382,
  KpBinary           = 383,
  KpOctal            = 384,
  KpDecimal          = 385,
  KpHexaDecimal      = 386,
  LeftCtrl           = 387,
  LeftShift          = 388,
  LeftAlt            = 389,
  LeftGui            = 390,
  RightCtrl          = 391,
  RightShift         = 392,
  RightAlt           = 393,
  RightGui           = 394,
  Mode               = 395,
  Sleep              = 396,
  Wake               = 397,
  ChannelIncrement   = 398,
  ChannelDecrement   = 399,
  MediaPlay          = 400,
  MediaPause         = 401,
  MediaRecord        = 402,
  MediaFastForward   = 403,
  MediaRewind        = 404,
  MediaNextTrack     = 405,
  MediaPreviousTrack = 406,
  MediaStop          = 407,
  MediaEject         = 408,
  MediaPlayPause     = 409,
  MediaSelect        = 410,
  AcNew              = 411,
  AcOpen             = 412,
  AcClose            = 413,
  AcExit             = 414,
  AcSave             = 415,
  AcPrint            = 416,
  AcProperties       = 417,
  AcSearch           = 418,
  AcHome             = 419,
  AcBack             = 420,
  AcForward          = 421,
  AcStop             = 422,
  AcRefresh          = 423,
  AcBookmarks        = 424,
  SoftLeft           = 425,
  SoftRight          = 426,
  Call               = 427,
  EndCall            = 428,
  LeftTab            = 429,
  Level5Shift        = 430,
  MultiKeyCompose    = 431,
  LMeta              = 432,
  RMeta              = 433,
  LHyper             = 434,
  RHyper             = 435
};

inline constexpr u32 NUM_KEY_CODES = 512;

enum class MouseButton : u32
{
  Primary   = 0,
  Secondary = 1,
  Middle    = 2,
  A1        = 3,
  A2        = 4
};

inline constexpr usize NUM_MOUSE_BUTTONS = 5;

enum class MouseButtons : u32
{
  None      = 0,
  Primary   = 1 << 0,
  Secondary = 1 << 1,
  Middle    = 1 << 2,
  A1        = 1 << 3,
  A2        = 1 << 4,
  All       = 0xFF
};

ASH_BIT_ENUM_OPS(MouseButtons)

struct KeyEvent
{
  ScanCode     scan_code = ScanCode::Unknown;
  KeyCode      key_code  = KeyCode::Unknown;
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
  Str8 text{};
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
  Str path{};
};

struct DropTextEvent
{
  Str8 text{};
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

  virtual Result<> get(Str mime, Vec<u8> & out)
  {
    (void) mime;
    (void) out;
    return Err{};
  }

  virtual Result<> set(Str mime, Span<u8 const> data)
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
    MouseButtons downs{};

    /// @brief which mouse buttons were released on this frame
    MouseButtons ups{};

    /// @brief the current state of each mouse button
    MouseButtons states{};

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
      downs          = MouseButtons::None;
      ups            = MouseButtons::None;
      states         = MouseButtons::None;
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
    Bits<u64, NUM_KEY_CODES> key_downs{};

    /// @brief bit mask of all the keys that were released on this frame
    Bits<u64, NUM_KEY_CODES> key_ups{};

    /// @brief bit mask of all the key states
    Bits<u64, NUM_KEY_CODES> key_states{};

    /// @brief bit mask of all the keys that were pressed on this frame, indexed using the scancode
    Bits<u64, NUM_SCAN_CODES> scan_downs{};

    /// @brief bit mask of all the keys that were released on this frame, indexed using the scancode
    Bits<u64, NUM_SCAN_CODES> scan_ups{};

    /// @brief bit mask of all the key states, indexed using the scancode
    Bits<u64, NUM_SCAN_CODES> scan_states{};

    /// @brief hold state of the key modifiers on this frame
    KeyModifiers modifier_downs = KeyModifiers::None;

    KeyModifiers modifier_ups = KeyModifiers::None;

    KeyModifiers modifier_states = KeyModifiers::None;

    void clear()
    {
      in       = false;
      out      = false;
      any_down = false;
      any_up   = false;
      any_up   = false;
      fill(key_downs, 0ULL);
      fill(key_ups, 0ULL);
      fill(key_states, 0ULL);
      fill(scan_downs, 0ULL);
      fill(scan_ups, 0ULL);
      fill(scan_states, 0ULL);
      modifier_downs  = KeyModifiers::None;
      modifier_ups    = KeyModifiers::None;
      modifier_states = KeyModifiers::None;
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

  /// @brief the theme changed
  bool theme_changed = false;

  /// @brief current text input data from the IME or keyboard
  Vec<c8> text{};

  /// @brief is the application requested to close
  bool close_requested = false;

  /// @brief is the application closing
  bool closing = false;

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
    text_input    = false;
    theme_changed = false;
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
    dst.text_input    = text_input;
    dst.theme_changed = theme_changed;
    dst.text.extend(text).unwrap();
    dst.close_requested = close_requested;
    dst.closing         = closing;
    dst.resized         = resized;
    dst.surface_resized = resized;
    dst.dropped         = dropped;
    dst.drop_hovering   = drop_hovering;
    dst.cursor          = cursor;
  }

  constexpr bool key_down(KeyCode k) const
  {
    return get_bit(key.key_downs, (usize) k);
  }

  constexpr bool key_up(KeyCode k) const
  {
    return get_bit(key.key_ups, (usize) k);
  }

  constexpr bool key_state(KeyCode k) const
  {
    return get_bit(key.key_states, (usize) k);
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

  constexpr bool modifier_down(KeyModifiers mods) const
  {
    return has_bits(key.modifier_downs, mods);
  }

  constexpr bool modifier_up(KeyModifiers mods) const
  {
    return has_bits(key.modifier_ups, mods);
  }

  constexpr bool modifier_state(KeyModifiers mods) const
  {
    return has_bits(key.modifier_states, mods);
  }

  constexpr bool mouse_down(MouseButton btn) const
  {
    return has_bits(mouse.downs, MouseButtons{1U << (usize) btn});
  }

  constexpr bool mouse_up(MouseButton btn) const
  {
    return has_bits(mouse.ups, MouseButtons{1U << (usize) btn});
  }

  constexpr bool mouse_state(MouseButton btn) const
  {
    return has_bits(mouse.states, MouseButtons{1U << (usize) btn});
  }

  constexpr u32 mouse_clicks(MouseButton btn) const
  {
    return mouse.num_clicks[(usize) btn];
  }
};

}    // namespace ash
