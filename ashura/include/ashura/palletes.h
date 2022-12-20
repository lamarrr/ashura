
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
constexpr color RED_50 = color::from_rgb(0xffU, 0xebU, 0xeeU);
constexpr color RED_100 = color::from_rgb(0xffU, 0xcdU, 0xd2U);
constexpr color RED_200 = color::from_rgb(0xefU, 0x9aU, 0x9aU);
constexpr color RED_300 = color::from_rgb(0xe5U, 0x73U, 0x73U);
constexpr color RED_400 = color::from_rgb(0xefU, 0x53U, 0x50U);
constexpr color RED_500 = color::from_rgb(0xf4U, 0x43U, 0x36U);
constexpr color RED_600 = color::from_rgb(0xe5U, 0x39U, 0x35U);
constexpr color RED_700 = color::from_rgb(0xd3U, 0x2fU, 0x2fU);
constexpr color RED_800 = color::from_rgb(0xc6U, 0x28U, 0x28U);
constexpr color RED_900 = color::from_rgb(0xb7U, 0x1cU, 0x1cU);
constexpr color RED_A100 = color::from_rgb(0xffU, 0x8aU, 0x80U);
constexpr color RED_A200 = color::from_rgb(0xffU, 0x52U, 0x52U);
constexpr color RED_A400 = color::from_rgb(0xffU, 0x17U, 0x44U);
constexpr color RED_A700 = color::from_rgb(0xd5U, 0x00U, 0x00U);

constexpr color PINK_50 = color::from_rgb(0xfcU, 0xe4U, 0xecU);
constexpr color PINK_100 = color::from_rgb(0xf8U, 0xbbU, 0xd0U);
constexpr color PINK_200 = color::from_rgb(0xf4U, 0x8fU, 0xb1U);
constexpr color PINK_300 = color::from_rgb(0xf0U, 0x62U, 0x92U);
constexpr color PINK_400 = color::from_rgb(0xecU, 0x40U, 0x7aU);
constexpr color PINK_500 = color::from_rgb(0xe9U, 0x1eU, 0x63U);
constexpr color PINK_600 = color::from_rgb(0xd8U, 0x1bU, 0x60U);
constexpr color PINK_700 = color::from_rgb(0xc2U, 0x18U, 0x5bU);
constexpr color PINK_800 = color::from_rgb(0xadU, 0x14U, 0x57U);
constexpr color PINK_900 = color::from_rgb(0x88U, 0x0eU, 0x4fU);
constexpr color PINK_A100 = color::from_rgb(0xffU, 0x80U, 0xabU);
constexpr color PINK_A200 = color::from_rgb(0xffU, 0x40U, 0x81U);
constexpr color PINK_A400 = color::from_rgb(0xf5U, 0x00U, 0x57U);
constexpr color PINK_A700 = color::from_rgb(0xc5U, 0x11U, 0x62U);

constexpr color PURPLE_50 = color::from_rgb(0xf3U, 0xe5U, 0xf5U);
constexpr color PURPLE_100 = color::from_rgb(0xe1U, 0xbeU, 0xe7U);
constexpr color PURPLE_200 = color::from_rgb(0xceU, 0x93U, 0xd8U);
constexpr color PURPLE_300 = color::from_rgb(0xbaU, 0x68U, 0xc8U);
constexpr color PURPLE_400 = color::from_rgb(0xabU, 0x47U, 0xbcU);
constexpr color PURPLE_500 = color::from_rgb(0x9cU, 0x27U, 0xb0U);
constexpr color PURPLE_600 = color::from_rgb(0x8eU, 0x24U, 0xaaU);
constexpr color PURPLE_700 = color::from_rgb(0x7bU, 0x1fU, 0xa2U);
constexpr color PURPLE_800 = color::from_rgb(0x6aU, 0x1bU, 0x9aU);
constexpr color PURPLE_900 = color::from_rgb(0x4aU, 0x14U, 0x8cU);
constexpr color PURPLE_A100 = color::from_rgb(0xeaU, 0x80U, 0xfcU);
constexpr color PURPLE_A200 = color::from_rgb(0xe0U, 0x40U, 0xfbU);
constexpr color PURPLE_A400 = color::from_rgb(0xd5U, 0x00U, 0xf9U);
constexpr color PURPLE_A700 = color::from_rgb(0xaaU, 0x00U, 0xffU);

