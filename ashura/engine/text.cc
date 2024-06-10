#include "ashura/engine/text.h"
#include "ashura/std/vec.h"

extern "C"
{
#include "SBAlgorithm.h"
#include "SBLine.h"
#include "SBParagraph.h"
#include "SBScriptLocator.h"
#include "harfbuzz/hb.h"
}

namespace ash
{

/// layout is output in PT_UNIT units. so it is independent of the actual
/// font-height and can be cached as necessary. text must have been sanitized
/// with invalid codepoints replaced before calling this.
///
static void shape(hb_font_t *font, hb_buffer_t *buffer, Span<u32 const> text,
                  u32 first, u32 count, hb_script_t script,
                  hb_direction_t direction, hb_language_t language,
                  bool use_kerning, bool use_ligatures,
                  Span<hb_glyph_info_t const>     &infos,
                  Span<hb_glyph_position_t const> &positions)
{
  // tags are opentype feature tags
  hb_feature_t const shaping_features[] = {
      // kerning operations
      {.tag   = HB_TAG('k', 'e', 'r', 'n'),
       .value = use_kerning,
       .start = HB_FEATURE_GLOBAL_START,
       .end   = HB_FEATURE_GLOBAL_END},
      // standard ligature glyph substitution
      {.tag   = HB_TAG('l', 'i', 'g', 'a'),
       .value = use_ligatures,
       .start = HB_FEATURE_GLOBAL_START,
       .end   = HB_FEATURE_GLOBAL_END},
      // contextual ligature glyph substitution
      {.tag   = HB_TAG('c', 'l', 'i', 'g'),
       .value = use_ligatures,
       .start = HB_FEATURE_GLOBAL_START,
       .end   = HB_FEATURE_GLOBAL_END}};

  hb_buffer_reset(buffer);
  // invalid character replacement
  hb_buffer_set_replacement_codepoint(buffer,
                                      HB_BUFFER_REPLACEMENT_CODEPOINT_DEFAULT);
  hb_buffer_set_script(buffer, script);
  hb_buffer_set_direction(buffer, direction);
  // OpenType BCP-47 language tag specifying locale-sensitive shaping operations
  // as defined in the font
  hb_buffer_set_language(buffer, language);
  hb_buffer_add_codepoints(buffer, text.data(), (i32) text.size(), first,
                           (i32) count);
  hb_shape_full(font, buffer, shaping_features, (u32) size(shaping_features),
                nullptr);

  u32                        num_pos;
  hb_glyph_position_t const *glyph_pos =
      hb_buffer_get_glyph_positions(buffer, &num_pos);
  CHECK(!(glyph_pos == nullptr && num_pos > 0));

  u32                    num_info;
  hb_glyph_info_t const *glyph_info =
      hb_buffer_get_glyph_infos(buffer, &num_info);
  CHECK(!(glyph_info == nullptr && num_info > 0));

  CHECK(num_pos == num_info);

  infos     = Span{glyph_info, num_info};
  positions = Span{glyph_pos, num_pos};
}

/// @brief only needs to be called if it contains multiple paragraphs
static void segment_paragraphs(Span<u32 const> text, Span<TextSegment> segments)
{
  u32 const text_size = (u32) text.size();
  for (u32 i = 0; i < text_size;)
  {
    segments[i].paragraph = true;
    i++;
    while (i < text_size)
    {
      if (text[i] == '\r' && ((i + 1) < text_size) && text[i + 1] == '\n')
      {
        i += 2;
        break;
      }
      else if (text[i] == '\n' || text[i] == '\r')
      {
        i++;
        break;
      }
      i++;
    }
  }
}

/// @brief only needs to be called if it contains multiple scripts
/// outputs iso15924 or OpenType tags
static void segment_scripts(Span<u32 const> text, Span<TextSegment> segments)
{
  SBCodepointSequence codepoints{.stringEncoding = SBStringEncodingUTF32,
                                 .stringBuffer   = (void *) text.data(),
                                 .stringLength   = text.size()};

  SBScriptLocatorRef locator = SBScriptLocatorCreate();
  CHECK(locator != nullptr);
  SBScriptLocatorLoadCodepoints(locator, &codepoints);

  SBScriptAgent const *agent = SBScriptLocatorGetAgent(locator);
  CHECK(agent != nullptr);

  while (SBScriptLocatorMoveNext(locator) == SBTrue)
  {
    u32         beg = (u32) agent->offset;
    u32         end = beg + (u32) agent->length;
    hb_script_t script =
        hb_script_from_iso15924_tag(SBScriptGetOpenTypeTag(agent->script));
    for (u32 i = beg; i < end; i++)
    {
      segments[i].script = script;
    }
  }

  SBScriptLocatorRelease(locator);
}

/// @brief only needs to be called if it is a bidirectional text
static void segment_directions(Span<u32 const> text, SBAlgorithmRef algorithm,
                               TextDirection base, Span<TextSegment> segments)
{
  // The embedding level is an integer value. LTR text segments have even
  // embedding levels (e.g., 0, 2, 4), and RTL text segments have odd embedding
  // levels (e.g., 1, 3, 5).
  u32 const text_size = (u32) text.size();
  for (u32 i = 0; i < text_size;)
  {
    u32 begin = i++;
    while (i < text_size && !segments[i].paragraph)
    {
      i++;
    }

    u32 const      length = i - begin;
    SBParagraphRef paragraph =
        SBAlgorithmCreateParagraph(algorithm, begin, length, (SBLevel) base);
    CHECK(paragraph != nullptr);
    CHECK(SBParagraphGetLength(paragraph) == length);
    SBLevel const       base_level     = SBParagraphGetBaseLevel(paragraph);
    TextDirection const base_direction = TextDirection{(u8) (base_level & 0x1)};
    SBLevel const      *levels         = SBParagraphGetLevelsPtr(paragraph);
    CHECK(levels != nullptr);
    for (u32 idx = begin; idx < i; idx++)
    {
      SBLevel const       level     = levels[idx];
      TextDirection const direction = TextDirection{(u8) (levels[idx] & 0x1)};
      segments[idx].base_direction  = base_direction;
      segments[idx].direction       = direction;
    }
    SBParagraphRelease(paragraph);
  }
}

/// @brief only needs to be called if line breaking is required.
static void segment_breakpoints(Span<u32 const>   text,
                                Span<TextSegment> segments, f32 max_width)
{
  if (max_width == F32_MAX)
  {
    return;
  }

  u32 const text_size = (u32) text.size();
  for (u32 i = 0; i < text_size;)
  {
    segments[i].breakable = true;
    i++;
    while (i < text_size && text[i] != ' ' && text[i] != '\t')
    {
      i++;
    }

    while (i < text_size && (text[i] == ' ' || text[i] == '\t'))
    {
      i++;
    }
  }
}

// shape each run at each break opportunity
//
//
// is there a way we can perform the text shaping in batches without knowing the
// size of the text?
//
// we can layout segments independent of actual font size
//
//
void layout_text(TextBlock const &block, f32 max_width, TextLayout &layout)
{
  CHECK(block.text.size() < I32_MAX);
  CHECK(is_segments_valid(to_span(block.segments), (u32) block.text.size()));

  layout.max_width = max_width;
  layout.extent    = Vec2{0, 0};

  hb_language_t language =
      block.language.is_empty() ?
          hb_language_get_default() :
          hb_language_from_string(block.language.data(),
                                  (i32) block.language.size());

  hb_buffer_t *buffer = hb_buffer_create();
  CHECK(buffer != nullptr);
  defer buffer_del{[&] { hb_buffer_destroy(buffer); }};

  SBCodepointSequence codepoints{.stringEncoding = SBStringEncodingUTF32,
                                 .stringBuffer   = (void *) block.text.data(),
                                 .stringLength   = (u32) block.text.size()};
  SBAlgorithmRef      algorithm = SBAlgorithmCreate(&codepoints);
  CHECK(algorithm != nullptr);
  defer algorithm_del{[&] { SBAlgorithmRelease(algorithm); }};

  CHECK(layout.segments.resize_defaulted(block.text.size()));

  for (TextSegment &s : layout.segments)
  {
    s = TextSegment{};
  }

  segment_paragraphs(block.text, layout.segments);
  segment_scripts(block.text, layout.segments);
  segment_directions(block.text, algorithm, block.direction, layout.segments);
  segment_breakpoints(block.text, layout.segments, max_width);

  TextSegment                    *s;
  Span<hb_glyph_info_t const>     infos;
  Span<hb_glyph_position_t const> positions;
  u32                             beg = 0;
  u32                             end = 0;
  shape(nullptr, buffer, block.text, beg, end - beg, (hb_script_t) s->script,
        (s->direction == TextDirection::LeftToRight) ? HB_DIRECTION_LTR :
                                                       HB_DIRECTION_RTL,
        language, false, true, infos, positions);
  // iter runs and shape
  //
  // for each run: shape, advance
  //
  // line breaks?
  //
  // break into lines based on max width
  // what about visual reordering? of segments on lines?

  // shape each segment independently, break the lines as necessary then
  // re-order based on base direction and then reverse using segment direction.
}

}        // namespace ash
