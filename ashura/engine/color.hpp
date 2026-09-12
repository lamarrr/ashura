/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/math.hpp"
#include "ashura/std/types.hpp"

namespace ash
{
namespace impl
{

constexpr bool parse_color_hex(char const * str, usize len, u8x4 & out)
{
    if (str == nullptr || (len != 7 && len != 9) || str[0] != '#')
    {
        return false;
    }

    auto hex_digit = [](char c, u8 & value) constexpr -> bool {
        if (c >= '0' && c <= '9')
        {
            value = static_cast<u8>(c - '0');
            return true;
        }

        if (c >= 'A' && c <= 'F')
        {
            value = static_cast<u8>(c - 'A' + 10);
            return true;
        }

        if (c >= 'a' && c <= 'f')
        {
            value = static_cast<u8>(c - 'a' + 10);
            return true;
        }

        return false;
    };

    u8x4 parsed = {0, 0, 0, 0xFF};

    for (auto channel = 0uz; channel < (len - 1) / 2; channel++)
    {
        u8 high;
        u8 low;

        if (!hex_digit(str[1 + channel * 2], high) ||
            !hex_digit(str[2 + channel * 2], low))
        {
            return false;
        }

        parsed[channel] = static_cast<u8>((high << 4) | low);
    }

    out = parsed;
    return true;
}
}    // namespace impl

consteval u8x4 operator""_cc(char const * str, usize len)
{
    u8x4 color;
    if (!impl::parse_color_hex(str, len, color))
    {
        return u8x4{0, 0, 0, 0};
    }
    return color;
}

namespace colors
{

inline constexpr u8x4 NONE    = "#00000000"_cc;
inline constexpr u8x4 WHITE   = "#FFFFFFFF"_cc;
inline constexpr u8x4 BLACK   = "#000000FF"_cc;
inline constexpr u8x4 RED     = "#FF0000FF"_cc;
inline constexpr u8x4 BLUE    = "#0000FFFF"_cc;
inline constexpr u8x4 GREEN   = "#00FF00FF"_cc;
inline constexpr u8x4 CYAN    = "#00FFFFFF"_cc;
inline constexpr u8x4 MAGENTA = "#FF00FFFF"_cc;
inline constexpr u8x4 YELLOW  = "#FFFF00FF"_cc;

inline constexpr u8x4 RED_50   = "#FFEBEEFF"_cc;
inline constexpr u8x4 RED_100  = "#FFCDD2FF"_cc;
inline constexpr u8x4 RED_200  = "#EF9A9AFF"_cc;
inline constexpr u8x4 RED_300  = "#E57373FF"_cc;
inline constexpr u8x4 RED_400  = "#EF5350FF"_cc;
inline constexpr u8x4 RED_500  = "#F44336FF"_cc;
inline constexpr u8x4 RED_600  = "#E53935FF"_cc;
inline constexpr u8x4 RED_700  = "#D32F2FFF"_cc;
inline constexpr u8x4 RED_800  = "#C62828FF"_cc;
inline constexpr u8x4 RED_900  = "#B71C1CFF"_cc;
inline constexpr u8x4 RED_A100 = "#FF8A80FF"_cc;
inline constexpr u8x4 RED_A200 = "#FF5252FF"_cc;
inline constexpr u8x4 RED_A400 = "#FF1744FF"_cc;
inline constexpr u8x4 RED_A700 = "#D50000FF"_cc;

inline constexpr u8x4 PINK_50   = "#FCE4ECFF"_cc;
inline constexpr u8x4 PINK_100  = "#F8BBD0FF"_cc;
inline constexpr u8x4 PINK_200  = "#F48FB1FF"_cc;
inline constexpr u8x4 PINK_300  = "#F06292FF"_cc;
inline constexpr u8x4 PINK_400  = "#EC407AFF"_cc;
inline constexpr u8x4 PINK_500  = "#E91E63FF"_cc;
inline constexpr u8x4 PINK_600  = "#D81B60FF"_cc;
inline constexpr u8x4 PINK_700  = "#C2185BFF"_cc;
inline constexpr u8x4 PINK_800  = "#AD1457FF"_cc;
inline constexpr u8x4 PINK_900  = "#880E4FFF"_cc;
inline constexpr u8x4 PINK_A100 = "#FF80ABFF"_cc;
inline constexpr u8x4 PINK_A200 = "#FF4081FF"_cc;
inline constexpr u8x4 PINK_A400 = "#F50057FF"_cc;
inline constexpr u8x4 PINK_A700 = "#C51162FF"_cc;

inline constexpr u8x4 PURPLE_50   = "#F3E5F5FF"_cc;
inline constexpr u8x4 PURPLE_100  = "#E1BEE7FF"_cc;
inline constexpr u8x4 PURPLE_200  = "#CE93D8FF"_cc;
inline constexpr u8x4 PURPLE_300  = "#BA68C8FF"_cc;
inline constexpr u8x4 PURPLE_400  = "#AB47BCFF"_cc;
inline constexpr u8x4 PURPLE_500  = "#9C27B0FF"_cc;
inline constexpr u8x4 PURPLE_600  = "#8E24AAFF"_cc;
inline constexpr u8x4 PURPLE_700  = "#7B1FA2FF"_cc;
inline constexpr u8x4 PURPLE_800  = "#6A1B9AFF"_cc;
inline constexpr u8x4 PURPLE_900  = "#4A148CFF"_cc;
inline constexpr u8x4 PURPLE_A100 = "#EA80FCFF"_cc;
inline constexpr u8x4 PURPLE_A200 = "#E040FBFF"_cc;
inline constexpr u8x4 PURPLE_A400 = "#D500F9FF"_cc;
inline constexpr u8x4 PURPLE_A700 = "#AA00FFFF"_cc;

inline constexpr u8x4 DEEP_PURPLE_50   = "#EDE7F6FF"_cc;
inline constexpr u8x4 DEEP_PURPLE_100  = "#D1C4E9FF"_cc;
inline constexpr u8x4 DEEP_PURPLE_200  = "#B39DDBFF"_cc;
inline constexpr u8x4 DEEP_PURPLE_300  = "#9575CDFF"_cc;
inline constexpr u8x4 DEEP_PURPLE_400  = "#7E57C2FF"_cc;
inline constexpr u8x4 DEEP_PURPLE_500  = "#673AB7FF"_cc;
inline constexpr u8x4 DEEP_PURPLE_600  = "#5E35B1FF"_cc;
inline constexpr u8x4 DEEP_PURPLE_700  = "#512DA8FF"_cc;
inline constexpr u8x4 DEEP_PURPLE_800  = "#4527A0FF"_cc;
inline constexpr u8x4 DEEP_PURPLE_900  = "#311B92FF"_cc;
inline constexpr u8x4 DEEP_PURPLE_A100 = "#B388FFFF"_cc;
inline constexpr u8x4 DEEP_PURPLE_A200 = "#7C4DFFFF"_cc;
inline constexpr u8x4 DEEP_PURPLE_A400 = "#651FFFFF"_cc;
inline constexpr u8x4 DEEP_PURPLE_A700 = "#6200EAFF"_cc;

inline constexpr u8x4 INDIGO_50   = "#E8EAF6FF"_cc;
inline constexpr u8x4 INDIGO_100  = "#C5CAE9FF"_cc;
inline constexpr u8x4 INDIGO_200  = "#9FA8DAFF"_cc;
inline constexpr u8x4 INDIGO_300  = "#7986CBFF"_cc;
inline constexpr u8x4 INDIGO_400  = "#5C6BC0FF"_cc;
inline constexpr u8x4 INDIGO_500  = "#3F51B5FF"_cc;
inline constexpr u8x4 INDIGO_600  = "#3949ABFF"_cc;
inline constexpr u8x4 INDIGO_700  = "#303F9FFF"_cc;
inline constexpr u8x4 INDIGO_800  = "#283593FF"_cc;
inline constexpr u8x4 INDIGO_900  = "#1A237EFF"_cc;
inline constexpr u8x4 INDIGO_A100 = "#8C9EFFFF"_cc;
inline constexpr u8x4 INDIGO_A200 = "#536DFEFF"_cc;
inline constexpr u8x4 INDIGO_A400 = "#3D5AFEFF"_cc;
inline constexpr u8x4 INDIGO_A700 = "#304FFEFF"_cc;

inline constexpr u8x4 BLUE_50   = "#E3F2FDFF"_cc;
inline constexpr u8x4 BLUE_100  = "#BBDEFBFF"_cc;
inline constexpr u8x4 BLUE_200  = "#90CAF9FF"_cc;
inline constexpr u8x4 BLUE_300  = "#64B5F6FF"_cc;
inline constexpr u8x4 BLUE_400  = "#42A5F5FF"_cc;
inline constexpr u8x4 BLUE_500  = "#2196F3FF"_cc;
inline constexpr u8x4 BLUE_600  = "#1E88E5FF"_cc;
inline constexpr u8x4 BLUE_700  = "#1976D2FF"_cc;
inline constexpr u8x4 BLUE_800  = "#1565C0FF"_cc;
inline constexpr u8x4 BLUE_900  = "#0D47A1FF"_cc;
inline constexpr u8x4 BLUE_A100 = "#82B1FFFF"_cc;
inline constexpr u8x4 BLUE_A200 = "#448AFFFF"_cc;
inline constexpr u8x4 BLUE_A400 = "#2979FFFF"_cc;
inline constexpr u8x4 BLUE_A700 = "#2962FFFF"_cc;

inline constexpr u8x4 LIGHT_BLUE_50   = "#E1F5FEFF"_cc;
inline constexpr u8x4 LIGHT_BLUE_100  = "#B3E5FCFF"_cc;
inline constexpr u8x4 LIGHT_BLUE_200  = "#81D4FAFF"_cc;
inline constexpr u8x4 LIGHT_BLUE_300  = "#4FC3F7FF"_cc;
inline constexpr u8x4 LIGHT_BLUE_400  = "#29B6F6FF"_cc;
inline constexpr u8x4 LIGHT_BLUE_500  = "#03A9F4FF"_cc;
inline constexpr u8x4 LIGHT_BLUE_600  = "#039BE5FF"_cc;
inline constexpr u8x4 LIGHT_BLUE_700  = "#0288D1FF"_cc;
inline constexpr u8x4 LIGHT_BLUE_800  = "#0277BDFF"_cc;
inline constexpr u8x4 LIGHT_BLUE_900  = "#01579BFF"_cc;
inline constexpr u8x4 LIGHT_BLUE_A100 = "#80D8FFFF"_cc;
inline constexpr u8x4 LIGHT_BLUE_A200 = "#40C4FFFF"_cc;
inline constexpr u8x4 LIGHT_BLUE_A400 = "#00B0FFFF"_cc;
inline constexpr u8x4 LIGHT_BLUE_A700 = "#0091EAFF"_cc;

inline constexpr u8x4 CYAN_50   = "#E0F7FAFF"_cc;
inline constexpr u8x4 CYAN_100  = "#B2EBF2FF"_cc;
inline constexpr u8x4 CYAN_200  = "#80DEEAFF"_cc;
inline constexpr u8x4 CYAN_300  = "#4DD0E1FF"_cc;
inline constexpr u8x4 CYAN_400  = "#26C6DAFF"_cc;
inline constexpr u8x4 CYAN_500  = "#00BCD4FF"_cc;
inline constexpr u8x4 CYAN_600  = "#00ACC1FF"_cc;
inline constexpr u8x4 CYAN_700  = "#0097A7FF"_cc;
inline constexpr u8x4 CYAN_800  = "#00838FFF"_cc;
inline constexpr u8x4 CYAN_900  = "#006064FF"_cc;
inline constexpr u8x4 CYAN_A100 = "#84FFFFFF"_cc;
inline constexpr u8x4 CYAN_A200 = "#18FFFFFF"_cc;
inline constexpr u8x4 CYAN_A400 = "#00E5FFFF"_cc;
inline constexpr u8x4 CYAN_A700 = "#00B8D4FF"_cc;

inline constexpr u8x4 TEAL_50   = "#E0F2F1FF"_cc;
inline constexpr u8x4 TEAL_100  = "#B2DFDBFF"_cc;
inline constexpr u8x4 TEAL_200  = "#80CBC4FF"_cc;
inline constexpr u8x4 TEAL_300  = "#4DB6ACFF"_cc;
inline constexpr u8x4 TEAL_400  = "#26A69AFF"_cc;
inline constexpr u8x4 TEAL_500  = "#009688FF"_cc;
inline constexpr u8x4 TEAL_600  = "#00897BFF"_cc;
inline constexpr u8x4 TEAL_700  = "#00796BFF"_cc;
inline constexpr u8x4 TEAL_800  = "#00695CFF"_cc;
inline constexpr u8x4 TEAL_900  = "#004D40FF"_cc;
inline constexpr u8x4 TEAL_A100 = "#A7FFEBFF"_cc;
inline constexpr u8x4 TEAL_A200 = "#64FFDAFF"_cc;
inline constexpr u8x4 TEAL_A400 = "#1DE9B6FF"_cc;
inline constexpr u8x4 TEAL_A700 = "#00BFA5FF"_cc;

inline constexpr u8x4 GREEN_50   = "#E8F5E9FF"_cc;
inline constexpr u8x4 GREEN_100  = "#C8E6C9FF"_cc;
inline constexpr u8x4 GREEN_200  = "#A5D6A7FF"_cc;
inline constexpr u8x4 GREEN_300  = "#81C784FF"_cc;
inline constexpr u8x4 GREEN_400  = "#66BB6AFF"_cc;
inline constexpr u8x4 GREEN_500  = "#4CAF50FF"_cc;
inline constexpr u8x4 GREEN_600  = "#43A047FF"_cc;
inline constexpr u8x4 GREEN_700  = "#388E3CFF"_cc;
inline constexpr u8x4 GREEN_800  = "#2E7D32FF"_cc;
inline constexpr u8x4 GREEN_900  = "#1B5E20FF"_cc;
inline constexpr u8x4 GREEN_A100 = "#B9F6CAFF"_cc;
inline constexpr u8x4 GREEN_A200 = "#69F0AEFF"_cc;
inline constexpr u8x4 GREEN_A400 = "#00E676FF"_cc;
inline constexpr u8x4 GREEN_A700 = "#00C853FF"_cc;

inline constexpr u8x4 LIGHT_GREEN_50   = "#F1F8E9FF"_cc;
inline constexpr u8x4 LIGHT_GREEN_100  = "#DCEDC8FF"_cc;
inline constexpr u8x4 LIGHT_GREEN_200  = "#C5E1A5FF"_cc;
inline constexpr u8x4 LIGHT_GREEN_300  = "#AED581FF"_cc;
inline constexpr u8x4 LIGHT_GREEN_400  = "#9CCC65FF"_cc;
inline constexpr u8x4 LIGHT_GREEN_500  = "#8BC34AFF"_cc;
inline constexpr u8x4 LIGHT_GREEN_600  = "#7CB342FF"_cc;
inline constexpr u8x4 LIGHT_GREEN_700  = "#689F38FF"_cc;
inline constexpr u8x4 LIGHT_GREEN_800  = "#558B2FFF"_cc;
inline constexpr u8x4 LIGHT_GREEN_900  = "#33691EFF"_cc;
inline constexpr u8x4 LIGHT_GREEN_A100 = "#CCFF90FF"_cc;
inline constexpr u8x4 LIGHT_GREEN_A200 = "#B2FF59FF"_cc;
inline constexpr u8x4 LIGHT_GREEN_A400 = "#76FF03FF"_cc;
inline constexpr u8x4 LIGHT_GREEN_A700 = "#64DD17FF"_cc;

inline constexpr u8x4 LIME_50   = "#F9FBE7FF"_cc;
inline constexpr u8x4 LIME_100  = "#F0F4C3FF"_cc;
inline constexpr u8x4 LIME_200  = "#E6EE9CFF"_cc;
inline constexpr u8x4 LIME_300  = "#DCE775FF"_cc;
inline constexpr u8x4 LIME_400  = "#D4E157FF"_cc;
inline constexpr u8x4 LIME_500  = "#CDDC39FF"_cc;
inline constexpr u8x4 LIME_600  = "#C0CA33FF"_cc;
inline constexpr u8x4 LIME_700  = "#AFB42BFF"_cc;
inline constexpr u8x4 LIME_800  = "#9E9D24FF"_cc;
inline constexpr u8x4 LIME_900  = "#827717FF"_cc;
inline constexpr u8x4 LIME_A100 = "#F4FF81FF"_cc;
inline constexpr u8x4 LIME_A200 = "#EEFF41FF"_cc;
inline constexpr u8x4 LIME_A400 = "#C6FF00FF"_cc;
inline constexpr u8x4 LIME_A700 = "#AEEA00FF"_cc;

inline constexpr u8x4 YELLOW_50   = "#FFFDE7FF"_cc;
inline constexpr u8x4 YELLOW_100  = "#FFF9C4FF"_cc;
inline constexpr u8x4 YELLOW_200  = "#FFF59DFF"_cc;
inline constexpr u8x4 YELLOW_300  = "#FFF176FF"_cc;
inline constexpr u8x4 YELLOW_400  = "#FFEE58FF"_cc;
inline constexpr u8x4 YELLOW_500  = "#FFEB3BFF"_cc;
inline constexpr u8x4 YELLOW_600  = "#FDD835FF"_cc;
inline constexpr u8x4 YELLOW_700  = "#FBC02DFF"_cc;
inline constexpr u8x4 YELLOW_800  = "#F9A825FF"_cc;
inline constexpr u8x4 YELLOW_900  = "#F57F17FF"_cc;
inline constexpr u8x4 YELLOW_A100 = "#FFFF8DFF"_cc;
inline constexpr u8x4 YELLOW_A200 = "#FFFF00FF"_cc;
inline constexpr u8x4 YELLOW_A400 = "#FFEA00FF"_cc;
inline constexpr u8x4 YELLOW_A700 = "#FFD600FF"_cc;

inline constexpr u8x4 AMBER_50   = "#FFF8E1FF"_cc;
inline constexpr u8x4 AMBER_100  = "#FFECB3FF"_cc;
inline constexpr u8x4 AMBER_200  = "#FFE082FF"_cc;
inline constexpr u8x4 AMBER_300  = "#FFD54FFF"_cc;
inline constexpr u8x4 AMBER_400  = "#FFCA28FF"_cc;
inline constexpr u8x4 AMBER_500  = "#FFC107FF"_cc;
inline constexpr u8x4 AMBER_600  = "#FFB300FF"_cc;
inline constexpr u8x4 AMBER_700  = "#FFA000FF"_cc;
inline constexpr u8x4 AMBER_800  = "#FF8F00FF"_cc;
inline constexpr u8x4 AMBER_900  = "#FF6F00FF"_cc;
inline constexpr u8x4 AMBER_A100 = "#FFE57FFF"_cc;
inline constexpr u8x4 AMBER_A200 = "#FFD740FF"_cc;
inline constexpr u8x4 AMBER_A400 = "#FFC400FF"_cc;
inline constexpr u8x4 AMBER_A700 = "#FFAB00FF"_cc;

inline constexpr u8x4 ORANGE_50   = "#FFF3E0FF"_cc;
inline constexpr u8x4 ORANGE_100  = "#FFE0B2FF"_cc;
inline constexpr u8x4 ORANGE_200  = "#FFCC80FF"_cc;
inline constexpr u8x4 ORANGE_300  = "#FFB74DFF"_cc;
inline constexpr u8x4 ORANGE_400  = "#FFA726FF"_cc;
inline constexpr u8x4 ORANGE_500  = "#FF9800FF"_cc;
inline constexpr u8x4 ORANGE_600  = "#FB8C00FF"_cc;
inline constexpr u8x4 ORANGE_700  = "#F57C00FF"_cc;
inline constexpr u8x4 ORANGE_800  = "#EF6C00FF"_cc;
inline constexpr u8x4 ORANGE_900  = "#E65100FF"_cc;
inline constexpr u8x4 ORANGE_A100 = "#FFD180FF"_cc;
inline constexpr u8x4 ORANGE_A200 = "#FFAB40FF"_cc;
inline constexpr u8x4 ORANGE_A400 = "#FF9100FF"_cc;
inline constexpr u8x4 ORANGE_A700 = "#FF6D00FF"_cc;

inline constexpr u8x4 DEEP_ORANGE_50   = "#FBE9E7FF"_cc;
inline constexpr u8x4 DEEP_ORANGE_100  = "#FFCCBCFF"_cc;
inline constexpr u8x4 DEEP_ORANGE_200  = "#FFAB91FF"_cc;
inline constexpr u8x4 DEEP_ORANGE_300  = "#FF8A65FF"_cc;
inline constexpr u8x4 DEEP_ORANGE_400  = "#FF7043FF"_cc;
inline constexpr u8x4 DEEP_ORANGE_500  = "#FF5722FF"_cc;
inline constexpr u8x4 DEEP_ORANGE_600  = "#F4511EFF"_cc;
inline constexpr u8x4 DEEP_ORANGE_700  = "#E64A19FF"_cc;
inline constexpr u8x4 DEEP_ORANGE_800  = "#D84315FF"_cc;
inline constexpr u8x4 DEEP_ORANGE_900  = "#BF360CFF"_cc;
inline constexpr u8x4 DEEP_ORANGE_A100 = "#FF9E80FF"_cc;
inline constexpr u8x4 DEEP_ORANGE_A200 = "#FF6E40FF"_cc;
inline constexpr u8x4 DEEP_ORANGE_A400 = "#FF3D00FF"_cc;
inline constexpr u8x4 DEEP_ORANGE_A700 = "#DD2C00FF"_cc;

inline constexpr u8x4 BROWN_50  = "#EFEBE9FF"_cc;
inline constexpr u8x4 BROWN_100 = "#D7CCC8FF"_cc;
inline constexpr u8x4 BROWN_200 = "#BCAAA4FF"_cc;
inline constexpr u8x4 BROWN_300 = "#A1887FFF"_cc;
inline constexpr u8x4 BROWN_400 = "#8D6E63FF"_cc;
inline constexpr u8x4 BROWN_500 = "#795548FF"_cc;
inline constexpr u8x4 BROWN_600 = "#6D4C41FF"_cc;
inline constexpr u8x4 BROWN_700 = "#5D4037FF"_cc;
inline constexpr u8x4 BROWN_800 = "#4E342EFF"_cc;
inline constexpr u8x4 BROWN_900 = "#3E2723FF"_cc;

inline constexpr u8x4 GRAY_50  = "#FAFAFAFF"_cc;
inline constexpr u8x4 GRAY_100 = "#F5F5F5FF"_cc;
inline constexpr u8x4 GRAY_200 = "#EEEEEEFF"_cc;
inline constexpr u8x4 GRAY_300 = "#E0E0E0FF"_cc;
inline constexpr u8x4 GRAY_400 = "#BDBDBDFF"_cc;
inline constexpr u8x4 GRAY_500 = "#9E9E9EFF"_cc;
inline constexpr u8x4 GRAY_600 = "#757575FF"_cc;
inline constexpr u8x4 GRAY_700 = "#616161FF"_cc;
inline constexpr u8x4 GRAY_800 = "#424242FF"_cc;
inline constexpr u8x4 GRAY_900 = "#212121FF"_cc;

inline constexpr u8x4 BLUE_GRAY_50  = "#ECEFF1FF"_cc;
inline constexpr u8x4 BLUE_GRAY_100 = "#CFD8DCFF"_cc;
inline constexpr u8x4 BLUE_GRAY_200 = "#B0BEC5FF"_cc;
inline constexpr u8x4 BLUE_GRAY_300 = "#90A4AEFF"_cc;
inline constexpr u8x4 BLUE_GRAY_400 = "#78909CFF"_cc;
inline constexpr u8x4 BLUE_GRAY_500 = "#607D8BFF"_cc;
inline constexpr u8x4 BLUE_GRAY_600 = "#546E7AFF"_cc;
inline constexpr u8x4 BLUE_GRAY_700 = "#455A64FF"_cc;
inline constexpr u8x4 BLUE_GRAY_800 = "#37474FFF"_cc;
inline constexpr u8x4 BLUE_GRAY_900 = "#263238FF"_cc;

}    // namespace colors

struct ColorGradient
{
    f32x4 top_    = {};
    f32x4 bottom_ = {};
    f32   angle_  = 0;
    f32   center_ = 0;