constexpr color DEEP_PURPLE_50 = color::from_rgb(0xedU, 0xe7U, 0xf6U);
constexpr color DEEP_PURPLE_100 = color::from_rgb(0xd1U, 0xc4U, 0xe9U);
constexpr color DEEP_PURPLE_200 = color::from_rgb(0xb3U, 0x9dU, 0xdbU);
constexpr color DEEP_PURPLE_300 = color::from_rgb(0x95U, 0x75U, 0xcdU);
constexpr color DEEP_PURPLE_400 = color::from_rgb(0x7eU, 0x57U, 0xc2U);
constexpr color DEEP_PURPLE_500 = color::from_rgb(0x67U, 0x3aU, 0xb7U);
constexpr color DEEP_PURPLE_600 = color::from_rgb(0x5eU, 0x35U, 0xb1U);
constexpr color DEEP_PURPLE_700 = color::from_rgb(0x51U, 0x2dU, 0xa8U);
constexpr color DEEP_PURPLE_800 = color::from_rgb(0x45U, 0x27U, 0xa0U);
constexpr color DEEP_PURPLE_900 = color::from_rgb(0x31U, 0x1bU, 0x92U);
constexpr color DEEP_PURPLE_A100 = color::from_rgb(0xb3U, 0x88U, 0xffU);
constexpr color DEEP_PURPLE_A200 = color::from_rgb(0x7cU, 0x4dU, 0xffU);
constexpr color DEEP_PURPLE_A400 = color::from_rgb(0x65U, 0x1fU, 0xffU);
constexpr color DEEP_PURPLE_A700 = color::from_rgb(0x62U, 0x00U, 0xeaU);

constexpr color INDIGO_50 = color::from_rgb(0xe8U, 0xeaU, 0xf6U);
constexpr color INDIGO_100 = color::from_rgb(0xc5U, 0xcaU, 0xe9U);
constexpr color INDIGO_200 = color::from_rgb(0x9fU, 0xa8U, 0xdaU);
constexpr color INDIGO_300 = color::from_rgb(0x79U, 0x86U, 0xcbU);
constexpr color INDIGO_400 = color::from_rgb(0x5cU, 0x6bU, 0xc0U);
constexpr color INDIGO_500 = color::from_rgb(0x3fU, 0x51U, 0xb5U);
constexpr color INDIGO_600 = color::from_rgb(0x39U, 0x49U, 0xabU);
constexpr color INDIGO_700 = color::from_rgb(0x30U, 0x3fU, 0x9fU);
constexpr color INDIGO_800 = color::from_rgb(0x28U, 0x35U, 0x93U);
constexpr color INDIGO_900 = color::from_rgb(0x1aU, 0x23U, 0x7eU);
constexpr color INDIGO_A100 = color::from_rgb(0x8cU, 0x9eU, 0xffU);
constexpr color INDIGO_A200 = color::from_rgb(0x53U, 0x6dU, 0xfeU);
constexpr color INDIGO_A400 = color::from_rgb(0x3dU, 0x5aU, 0xfeU);
constexpr color INDIGO_A700 = color::from_rgb(0x30U, 0x4fU, 0xfeU);

