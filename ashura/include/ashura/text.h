#pragma once

#include "ashura/font.h"
#include "ashura/primitives.h"
#include "harfbuzz/hb.h"
#include "stx/span.h"
#include "stx/text.h"
#include "stx/vec.h"
#include <string_view>

namespace ash
{

// BCP-47 language codes
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

enum class Script : int
{
  Common                = HB_SCRIPT_COMMON,
  Inherited             = HB_SCRIPT_INHERITED,
  Unknown               = HB_SCRIPT_UNKNOWN,
  Arabic                = HB_SCRIPT_ARABIC,
  Armenian              = HB_SCRIPT_ARMENIAN,
  Bengali               = HB_SCRIPT_BENGALI,
  Cyrillic              = HB_SCRIPT_CYRILLIC,
  Devanagari            = HB_SCRIPT_DEVANAGARI,
  Georgian              = HB_SCRIPT_GEORGIAN,
  Greek                 = HB_SCRIPT_GREEK,
  Gujarati              = HB_SCRIPT_GUJARATI,
  Gurmukhi              = HB_SCRIPT_GURMUKHI,
  Hangul                = HB_SCRIPT_HANGUL,
  Han                   = HB_SCRIPT_HAN,
  Hebrew                = HB_SCRIPT_HEBREW,
  Hiragana              = HB_SCRIPT_HIRAGANA,
  Kannada               = HB_SCRIPT_KANNADA,
  Katakana              = HB_SCRIPT_KATAKANA,
  Lao                   = HB_SCRIPT_LAO,
  Latin                 = HB_SCRIPT_LATIN,
  Malayalam             = HB_SCRIPT_MALAYALAM,
  Oriya                 = HB_SCRIPT_ORIYA,
  Tamil                 = HB_SCRIPT_TAMIL,
  Telugu                = HB_SCRIPT_TELUGU,
  Thai                  = HB_SCRIPT_THAI,
  Tibetan               = HB_SCRIPT_TIBETAN,
  Bopomofo              = HB_SCRIPT_BOPOMOFO,
  Braille               = HB_SCRIPT_BRAILLE,
  CanadianSyllabics     = HB_SCRIPT_CANADIAN_SYLLABICS,
  Cherokee              = HB_SCRIPT_CHEROKEE,
  Ethiopic              = HB_SCRIPT_ETHIOPIC,
  Khmer                 = HB_SCRIPT_KHMER,
  Mongolian             = HB_SCRIPT_MONGOLIAN,
  Myanmar               = HB_SCRIPT_MYANMAR,
  Ogham                 = HB_SCRIPT_OGHAM,
  Runic                 = HB_SCRIPT_RUNIC,
  Sinhala               = HB_SCRIPT_SINHALA,
  Syriac                = HB_SCRIPT_SYRIAC,
  Thaana                = HB_SCRIPT_THAANA,
  Yi                    = HB_SCRIPT_YI,
  Deseret               = HB_SCRIPT_DESERET,
  Gothic                = HB_SCRIPT_GOTHIC,
  OldItalic             = HB_SCRIPT_OLD_ITALIC,
  Buhid                 = HB_SCRIPT_BUHID,
  Hanunoo               = HB_SCRIPT_HANUNOO,
  Tagalog               = HB_SCRIPT_TAGALOG,
  Tagbanwa              = HB_SCRIPT_TAGBANWA,
  Cypriot               = HB_SCRIPT_CYPRIOT,
  Limbu                 = HB_SCRIPT_LIMBU,
  LinearB               = HB_SCRIPT_LINEAR_B,
  Osmanya               = HB_SCRIPT_OSMANYA,
  Shavian               = HB_SCRIPT_SHAVIAN,
  TaiLe                 = HB_SCRIPT_TAI_LE,
  Ugaritic              = HB_SCRIPT_UGARITIC,
  Buginese              = HB_SCRIPT_BUGINESE,
  Coptic                = HB_SCRIPT_COPTIC,
  Glagolitic            = HB_SCRIPT_GLAGOLITIC,
  Kharoshthi            = HB_SCRIPT_KHAROSHTHI,
  NewTaiLue             = HB_SCRIPT_NEW_TAI_LUE,
  OldPersian            = HB_SCRIPT_OLD_PERSIAN,
  SylotiNagri           = HB_SCRIPT_SYLOTI_NAGRI,
  Tifinagh              = HB_SCRIPT_TIFINAGH,
  Balinese              = HB_SCRIPT_BALINESE,
  Cuneiform             = HB_SCRIPT_CUNEIFORM,
  Nko                   = HB_SCRIPT_NKO,
  PhagsPa               = HB_SCRIPT_PHAGS_PA,
  Phoenician            = HB_SCRIPT_PHOENICIAN,
  Carian                = HB_SCRIPT_CARIAN,
  Cham                  = HB_SCRIPT_CHAM,
  KayahLi               = HB_SCRIPT_KAYAH_LI,
  Lepcha                = HB_SCRIPT_LEPCHA,
  Lycian                = HB_SCRIPT_LYCIAN,
  Lydian                = HB_SCRIPT_LYDIAN,
  OlChiki               = HB_SCRIPT_OL_CHIKI,
  Rejang                = HB_SCRIPT_REJANG,
  Saurashtra            = HB_SCRIPT_SAURASHTRA,
  Sundanese             = HB_SCRIPT_SUNDANESE,
  Vai                   = HB_SCRIPT_VAI,
  Avestan               = HB_SCRIPT_AVESTAN,
  Bamum                 = HB_SCRIPT_BAMUM,
  EgyptianHieroglyphs   = HB_SCRIPT_EGYPTIAN_HIEROGLYPHS,
  ImperialAramaic       = HB_SCRIPT_IMPERIAL_ARAMAIC,
  InscriptionalPahlavi  = HB_SCRIPT_INSCRIPTIONAL_PAHLAVI,
  InscriptionalParthian = HB_SCRIPT_INSCRIPTIONAL_PARTHIAN,
  Javanese              = HB_SCRIPT_JAVANESE,
  Kaithi                = HB_SCRIPT_KAITHI,
  Lisu                  = HB_SCRIPT_LISU,
  MeeteiMayek           = HB_SCRIPT_MEETEI_MAYEK,
  OldSouthArabian       = HB_SCRIPT_OLD_SOUTH_ARABIAN,
  OldTurkic             = HB_SCRIPT_OLD_TURKIC,
  Samaritan             = HB_SCRIPT_SAMARITAN,
  TaiTham               = HB_SCRIPT_TAI_THAM,
  TaiViet               = HB_SCRIPT_TAI_VIET,
  Batak                 = HB_SCRIPT_BATAK,
  Brahmi                = HB_SCRIPT_BRAHMI,
  Mandaic               = HB_SCRIPT_MANDAIC,
  Chakma                = HB_SCRIPT_CHAKMA,
  MeroiticCursive       = HB_SCRIPT_MEROITIC_CURSIVE,
  MeroiticHieroglyphs   = HB_SCRIPT_MEROITIC_HIEROGLYPHS,
  Miao                  = HB_SCRIPT_MIAO,
  Sharada               = HB_SCRIPT_SHARADA,
  SoraSompeng           = HB_SCRIPT_SORA_SOMPENG,
  Takri                 = HB_SCRIPT_TAKRI,
  BassaVah              = HB_SCRIPT_BASSA_VAH,
  CaucasianAlbanian     = HB_SCRIPT_CAUCASIAN_ALBANIAN,
  Duployan              = HB_SCRIPT_DUPLOYAN,
  Elbasan               = HB_SCRIPT_ELBASAN,
  Grantha               = HB_SCRIPT_GRANTHA,
  Khojki                = HB_SCRIPT_KHOJKI,
  Khudawadi             = HB_SCRIPT_KHUDAWADI,
  LinearA               = HB_SCRIPT_LINEAR_A,
  Mahajani              = HB_SCRIPT_MAHAJANI,
  Manichaean            = HB_SCRIPT_MANICHAEAN,
  MendeKikakui          = HB_SCRIPT_MENDE_KIKAKUI,
  Modi                  = HB_SCRIPT_MODI,
  Mro                   = HB_SCRIPT_MRO,
  Nabataean             = HB_SCRIPT_NABATAEAN,
  OldNorthArabian       = HB_SCRIPT_OLD_NORTH_ARABIAN,
  OldPermic             = HB_SCRIPT_OLD_PERMIC,
  PahawhHmong           = HB_SCRIPT_PAHAWH_HMONG,
  Palmyrene             = HB_SCRIPT_PALMYRENE,
  PauCinHau             = HB_SCRIPT_PAU_CIN_HAU,
  PsalterPahlavi        = HB_SCRIPT_PSALTER_PAHLAVI,
  Siddham               = HB_SCRIPT_SIDDHAM,
  Tirhuta               = HB_SCRIPT_TIRHUTA,
  WarangCiti            = HB_SCRIPT_WARANG_CITI,
  Ahom                  = HB_SCRIPT_AHOM,
  AnatolianHieroglyphs  = HB_SCRIPT_ANATOLIAN_HIEROGLYPHS,
  Hatran                = HB_SCRIPT_HATRAN,
  Multani               = HB_SCRIPT_MULTANI,
  OldHungarian          = HB_SCRIPT_OLD_HUNGARIAN,
  Signwriting           = HB_SCRIPT_SIGNWRITING,
  Adlam                 = HB_SCRIPT_ADLAM,
  Bhaiksuki             = HB_SCRIPT_BHAIKSUKI,
  Marchen               = HB_SCRIPT_MARCHEN,
  Osage                 = HB_SCRIPT_OSAGE,
  Tangut                = HB_SCRIPT_TANGUT,
  Newa                  = HB_SCRIPT_NEWA,
  MasaramGondi          = HB_SCRIPT_MASARAM_GONDI,
  Nushu                 = HB_SCRIPT_NUSHU,
  Soyombo               = HB_SCRIPT_SOYOMBO,
  ZanabazarSquare       = HB_SCRIPT_ZANABAZAR_SQUARE,
  Dogra                 = HB_SCRIPT_DOGRA,
  GunjalaGondi          = HB_SCRIPT_GUNJALA_GONDI,
  HanifiRohingya        = HB_SCRIPT_HANIFI_ROHINGYA,
  Makasar               = HB_SCRIPT_MAKASAR,
  Medefaidrin           = HB_SCRIPT_MEDEFAIDRIN,
  OldSogdian            = HB_SCRIPT_OLD_SOGDIAN,
  Sogdian               = HB_SCRIPT_SOGDIAN,
  Elymaic               = HB_SCRIPT_ELYMAIC,
  Nandinagari           = HB_SCRIPT_NANDINAGARI,
  NyiakengPuachueHmong  = HB_SCRIPT_NYIAKENG_PUACHUE_HMONG,
  Wancho                = HB_SCRIPT_WANCHO,
  Chorasmian            = HB_SCRIPT_CHORASMIAN,
  DivesAkuru            = HB_SCRIPT_DIVES_AKURU,
  KhitanSmallScript     = HB_SCRIPT_KHITAN_SMALL_SCRIPT,
  Yezidi                = HB_SCRIPT_YEZIDI,
  CyproMinoan           = HB_SCRIPT_CYPRO_MINOAN,
  OldUyghur             = HB_SCRIPT_OLD_UYGHUR,
  Tangsa                = HB_SCRIPT_TANGSA,
  Toto                  = HB_SCRIPT_TOTO,
  Vithkuqi              = HB_SCRIPT_VITHKUQI,
  Math                  = HB_SCRIPT_MATH,
  Kawi                  = HB_SCRIPT_KAWI,
  NagMundari            = HB_SCRIPT_NAG_MUNDARI,
  Invalid               = HB_SCRIPT_INVALID
};

enum class TextDirection : u8
{
  LeftToRight,
  RightToLeft
};

struct TextProps
{
  std::string_view                  font;                                /// name to use to match the font
  stx::Span<std::string_view const> fallback_fonts;                      /// font to fallback to if specified font is not available
  f32                               font_height             = 16;        /// px
  color                             foreground_color        = colors::BLACK;
  color                             background_color        = colors::TRANSPARENT;
  color                             underline_color         = colors::BLACK;
  f32                               underline_thickness     = 0;            /// px
  bool                              wavy_underline          = false;        // TODO(lamarrr): implement
  color                             strikethrough_color     = colors::BLACK;
  f32                               strikethrough_thickness = 0;            /// px
  color                             stroke_color            = colors::BLACK;
  vec2                              stroke_offset;                          /// px
  f32                               letter_spacing = 1;                     /// px
  f32                               word_spacing   = 4;                     /// px
  f32                               line_height    = 1.2f;                  /// will be multiplied by font_height
  TextDirection                     direction      = TextDirection::LeftToRight;
  u32                               tab_size       = 8;                     /// number of spaces to replace a tab with
  Script                            script         = Script::Latin;
  std::string_view                  language       = languages::ENGLISH;
  bool                              use_kerning    = true;
  bool                              use_ligatures  = true;        /// use standard and contextual font ligature substitution
};

/// A text run is a sequence of characters sharing a single property i.e. foreground color, font family etc.
struct TextRun
{
  stx::Span<char const>  text;        /// utf-8-encoded text, Span because string view doesnt support non-string types
  stx::Option<TextProps> props;
};

enum class TextAlign : u8
{
  Left,
  Center,
  Right
};

// TODO(lamarrr): implement ellipsis overflow wrapping
enum class TextOverflow : u8
{
  Wrap,
  Ellipsis
};

struct Paragraph
{
  stx::Span<TextRun const> runs;
  TextProps                props;
  TextAlign                align = TextAlign::Left;
  // TextOverflow             overflow = TextOverflow::Wrap;
  // std::string_view         ellipsis = "...";
};

struct TextRunArea
{
  vec2 offset;
  vec2 extent;
  vec2 baseline;
  vec2 line_top;
};

/// this is part of a word that is styled by a run
struct TextRunSubWord
{
  stx::Span<char const> text;
  usize                 run            = 0;
  usize                 font           = 0;
  f32                   glyph_scale    = 0;
  usize                 nspace_chars   = 0;
  usize                 nnewline_chars = 0;
  usize                 nline_breaks   = 0;
  f32                   width          = 0;        /// width of all the letters excluding the trailing white spaces
  usize                 glyphs_begin   = 0;
  usize                 nglyphs        = 0;
  bool                  is_wrapped     = false;
  TextRunArea           area;
};

struct GlyphLayout
{
  vec2  offset;
  vec2  extent;
  usize run   = 0;
  usize font  = 0;
  u32   glyph = 0;
};

struct TextLayout
{
  stx::Vec<TextRunSubWord> subwords;
  stx::Vec<u32>            glyph_indices;
  stx::Vec<GlyphLayout>    glyph_layouts;