    constexpr ColorGradient() = default;

    constexpr ColorGradient(f32x4 c) : top_{c}, bottom_{c}, angle_{0}, center_{0}
    {
    }

    constexpr ColorGradient(u8x4 c) : ColorGradient{norm(c)}
    {
    }

    constexpr ColorGradient(f32x4 top, f32x4 bottom, f32 angle, f32 center) :
      top_{top},
      bottom_{bottom},
      angle_{angle},
      center_{center}
    {
    }

    constexpr ColorGradient(u8x4 top, u8x4 bottom, f32 angle, f32 center) :
      ColorGradient{norm(top), norm(bottom), angle, center}
    {
    }

    constexpr ColorGradient & to_horizontal()
    {
        angle_ = 0;
        return *this;
    }

    constexpr ColorGradient & to_vertical()
    {
        angle_ = 0.5F * PI;
        return *this;
    }

    constexpr bool is_transparent() const
    {
        return top_.a() == 0 && bottom_.a() == 0;
    }

    constexpr f32x4 top() const
    {
        return top_;
    }

    constexpr f32x4 bottom() const
    {
        return bottom_;
    }

    constexpr f32 angle() const
    {
        return angle_;
    }

    constexpr f32 center() const
    {
        return center_;
    }
};

}    // namespace ash