constexpr color BLUE_50 = color::from_rgb(0xe3U, 0xf2U, 0xfdU);
constexpr color BLUE_100 = color::from_rgb(0xbbU, 0xdeU, 0xfbU);
constexpr color BLUE_200 = color::from_rgb(0x90U, 0xcaU, 0xf9U);
constexpr color BLUE_300 = color::from_rgb(0x64U, 0xb5U, 0xf6U);
constexpr color BLUE_400 = color::from_rgb(0x42U, 0xa5U, 0xf5U);
constexpr color BLUE_500 = color::from_rgb(0x21U, 0x96U, 0xf3U);
constexpr color BLUE_600 = color::from_rgb(0x1eU, 0x88U, 0xe5U);
constexpr color BLUE_700 = color::from_rgb(0x19U, 0x76U, 0xd2U);
constexpr color BLUE_800 = color::from_rgb(0x15U, 0x65U, 0xc0U);
constexpr color BLUE_900 = color::from_rgb(0x0dU, 0x47U, 0xa1U);
constexpr color BLUE_A100 = color::from_rgb(0x82U, 0xb1U, 0xffU);
constexpr color BLUE_A200 = color::from_rgb(0x44U, 0x8aU, 0xffU);
constexpr color BLUE_A400 = color::from_rgb(0x29U, 0x79U, 0xffU);
constexpr color BLUE_A700 = color::from_rgb(0x29U, 0x62U, 0xffU);

constexpr color LIGHT_BLUE_50 = color::from_rgb(0xe1U, 0xf5U, 0xfeU);
constexpr color LIGHT_BLUE_100 = color::from_rgb(0xb3U, 0xe5U, 0xfcU);
constexpr color LIGHT_BLUE_200 = color::from_rgb(0x81U, 0xd4U, 0xfaU);
constexpr color LIGHT_BLUE_300 = color::from_rgb(0x4fU, 0xc3U, 0xf7U);
constexpr color LIGHT_BLUE_400 = color::from_rgb(0x29U, 0xb6U, 0xf6U);
constexpr color LIGHT_BLUE_500 = color::from_rgb(0x03U, 0xa9U, 0xf4U);
constexpr color LIGHT_BLUE_600 = color::from_rgb(0x03U, 0x9bU, 0xe5U);
constexpr color LIGHT_BLUE_700 = color::from_rgb(0x02U, 0x88U, 0xd1U);
constexpr color LIGHT_BLUE_800 = color::from_rgb(0x02U, 0x77U, 0xbdU);
constexpr color LIGHT_BLUE_900 = color::from_rgb(0x01U, 0x57U, 0x9bU);
constexpr color LIGHT_BLUE_A100 = color::from_rgb(0x80U, 0xd8U, 0xffU);
constexpr color LIGHT_BLUE_A200 = color::from_rgb(0x40U, 0xc4U, 0xffU);
constexpr color LIGHT_BLUE_A400 = color::from_rgb(0x00U, 0xb0U, 0xffU);
constexpr color LIGHT_BLUE_A700 = color::from_rgb(0x00U, 0x91U, 0xeaU);

constexpr color CYAN_50 = color::from_rgb(0xe0U, 0xf7U, 0xfaU);
constexpr color CYAN_100 = color::from_rgb(0xb2U, 0xebU, 0xf2U);
constexpr color CYAN_200 = color::from_rgb(0x80U, 0xdeU, 0xeaU);
constexpr color CYAN_300 = color::from_rgb(0x4dU, 0xd0U, 0xe1U);
constexpr color CYAN_400 = color::from_rgb(0x26U, 0xc6U, 0xdaU);
constexpr color CYAN_500 = color::from_rgb(0x00U, 0xbcU, 0xd4U);
constexpr color CYAN_600 = color::from_rgb(0x00U, 0xacU, 0xc1U);
constexpr color CYAN_700 = color::from_rgb(0x00U, 0x97U, 0xa7U);
constexpr color CYAN_800 = color::from_rgb(0x00U, 0x83U, 0x8fU);
constexpr color CYAN_900 = color::from_rgb(0x00U, 0x60U, 0x64U);
constexpr color CYAN_A100 = color::from_rgb(0x84U, 0xffU, 0xffU);
constexpr color CYAN_A200 = color::from_rgb(0x18U, 0xffU, 0xffU);
constexpr color CYAN_A400 = color::from_rgb(0x00U, 0xe5U, 0xffU);
constexpr color CYAN_A700 = color::from_rgb(0x00U, 0xb8U, 0xd4U);

