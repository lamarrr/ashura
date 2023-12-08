
#pragma once
#include <string_view>

namespace ash
{

// IETF BCP 47 language tags. See:
// https://en.wikipedia.org/wiki/IETF_language_tag
namespace languages
{
constexpr std::string_view AFRIKAANS         = "af";
constexpr std::string_view AMHARIC           = "am";
constexpr std::string_view ARABIC            = "ar";
constexpr std::string_view MAPUDUNGUN        = "arn";
constexpr std::string_view ASSAMESE          = "as";
constexpr std::string_view AZERBAIJANI       = "az";
constexpr std::string_view BASHKIR           = "ba";
constexpr std::string_view BELARUSIAN        = "be";
constexpr std::string_view BULGARIAN         = "bg";
constexpr std::string_view BENGALI           = "bn";
constexpr std::string_view TIBETAN           = "bo";
constexpr std::string_view BRETON            = "br";
constexpr std::string_view BOSNIAN           = "bs";
constexpr std::string_view CATALAN           = "ca";
constexpr std::string_view CORSICAN          = "co";
constexpr std::string_view CZECH             = "cs";
constexpr std::string_view WELSH             = "cy";
constexpr std::string_view DANISH            = "da";
constexpr std::string_view GERMAN            = "de";
constexpr std::string_view LOWER_SORBIAN     = "dsb";
constexpr std::string_view DIVEHI            = "dv";
constexpr std::string_view GREEK             = "el";
constexpr std::string_view ENGLISH           = "en";
constexpr std::string_view SPANISH           = "es";
constexpr std::string_view ESTONIAN          = "et";
constexpr std::string_view BASQUE            = "eu";
constexpr std::string_view PERSIAN           = "fa";
constexpr std::string_view FINNISH           = "fi";
constexpr std::string_view FILIPINO          = "fil";
constexpr std::string_view FAROESE           = "fo";
constexpr std::string_view FRENCH            = "fr";
constexpr std::string_view FRISIAN           = "fy";
constexpr std::string_view IRISH             = "ga";
constexpr std::string_view SCOTTISH_GAELIC   = "gd";
constexpr std::string_view GALICIAN          = "gl";
constexpr std::string_view ALSATIAN          = "gsw";
constexpr std::string_view GUJARATI          = "gu";
constexpr std::string_view HAUSA             = "ha";
constexpr std::string_view HEBREW            = "he";
constexpr std::string_view HINDI             = "hi";
constexpr std::string_view CROATIAN          = "hr";
constexpr std::string_view UPPER_SORBIAN     = "hsb";
constexpr std::string_view HUNGARIAN         = "hu";
constexpr std::string_view ARMENIAN          = "hy";
constexpr std::string_view INDONESIAN        = "id";
constexpr std::string_view IGBO              = "ig";
constexpr std::string_view YI                = "ii";
constexpr std::string_view ICELANDIC         = "is";
constexpr std::string_view ITALIAN           = "it";
constexpr std::string_view INUKTITUT         = "iu";
constexpr std::string_view JAPANESE          = "ja";
constexpr std::string_view GEORGIAN          = "ka";
constexpr std::string_view KAZAKH            = "kk";
constexpr std::string_view GREENLANDIC       = "kl";
constexpr std::string_view KHMER             = "km";
constexpr std::string_view KANNADA           = "kn";
constexpr std::string_view KOREAN            = "ko";
constexpr std::string_view KONKANI           = "kok";
constexpr std::string_view KYRGYZ            = "ky";
constexpr std::string_view LUXEMBOURGISH     = "lb";
constexpr std::string_view LAO               = "lo";
constexpr std::string_view LITHUANIAN        = "lt";
constexpr std::string_view LATVIAN           = "lv";
constexpr std::string_view MAORI_REO         = "mi";
constexpr std::string_view MACEDONIAN        = "mk";
constexpr std::string_view MALAYALAM         = "ml";
constexpr std::string_view MONGOLIAN         = "mn";
constexpr std::string_view MOHAWK            = "moh";
constexpr std::string_view MARATHI           = "mr";
constexpr std::string_view MALAY_BAHASA      = "ms";
constexpr std::string_view MALTESE           = "mt";
constexpr std::string_view BURMESE           = "my";
constexpr std::string_view NORWEGIAN_BOKMAL  = "nb";
constexpr std::string_view NEPALI            = "ne";
constexpr std::string_view DUTCH             = "nl";
constexpr std::string_view NORWEGIAN_NYNORSK = "nn";
constexpr std::string_view NORWEGIAN         = "no";
constexpr std::string_view SESOTHO           = "st";
constexpr std::string_view OCCITAN           = "oc";
constexpr std::string_view ODIA              = "or";
constexpr std::string_view PUNJABI           = "pa";
constexpr std::string_view POLISH            = "pl";
constexpr std::string_view DARI              = "prs";
constexpr std::string_view PASHTO            = "ps";
constexpr std::string_view PORTUGUESE        = "pt";
constexpr std::string_view KICHE             = "quc";
constexpr std::string_view QUECHUA           = "qu";
constexpr std::string_view ROMANSH           = "rm";
constexpr std::string_view ROMANIAN          = "ro";
constexpr std::string_view RUSSIAN           = "ru";
constexpr std::string_view KINYARWANDA       = "rw";
constexpr std::string_view SANSKRIT          = "sa";
constexpr std::string_view YAKUT             = "sah";
constexpr std::string_view SAMI_NORTHERN     = "se";
constexpr std::string_view SINHALA           = "si";
constexpr std::string_view SLOVAK            = "sk";
constexpr std::string_view SLOVENIAN         = "sl";
constexpr std::string_view SAMI_SOUTHERN     = "sma";
constexpr std::string_view SAMI_LULE         = "smj";
constexpr std::string_view SAMI_INARI        = "smn";
constexpr std::string_view SAMI_SKOLT        = "sms";
constexpr std::string_view ALBANIAN          = "sq";
constexpr std::string_view SERBIAN           = "sr";
constexpr std::string_view SWEDISH           = "sv";
constexpr std::string_view KISWAHILI         = "sw";
constexpr std::string_view SYRIAC            = "syc";
constexpr std::string_view TAMIL             = "ta";
constexpr std::string_view TELUGU            = "te";
constexpr std::string_view TAJIK             = "tg";
constexpr std::string_view THAI              = "th";
constexpr std::string_view TURKMEN           = "tk";
constexpr std::string_view TSWANA            = "tn";
constexpr std::string_view TURKISH           = "tr";
constexpr std::string_view TATAR             = "tt";
constexpr std::string_view TAMAZIGHT         = "tzm";
constexpr std::string_view UYGHUR            = "ug";
constexpr std::string_view UKRAINIAN         = "uk";
constexpr std::string_view URDU              = "ur";
constexpr std::string_view UZBEK             = "uz";
constexpr std::string_view VIETNAMESE        = "vi";
constexpr std::string_view WOLOF             = "wo";
constexpr std::string_view XHOSA             = "xh";
constexpr std::string_view YORUBA            = "yo";
constexpr std::string_view CHINESE           = "zh";
constexpr std::string_view ZULU              = "zu";
}        // namespace languages

}        // namespace ash
