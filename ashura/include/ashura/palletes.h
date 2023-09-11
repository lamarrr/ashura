
#pragma once

#include "ashura/primitives.h"

namespace ash
{

// ios default system colors
namespace ios
{
constexpr Color LIGHT_BLUE   = Color::from_rgb(0, 122, 255);
constexpr Color DARK_BLUE    = Color::from_rgb(10, 132, 255);
constexpr Color LIGHT_BROWN  = Color::from_rgb(162, 132, 94);
constexpr Color DARK_BROWN   = Color::from_rgb(172, 142, 104);
constexpr Color LIGHT_CYAN   = Color::from_rgb(50, 173, 230);
constexpr Color DARK_CYAN    = Color::from_rgb(100, 210, 255);
constexpr Color LIGHT_GREEN  = Color::from_rgb(52, 199, 89);
constexpr Color DARK_GREEN   = Color::from_rgb(48, 209, 88);
constexpr Color LIGHT_INDIGO = Color::from_rgb(88, 86, 214);
constexpr Color DARK_INDIGO  = Color::from_rgb(94, 92, 230);
constexpr Color LIGHT_MINT   = Color::from_rgb(0, 199, 190);
constexpr Color DARK_MINT    = Color::from_rgb(102, 212, 207);
constexpr Color LIGHT_ORANGE = Color::from_rgb(255, 149, 0);
constexpr Color DARK_ORANGE  = Color::from_rgb(255, 159, 10);
constexpr Color LIGHT_PINK   = Color::from_rgb(255, 45, 85);
constexpr Color DARK_PINK    = Color::from_rgb(255, 55, 95);
constexpr Color LIGHT_PURPLE = Color::from_rgb(175, 82, 222);
constexpr Color DARK_PURPLE  = Color::from_rgb(191, 90, 242);
constexpr Color LIGHT_RED    = Color::from_rgb(255, 59, 48);
constexpr Color DARK_RED     = Color::from_rgb(255, 69, 58);
constexpr Color LIGHT_TEAL   = Color::from_rgb(48, 176, 199);
constexpr Color DARK_TEAL    = Color::from_rgb(64, 200, 224);
constexpr Color LIGHT_YELLOW = Color::from_rgb(255, 204, 0);
constexpr Color DARK_YELLOW  = Color::from_rgb(255, 214, 10);

constexpr Color LIGHT_GRAY   = Color::from_rgb(142, 142, 147);
constexpr Color DARK_GRAY    = Color::from_rgb(142, 142, 147);
constexpr Color LIGHT_GRAY_2 = Color::from_rgb(174, 174, 178);
constexpr Color DARK_GRAY_2  = Color::from_rgb(99, 99, 102);
constexpr Color LIGHT_GRAY_3 = Color::from_rgb(199, 199, 204);
constexpr Color DARK_GRAY_3  = Color::from_rgb(72, 72, 74);
constexpr Color LIGHT_GRAY_4 = Color::from_rgb(209, 209, 214);
constexpr Color DARK_GRAY_4  = Color::from_rgb(58, 58, 60);
constexpr Color LIGHT_GRAY_5 = Color::from_rgb(229, 229, 234);
constexpr Color DARK_GRAY_5  = Color::from_rgb(44, 44, 46);
constexpr Color LIGHT_GRAY_6 = Color::from_rgb(242, 242, 247);
constexpr Color DARK_GRAY_6  = Color::from_rgb(28, 28, 30);

// ios accessible colors
namespace accessible
{
constexpr Color LIGHT_BLUE   = Color::from_rgb(0, 64, 221);
constexpr Color DARK_BLUE    = Color::from_rgb(64, 156, 255);
constexpr Color LIGHT_BROWN  = Color::from_rgb(127, 101, 69);
constexpr Color DARK_BROWN   = Color::from_rgb(181, 148, 105);
constexpr Color LIGHT_CYAN   = Color::from_rgb(0, 113, 164);
constexpr Color DARK_CYAN    = Color::from_rgb(112, 215, 255);
constexpr Color LIGHT_GREEN  = Color::from_rgb(36, 138, 61);
constexpr Color DARK_GREEN   = Color::from_rgb(48, 219, 91);
constexpr Color LIGHT_INDIGO = Color::from_rgb(54, 52, 163);
constexpr Color DARK_INDIGO  = Color::from_rgb(125, 122, 255);
constexpr Color LIGHT_MINT   = Color::from_rgb(12, 129, 123);
constexpr Color DARK_MINT    = Color::from_rgb(102, 212, 207);
constexpr Color LIGHT_ORANGE = Color::from_rgb(201, 52, 0);
constexpr Color DARK_ORANGE  = Color::from_rgb(255, 179, 64);
constexpr Color LIGHT_PINK   = Color::from_rgb(211, 15, 69);
constexpr Color DARK_PINK    = Color::from_rgb(255, 100, 130);
constexpr Color LIGHT_PURPLE = Color::from_rgb(137, 68, 171);
constexpr Color DARK_PURPLE  = Color::from_rgb(218, 143, 255);
constexpr Color LIGHT_RED    = Color::from_rgb(215, 0, 21);
constexpr Color DARK_RED     = Color::from_rgb(255, 105, 97);
constexpr Color LIGHT_TEAL   = Color::from_rgb(0, 130, 153);
constexpr Color DARK_TEAL    = Color::from_rgb(93, 230, 255);
constexpr Color LIGHT_YELLOW = Color::from_rgb(178, 80, 0);
constexpr Color DARK_YELLOW  = Color::from_rgb(255, 212, 38);

constexpr Color LIGHT_GRAY   = Color::from_rgb(108, 108, 112);
constexpr Color DARK_GRAY    = Color::from_rgb(174, 174, 178);
constexpr Color LIGHT_GRAY_2 = Color::from_rgb(142, 142, 147);
constexpr Color DARK_GRAY_2  = Color::from_rgb(124, 124, 128);
constexpr Color LIGHT_GRAY_3 = Color::from_rgb(174, 174, 178);
constexpr Color DARK_GRAY_3  = Color::from_rgb(84, 84, 86);
constexpr Color LIGHT_GRAY_4 = Color::from_rgb(188, 188, 192);
constexpr Color DARK_GRAY_4  = Color::from_rgb(68, 68, 70);
constexpr Color LIGHT_GRAY_5 = Color::from_rgb(216, 216, 220);
constexpr Color DARK_GRAY_5  = Color::from_rgb(54, 54, 56);
constexpr Color LIGHT_GRAY_6 = Color::from_rgb(235, 235, 240);
constexpr Color DARK_GRAY_6  = Color::from_rgb(36, 36, 36);
}        // namespace accessible

}        // namespace ios

namespace material
{
constexpr Color RED_50   = Color::from_rgb(0xff, 0xeb, 0xee);
constexpr Color RED_100  = Color::from_rgb(0xff, 0xcd, 0xd2);
constexpr Color RED_200  = Color::from_rgb(0xef, 0x9a, 0x9a);
constexpr Color RED_300  = Color::from_rgb(0xe5, 0x73, 0x73);
constexpr Color RED_400  = Color::from_rgb(0xef, 0x53, 0x50);
constexpr Color RED_500  = Color::from_rgb(0xf4, 0x43, 0x36);
constexpr Color RED_600  = Color::from_rgb(0xe5, 0x39, 0x35);
constexpr Color RED_700  = Color::from_rgb(0xd3, 0x2f, 0x2f);
constexpr Color RED_800  = Color::from_rgb(0xc6, 0x28, 0x28);
constexpr Color RED_900  = Color::from_rgb(0xb7, 0x1c, 0x1c);
constexpr Color RED_A100 = Color::from_rgb(0xff, 0x8a, 0x80);
constexpr Color RED_A200 = Color::from_rgb(0xff, 0x52, 0x52);
constexpr Color RED_A400 = Color::from_rgb(0xff, 0x17, 0x44);
constexpr Color RED_A700 = Color::from_rgb(0xd5, 0x00, 0x00);

constexpr Color PINK_50   = Color::from_rgb(0xfc, 0xe4, 0xec);
constexpr Color PINK_100  = Color::from_rgb(0xf8, 0xbb, 0xd0);
constexpr Color PINK_200  = Color::from_rgb(0xf4, 0x8f, 0xb1);
constexpr Color PINK_300  = Color::from_rgb(0xf0, 0x62, 0x92);
constexpr Color PINK_400  = Color::from_rgb(0xec, 0x40, 0x7a);
constexpr Color PINK_500  = Color::from_rgb(0xe9, 0x1e, 0x63);
constexpr Color PINK_600  = Color::from_rgb(0xd8, 0x1b, 0x60);
constexpr Color PINK_700  = Color::from_rgb(0xc2, 0x18, 0x5b);
constexpr Color PINK_800  = Color::from_rgb(0xad, 0x14, 0x57);
constexpr Color PINK_900  = Color::from_rgb(0x88, 0x0e, 0x4f);
constexpr Color PINK_A100 = Color::from_rgb(0xff, 0x80, 0xab);
constexpr Color PINK_A200 = Color::from_rgb(0xff, 0x40, 0x81);
constexpr Color PINK_A400 = Color::from_rgb(0xf5, 0x00, 0x57);
constexpr Color PINK_A700 = Color::from_rgb(0xc5, 0x11, 0x62);

constexpr Color PURPLE_50   = Color::from_rgb(0xf3, 0xe5, 0xf5);
constexpr Color PURPLE_100  = Color::from_rgb(0xe1, 0xbe, 0xe7);
constexpr Color PURPLE_200  = Color::from_rgb(0xce, 0x93, 0xd8);
constexpr Color PURPLE_300  = Color::from_rgb(0xba, 0x68, 0xc8);
constexpr Color PURPLE_400  = Color::from_rgb(0xab, 0x47, 0xbc);
constexpr Color PURPLE_500  = Color::from_rgb(0x9c, 0x27, 0xb0);
constexpr Color PURPLE_600  = Color::from_rgb(0x8e, 0x24, 0xaa);
constexpr Color PURPLE_700  = Color::from_rgb(0x7b, 0x1f, 0xa2);
constexpr Color PURPLE_800  = Color::from_rgb(0x6a, 0x1b, 0x9a);
constexpr Color PURPLE_900  = Color::from_rgb(0x4a, 0x14, 0x8c);
constexpr Color PURPLE_A100 = Color::from_rgb(0xea, 0x80, 0xfc);
constexpr Color PURPLE_A200 = Color::from_rgb(0xe0, 0x40, 0xfb);
constexpr Color PURPLE_A400 = Color::from_rgb(0xd5, 0x00, 0xf9);
constexpr Color PURPLE_A700 = Color::from_rgb(0xaa, 0x00, 0xff);

constexpr Color DEEP_PURPLE_50   = Color::from_rgb(0xed, 0xe7, 0xf6);
constexpr Color DEEP_PURPLE_100  = Color::from_rgb(0xd1, 0xc4, 0xe9);
constexpr Color DEEP_PURPLE_200  = Color::from_rgb(0xb3, 0x9d, 0xdb);
constexpr Color DEEP_PURPLE_300  = Color::from_rgb(0x95, 0x75, 0xcd);
constexpr Color DEEP_PURPLE_400  = Color::from_rgb(0x7e, 0x57, 0xc2);
constexpr Color DEEP_PURPLE_500  = Color::from_rgb(0x67, 0x3a, 0xb7);
constexpr Color DEEP_PURPLE_600  = Color::from_rgb(0x5e, 0x35, 0xb1);
constexpr Color DEEP_PURPLE_700  = Color::from_rgb(0x51, 0x2d, 0xa8);
constexpr Color DEEP_PURPLE_800  = Color::from_rgb(0x45, 0x27, 0xa0);
constexpr Color DEEP_PURPLE_900  = Color::from_rgb(0x31, 0x1b, 0x92);
constexpr Color DEEP_PURPLE_A100 = Color::from_rgb(0xb3, 0x88, 0xff);
constexpr Color DEEP_PURPLE_A200 = Color::from_rgb(0x7c, 0x4d, 0xff);
constexpr Color DEEP_PURPLE_A400 = Color::from_rgb(0x65, 0x1f, 0xff);
constexpr Color DEEP_PURPLE_A700 = Color::from_rgb(0x62, 0x00, 0xea);

constexpr Color INDIGO_50   = Color::from_rgb(0xe8, 0xea, 0xf6);
constexpr Color INDIGO_100  = Color::from_rgb(0xc5, 0xca, 0xe9);
constexpr Color INDIGO_200  = Color::from_rgb(0x9f, 0xa8, 0xda);
constexpr Color INDIGO_300  = Color::from_rgb(0x79, 0x86, 0xcb);
constexpr Color INDIGO_400  = Color::from_rgb(0x5c, 0x6b, 0xc0);
constexpr Color INDIGO_500  = Color::from_rgb(0x3f, 0x51, 0xb5);
constexpr Color INDIGO_600  = Color::from_rgb(0x39, 0x49, 0xab);
constexpr Color INDIGO_700  = Color::from_rgb(0x30, 0x3f, 0x9f);
constexpr Color INDIGO_800  = Color::from_rgb(0x28, 0x35, 0x93);
constexpr Color INDIGO_900  = Color::from_rgb(0x1a, 0x23, 0x7e);
constexpr Color INDIGO_A100 = Color::from_rgb(0x8c, 0x9e, 0xff);
constexpr Color INDIGO_A200 = Color::from_rgb(0x53, 0x6d, 0xfe);
constexpr Color INDIGO_A400 = Color::from_rgb(0x3d, 0x5a, 0xfe);
constexpr Color INDIGO_A700 = Color::from_rgb(0x30, 0x4f, 0xfe);

constexpr Color BLUE_50   = Color::from_rgb(0xe3, 0xf2, 0xfd);
constexpr Color BLUE_100  = Color::from_rgb(0xbb, 0xde, 0xfb);
constexpr Color BLUE_200  = Color::from_rgb(0x90, 0xca, 0xf9);
constexpr Color BLUE_300  = Color::from_rgb(0x64, 0xb5, 0xf6);
constexpr Color BLUE_400  = Color::from_rgb(0x42, 0xa5, 0xf5);
constexpr Color BLUE_500  = Color::from_rgb(0x21, 0x96, 0xf3);
constexpr Color BLUE_600  = Color::from_rgb(0x1e, 0x88, 0xe5);
constexpr Color BLUE_700  = Color::from_rgb(0x19, 0x76, 0xd2);
constexpr Color BLUE_800  = Color::from_rgb(0x15, 0x65, 0xc0);
constexpr Color BLUE_900  = Color::from_rgb(0x0d, 0x47, 0xa1);
constexpr Color BLUE_A100 = Color::from_rgb(0x82, 0xb1, 0xff);
constexpr Color BLUE_A200 = Color::from_rgb(0x44, 0x8a, 0xff);
constexpr Color BLUE_A400 = Color::from_rgb(0x29, 0x79, 0xff);
constexpr Color BLUE_A700 = Color::from_rgb(0x29, 0x62, 0xff);

constexpr Color LIGHT_BLUE_50   = Color::from_rgb(0xe1, 0xf5, 0xfe);
constexpr Color LIGHT_BLUE_100  = Color::from_rgb(0xb3, 0xe5, 0xfc);
constexpr Color LIGHT_BLUE_200  = Color::from_rgb(0x81, 0xd4, 0xfa);
constexpr Color LIGHT_BLUE_300  = Color::from_rgb(0x4f, 0xc3, 0xf7);
constexpr Color LIGHT_BLUE_400  = Color::from_rgb(0x29, 0xb6, 0xf6);
constexpr Color LIGHT_BLUE_500  = Color::from_rgb(0x03, 0xa9, 0xf4);
constexpr Color LIGHT_BLUE_600  = Color::from_rgb(0x03, 0x9b, 0xe5);
constexpr Color LIGHT_BLUE_700  = Color::from_rgb(0x02, 0x88, 0xd1);
constexpr Color LIGHT_BLUE_800  = Color::from_rgb(0x02, 0x77, 0xbd);
constexpr Color LIGHT_BLUE_900  = Color::from_rgb(0x01, 0x57, 0x9b);
constexpr Color LIGHT_BLUE_A100 = Color::from_rgb(0x80, 0xd8, 0xff);
constexpr Color LIGHT_BLUE_A200 = Color::from_rgb(0x40, 0xc4, 0xff);
constexpr Color LIGHT_BLUE_A400 = Color::from_rgb(0x00, 0xb0, 0xff);
constexpr Color LIGHT_BLUE_A700 = Color::from_rgb(0x00, 0x91, 0xea);

constexpr Color CYAN_50   = Color::from_rgb(0xe0, 0xf7, 0xfa);
constexpr Color CYAN_100  = Color::from_rgb(0xb2, 0xeb, 0xf2);
constexpr Color CYAN_200  = Color::from_rgb(0x80, 0xde, 0xea);
constexpr Color CYAN_300  = Color::from_rgb(0x4d, 0xd0, 0xe1);
constexpr Color CYAN_400  = Color::from_rgb(0x26, 0xc6, 0xda);
constexpr Color CYAN_500  = Color::from_rgb(0x00, 0xbc, 0xd4);
constexpr Color CYAN_600  = Color::from_rgb(0x00, 0xac, 0xc1);
constexpr Color CYAN_700  = Color::from_rgb(0x00, 0x97, 0xa7);
constexpr Color CYAN_800  = Color::from_rgb(0x00, 0x83, 0x8f);
constexpr Color CYAN_900  = Color::from_rgb(0x00, 0x60, 0x64);
constexpr Color CYAN_A100 = Color::from_rgb(0x84, 0xff, 0xff);
constexpr Color CYAN_A200 = Color::from_rgb(0x18, 0xff, 0xff);
constexpr Color CYAN_A400 = Color::from_rgb(0x00, 0xe5, 0xff);
constexpr Color CYAN_A700 = Color::from_rgb(0x00, 0xb8, 0xd4);

constexpr Color TEAL_50   = Color::from_rgb(0xe0, 0xf2, 0xf1);
constexpr Color TEAL_100  = Color::from_rgb(0xb2, 0xdf, 0xdb);
constexpr Color TEAL_200  = Color::from_rgb(0x80, 0xcb, 0xc4);
constexpr Color TEAL_300  = Color::from_rgb(0x4d, 0xb6, 0xac);
constexpr Color TEAL_400  = Color::from_rgb(0x26, 0xa6, 0x9a);
constexpr Color TEAL_500  = Color::from_rgb(0x00, 0x96, 0x88);
constexpr Color TEAL_600  = Color::from_rgb(0x00, 0x89, 0x7b);
constexpr Color TEAL_700  = Color::from_rgb(0x00, 0x79, 0x6b);
constexpr Color TEAL_800  = Color::from_rgb(0x00, 0x69, 0x5c);
constexpr Color TEAL_900  = Color::from_rgb(0x00, 0x4d, 0x40);
constexpr Color TEAL_A100 = Color::from_rgb(0xa7, 0xff, 0xeb);
constexpr Color TEAL_A200 = Color::from_rgb(0x64, 0xff, 0xda);
constexpr Color TEAL_A400 = Color::from_rgb(0x1d, 0xe9, 0xb6);
constexpr Color TEAL_A700 = Color::from_rgb(0x00, 0xbf, 0xa5);

constexpr Color GREEN_50   = Color::from_rgb(0xe8, 0xf5, 0xe9);
constexpr Color GREEN_100  = Color::from_rgb(0xc8, 0xe6, 0xc9);
constexpr Color GREEN_200  = Color::from_rgb(0xa5, 0xd6, 0xa7);
constexpr Color GREEN_300  = Color::from_rgb(0x81, 0xc7, 0x84);
constexpr Color GREEN_400  = Color::from_rgb(0x66, 0xbb, 0x6a);
constexpr Color GREEN_500  = Color::from_rgb(0x4c, 0xaf, 0x50);
constexpr Color GREEN_600  = Color::from_rgb(0x43, 0xa0, 0x47);
constexpr Color GREEN_700  = Color::from_rgb(0x38, 0x8e, 0x3c);
constexpr Color GREEN_800  = Color::from_rgb(0x2e, 0x7d, 0x32);
constexpr Color GREEN_900  = Color::from_rgb(0x1b, 0x5e, 0x20);
constexpr Color GREEN_A100 = Color::from_rgb(0xb9, 0xf6, 0xca);
constexpr Color GREEN_A200 = Color::from_rgb(0x69, 0xf0, 0xae);
constexpr Color GREEN_A400 = Color::from_rgb(0x00, 0xe6, 0x76);
constexpr Color GREEN_A700 = Color::from_rgb(0x00, 0xc8, 0x53);

constexpr Color LIGHT_GREEN_50   = Color::from_rgb(0xf1, 0xf8, 0xe9);
constexpr Color LIGHT_GREEN_100  = Color::from_rgb(0xdc, 0xed, 0xc8);
constexpr Color LIGHT_GREEN_200  = Color::from_rgb(0xc5, 0xe1, 0xa5);
constexpr Color LIGHT_GREEN_300  = Color::from_rgb(0xae, 0xd5, 0x81);
constexpr Color LIGHT_GREEN_400  = Color::from_rgb(0x9c, 0xcc, 0x65);
constexpr Color LIGHT_GREEN_500  = Color::from_rgb(0x8b, 0xc3, 0x4a);
constexpr Color LIGHT_GREEN_600  = Color::from_rgb(0x7c, 0xb3, 0x42);
constexpr Color LIGHT_GREEN_700  = Color::from_rgb(0x68, 0x9f, 0x38);
constexpr Color LIGHT_GREEN_800  = Color::from_rgb(0x55, 0x8b, 0x2f);
constexpr Color LIGHT_GREEN_900  = Color::from_rgb(0x33, 0x69, 0x1e);
constexpr Color LIGHT_GREEN_A100 = Color::from_rgb(0xcc, 0xff, 0x90);
constexpr Color LIGHT_GREEN_A200 = Color::from_rgb(0xb2, 0xff, 0x59);
constexpr Color LIGHT_GREEN_A400 = Color::from_rgb(0x76, 0xff, 0x03);
constexpr Color LIGHT_GREEN_A700 = Color::from_rgb(0x64, 0xdd, 0x17);

constexpr Color LIME_50   = Color::from_rgb(0xf9, 0xfb, 0xe7);
constexpr Color LIME_100  = Color::from_rgb(0xf0, 0xf4, 0xc3);
constexpr Color LIME_200  = Color::from_rgb(0xe6, 0xee, 0x9c);
constexpr Color LIME_300  = Color::from_rgb(0xdc, 0xe7, 0x75);
constexpr Color LIME_400  = Color::from_rgb(0xd4, 0xe1, 0x57);
constexpr Color LIME_500  = Color::from_rgb(0xcd, 0xdc, 0x39);
constexpr Color LIME_600  = Color::from_rgb(0xc0, 0xca, 0x33);
constexpr Color LIME_700  = Color::from_rgb(0xaf, 0xb4, 0x2b);
constexpr Color LIME_800  = Color::from_rgb(0x9e, 0x9d, 0x24);
constexpr Color LIME_900  = Color::from_rgb(0x82, 0x77, 0x17);
constexpr Color LIME_A100 = Color::from_rgb(0xf4, 0xff, 0x81);
constexpr Color LIME_A200 = Color::from_rgb(0xee, 0xff, 0x41);
constexpr Color LIME_A400 = Color::from_rgb(0xc6, 0xff, 0x00);
constexpr Color LIME_A700 = Color::from_rgb(0xae, 0xea, 0x00);

constexpr Color YELLOW_50   = Color::from_rgb(0xff, 0xfd, 0xe7);
constexpr Color YELLOW_100  = Color::from_rgb(0xff, 0xf9, 0xc4);
constexpr Color YELLOW_200  = Color::from_rgb(0xff, 0xf5, 0x9d);
constexpr Color YELLOW_300  = Color::from_rgb(0xff, 0xf1, 0x76);
constexpr Color YELLOW_400  = Color::from_rgb(0xff, 0xee, 0x58);
constexpr Color YELLOW_500  = Color::from_rgb(0xff, 0xeb, 0x3b);
constexpr Color YELLOW_600  = Color::from_rgb(0xfd, 0xd8, 0x35);
constexpr Color YELLOW_700  = Color::from_rgb(0xfb, 0xc0, 0x2d);
constexpr Color YELLOW_800  = Color::from_rgb(0xf9, 0xa8, 0x25);
constexpr Color YELLOW_900  = Color::from_rgb(0xf5, 0x7f, 0x17);
constexpr Color YELLOW_A100 = Color::from_rgb(0xff, 0xff, 0x8d);
constexpr Color YELLOW_A200 = Color::from_rgb(0xff, 0xff, 0x00);
constexpr Color YELLOW_A400 = Color::from_rgb(0xff, 0xea, 0x00);
constexpr Color YELLOW_A700 = Color::from_rgb(0xff, 0xd6, 0x00);

constexpr Color AMBER_50   = Color::from_rgb(0xff, 0xf8, 0xe1);
constexpr Color AMBER_100  = Color::from_rgb(0xff, 0xec, 0xb3);
constexpr Color AMBER_200  = Color::from_rgb(0xff, 0xe0, 0x82);
constexpr Color AMBER_300  = Color::from_rgb(0xff, 0xd5, 0x4f);
constexpr Color AMBER_400  = Color::from_rgb(0xff, 0xca, 0x28);
constexpr Color AMBER_500  = Color::from_rgb(0xff, 0xc1, 0x07);
constexpr Color AMBER_600  = Color::from_rgb(0xff, 0xb3, 0x00);
constexpr Color AMBER_700  = Color::from_rgb(0xff, 0xa0, 0x00);
constexpr Color AMBER_800  = Color::from_rgb(0xff, 0x8f, 0x00);
constexpr Color AMBER_900  = Color::from_rgb(0xff, 0x6f, 0x00);
constexpr Color AMBER_A100 = Color::from_rgb(0xff, 0xe5, 0x7f);
constexpr Color AMBER_A200 = Color::from_rgb(0xff, 0xd7, 0x40);
constexpr Color AMBER_A400 = Color::from_rgb(0xff, 0xc4, 0x00);
constexpr Color AMBER_A700 = Color::from_rgb(0xff, 0xab, 0x00);

constexpr Color ORANGE_50   = Color::from_rgb(0xff, 0xf3, 0xe0);
constexpr Color ORANGE_100  = Color::from_rgb(0xff, 0xe0, 0xb2);
constexpr Color ORANGE_200  = Color::from_rgb(0xff, 0xcc, 0x80);
constexpr Color ORANGE_300  = Color::from_rgb(0xff, 0xb7, 0x4d);
constexpr Color ORANGE_400  = Color::from_rgb(0xff, 0xa7, 0x26);
constexpr Color ORANGE_500  = Color::from_rgb(0xff, 0x98, 0x00);
constexpr Color ORANGE_600  = Color::from_rgb(0xfb, 0x8c, 0x00);
constexpr Color ORANGE_700  = Color::from_rgb(0xf5, 0x7c, 0x00);
constexpr Color ORANGE_800  = Color::from_rgb(0xef, 0x6c, 0x00);
constexpr Color ORANGE_900  = Color::from_rgb(0xe6, 0x51, 0x00);
constexpr Color ORANGE_A100 = Color::from_rgb(0xff, 0xd1, 0x80);
constexpr Color ORANGE_A200 = Color::from_rgb(0xff, 0xab, 0x40);
constexpr Color ORANGE_A400 = Color::from_rgb(0xff, 0x91, 0x00);
constexpr Color ORANGE_A700 = Color::from_rgb(0xff, 0x6d, 0x00);

constexpr Color DEEP_ORANGE_50   = Color::from_rgb(0xfb, 0xe9, 0xe7);
constexpr Color DEEP_ORANGE_100  = Color::from_rgb(0xff, 0xcc, 0xbc);
constexpr Color DEEP_ORANGE_200  = Color::from_rgb(0xff, 0xab, 0x91);
constexpr Color DEEP_ORANGE_300  = Color::from_rgb(0xff, 0x8a, 0x65);
constexpr Color DEEP_ORANGE_400  = Color::from_rgb(0xff, 0x70, 0x43);
constexpr Color DEEP_ORANGE_500  = Color::from_rgb(0xff, 0x57, 0x22);
constexpr Color DEEP_ORANGE_600  = Color::from_rgb(0xf4, 0x51, 0x1e);
constexpr Color DEEP_ORANGE_700  = Color::from_rgb(0xe6, 0x4a, 0x19);
constexpr Color DEEP_ORANGE_800  = Color::from_rgb(0xd8, 0x43, 0x15);
constexpr Color DEEP_ORANGE_900  = Color::from_rgb(0xbf, 0x36, 0x0c);
constexpr Color DEEP_ORANGE_A100 = Color::from_rgb(0xff, 0x9e, 0x80);
constexpr Color DEEP_ORANGE_A200 = Color::from_rgb(0xff, 0x6e, 0x40);
constexpr Color DEEP_ORANGE_A400 = Color::from_rgb(0xff, 0x3d, 0x00);
constexpr Color DEEP_ORANGE_A700 = Color::from_rgb(0xdd, 0x2c, 0x00);

constexpr Color BROWN_50  = Color::from_rgb(0xef, 0xeb, 0xe9);
constexpr Color BROWN_100 = Color::from_rgb(0xd7, 0xcc, 0xc8);
constexpr Color BROWN_200 = Color::from_rgb(0xbc, 0xaa, 0xa4);
constexpr Color BROWN_300 = Color::from_rgb(0xa1, 0x88, 0x7f);
constexpr Color BROWN_400 = Color::from_rgb(0x8d, 0x6e, 0x63);
constexpr Color BROWN_500 = Color::from_rgb(0x79, 0x55, 0x48);
constexpr Color BROWN_600 = Color::from_rgb(0x6d, 0x4c, 0x41);
constexpr Color BROWN_700 = Color::from_rgb(0x5d, 0x40, 0x37);
constexpr Color BROWN_800 = Color::from_rgb(0x4e, 0x34, 0x2e);
constexpr Color BROWN_900 = Color::from_rgb(0x3e, 0x27, 0x23);

constexpr Color GRAY_50  = Color::from_rgb(0xfa, 0xfa, 0xfa);
constexpr Color GRAY_100 = Color::from_rgb(0xf5, 0xf5, 0xf5);
constexpr Color GRAY_200 = Color::from_rgb(0xee, 0xee, 0xee);
constexpr Color GRAY_300 = Color::from_rgb(0xe0, 0xe0, 0xe0);
constexpr Color GRAY_400 = Color::from_rgb(0xbd, 0xbd, 0xbd);
constexpr Color GRAY_500 = Color::from_rgb(0x9e, 0x9e, 0x9e);
constexpr Color GRAY_600 = Color::from_rgb(0x75, 0x75, 0x75);
constexpr Color GRAY_700 = Color::from_rgb(0x61, 0x61, 0x61);
constexpr Color GRAY_800 = Color::from_rgb(0x42, 0x42, 0x42);
constexpr Color GRAY_900 = Color::from_rgb(0x21, 0x21, 0x21);

constexpr Color BLUE_GRAY_50  = Color::from_rgb(0xec, 0xef, 0xf1);
constexpr Color BLUE_GRAY_100 = Color::from_rgb(0xcf, 0xd8, 0xdc);
constexpr Color BLUE_GRAY_200 = Color::from_rgb(0xb0, 0xbe, 0xc5);
constexpr Color BLUE_GRAY_300 = Color::from_rgb(0x90, 0xa4, 0xae);
constexpr Color BLUE_GRAY_400 = Color::from_rgb(0x78, 0x90, 0x9c);
constexpr Color BLUE_GRAY_500 = Color::from_rgb(0x60, 0x7d, 0x8b);
constexpr Color BLUE_GRAY_600 = Color::from_rgb(0x54, 0x6e, 0x7a);
constexpr Color BLUE_GRAY_700 = Color::from_rgb(0x45, 0x5a, 0x64);
constexpr Color BLUE_GRAY_800 = Color::from_rgb(0x37, 0x47, 0x4f);
constexpr Color BLUE_GRAY_900 = Color::from_rgb(0x26, 0x32, 0x38);

constexpr Color WHITE = Color::from_rgb(0xff, 0xff, 0xff);
constexpr Color BLACK = Color::from_rgb(0x00, 0x00, 0x00);

}        // namespace material

}        // namespace ash