constexpr color TEAL_50 = color::from_rgb(0xe0U, 0xf2U, 0xf1U);
constexpr color TEAL_100 = color::from_rgb(0xb2U, 0xdfU, 0xdbU);
constexpr color TEAL_200 = color::from_rgb(0x80U, 0xcbU, 0xc4U);
constexpr color TEAL_300 = color::from_rgb(0x4dU, 0xb6U, 0xacU);
constexpr color TEAL_400 = color::from_rgb(0x26U, 0xa6U, 0x9aU);
constexpr color TEAL_500 = color::from_rgb(0x00U, 0x96U, 0x88U);
constexpr color TEAL_600 = color::from_rgb(0x00U, 0x89U, 0x7bU);
constexpr color TEAL_700 = color::from_rgb(0x00U, 0x79U, 0x6bU);
constexpr color TEAL_800 = color::from_rgb(0x00U, 0x69U, 0x5cU);
constexpr color TEAL_900 = color::from_rgb(0x00U, 0x4dU, 0x40U);
constexpr color TEAL_A100 = color::from_rgb(0xa7U, 0xffU, 0xebU);
constexpr color TEAL_A200 = color::from_rgb(0x64U, 0xffU, 0xdaU);
constexpr color TEAL_A400 = color::from_rgb(0x1dU, 0xe9U, 0xb6U);
constexpr color TEAL_A700 = color::from_rgb(0x00U, 0xbfU, 0xa5U);

constexpr color GREEN_50 = color::from_rgb(0xe8U, 0xf5U, 0xe9U);
constexpr color GREEN_100 = color::from_rgb(0xc8U, 0xe6U, 0xc9U);
constexpr color GREEN_200 = color::from_rgb(0xa5U, 0xd6U, 0xa7U);
constexpr color GREEN_300 = color::from_rgb(0x81U, 0xc7U, 0x84U);
constexpr color GREEN_400 = color::from_rgb(0x66U, 0xbbU, 0x6aU);
constexpr color GREEN_500 = color::from_rgb(0x4cU, 0xafU, 0x50U);
constexpr color GREEN_600 = color::from_rgb(0x43U, 0xa0U, 0x47U);
constexpr color GREEN_700 = color::from_rgb(0x38U, 0x8eU, 0x3cU);
constexpr color GREEN_800 = color::from_rgb(0x2eU, 0x7dU, 0x32U);
constexpr color GREEN_900 = color::from_rgb(0x1bU, 0x5eU, 0x20U);
constexpr color GREEN_A100 = color::from_rgb(0xb9U, 0xf6U, 0xcaU);
constexpr color GREEN_A200 = color::from_rgb(0x69U, 0xf0U, 0xaeU);
constexpr color GREEN_A400 = color::from_rgb(0x00U, 0xe6U, 0x76U);
constexpr color GREEN_A700 = color::from_rgb(0x00U, 0xc8U, 0x53U);

