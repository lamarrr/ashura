
#pragma once
#include "ashura/std/types.h"

namespace ash
{

typedef Vec4 Color;

namespace colors
{
constexpr Color WHITE       = Vec4{0xff, 0xff, 0xff, 0xff} / 255;
constexpr Color BLACK       = Vec4{0x00, 0x00, 0x00, 0xff} / 255;
constexpr Color RED         = Vec4{0xff, 0x00, 0x00, 0xff} / 255;
constexpr Color BLUE        = Vec4{0x00, 0x00, 0xff, 0xff} / 255;
constexpr Color GREEN       = Vec4{0x00, 0xff, 0x00, 0xff} / 255;
constexpr Color CYAN        = Vec4{0x00, 0xff, 0xff, 0xff} / 255;
constexpr Color MAGENTA     = Vec4{0xff, 0x00, 0xff, 0xff} / 255;
constexpr Color YELLOW      = Vec4{0xff, 0xff, 0x00, 0xff} / 255;
}        // namespace colors

// ios default system colors
namespace ios
{
constexpr Color LIGHT_BLUE   = Vec4{0, 122, 255, 255} / 255;
constexpr Color DARK_BLUE    = Vec4{10, 132, 255, 255} / 255;
constexpr Color LIGHT_BROWN  = Vec4{162, 132, 94, 255} / 255;
constexpr Color DARK_BROWN   = Vec4{172, 142, 104, 255} / 255;
constexpr Color LIGHT_CYAN   = Vec4{50, 173, 230, 255} / 255;
constexpr Color DARK_CYAN    = Vec4{100, 210, 255, 255} / 255;
constexpr Color LIGHT_GREEN  = Vec4{52, 199, 89, 255} / 255;
constexpr Color DARK_GREEN   = Vec4{48, 209, 88, 255} / 255;
constexpr Color LIGHT_INDIGO = Vec4{88, 86, 214, 255} / 255;
constexpr Color DARK_INDIGO  = Vec4{94, 92, 230, 255} / 255;
constexpr Color LIGHT_MINT   = Vec4{0, 199, 190, 255} / 255;
constexpr Color DARK_MINT    = Vec4{102, 212, 207, 255} / 255;
constexpr Color LIGHT_ORANGE = Vec4{255, 149, 0, 255} / 255;
constexpr Color DARK_ORANGE  = Vec4{255, 159, 10, 255} / 255;
constexpr Color LIGHT_PINK   = Vec4{255, 45, 85, 255} / 255;
constexpr Color DARK_PINK    = Vec4{255, 55, 95, 255} / 255;
constexpr Color LIGHT_PURPLE = Vec4{175, 82, 222, 255} / 255;
constexpr Color DARK_PURPLE  = Vec4{191, 90, 242, 255} / 255;
constexpr Color LIGHT_RED    = Vec4{255, 59, 48, 255} / 255;
constexpr Color DARK_RED     = Vec4{255, 69, 58, 255} / 255;
constexpr Color LIGHT_TEAL   = Vec4{48, 176, 199, 255} / 255;
constexpr Color DARK_TEAL    = Vec4{64, 200, 224, 255} / 255;
constexpr Color LIGHT_YELLOW = Vec4{255, 204, 0, 255} / 255;
constexpr Color DARK_YELLOW  = Vec4{255, 214, 10, 255} / 255;

constexpr Color LIGHT_GRAY   = Vec4{142, 142, 147, 255} / 255;
constexpr Color DARK_GRAY    = Vec4{142, 142, 147, 255} / 255;
constexpr Color LIGHT_GRAY_2 = Vec4{174, 174, 178, 255} / 255;
constexpr Color DARK_GRAY_2  = Vec4{99, 99, 102, 255} / 255;
constexpr Color LIGHT_GRAY_3 = Vec4{199, 199, 204, 255} / 255;
constexpr Color DARK_GRAY_3  = Vec4{72, 72, 74, 255} / 255;
constexpr Color LIGHT_GRAY_4 = Vec4{209, 209, 214, 255} / 255;
constexpr Color DARK_GRAY_4  = Vec4{58, 58, 60, 255} / 255;
constexpr Color LIGHT_GRAY_5 = Vec4{229, 229, 234, 255} / 255;
constexpr Color DARK_GRAY_5  = Vec4{44, 44, 46, 255} / 255;
constexpr Color LIGHT_GRAY_6 = Vec4{242, 242, 247, 255} / 255;
constexpr Color DARK_GRAY_6  = Vec4{28, 28, 30, 255} / 255;

// ios accessible colors
namespace accessible
{
constexpr Color LIGHT_BLUE   = Vec4{0, 64, 221, 255} / 255;
constexpr Color DARK_BLUE    = Vec4{64, 156, 255, 255} / 255;
constexpr Color LIGHT_BROWN  = Vec4{127, 101, 69, 255} / 255;
constexpr Color DARK_BROWN   = Vec4{181, 148, 105, 255} / 255;
constexpr Color LIGHT_CYAN   = Vec4{0, 113, 164, 255} / 255;
constexpr Color DARK_CYAN    = Vec4{112, 215, 255, 255} / 255;
constexpr Color LIGHT_GREEN  = Vec4{36, 138, 61, 255} / 255;
constexpr Color DARK_GREEN   = Vec4{48, 219, 91, 255} / 255;
constexpr Color LIGHT_INDIGO = Vec4{54, 52, 163, 255} / 255;
constexpr Color DARK_INDIGO  = Vec4{125, 122, 255, 255} / 255;
constexpr Color LIGHT_MINT   = Vec4{12, 129, 123, 255} / 255;
constexpr Color DARK_MINT    = Vec4{102, 212, 207, 255} / 255;
constexpr Color LIGHT_ORANGE = Vec4{201, 52, 0, 255} / 255;
constexpr Color DARK_ORANGE  = Vec4{255, 179, 64, 255} / 255;
constexpr Color LIGHT_PINK   = Vec4{211, 15, 69, 255} / 255;
constexpr Color DARK_PINK    = Vec4{255, 100, 130, 255} / 255;
constexpr Color LIGHT_PURPLE = Vec4{137, 68, 171, 255} / 255;
constexpr Color DARK_PURPLE  = Vec4{218, 143, 255, 255} / 255;
constexpr Color LIGHT_RED    = Vec4{215, 0, 21, 255} / 255;
constexpr Color DARK_RED     = Vec4{255, 105, 97, 255} / 255;
constexpr Color LIGHT_TEAL   = Vec4{0, 130, 153, 255} / 255;
constexpr Color DARK_TEAL    = Vec4{93, 230, 255, 255} / 255;
constexpr Color LIGHT_YELLOW = Vec4{178, 80, 0, 255} / 255;
constexpr Color DARK_YELLOW  = Vec4{255, 212, 38, 255} / 255;

constexpr Color LIGHT_GRAY   = Vec4{108, 108, 112, 255} / 255;
constexpr Color DARK_GRAY    = Vec4{174, 174, 178, 255} / 255;
constexpr Color LIGHT_GRAY_2 = Vec4{142, 142, 147, 255} / 255;
constexpr Color DARK_GRAY_2  = Vec4{124, 124, 128, 255} / 255;
constexpr Color LIGHT_GRAY_3 = Vec4{174, 174, 178, 255} / 255;
constexpr Color DARK_GRAY_3  = Vec4{84, 84, 86, 255} / 255;
constexpr Color LIGHT_GRAY_4 = Vec4{188, 188, 192, 255} / 255;
constexpr Color DARK_GRAY_4  = Vec4{68, 68, 70, 255} / 255;
constexpr Color LIGHT_GRAY_5 = Vec4{216, 216, 220, 255} / 255;
constexpr Color DARK_GRAY_5  = Vec4{54, 54, 56, 255} / 255;
constexpr Color LIGHT_GRAY_6 = Vec4{235, 235, 240, 255} / 255;
constexpr Color DARK_GRAY_6  = Vec4{36, 36, 36, 255} / 255;
}        // namespace accessible

}        // namespace ios

namespace material
{
constexpr Color RED_50   = Vec4{0xff, 0xeb, 0xee, 255} / 255;
constexpr Color RED_100  = Vec4{0xff, 0xcd, 0xd2, 255} / 255;
constexpr Color RED_200  = Vec4{0xef, 0x9a, 0x9a, 255} / 255;
constexpr Color RED_300  = Vec4{0xe5, 0x73, 0x73, 255} / 255;
constexpr Color RED_400  = Vec4{0xef, 0x53, 0x50, 255} / 255;
constexpr Color RED_500  = Vec4{0xf4, 0x43, 0x36, 255} / 255;
constexpr Color RED_600  = Vec4{0xe5, 0x39, 0x35, 255} / 255;
constexpr Color RED_700  = Vec4{0xd3, 0x2f, 0x2f, 255} / 255;
constexpr Color RED_800  = Vec4{0xc6, 0x28, 0x28, 255} / 255;
constexpr Color RED_900  = Vec4{0xb7, 0x1c, 0x1c, 255} / 255;
constexpr Color RED_A100 = Vec4{0xff, 0x8a, 0x80, 255} / 255;
constexpr Color RED_A200 = Vec4{0xff, 0x52, 0x52, 255} / 255;
constexpr Color RED_A400 = Vec4{0xff, 0x17, 0x44, 255} / 255;
constexpr Color RED_A700 = Vec4{0xd5, 0x00, 0x00, 255} / 255;

constexpr Color PINK_50   = Vec4{0xfc, 0xe4, 0xec, 255} / 255;
constexpr Color PINK_100  = Vec4{0xf8, 0xbb, 0xd0, 255} / 255;
constexpr Color PINK_200  = Vec4{0xf4, 0x8f, 0xb1, 255} / 255;
constexpr Color PINK_300  = Vec4{0xf0, 0x62, 0x92, 255} / 255;
constexpr Color PINK_400  = Vec4{0xec, 0x40, 0x7a, 255} / 255;
constexpr Color PINK_500  = Vec4{0xe9, 0x1e, 0x63, 255} / 255;
constexpr Color PINK_600  = Vec4{0xd8, 0x1b, 0x60, 255} / 255;
constexpr Color PINK_700  = Vec4{0xc2, 0x18, 0x5b, 255} / 255;
constexpr Color PINK_800  = Vec4{0xad, 0x14, 0x57, 255} / 255;
constexpr Color PINK_900  = Vec4{0x88, 0x0e, 0x4f, 255} / 255;
constexpr Color PINK_A100 = Vec4{0xff, 0x80, 0xab, 255} / 255;
constexpr Color PINK_A200 = Vec4{0xff, 0x40, 0x81, 255} / 255;
constexpr Color PINK_A400 = Vec4{0xf5, 0x00, 0x57, 255} / 255;
constexpr Color PINK_A700 = Vec4{0xc5, 0x11, 0x62, 255} / 255;

constexpr Color PURPLE_50   = Vec4{0xf3, 0xe5, 0xf5, 255} / 255;
constexpr Color PURPLE_100  = Vec4{0xe1, 0xbe, 0xe7, 255} / 255;
constexpr Color PURPLE_200  = Vec4{0xce, 0x93, 0xd8, 255} / 255;
constexpr Color PURPLE_300  = Vec4{0xba, 0x68, 0xc8, 255} / 255;
constexpr Color PURPLE_400  = Vec4{0xab, 0x47, 0xbc, 255} / 255;
constexpr Color PURPLE_500  = Vec4{0x9c, 0x27, 0xb0, 255} / 255;
constexpr Color PURPLE_600  = Vec4{0x8e, 0x24, 0xaa, 255} / 255;
constexpr Color PURPLE_700  = Vec4{0x7b, 0x1f, 0xa2, 255} / 255;
constexpr Color PURPLE_800  = Vec4{0x6a, 0x1b, 0x9a, 255} / 255;
constexpr Color PURPLE_900  = Vec4{0x4a, 0x14, 0x8c, 255} / 255;
constexpr Color PURPLE_A100 = Vec4{0xea, 0x80, 0xfc, 255} / 255;
constexpr Color PURPLE_A200 = Vec4{0xe0, 0x40, 0xfb, 255} / 255;
constexpr Color PURPLE_A400 = Vec4{0xd5, 0x00, 0xf9, 255} / 255;
constexpr Color PURPLE_A700 = Vec4{0xaa, 0x00, 0xff, 255} / 255;

constexpr Color DEEP_PURPLE_50   = Vec4{0xed, 0xe7, 0xf6, 255} / 255;
constexpr Color DEEP_PURPLE_100  = Vec4{0xd1, 0xc4, 0xe9, 255} / 255;
constexpr Color DEEP_PURPLE_200  = Vec4{0xb3, 0x9d, 0xdb, 255} / 255;
constexpr Color DEEP_PURPLE_300  = Vec4{0x95, 0x75, 0xcd, 255} / 255;
constexpr Color DEEP_PURPLE_400  = Vec4{0x7e, 0x57, 0xc2, 255} / 255;
constexpr Color DEEP_PURPLE_500  = Vec4{0x67, 0x3a, 0xb7, 255} / 255;
constexpr Color DEEP_PURPLE_600  = Vec4{0x5e, 0x35, 0xb1, 255} / 255;
constexpr Color DEEP_PURPLE_700  = Vec4{0x51, 0x2d, 0xa8, 255} / 255;
constexpr Color DEEP_PURPLE_800  = Vec4{0x45, 0x27, 0xa0, 255} / 255;
constexpr Color DEEP_PURPLE_900  = Vec4{0x31, 0x1b, 0x92, 255} / 255;
constexpr Color DEEP_PURPLE_A100 = Vec4{0xb3, 0x88, 0xff, 255} / 255;
constexpr Color DEEP_PURPLE_A200 = Vec4{0x7c, 0x4d, 0xff, 255} / 255;
constexpr Color DEEP_PURPLE_A400 = Vec4{0x65, 0x1f, 0xff, 255} / 255;
constexpr Color DEEP_PURPLE_A700 = Vec4{0x62, 0x00, 0xea, 255} / 255;

constexpr Color INDIGO_50   = Vec4{0xe8, 0xea, 0xf6, 255} / 255;
constexpr Color INDIGO_100  = Vec4{0xc5, 0xca, 0xe9, 255} / 255;
constexpr Color INDIGO_200  = Vec4{0x9f, 0xa8, 0xda, 255} / 255;
constexpr Color INDIGO_300  = Vec4{0x79, 0x86, 0xcb, 255} / 255;
constexpr Color INDIGO_400  = Vec4{0x5c, 0x6b, 0xc0, 255} / 255;
constexpr Color INDIGO_500  = Vec4{0x3f, 0x51, 0xb5, 255} / 255;
constexpr Color INDIGO_600  = Vec4{0x39, 0x49, 0xab, 255} / 255;
constexpr Color INDIGO_700  = Vec4{0x30, 0x3f, 0x9f, 255} / 255;
constexpr Color INDIGO_800  = Vec4{0x28, 0x35, 0x93, 255} / 255;
constexpr Color INDIGO_900  = Vec4{0x1a, 0x23, 0x7e, 255} / 255;
constexpr Color INDIGO_A100 = Vec4{0x8c, 0x9e, 0xff, 255} / 255;
constexpr Color INDIGO_A200 = Vec4{0x53, 0x6d, 0xfe, 255} / 255;
constexpr Color INDIGO_A400 = Vec4{0x3d, 0x5a, 0xfe, 255} / 255;
constexpr Color INDIGO_A700 = Vec4{0x30, 0x4f, 0xfe, 255} / 255;

constexpr Color BLUE_50   = Vec4{0xe3, 0xf2, 0xfd, 255} / 255;
constexpr Color BLUE_100  = Vec4{0xbb, 0xde, 0xfb, 255} / 255;
constexpr Color BLUE_200  = Vec4{0x90, 0xca, 0xf9, 255} / 255;
constexpr Color BLUE_300  = Vec4{0x64, 0xb5, 0xf6, 255} / 255;
constexpr Color BLUE_400  = Vec4{0x42, 0xa5, 0xf5, 255} / 255;
constexpr Color BLUE_500  = Vec4{0x21, 0x96, 0xf3, 255} / 255;
constexpr Color BLUE_600  = Vec4{0x1e, 0x88, 0xe5, 255} / 255;
constexpr Color BLUE_700  = Vec4{0x19, 0x76, 0xd2, 255} / 255;
constexpr Color BLUE_800  = Vec4{0x15, 0x65, 0xc0, 255} / 255;
constexpr Color BLUE_900  = Vec4{0x0d, 0x47, 0xa1, 255} / 255;
constexpr Color BLUE_A100 = Vec4{0x82, 0xb1, 0xff, 255} / 255;
constexpr Color BLUE_A200 = Vec4{0x44, 0x8a, 0xff, 255} / 255;
constexpr Color BLUE_A400 = Vec4{0x29, 0x79, 0xff, 255} / 255;
constexpr Color BLUE_A700 = Vec4{0x29, 0x62, 0xff, 255} / 255;

constexpr Color LIGHT_BLUE_50   = Vec4{0xe1, 0xf5, 0xfe, 255} / 255;
constexpr Color LIGHT_BLUE_100  = Vec4{0xb3, 0xe5, 0xfc, 255} / 255;
constexpr Color LIGHT_BLUE_200  = Vec4{0x81, 0xd4, 0xfa, 255} / 255;
constexpr Color LIGHT_BLUE_300  = Vec4{0x4f, 0xc3, 0xf7, 255} / 255;
constexpr Color LIGHT_BLUE_400  = Vec4{0x29, 0xb6, 0xf6, 255} / 255;
constexpr Color LIGHT_BLUE_500  = Vec4{0x03, 0xa9, 0xf4, 255} / 255;
constexpr Color LIGHT_BLUE_600  = Vec4{0x03, 0x9b, 0xe5, 255} / 255;
constexpr Color LIGHT_BLUE_700  = Vec4{0x02, 0x88, 0xd1, 255} / 255;
constexpr Color LIGHT_BLUE_800  = Vec4{0x02, 0x77, 0xbd, 255} / 255;
constexpr Color LIGHT_BLUE_900  = Vec4{0x01, 0x57, 0x9b, 255} / 255;
constexpr Color LIGHT_BLUE_A100 = Vec4{0x80, 0xd8, 0xff, 255} / 255;
constexpr Color LIGHT_BLUE_A200 = Vec4{0x40, 0xc4, 0xff, 255} / 255;
constexpr Color LIGHT_BLUE_A400 = Vec4{0x00, 0xb0, 0xff, 255} / 255;
constexpr Color LIGHT_BLUE_A700 = Vec4{0x00, 0x91, 0xea, 255} / 255;

constexpr Color CYAN_50   = Vec4{0xe0, 0xf7, 0xfa, 255} / 255;
constexpr Color CYAN_100  = Vec4{0xb2, 0xeb, 0xf2, 255} / 255;
constexpr Color CYAN_200  = Vec4{0x80, 0xde, 0xea, 255} / 255;
constexpr Color CYAN_300  = Vec4{0x4d, 0xd0, 0xe1, 255} / 255;
constexpr Color CYAN_400  = Vec4{0x26, 0xc6, 0xda, 255} / 255;
constexpr Color CYAN_500  = Vec4{0x00, 0xbc, 0xd4, 255} / 255;
constexpr Color CYAN_600  = Vec4{0x00, 0xac, 0xc1, 255} / 255;
constexpr Color CYAN_700  = Vec4{0x00, 0x97, 0xa7, 255} / 255;
constexpr Color CYAN_800  = Vec4{0x00, 0x83, 0x8f, 255} / 255;
constexpr Color CYAN_900  = Vec4{0x00, 0x60, 0x64, 255} / 255;
constexpr Color CYAN_A100 = Vec4{0x84, 0xff, 0xff, 255} / 255;
constexpr Color CYAN_A200 = Vec4{0x18, 0xff, 0xff, 255} / 255;
constexpr Color CYAN_A400 = Vec4{0x00, 0xe5, 0xff, 255} / 255;
constexpr Color CYAN_A700 = Vec4{0x00, 0xb8, 0xd4, 255} / 255;

constexpr Color TEAL_50   = Vec4{0xe0, 0xf2, 0xf1, 255} / 255;
constexpr Color TEAL_100  = Vec4{0xb2, 0xdf, 0xdb, 255} / 255;
constexpr Color TEAL_200  = Vec4{0x80, 0xcb, 0xc4, 255} / 255;
constexpr Color TEAL_300  = Vec4{0x4d, 0xb6, 0xac, 255} / 255;
constexpr Color TEAL_400  = Vec4{0x26, 0xa6, 0x9a, 255} / 255;
constexpr Color TEAL_500  = Vec4{0x00, 0x96, 0x88, 255} / 255;
constexpr Color TEAL_600  = Vec4{0x00, 0x89, 0x7b, 255} / 255;
constexpr Color TEAL_700  = Vec4{0x00, 0x79, 0x6b, 255} / 255;
constexpr Color TEAL_800  = Vec4{0x00, 0x69, 0x5c, 255} / 255;
constexpr Color TEAL_900  = Vec4{0x00, 0x4d, 0x40, 255} / 255;
constexpr Color TEAL_A100 = Vec4{0xa7, 0xff, 0xeb, 255} / 255;
constexpr Color TEAL_A200 = Vec4{0x64, 0xff, 0xda, 255} / 255;
constexpr Color TEAL_A400 = Vec4{0x1d, 0xe9, 0xb6, 255} / 255;
constexpr Color TEAL_A700 = Vec4{0x00, 0xbf, 0xa5, 255} / 255;

constexpr Color GREEN_50   = Vec4{0xe8, 0xf5, 0xe9, 255} / 255;
constexpr Color GREEN_100  = Vec4{0xc8, 0xe6, 0xc9, 255} / 255;
constexpr Color GREEN_200  = Vec4{0xa5, 0xd6, 0xa7, 255} / 255;
constexpr Color GREEN_300  = Vec4{0x81, 0xc7, 0x84, 255} / 255;
constexpr Color GREEN_400  = Vec4{0x66, 0xbb, 0x6a, 255} / 255;
constexpr Color GREEN_500  = Vec4{0x4c, 0xaf, 0x50, 255} / 255;
constexpr Color GREEN_600  = Vec4{0x43, 0xa0, 0x47, 255} / 255;
constexpr Color GREEN_700  = Vec4{0x38, 0x8e, 0x3c, 255} / 255;
constexpr Color GREEN_800  = Vec4{0x2e, 0x7d, 0x32, 255} / 255;
constexpr Color GREEN_900  = Vec4{0x1b, 0x5e, 0x20, 255} / 255;
constexpr Color GREEN_A100 = Vec4{0xb9, 0xf6, 0xca, 255} / 255;
constexpr Color GREEN_A200 = Vec4{0x69, 0xf0, 0xae, 255} / 255;
constexpr Color GREEN_A400 = Vec4{0x00, 0xe6, 0x76, 255} / 255;
constexpr Color GREEN_A700 = Vec4{0x00, 0xc8, 0x53, 255} / 255;

constexpr Color LIGHT_GREEN_50   = Vec4{0xf1, 0xf8, 0xe9, 255} / 255;
constexpr Color LIGHT_GREEN_100  = Vec4{0xdc, 0xed, 0xc8, 255} / 255;
constexpr Color LIGHT_GREEN_200  = Vec4{0xc5, 0xe1, 0xa5, 255} / 255;
constexpr Color LIGHT_GREEN_300  = Vec4{0xae, 0xd5, 0x81, 255} / 255;
constexpr Color LIGHT_GREEN_400  = Vec4{0x9c, 0xcc, 0x65, 255} / 255;
constexpr Color LIGHT_GREEN_500  = Vec4{0x8b, 0xc3, 0x4a, 255} / 255;
constexpr Color LIGHT_GREEN_600  = Vec4{0x7c, 0xb3, 0x42, 255} / 255;
constexpr Color LIGHT_GREEN_700  = Vec4{0x68, 0x9f, 0x38, 255} / 255;
constexpr Color LIGHT_GREEN_800  = Vec4{0x55, 0x8b, 0x2f, 255} / 255;
constexpr Color LIGHT_GREEN_900  = Vec4{0x33, 0x69, 0x1e, 255} / 255;
constexpr Color LIGHT_GREEN_A100 = Vec4{0xcc, 0xff, 0x90, 255} / 255;
constexpr Color LIGHT_GREEN_A200 = Vec4{0xb2, 0xff, 0x59, 255} / 255;
constexpr Color LIGHT_GREEN_A400 = Vec4{0x76, 0xff, 0x03, 255} / 255;
constexpr Color LIGHT_GREEN_A700 = Vec4{0x64, 0xdd, 0x17, 255} / 255;

constexpr Color LIME_50   = Vec4{0xf9, 0xfb, 0xe7, 255} / 255;
constexpr Color LIME_100  = Vec4{0xf0, 0xf4, 0xc3, 255} / 255;
constexpr Color LIME_200  = Vec4{0xe6, 0xee, 0x9c, 255} / 255;
constexpr Color LIME_300  = Vec4{0xdc, 0xe7, 0x75, 255} / 255;
constexpr Color LIME_400  = Vec4{0xd4, 0xe1, 0x57, 255} / 255;
constexpr Color LIME_500  = Vec4{0xcd, 0xdc, 0x39, 255} / 255;
constexpr Color LIME_600  = Vec4{0xc0, 0xca, 0x33, 255} / 255;
constexpr Color LIME_700  = Vec4{0xaf, 0xb4, 0x2b, 255} / 255;
constexpr Color LIME_800  = Vec4{0x9e, 0x9d, 0x24, 255} / 255;
constexpr Color LIME_900  = Vec4{0x82, 0x77, 0x17, 255} / 255;
constexpr Color LIME_A100 = Vec4{0xf4, 0xff, 0x81, 255} / 255;
constexpr Color LIME_A200 = Vec4{0xee, 0xff, 0x41, 255} / 255;
constexpr Color LIME_A400 = Vec4{0xc6, 0xff, 0x00, 255} / 255;
constexpr Color LIME_A700 = Vec4{0xae, 0xea, 0x00, 255} / 255;

constexpr Color YELLOW_50   = Vec4{0xff, 0xfd, 0xe7, 255} / 255;
constexpr Color YELLOW_100  = Vec4{0xff, 0xf9, 0xc4, 255} / 255;
constexpr Color YELLOW_200  = Vec4{0xff, 0xf5, 0x9d, 255} / 255;
constexpr Color YELLOW_300  = Vec4{0xff, 0xf1, 0x76, 255} / 255;
constexpr Color YELLOW_400  = Vec4{0xff, 0xee, 0x58, 255} / 255;
constexpr Color YELLOW_500  = Vec4{0xff, 0xeb, 0x3b, 255} / 255;
constexpr Color YELLOW_600  = Vec4{0xfd, 0xd8, 0x35, 255} / 255;
constexpr Color YELLOW_700  = Vec4{0xfb, 0xc0, 0x2d, 255} / 255;
constexpr Color YELLOW_800  = Vec4{0xf9, 0xa8, 0x25, 255} / 255;
constexpr Color YELLOW_900  = Vec4{0xf5, 0x7f, 0x17, 255} / 255;
constexpr Color YELLOW_A100 = Vec4{0xff, 0xff, 0x8d, 255} / 255;
constexpr Color YELLOW_A200 = Vec4{0xff, 0xff, 0x00, 255} / 255;
constexpr Color YELLOW_A400 = Vec4{0xff, 0xea, 0x00, 255} / 255;
constexpr Color YELLOW_A700 = Vec4{0xff, 0xd6, 0x00, 255} / 255;

constexpr Color AMBER_50   = Vec4{0xff, 0xf8, 0xe1, 255} / 255;
constexpr Color AMBER_100  = Vec4{0xff, 0xec, 0xb3, 255} / 255;
constexpr Color AMBER_200  = Vec4{0xff, 0xe0, 0x82, 255} / 255;
constexpr Color AMBER_300  = Vec4{0xff, 0xd5, 0x4f, 255} / 255;
constexpr Color AMBER_400  = Vec4{0xff, 0xca, 0x28, 255} / 255;
constexpr Color AMBER_500  = Vec4{0xff, 0xc1, 0x07, 255} / 255;
constexpr Color AMBER_600  = Vec4{0xff, 0xb3, 0x00, 255} / 255;
constexpr Color AMBER_700  = Vec4{0xff, 0xa0, 0x00, 255} / 255;
constexpr Color AMBER_800  = Vec4{0xff, 0x8f, 0x00, 255} / 255;
constexpr Color AMBER_900  = Vec4{0xff, 0x6f, 0x00, 255} / 255;
constexpr Color AMBER_A100 = Vec4{0xff, 0xe5, 0x7f, 255} / 255;
constexpr Color AMBER_A200 = Vec4{0xff, 0xd7, 0x40, 255} / 255;
constexpr Color AMBER_A400 = Vec4{0xff, 0xc4, 0x00, 255} / 255;
constexpr Color AMBER_A700 = Vec4{0xff, 0xab, 0x00, 255} / 255;

constexpr Color ORANGE_50   = Vec4{0xff, 0xf3, 0xe0, 255} / 255;
constexpr Color ORANGE_100  = Vec4{0xff, 0xe0, 0xb2, 255} / 255;
constexpr Color ORANGE_200  = Vec4{0xff, 0xcc, 0x80, 255} / 255;
constexpr Color ORANGE_300  = Vec4{0xff, 0xb7, 0x4d, 255} / 255;
constexpr Color ORANGE_400  = Vec4{0xff, 0xa7, 0x26, 255} / 255;
constexpr Color ORANGE_500  = Vec4{0xff, 0x98, 0x00, 255} / 255;
constexpr Color ORANGE_600  = Vec4{0xfb, 0x8c, 0x00, 255} / 255;
constexpr Color ORANGE_700  = Vec4{0xf5, 0x7c, 0x00, 255} / 255;
constexpr Color ORANGE_800  = Vec4{0xef, 0x6c, 0x00, 255} / 255;
constexpr Color ORANGE_900  = Vec4{0xe6, 0x51, 0x00, 255} / 255;
constexpr Color ORANGE_A100 = Vec4{0xff, 0xd1, 0x80, 255} / 255;
constexpr Color ORANGE_A200 = Vec4{0xff, 0xab, 0x40, 255} / 255;
constexpr Color ORANGE_A400 = Vec4{0xff, 0x91, 0x00, 255} / 255;
constexpr Color ORANGE_A700 = Vec4{0xff, 0x6d, 0x00, 255} / 255;

constexpr Color DEEP_ORANGE_50   = Vec4{0xfb, 0xe9, 0xe7, 255} / 255;
constexpr Color DEEP_ORANGE_100  = Vec4{0xff, 0xcc, 0xbc, 255} / 255;
constexpr Color DEEP_ORANGE_200  = Vec4{0xff, 0xab, 0x91, 255} / 255;
constexpr Color DEEP_ORANGE_300  = Vec4{0xff, 0x8a, 0x65, 255} / 255;
constexpr Color DEEP_ORANGE_400  = Vec4{0xff, 0x70, 0x43, 255} / 255;
constexpr Color DEEP_ORANGE_500  = Vec4{0xff, 0x57, 0x22, 255} / 255;
constexpr Color DEEP_ORANGE_600  = Vec4{0xf4, 0x51, 0x1e, 255} / 255;
constexpr Color DEEP_ORANGE_700  = Vec4{0xe6, 0x4a, 0x19, 255} / 255;
constexpr Color DEEP_ORANGE_800  = Vec4{0xd8, 0x43, 0x15, 255} / 255;
constexpr Color DEEP_ORANGE_900  = Vec4{0xbf, 0x36, 0x0c, 255} / 255;
constexpr Color DEEP_ORANGE_A100 = Vec4{0xff, 0x9e, 0x80, 255} / 255;
constexpr Color DEEP_ORANGE_A200 = Vec4{0xff, 0x6e, 0x40, 255} / 255;
constexpr Color DEEP_ORANGE_A400 = Vec4{0xff, 0x3d, 0x00, 255} / 255;
constexpr Color DEEP_ORANGE_A700 = Vec4{0xdd, 0x2c, 0x00, 255} / 255;

constexpr Color BROWN_50  = Vec4{0xef, 0xeb, 0xe9, 255} / 255;
constexpr Color BROWN_100 = Vec4{0xd7, 0xcc, 0xc8, 255} / 255;
constexpr Color BROWN_200 = Vec4{0xbc, 0xaa, 0xa4, 255} / 255;
constexpr Color BROWN_300 = Vec4{0xa1, 0x88, 0x7f, 255} / 255;
constexpr Color BROWN_400 = Vec4{0x8d, 0x6e, 0x63, 255} / 255;
constexpr Color BROWN_500 = Vec4{0x79, 0x55, 0x48, 255} / 255;
constexpr Color BROWN_600 = Vec4{0x6d, 0x4c, 0x41, 255} / 255;
constexpr Color BROWN_700 = Vec4{0x5d, 0x40, 0x37, 255} / 255;
constexpr Color BROWN_800 = Vec4{0x4e, 0x34, 0x2e, 255} / 255;
constexpr Color BROWN_900 = Vec4{0x3e, 0x27, 0x23, 255} / 255;

constexpr Color GRAY_50  = Vec4{0xfa, 0xfa, 0xfa, 255} / 255;
constexpr Color GRAY_100 = Vec4{0xf5, 0xf5, 0xf5, 255} / 255;
constexpr Color GRAY_200 = Vec4{0xee, 0xee, 0xee, 255} / 255;
constexpr Color GRAY_300 = Vec4{0xe0, 0xe0, 0xe0, 255} / 255;
constexpr Color GRAY_400 = Vec4{0xbd, 0xbd, 0xbd, 255} / 255;
constexpr Color GRAY_500 = Vec4{0x9e, 0x9e, 0x9e, 255} / 255;
constexpr Color GRAY_600 = Vec4{0x75, 0x75, 0x75, 255} / 255;
constexpr Color GRAY_700 = Vec4{0x61, 0x61, 0x61, 255} / 255;
constexpr Color GRAY_800 = Vec4{0x42, 0x42, 0x42, 255} / 255;
constexpr Color GRAY_900 = Vec4{0x21, 0x21, 0x21, 255} / 255;

constexpr Color BLUE_GRAY_50  = Vec4{0xec, 0xef, 0xf1, 255} / 255;
constexpr Color BLUE_GRAY_100 = Vec4{0xcf, 0xd8, 0xdc, 255} / 255;
constexpr Color BLUE_GRAY_200 = Vec4{0xb0, 0xbe, 0xc5, 255} / 255;
constexpr Color BLUE_GRAY_300 = Vec4{0x90, 0xa4, 0xae, 255} / 255;
constexpr Color BLUE_GRAY_400 = Vec4{0x78, 0x90, 0x9c, 255} / 255;
constexpr Color BLUE_GRAY_500 = Vec4{0x60, 0x7d, 0x8b, 255} / 255;
constexpr Color BLUE_GRAY_600 = Vec4{0x54, 0x6e, 0x7a, 255} / 255;
constexpr Color BLUE_GRAY_700 = Vec4{0x45, 0x5a, 0x64, 255} / 255;
constexpr Color BLUE_GRAY_800 = Vec4{0x37, 0x47, 0x4f, 255} / 255;
constexpr Color BLUE_GRAY_900 = Vec4{0x26, 0x32, 0x38, 255} / 255;

constexpr Color WHITE = Vec4{0xff, 0xff, 0xff, 255} / 255;
constexpr Color BLACK = Vec4{0x00, 0x00, 0x00, 255} / 255;

}        // namespace material

}        // namespace ash
