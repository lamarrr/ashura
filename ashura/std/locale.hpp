/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/span.hpp"

namespace ash
{

/// @brief IETF BCP 47 language tags. See:
/// https://en.wikipedia.org/wiki/IETF_language_tag
namespace lang
{
inline constexpr Str AFRIKAANS         = "af"_s;
inline constexpr Str AMHARIC           = "am"_s;
inline constexpr Str ARABIC            = "ar"_s;
inline constexpr Str MAPUDUNGUN        = "arn"_s;
inline constexpr Str ASSAMESE          = "as"_s;
inline constexpr Str AZERBAIJANI       = "az"_s;
inline constexpr Str BASHKIR           = "ba"_s;
inline constexpr Str BELARUSIAN        = "be"_s;
inline constexpr Str BULGARIAN         = "bg"_s;
inline constexpr Str BENGALI           = "bn"_s;
inline constexpr Str TIBETAN           = "bo"_s;
inline constexpr Str BRETON            = "br"_s;
inline constexpr Str BOSNIAN           = "bs"_s;
inline constexpr Str CATALAN           = "ca"_s;
inline constexpr Str CORSICAN          = "co"_s;
inline constexpr Str CZECH             = "cs"_s;
inline constexpr Str WELSH             = "cy"_s;
inline constexpr Str DANISH            = "da"_s;
inline constexpr Str GERMAN            = "de"_s;
inline constexpr Str LOWER_SORBIAN     = "dsb"_s;
inline constexpr Str DIVEHI            = "dv"_s;
inline constexpr Str GREEK             = "el"_s;
inline constexpr Str ENGLISH           = "en"_s;
inline constexpr Str SPANISH           = "es"_s;
inline constexpr Str ESTONIAN          = "et"_s;
inline constexpr Str BASQUE            = "eu"_s;
inline constexpr Str PERSIAN           = "fa"_s;
inline constexpr Str FINNISH           = "fi"_s;
inline constexpr Str FILIPINO          = "fil"_s;
inline constexpr Str FAROESE           = "fo"_s;
inline constexpr Str FRENCH            = "fr"_s;
inline constexpr Str FRISIAN           = "fy"_s;
inline constexpr Str IRISH             = "ga"_s;
inline constexpr Str SCOTTISH_GAELIC   = "gd"_s;
inline constexpr Str GALICIAN          = "gl"_s;
inline constexpr Str ALSATIAN          = "gsw"_s;
inline constexpr Str GUJARATI          = "gu"_s;
inline constexpr Str HAUSA             = "ha"_s;
inline constexpr Str HEBREW            = "he"_s;
inline constexpr Str HINDI             = "hi"_s;
inline constexpr Str CROATIAN          = "hr"_s;
inline constexpr Str UPPER_SORBIAN     = "hsb"_s;
inline constexpr Str HUNGARIAN         = "hu"_s;
inline constexpr Str ARMENIAN          = "hy"_s;
inline constexpr Str INDONESIAN        = "id"_s;
inline constexpr Str IGBO              = "ig"_s;
inline constexpr Str YI                = "ii"_s;
inline constexpr Str ICELANDIC         = "is"_s;
inline constexpr Str ITALIAN           = "it"_s;
inline constexpr Str INUKTITUT         = "iu"_s;
inline constexpr Str JAPANESE          = "ja"_s;
inline constexpr Str GEORGIAN          = "ka"_s;
inline constexpr Str KAZAKH            = "kk"_s;
inline constexpr Str GREENLANDIC       = "kl"_s;
inline constexpr Str KHMER             = "km"_s;
inline constexpr Str KANNADA           = "kn"_s;
inline constexpr Str KOREAN            = "ko"_s;
inline constexpr Str KONKANI           = "kok"_s;
inline constexpr Str KYRGYZ            = "ky"_s;
inline constexpr Str LUXEMBOURGISH     = "lb"_s;
inline constexpr Str LAO               = "lo"_s;
inline constexpr Str LITHUANIAN        = "lt"_s;
inline constexpr Str LATVIAN           = "lv"_s;
inline constexpr Str MAORI_REO         = "mi"_s;
inline constexpr Str MACEDONIAN        = "mk"_s;
inline constexpr Str MALAYALAM         = "ml"_s;
inline constexpr Str MONGOLIAN         = "mn"_s;
inline constexpr Str MOHAWK            = "moh"_s;
inline constexpr Str MARATHI           = "mr"_s;
inline constexpr Str MALAY_BAHASA      = "ms"_s;
inline constexpr Str MALTESE           = "mt"_s;
inline constexpr Str BURMESE           = "my"_s;
inline constexpr Str NORWEGIAN_BOKMAL  = "nb"_s;
inline constexpr Str NEPALI            = "ne"_s;
inline constexpr Str DUTCH             = "nl"_s;
inline constexpr Str NORWEGIAN_NYNORSK = "nn"_s;
inline constexpr Str NORWEGIAN         = "no"_s;
inline constexpr Str SESOTHO           = "st"_s;
inline constexpr Str OCCITAN           = "oc"_s;
inline constexpr Str ODIA              = "or"_s;
inline constexpr Str PUNJABI           = "pa"_s;
inline constexpr Str POLISH            = "pl"_s;
inline constexpr Str DARI              = "prs"_s;
inline constexpr Str PASHTO            = "ps"_s;
inline constexpr Str PORTUGUESE        = "pt"_s;
inline constexpr Str KICHE             = "quc"_s;
inline constexpr Str QUECHUA           = "qu"_s;
inline constexpr Str ROMANSH           = "rm"_s;
inline constexpr Str ROMANIAN          = "ro"_s;
inline constexpr Str RUSSIAN           = "ru"_s;
inline constexpr Str KINYARWANDA       = "rw"_s;
inline constexpr Str SANSKRIT          = "sa"_s;
inline constexpr Str YAKUT             = "sah"_s;
inline constexpr Str SAMI_NORTHERN     = "se"_s;
inline constexpr Str SINHALA           = "si"_s;
inline constexpr Str SLOVAK            = "sk"_s;
inline constexpr Str SLOVENIAN         = "sl"_s;
inline constexpr Str SAMI_SOUTHERN     = "sma"_s;
inline constexpr Str SAMI_LULE         = "smj"_s;
inline constexpr Str SAMI_INARI        = "smn"_s;
inline constexpr Str SAMI_SKOLT        = "sms"_s;
inline constexpr Str ALBANIAN          = "sq"_s;
inline constexpr Str SERBIAN           = "sr"_s;
inline constexpr Str SWEDISH           = "sv"_s;
inline constexpr Str KISWAHILI         = "sw"_s;
inline constexpr Str SYRIAC            = "syc"_s;
inline constexpr Str TAMIL             = "ta"_s;
inline constexpr Str TELUGU            = "te"_s;
inline constexpr Str TAJIK             = "tg"_s;
inline constexpr Str THAI              = "th"_s;
inline constexpr Str TURKMEN           = "tk"_s;
inline constexpr Str TSWANA            = "tn"_s;
inline constexpr Str TURKISH           = "tr"_s;
inline constexpr Str TATAR             = "tt"_s;
inline constexpr Str TAMAZIGHT         = "tzm"_s;
inline constexpr Str UYGHUR            = "ug"_s;
inline constexpr Str UKRAINIAN         = "uk"_s;
inline constexpr Str URDU              = "ur"_s;
inline constexpr Str UZBEK             = "uz"_s;
inline constexpr Str VIETNAMESE        = "vi"_s;
inline constexpr Str WOLOF             = "wo"_s;
inline constexpr Str XHOSA             = "xh"_s;
inline constexpr Str YORUBA            = "yo"_s;
inline constexpr Str CHINESE           = "zh"_s;
inline constexpr Str ZULU              = "zu"_s;
}    // namespace lang

}    // namespace ash