constexpr color LIGHT_GREEN_50 = color::from_rgb(0xf1U, 0xf8U, 0xe9U);
constexpr color LIGHT_GREEN_100 = color::from_rgb(0xdcU, 0xedU, 0xc8U);
constexpr color LIGHT_GREEN_200 = color::from_rgb(0xc5U, 0xe1U, 0xa5U);
constexpr color LIGHT_GREEN_300 = color::from_rgb(0xaeU, 0xd5U, 0x81U);
constexpr color LIGHT_GREEN_400 = color::from_rgb(0x9cU, 0xccU, 0x65U);
constexpr color LIGHT_GREEN_500 = color::from_rgb(0x8bU, 0xc3U, 0x4aU);
constexpr color LIGHT_GREEN_600 = color::from_rgb(0x7cU, 0xb3U, 0x42U);
constexpr color LIGHT_GREEN_700 = color::from_rgb(0x68U, 0x9fU, 0x38U);
constexpr color LIGHT_GREEN_800 = color::from_rgb(0x55U, 0x8bU, 0x2fU);
constexpr color LIGHT_GREEN_900 = color::from_rgb(0x33U, 0x69U, 0x1eU);
constexpr color LIGHT_GREEN_A100 = color::from_rgb(0xccU, 0xffU, 0x90U);
constexpr color LIGHT_GREEN_A200 = color::from_rgb(0xb2U, 0xffU, 0x59U);
constexpr color LIGHT_GREEN_A400 = color::from_rgb(0x76U, 0xffU, 0x03U);
constexpr color LIGHT_GREEN_A700 = color::from_rgb(0x64U, 0xddU, 0x17U);

constexpr color LIME_50 = color::from_rgb(0xf9U, 0xfbU, 0xe7U);
constexpr color LIME_100 = color::from_rgb(0xf0U, 0xf4U, 0xc3U);
constexpr color LIME_200 = color::from_rgb(0xe6U, 0xeeU, 0x9cU);
constexpr color LIME_300 = color::from_rgb(0xdcU, 0xe7U, 0x75U);
constexpr color LIME_400 = color::from_rgb(0xd4U, 0xe1U, 0x57U);
constexpr color LIME_500 = color::from_rgb(0xcdU, 0xdcU, 0x39U);
constexpr color LIME_600 = color::from_rgb(0xc0U, 0xcaU, 0x33U);
constexpr color LIME_700 = color::from_rgb(0xafU, 0xb4U, 0x2bU);
constexpr color LIME_800 = color::from_rgb(0x9eU, 0x9dU, 0x24U);
constexpr color LIME_900 = color::from_rgb(0x82U, 0x77U, 0x17U);
constexpr color LIME_A100 = color::from_rgb(0xf4U, 0xffU, 0x81U);
constexpr color LIME_A200 = color::from_rgb(0xeeU, 0xffU, 0x41U);
constexpr color LIME_A400 = color::from_rgb(0xc6U, 0xffU, 0x00U);
constexpr color LIME_A700 = color::from_rgb(0xaeU, 0xeaU, 0x00U);

constexpr color YELLOW_50 = color::from_rgb(0xffU, 0xfdU, 0xe7U);
constexpr color YELLOW_100 = color::from_rgb(0xffU, 0xf9U, 0xc4U);
constexpr color YELLOW_200 = color::from_rgb(0xffU, 0xf5U, 0x9dU);
constexpr color YELLOW_300 = color::from_rgb(0xffU, 0xf1U, 0x76U);
constexpr color YELLOW_400 = color::from_rgb(0xffU, 0xeeU, 0x58U);
constexpr color YELLOW_500 = color::from_rgb(0xffU, 0xebU, 0x3bU);
constexpr color YELLOW_600 = color::from_rgb(0xfdU, 0xd8U, 0x35U);
constexpr color YELLOW_700 = color::from_rgb(0xfbU, 0xc0U, 0x2dU);
constexpr color YELLOW_800 = color::from_rgb(0xf9U, 0xa8U, 0x25U);
constexpr color YELLOW_900 = color::from_rgb(0xf5U, 0x7fU, 0x17U);
constexpr color YELLOW_A100 = color::from_rgb(0xffU, 0xffU, 0x8dU);
constexpr color YELLOW_A200 = color::from_rgb(0xffU, 0xffU, 0x00U);
constexpr color YELLOW_A400 = color::from_rgb(0xffU, 0xeaU, 0x00U);
constexpr color YELLOW_A700 = color::from_rgb(0xffU, 0xd6U, 0x00U);

