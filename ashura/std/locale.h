/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/span.h"

namespace ash
{

/// @brief IETF BCP 47 language tags. See:
/// https://en.wikipedia.org/wiki/IETF_language_tag
namespace lang
{
inline constexpr Str AFRIKAANS         = "af"_str;
inline constexpr Str AMHARIC           = "am"_str;
inline constexpr Str ARABIC            = "ar"_str;
inline constexpr Str MAPUDUNGUN        = "arn"_str;
inline constexpr Str ASSAMESE          = "as"_str;
inline constexpr Str AZERBAIJANI       = "az"_str;
inline constexpr Str BASHKIR           = "ba"_str;
inline constexpr Str BELARUSIAN        = "be"_str;
inline constexpr Str BULGARIAN         = "bg"_str;
inline constexpr Str BENGALI           = "bn"_str;
inline constexpr Str TIBETAN           = "bo"_str;
inline constexpr Str BRETON            = "br"_str;
inline constexpr Str BOSNIAN           = "bs"_str;
inline constexpr Str CATALAN           = "ca"_str;
inline constexpr Str CORSICAN          = "co"_str;
inline constexpr Str CZECH             = "cs"_str;
inline constexpr Str WELSH             = "cy"_str;
inline constexpr Str DANISH            = "da"_str;
inline constexpr Str GERMAN            = "de"_str;
inline constexpr Str LOWER_SORBIAN     = "dsb"_str;
inline constexpr Str DIVEHI            = "dv"_str;
inline constexpr Str GREEK             = "el"_str;
inline constexpr Str ENGLISH           = "en"_str;
inline constexpr Str SPANISH           = "es"_str;
inline constexpr Str ESTONIAN          = "et"_str;
inline constexpr Str BASQUE            = "eu"_str;
inline constexpr Str PERSIAN           = "fa"_str;
inline constexpr Str FINNISH           = "fi"_str;
inline constexpr Str FILIPINO          = "fil"_str;
inline constexpr Str FAROESE           = "fo"_str;
inline constexpr Str FRENCH            = "fr"_str;
inline constexpr Str FRISIAN           = "fy"_str;
inline constexpr Str IRISH             = "ga"_str;
inline constexpr Str SCOTTISH_GAELIC   = "gd"_str;
inline constexpr Str GALICIAN          = "gl"_str;
inline constexpr Str ALSATIAN          = "gsw"_str;
inline constexpr Str GUJARATI          = "gu"_str;
inline constexpr Str HAUSA             = "ha"_str;
inline constexpr Str HEBREW            = "he"_str;
inline constexpr Str HINDI             = "hi"_str;
inline constexpr Str CROATIAN          = "hr"_str;
inline constexpr Str UPPER_SORBIAN     = "hsb"_str;
inline constexpr Str HUNGARIAN         = "hu"_str;
inline constexpr Str ARMENIAN          = "hy"_str;
inline constexpr Str INDONESIAN        = "id"_str;
inline constexpr Str IGBO              = "ig"_str;
inline constexpr Str YI                = "ii"_str;
inline constexpr Str ICELANDIC         = "is"_str;
inline constexpr Str ITALIAN           = "it"_str;
inline constexpr Str INUKTITUT         = "iu"_str;
inline constexpr Str JAPANESE          = "ja"_str;
inline constexpr Str GEORGIAN          = "ka"_str;
inline constexpr Str KAZAKH            = "kk"_str;
inline constexpr Str GREENLANDIC       = "kl"_str;
inline constexpr Str KHMER             = "km"_str;
inline constexpr Str KANNADA           = "kn"_str;
inline constexpr Str KOREAN            = "ko"_str;
inline constexpr Str KONKANI           = "kok"_str;
inline constexpr Str KYRGYZ            = "ky"_str;
inline constexpr Str LUXEMBOURGISH     = "lb"_str;
inline constexpr Str LAO               = "lo"_str;
inline constexpr Str LITHUANIAN        = "lt"_str;
inline constexpr Str LATVIAN           = "lv"_str;
inline constexpr Str MAORI_REO         = "mi"_str;
inline constexpr Str MACEDONIAN        = "mk"_str;
inline constexpr Str MALAYALAM         = "ml"_str;
inline constexpr Str MONGOLIAN         = "mn"_str;
inline constexpr Str MOHAWK            = "moh"_str;
inline constexpr Str MARATHI           = "mr"_str;
inline constexpr Str MALAY_BAHASA      = "ms"_str;
inline constexpr Str MALTESE           = "mt"_str;
inline constexpr Str BURMESE           = "my"_str;
inline constexpr Str NORWEGIAN_BOKMAL  = "nb"_str;
inline constexpr Str NEPALI            = "ne"_str;
inline constexpr Str DUTCH             = "nl"_str;
inline constexpr Str NORWEGIAN_NYNORSK = "nn"_str;
inline constexpr Str NORWEGIAN         = "no"_str;
inline constexpr Str SESOTHO           = "st"_str;
inline constexpr Str OCCITAN           = "oc"_str;
inline constexpr Str ODIA              = "or"_str;
inline constexpr Str PUNJABI           = "pa"_str;
inline constexpr Str POLISH            = "pl"_str;
inline constexpr Str DARI              = "prs"_str;
inline constexpr Str PASHTO            = "ps"_str;
inline constexpr Str PORTUGUESE        = "pt"_str;
inline constexpr Str KICHE             = "quc"_str;
inline constexpr Str QUECHUA           = "qu"_str;
inline constexpr Str ROMANSH           = "rm"_str;
inline constexpr Str ROMANIAN          = "ro"_str;
inline constexpr Str RUSSIAN           = "ru"_str;
inline constexpr Str KINYARWANDA       = "rw"_str;
inline constexpr Str SANSKRIT          = "sa"_str;
inline constexpr Str YAKUT             = "sah"_str;
inline constexpr Str SAMI_NORTHERN     = "se"_str;
inline constexpr Str SINHALA           = "si"_str;
inline constexpr Str SLOVAK            = "sk"_str;
inline constexpr Str SLOVENIAN         = "sl"_str;
inline constexpr Str SAMI_SOUTHERN     = "sma"_str;
inline constexpr Str SAMI_LULE         = "smj"_str;
inline constexpr Str SAMI_INARI        = "smn"_str;
inline constexpr Str SAMI_SKOLT        = "sms"_str;
inline constexpr Str ALBANIAN          = "sq"_str;
inline constexpr Str SERBIAN           = "sr"_str;
inline constexpr Str SWEDISH           = "sv"_str;
inline constexpr Str KISWAHILI         = "sw"_str;
inline constexpr Str SYRIAC            = "syc"_str;
inline constexpr Str TAMIL             = "ta"_str;
inline constexpr Str TELUGU            = "te"_str;
inline constexpr Str TAJIK             = "tg"_str;
inline constexpr Str THAI              = "th"_str;
inline constexpr Str TURKMEN           = "tk"_str;
inline constexpr Str TSWANA            = "tn"_str;
inline constexpr Str TURKISH           = "tr"_str;
inline constexpr Str TATAR             = "tt"_str;
inline constexpr Str TAMAZIGHT         = "tzm"_str;
inline constexpr Str UYGHUR            = "ug"_str;
inline constexpr Str UKRAINIAN         = "uk"_str;
inline constexpr Str URDU              = "ur"_str;
inline constexpr Str UZBEK             = "uz"_str;
inline constexpr Str VIETNAMESE        = "vi"_str;
inline constexpr Str WOLOF             = "wo"_str;
inline constexpr Str XHOSA             = "xh"_str;
inline constexpr Str YORUBA            = "yo"_str;
inline constexpr Str CHINESE           = "zh"_str;
inline constexpr Str ZULU              = "zu"_str;
}    // namespace lang

}    // namespace ash
