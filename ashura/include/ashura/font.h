#pragma once
#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <string_view>
#include <utility>

#include "ashura/image.h"
#include "ashura/primitives.h"
#include "ashura/rect_pack.h"
#include "freetype/freetype.h"
#include "harfbuzz/hb.h"
#include "stx/allocator.h"
#include "stx/enum.h"
#include "stx/limits.h"
#include "stx/memory.h"
#include "stx/span.h"
#include "stx/string.h"
#include "stx/vec.h"

namespace ash
{

enum class FontLoadError : u8
{
  InvalidPath
};

enum class TextAlign : u8
{
  Left,
  Center,
  Right
};

enum class TextDirection : u8
{
  LeftToRight,
  RightToLeft
};

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

enum class Script : stx::enum_ut<hb_script_t>
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

// TODO(lamarrr): implement
enum class TextOverflow : u8
{
  None,
  Ellipsis
};

struct TextStyle
{
  f32   font_height         = 16;
  f32   line_height         = 1.2f;        /// multiplied by font_height
  f32   letter_spacing      = 1;
  f32   word_spacing        = 4;
  u32   tab_size            = 8;
  bool  use_kerning         = true;
  bool  use_ligatures       = true;         /// use standard and contextual ligature substitution
  bool  underline           = false;
  bool  strikethrough       = false;        // TODO(lamarrr): implement
  color foreground_color    = colors::BLACK;
  color background_color    = colors::TRANSPARENT;
  color underline_color     = colors::TRANSPARENT;
  f32   underline_thickness = 1;
};

/// A text run is a sequence of characters sharing a single property set. Any
/// change to the format, such as font style, foreground color, font family, or
/// any other formatting effect, breaks the text run.
struct TextRun
{
  stx::Span<char const> text;        /// utf-8-encoded text
  usize                 font = 0;
  TextStyle             style;
  TextDirection         direction = TextDirection::LeftToRight;
  Script                script    = Script::Latin;
  std::string_view      language  = languages::ENGLISH;
};

struct Paragraph
{
  stx::Span<TextRun const> runs;
  TextAlign                align    = TextAlign::Left;
  TextOverflow             overflow = TextOverflow::None;
};

struct Font
{
  static constexpr hb_tag_t KERNING_FEATURE             = HB_TAG('k', 'e', 'r', 'n');        /// kerning operations
  static constexpr hb_tag_t LIGATURE_FEATURE            = HB_TAG('l', 'i', 'g', 'a');        /// standard ligature substitution
  static constexpr hb_tag_t CONTEXTUAL_LIGATURE_FEATURE = HB_TAG('c', 'l', 'i', 'g');        /// contextual ligature substitution

  hb_face_t   *hbface           = nullptr;
  hb_font_t   *hbfont           = nullptr;
  hb_buffer_t *hbscratch_buffer = nullptr;
  FT_Library   ftlib            = nullptr;
  FT_Face      ftface           = nullptr;
  bool         has_color        = false;
  stx::Memory  font_data;

  Font(hb_face_t *ahbface, hb_font_t *ahbfont, hb_buffer_t *ahbscratch_buffer, FT_Library aftlib, FT_Face aftface,
       bool ahas_color, stx::Memory afont_data) :
      hbface{ahbface},
      hbfont{ahbfont},
      hbscratch_buffer{ahbscratch_buffer},
      ftlib{aftlib},
      ftface{aftface},
      has_color{ahas_color},
      font_data{std::move(afont_data)}
  {}

  STX_MAKE_PINNED(Font)