constexpr color AMBER_50 = color::from_rgb(0xffU, 0xf8U, 0xe1U);
constexpr color AMBER_100 = color::from_rgb(0xffU, 0xecU, 0xb3U);
constexpr color AMBER_200 = color::from_rgb(0xffU, 0xe0U, 0x82U);
constexpr color AMBER_300 = color::from_rgb(0xffU, 0xd5U, 0x4fU);
constexpr color AMBER_400 = color::from_rgb(0xffU, 0xcaU, 0x28U);
constexpr color AMBER_500 = color::from_rgb(0xffU, 0xc1U, 0x07U);
constexpr color AMBER_600 = color::from_rgb(0xffU, 0xb3U, 0x00U);
constexpr color AMBER_700 = color::from_rgb(0xffU, 0xa0U, 0x00U);
constexpr color AMBER_800 = color::from_rgb(0xffU, 0x8fU, 0x00U);
constexpr color AMBER_900 = color::from_rgb(0xffU, 0x6fU, 0x00U);
constexpr color AMBER_A100 = color::from_rgb(0xffU, 0xe5U, 0x7fU);
constexpr color AMBER_A200 = color::from_rgb(0xffU, 0xd7U, 0x40U);
constexpr color AMBER_A400 = color::from_rgb(0xffU, 0xc4U, 0x00U);
constexpr color AMBER_A700 = color::from_rgb(0xffU, 0xabU, 0x00U);

constexpr color ORANGE_50 = color::from_rgb(0xffU, 0xf3U, 0xe0U);
constexpr color ORANGE_100 = color::from_rgb(0xffU, 0xe0U, 0xb2U);
constexpr color ORANGE_200 = color::from_rgb(0xffU, 0xccU, 0x80U);
constexpr color ORANGE_300 = color::from_rgb(0xffU, 0xb7U, 0x4dU);
constexpr color ORANGE_400 = color::from_rgb(0xffU, 0xa7U, 0x26U);
constexpr color ORANGE_500 = color::from_rgb(0xffU, 0x98U, 0x00U);
constexpr color ORANGE_600 = color::from_rgb(0xfbU, 0x8cU, 0x00U);
constexpr color ORANGE_700 = color::from_rgb(0xf5U, 0x7cU, 0x00U);
constexpr color ORANGE_800 = color::from_rgb(0xefU, 0x6cU, 0x00U);
constexpr color ORANGE_900 = color::from_rgb(0xe6U, 0x51U, 0x00U);
constexpr color ORANGE_A100 = color::from_rgb(0xffU, 0xd1U, 0x80U);
constexpr color ORANGE_A200 = color::from_rgb(0xffU, 0xabU, 0x40U);
constexpr color ORANGE_A400 = color::from_rgb(0xffU, 0x91U, 0x00U);
constexpr color ORANGE_A700 = color::from_rgb(0xffU, 0x6dU, 0x00U);

constexpr color DEEP_ORANGE_50 = color::from_rgb(0xfbU, 0xe9U, 0xe7U);
constexpr color DEEP_ORANGE_100 = color::from_rgb(0xffU, 0xccU, 0xbcU);
constexpr color DEEP_ORANGE_200 = color::from_rgb(0xffU, 0xabU, 0x91U);
constexpr color DEEP_ORANGE_300 = color::from_rgb(0xffU, 0x8aU, 0x65U);
constexpr color DEEP_ORANGE_400 = color::from_rgb(0xffU, 0x70U, 0x43U);
constexpr color DEEP_ORANGE_500 = color::from_rgb(0xffU, 0x57U, 0x22U);
constexpr color DEEP_ORANGE_600 = color::from_rgb(0xf4U, 0x51U, 0x1eU);
constexpr color DEEP_ORANGE_700 = color::from_rgb(0xe6U, 0x4aU, 0x19U);
constexpr color DEEP_ORANGE_800 = color::from_rgb(0xd8U, 0x43U, 0x15U);
constexpr color DEEP_ORANGE_900 = color::from_rgb(0xbfU, 0x36U, 0x0cU);
constexpr color DEEP_ORANGE_A100 = color::from_rgb(0xffU, 0x9eU, 0x80U);
constexpr color DEEP_ORANGE_A200 = color::from_rgb(0xffU, 0x6eU, 0x40U);
constexpr color DEEP_ORANGE_A400 = color::from_rgb(0xffU, 0x3dU, 0x00U);
constexpr color DEEP_ORANGE_A700 = color::from_rgb(0xddU, 0x2cU, 0x00U);

