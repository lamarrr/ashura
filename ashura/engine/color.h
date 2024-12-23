/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/math.h"
#include "ashura/std/types.h"

namespace ash
{

namespace colors
{
inline constexpr Vec4U8 NONE    = Vec4U8{0x00, 0x00, 0x00, 0x00};
inline constexpr Vec4U8 WHITE   = Vec4U8{0xFF, 0xFF, 0xFF, 0xFF};
inline constexpr Vec4U8 BLACK   = Vec4U8{0x00, 0x00, 0x00, 0xFF};
inline constexpr Vec4U8 RED     = Vec4U8{0xFF, 0x00, 0x00, 0xFF};
inline constexpr Vec4U8 BLUE    = Vec4U8{0x00, 0x00, 0xFF, 0xFF};
inline constexpr Vec4U8 GREEN   = Vec4U8{0x00, 0xFF, 0x00, 0xFF};
inline constexpr Vec4U8 CYAN    = Vec4U8{0x00, 0xFF, 0xFF, 0xFF};
inline constexpr Vec4U8 MAGENTA = Vec4U8{0xFF, 0x00, 0xFF, 0xFF};
inline constexpr Vec4U8 YELLOW  = Vec4U8{0xFF, 0xFF, 0x00, 0xFF};
}    // namespace colors

// ios default system colors
namespace ios
{
inline constexpr Vec4U8 LIGHT_BLUE   = Vec4U8{0, 122, 0xFF, 0xFF};
inline constexpr Vec4U8 DARK_BLUE    = Vec4U8{10, 132, 0xFF, 0xFF};
inline constexpr Vec4U8 LIGHT_BROWN  = Vec4U8{162, 132, 94, 0xFF};
inline constexpr Vec4U8 DARK_BROWN   = Vec4U8{172, 142, 104, 0xFF};
inline constexpr Vec4U8 LIGHT_CYAN   = Vec4U8{50, 173, 230, 0xFF};
inline constexpr Vec4U8 DARK_CYAN    = Vec4U8{100, 210, 0xFF, 0xFF};
inline constexpr Vec4U8 LIGHT_GREEN  = Vec4U8{52, 199, 89, 0xFF};
inline constexpr Vec4U8 DARK_GREEN   = Vec4U8{48, 209, 88, 0xFF};
inline constexpr Vec4U8 LIGHT_INDIGO = Vec4U8{88, 86, 214, 0xFF};
inline constexpr Vec4U8 DARK_INDIGO  = Vec4U8{94, 92, 230, 0xFF};
inline constexpr Vec4U8 LIGHT_MINT   = Vec4U8{0, 199, 190, 0xFF};
inline constexpr Vec4U8 DARK_MINT    = Vec4U8{102, 212, 207, 0xFF};
inline constexpr Vec4U8 LIGHT_ORANGE = Vec4U8{255, 149, 0, 0xFF};
inline constexpr Vec4U8 DARK_ORANGE  = Vec4U8{255, 159, 10, 0xFF};
inline constexpr Vec4U8 LIGHT_PINK   = Vec4U8{255, 45, 85, 0xFF};
inline constexpr Vec4U8 DARK_PINK    = Vec4U8{255, 55, 95, 0xFF};
inline constexpr Vec4U8 LIGHT_PURPLE = Vec4U8{175, 82, 222, 0xFF};
inline constexpr Vec4U8 DARK_PURPLE  = Vec4U8{191, 90, 242, 0xFF};
inline constexpr Vec4U8 LIGHT_RED    = Vec4U8{255, 59, 48, 0xFF};
inline constexpr Vec4U8 DARK_RED     = Vec4U8{255, 69, 58, 0xFF};
inline constexpr Vec4U8 LIGHT_TEAL   = Vec4U8{48, 176, 199, 0xFF};
inline constexpr Vec4U8 DARK_TEAL    = Vec4U8{64, 200, 224, 0xFF};
inline constexpr Vec4U8 LIGHT_YELLOW = Vec4U8{255, 204, 0, 0xFF};
inline constexpr Vec4U8 DARK_YELLOW  = Vec4U8{255, 214, 10, 0xFF};

inline constexpr Vec4U8 LIGHT_GRAY   = Vec4U8{142, 142, 147, 0xFF};
inline constexpr Vec4U8 DARK_GRAY    = Vec4U8{142, 142, 147, 0xFF};
inline constexpr Vec4U8 LIGHT_GRAY_2 = Vec4U8{174, 174, 178, 0xFF};
inline constexpr Vec4U8 DARK_GRAY_2  = Vec4U8{99, 99, 102, 0xFF};
inline constexpr Vec4U8 LIGHT_GRAY_3 = Vec4U8{199, 199, 204, 0xFF};
inline constexpr Vec4U8 DARK_GRAY_3  = Vec4U8{72, 72, 74, 0xFF};
inline constexpr Vec4U8 LIGHT_GRAY_4 = Vec4U8{209, 209, 214, 0xFF};
inline constexpr Vec4U8 DARK_GRAY_4  = Vec4U8{58, 58, 60, 0xFF};
inline constexpr Vec4U8 LIGHT_GRAY_5 = Vec4U8{229, 229, 234, 0xFF};
inline constexpr Vec4U8 DARK_GRAY_5  = Vec4U8{44, 44, 46, 0xFF};
inline constexpr Vec4U8 LIGHT_GRAY_6 = Vec4U8{242, 242, 247, 0xFF};
inline constexpr Vec4U8 DARK_GRAY_6  = Vec4U8{28, 28, 30, 0xFF};

// ios accessible colors
namespace accessible
{
inline constexpr Vec4U8 LIGHT_BLUE   = Vec4U8{0, 64, 221, 0xFF};
inline constexpr Vec4U8 DARK_BLUE    = Vec4U8{64, 156, 0xFF, 0xFF};
inline constexpr Vec4U8 LIGHT_BROWN  = Vec4U8{127, 101, 69, 0xFF};
inline constexpr Vec4U8 DARK_BROWN   = Vec4U8{181, 148, 105, 0xFF};
inline constexpr Vec4U8 LIGHT_CYAN   = Vec4U8{0, 113, 164, 0xFF};
inline constexpr Vec4U8 DARK_CYAN    = Vec4U8{112, 215, 0xFF, 0xFF};
inline constexpr Vec4U8 LIGHT_GREEN  = Vec4U8{36, 138, 61, 0xFF};
inline constexpr Vec4U8 DARK_GREEN   = Vec4U8{48, 219, 91, 0xFF};
inline constexpr Vec4U8 LIGHT_INDIGO = Vec4U8{54, 52, 163, 0xFF};
inline constexpr Vec4U8 DARK_INDIGO  = Vec4U8{125, 122, 0xFF, 0xFF};
inline constexpr Vec4U8 LIGHT_MINT   = Vec4U8{12, 129, 123, 0xFF};
inline constexpr Vec4U8 DARK_MINT    = Vec4U8{102, 212, 207, 0xFF};
inline constexpr Vec4U8 LIGHT_ORANGE = Vec4U8{201, 52, 0, 0xFF};
inline constexpr Vec4U8 DARK_ORANGE  = Vec4U8{255, 179, 64, 0xFF};
inline constexpr Vec4U8 LIGHT_PINK   = Vec4U8{211, 15, 69, 0xFF};
inline constexpr Vec4U8 DARK_PINK    = Vec4U8{255, 100, 130, 0xFF};
inline constexpr Vec4U8 LIGHT_PURPLE = Vec4U8{137, 68, 171, 0xFF};
inline constexpr Vec4U8 DARK_PURPLE  = Vec4U8{218, 143, 0xFF, 0xFF};
inline constexpr Vec4U8 LIGHT_RED    = Vec4U8{215, 0, 21, 0xFF};
inline constexpr Vec4U8 DARK_RED     = Vec4U8{255, 105, 97, 0xFF};
inline constexpr Vec4U8 LIGHT_TEAL   = Vec4U8{0, 130, 153, 0xFF};
inline constexpr Vec4U8 DARK_TEAL    = Vec4U8{93, 230, 0xFF, 0xFF};
inline constexpr Vec4U8 LIGHT_YELLOW = Vec4U8{178, 80, 0, 0xFF};
inline constexpr Vec4U8 DARK_YELLOW  = Vec4U8{255, 212, 38, 0xFF};

inline constexpr Vec4U8 LIGHT_GRAY   = Vec4U8{108, 108, 112, 0xFF};
inline constexpr Vec4U8 DARK_GRAY    = Vec4U8{174, 174, 178, 0xFF};
inline constexpr Vec4U8 LIGHT_GRAY_2 = Vec4U8{142, 142, 147, 0xFF};
inline constexpr Vec4U8 DARK_GRAY_2  = Vec4U8{124, 124, 128, 0xFF};
inline constexpr Vec4U8 LIGHT_GRAY_3 = Vec4U8{174, 174, 178, 0xFF};
inline constexpr Vec4U8 DARK_GRAY_3  = Vec4U8{84, 84, 86, 0xFF};
inline constexpr Vec4U8 LIGHT_GRAY_4 = Vec4U8{188, 188, 192, 0xFF};
inline constexpr Vec4U8 DARK_GRAY_4  = Vec4U8{68, 68, 70, 0xFF};
inline constexpr Vec4U8 LIGHT_GRAY_5 = Vec4U8{216, 216, 220, 0xFF};
inline constexpr Vec4U8 DARK_GRAY_5  = Vec4U8{54, 54, 56, 0xFF};
inline constexpr Vec4U8 LIGHT_GRAY_6 = Vec4U8{235, 235, 240, 0xFF};
inline constexpr Vec4U8 DARK_GRAY_6  = Vec4U8{36, 36, 36, 0xFF};
}    // namespace accessible

}    // namespace ios

/// @brief Material Design Colors
namespace mdc
{
inline constexpr Vec4U8 RED_50   = Vec4U8{0xFF, 0xEB, 0xEE, 0xFF};
inline constexpr Vec4U8 RED_100  = Vec4U8{0xFF, 0xCD, 0xD2, 0xFF};
inline constexpr Vec4U8 RED_200  = Vec4U8{0xEF, 0x9A, 0x9A, 0xFF};
inline constexpr Vec4U8 RED_300  = Vec4U8{0xE5, 0x73, 0x73, 0xFF};
inline constexpr Vec4U8 RED_400  = Vec4U8{0xEF, 0x53, 0x50, 0xFF};
inline constexpr Vec4U8 RED_500  = Vec4U8{0xF4, 0x43, 0x36, 0xFF};
inline constexpr Vec4U8 RED_600  = Vec4U8{0xE5, 0x39, 0x35, 0xFF};
inline constexpr Vec4U8 RED_700  = Vec4U8{0xD3, 0x2F, 0x2F, 0xFF};
inline constexpr Vec4U8 RED_800  = Vec4U8{0xC6, 0x28, 0x28, 0xFF};
inline constexpr Vec4U8 RED_900  = Vec4U8{0xB7, 0x1C, 0x1C, 0xFF};
inline constexpr Vec4U8 RED_A100 = Vec4U8{0xFF, 0x8A, 0x80, 0xFF};
inline constexpr Vec4U8 RED_A200 = Vec4U8{0xFF, 0x52, 0x52, 0xFF};
inline constexpr Vec4U8 RED_A400 = Vec4U8{0xFF, 0x17, 0x44, 0xFF};
inline constexpr Vec4U8 RED_A700 = Vec4U8{0xD5, 0x00, 0x00, 0xFF};

inline constexpr Vec4U8 PINK_50   = Vec4U8{0xFC, 0xE4, 0xEC, 0xFF};
inline constexpr Vec4U8 PINK_100  = Vec4U8{0xF8, 0xBB, 0xD0, 0xFF};
inline constexpr Vec4U8 PINK_200  = Vec4U8{0xF4, 0x8F, 0xB1, 0xFF};
inline constexpr Vec4U8 PINK_300  = Vec4U8{0xF0, 0x62, 0x92, 0xFF};
inline constexpr Vec4U8 PINK_400  = Vec4U8{0xEC, 0x40, 0x7A, 0xFF};
inline constexpr Vec4U8 PINK_500  = Vec4U8{0xE9, 0x1E, 0x63, 0xFF};
inline constexpr Vec4U8 PINK_600  = Vec4U8{0xD8, 0x1B, 0x60, 0xFF};
inline constexpr Vec4U8 PINK_700  = Vec4U8{0xC2, 0x18, 0x5B, 0xFF};
inline constexpr Vec4U8 PINK_800  = Vec4U8{0xAD, 0x14, 0x57, 0xFF};
inline constexpr Vec4U8 PINK_900  = Vec4U8{0x88, 0x0E, 0x4F, 0xFF};
inline constexpr Vec4U8 PINK_A100 = Vec4U8{0xFF, 0x80, 0xAB, 0xFF};
inline constexpr Vec4U8 PINK_A200 = Vec4U8{0xFF, 0x40, 0x81, 0xFF};
inline constexpr Vec4U8 PINK_A400 = Vec4U8{0xF5, 0x00, 0x57, 0xFF};
inline constexpr Vec4U8 PINK_A700 = Vec4U8{0xC5, 0x11, 0x62, 0xFF};

inline constexpr Vec4U8 PURPLE_50   = Vec4U8{0xF3, 0xE5, 0xF5, 0xFF};
inline constexpr Vec4U8 PURPLE_100  = Vec4U8{0xE1, 0xBE, 0xE7, 0xFF};
inline constexpr Vec4U8 PURPLE_200  = Vec4U8{0xCE, 0x93, 0xD8, 0xFF};
inline constexpr Vec4U8 PURPLE_300  = Vec4U8{0xBA, 0x68, 0xC8, 0xFF};
inline constexpr Vec4U8 PURPLE_400  = Vec4U8{0xAB, 0x47, 0xBC, 0xFF};
inline constexpr Vec4U8 PURPLE_500  = Vec4U8{0x9C, 0x27, 0xB0, 0xFF};
inline constexpr Vec4U8 PURPLE_600  = Vec4U8{0x8E, 0x24, 0xAA, 0xFF};
inline constexpr Vec4U8 PURPLE_700  = Vec4U8{0x7B, 0x1F, 0xA2, 0xFF};
inline constexpr Vec4U8 PURPLE_800  = Vec4U8{0x6A, 0x1B, 0x9A, 0xFF};
inline constexpr Vec4U8 PURPLE_900  = Vec4U8{0x4A, 0x14, 0x8C, 0xFF};
inline constexpr Vec4U8 PURPLE_A100 = Vec4U8{0xEA, 0x80, 0xFC, 0xFF};
inline constexpr Vec4U8 PURPLE_A200 = Vec4U8{0xE0, 0x40, 0xFB, 0xFF};
inline constexpr Vec4U8 PURPLE_A400 = Vec4U8{0xD5, 0x00, 0xF9, 0xFF};
inline constexpr Vec4U8 PURPLE_A700 = Vec4U8{0xAA, 0x00, 0xFF, 0xFF};

inline constexpr Vec4U8 DEEP_PURPLE_50   = Vec4U8{0xED, 0xE7, 0xF6, 0xFF};
inline constexpr Vec4U8 DEEP_PURPLE_100  = Vec4U8{0xD1, 0xC4, 0xE9, 0xFF};
inline constexpr Vec4U8 DEEP_PURPLE_200  = Vec4U8{0xB3, 0x9D, 0xDB, 0xFF};
inline constexpr Vec4U8 DEEP_PURPLE_300  = Vec4U8{0x95, 0x75, 0xCD, 0xFF};
inline constexpr Vec4U8 DEEP_PURPLE_400  = Vec4U8{0x7E, 0x57, 0xC2, 0xFF};
inline constexpr Vec4U8 DEEP_PURPLE_500  = Vec4U8{0x67, 0x3A, 0xB7, 0xFF};
inline constexpr Vec4U8 DEEP_PURPLE_600  = Vec4U8{0x5E, 0x35, 0xB1, 0xFF};
inline constexpr Vec4U8 DEEP_PURPLE_700  = Vec4U8{0x51, 0x2D, 0xA8, 0xFF};
inline constexpr Vec4U8 DEEP_PURPLE_800  = Vec4U8{0x45, 0x27, 0xA0, 0xFF};
inline constexpr Vec4U8 DEEP_PURPLE_900  = Vec4U8{0x31, 0x1B, 0x92, 0xFF};
inline constexpr Vec4U8 DEEP_PURPLE_A100 = Vec4U8{0xB3, 0x88, 0xFF, 0xFF};
inline constexpr Vec4U8 DEEP_PURPLE_A200 = Vec4U8{0x7C, 0x4D, 0xFF, 0xFF};
inline constexpr Vec4U8 DEEP_PURPLE_A400 = Vec4U8{0x65, 0x1F, 0xFF, 0xFF};
inline constexpr Vec4U8 DEEP_PURPLE_A700 = Vec4U8{0x62, 0x00, 0xEA, 0xFF};

inline constexpr Vec4U8 INDIGO_50   = Vec4U8{0xE8, 0xEA, 0xF6, 0xFF};
inline constexpr Vec4U8 INDIGO_100  = Vec4U8{0xC5, 0xCA, 0xE9, 0xFF};
inline constexpr Vec4U8 INDIGO_200  = Vec4U8{0x9F, 0xA8, 0xDA, 0xFF};
inline constexpr Vec4U8 INDIGO_300  = Vec4U8{0x79, 0x86, 0xCB, 0xFF};
inline constexpr Vec4U8 INDIGO_400  = Vec4U8{0x5C, 0x6B, 0xC0, 0xFF};
inline constexpr Vec4U8 INDIGO_500  = Vec4U8{0x3F, 0x51, 0xB5, 0xFF};
inline constexpr Vec4U8 INDIGO_600  = Vec4U8{0x39, 0x49, 0xAB, 0xFF};
inline constexpr Vec4U8 INDIGO_700  = Vec4U8{0x30, 0x3F, 0x9F, 0xFF};
inline constexpr Vec4U8 INDIGO_800  = Vec4U8{0x28, 0x35, 0x93, 0xFF};
inline constexpr Vec4U8 INDIGO_900  = Vec4U8{0x1A, 0x23, 0x7E, 0xFF};
inline constexpr Vec4U8 INDIGO_A100 = Vec4U8{0x8C, 0x9E, 0xFF, 0xFF};
inline constexpr Vec4U8 INDIGO_A200 = Vec4U8{0x53, 0x6D, 0xFE, 0xFF};
inline constexpr Vec4U8 INDIGO_A400 = Vec4U8{0x3D, 0x5A, 0xFE, 0xFF};
inline constexpr Vec4U8 INDIGO_A700 = Vec4U8{0x30, 0x4F, 0xFE, 0xFF};

inline constexpr Vec4U8 BLUE_50   = Vec4U8{0xE3, 0xF2, 0xFD, 0xFF};
inline constexpr Vec4U8 BLUE_100  = Vec4U8{0xBB, 0xDE, 0xFB, 0xFF};
inline constexpr Vec4U8 BLUE_200  = Vec4U8{0x90, 0xCA, 0xF9, 0xFF};
inline constexpr Vec4U8 BLUE_300  = Vec4U8{0x64, 0xB5, 0xF6, 0xFF};
inline constexpr Vec4U8 BLUE_400  = Vec4U8{0x42, 0xA5, 0xF5, 0xFF};
inline constexpr Vec4U8 BLUE_500  = Vec4U8{0x21, 0x96, 0xF3, 0xFF};
inline constexpr Vec4U8 BLUE_600  = Vec4U8{0x1E, 0x88, 0xE5, 0xFF};
inline constexpr Vec4U8 BLUE_700  = Vec4U8{0x19, 0x76, 0xD2, 0xFF};
inline constexpr Vec4U8 BLUE_800  = Vec4U8{0x15, 0x65, 0xC0, 0xFF};
inline constexpr Vec4U8 BLUE_900  = Vec4U8{0x0D, 0x47, 0xA1, 0xFF};
inline constexpr Vec4U8 BLUE_A100 = Vec4U8{0x82, 0xB1, 0xFF, 0xFF};
inline constexpr Vec4U8 BLUE_A200 = Vec4U8{0x44, 0x8A, 0xFF, 0xFF};
inline constexpr Vec4U8 BLUE_A400 = Vec4U8{0x29, 0x79, 0xFF, 0xFF};
inline constexpr Vec4U8 BLUE_A700 = Vec4U8{0x29, 0x62, 0xFF, 0xFF};

inline constexpr Vec4U8 LIGHT_BLUE_50   = Vec4U8{0xE1, 0xF5, 0xFE, 0xFF};
inline constexpr Vec4U8 LIGHT_BLUE_100  = Vec4U8{0xB3, 0xE5, 0xFC, 0xFF};
inline constexpr Vec4U8 LIGHT_BLUE_200  = Vec4U8{0x81, 0xD4, 0xFA, 0xFF};
inline constexpr Vec4U8 LIGHT_BLUE_300  = Vec4U8{0x4F, 0xC3, 0xF7, 0xFF};
inline constexpr Vec4U8 LIGHT_BLUE_400  = Vec4U8{0x29, 0xB6, 0xF6, 0xFF};
inline constexpr Vec4U8 LIGHT_BLUE_500  = Vec4U8{0x03, 0xA9, 0xF4, 0xFF};
inline constexpr Vec4U8 LIGHT_BLUE_600  = Vec4U8{0x03, 0x9B, 0xE5, 0xFF};
inline constexpr Vec4U8 LIGHT_BLUE_700  = Vec4U8{0x02, 0x88, 0xD1, 0xFF};
inline constexpr Vec4U8 LIGHT_BLUE_800  = Vec4U8{0x02, 0x77, 0xBD, 0xFF};
inline constexpr Vec4U8 LIGHT_BLUE_900  = Vec4U8{0x01, 0x57, 0x9B, 0xFF};
inline constexpr Vec4U8 LIGHT_BLUE_A100 = Vec4U8{0x80, 0xD8, 0xFF, 0xFF};
inline constexpr Vec4U8 LIGHT_BLUE_A200 = Vec4U8{0x40, 0xC4, 0xFF, 0xFF};
inline constexpr Vec4U8 LIGHT_BLUE_A400 = Vec4U8{0x00, 0xB0, 0xFF, 0xFF};
inline constexpr Vec4U8 LIGHT_BLUE_A700 = Vec4U8{0x00, 0x91, 0xEA, 0xFF};

inline constexpr Vec4U8 CYAN_50   = Vec4U8{0xE0, 0xF7, 0xFA, 0xFF};
inline constexpr Vec4U8 CYAN_100  = Vec4U8{0xB2, 0xEB, 0xF2, 0xFF};
inline constexpr Vec4U8 CYAN_200  = Vec4U8{0x80, 0xDE, 0xEA, 0xFF};
inline constexpr Vec4U8 CYAN_300  = Vec4U8{0x4D, 0xD0, 0xE1, 0xFF};
inline constexpr Vec4U8 CYAN_400  = Vec4U8{0x26, 0xC6, 0xDA, 0xFF};
inline constexpr Vec4U8 CYAN_500  = Vec4U8{0x00, 0xBC, 0xD4, 0xFF};
inline constexpr Vec4U8 CYAN_600  = Vec4U8{0x00, 0xAC, 0xC1, 0xFF};
inline constexpr Vec4U8 CYAN_700  = Vec4U8{0x00, 0x97, 0xA7, 0xFF};
inline constexpr Vec4U8 CYAN_800  = Vec4U8{0x00, 0x83, 0x8F, 0xFF};
inline constexpr Vec4U8 CYAN_900  = Vec4U8{0x00, 0x60, 0x64, 0xFF};
inline constexpr Vec4U8 CYAN_A100 = Vec4U8{0x84, 0xFF, 0xFF, 0xFF};
inline constexpr Vec4U8 CYAN_A200 = Vec4U8{0x18, 0xFF, 0xFF, 0xFF};
inline constexpr Vec4U8 CYAN_A400 = Vec4U8{0x00, 0xE5, 0xFF, 0xFF};
inline constexpr Vec4U8 CYAN_A700 = Vec4U8{0x00, 0xB8, 0xD4, 0xFF};

inline constexpr Vec4U8 TEAL_50   = Vec4U8{0xE0, 0xF2, 0xF1, 0xFF};
inline constexpr Vec4U8 TEAL_100  = Vec4U8{0xB2, 0xDF, 0xDB, 0xFF};
inline constexpr Vec4U8 TEAL_200  = Vec4U8{0x80, 0xCB, 0xC4, 0xFF};
inline constexpr Vec4U8 TEAL_300  = Vec4U8{0x4D, 0xB6, 0xAC, 0xFF};
inline constexpr Vec4U8 TEAL_400  = Vec4U8{0x26, 0xA6, 0x9A, 0xFF};
inline constexpr Vec4U8 TEAL_500  = Vec4U8{0x00, 0x96, 0x88, 0xFF};
inline constexpr Vec4U8 TEAL_600  = Vec4U8{0x00, 0x89, 0x7B, 0xFF};
inline constexpr Vec4U8 TEAL_700  = Vec4U8{0x00, 0x79, 0x6B, 0xFF};
inline constexpr Vec4U8 TEAL_800  = Vec4U8{0x00, 0x69, 0x5C, 0xFF};
inline constexpr Vec4U8 TEAL_900  = Vec4U8{0x00, 0x4D, 0x40, 0xFF};
inline constexpr Vec4U8 TEAL_A100 = Vec4U8{0xA7, 0xFF, 0xEB, 0xFF};
inline constexpr Vec4U8 TEAL_A200 = Vec4U8{0x64, 0xFF, 0xDA, 0xFF};
inline constexpr Vec4U8 TEAL_A400 = Vec4U8{0x1D, 0xE9, 0xB6, 0xFF};
inline constexpr Vec4U8 TEAL_A700 = Vec4U8{0x00, 0xBF, 0xA5, 0xFF};

inline constexpr Vec4U8 GREEN_50   = Vec4U8{0xE8, 0xF5, 0xE9, 0xFF};
inline constexpr Vec4U8 GREEN_100  = Vec4U8{0xC8, 0xE6, 0xC9, 0xFF};
inline constexpr Vec4U8 GREEN_200  = Vec4U8{0xA5, 0xD6, 0xA7, 0xFF};
inline constexpr Vec4U8 GREEN_300  = Vec4U8{0x81, 0xC7, 0x84, 0xFF};
inline constexpr Vec4U8 GREEN_400  = Vec4U8{0x66, 0xBB, 0x6A, 0xFF};
inline constexpr Vec4U8 GREEN_500  = Vec4U8{0x4C, 0xAF, 0x50, 0xFF};
inline constexpr Vec4U8 GREEN_600  = Vec4U8{0x43, 0xA0, 0x47, 0xFF};
inline constexpr Vec4U8 GREEN_700  = Vec4U8{0x38, 0x8E, 0x3C, 0xFF};
inline constexpr Vec4U8 GREEN_800  = Vec4U8{0x2E, 0x7D, 0x32, 0xFF};
inline constexpr Vec4U8 GREEN_900  = Vec4U8{0x1B, 0x5E, 0x20, 0xFF};
inline constexpr Vec4U8 GREEN_A100 = Vec4U8{0xB9, 0xF6, 0xCA, 0xFF};
inline constexpr Vec4U8 GREEN_A200 = Vec4U8{0x69, 0xF0, 0xAE, 0xFF};
inline constexpr Vec4U8 GREEN_A400 = Vec4U8{0x00, 0xE6, 0x76, 0xFF};
inline constexpr Vec4U8 GREEN_A700 = Vec4U8{0x00, 0xC8, 0x53, 0xFF};

inline constexpr Vec4U8 LIGHT_GREEN_50   = Vec4U8{0xF1, 0xF8, 0xE9, 0xFF};
inline constexpr Vec4U8 LIGHT_GREEN_100  = Vec4U8{0xDC, 0xED, 0xC8, 0xFF};
inline constexpr Vec4U8 LIGHT_GREEN_200  = Vec4U8{0xC5, 0xE1, 0xA5, 0xFF};
inline constexpr Vec4U8 LIGHT_GREEN_300  = Vec4U8{0xAE, 0xD5, 0x81, 0xFF};
inline constexpr Vec4U8 LIGHT_GREEN_400  = Vec4U8{0x9C, 0xCC, 0x65, 0xFF};
inline constexpr Vec4U8 LIGHT_GREEN_500  = Vec4U8{0x8B, 0xC3, 0x4A, 0xFF};
inline constexpr Vec4U8 LIGHT_GREEN_600  = Vec4U8{0x7C, 0xB3, 0x42, 0xFF};
inline constexpr Vec4U8 LIGHT_GREEN_700  = Vec4U8{0x68, 0x9F, 0x38, 0xFF};
inline constexpr Vec4U8 LIGHT_GREEN_800  = Vec4U8{0x55, 0x8B, 0x2F, 0xFF};
inline constexpr Vec4U8 LIGHT_GREEN_900  = Vec4U8{0x33, 0x69, 0x1E, 0xFF};
inline constexpr Vec4U8 LIGHT_GREEN_A100 = Vec4U8{0xCC, 0xFF, 0x90, 0xFF};
inline constexpr Vec4U8 LIGHT_GREEN_A200 = Vec4U8{0xB2, 0xFF, 0x59, 0xFF};
inline constexpr Vec4U8 LIGHT_GREEN_A400 = Vec4U8{0x76, 0xFF, 0x03, 0xFF};
inline constexpr Vec4U8 LIGHT_GREEN_A700 = Vec4U8{0x64, 0xDD, 0x17, 0xFF};

inline constexpr Vec4U8 LIME_50   = Vec4U8{0xF9, 0xFB, 0xE7, 0xFF};
inline constexpr Vec4U8 LIME_100  = Vec4U8{0xF0, 0xF4, 0xC3, 0xFF};
inline constexpr Vec4U8 LIME_200  = Vec4U8{0xE6, 0xEE, 0x9C, 0xFF};
inline constexpr Vec4U8 LIME_300  = Vec4U8{0xDC, 0xE7, 0x75, 0xFF};
inline constexpr Vec4U8 LIME_400  = Vec4U8{0xD4, 0xE1, 0x57, 0xFF};
inline constexpr Vec4U8 LIME_500  = Vec4U8{0xCD, 0xDC, 0x39, 0xFF};
inline constexpr Vec4U8 LIME_600  = Vec4U8{0xC0, 0xCA, 0x33, 0xFF};
inline constexpr Vec4U8 LIME_700  = Vec4U8{0xAF, 0xB4, 0x2B, 0xFF};
inline constexpr Vec4U8 LIME_800  = Vec4U8{0x9E, 0x9D, 0x24, 0xFF};
inline constexpr Vec4U8 LIME_900  = Vec4U8{0x82, 0x77, 0x17, 0xFF};
inline constexpr Vec4U8 LIME_A100 = Vec4U8{0xF4, 0xFF, 0x81, 0xFF};
inline constexpr Vec4U8 LIME_A200 = Vec4U8{0xEE, 0xFF, 0x41, 0xFF};
inline constexpr Vec4U8 LIME_A400 = Vec4U8{0xC6, 0xFF, 0x00, 0xFF};
inline constexpr Vec4U8 LIME_A700 = Vec4U8{0xAE, 0xEA, 0x00, 0xFF};

inline constexpr Vec4U8 YELLOW_50   = Vec4U8{0xFF, 0xFD, 0xE7, 0xFF};
inline constexpr Vec4U8 YELLOW_100  = Vec4U8{0xFF, 0xF9, 0xC4, 0xFF};
inline constexpr Vec4U8 YELLOW_200  = Vec4U8{0xFF, 0xF5, 0x9D, 0xFF};
inline constexpr Vec4U8 YELLOW_300  = Vec4U8{0xFF, 0xF1, 0x76, 0xFF};
inline constexpr Vec4U8 YELLOW_400  = Vec4U8{0xFF, 0xEE, 0x58, 0xFF};
inline constexpr Vec4U8 YELLOW_500  = Vec4U8{0xFF, 0xEB, 0x3B, 0xFF};
inline constexpr Vec4U8 YELLOW_600  = Vec4U8{0xFD, 0xD8, 0x35, 0xFF};
inline constexpr Vec4U8 YELLOW_700  = Vec4U8{0xFB, 0xC0, 0x2D, 0xFF};
inline constexpr Vec4U8 YELLOW_800  = Vec4U8{0xF9, 0xA8, 0x25, 0xFF};
inline constexpr Vec4U8 YELLOW_900  = Vec4U8{0xF5, 0x7F, 0x17, 0xFF};
inline constexpr Vec4U8 YELLOW_A100 = Vec4U8{0xFF, 0xFF, 0x8D, 0xFF};
inline constexpr Vec4U8 YELLOW_A200 = Vec4U8{0xFF, 0xFF, 0x00, 0xFF};
inline constexpr Vec4U8 YELLOW_A400 = Vec4U8{0xFF, 0xEA, 0x00, 0xFF};
inline constexpr Vec4U8 YELLOW_A700 = Vec4U8{0xFF, 0xD6, 0x00, 0xFF};

inline constexpr Vec4U8 AMBER_50   = Vec4U8{0xFF, 0xF8, 0xE1, 0xFF};
inline constexpr Vec4U8 AMBER_100  = Vec4U8{0xFF, 0xEC, 0xB3, 0xFF};
inline constexpr Vec4U8 AMBER_200  = Vec4U8{0xFF, 0xE0, 0x82, 0xFF};
inline constexpr Vec4U8 AMBER_300  = Vec4U8{0xFF, 0xD5, 0x4F, 0xFF};
inline constexpr Vec4U8 AMBER_400  = Vec4U8{0xFF, 0xCA, 0x28, 0xFF};
inline constexpr Vec4U8 AMBER_500  = Vec4U8{0xFF, 0xC1, 0x07, 0xFF};
inline constexpr Vec4U8 AMBER_600  = Vec4U8{0xFF, 0xB3, 0x00, 0xFF};
inline constexpr Vec4U8 AMBER_700  = Vec4U8{0xFF, 0xA0, 0x00, 0xFF};
inline constexpr Vec4U8 AMBER_800  = Vec4U8{0xFF, 0x8F, 0x00, 0xFF};
inline constexpr Vec4U8 AMBER_900  = Vec4U8{0xFF, 0x6F, 0x00, 0xFF};
inline constexpr Vec4U8 AMBER_A100 = Vec4U8{0xFF, 0xE5, 0x7F, 0xFF};
inline constexpr Vec4U8 AMBER_A200 = Vec4U8{0xFF, 0xD7, 0x40, 0xFF};
inline constexpr Vec4U8 AMBER_A400 = Vec4U8{0xFF, 0xC4, 0x00, 0xFF};
inline constexpr Vec4U8 AMBER_A700 = Vec4U8{0xFF, 0xAB, 0x00, 0xFF};

inline constexpr Vec4U8 ORANGE_50   = Vec4U8{0xFF, 0xF3, 0xE0, 0xFF};
inline constexpr Vec4U8 ORANGE_100  = Vec4U8{0xFF, 0xE0, 0xB2, 0xFF};
inline constexpr Vec4U8 ORANGE_200  = Vec4U8{0xFF, 0xCC, 0x80, 0xFF};
inline constexpr Vec4U8 ORANGE_300  = Vec4U8{0xFF, 0xB7, 0x4D, 0xFF};
inline constexpr Vec4U8 ORANGE_400  = Vec4U8{0xFF, 0xA7, 0x26, 0xFF};
inline constexpr Vec4U8 ORANGE_500  = Vec4U8{0xFF, 0x98, 0x00, 0xFF};
inline constexpr Vec4U8 ORANGE_600  = Vec4U8{0xFB, 0x8C, 0x00, 0xFF};
inline constexpr Vec4U8 ORANGE_700  = Vec4U8{0xF5, 0x7C, 0x00, 0xFF};
inline constexpr Vec4U8 ORANGE_800  = Vec4U8{0xEF, 0x6C, 0x00, 0xFF};
inline constexpr Vec4U8 ORANGE_900  = Vec4U8{0xE6, 0x51, 0x00, 0xFF};
inline constexpr Vec4U8 ORANGE_A100 = Vec4U8{0xFF, 0xD1, 0x80, 0xFF};
inline constexpr Vec4U8 ORANGE_A200 = Vec4U8{0xFF, 0xAB, 0x40, 0xFF};
inline constexpr Vec4U8 ORANGE_A400 = Vec4U8{0xFF, 0x91, 0x00, 0xFF};
inline constexpr Vec4U8 ORANGE_A700 = Vec4U8{0xFF, 0x6D, 0x00, 0xFF};

inline constexpr Vec4U8 DEEP_ORANGE_50   = Vec4U8{0xFB, 0xE9, 0xE7, 0xFF};
inline constexpr Vec4U8 DEEP_ORANGE_100  = Vec4U8{0xFF, 0xCC, 0xBC, 0xFF};
inline constexpr Vec4U8 DEEP_ORANGE_200  = Vec4U8{0xFF, 0xAB, 0x91, 0xFF};
inline constexpr Vec4U8 DEEP_ORANGE_300  = Vec4U8{0xFF, 0x8A, 0x65, 0xFF};
inline constexpr Vec4U8 DEEP_ORANGE_400  = Vec4U8{0xFF, 0x70, 0x43, 0xFF};
inline constexpr Vec4U8 DEEP_ORANGE_500  = Vec4U8{0xFF, 0x57, 0x22, 0xFF};
inline constexpr Vec4U8 DEEP_ORANGE_600  = Vec4U8{0xF4, 0x51, 0x1E, 0xFF};
inline constexpr Vec4U8 DEEP_ORANGE_700  = Vec4U8{0xE6, 0x4A, 0x19, 0xFF};
inline constexpr Vec4U8 DEEP_ORANGE_800  = Vec4U8{0xD8, 0x43, 0x15, 0xFF};
inline constexpr Vec4U8 DEEP_ORANGE_900  = Vec4U8{0xBF, 0x36, 0x0C, 0xFF};
inline constexpr Vec4U8 DEEP_ORANGE_A100 = Vec4U8{0xFF, 0x9E, 0x80, 0xFF};
inline constexpr Vec4U8 DEEP_ORANGE_A200 = Vec4U8{0xFF, 0x6E, 0x40, 0xFF};
inline constexpr Vec4U8 DEEP_ORANGE_A400 = Vec4U8{0xFF, 0x3D, 0x00, 0xFF};
inline constexpr Vec4U8 DEEP_ORANGE_A700 = Vec4U8{0xDD, 0x2C, 0x00, 0xFF};

inline constexpr Vec4U8 BROWN_50  = Vec4U8{0xEF, 0xEB, 0xE9, 0xFF};
inline constexpr Vec4U8 BROWN_100 = Vec4U8{0xD7, 0xCC, 0xC8, 0xFF};
inline constexpr Vec4U8 BROWN_200 = Vec4U8{0xBC, 0xAA, 0xA4, 0xFF};
inline constexpr Vec4U8 BROWN_300 = Vec4U8{0xA1, 0x88, 0x7F, 0xFF};
inline constexpr Vec4U8 BROWN_400 = Vec4U8{0x8D, 0x6E, 0x63, 0xFF};
inline constexpr Vec4U8 BROWN_500 = Vec4U8{0x79, 0x55, 0x48, 0xFF};
inline constexpr Vec4U8 BROWN_600 = Vec4U8{0x6D, 0x4C, 0x41, 0xFF};
inline constexpr Vec4U8 BROWN_700 = Vec4U8{0x5D, 0x40, 0x37, 0xFF};
inline constexpr Vec4U8 BROWN_800 = Vec4U8{0x4E, 0x34, 0x2E, 0xFF};
inline constexpr Vec4U8 BROWN_900 = Vec4U8{0x3E, 0x27, 0x23, 0xFF};

inline constexpr Vec4U8 GRAY_50  = Vec4U8{0xFA, 0xFA, 0xFA, 0xFF};
inline constexpr Vec4U8 GRAY_100 = Vec4U8{0xF5, 0xF5, 0xF5, 0xFF};
inline constexpr Vec4U8 GRAY_200 = Vec4U8{0xEE, 0xEE, 0xEE, 0xFF};
inline constexpr Vec4U8 GRAY_300 = Vec4U8{0xE0, 0xE0, 0xE0, 0xFF};
inline constexpr Vec4U8 GRAY_400 = Vec4U8{0xBD, 0xBD, 0xBD, 0xFF};
inline constexpr Vec4U8 GRAY_500 = Vec4U8{0x9E, 0x9E, 0x9E, 0xFF};
inline constexpr Vec4U8 GRAY_600 = Vec4U8{0x75, 0x75, 0x75, 0xFF};
inline constexpr Vec4U8 GRAY_700 = Vec4U8{0x61, 0x61, 0x61, 0xFF};
inline constexpr Vec4U8 GRAY_800 = Vec4U8{0x42, 0x42, 0x42, 0xFF};
inline constexpr Vec4U8 GRAY_900 = Vec4U8{0x21, 0x21, 0x21, 0xFF};

inline constexpr Vec4U8 BLUE_GRAY_50  = Vec4U8{0xEC, 0xEF, 0xF1, 0xFF};
inline constexpr Vec4U8 BLUE_GRAY_100 = Vec4U8{0xCF, 0xD8, 0xDC, 0xFF};
inline constexpr Vec4U8 BLUE_GRAY_200 = Vec4U8{0xB0, 0xBE, 0xC5, 0xFF};
inline constexpr Vec4U8 BLUE_GRAY_300 = Vec4U8{0x90, 0xA4, 0xAE, 0xFF};
inline constexpr Vec4U8 BLUE_GRAY_400 = Vec4U8{0x78, 0x90, 0x9C, 0xFF};
inline constexpr Vec4U8 BLUE_GRAY_500 = Vec4U8{0x60, 0x7D, 0x8B, 0xFF};
inline constexpr Vec4U8 BLUE_GRAY_600 = Vec4U8{0x54, 0x6E, 0x7A, 0xFF};
inline constexpr Vec4U8 BLUE_GRAY_700 = Vec4U8{0x45, 0x5A, 0x64, 0xFF};
inline constexpr Vec4U8 BLUE_GRAY_800 = Vec4U8{0x37, 0x47, 0x4F, 0xFF};
inline constexpr Vec4U8 BLUE_GRAY_900 = Vec4U8{0x26, 0x32, 0x38, 0xFF};

inline constexpr Vec4U8 WHITE = Vec4U8{0xFF, 0xFF, 0xFF, 0xFF};
inline constexpr Vec4U8 BLACK = Vec4U8{0x00, 0x00, 0x00, 0xFF};

}    // namespace mdc

struct ColorGradient
{
  Vec4 tl;
  Vec4 tr;
  Vec4 bl;
  Vec4 br;