  ~Font()
  {
    hb_face_destroy(hbface);
    hb_font_destroy(hbfont);
    hb_buffer_destroy(hbscratch_buffer);
    ASH_CHECK(FT_Done_Face(ftface) == 0);
    ASH_CHECK(FT_Done_FreeType(ftlib) == 0);
  }
};

inline stx::Rc<Font *> load_font_from_memory(stx::Memory memory, usize size)
{
  hb_blob_t *hbblob = hb_blob_create(AS(char *, memory.handle), AS(uint, size), HB_MEMORY_MODE_READONLY, nullptr, nullptr);
  ASH_CHECK(hbblob != nullptr);

  hb_face_t *hbface = hb_face_create(hbblob, 0);
  ASH_CHECK(hbface != nullptr);

  hb_font_t *hbfont = hb_font_create(hbface);
  ASH_CHECK(hbfont != nullptr);

  hb_buffer_t *hbscratch_buffer = hb_buffer_create();
  ASH_CHECK(hbscratch_buffer != nullptr);

  FT_Library ftlib;
  ASH_CHECK(FT_Init_FreeType(&ftlib) == 0);

  FT_Face ftface;
  ASH_CHECK(FT_New_Memory_Face(ftlib, AS(FT_Byte const *, memory.handle), AS(FT_Long, size), 0, &ftface) == 0);

  return stx::rc::make_inplace<Font>(stx::os_allocator, hbface, hbfont, hbscratch_buffer, ftlib, ftface, FT_HAS_COLOR(ftface),
                                     std::move(memory))
      .unwrap();
}

inline stx::Result<stx::Rc<Font *>, FontLoadError> load_font_from_file(stx::CStringView path)
{
  if (!std::filesystem::exists(path.c_str()))
  {
    return stx::Err(FontLoadError::InvalidPath);
  }

  std::FILE *file = std::fopen(path.c_str(), "rb");
  ASH_CHECK(file != nullptr);

  ASH_CHECK(std::fseek(file, 0, SEEK_END) == 0);

  long file_size = std::ftell(file);
  ASH_CHECK(file_size >= 0);

  stx::Memory memory = stx::mem::allocate(stx::os_allocator, file_size).unwrap();

  ASH_CHECK(std::fseek(file, 0, SEEK_SET) == 0);

  ASH_CHECK(std::fread(memory.handle, 1, file_size, file) == file_size);

  ASH_CHECK(std::fclose(file) == 0);

  return stx::Ok(load_font_from_memory(std::move(memory), file_size));
}

namespace gfx
{

struct Glyph
{
  bool        is_valid = false;
  u32         index    = 0;                          /// the glyph index
  ash::offset offset;                                /// offset into the atlas its glyph resides
  ash::extent extent;                                /// extent of the glyph in the atlas
  f32         x      = 0;                            /// defines x-offset from cursor position the glyph will be placed
  f32         ascent = 0;                            /// defines ascent from baseline of the text
  vec2        advance;                               /// advancement of the cursor after drawing this glyph
  f32         s0 = 0, t0 = 0, s1 = 0, t1 = 0;        /// texture coordinates of this glyph in the atlas
};

/// stores codepoint glyphs for a font at a specific font height
struct FontAtlas
{
  stx::Vec<Glyph> glyphs;
  ash::extent     extent;                  /// overall extent of the atlas
  u32             font_height = 26;        /// font height at which the cache/atlas/glyphs will be rendered and cached
  image           texture     = 0;         /// atlas containing the packed glyphs