constexpr color BROWN_50 = color::from_rgb(0xefU, 0xebU, 0xe9U);
constexpr color BROWN_100 = color::from_rgb(0xd7U, 0xccU, 0xc8U);
constexpr color BROWN_200 = color::from_rgb(0xbcU, 0xaaU, 0xa4U);
constexpr color BROWN_300 = color::from_rgb(0xa1U, 0x88U, 0x7fU);
constexpr color BROWN_400 = color::from_rgb(0x8dU, 0x6eU, 0x63U);
constexpr color BROWN_500 = color::from_rgb(0x79U, 0x55U, 0x48U);
constexpr color BROWN_600 = color::from_rgb(0x6dU, 0x4cU, 0x41U);
constexpr color BROWN_700 = color::from_rgb(0x5dU, 0x40U, 0x37U);
constexpr color BROWN_800 = color::from_rgb(0x4eU, 0x34U, 0x2eU);
constexpr color BROWN_900 = color::from_rgb(0x3eU, 0x27U, 0x23U);

constexpr color GRAY_50 = color::from_rgb(0xfaU, 0xfaU, 0xfaU);
constexpr color GRAY_100 = color::from_rgb(0xf5U, 0xf5U, 0xf5U);
constexpr color GRAY_200 = color::from_rgb(0xeeU, 0xeeU, 0xeeU);
constexpr color GRAY_300 = color::from_rgb(0xe0U, 0xe0U, 0xe0U);
constexpr color GRAY_400 = color::from_rgb(0xbdU, 0xbdU, 0xbdU);
constexpr color GRAY_500 = color::from_rgb(0x9eU, 0x9eU, 0x9eU);
constexpr color GRAY_600 = color::from_rgb(0x75U, 0x75U, 0x75U);
constexpr color GRAY_700 = color::from_rgb(0x61U, 0x61U, 0x61U);
constexpr color GRAY_800 = color::from_rgb(0x42U, 0x42U, 0x42U);
constexpr color GRAY_900 = color::from_rgb(0x21U, 0x21U, 0x21U);

constexpr color BLUE_GRAY_50 = color::from_rgb(0xecU, 0xefU, 0xf1U);
constexpr color BLUE_GRAY_100 = color::from_rgb(0xcfU, 0xd8U, 0xdcU);
constexpr color BLUE_GRAY_200 = color::from_rgb(0xb0U, 0xbeU, 0xc5U);
constexpr color BLUE_GRAY_300 = color::from_rgb(0x90U, 0xa4U, 0xaeU);
constexpr color BLUE_GRAY_400 = color::from_rgb(0x78U, 0x90U, 0x9cU);
constexpr color BLUE_GRAY_500 = color::from_rgb(0x60U, 0x7dU, 0x8bU);
constexpr color BLUE_GRAY_600 = color::from_rgb(0x54U, 0x6eU, 0x7aU);
constexpr color BLUE_GRAY_700 = color::from_rgb(0x45U, 0x5aU, 0x64U);
constexpr color BLUE_GRAY_800 = color::from_rgb(0x37U, 0x47U, 0x4fU);
constexpr color BLUE_GRAY_900 = color::from_rgb(0x26U, 0x32U, 0x38U);

constexpr color WHITE = color::from_rgb(0xffU, 0xffU, 0xffU);
constexpr color BLACK = color::from_rgb(0x00U, 0x00U, 0x00U);

}  // namespace material

}  // namespace asr