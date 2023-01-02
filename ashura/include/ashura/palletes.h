
#pragma once

#include "asr/primitives.h"

namespace asr {

// ios default system colors
namespace ios {
constexpr color LIGHT_BLUE = color::from_rgb(0, 122, 255);
constexpr color DARK_BLUE = color::from_rgb(10, 132, 255);
constexpr color LIGHT_BROWN = color::from_rgb(162, 132, 94);
constexpr color DARK_BROWN = color::from_rgb(172, 142, 104);
constexpr color LIGHT_CYAN = color::from_rgb(50, 173, 230);
constexpr color DARK_CYAN = color::from_rgb(100, 210, 255);
constexpr color LIGHT_GREEN = color::from_rgb(52, 199, 89);
constexpr color DARK_GREEN = color::from_rgb(48, 209, 88);
constexpr color LIGHT_INDIGO = color::from_rgb(88, 86, 214);
constexpr color DARK_INDIGO = color::from_rgb(94, 92, 230);
constexpr color LIGHT_MINT = color::from_rgb(0, 199, 190);
constexpr color DARK_MINT = color::from_rgb(102, 212, 207);
constexpr color LIGHT_ORANGE = color::from_rgb(255, 149, 0);
constexpr color DARK_ORANGE = color::from_rgb(255, 159, 10);
constexpr color LIGHT_PINK = color::from_rgb(255, 45, 85);
constexpr color DARK_PINK = color::from_rgb(255, 55, 95);
constexpr color LIGHT_PURPLE = color::from_rgb(175, 82, 222);
constexpr color DARK_PURPLE = color::from_rgb(191, 90, 242);
constexpr color LIGHT_RED = color::from_rgb(255, 59, 48);
constexpr color DARK_RED = color::from_rgb(255, 69, 58);
constexpr color LIGHT_TEAL = color::from_rgb(48, 176, 199);
constexpr color DARK_TEAL = color::from_rgb(64, 200, 224);
constexpr color LIGHT_YELLOW = color::from_rgb(255, 204, 0);
constexpr color DARK_YELLOW = color::from_rgb(255, 214, 10);

constexpr color LIGHT_GRAY = color::from_rgb(142, 142, 147);
constexpr color DARK_GRAY = color::from_rgb(142, 142, 147);
constexpr color LIGHT_GRAY_2 = color::from_rgb(174, 174, 178);
constexpr color DARK_GRAY_2 = color::from_rgb(99, 99, 102);
constexpr color LIGHT_GRAY_3 = color::from_rgb(199, 199, 204);
constexpr color DARK_GRAY_3 = color::from_rgb(72, 72, 74);
constexpr color LIGHT_GRAY_4 = color::from_rgb(209, 209, 214);
constexpr color DARK_GRAY_4 = color::from_rgb(58, 58, 60);
constexpr color LIGHT_GRAY_5 = color::from_rgb(229, 229, 234);
constexpr color DARK_GRAY_5 = color::from_rgb(44, 44, 46);
constexpr color LIGHT_GRAY_6 = color::from_rgb(242, 242, 247);
constexpr color DARK_GRAY_6 = color::from_rgb(28, 28, 30);

// ios accessible colors
namespace accessible {
constexpr color LIGHT_BLUE = color::from_rgb(0, 64, 221);
constexpr color DARK_BLUE = color::from_rgb(64, 156, 255);
constexpr color LIGHT_BROWN = color::from_rgb(127, 101, 69);
constexpr color DARK_BROWN = color::from_rgb(181, 148, 105);
constexpr color LIGHT_CYAN = color::from_rgb(0, 113, 164);
constexpr color DARK_CYAN = color::from_rgb(112, 215, 255);
constexpr color LIGHT_GREEN = color::from_rgb(36, 138, 61);
constexpr color DARK_GREEN = color::from_rgb(48, 219, 91);
constexpr color LIGHT_INDIGO = color::from_rgb(54, 52, 163);
constexpr color DARK_INDIGO = color::from_rgb(125, 122, 255);
constexpr color LIGHT_MINT = color::from_rgb(12, 129, 123);
constexpr color DARK_MINT = color::from_rgb(102, 212, 207);
constexpr color LIGHT_ORANGE = color::from_rgb(201, 52, 0);
constexpr color DARK_ORANGE = color::from_rgb(255, 179, 64);
constexpr color LIGHT_PINK = color::from_rgb(211, 15, 69);
constexpr color DARK_PINK = color::from_rgb(255, 100, 130);
constexpr color LIGHT_PURPLE = color::from_rgb(137, 68, 171);
constexpr color DARK_PURPLE = color::from_rgb(218, 143, 255);
constexpr color LIGHT_RED = color::from_rgb(215, 0, 21);
constexpr color DARK_RED = color::from_rgb(255, 105, 97);
constexpr color LIGHT_TEAL = color::from_rgb(0, 130, 153);
constexpr color DARK_TEAL = color::from_rgb(93, 230, 255);
constexpr color LIGHT_YELLOW = color::from_rgb(178, 80, 0);
constexpr color DARK_YELLOW = color::from_rgb(255, 212, 38);

constexpr color LIGHT_GRAY = color::from_rgb(108, 108, 112);
constexpr color DARK_GRAY = color::from_rgb(174, 174, 178);
constexpr color LIGHT_GRAY_2 = color::from_rgb(142, 142, 147);
constexpr color DARK_GRAY_2 = color::from_rgb(124, 124, 128);
constexpr color LIGHT_GRAY_3 = color::from_rgb(174, 174, 178);
constexpr color DARK_GRAY_3 = color::from_rgb(84, 84, 86);
constexpr color LIGHT_GRAY_4 = color::from_rgb(188, 188, 192);
constexpr color DARK_GRAY_4 = color::from_rgb(68, 68, 70);
constexpr color LIGHT_GRAY_5 = color::from_rgb(216, 216, 220);
constexpr color DARK_GRAY_5 = color::from_rgb(54, 54, 56);
constexpr color LIGHT_GRAY_6 = color::from_rgb(235, 235, 240);
constexpr color DARK_GRAY_6 = color::from_rgb(36, 36, 36);
}  // namespace accessible

}  // namespace ios

namespace material {
constexpr color RED_50 = color::from_rgb(0xff, 0xeb, 0xee);
constexpr color RED_100 = color::from_rgb(0xff, 0xcd, 0xd2);
constexpr color RED_200 = color::from_rgb(0xef, 0x9a, 0x9a);
constexpr color RED_300 = color::from_rgb(0xe5, 0x73, 0x73);
constexpr color RED_400 = color::from_rgb(0xef, 0x53, 0x50);
constexpr color RED_500 = color::from_rgb(0xf4, 0x43, 0x36);
constexpr color RED_600 = color::from_rgb(0xe5, 0x39, 0x35);
constexpr color RED_700 = color::from_rgb(0xd3, 0x2f, 0x2f);
constexpr color RED_800 = color::from_rgb(0xc6, 0x28, 0x28);
constexpr color RED_900 = color::from_rgb(0xb7, 0x1c, 0x1c);
constexpr color RED_A100 = color::from_rgb(0xff, 0x8a, 0x80);
constexpr color RED_A200 = color::from_rgb(0xff, 0x52, 0x52);
constexpr color RED_A400 = color::from_rgb(0xff, 0x17, 0x44);
constexpr color RED_A700 = color::from_rgb(0xd5, 0x00, 0x00);

constexpr color PINK_50 = color::from_rgb(0xfc, 0xe4, 0xec);
constexpr color PINK_100 = color::from_rgb(0xf8, 0xbb, 0xd0);
constexpr color PINK_200 = color::from_rgb(0xf4, 0x8f, 0xb1);
constexpr color PINK_300 = color::from_rgb(0xf0, 0x62, 0x92);
constexpr color PINK_400 = color::from_rgb(0xec, 0x40, 0x7a);
constexpr color PINK_500 = color::from_rgb(0xe9, 0x1e, 0x63);
constexpr color PINK_600 = color::from_rgb(0xd8, 0x1b, 0x60);
constexpr color PINK_700 = color::from_rgb(0xc2, 0x18, 0x5b);
constexpr color PINK_800 = color::from_rgb(0xad, 0x14, 0x57);
constexpr color PINK_900 = color::from_rgb(0x88, 0x0e, 0x4f);
constexpr color PINK_A100 = color::from_rgb(0xff, 0x80, 0xab);
constexpr color PINK_A200 = color::from_rgb(0xff, 0x40, 0x81);
constexpr color PINK_A400 = color::from_rgb(0xf5, 0x00, 0x57);
constexpr color PINK_A700 = color::from_rgb(0xc5, 0x11, 0x62);

constexpr color PURPLE_50 = color::from_rgb(0xf3, 0xe5, 0xf5);
constexpr color PURPLE_100 = color::from_rgb(0xe1, 0xbe, 0xe7);
constexpr color PURPLE_200 = color::from_rgb(0xce, 0x93, 0xd8);
constexpr color PURPLE_300 = color::from_rgb(0xba, 0x68, 0xc8);
constexpr color PURPLE_400 = color::from_rgb(0xab, 0x47, 0xbc);
constexpr color PURPLE_500 = color::from_rgb(0x9c, 0x27, 0xb0);
constexpr color PURPLE_600 = color::from_rgb(0x8e, 0x24, 0xaa);
constexpr color PURPLE_700 = color::from_rgb(0x7b, 0x1f, 0xa2);
constexpr color PURPLE_800 = color::from_rgb(0x6a, 0x1b, 0x9a);
constexpr color PURPLE_900 = color::from_rgb(0x4a, 0x14, 0x8c);
constexpr color PURPLE_A100 = color::from_rgb(0xea, 0x80, 0xfc);
constexpr color PURPLE_A200 = color::from_rgb(0xe0, 0x40, 0xfb);
constexpr color PURPLE_A400 = color::from_rgb(0xd5, 0x00, 0xf9);
constexpr color PURPLE_A700 = color::from_rgb(0xaa, 0x00, 0xff);

constexpr color DEEP_PURPLE_50 = color::from_rgb(0xed, 0xe7, 0xf6);
constexpr color DEEP_PURPLE_100 = color::from_rgb(0xd1, 0xc4, 0xe9);
constexpr color DEEP_PURPLE_200 = color::from_rgb(0xb3, 0x9d, 0xdb);
constexpr color DEEP_PURPLE_300 = color::from_rgb(0x95, 0x75, 0xcd);
constexpr color DEEP_PURPLE_400 = color::from_rgb(0x7e, 0x57, 0xc2);
constexpr color DEEP_PURPLE_500 = color::from_rgb(0x67, 0x3a, 0xb7);
constexpr color DEEP_PURPLE_600 = color::from_rgb(0x5e, 0x35, 0xb1);
constexpr color DEEP_PURPLE_700 = color::from_rgb(0x51, 0x2d, 0xa8);
constexpr color DEEP_PURPLE_800 = color::from_rgb(0x45, 0x27, 0xa0);
constexpr color DEEP_PURPLE_900 = color::from_rgb(0x31, 0x1b, 0x92);
constexpr color DEEP_PURPLE_A100 = color::from_rgb(0xb3, 0x88, 0xff);
constexpr color DEEP_PURPLE_A200 = color::from_rgb(0x7c, 0x4d, 0xff);
constexpr color DEEP_PURPLE_A400 = color::from_rgb(0x65, 0x1f, 0xff);
constexpr color DEEP_PURPLE_A700 = color::from_rgb(0x62, 0x00, 0xea);

constexpr color INDIGO_50 = color::from_rgb(0xe8, 0xea, 0xf6);
constexpr color INDIGO_100 = color::from_rgb(0xc5, 0xca, 0xe9);
constexpr color INDIGO_200 = color::from_rgb(0x9f, 0xa8, 0xda);
constexpr color INDIGO_300 = color::from_rgb(0x79, 0x86, 0xcb);
constexpr color INDIGO_400 = color::from_rgb(0x5c, 0x6b, 0xc0);
constexpr color INDIGO_500 = color::from_rgb(0x3f, 0x51, 0xb5);
constexpr color INDIGO_600 = color::from_rgb(0x39, 0x49, 0xab);
constexpr color INDIGO_700 = color::from_rgb(0x30, 0x3f, 0x9f);
constexpr color INDIGO_800 = color::from_rgb(0x28, 0x35, 0x93);
constexpr color INDIGO_900 = color::from_rgb(0x1a, 0x23, 0x7e);
constexpr color INDIGO_A100 = color::from_rgb(0x8c, 0x9e, 0xff);
constexpr color INDIGO_A200 = color::from_rgb(0x53, 0x6d, 0xfe);
constexpr color INDIGO_A400 = color::from_rgb(0x3d, 0x5a, 0xfe);
constexpr color INDIGO_A700 = color::from_rgb(0x30, 0x4f, 0xfe);

constexpr color BLUE_50 = color::from_rgb(0xe3, 0xf2, 0xfd);
constexpr color BLUE_100 = color::from_rgb(0xbb, 0xde, 0xfb);
constexpr color BLUE_200 = color::from_rgb(0x90, 0xca, 0xf9);
constexpr color BLUE_300 = color::from_rgb(0x64, 0xb5, 0xf6);
constexpr color BLUE_400 = color::from_rgb(0x42, 0xa5, 0xf5);
constexpr color BLUE_500 = color::from_rgb(0x21, 0x96, 0xf3);
constexpr color BLUE_600 = color::from_rgb(0x1e, 0x88, 0xe5);
constexpr color BLUE_700 = color::from_rgb(0x19, 0x76, 0xd2);
constexpr color BLUE_800 = color::from_rgb(0x15, 0x65, 0xc0);
constexpr color BLUE_900 = color::from_rgb(0x0d, 0x47, 0xa1);
constexpr color BLUE_A100 = color::from_rgb(0x82, 0xb1, 0xff);
constexpr color BLUE_A200 = color::from_rgb(0x44, 0x8a, 0xff);
constexpr color BLUE_A400 = color::from_rgb(0x29, 0x79, 0xff);
constexpr color BLUE_A700 = color::from_rgb(0x29, 0x62, 0xff);

constexpr color LIGHT_BLUE_50 = color::from_rgb(0xe1, 0xf5, 0xfe);
constexpr color LIGHT_BLUE_100 = color::from_rgb(0xb3, 0xe5, 0xfc);
constexpr color LIGHT_BLUE_200 = color::from_rgb(0x81, 0xd4, 0xfa);
constexpr color LIGHT_BLUE_300 = color::from_rgb(0x4f, 0xc3, 0xf7);
constexpr color LIGHT_BLUE_400 = color::from_rgb(0x29, 0xb6, 0xf6);
constexpr color LIGHT_BLUE_500 = color::from_rgb(0x03, 0xa9, 0xf4);
constexpr color LIGHT_BLUE_600 = color::from_rgb(0x03, 0x9b, 0xe5);
constexpr color LIGHT_BLUE_700 = color::from_rgb(0x02, 0x88, 0xd1);
constexpr color LIGHT_BLUE_800 = color::from_rgb(0x02, 0x77, 0xbd);
constexpr color LIGHT_BLUE_900 = color::from_rgb(0x01, 0x57, 0x9b);
constexpr color LIGHT_BLUE_A100 = color::from_rgb(0x80, 0xd8, 0xff);
constexpr color LIGHT_BLUE_A200 = color::from_rgb(0x40, 0xc4, 0xff);
constexpr color LIGHT_BLUE_A400 = color::from_rgb(0x00, 0xb0, 0xff);
constexpr color LIGHT_BLUE_A700 = color::from_rgb(0x00, 0x91, 0xea);

constexpr color CYAN_50 = color::from_rgb(0xe0, 0xf7, 0xfa);
constexpr color CYAN_100 = color::from_rgb(0xb2, 0xeb, 0xf2);
constexpr color CYAN_200 = color::from_rgb(0x80, 0xde, 0xea);
constexpr color CYAN_300 = color::from_rgb(0x4d, 0xd0, 0xe1);
constexpr color CYAN_400 = color::from_rgb(0x26, 0xc6, 0xda);
constexpr color CYAN_500 = color::from_rgb(0x00, 0xbc, 0xd4);
constexpr color CYAN_600 = color::from_rgb(0x00, 0xac, 0xc1);
constexpr color CYAN_700 = color::from_rgb(0x00, 0x97, 0xa7);
constexpr color CYAN_800 = color::from_rgb(0x00, 0x83, 0x8f);
constexpr color CYAN_900 = color::from_rgb(0x00, 0x60, 0x64);
constexpr color CYAN_A100 = color::from_rgb(0x84, 0xff, 0xff);
constexpr color CYAN_A200 = color::from_rgb(0x18, 0xff, 0xff);
constexpr color CYAN_A400 = color::from_rgb(0x00, 0xe5, 0xff);
constexpr color CYAN_A700 = color::from_rgb(0x00, 0xb8, 0xd4);

constexpr color TEAL_50 = color::from_rgb(0xe0, 0xf2, 0xf1);
constexpr color TEAL_100 = color::from_rgb(0xb2, 0xdf, 0xdb);
constexpr color TEAL_200 = color::from_rgb(0x80, 0xcb, 0xc4);
constexpr color TEAL_300 = color::from_rgb(0x4d, 0xb6, 0xac);
constexpr color TEAL_400 = color::from_rgb(0x26, 0xa6, 0x9a);
constexpr color TEAL_500 = color::from_rgb(0x00, 0x96, 0x88);
constexpr color TEAL_600 = color::from_rgb(0x00, 0x89, 0x7b);
constexpr color TEAL_700 = color::from_rgb(0x00, 0x79, 0x6b);
constexpr color TEAL_800 = color::from_rgb(0x00, 0x69, 0x5c);
constexpr color TEAL_900 = color::from_rgb(0x00, 0x4d, 0x40);
constexpr color TEAL_A100 = color::from_rgb(0xa7, 0xff, 0xeb);
constexpr color TEAL_A200 = color::from_rgb(0x64, 0xff, 0xda);
constexpr color TEAL_A400 = color::from_rgb(0x1d, 0xe9, 0xb6);
constexpr color TEAL_A700 = color::from_rgb(0x00, 0xbf, 0xa5);

constexpr color GREEN_50 = color::from_rgb(0xe8, 0xf5, 0xe9);
constexpr color GREEN_100 = color::from_rgb(0xc8, 0xe6, 0xc9);
constexpr color GREEN_200 = color::from_rgb(0xa5, 0xd6, 0xa7);
constexpr color GREEN_300 = color::from_rgb(0x81, 0xc7, 0x84);
constexpr color GREEN_400 = color::from_rgb(0x66, 0xbb, 0x6a);
constexpr color GREEN_500 = color::from_rgb(0x4c, 0xaf, 0x50);
constexpr color GREEN_600 = color::from_rgb(0x43, 0xa0, 0x47);
constexpr color GREEN_700 = color::from_rgb(0x38, 0x8e, 0x3c);
constexpr color GREEN_800 = color::from_rgb(0x2e, 0x7d, 0x32);
constexpr color GREEN_900 = color::from_rgb(0x1b, 0x5e, 0x20);
constexpr color GREEN_A100 = color::from_rgb(0xb9, 0xf6, 0xca);
constexpr color GREEN_A200 = color::from_rgb(0x69, 0xf0, 0xae);
constexpr color GREEN_A400 = color::from_rgb(0x00, 0xe6, 0x76);
constexpr color GREEN_A700 = color::from_rgb(0x00, 0xc8, 0x53);

constexpr color LIGHT_GREEN_50 = color::from_rgb(0xf1, 0xf8, 0xe9);
constexpr color LIGHT_GREEN_100 = color::from_rgb(0xdc, 0xed, 0xc8);
constexpr color LIGHT_GREEN_200 = color::from_rgb(0xc5, 0xe1, 0xa5);
constexpr color LIGHT_GREEN_300 = color::from_rgb(0xae, 0xd5, 0x81);
constexpr color LIGHT_GREEN_400 = color::from_rgb(0x9c, 0xcc, 0x65);
constexpr color LIGHT_GREEN_500 = color::from_rgb(0x8b, 0xc3, 0x4a);
constexpr color LIGHT_GREEN_600 = color::from_rgb(0x7c, 0xb3, 0x42);
constexpr color LIGHT_GREEN_700 = color::from_rgb(0x68, 0x9f, 0x38);
constexpr color LIGHT_GREEN_800 = color::from_rgb(0x55, 0x8b, 0x2f);
constexpr color LIGHT_GREEN_900 = color::from_rgb(0x33, 0x69, 0x1e);
constexpr color LIGHT_GREEN_A100 = color::from_rgb(0xcc, 0xff, 0x90);
constexpr color LIGHT_GREEN_A200 = color::from_rgb(0xb2, 0xff, 0x59);
constexpr color LIGHT_GREEN_A400 = color::from_rgb(0x76, 0xff, 0x03);
constexpr color LIGHT_GREEN_A700 = color::from_rgb(0x64, 0xdd, 0x17);

constexpr color LIME_50 = color::from_rgb(0xf9, 0xfb, 0xe7);
constexpr color LIME_100 = color::from_rgb(0xf0, 0xf4, 0xc3);
constexpr color LIME_200 = color::from_rgb(0xe6, 0xee, 0x9c);
constexpr color LIME_300 = color::from_rgb(0xdc, 0xe7, 0x75);
constexpr color LIME_400 = color::from_rgb(0xd4, 0xe1, 0x57);
constexpr color LIME_500 = color::from_rgb(0xcd, 0xdc, 0x39);
constexpr color LIME_600 = color::from_rgb(0xc0, 0xca, 0x33);
constexpr color LIME_700 = color::from_rgb(0xaf, 0xb4, 0x2b);
constexpr color LIME_800 = color::from_rgb(0x9e, 0x9d, 0x24);
constexpr color LIME_900 = color::from_rgb(0x82, 0x77, 0x17);
constexpr color LIME_A100 = color::from_rgb(0xf4, 0xff, 0x81);
constexpr color LIME_A200 = color::from_rgb(0xee, 0xff, 0x41);
constexpr color LIME_A400 = color::from_rgb(0xc6, 0xff, 0x00);
constexpr color LIME_A700 = color::from_rgb(0xae, 0xea, 0x00);

constexpr color YELLOW_50 = color::from_rgb(0xff, 0xfd, 0xe7);
constexpr color YELLOW_100 = color::from_rgb(0xff, 0xf9, 0xc4);
constexpr color YELLOW_200 = color::from_rgb(0xff, 0xf5, 0x9d);
constexpr color YELLOW_300 = color::from_rgb(0xff, 0xf1, 0x76);
constexpr color YELLOW_400 = color::from_rgb(0xff, 0xee, 0x58);
constexpr color YELLOW_500 = color::from_rgb(0xff, 0xeb, 0x3b);
constexpr color YELLOW_600 = color::from_rgb(0xfd, 0xd8, 0x35);
constexpr color YELLOW_700 = color::from_rgb(0xfb, 0xc0, 0x2d);
constexpr color YELLOW_800 = color::from_rgb(0xf9, 0xa8, 0x25);
constexpr color YELLOW_900 = color::from_rgb(0xf5, 0x7f, 0x17);
constexpr color YELLOW_A100 = color::from_rgb(0xff, 0xff, 0x8d);
constexpr color YELLOW_A200 = color::from_rgb(0xff, 0xff, 0x00);
constexpr color YELLOW_A400 = color::from_rgb(0xff, 0xea, 0x00);
constexpr color YELLOW_A700 = color::from_rgb(0xff, 0xd6, 0x00);

constexpr color AMBER_50 = color::from_rgb(0xff, 0xf8, 0xe1);
constexpr color AMBER_100 = color::from_rgb(0xff, 0xec, 0xb3);
constexpr color AMBER_200 = color::from_rgb(0xff, 0xe0, 0x82);
constexpr color AMBER_300 = color::from_rgb(0xff, 0xd5, 0x4f);
constexpr color AMBER_400 = color::from_rgb(0xff, 0xca, 0x28);
constexpr color AMBER_500 = color::from_rgb(0xff, 0xc1, 0x07);
constexpr color AMBER_600 = color::from_rgb(0xff, 0xb3, 0x00);
constexpr color AMBER_700 = color::from_rgb(0xff, 0xa0, 0x00);
constexpr color AMBER_800 = color::from_rgb(0xff, 0x8f, 0x00);
constexpr color AMBER_900 = color::from_rgb(0xff, 0x6f, 0x00);
constexpr color AMBER_A100 = color::from_rgb(0xff, 0xe5, 0x7f);
constexpr color AMBER_A200 = color::from_rgb(0xff, 0xd7, 0x40);
constexpr color AMBER_A400 = color::from_rgb(0xff, 0xc4, 0x00);
constexpr color AMBER_A700 = color::from_rgb(0xff, 0xab, 0x00);

constexpr color ORANGE_50 = color::from_rgb(0xff, 0xf3, 0xe0);
constexpr color ORANGE_100 = color::from_rgb(0xff, 0xe0, 0xb2);
constexpr color ORANGE_200 = color::from_rgb(0xff, 0xcc, 0x80);
constexpr color ORANGE_300 = color::from_rgb(0xff, 0xb7, 0x4d);
constexpr color ORANGE_400 = color::from_rgb(0xff, 0xa7, 0x26);
constexpr color ORANGE_500 = color::from_rgb(0xff, 0x98, 0x00);
constexpr color ORANGE_600 = color::from_rgb(0xfb, 0x8c, 0x00);
constexpr color ORANGE_700 = color::from_rgb(0xf5, 0x7c, 0x00);
constexpr color ORANGE_800 = color::from_rgb(0xef, 0x6c, 0x00);
constexpr color ORANGE_900 = color::from_rgb(0xe6, 0x51, 0x00);
constexpr color ORANGE_A100 = color::from_rgb(0xff, 0xd1, 0x80);
constexpr color ORANGE_A200 = color::from_rgb(0xff, 0xab, 0x40);
constexpr color ORANGE_A400 = color::from_rgb(0xff, 0x91, 0x00);
constexpr color ORANGE_A700 = color::from_rgb(0xff, 0x6d, 0x00);

constexpr color DEEP_ORANGE_50 = color::from_rgb(0xfb, 0xe9, 0xe7);
constexpr color DEEP_ORANGE_100 = color::from_rgb(0xff, 0xcc, 0xbc);
constexpr color DEEP_ORANGE_200 = color::from_rgb(0xff, 0xab, 0x91);
constexpr color DEEP_ORANGE_300 = color::from_rgb(0xff, 0x8a, 0x65);
constexpr color DEEP_ORANGE_400 = color::from_rgb(0xff, 0x70, 0x43);
constexpr color DEEP_ORANGE_500 = color::from_rgb(0xff, 0x57, 0x22);
constexpr color DEEP_ORANGE_600 = color::from_rgb(0xf4, 0x51, 0x1e);
constexpr color DEEP_ORANGE_700 = color::from_rgb(0xe6, 0x4a, 0x19);
constexpr color DEEP_ORANGE_800 = color::from_rgb(0xd8, 0x43, 0x15);
constexpr color DEEP_ORANGE_900 = color::from_rgb(0xbf, 0x36, 0x0c);
constexpr color DEEP_ORANGE_A100 = color::from_rgb(0xff, 0x9e, 0x80);
constexpr color DEEP_ORANGE_A200 = color::from_rgb(0xff, 0x6e, 0x40);
constexpr color DEEP_ORANGE_A400 = color::from_rgb(0xff, 0x3d, 0x00);
constexpr color DEEP_ORANGE_A700 = color::from_rgb(0xdd, 0x2c, 0x00);

constexpr color BROWN_50 = color::from_rgb(0xef, 0xeb, 0xe9);
constexpr color BROWN_100 = color::from_rgb(0xd7, 0xcc, 0xc8);
constexpr color BROWN_200 = color::from_rgb(0xbc, 0xaa, 0xa4);
constexpr color BROWN_300 = color::from_rgb(0xa1, 0x88, 0x7f);
constexpr color BROWN_400 = color::from_rgb(0x8d, 0x6e, 0x63);
constexpr color BROWN_500 = color::from_rgb(0x79, 0x55, 0x48);
constexpr color BROWN_600 = color::from_rgb(0x6d, 0x4c, 0x41);
constexpr color BROWN_700 = color::from_rgb(0x5d, 0x40, 0x37);
constexpr color BROWN_800 = color::from_rgb(0x4e, 0x34, 0x2e);
constexpr color BROWN_900 = color::from_rgb(0x3e, 0x27, 0x23);

constexpr color GRAY_50 = color::from_rgb(0xfa, 0xfa, 0xfa);
constexpr color GRAY_100 = color::from_rgb(0xf5, 0xf5, 0xf5);
constexpr color GRAY_200 = color::from_rgb(0xee, 0xee, 0xee);
constexpr color GRAY_300 = color::from_rgb(0xe0, 0xe0, 0xe0);
constexpr color GRAY_400 = color::from_rgb(0xbd, 0xbd, 0xbd);
constexpr color GRAY_500 = color::from_rgb(0x9e, 0x9e, 0x9e);
constexpr color GRAY_600 = color::from_rgb(0x75, 0x75, 0x75);
constexpr color GRAY_700 = color::from_rgb(0x61, 0x61, 0x61);
constexpr color GRAY_800 = color::from_rgb(0x42, 0x42, 0x42);
constexpr color GRAY_900 = color::from_rgb(0x21, 0x21, 0x21);

constexpr color BLUE_GRAY_50 = color::from_rgb(0xec, 0xef, 0xf1);
constexpr color BLUE_GRAY_100 = color::from_rgb(0xcf, 0xd8, 0xdc);
constexpr color BLUE_GRAY_200 = color::from_rgb(0xb0, 0xbe, 0xc5);
constexpr color BLUE_GRAY_300 = color::from_rgb(0x90, 0xa4, 0xae);
constexpr color BLUE_GRAY_400 = color::from_rgb(0x78, 0x90, 0x9c);
constexpr color BLUE_GRAY_500 = color::from_rgb(0x60, 0x7d, 0x8b);
constexpr color BLUE_GRAY_600 = color::from_rgb(0x54, 0x6e, 0x7a);
constexpr color BLUE_GRAY_700 = color::from_rgb(0x45, 0x5a, 0x64);
constexpr color BLUE_GRAY_800 = color::from_rgb(0x37, 0x47, 0x4f);
constexpr color BLUE_GRAY_900 = color::from_rgb(0x26, 0x32, 0x38);

constexpr color WHITE = color::from_rgb(0xff, 0xff, 0xff);
constexpr color BLACK = color::from_rgb(0x00, 0x00, 0x00);

}  // namespace material

}  // namespace asr
