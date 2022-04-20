
#pragma once

#include "vlk/primitives.h"

namespace vlk {
namespace ui {

// ios default system colors
namespace ios {
constexpr Color LIGHT_BLUE = Color::from_rgb(0, 122, 255);
constexpr Color DARK_BLUE = Color::from_rgb(10, 132, 255);
constexpr Color LIGHT_BROWN = Color::from_rgb(162, 132, 94);
constexpr Color DARK_BROWN = Color::from_rgb(172, 142, 104);
constexpr Color LIGHT_CYAN = Color::from_rgb(50, 173, 230);
constexpr Color DARK_CYAN = Color::from_rgb(100, 210, 255);
constexpr Color LIGHT_GREEN = Color::from_rgb(52, 199, 89);
constexpr Color DARK_GREEN = Color::from_rgb(48, 209, 88);
constexpr Color LIGHT_INDIGO = Color::from_rgb(88, 86, 214);
constexpr Color DARK_INDIGO = Color::from_rgb(94, 92, 230);
constexpr Color LIGHT_MINT = Color::from_rgb(0, 199, 190);
constexpr Color DARK_MINT = Color::from_rgb(102, 212, 207);
constexpr Color LIGHT_ORANGE = Color::from_rgb(255, 149, 0);
constexpr Color DARK_ORANGE = Color::from_rgb(255, 159, 10);
constexpr Color LIGHT_PINK = Color::from_rgb(255, 45, 85);
constexpr Color DARK_PINK = Color::from_rgb(255, 55, 95);
constexpr Color LIGHT_PURPLE = Color::from_rgb(175, 82, 222);
constexpr Color DARK_PURPLE = Color::from_rgb(191, 90, 242);
constexpr Color LIGHT_RED = Color::from_rgb(255, 59, 48);
constexpr Color DARK_RED = Color::from_rgb(255, 69, 58);
constexpr Color LIGHT_TEAL = Color::from_rgb(48, 176, 199);
constexpr Color DARK_TEAL = Color::from_rgb(64, 200, 224);
constexpr Color LIGHT_YELLOW = Color::from_rgb(255, 204, 0);
constexpr Color DARK_YELLOW = Color::from_rgb(255, 214, 10);

constexpr Color LIGHT_GRAY = Color::from_rgb(142, 142, 147);
constexpr Color DARK_GRAY = Color::from_rgb(142, 142, 147);
constexpr Color LIGHT_GRAY_2 = Color::from_rgb(174, 174, 178);
constexpr Color DARK_GRAY_2 = Color::from_rgb(99, 99, 102);
constexpr Color LIGHT_GRAY_3 = Color::from_rgb(199, 199, 204);
constexpr Color DARK_GRAY_3 = Color::from_rgb(72, 72, 74);
constexpr Color LIGHT_GRAY_4 = Color::from_rgb(209, 209, 214);
constexpr Color DARK_GRAY_4 = Color::from_rgb(58, 58, 60);
constexpr Color LIGHT_GRAY_5 = Color::from_rgb(229, 229, 234);
constexpr Color DARK_GRAY_5 = Color::from_rgb(44, 44, 46);
constexpr Color LIGHT_GRAY_6 = Color::from_rgb(242, 242, 247);
constexpr Color DARK_GRAY_6 = Color::from_rgb(28, 28, 30);
}  // namespace ios

// ios accessible colors
namespace ios_accessible {
constexpr Color LIGHT_BLUE = Color::from_rgb(0, 64, 221);
constexpr Color DARK_BLUE = Color::from_rgb(64, 156, 255);
constexpr Color LIGHT_BROWN = Color::from_rgb(127, 101, 69);
constexpr Color DARK_BROWN = Color::from_rgb(181, 148, 105);
constexpr Color LIGHT_CYAN = Color::from_rgb(0, 113, 164);
constexpr Color DARK_CYAN = Color::from_rgb(112, 215, 255);
constexpr Color LIGHT_GREEN = Color::from_rgb(36, 138, 61);
constexpr Color DARK_GREEN = Color::from_rgb(48, 219, 91);
constexpr Color LIGHT_INDIGO = Color::from_rgb(54, 52, 163);
constexpr Color DARK_INDIGO = Color::from_rgb(125, 122, 255);
constexpr Color LIGHT_MINT = Color::from_rgb(12, 129, 123);
constexpr Color DARK_MINT = Color::from_rgb(102, 212, 207);
constexpr Color LIGHT_ORANGE = Color::from_rgb(201, 52, 0);
constexpr Color DARK_ORANGE = Color::from_rgb(255, 179, 64);
constexpr Color LIGHT_PINK = Color::from_rgb(211, 15, 69);
constexpr Color DARK_PINK = Color::from_rgb(255, 100, 130);
constexpr Color LIGHT_PURPLE = Color::from_rgb(137, 68, 171);
constexpr Color DARK_PURPLE = Color::from_rgb(218, 143, 255);
constexpr Color LIGHT_RED = Color::from_rgb(215, 0, 21);
constexpr Color DARK_RED = Color::from_rgb(255, 105, 97);
constexpr Color LIGHT_TEAL = Color::from_rgb(0, 130, 153);
constexpr Color DARK_TEAL = Color::from_rgb(93, 230, 255);
constexpr Color LIGHT_YELLOW = Color::from_rgb(178, 80, 0);
constexpr Color DARK_YELLOW = Color::from_rgb(255, 212, 38);

constexpr Color LIGHT_GRAY = Color::from_rgb(108, 108, 112);
constexpr Color DARK_GRAY = Color::from_rgb(174, 174, 178);
constexpr Color LIGHT_GRAY_2 = Color::from_rgb(142, 142, 147);
constexpr Color DARK_GRAY_2 = Color::from_rgb(124, 124, 128);
constexpr Color LIGHT_GRAY_3 = Color::from_rgb(174, 174, 178);
constexpr Color DARK_GRAY_3 = Color::from_rgb(84, 84, 86);
constexpr Color LIGHT_GRAY_4 = Color::from_rgb(188, 188, 192);
constexpr Color DARK_GRAY_4 = Color::from_rgb(68, 68, 70);
constexpr Color LIGHT_GRAY_5 = Color::from_rgb(216, 216, 220);
constexpr Color DARK_GRAY_5 = Color::from_rgb(54, 54, 56);
constexpr Color LIGHT_GRAY_6 = Color::from_rgb(235, 235, 240);
constexpr Color DARK_GRAY_6 = Color::from_rgb(36, 36, 36);
}  // namespace ios_accessible

}  // namespace ui
}  // namespace vlk