  stx::Span<Glyph const> get(u32 glyph_index) const
  {
    if (glyph_index >= glyphs.size())
    {
      return {};
    }
    stx::Span glyph = glyphs.span().slice(glyph_index, 1);
    if (glyph.is_empty())
    {
      return glyph;
    }
    return glyph[0].is_valid ? glyph : glyph.slice(0, 0);
  }
};

inline std::pair<FontAtlas, ImageBuffer> render_atlas(Font const &font, u32 font_height, extent max_extent)
{
  /// *64 to convert font height to 26.6 pixel format
  ASH_CHECK(font_height > 0);
  ASH_CHECK(FT_Set_Char_Size(font.ftface, 0, font_height * 64, 72, 72) == 0);

  stx::Vec<Glyph> glyphs;

  {
    FT_UInt glyph_index;

    FT_ULong codepoint = FT_Get_First_Char(font.ftface, &glyph_index);

    while (glyph_index != 0)
    {
      if (FT_Load_Glyph(font.ftface, glyph_index, 0) == 0)
      {
        u32 width  = font.ftface->glyph->bitmap.width;
        u32 height = font.ftface->glyph->bitmap.rows;

        // convert from 26.6 pixel format
        vec2 advance{font.ftface->glyph->advance.x / 64.0f, font.ftface->glyph->advance.y / 64.0f};

        glyphs
            .push(Glyph{.is_valid = true,
                        .index    = glyph_index,
                        .offset   = offset{},
                        .extent   = extent{width, height},
                        .x        = AS(f32, font.ftface->glyph->bitmap_left),
                        .ascent   = AS(f32, font.ftface->glyph->bitmap_top),
                        .advance  = advance,
                        .s0       = 0,
                        .t0       = 0,
                        .s1       = 0,
                        .t1       = 0})
            .unwrap();
      }
      codepoint = FT_Get_Next_Char(font.ftface, codepoint, &glyph_index);
    }
  }

  stx::Memory rects_mem = stx::mem::allocate(stx::os_allocator, sizeof(rp::rect) * glyphs.size()).unwrap();

  stx::Span rects{AS(rp::rect *, rects_mem.handle), glyphs.size()};

  for (usize i = 0; i < glyphs.size(); i++)
  {
    rects[i].glyph_index = glyphs[i].index;
    rects[i].w           = AS(i32, glyphs[i].extent.width + 2);
    rects[i].h           = AS(i32, glyphs[i].extent.height + 2);
  }

  stx::Memory nodes_memory = stx::mem::allocate(stx::os_allocator, sizeof(rp::Node) * max_extent.width).unwrap();

  rp::Context context = rp::init(max_extent.width, max_extent.height, AS(rp::Node *, nodes_memory.handle), max_extent.width, false);
  ASH_CHECK(rp::pack_rects(context, rects.data(), AS(i32, rects.size())));

  // NOTE: vulkan doesn't allow zero-extent images
  extent atlas_extent{1, 1};

  for (usize i = 0; i < rects.size(); i++)
  {
    atlas_extent.width  = std::max<u32>(atlas_extent.width, rects[i].x + rects[i].w);
    atlas_extent.height = std::max<u32>(atlas_extent.height, rects[i].y + rects[i].h);
  }

  rects.sort([](rp::rect const &a, rp::rect const &b) { return a.glyph_index < b.glyph_index; });

  glyphs.span().sort([](Glyph const &a, Glyph const &b) { return a.index < b.index; });

  for (usize i = 0; i < glyphs.size(); i++)
  {
    u32 x = AS(u32, rects[i].x) + 1;
    u32 y = AS(u32, rects[i].y) + 1;
    u32 w = glyphs[i].extent.width;
    u32 h = glyphs[i].extent.height;

    glyphs[i].offset = {x, y};
    // TODO(lamarrr): 0.5f offset to prevent texture bleeding when sampling
    // borders
    // https://gamedev.stackexchange.com/questions/46963/how-to-avoid-texture-bleeding-in-a-texture-atlas
    glyphs[i].s0 = (AS(f32, x) + 0.5f) / atlas_extent.width;
    glyphs[i].s1 = (AS(f32, x + w) - 0.5f) / atlas_extent.width;
    glyphs[i].t0 = (AS(f32, y) + 0.5f) / atlas_extent.height;
    glyphs[i].t1 = (AS(f32, y + h) - 0.5f) / atlas_extent.height;
  }

  {
    usize iter    = 0;
    usize nglyphs = glyphs.size();
    for (u32 next_index = 0; iter < nglyphs; next_index++, iter++)
    {
      for (; next_index < glyphs[iter].index; next_index++)
      {
        glyphs
            .push(Glyph{.is_valid = false,
                        .index    = next_index,
                        .offset   = {},
                        .extent   = {},
                        .x        = 0,
                        .ascent   = 0,
                        .advance  = {},
                        .s0       = 0,
                        .t0       = 0,
                        .s1       = 0,
                        .t1       = 0})
            .unwrap();
      }
    }
  }

  glyphs.span().sort([](Glyph const &a, Glyph const &b) { return a.index < b.index; });

  ImageFormat format = font.has_color ? ImageFormat::Bgra : ImageFormat::Antialiasing;

  usize nchannels = font.has_color ? 4 : 1;

  usize size = atlas_extent.area() * nchannels;

  stx::Memory buffer_mem = stx::mem::allocate(stx::os_allocator, size).unwrap();

  u8 *buffer = AS(u8 *, buffer_mem.handle);

  std::memset(buffer, 0, size);

  // it rarely happens that a font will contain both colored and gray fonts, if
  // it happens, at least do something reasonable by converting between both

  for (Glyph const &glyph : glyphs)
  {
    if (glyph.is_valid)
    {
      ASH_CHECK(FT_Load_Glyph(font.ftface, glyph.index,
                              font.has_color ? (FT_LOAD_DEFAULT | FT_LOAD_RENDER | FT_LOAD_COLOR) :
                                               (FT_LOAD_DEFAULT | FT_LOAD_RENDER)) == 0);

      FT_Pixel_Mode pixel_mode = AS(FT_Pixel_Mode, font.ftface->glyph->bitmap.pixel_mode);

      ASH_CHECK(pixel_mode == FT_PIXEL_MODE_GRAY || pixel_mode == FT_PIXEL_MODE_BGRA);

      u8 const *bitmap = font.ftface->glyph->bitmap.buffer;

      usize stride = glyph.extent.width * nchannels;

      // copy the rendered glyph to the atlas
      if (pixel_mode == FT_PIXEL_MODE_GRAY && !font.has_color)
      {
        for (usize j = glyph.offset.y; j < glyph.offset.y + glyph.extent.height; j++)
        {
          u8 *out = buffer + j * stride + glyph.offset.x;
          std::copy(bitmap, bitmap + glyph.extent.width, out);
          bitmap += font.ftface->glyph->bitmap.pitch;
        }
      }
      else if (pixel_mode == FT_PIXEL_MODE_BGRA && font.has_color)
      {
        for (usize j = glyph.offset.y; j < glyph.offset.y + glyph.extent.height; j++)
        {
          u8 *out = buffer + j * stride + glyph.offset.x * 4;
          std::copy(bitmap, bitmap + stride, out);
          bitmap += font.ftface->glyph->bitmap.pitch;
        }
      }
      else
      {
        // TODO(lamarrr): log warning about multi-image font
        spdlog::warn("multi-colored font detected");
      }
    }
  }

  return std::make_pair(
      FontAtlas{.glyphs = std::move(glyphs), .extent = atlas_extent, .font_height = font_height, .texture = 0},
      ImageBuffer{.memory = std::move(buffer_mem), .extent = atlas_extent, .format = format});
}

struct FontGlyph
{
  usize font  = 0;
  u32   glyph = 0;
};

/// this is part of a word that is styled by its run
struct RunSubWord
{
  stx::Span<char const> text;
  usize                 run          = 0;
  usize                 nspaces      = 0;
  usize                 nline_breaks = 0;
  f32                   width        = 0;
  usize                 glyph_start  = 0;
  usize                 nglyphs      = 0;
  bool                  is_wrapped   = false;
};

struct GlyphPlacement
{
  vec2      offset;        // offset from root text position
  FontGlyph glyph;
};

struct LineStrokePlacement
{
};

struct TextConstraint
{
  f32 min_width  = 0;        // width of smallest word
  f32 min_height = 0;        // corresponding height of the text when the paragraph is at maximum width
  f32 max_width  = 0;        // width when the paragraph laid out on a single line without wrapping. this assumes there is infinite space
  f32 max_height = 0;        // height when the paragraph is at minimum width.
};

struct TextPlacementContext
{
  TextConstraint                constraint;
  stx::Vec<RunSubWord>          subwords;
  stx::Vec<FontGlyph>           glyphs;
  stx::Vec<GlyphPlacement>      glyph_placements;
  stx::Vec<LineStrokePlacement> stroke_placements;