  // TODO(lamarrr): [future] add bidi
  /// performs layout of the paragraph and returns the width and height of the paragraph
  vec2 layout(Paragraph const &paragraph, stx::Span<BundledFont const> const font_bundle, f32 max_line_width)
  {
    constexpr u32 SPACE   = ' ';
    constexpr u32 TAB     = '\t';
    constexpr u32 NEWLINE = '\n';
    constexpr u32 RETURN  = '\r';

    subwords.clear();
    glyph_indices.clear();
    glyph_layouts.clear();

    vec2 span;

    // there's no layout to perform without a font
    if (font_bundle.is_empty())
    {
      return span;
    }

    for (usize i = 0; i < paragraph.runs.size(); i++)
    {
      TextRun const   &run   = paragraph.runs[i];
      TextProps const &props = run.props.as_cref().unwrap_or(paragraph.props);

      for (char const *word_begin = run.text.begin(); word_begin < run.text.end();)
      {
        usize       nspace_chars   = 0;
        usize       nnewline_chars = 0;
        char const *seeker         = word_begin;
        char const *word_end       = seeker;
        u32         codepoint      = 0;

        for (; seeker < run.text.end();)
        {
          codepoint = stx::utf8_next(seeker);

          if (codepoint == RETURN || codepoint == NEWLINE || codepoint == TAB || codepoint == SPACE)
          {
            break;
          }
        }

        word_end = seeker;

        if (codepoint == RETURN)
        {
          word_end = seeker - 1;

          if (seeker + 1 < run.text.end())
          {
            if (*(seeker + 1) == NEWLINE)
            {
              seeker++;
              nnewline_chars++;
            }
          }
        }
        else if (codepoint == SPACE)
        {
          word_end = seeker - 1;
          nspace_chars++;

          for (char const *iter = seeker; iter < run.text.end();)
          {
            seeker        = iter;
            u32 codepoint = stx::utf8_next(iter);

            if (codepoint == SPACE)
            {
              nspace_chars++;
            }
            else
            {
              break;
            }
          }
        }
        else if (codepoint == TAB)
        {
          word_end = seeker - 1;
          nspace_chars += props.tab_size;

          for (char const *iter = seeker; iter < run.text.end();)
          {
            seeker        = iter;
            u32 codepoint = stx::utf8_next(iter);
            if (codepoint == TAB)
            {
              nspace_chars += props.tab_size;
            }
            else
            {
              break;
            }
          }
        }
        else if (codepoint == NEWLINE)
        {
          word_end = seeker - 1;
          nnewline_chars++;

          for (char const *iter = seeker; iter < run.text.end();)
          {
            seeker        = iter;
            u32 codepoint = stx::utf8_next(iter);

            if (codepoint == NEWLINE)
            {
              nnewline_chars++;
            }
            else
            {
              break;
            }
          }
        }

        subwords
            .push(TextRunSubWord{.text           = run.text.slice(word_begin - run.text.begin(), word_end - word_begin),
                                 .run            = i,
                                 .nspace_chars   = nspace_chars,
                                 .nnewline_chars = nnewline_chars})
            .unwrap();

        word_begin = seeker;
      }
    }

    for (TextRunSubWord &subword : subwords)
    {
      TextProps const &props  = paragraph.runs[subword.run].props.as_cref().unwrap_or(paragraph.props);
      stx::Span        font_s = font_bundle.which([&](BundledFont const &f) {
        return f.name == props.font;
      });

      // if font not found, check if fallback font is found
      if (font_s.is_empty())
      {
        for (std::string_view fallback : props.fallback_fonts)
        {
          font_s = font_bundle.which([&](BundledFont const &f) {
            return f.name == fallback;
          });

          if (!font_s.is_empty())
          {
            break;
          }
        }
      }

      // if no font or fallback font is found use the first font in the bundle
      if (font_s.is_empty())
      {
        font_s = font_bundle.slice(0, 1);
      }

      usize            font_index = AS(usize, font_s.begin() - font_bundle.begin());
      Font const      &font       = *font_bundle[font_index].font;
      FontAtlas const &atlas      = font_bundle[font_index].atlas;

      // use glyph at index 0 as replacement glyph, this is usually the invalid character replacement glyph
      stx::Span replacement_glyph = atlas.get(0);

      hb_feature_t shaping_features[] = {{.tag = Font::KERNING_FEATURE, .value = props.use_kerning, .start = 0, .end = stx::U_MAX},
                                         {.tag = Font::LIGATURE_FEATURE, .value = props.use_ligatures, .start = 0, .end = stx::U_MAX},
                                         {.tag = Font::CONTEXTUAL_LIGATURE_FEATURE, .value = props.use_ligatures, .start = 0, .end = stx::U_MAX}};

      hb_font_set_scale(font.hb_font, 64 * atlas.font_height, 64 * atlas.font_height);

      hb_buffer_reset(font.hb_scratch_buffer);
      hb_buffer_set_script(font.hb_scratch_buffer, AS(hb_script_t, props.script));

      if (props.direction == TextDirection::LeftToRight)
      {
        hb_buffer_set_direction(font.hb_scratch_buffer, HB_DIRECTION_LTR);
      }
      else
      {
        hb_buffer_set_direction(font.hb_scratch_buffer, HB_DIRECTION_RTL);
      }

      hb_buffer_set_language(font.hb_scratch_buffer, hb_language_from_string(props.language.data(), AS(int, props.language.size())));
      hb_buffer_add_utf8(font.hb_scratch_buffer, subword.text.begin(), AS(int, subword.text.size()), 0, AS(int, subword.text.size()));
      hb_shape(font.hb_font, font.hb_scratch_buffer, shaping_features, AS(uint, std::size(shaping_features)));

      uint             nglyphs;
      hb_glyph_info_t *glyph_info = hb_buffer_get_glyph_infos(font.hb_scratch_buffer, &nglyphs);
      ASH_CHECK(!(glyph_info != nullptr && nglyphs > 0));

      f32 width       = 0;
      f32 glyph_scale = props.font_height / atlas.font_height;

      subword.font         = font_index;
      subword.glyphs_begin = glyph_indices.size();

      for (usize i = 0; i < nglyphs; i++)
      {
        u32       glyph_index = glyph_info[i].codepoint;
        stx::Span glyph       = atlas.get(glyph_index);

        if (!glyph.is_empty())
        {
          width += glyph_scale * glyph[0].advance.x + props.letter_spacing;
          glyph_indices.push_inplace(glyph_index).unwrap();
          subword.nglyphs++;
        }
        else if (!replacement_glyph.is_empty())
        {
          width += glyph_scale * replacement_glyph[0].advance.x + props.letter_spacing;
          glyph_indices.push_inplace(replacement_glyph[0].index).unwrap();
          subword.nglyphs++;
        }
      }

      subword.glyph_scale = glyph_scale;
      subword.width       = width;
    }

    // wrap words to new line if its width exceeds the maximum line width
    {
      //
      // if it has a newline, the wrapping doesn't count as a line break
      //
      f32 cursor_x = 0;

      // TODO(lamarrr): calculate nline_breaks

      for (TextRunSubWord *iter = subwords.begin(); iter < subwords.end();)
      {
        TextRunSubWord *subword = iter;

        for (; subword < subwords.end();)
        {
          TextProps const &props         = paragraph.runs[subword->run].props.as_cref().unwrap_or(paragraph.props);
          f32              subword_width = subword->width + subword->nspace_chars * props.word_spacing;

          // if end of word
          if (subword->nspace_chars > 0 || subword->nnewline_chars > 0 || subword == subwords.end() - 1)
          {
            // check if wrapping needed
            if (cursor_x + subword_width > max_line_width)
            {
              iter->is_wrapped = true;
              cursor_x         = subword_width;        // TODO(lamarrr): this isn't correct
              if (subword->nnewline_chars > 0)
              {
                cursor_x = 0;
              }
            }
            else
            {
              if (subword->nnewline_chars > 0)
              {
                cursor_x = 0;
              }
              else
              {
                cursor_x += subword_width;
              }
            }
            subword++;
            break;
          }
          else
          {
            // continue until we reach end of word
            cursor_x += subword_width;
            subword++;
          }
        }

        iter = subword;
      }

      for (TextRunSubWord *iter = subwords.begin(); iter < subwords.end();)
      {
        TextRunSubWord  *word_begin = iter;
        TextProps const &props      = paragraph.runs[word_begin->run].props.as_cref().unwrap_or(paragraph.props);
        f32              word_width = word_begin->width + word_begin->nspace_chars * props.word_spacing;

        TextRunSubWord *word_end = word_begin + 1;

        if (word_begin->nspace_chars == 0 && word_begin->nnewline_chars == 0)
        {
          for (; word_end < subwords.end();)
          {
            TextProps const &props = paragraph.runs[word_end->run].props.as_cref().unwrap_or(paragraph.props);
            word_width += word_end->width + word_end->nspace_chars * props.word_spacing;
            if (word_end->nspace_chars > 0 || word_end->nnewline_chars > 0)
            {
              word_end++;
              break;
            }
            else
            {
              word_end++;
            }
          }
        }

        if (cursor_x + word_width > max_line_width)
        {
          word_begin->is_wrapped = true;
          cursor_x               = word_width;
        }
        else
        {
          cursor_x += word_width;
        }
      }
    }

    {
      f32 line_top = 0;

      for (TextRunSubWord *iter = subwords.begin(); iter < subwords.end();)
      {
        TextRunSubWord *line_begin   = iter;
        TextRunSubWord *line_end     = iter;
        usize           nline_breaks = 0;

        for (; line_end < subwords.end(); line_end++)
        {
          if (line_end->nline_breaks > 0)
          {
            nline_breaks = line_end->nline_breaks;
            line_end++;
            break;
          }
        }

        f32 line_width  = 0;
        f32 line_height = 0;
        f32 max_ascent  = 0;
        f32 max_descent = 0;

        for (TextRunSubWord const *subword = line_begin; subword < line_end; subword++)
        {
          TextProps const &props = paragraph.runs[subword->run].props.as_cref().unwrap_or(paragraph.props);
          FontAtlas const &atlas = font_bundle[subword->font].atlas;

          line_width += subword->width + subword->nspace_chars * props.word_spacing;
          line_height = std::max(line_height, props.line_height * props.font_height);

          max_ascent  = std::max(max_ascent, subword->glyph_scale * atlas.ascent);
          max_descent = std::max(max_descent, subword->glyph_scale * atlas.descent);
        }

        f32 line_vertical_padding = std::max((line_height - (max_ascent + max_descent)) / 2, 0.0f);
        f32 baseline_y            = line_top + line_vertical_padding + max_ascent;

        f32 line_alignment_x = 0;

        if (paragraph.align == TextAlign::Center)
        {
          line_alignment_x = std::max(max_line_width - line_width, 0.0f) / 2;
        }
        else if (paragraph.align == TextAlign::Right)
        {
          line_alignment_x = std::max(max_line_width - line_width, 0.0f);
        }

        f32 cursor_x = 0;

        for (TextRunSubWord *subword = line_begin; subword < line_end;)
        {
          TextProps const &props = paragraph.runs[subword->run].props.as_cref().unwrap_or(paragraph.props);
          if (props.direction == TextDirection::LeftToRight)
          {
            FontAtlas const &atlas = font_bundle[subword->font].atlas;

            subword->area.offset   = vec2{line_alignment_x + cursor_x, line_top};
            subword->area.extent   = vec2{subword->width + subword->nspace_chars * props.word_spacing, line_height};
            subword->area.baseline = vec2{line_alignment_x + cursor_x, baseline_y};
            subword->area.line_top = vec2{line_alignment_x + cursor_x, baseline_y - subword->glyph_scale * atlas.ascent};

            for (u32 glyph_index : glyph_indices.span().slice(subword->glyphs_begin, subword->nglyphs))
            {
              Glyph const &glyph  = atlas.glyphs[glyph_index];
              vec2         offset = vec2{line_alignment_x + cursor_x + glyph.bearing.x, baseline_y - glyph.bearing.y};

              glyph_layouts.push(GlyphLayout{
                                     .offset = offset,
                                     .extent = subword->glyph_scale * glyph.extent.to_vec(),
                                     .run    = subword->run,
                                     .font   = subword->font,
                                     .glyph  = glyph_index})
                  .unwrap();

              cursor_x += subword->glyph_scale * glyph.advance.x + props.letter_spacing;
            }

            cursor_x += subword->nspace_chars * props.word_spacing;
            subword++;
          }
          else
          {
            f32             rtl_width = 0;
            TextRunSubWord *rtl_begin = subword;
            TextRunSubWord *rtl_end   = subword + 1;

            TextProps const &props = paragraph.runs[rtl_begin->run].props.as_cref().unwrap_or(paragraph.props);

            rtl_width += rtl_begin->width + rtl_begin->nspace_chars * props.word_spacing;

            for (; rtl_end < line_end; rtl_end++)
            {
              TextProps const &props = paragraph.runs[rtl_end->run].props.as_cref().unwrap_or(paragraph.props);
              if (props.direction == TextDirection::LeftToRight)
              {
                break;
              }
              else
              {
                rtl_width += rtl_end->width + rtl_end->nspace_chars * props.word_spacing;
              }
            }

            f32 rtl_cursor_x = cursor_x + rtl_width;

            for (TextRunSubWord *rtl_iter = rtl_begin; rtl_iter < rtl_end; rtl_iter++)
            {
              TextProps const &props = paragraph.runs[rtl_iter->run].props.as_cref().unwrap_or(paragraph.props);
              FontAtlas const &atlas = font_bundle[rtl_iter->font].atlas;

              rtl_cursor_x -= rtl_iter->width + rtl_iter->nspace_chars * props.word_spacing;

              rtl_iter->area.offset   = vec2{line_alignment_x + rtl_cursor_x, line_top};
              rtl_iter->area.extent   = vec2{rtl_iter->width + rtl_iter->nspace_chars * props.word_spacing, line_height};
              rtl_iter->area.baseline = vec2{line_alignment_x + rtl_cursor_x, baseline_y};
              rtl_iter->area.line_top = vec2{line_alignment_x + cursor_x, baseline_y - rtl_iter->glyph_scale * atlas.ascent};

              f32 glyph_cursor_x = rtl_cursor_x;

              for (u32 glyph_index : glyph_indices.span().slice(rtl_iter->glyphs_begin, rtl_iter->nglyphs))
              {
                Glyph const &glyph  = atlas.glyphs[glyph_index];
                vec2         offset = vec2{line_alignment_x + glyph_cursor_x + glyph.bearing.x, baseline_y - glyph.bearing.y};

                glyph_layouts.push(GlyphLayout{
                                       .offset = offset,
                                       .extent = rtl_iter->glyph_scale * glyph.extent.to_vec(),
                                       .run    = subword->run,
                                       .font   = subword->font,
                                       .glyph  = glyph_index})
                    .unwrap();

                glyph_cursor_x += rtl_iter->glyph_scale * glyph.advance.x + props.letter_spacing;
              }
            }

            cursor_x += rtl_width;
            subword = rtl_end;
          }
        }

        line_top += nline_breaks * line_height;

        span.x = std::max(span.x, line_alignment_x + line_width);

        span.y = line_top;

        iter = line_end;
      }
    }

    glyph_indices.clear();

    return span;
  }
};

}        // namespace ash