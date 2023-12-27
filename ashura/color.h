
#pragma once

#include "ashura/types.h"

namespace ash
{

typedef Vec4 Color;

namespace colors
{
constexpr Color TRANSPARENT{0x00 / 255.0F, 0x00 / 255.0F, 0x00 / 255.0F,
                            0x00 / 255.0F};
constexpr Color WHITE{0xff / 255.0F, 0xff / 255.0F, 0xff / 255.0F,
                      0xff / 255.0F};
constexpr Color BLACK{0x00 / 255.0F, 0x00 / 255.0F, 0x00 / 255.0F,
                      0xff / 255.0F};
constexpr Color RED{0xff / 255.0F, 0x00 / 255.0F, 0x00 / 255.0F, 0xff / 255.0F};
constexpr Color BLUE{0x00 / 255.0F, 0x00 / 255.0F, 0xff / 255.0F,
                     0xff / 255.0F};
constexpr Color GREEN{0x00 / 255.0F, 0xff / 255.0F, 0x00 / 255.0F,
                      0xff / 255.0F};
constexpr Color CYAN{0x00 / 255.0F, 0xff / 255.0F, 0xff / 255.0F,
                     0xff / 255.0F};
constexpr Color MAGENTA{0xff / 255.0F, 0x00 / 255.0F, 0xff / 255.0F,
                        0xff / 255.0F};
constexpr Color YELLOW{0xff / 255.0F, 0xff / 255.0F, 0x00 / 255.0F,
                       0xff / 255.0F};
}        // namespace colors

// ios default system colors
namespace ios
{
constexpr Color LIGHT_BLUE{0 / 255.0F, 122 / 255.0F, 255 / 255.0F,
                           255 / 255.0F};
constexpr Color DARK_BLUE{10 / 255.0F, 132 / 255.0F, 255 / 255.0F,
                          255 / 255.0F};
constexpr Color LIGHT_BROWN{162 / 255.0F, 132 / 255.0F, 94 / 255.0F,
                            255 / 255.0F};
constexpr Color DARK_BROWN{172 / 255.0F, 142 / 255.0F, 104 / 255.0F,
                           255 / 255.0F};
constexpr Color LIGHT_CYAN{50 / 255.0F, 173 / 255.0F, 230 / 255.0F,
                           255 / 255.0F};
constexpr Color DARK_CYAN{100 / 255.0F, 210 / 255.0F, 255 / 255.0F,
                          255 / 255.0F};
constexpr Color LIGHT_GREEN{52 / 255.0F, 199 / 255.0F, 89 / 255.0F,
                            255 / 255.0F};
constexpr Color DARK_GREEN{48 / 255.0F, 209 / 255.0F, 88 / 255.0F,
                           255 / 255.0F};
constexpr Color LIGHT_INDIGO{88 / 255.0F, 86 / 255.0F, 214 / 255.0F,
                             255 / 255.0F};
constexpr Color DARK_INDIGO{94 / 255.0F, 92 / 255.0F, 230 / 255.0F,
                            255 / 255.0F};
constexpr Color LIGHT_MINT{0 / 255.0F, 199 / 255.0F, 190 / 255.0F,
                           255 / 255.0F};
constexpr Color DARK_MINT{102 / 255.0F, 212 / 255.0F, 207 / 255.0F,
                          255 / 255.0F};
constexpr Color LIGHT_ORANGE{255 / 255.0F, 149 / 255.0F, 0 / 255.0F,
                             255 / 255.0F};
constexpr Color DARK_ORANGE{255 / 255.0F, 159 / 255.0F, 10 / 255.0F,
                            255 / 255.0F};
constexpr Color LIGHT_PINK{255 / 255.0F, 45 / 255.0F, 85 / 255.0F,
                           255 / 255.0F};
constexpr Color DARK_PINK{255 / 255.0F, 55 / 255.0F, 95 / 255.0F, 255 / 255.0F};
constexpr Color LIGHT_PURPLE{175 / 255.0F, 82 / 255.0F, 222 / 255.0F,
                             255 / 255.0F};
constexpr Color DARK_PURPLE{191 / 255.0F, 90 / 255.0F, 242 / 255.0F,
                            255 / 255.0F};
constexpr Color LIGHT_RED{255 / 255.0F, 59 / 255.0F, 48 / 255.0F, 255 / 255.0F};
constexpr Color DARK_RED{255 / 255.0F, 69 / 255.0F, 58 / 255.0F, 255 / 255.0F};
constexpr Color LIGHT_TEAL{48 / 255.0F, 176 / 255.0F, 199 / 255.0F,
                           255 / 255.0F};
constexpr Color DARK_TEAL{64 / 255.0F, 200 / 255.0F, 224 / 255.0F,
                          255 / 255.0F};
constexpr Color LIGHT_YELLOW{255 / 255.0F, 204 / 255.0F, 0 / 255.0F,
                             255 / 255.0F};
constexpr Color DARK_YELLOW{255 / 255.0F, 214 / 255.0F, 10 / 255.0F,
                            255 / 255.0F};

constexpr Color LIGHT_GRAY{142 / 255.0F, 142 / 255.0F, 147 / 255.0F,
                           255 / 255.0F};
constexpr Color DARK_GRAY{142 / 255.0F, 142 / 255.0F, 147 / 255.0F,
                          255 / 255.0F};
constexpr Color LIGHT_GRAY_2{174 / 255.0F, 174 / 255.0F, 178 / 255.0F,
                             255 / 255.0F};
constexpr Color DARK_GRAY_2{99 / 255.0F, 99 / 255.0F, 102 / 255.0F,
                            255 / 255.0F};
constexpr Color LIGHT_GRAY_3{199 / 255.0F, 199 / 255.0F, 204 / 255.0F,
                             255 / 255.0F};
constexpr Color DARK_GRAY_3{72 / 255.0F, 72 / 255.0F, 74 / 255.0F,
                            255 / 255.0F};
constexpr Color LIGHT_GRAY_4{209 / 255.0F, 209 / 255.0F, 214 / 255.0F,
                             255 / 255.0F};
constexpr Color DARK_GRAY_4{58 / 255.0F, 58 / 255.0F, 60 / 255.0F,
                            255 / 255.0F};
constexpr Color LIGHT_GRAY_5{229 / 255.0F, 229 / 255.0F, 234 / 255.0F,
                             255 / 255.0F};
constexpr Color DARK_GRAY_5{44 / 255.0F, 44 / 255.0F, 46 / 255.0F,
                            255 / 255.0F};
constexpr Color LIGHT_GRAY_6{242 / 255.0F, 242 / 255.0F, 247 / 255.0F,
                             255 / 255.0F};
constexpr Color DARK_GRAY_6{28 / 255.0F, 28 / 255.0F, 30 / 255.0F,
                            255 / 255.0F};

// ios accessible colors
namespace accessible
{
constexpr Color LIGHT_BLUE{0 / 255.0F, 64 / 255.0F, 221 / 255.0F, 255 / 255.0F};
constexpr Color DARK_BLUE{64 / 255.0F, 156 / 255.0F, 255 / 255.0F,
                          255 / 255.0F};
constexpr Color LIGHT_BROWN{127 / 255.0F, 101 / 255.0F, 69 / 255.0F,
                            255 / 255.0F};
constexpr Color DARK_BROWN{181 / 255.0F, 148 / 255.0F, 105 / 255.0F,
                           255 / 255.0F};
constexpr Color LIGHT_CYAN{0 / 255.0F, 113 / 255.0F, 164 / 255.0F,
                           255 / 255.0F};
constexpr Color DARK_CYAN{112 / 255.0F, 215 / 255.0F, 255 / 255.0F,
                          255 / 255.0F};
constexpr Color LIGHT_GREEN{36 / 255.0F, 138 / 255.0F, 61 / 255.0F,
                            255 / 255.0F};
constexpr Color DARK_GREEN{48 / 255.0F, 219 / 255.0F, 91 / 255.0F,
                           255 / 255.0F};
constexpr Color LIGHT_INDIGO{54 / 255.0F, 52 / 255.0F, 163 / 255.0F,
                             255 / 255.0F};
constexpr Color DARK_INDIGO{125 / 255.0F, 122 / 255.0F, 255 / 255.0F,
                            255 / 255.0F};
constexpr Color LIGHT_MINT{12 / 255.0F, 129 / 255.0F, 123 / 255.0F,
                           255 / 255.0F};
constexpr Color DARK_MINT{102 / 255.0F, 212 / 255.0F, 207 / 255.0F,
                          255 / 255.0F};
constexpr Color LIGHT_ORANGE{201 / 255.0F, 52 / 255.0F, 0 / 255.0F,
                             255 / 255.0F};
constexpr Color DARK_ORANGE{255 / 255.0F, 179 / 255.0F, 64 / 255.0F,
                            255 / 255.0F};
constexpr Color LIGHT_PINK{211 / 255.0F, 15 / 255.0F, 69 / 255.0F,
                           255 / 255.0F};
constexpr Color DARK_PINK{255 / 255.0F, 100 / 255.0F, 130 / 255.0F,
                          255 / 255.0F};
constexpr Color LIGHT_PURPLE{137 / 255.0F, 68 / 255.0F, 171 / 255.0F,
                             255 / 255.0F};
constexpr Color DARK_PURPLE{218 / 255.0F, 143 / 255.0F, 255 / 255.0F,
                            255 / 255.0F};
constexpr Color LIGHT_RED{215 / 255.0F, 0 / 255.0F, 21 / 255.0F, 255 / 255.0F};
constexpr Color DARK_RED{255 / 255.0F, 105 / 255.0F, 97 / 255.0F, 255 / 255.0F};
constexpr Color LIGHT_TEAL{0 / 255.0F, 130 / 255.0F, 153 / 255.0F,
                           255 / 255.0F};
constexpr Color DARK_TEAL{93 / 255.0F, 230 / 255.0F, 255 / 255.0F,
                          255 / 255.0F};
constexpr Color LIGHT_YELLOW{178 / 255.0F, 80 / 255.0F, 0 / 255.0F,
                             255 / 255.0F};
constexpr Color DARK_YELLOW{255 / 255.0F, 212 / 255.0F, 38 / 255.0F,
                            255 / 255.0F};

constexpr Color LIGHT_GRAY{108 / 255.0F, 108 / 255.0F, 112 / 255.0F,
                           255 / 255.0F};
constexpr Color DARK_GRAY{174 / 255.0F, 174 / 255.0F, 178 / 255.0F,
                          255 / 255.0F};
constexpr Color LIGHT_GRAY_2{142 / 255.0F, 142 / 255.0F, 147 / 255.0F,
                             255 / 255.0F};
constexpr Color DARK_GRAY_2{124 / 255.0F, 124 / 255.0F, 128 / 255.0F,
                            255 / 255.0F};
constexpr Color LIGHT_GRAY_3{174 / 255.0F, 174 / 255.0F, 178 / 255.0F,
                             255 / 255.0F};
constexpr Color DARK_GRAY_3{84 / 255.0F, 84 / 255.0F, 86 / 255.0F,
                            255 / 255.0F};
constexpr Color LIGHT_GRAY_4{188 / 255.0F, 188 / 255.0F, 192 / 255.0F,
                             255 / 255.0F};
constexpr Color DARK_GRAY_4{68 / 255.0F, 68 / 255.0F, 70 / 255.0F,
                            255 / 255.0F};
constexpr Color LIGHT_GRAY_5{216 / 255.0F, 216 / 255.0F, 220 / 255.0F,
                             255 / 255.0F};
constexpr Color DARK_GRAY_5{54 / 255.0F, 54 / 255.0F, 56 / 255.0F,
                            255 / 255.0F};
constexpr Color LIGHT_GRAY_6{235 / 255.0F, 235 / 255.0F, 240 / 255.0F,
                             255 / 255.0F};
constexpr Color DARK_GRAY_6{36 / 255.0F, 36 / 255.0F, 36 / 255.0F,
                            255 / 255.0F};
}        // namespace accessible

}        // namespace ios

namespace material
{
constexpr Color RED_50{0xff / 255.0F, 0xeb / 255.0F, 0xee / 255.0F,
                       255 / 255.0F};
constexpr Color RED_100{0xff / 255.0F, 0xcd / 255.0F, 0xd2 / 255.0F,
                        255 / 255.0F};
constexpr Color RED_200{0xef / 255.0F, 0x9a / 255.0F, 0x9a / 255.0F,
                        255 / 255.0F};
constexpr Color RED_300{0xe5 / 255.0F, 0x73 / 255.0F, 0x73 / 255.0F,
                        255 / 255.0F};
constexpr Color RED_400{0xef / 255.0F, 0x53 / 255.0F, 0x50 / 255.0F,
                        255 / 255.0F};
constexpr Color RED_500{0xf4 / 255.0F, 0x43 / 255.0F, 0x36 / 255.0F,
                        255 / 255.0F};
constexpr Color RED_600{0xe5 / 255.0F, 0x39 / 255.0F, 0x35 / 255.0F,
                        255 / 255.0F};
constexpr Color RED_700{0xd3 / 255.0F, 0x2f / 255.0F, 0x2f / 255.0F,
                        255 / 255.0F};
constexpr Color RED_800{0xc6 / 255.0F, 0x28 / 255.0F, 0x28 / 255.0F,
                        255 / 255.0F};
constexpr Color RED_900{0xb7 / 255.0F, 0x1c / 255.0F, 0x1c / 255.0F,
                        255 / 255.0F};
constexpr Color RED_A100{0xff / 255.0F, 0x8a / 255.0F, 0x80 / 255.0F,
                         255 / 255.0F};
constexpr Color RED_A200{0xff / 255.0F, 0x52 / 255.0F, 0x52 / 255.0F,
                         255 / 255.0F};
constexpr Color RED_A400{0xff / 255.0F, 0x17 / 255.0F, 0x44 / 255.0F,
                         255 / 255.0F};
constexpr Color RED_A700{0xd5 / 255.0F, 0x00 / 255.0F, 0x00 / 255.0F,
                         255 / 255.0F};

constexpr Color PINK_50{0xfc / 255.0F, 0xe4 / 255.0F, 0xec / 255.0F,
                        255 / 255.0F};
constexpr Color PINK_100{0xf8 / 255.0F, 0xbb / 255.0F, 0xd0 / 255.0F,
                         255 / 255.0F};
constexpr Color PINK_200{0xf4 / 255.0F, 0x8f / 255.0F, 0xb1 / 255.0F,
                         255 / 255.0F};
constexpr Color PINK_300{0xf0 / 255.0F, 0x62 / 255.0F, 0x92 / 255.0F,
                         255 / 255.0F};
constexpr Color PINK_400{0xec / 255.0F, 0x40 / 255.0F, 0x7a / 255.0F,
                         255 / 255.0F};
constexpr Color PINK_500{0xe9 / 255.0F, 0x1e / 255.0F, 0x63 / 255.0F,
                         255 / 255.0F};
constexpr Color PINK_600{0xd8 / 255.0F, 0x1b / 255.0F, 0x60 / 255.0F,
                         255 / 255.0F};
constexpr Color PINK_700{0xc2 / 255.0F, 0x18 / 255.0F, 0x5b / 255.0F,
                         255 / 255.0F};
constexpr Color PINK_800{0xad / 255.0F, 0x14 / 255.0F, 0x57 / 255.0F,
                         255 / 255.0F};
constexpr Color PINK_900{0x88 / 255.0F, 0x0e / 255.0F, 0x4f / 255.0F,
                         255 / 255.0F};
constexpr Color PINK_A100{0xff / 255.0F, 0x80 / 255.0F, 0xab / 255.0F,
                          255 / 255.0F};
constexpr Color PINK_A200{0xff / 255.0F, 0x40 / 255.0F, 0x81 / 255.0F,
                          255 / 255.0F};
constexpr Color PINK_A400{0xf5 / 255.0F, 0x00 / 255.0F, 0x57 / 255.0F,
                          255 / 255.0F};
constexpr Color PINK_A700{0xc5 / 255.0F, 0x11 / 255.0F, 0x62 / 255.0F,
                          255 / 255.0F};

constexpr Color PURPLE_50{0xf3 / 255.0F, 0xe5 / 255.0F, 0xf5 / 255.0F,
                          255 / 255.0F};
constexpr Color PURPLE_100{0xe1 / 255.0F, 0xbe / 255.0F, 0xe7 / 255.0F,
                           255 / 255.0F};
constexpr Color PURPLE_200{0xce / 255.0F, 0x93 / 255.0F, 0xd8 / 255.0F,
                           255 / 255.0F};
constexpr Color PURPLE_300{0xba / 255.0F, 0x68 / 255.0F, 0xc8 / 255.0F,
                           255 / 255.0F};
constexpr Color PURPLE_400{0xab / 255.0F, 0x47 / 255.0F, 0xbc / 255.0F,
                           255 / 255.0F};
constexpr Color PURPLE_500{0x9c / 255.0F, 0x27 / 255.0F, 0xb0 / 255.0F,
                           255 / 255.0F};
constexpr Color PURPLE_600{0x8e / 255.0F, 0x24 / 255.0F, 0xaa / 255.0F,
                           255 / 255.0F};
constexpr Color PURPLE_700{0x7b / 255.0F, 0x1f / 255.0F, 0xa2 / 255.0F,
                           255 / 255.0F};
constexpr Color PURPLE_800{0x6a / 255.0F, 0x1b / 255.0F, 0x9a / 255.0F,
                           255 / 255.0F};
constexpr Color PURPLE_900{0x4a / 255.0F, 0x14 / 255.0F, 0x8c / 255.0F,
                           255 / 255.0F};
constexpr Color PURPLE_A100{0xea / 255.0F, 0x80 / 255.0F, 0xfc / 255.0F,
                            255 / 255.0F};
constexpr Color PURPLE_A200{0xe0 / 255.0F, 0x40 / 255.0F, 0xfb / 255.0F,
                            255 / 255.0F};
constexpr Color PURPLE_A400{0xd5 / 255.0F, 0x00 / 255.0F, 0xf9 / 255.0F,
                            255 / 255.0F};
constexpr Color PURPLE_A700{0xaa / 255.0F, 0x00 / 255.0F, 0xff / 255.0F,
                            255 / 255.0F};

constexpr Color DEEP_PURPLE_50{0xed / 255.0F, 0xe7 / 255.0F, 0xf6 / 255.0F,
                               255 / 255.0F};
constexpr Color DEEP_PURPLE_100{0xd1 / 255.0F, 0xc4 / 255.0F, 0xe9 / 255.0F,
                                255 / 255.0F};
constexpr Color DEEP_PURPLE_200{0xb3 / 255.0F, 0x9d / 255.0F, 0xdb / 255.0F,
                                255 / 255.0F};
constexpr Color DEEP_PURPLE_300{0x95 / 255.0F, 0x75 / 255.0F, 0xcd / 255.0F,
                                255 / 255.0F};
constexpr Color DEEP_PURPLE_400{0x7e / 255.0F, 0x57 / 255.0F, 0xc2 / 255.0F,
                                255 / 255.0F};
constexpr Color DEEP_PURPLE_500{0x67 / 255.0F, 0x3a / 255.0F, 0xb7 / 255.0F,
                                255 / 255.0F};
constexpr Color DEEP_PURPLE_600{0x5e / 255.0F, 0x35 / 255.0F, 0xb1 / 255.0F,
                                255 / 255.0F};
constexpr Color DEEP_PURPLE_700{0x51 / 255.0F, 0x2d / 255.0F, 0xa8 / 255.0F,
                                255 / 255.0F};
constexpr Color DEEP_PURPLE_800{0x45 / 255.0F, 0x27 / 255.0F, 0xa0 / 255.0F,
                                255 / 255.0F};
constexpr Color DEEP_PURPLE_900{0x31 / 255.0F, 0x1b / 255.0F, 0x92 / 255.0F,
                                255 / 255.0F};
constexpr Color DEEP_PURPLE_A100{0xb3 / 255.0F, 0x88 / 255.0F, 0xff / 255.0F,
                                 255 / 255.0F};
constexpr Color DEEP_PURPLE_A200{0x7c / 255.0F, 0x4d / 255.0F, 0xff / 255.0F,
                                 255 / 255.0F};
constexpr Color DEEP_PURPLE_A400{0x65 / 255.0F, 0x1f / 255.0F, 0xff / 255.0F,
                                 255 / 255.0F};
constexpr Color DEEP_PURPLE_A700{0x62 / 255.0F, 0x00 / 255.0F, 0xea / 255.0F,
                                 255 / 255.0F};

constexpr Color INDIGO_50{0xe8 / 255.0F, 0xea / 255.0F, 0xf6 / 255.0F,
                          255 / 255.0F};
constexpr Color INDIGO_100{0xc5 / 255.0F, 0xca / 255.0F, 0xe9 / 255.0F,
                           255 / 255.0F};
constexpr Color INDIGO_200{0x9f / 255.0F, 0xa8 / 255.0F, 0xda / 255.0F,
                           255 / 255.0F};
constexpr Color INDIGO_300{0x79 / 255.0F, 0x86 / 255.0F, 0xcb / 255.0F,
                           255 / 255.0F};
constexpr Color INDIGO_400{0x5c / 255.0F, 0x6b / 255.0F, 0xc0 / 255.0F,
                           255 / 255.0F};
constexpr Color INDIGO_500{0x3f / 255.0F, 0x51 / 255.0F, 0xb5 / 255.0F,
                           255 / 255.0F};
constexpr Color INDIGO_600{0x39 / 255.0F, 0x49 / 255.0F, 0xab / 255.0F,
                           255 / 255.0F};
constexpr Color INDIGO_700{0x30 / 255.0F, 0x3f / 255.0F, 0x9f / 255.0F,
                           255 / 255.0F};
constexpr Color INDIGO_800{0x28 / 255.0F, 0x35 / 255.0F, 0x93 / 255.0F,
                           255 / 255.0F};
constexpr Color INDIGO_900{0x1a / 255.0F, 0x23 / 255.0F, 0x7e / 255.0F,
                           255 / 255.0F};
constexpr Color INDIGO_A100{0x8c / 255.0F, 0x9e / 255.0F, 0xff / 255.0F,
                            255 / 255.0F};
constexpr Color INDIGO_A200{0x53 / 255.0F, 0x6d / 255.0F, 0xfe / 255.0F,
                            255 / 255.0F};
constexpr Color INDIGO_A400{0x3d / 255.0F, 0x5a / 255.0F, 0xfe / 255.0F,
                            255 / 255.0F};
constexpr Color INDIGO_A700{0x30 / 255.0F, 0x4f / 255.0F, 0xfe / 255.0F,
                            255 / 255.0F};

constexpr Color BLUE_50{0xe3 / 255.0F, 0xf2 / 255.0F, 0xfd / 255.0F,
                        255 / 255.0F};
constexpr Color BLUE_100{0xbb / 255.0F, 0xde / 255.0F, 0xfb / 255.0F,
                         255 / 255.0F};
constexpr Color BLUE_200{0x90 / 255.0F, 0xca / 255.0F, 0xf9 / 255.0F,
                         255 / 255.0F};
constexpr Color BLUE_300{0x64 / 255.0F, 0xb5 / 255.0F, 0xf6 / 255.0F,
                         255 / 255.0F};
constexpr Color BLUE_400{0x42 / 255.0F, 0xa5 / 255.0F, 0xf5 / 255.0F,
                         255 / 255.0F};
constexpr Color BLUE_500{0x21 / 255.0F, 0x96 / 255.0F, 0xf3 / 255.0F,
                         255 / 255.0F};
constexpr Color BLUE_600{0x1e / 255.0F, 0x88 / 255.0F, 0xe5 / 255.0F,
                         255 / 255.0F};
constexpr Color BLUE_700{0x19 / 255.0F, 0x76 / 255.0F, 0xd2 / 255.0F,
                         255 / 255.0F};
constexpr Color BLUE_800{0x15 / 255.0F, 0x65 / 255.0F, 0xc0 / 255.0F,
                         255 / 255.0F};
constexpr Color BLUE_900{0x0d / 255.0F, 0x47 / 255.0F, 0xa1 / 255.0F,
                         255 / 255.0F};
constexpr Color BLUE_A100{0x82 / 255.0F, 0xb1 / 255.0F, 0xff / 255.0F,
                          255 / 255.0F};
constexpr Color BLUE_A200{0x44 / 255.0F, 0x8a / 255.0F, 0xff / 255.0F,
                          255 / 255.0F};
constexpr Color BLUE_A400{0x29 / 255.0F, 0x79 / 255.0F, 0xff / 255.0F,
                          255 / 255.0F};
constexpr Color BLUE_A700{0x29 / 255.0F, 0x62 / 255.0F, 0xff / 255.0F,
                          255 / 255.0F};

constexpr Color LIGHT_BLUE_50{0xe1 / 255.0F, 0xf5 / 255.0F, 0xfe / 255.0F,
                              255 / 255.0F};
constexpr Color LIGHT_BLUE_100{0xb3 / 255.0F, 0xe5 / 255.0F, 0xfc / 255.0F,
                               255 / 255.0F};
constexpr Color LIGHT_BLUE_200{0x81 / 255.0F, 0xd4 / 255.0F, 0xfa / 255.0F,
                               255 / 255.0F};
constexpr Color LIGHT_BLUE_300{0x4f / 255.0F, 0xc3 / 255.0F, 0xf7 / 255.0F,
                               255 / 255.0F};
constexpr Color LIGHT_BLUE_400{0x29 / 255.0F, 0xb6 / 255.0F, 0xf6 / 255.0F,
                               255 / 255.0F};
constexpr Color LIGHT_BLUE_500{0x03 / 255.0F, 0xa9 / 255.0F, 0xf4 / 255.0F,
                               255 / 255.0F};
constexpr Color LIGHT_BLUE_600{0x03 / 255.0F, 0x9b / 255.0F, 0xe5 / 255.0F,
                               255 / 255.0F};
constexpr Color LIGHT_BLUE_700{0x02 / 255.0F, 0x88 / 255.0F, 0xd1 / 255.0F,
                               255 / 255.0F};
constexpr Color LIGHT_BLUE_800{0x02 / 255.0F, 0x77 / 255.0F, 0xbd / 255.0F,
                               255 / 255.0F};
constexpr Color LIGHT_BLUE_900{0x01 / 255.0F, 0x57 / 255.0F, 0x9b / 255.0F,
                               255 / 255.0F};
constexpr Color LIGHT_BLUE_A100{0x80 / 255.0F, 0xd8 / 255.0F, 0xff / 255.0F,
                                255 / 255.0F};
constexpr Color LIGHT_BLUE_A200{0x40 / 255.0F, 0xc4 / 255.0F, 0xff / 255.0F,
                                255 / 255.0F};
constexpr Color LIGHT_BLUE_A400{0x00 / 255.0F, 0xb0 / 255.0F, 0xff / 255.0F,
                                255 / 255.0F};
constexpr Color LIGHT_BLUE_A700{0x00 / 255.0F, 0x91 / 255.0F, 0xea / 255.0F,
                                255 / 255.0F};

constexpr Color CYAN_50{0xe0 / 255.0F, 0xf7 / 255.0F, 0xfa / 255.0F,
                        255 / 255.0F};
constexpr Color CYAN_100{0xb2 / 255.0F, 0xeb / 255.0F, 0xf2 / 255.0F,
                         255 / 255.0F};
constexpr Color CYAN_200{0x80 / 255.0F, 0xde / 255.0F, 0xea / 255.0F,
                         255 / 255.0F};
constexpr Color CYAN_300{0x4d / 255.0F, 0xd0 / 255.0F, 0xe1 / 255.0F,
                         255 / 255.0F};
constexpr Color CYAN_400{0x26 / 255.0F, 0xc6 / 255.0F, 0xda / 255.0F,
                         255 / 255.0F};
constexpr Color CYAN_500{0x00 / 255.0F, 0xbc / 255.0F, 0xd4 / 255.0F,
                         255 / 255.0F};
constexpr Color CYAN_600{0x00 / 255.0F, 0xac / 255.0F, 0xc1 / 255.0F,
                         255 / 255.0F};
constexpr Color CYAN_700{0x00 / 255.0F, 0x97 / 255.0F, 0xa7 / 255.0F,
                         255 / 255.0F};
constexpr Color CYAN_800{0x00 / 255.0F, 0x83 / 255.0F, 0x8f / 255.0F,
                         255 / 255.0F};
constexpr Color CYAN_900{0x00 / 255.0F, 0x60 / 255.0F, 0x64 / 255.0F,
                         255 / 255.0F};
constexpr Color CYAN_A100{0x84 / 255.0F, 0xff / 255.0F, 0xff / 255.0F,
                          255 / 255.0F};
constexpr Color CYAN_A200{0x18 / 255.0F, 0xff / 255.0F, 0xff / 255.0F,
                          255 / 255.0F};
constexpr Color CYAN_A400{0x00 / 255.0F, 0xe5 / 255.0F, 0xff / 255.0F,
                          255 / 255.0F};
constexpr Color CYAN_A700{0x00 / 255.0F, 0xb8 / 255.0F, 0xd4 / 255.0F,
                          255 / 255.0F};

constexpr Color TEAL_50{0xe0 / 255.0F, 0xf2 / 255.0F, 0xf1 / 255.0F,
                        255 / 255.0F};
constexpr Color TEAL_100{0xb2 / 255.0F, 0xdf / 255.0F, 0xdb / 255.0F,
                         255 / 255.0F};
constexpr Color TEAL_200{0x80 / 255.0F, 0xcb / 255.0F, 0xc4 / 255.0F,
                         255 / 255.0F};
constexpr Color TEAL_300{0x4d / 255.0F, 0xb6 / 255.0F, 0xac / 255.0F,
                         255 / 255.0F};
constexpr Color TEAL_400{0x26 / 255.0F, 0xa6 / 255.0F, 0x9a / 255.0F,
                         255 / 255.0F};
constexpr Color TEAL_500{0x00 / 255.0F, 0x96 / 255.0F, 0x88 / 255.0F,
                         255 / 255.0F};
constexpr Color TEAL_600{0x00 / 255.0F, 0x89 / 255.0F, 0x7b / 255.0F,
                         255 / 255.0F};
constexpr Color TEAL_700{0x00 / 255.0F, 0x79 / 255.0F, 0x6b / 255.0F,
                         255 / 255.0F};
constexpr Color TEAL_800{0x00 / 255.0F, 0x69 / 255.0F, 0x5c / 255.0F,
                         255 / 255.0F};
constexpr Color TEAL_900{0x00 / 255.0F, 0x4d / 255.0F, 0x40 / 255.0F,
                         255 / 255.0F};
constexpr Color TEAL_A100{0xa7 / 255.0F, 0xff / 255.0F, 0xeb / 255.0F,
                          255 / 255.0F};
constexpr Color TEAL_A200{0x64 / 255.0F, 0xff / 255.0F, 0xda / 255.0F,
                          255 / 255.0F};
constexpr Color TEAL_A400{0x1d / 255.0F, 0xe9 / 255.0F, 0xb6 / 255.0F,
                          255 / 255.0F};
constexpr Color TEAL_A700{0x00 / 255.0F, 0xbf / 255.0F, 0xa5 / 255.0F,
                          255 / 255.0F};

constexpr Color GREEN_50{0xe8 / 255.0F, 0xf5 / 255.0F, 0xe9 / 255.0F,
                         255 / 255.0F};
constexpr Color GREEN_100{0xc8 / 255.0F, 0xe6 / 255.0F, 0xc9 / 255.0F,
                          255 / 255.0F};
constexpr Color GREEN_200{0xa5 / 255.0F, 0xd6 / 255.0F, 0xa7 / 255.0F,
                          255 / 255.0F};
constexpr Color GREEN_300{0x81 / 255.0F, 0xc7 / 255.0F, 0x84 / 255.0F,
                          255 / 255.0F};
constexpr Color GREEN_400{0x66 / 255.0F, 0xbb / 255.0F, 0x6a / 255.0F,
                          255 / 255.0F};
constexpr Color GREEN_500{0x4c / 255.0F, 0xaf / 255.0F, 0x50 / 255.0F,
                          255 / 255.0F};
constexpr Color GREEN_600{0x43 / 255.0F, 0xa0 / 255.0F, 0x47 / 255.0F,
                          255 / 255.0F};
constexpr Color GREEN_700{0x38 / 255.0F, 0x8e / 255.0F, 0x3c / 255.0F,
                          255 / 255.0F};
constexpr Color GREEN_800{0x2e / 255.0F, 0x7d / 255.0F, 0x32 / 255.0F,
                          255 / 255.0F};
constexpr Color GREEN_900{0x1b / 255.0F, 0x5e / 255.0F, 0x20 / 255.0F,
                          255 / 255.0F};
constexpr Color GREEN_A100{0xb9 / 255.0F, 0xf6 / 255.0F, 0xca / 255.0F,
                           255 / 255.0F};
constexpr Color GREEN_A200{0x69 / 255.0F, 0xf0 / 255.0F, 0xae / 255.0F,
                           255 / 255.0F};
constexpr Color GREEN_A400{0x00 / 255.0F, 0xe6 / 255.0F, 0x76 / 255.0F,
                           255 / 255.0F};
constexpr Color GREEN_A700{0x00 / 255.0F, 0xc8 / 255.0F, 0x53 / 255.0F,
                           255 / 255.0F};

constexpr Color LIGHT_GREEN_50{0xf1 / 255.0F, 0xf8 / 255.0F, 0xe9 / 255.0F,
                               255 / 255.0F};
constexpr Color LIGHT_GREEN_100{0xdc / 255.0F, 0xed / 255.0F, 0xc8 / 255.0F,
                                255 / 255.0F};
constexpr Color LIGHT_GREEN_200{0xc5 / 255.0F, 0xe1 / 255.0F, 0xa5 / 255.0F,
                                255 / 255.0F};
constexpr Color LIGHT_GREEN_300{0xae / 255.0F, 0xd5 / 255.0F, 0x81 / 255.0F,
                                255 / 255.0F};
constexpr Color LIGHT_GREEN_400{0x9c / 255.0F, 0xcc / 255.0F, 0x65 / 255.0F,
                                255 / 255.0F};
constexpr Color LIGHT_GREEN_500{0x8b / 255.0F, 0xc3 / 255.0F, 0x4a / 255.0F,
                                255 / 255.0F};
constexpr Color LIGHT_GREEN_600{0x7c / 255.0F, 0xb3 / 255.0F, 0x42 / 255.0F,
                                255 / 255.0F};
constexpr Color LIGHT_GREEN_700{0x68 / 255.0F, 0x9f / 255.0F, 0x38 / 255.0F,
                                255 / 255.0F};
constexpr Color LIGHT_GREEN_800{0x55 / 255.0F, 0x8b / 255.0F, 0x2f / 255.0F,
                                255 / 255.0F};
constexpr Color LIGHT_GREEN_900{0x33 / 255.0F, 0x69 / 255.0F, 0x1e / 255.0F,
                                255 / 255.0F};
constexpr Color LIGHT_GREEN_A100{0xcc / 255.0F, 0xff / 255.0F, 0x90 / 255.0F,
                                 255 / 255.0F};
constexpr Color LIGHT_GREEN_A200{0xb2 / 255.0F, 0xff / 255.0F, 0x59 / 255.0F,
                                 255 / 255.0F};
constexpr Color LIGHT_GREEN_A400{0x76 / 255.0F, 0xff / 255.0F, 0x03 / 255.0F,
                                 255 / 255.0F};
constexpr Color LIGHT_GREEN_A700{0x64 / 255.0F, 0xdd / 255.0F, 0x17 / 255.0F,
                                 255 / 255.0F};

constexpr Color LIME_50{0xf9 / 255.0F, 0xfb / 255.0F, 0xe7 / 255.0F,
                        255 / 255.0F};
constexpr Color LIME_100{0xf0 / 255.0F, 0xf4 / 255.0F, 0xc3 / 255.0F,
                         255 / 255.0F};
constexpr Color LIME_200{0xe6 / 255.0F, 0xee / 255.0F, 0x9c / 255.0F,
                         255 / 255.0F};
constexpr Color LIME_300{0xdc / 255.0F, 0xe7 / 255.0F, 0x75 / 255.0F,
                         255 / 255.0F};
constexpr Color LIME_400{0xd4 / 255.0F, 0xe1 / 255.0F, 0x57 / 255.0F,
                         255 / 255.0F};
constexpr Color LIME_500{0xcd / 255.0F, 0xdc / 255.0F, 0x39 / 255.0F,
                         255 / 255.0F};
constexpr Color LIME_600{0xc0 / 255.0F, 0xca / 255.0F, 0x33 / 255.0F,
                         255 / 255.0F};
constexpr Color LIME_700{0xaf / 255.0F, 0xb4 / 255.0F, 0x2b / 255.0F,
                         255 / 255.0F};
constexpr Color LIME_800{0x9e / 255.0F, 0x9d / 255.0F, 0x24 / 255.0F,
                         255 / 255.0F};
constexpr Color LIME_900{0x82 / 255.0F, 0x77 / 255.0F, 0x17 / 255.0F,
                         255 / 255.0F};
constexpr Color LIME_A100{0xf4 / 255.0F, 0xff / 255.0F, 0x81 / 255.0F,
                          255 / 255.0F};
constexpr Color LIME_A200{0xee / 255.0F, 0xff / 255.0F, 0x41 / 255.0F,
                          255 / 255.0F};
constexpr Color LIME_A400{0xc6 / 255.0F, 0xff / 255.0F, 0x00 / 255.0F,
                          255 / 255.0F};
constexpr Color LIME_A700{0xae / 255.0F, 0xea / 255.0F, 0x00 / 255.0F,
                          255 / 255.0F};

constexpr Color YELLOW_50{0xff / 255.0F, 0xfd / 255.0F, 0xe7 / 255.0F,
                          255 / 255.0F};
constexpr Color YELLOW_100{0xff / 255.0F, 0xf9 / 255.0F, 0xc4 / 255.0F,
                           255 / 255.0F};
constexpr Color YELLOW_200{0xff / 255.0F, 0xf5 / 255.0F, 0x9d / 255.0F,
                           255 / 255.0F};
constexpr Color YELLOW_300{0xff / 255.0F, 0xf1 / 255.0F, 0x76 / 255.0F,
                           255 / 255.0F};
constexpr Color YELLOW_400{0xff / 255.0F, 0xee / 255.0F, 0x58 / 255.0F,
                           255 / 255.0F};
constexpr Color YELLOW_500{0xff / 255.0F, 0xeb / 255.0F, 0x3b / 255.0F,
                           255 / 255.0F};
constexpr Color YELLOW_600{0xfd / 255.0F, 0xd8 / 255.0F, 0x35 / 255.0F,
                           255 / 255.0F};
constexpr Color YELLOW_700{0xfb / 255.0F, 0xc0 / 255.0F, 0x2d / 255.0F,
                           255 / 255.0F};
constexpr Color YELLOW_800{0xf9 / 255.0F, 0xa8 / 255.0F, 0x25 / 255.0F,
                           255 / 255.0F};
constexpr Color YELLOW_900{0xf5 / 255.0F, 0x7f / 255.0F, 0x17 / 255.0F,
                           255 / 255.0F};
constexpr Color YELLOW_A100{0xff / 255.0F, 0xff / 255.0F, 0x8d / 255.0F,
                            255 / 255.0F};
constexpr Color YELLOW_A200{0xff / 255.0F, 0xff / 255.0F, 0x00 / 255.0F,
                            255 / 255.0F};
constexpr Color YELLOW_A400{0xff / 255.0F, 0xea / 255.0F, 0x00 / 255.0F,
                            255 / 255.0F};
constexpr Color YELLOW_A700{0xff / 255.0F, 0xd6 / 255.0F, 0x00 / 255.0F,
                            255 / 255.0F};

constexpr Color AMBER_50{0xff / 255.0F, 0xf8 / 255.0F, 0xe1 / 255.0F,
                         255 / 255.0F};
constexpr Color AMBER_100{0xff / 255.0F, 0xec / 255.0F, 0xb3 / 255.0F,
                          255 / 255.0F};
constexpr Color AMBER_200{0xff / 255.0F, 0xe0 / 255.0F, 0x82 / 255.0F,
                          255 / 255.0F};
constexpr Color AMBER_300{0xff / 255.0F, 0xd5 / 255.0F, 0x4f / 255.0F,
                          255 / 255.0F};
constexpr Color AMBER_400{0xff / 255.0F, 0xca / 255.0F, 0x28 / 255.0F,
                          255 / 255.0F};
constexpr Color AMBER_500{0xff / 255.0F, 0xc1 / 255.0F, 0x07 / 255.0F,
                          255 / 255.0F};
constexpr Color AMBER_600{0xff / 255.0F, 0xb3 / 255.0F, 0x00 / 255.0F,
                          255 / 255.0F};
constexpr Color AMBER_700{0xff / 255.0F, 0xa0 / 255.0F, 0x00 / 255.0F,
                          255 / 255.0F};
constexpr Color AMBER_800{0xff / 255.0F, 0x8f / 255.0F, 0x00 / 255.0F,
                          255 / 255.0F};
constexpr Color AMBER_900{0xff / 255.0F, 0x6f / 255.0F, 0x00 / 255.0F,
                          255 / 255.0F};
constexpr Color AMBER_A100{0xff / 255.0F, 0xe5 / 255.0F, 0x7f / 255.0F,
                           255 / 255.0F};
constexpr Color AMBER_A200{0xff / 255.0F, 0xd7 / 255.0F, 0x40 / 255.0F,
                           255 / 255.0F};
constexpr Color AMBER_A400{0xff / 255.0F, 0xc4 / 255.0F, 0x00 / 255.0F,
                           255 / 255.0F};
constexpr Color AMBER_A700{0xff / 255.0F, 0xab / 255.0F, 0x00 / 255.0F,
                           255 / 255.0F};

constexpr Color ORANGE_50{0xff / 255.0F, 0xf3 / 255.0F, 0xe0 / 255.0F,
                          255 / 255.0F};
constexpr Color ORANGE_100{0xff / 255.0F, 0xe0 / 255.0F, 0xb2 / 255.0F,
                           255 / 255.0F};
constexpr Color ORANGE_200{0xff / 255.0F, 0xcc / 255.0F, 0x80 / 255.0F,
                           255 / 255.0F};
constexpr Color ORANGE_300{0xff / 255.0F, 0xb7 / 255.0F, 0x4d / 255.0F,
                           255 / 255.0F};
constexpr Color ORANGE_400{0xff / 255.0F, 0xa7 / 255.0F, 0x26 / 255.0F,
                           255 / 255.0F};
constexpr Color ORANGE_500{0xff / 255.0F, 0x98 / 255.0F, 0x00 / 255.0F,
                           255 / 255.0F};
constexpr Color ORANGE_600{0xfb / 255.0F, 0x8c / 255.0F, 0x00 / 255.0F,
                           255 / 255.0F};
constexpr Color ORANGE_700{0xf5 / 255.0F, 0x7c / 255.0F, 0x00 / 255.0F,
                           255 / 255.0F};
constexpr Color ORANGE_800{0xef / 255.0F, 0x6c / 255.0F, 0x00 / 255.0F,
                           255 / 255.0F};
constexpr Color ORANGE_900{0xe6 / 255.0F, 0x51 / 255.0F, 0x00 / 255.0F,
                           255 / 255.0F};
constexpr Color ORANGE_A100{0xff / 255.0F, 0xd1 / 255.0F, 0x80 / 255.0F,
                            255 / 255.0F};
constexpr Color ORANGE_A200{0xff / 255.0F, 0xab / 255.0F, 0x40 / 255.0F,
                            255 / 255.0F};
constexpr Color ORANGE_A400{0xff / 255.0F, 0x91 / 255.0F, 0x00 / 255.0F,
                            255 / 255.0F};
constexpr Color ORANGE_A700{0xff / 255.0F, 0x6d / 255.0F, 0x00 / 255.0F,
                            255 / 255.0F};

constexpr Color DEEP_ORANGE_50{0xfb / 255.0F, 0xe9 / 255.0F, 0xe7 / 255.0F,
                               255 / 255.0F};
constexpr Color DEEP_ORANGE_100{0xff / 255.0F, 0xcc / 255.0F, 0xbc / 255.0F,
                                255 / 255.0F};
constexpr Color DEEP_ORANGE_200{0xff / 255.0F, 0xab / 255.0F, 0x91 / 255.0F,
                                255 / 255.0F};
constexpr Color DEEP_ORANGE_300{0xff / 255.0F, 0x8a / 255.0F, 0x65 / 255.0F,
                                255 / 255.0F};
constexpr Color DEEP_ORANGE_400{0xff / 255.0F, 0x70 / 255.0F, 0x43 / 255.0F,
                                255 / 255.0F};
constexpr Color DEEP_ORANGE_500{0xff / 255.0F, 0x57 / 255.0F, 0x22 / 255.0F,
                                255 / 255.0F};
constexpr Color DEEP_ORANGE_600{0xf4 / 255.0F, 0x51 / 255.0F, 0x1e / 255.0F,
                                255 / 255.0F};
constexpr Color DEEP_ORANGE_700{0xe6 / 255.0F, 0x4a / 255.0F, 0x19 / 255.0F,
                                255 / 255.0F};
constexpr Color DEEP_ORANGE_800{0xd8 / 255.0F, 0x43 / 255.0F, 0x15 / 255.0F,
                                255 / 255.0F};
constexpr Color DEEP_ORANGE_900{0xbf / 255.0F, 0x36 / 255.0F, 0x0c / 255.0F,
                                255 / 255.0F};
constexpr Color DEEP_ORANGE_A100{0xff / 255.0F, 0x9e / 255.0F, 0x80 / 255.0F,
                                 255 / 255.0F};
constexpr Color DEEP_ORANGE_A200{0xff / 255.0F, 0x6e / 255.0F, 0x40 / 255.0F,
                                 255 / 255.0F};
constexpr Color DEEP_ORANGE_A400{0xff / 255.0F, 0x3d / 255.0F, 0x00 / 255.0F,
                                 255 / 255.0F};
constexpr Color DEEP_ORANGE_A700{0xdd / 255.0F, 0x2c / 255.0F, 0x00 / 255.0F,
                                 255 / 255.0F};

constexpr Color BROWN_50{0xef / 255.0F, 0xeb / 255.0F, 0xe9 / 255.0F,
                         255 / 255.0F};
constexpr Color BROWN_100{0xd7 / 255.0F, 0xcc / 255.0F, 0xc8 / 255.0F,
                          255 / 255.0F};
constexpr Color BROWN_200{0xbc / 255.0F, 0xaa / 255.0F, 0xa4 / 255.0F,
                          255 / 255.0F};
constexpr Color BROWN_300{0xa1 / 255.0F, 0x88 / 255.0F, 0x7f / 255.0F,
                          255 / 255.0F};
constexpr Color BROWN_400{0x8d / 255.0F, 0x6e / 255.0F, 0x63 / 255.0F,
                          255 / 255.0F};
constexpr Color BROWN_500{0x79 / 255.0F, 0x55 / 255.0F, 0x48 / 255.0F,
                          255 / 255.0F};
constexpr Color BROWN_600{0x6d / 255.0F, 0x4c / 255.0F, 0x41 / 255.0F,
                          255 / 255.0F};
constexpr Color BROWN_700{0x5d / 255.0F, 0x40 / 255.0F, 0x37 / 255.0F,
                          255 / 255.0F};
constexpr Color BROWN_800{0x4e / 255.0F, 0x34 / 255.0F, 0x2e / 255.0F,
                          255 / 255.0F};
constexpr Color BROWN_900{0x3e / 255.0F, 0x27 / 255.0F, 0x23 / 255.0F,
                          255 / 255.0F};

constexpr Color GRAY_50{0xfa / 255.0F, 0xfa / 255.0F, 0xfa / 255.0F,
                        255 / 255.0F};
constexpr Color GRAY_100{0xf5 / 255.0F, 0xf5 / 255.0F, 0xf5 / 255.0F,
                         255 / 255.0F};
constexpr Color GRAY_200{0xee / 255.0F, 0xee / 255.0F, 0xee / 255.0F,
                         255 / 255.0F};
constexpr Color GRAY_300{0xe0 / 255.0F, 0xe0 / 255.0F, 0xe0 / 255.0F,
                         255 / 255.0F};
constexpr Color GRAY_400{0xbd / 255.0F, 0xbd / 255.0F, 0xbd / 255.0F,
                         255 / 255.0F};
constexpr Color GRAY_500{0x9e / 255.0F, 0x9e / 255.0F, 0x9e / 255.0F,
                         255 / 255.0F};
constexpr Color GRAY_600{0x75 / 255.0F, 0x75 / 255.0F, 0x75 / 255.0F,
                         255 / 255.0F};
constexpr Color GRAY_700{0x61 / 255.0F, 0x61 / 255.0F, 0x61 / 255.0F,
                         255 / 255.0F};
constexpr Color GRAY_800{0x42 / 255.0F, 0x42 / 255.0F, 0x42 / 255.0F,
                         255 / 255.0F};
constexpr Color GRAY_900{0x21 / 255.0F, 0x21 / 255.0F, 0x21 / 255.0F,
                         255 / 255.0F};

constexpr Color BLUE_GRAY_50{0xec / 255.0F, 0xef / 255.0F, 0xf1 / 255.0F,
                             255 / 255.0F};
constexpr Color BLUE_GRAY_100{0xcf / 255.0F, 0xd8 / 255.0F, 0xdc / 255.0F,
                              255 / 255.0F};
constexpr Color BLUE_GRAY_200{0xb0 / 255.0F, 0xbe / 255.0F, 0xc5 / 255.0F,
                              255 / 255.0F};
constexpr Color BLUE_GRAY_300{0x90 / 255.0F, 0xa4 / 255.0F, 0xae / 255.0F,
                              255 / 255.0F};
constexpr Color BLUE_GRAY_400{0x78 / 255.0F, 0x90 / 255.0F, 0x9c / 255.0F,
                              255 / 255.0F};
constexpr Color BLUE_GRAY_500{0x60 / 255.0F, 0x7d / 255.0F, 0x8b / 255.0F,
                              255 / 255.0F};
constexpr Color BLUE_GRAY_600{0x54 / 255.0F, 0x6e / 255.0F, 0x7a / 255.0F,
                              255 / 255.0F};
constexpr Color BLUE_GRAY_700{0x45 / 255.0F, 0x5a / 255.0F, 0x64 / 255.0F,
                              255 / 255.0F};
constexpr Color BLUE_GRAY_800{0x37 / 255.0F, 0x47 / 255.0F, 0x4f / 255.0F,
                              255 / 255.0F};
constexpr Color BLUE_GRAY_900{0x26 / 255.0F, 0x32 / 255.0F, 0x38 / 255.0F,
                              255 / 255.0F};

constexpr Color WHITE{0xff / 255.0F, 0xff / 255.0F, 0xff / 255.0F,
                      255 / 255.0F};
constexpr Color BLACK{0x00 / 255.0F, 0x00 / 255.0F, 0x00 / 255.0F,
                      255 / 255.0F};

}        // namespace material

}        // namespace ash