  /**
   * @brief performs layout of the paragraph
   *
   * @return the width and height of the paragraph
   */
  vec2 layout(Paragraph paragraph, stx::Span<Font const> fonts, stx::Span<FontAtlas const> atlases, f32 max_line_width)
  {
    constexpr u32 SPACE   = ' ';
    constexpr u32 TAB     = '\t';
    constexpr u32 NEWLINE = '\n';
    constexpr u32 RETURN  = '\r';

    glyph_placements.clear();
    stroke_placements.clear();
    subwords.clear();
    glyphs.clear();

    for (usize i = 0; i < paragraph.runs.size(); i++)
    {
      TextRun const &run = paragraph.runs[i];

      for (char const *word_begin = run.text.begin(); word_begin < run.text.end();)
      {
        usize       nspaces      = 0;
        usize       nline_breaks = 0;
        char const *seeker       = word_begin;
        char const *word_end     = seeker;
        u32         codepoint    = 0;

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
              nline_breaks++;
            }
          }
        }
        else if (codepoint == SPACE)
        {
          word_end = seeker - 1;
          nspaces++;

          for (char const *iter = seeker; iter < run.text.end();)
          {
            seeker        = iter;
            u32 codepoint = stx::utf8_next(iter);

            if (codepoint == SPACE)
            {
              nspaces++;
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
          nspaces += run.style.tab_size;

          for (char const *iter = seeker; iter < run.text.end();)
          {
            seeker        = iter;
            u32 codepoint = stx::utf8_next(iter);
            if (codepoint == TAB)
            {
              nspaces += run.style.tab_size;
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
          nline_breaks++;

          for (char const *iter = seeker; iter < run.text.end();)
          {
            seeker        = iter;
            u32 codepoint = stx::utf8_next(iter);

            if (codepoint == NEWLINE)
            {
              nline_breaks++;
            }
            else
            {
              break;
            }
          }
        }

        subwords
            .push(RunSubWord{.text         = run.text.slice(word_begin - run.text.begin(), word_end - word_begin),
                             .run          = i,
                             .nspaces      = nspaces,
                             .nline_breaks = nline_breaks})
            .unwrap();

        word_begin = seeker;
      }
    }

    for (RunSubWord &subword : subwords)
    {
      TextRun const   &run   = paragraph.runs[subword.run];
      Font const      &font  = fonts[run.font];
      FontAtlas const &atlas = atlases[run.font];

      hb_feature_t const shaping_features[] = {{Font::KERNING_FEATURE, run.style.use_kerning, 0, stx::U_MAX},
                                               {Font::LIGATURE_FEATURE, run.style.use_ligatures, 0, stx::U_MAX},
                                               {Font::CONTEXTUAL_LIGATURE_FEATURE, run.style.use_ligatures, 0, stx::U_MAX}};

      hb_font_set_scale(font.hbfont, 64 * atlas.font_height, 64 * atlas.font_height);

      hb_buffer_reset(font.hbscratch_buffer);
      hb_buffer_set_script(font.hbscratch_buffer, AS(hb_script_t, run.script));

      if (run.direction == TextDirection::LeftToRight)
      {
        hb_buffer_set_direction(font.hbscratch_buffer, HB_DIRECTION_LTR);
      }
      else
      {
        hb_buffer_set_direction(font.hbscratch_buffer, HB_DIRECTION_RTL);
      }
      hb_buffer_set_language(font.hbscratch_buffer, hb_language_from_string(run.language.data(), AS(int, run.language.size())));
      hb_buffer_add_utf8(font.hbscratch_buffer, subword.text.begin(), AS(int, subword.text.size()), 0, AS(int, subword.text.size()));

      hb_shape(font.hbfont, font.hbscratch_buffer, shaping_features, AS(uint, std::size(shaping_features)));

      uint             nglyphs;
      hb_glyph_info_t *glyph_info = hb_buffer_get_glyph_infos(font.hbscratch_buffer, &nglyphs);

      f32 font_scale = run.style.font_height / atlas.font_height;

      f32 width = 0;

      subword.glyph_start = glyphs.size();
      subword.nglyphs     = nglyphs;

      for (usize i = 0; i < nglyphs; i++)
      {
        u32       glyph_index = glyph_info[i].codepoint;
        stx::Span glyph       = atlas.get(glyph_index);

        // TODO(lamarrr): we seem to be getting an extra glyph index 0
        if (!glyph.is_empty())
        {
          width += glyph[0].advance.x * font_scale + run.style.letter_spacing;
          glyphs.push(FontGlyph{.font = run.font, .glyph = glyph_index}).unwrap();
        }
        else
        {
          width += atlas.glyphs[0].advance.x * font_scale + run.style.letter_spacing;
          glyphs.push(FontGlyph{.font = run.font, .glyph = 0}).unwrap();
        }
      }

      subword.width = width;
    }

    // perform wrapping of text
    {
      f32 cursor_x = 0;

      for (RunSubWord *iter = subwords.begin(); iter < subwords.end();)
      {
        RunSubWord *subword = iter;

        for (; subword < subwords.end();)
        {
          f32 spaced_word_width = subword->width + subword->nspaces * paragraph.runs[subword->run].style.word_spacing;

          // if end of word
          if (subword->nspaces > 0 || subword->nline_breaks > 0 || subword == subwords.end() - 1)
          {
            // check if wrapping needed
            if (cursor_x + spaced_word_width > max_line_width)
            {
              iter->is_wrapped = true;
              cursor_x         = spaced_word_width;
              if (subword->nline_breaks > 0)
              {
                cursor_x = 0;
              }
            }
            else
            {
              if (subword->nline_breaks > 0)
              {
                cursor_x = 0;
              }
              else
              {
                cursor_x += spaced_word_width;
              }
            }
            subword++;
            break;
          }
          else
          {
            // continue until we reach end of word
            cursor_x += spaced_word_width;
            subword++;
          }
        }

        iter = subword;
      }
    }

    // add new line to wraps if there's no previous newline
    {
      f32   baseline          = 0;
      usize nprev_line_breaks = 0;

      for (RunSubWord const *iter = subwords.begin(); iter < subwords.end();)
      {
        RunSubWord const *line_begin   = iter;
        RunSubWord const *line_end     = iter + 1;
        usize             nline_breaks = 0;

        if (line_begin->is_wrapped && nprev_line_breaks == 0)
        {
          nprev_line_breaks = 1;
        }

        if (line_begin->nline_breaks == 0)
        {
          for (; line_end < subwords.end();)
          {
            if (line_end->nline_breaks > 0)
            {
              nline_breaks = line_end->nline_breaks;
              line_end++;
              break;
            }
            else if (line_end->is_wrapped)
            {
              break;
            }
            else
            {
              line_end++;
            }
          }
        }

        f32 line_width  = 0;
        f32 line_height = 0;
        f32 max_ascent  = 0;

        for (RunSubWord const *subword = line_begin; subword < line_end; subword++)
        {
          line_width += subword->width + subword->nspaces * paragraph.runs[subword->run].style.word_spacing;
          line_height = std::max(line_height, paragraph.runs[subword->run].style.line_height * paragraph.runs[subword->run].style.font_height);

          TextRun const   &run        = paragraph.runs[subword->run];
          FontAtlas const &atlas      = atlases[run.font];
          f32              font_scale = run.style.font_height / atlas.font_height;

          for (FontGlyph const &glyph : glyphs.span().slice(subword->glyph_start, subword->nglyphs))
          {
            max_ascent = std::max(max_ascent, atlas.glyphs[glyph.glyph].ascent * font_scale);
          }
        }

        // TODO(lamarrr): add alignment
        f32 vert_spacing = std::max(line_height - max_ascent, 0.0f) / 2;
        f32 alignment    = 0;

        if (paragraph.align == TextAlign::Center)
        {
          alignment = (std::max(line_width, max_line_width) - line_width) / 2;
        }
        else if (paragraph.align == TextAlign::Right)
        {
          alignment = std::max(line_width, max_line_width) - line_width;
        }

        baseline += nprev_line_breaks * line_height;

        f32 cursor_x = 0;

        for (RunSubWord const *subword = line_begin; subword < line_end;)
        {
          if (paragraph.runs[subword->run].direction == TextDirection::LeftToRight)
          {
            TextRun const   &run   = paragraph.runs[subword->run];
            FontAtlas const &atlas = atlases[run.font];
            Font const      &font  = fonts[run.font];

            f32 font_scale     = run.style.font_height / atlas.font_height;
            f32 letter_spacing = run.style.letter_spacing;
            f32 word_spacing   = run.style.word_spacing;
            f32 init_cursor_x  = cursor_x;

            for (FontGlyph const &glyph : glyphs.span().slice(subword->glyph_start, subword->nglyphs))
            {
              Glyph const &g       = atlas.glyphs[glyph.glyph];
              vec2         advance = g.advance * font_scale;
              draw_glyph(g, run, atlas.texture, position + vec2{cursor_x, baseline}, font_scale, line_height, vert_spacing, font.has_color);
              cursor_x += advance.x + letter_spacing;
            }

            if (run.style.background_color.is_visible() && subword->nspaces > 0)
            {
              save();
              brush.color = run.style.background_color;
              brush.fill  = true;
              draw_rect(rect{.offset = position + vec2{cursor_x, baseline - line_height}, .extent = vec2{word_spacing * subword->nspaces, line_height}});
              restore();
            }

            if (run.style.underline_color.is_visible())
            {
              save();
              brush.color = run.style.underline_color;
              brush.fill  = true;
              draw_rect(rect{.offset = position + vec2{init_cursor_x, baseline}, .extent = vec2{subword->width + subword->nspaces * word_spacing, run.style.underline_thickness}});
              restore();
            }

            // TODO(lamarrr): implement strikethrough

            cursor_x += subword->nspaces * word_spacing;
            subword++;
          }
          else
          {
            f32               rtl_width = 0;
            RunSubWord const *rtl_begin = subword;
            RunSubWord const *rtl_end   = subword + 1;

            rtl_width += rtl_begin->width + rtl_begin->nspaces * paragraph.runs[rtl_begin->run].style.word_spacing;

            for (; rtl_end < line_end; rtl_end++)
            {
              if (paragraph.runs[rtl_end->run].direction == TextDirection::LeftToRight)
              {
                break;
              }
              else
              {
                rtl_width += rtl_end->width + rtl_end->nspaces * paragraph.runs[rtl_end->run].style.word_spacing;
              }
            }

            f32 rtl_cursor_x = cursor_x + rtl_width;

            for (RunSubWord const *rtl_iter = rtl_begin; rtl_iter < rtl_end; rtl_iter++)
            {
              TextRun const   &run   = paragraph.runs[rtl_iter->run];
              FontAtlas const &atlas = atlases[run.font];
              Font const      &font  = fonts[run.font];

              f32 font_scale     = run.style.font_height / atlas.font_height;
              f32 letter_spacing = run.style.letter_spacing;
              f32 spacing        = rtl_iter->nspaces * run.style.word_spacing;
              rtl_cursor_x -= spacing;

              if (run.style.background_color.is_visible() && rtl_iter->nspaces > 0)
              {
                save();
                brush.color = run.style.background_color;
                brush.fill  = true;
                draw_rect(rect{.offset = position + vec2{rtl_cursor_x, baseline - line_height}, .extent = vec2{spacing, line_height}});
                restore();
              }

              if (run.style.background_color.is_visible() && rtl_iter->nspaces > 0)
              {
                save();
                brush.color = run.style.background_color;
                brush.fill  = true;
                draw_rect(rect{.offset = position + vec2{rtl_cursor_x, baseline - line_height}, .extent = vec2{spacing, line_height}});
                restore();
              }

              rtl_cursor_x -= rtl_iter->width;

              f32 glyph_cursor_x = rtl_cursor_x;

              for (FontGlyph const &glyph : glyphs.span().slice(rtl_iter->glyph_start, rtl_iter->nglyphs))
              {
                Glyph const &g       = atlas.glyphs[glyph.glyph];
                vec2         advance = g.advance * font_scale;
                draw_glyph(g, run, atlas.texture, position + vec2{glyph_cursor_x, baseline}, font_scale, line_height, vert_spacing, font.has_color);
                glyph_cursor_x += advance.x + letter_spacing;
              }

              if (run.style.underline_color.is_visible())
              {
                save();
                brush.color = run.style.underline_color;
                brush.fill  = true;
                draw_rect(rect{.offset = position + vec2{rtl_cursor_x, baseline}, .extent = vec2{rtl_iter->width + spacing, run.style.underline_thickness}});
                restore();
              }
            }

            cursor_x += rtl_width;
            subword = rtl_end;
          }
        }

        nprev_line_breaks = nline_breaks;
        iter              = line_end;
      }
    }
  }

  void calculate_constraint(Paragraph paragraph, stx::Span<Font const> fonts, stx::Span<FontAtlas const> atlases)
  {
    vec2 min              = layout(paragraph, fonts, atlases, 0);
    vec2 max              = layout(paragraph, fonts, atlases, stx::F32_MAX / 2);
    constraint.min_width  = min.x;
    constraint.min_height = max.y;
    constraint.max_width  = max.y;
    constraint.max_height = min.x;
  }
};

}        // namespace gfx
}        // namespace ash