  constexpr ColorGradient() = default;

  constexpr ColorGradient(Vec4 c) : tl{c}, tr{c}, bl{c}, br{c}
  {
  }

  constexpr ColorGradient(Vec4U8 c) : ColorGradient{norm(c)}
  {
  }

  constexpr ColorGradient(Vec4 tl, Vec4 tr, Vec4 bl, Vec4 br) :
    tl{tl},
    tr{tr},
    bl{bl},
    br{br}
  {
  }

  constexpr ColorGradient(Vec4U8 tl, Vec4U8 tr, Vec4U8 bl, Vec4U8 br) :
    ColorGradient{norm(tl), norm(tr), norm(bl), norm(br)}
  {
  }

  static constexpr ColorGradient x(Vec4 x0, Vec4 x1)
  {
    return ColorGradient{x0, x1, x0, x1};
  }

  static constexpr ColorGradient x(Vec4U8 x0, Vec4U8 x1)
  {
    return x(norm(x0), norm(x1));
  }

  static constexpr ColorGradient y(Vec4 y0, Vec4 y1)
  {
    return ColorGradient{y0, y0, y1, y1};
  }

  static constexpr ColorGradient y(Vec4U8 y0, Vec4U8 y1)
  {
    return y(norm(y0), norm(y1));
  }

  constexpr Vec4 const & operator[](usize i) const
  {
    return (&tl)[i];
  }

  constexpr Vec4 & operator[](usize i)
  {
    return (&tl)[i];
  }

  constexpr bool is_transparent() const
  {
    return tl.w == 0 && tr.w == 0 && bl.w == 0 && br.w == 0;
  }
};

}    // namespace ash
