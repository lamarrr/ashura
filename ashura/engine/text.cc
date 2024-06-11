#include "ashura/engine/text.h"
#include "ashura/engine/font_impl.h"
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
/// @param script OpenType (ISO15924) Script
/// Tag. See: https://unicode.org/reports/tr24/#Relation_To_ISO15924
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
  hb_shape(font, buffer, shaping_features, (u32) size(shaping_features));

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
    u32 beg = (u32) agent->offset;
    u32 end = beg + (u32) agent->length;
    for (u32 i = beg; i < end; i++)
    {
      segments[i].script = TextScript{agent->script};
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

    u32 const      length    = i - begin;
    SBParagraphRef paragraph = SBAlgorithmCreateParagraph(
        algorithm, begin, length,
        (base == TextDirection::LeftToRight) ? SBLevelDefaultLTR :
                                               SBLevelDefaultRTL);
    CHECK(paragraph != nullptr);
    CHECK(SBParagraphGetLength(paragraph) == length);
    SBLevel const       base_level     = SBParagraphGetBaseLevel(paragraph);
    TextDirection const base_direction = ((base_level & 0x1) == 0) ?
                                             TextDirection::LeftToRight :
                                             TextDirection::RightToLeft;
    SBLevel const      *levels         = SBParagraphGetLevelsPtr(paragraph);
    CHECK(levels != nullptr);
    for (u32 idx = begin; idx < i; idx++)
    {
      SBLevel const       level     = levels[idx];
      TextDirection const direction = ((level & 0x1) == 0) ?
                                          TextDirection::LeftToRight :
                                          TextDirection::RightToLeft;
      segments[idx].base_direction  = base_direction;
      segments[idx].direction       = direction;
    }
    SBParagraphRelease(paragraph);
  }
}

/// @brief only needs to be called if line breaking is required.
static void segment_breakpoints(Span<u32 const> text, f32 max_width,
                                Span<TextSegment> segments)
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

static void insert_run(TextLayout &l, FontStyle const &s, u32 first, u32 count,
                       u16 font, TextDirection direction, bool paragraph,
                       bool breakable, Span<hb_glyph_info_t const> infos,
                       Span<hb_glyph_position_t const> positions)
{
  CHECK(infos.size() == positions.size());
  u32 num_glyphs  = (u32) infos.size();
  u32 first_glyph = (u32) l.glyphs.size();
  f32 advance     = 0;
  u32 cluster     = U32_MAX;

  for (u32 i = 0; i < num_glyphs; i++)
  {
    GlyphShape g{.glyph   = infos[i].codepoint,
                 .cluster = infos[i].cluster,
                 .advance = {positions[i].x_advance, positions[i].y_advance},
                 .offset  = {positions[i].x_offset, positions[i].y_offset}};
    CHECK(l.glyphs.push(g));

    advance += (positions[i].x_advance / (f32) PT_UNIT) * s.font_height;

    if (cluster == U32_MAX)
    {
      cluster = infos[i].cluster;
    }
    if (infos[i].cluster != cluster)
    {
      advance += s.letter_spacing;
      cluster = infos[i].cluster;
    }
  }

  CHECK(l.runs.push(TextRun{.first       = first,
                            .count       = count,
                            .font        = font,
                            .first_glyph = first_glyph,
                            .num_glyphs  = num_glyphs,
                            .advance     = advance,
                            .direction   = direction,
                            .paragraph   = paragraph,
                            .breakable   = breakable}));
}

void layout_text(TextBlock const &block, f32 max_width, TextLayout &layout)
{
  u32 const text_size = (u32) block.text.size();
  CHECK(block.text.size() <= I32_MAX);
  CHECK(block.fonts.size() <= U16_MAX);
  CHECK(block.runs.size() == block.fonts.size());

  CHECK(layout.segments.resize_defaulted(block.text.size()));
  Span segments = to_span(layout.segments);

  for (TextSegment &s : segments)
  {
    s = TextSegment{};
  }

  {
    u32 run_begin = 0;
    for (u32 irun = 0; irun < (u32) block.runs.size(); irun++)
    {
      u32 const run_size = block.runs[irun];
      CHECK(run_begin < block.text.size());
      CHECK((run_begin + run_size) <= block.text.size());
      for (u32 i = run_begin; i < run_begin + run_size; i++)
      {
        segments[i].font = irun;
      }
      run_begin += run_size;
    }
  }

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
                                 .stringLength   = text_size};
  SBAlgorithmRef      algorithm = SBAlgorithmCreate(&codepoints);
  CHECK(algorithm != nullptr);
  defer algorithm_del{[&] { SBAlgorithmRelease(algorithm); }};

  segment_paragraphs(block.text, segments);
  segment_scripts(block.text, segments);
  segment_directions(block.text, algorithm, block.direction, segments);
  segment_breakpoints(block.text, max_width, segments);

  for (u32 i = 0; i < text_size;)
  {
    u32                begin   = i++;
    TextSegment const &segment = segments[begin];
    while (i < text_size && segment.font == segments[i].font &&
           segment.script == segments[i].script && !segment.paragraph &&
           segment.direction == segments[i].direction && segment.breakable)
    {
      i++;
    }

    FontStyle const &s = block.fonts[segment.font];
    FontImpl        *f = (FontImpl *) s.font;

    Span<hb_glyph_info_t const>     infos;
    Span<hb_glyph_position_t const> positions;
    shape(f->hb_font, buffer, block.text, begin, i - begin,
          hb_script_from_iso15924_tag(
              SBScriptGetOpenTypeTag(SBScript{(u8) segment.script})),
          (segment.direction == TextDirection::LeftToRight) ? HB_DIRECTION_LTR :
                                                              HB_DIRECTION_RTL,
          language, block.use_kerning, block.use_ligatures, infos, positions);

    insert_run(layout, s, begin, i - begin, segment.font, segment.direction,
               segment.paragraph, segment.breakable, infos, positions);
  }

  if (max_width != F32_MAX)
  {
    u32 const num_runs = (u32) layout.runs.size();
    for (u32 i = 0; i < num_runs;)
    {
      u32 begin = i++;
      while (i < num_runs && !layout.runs[i].paragraph)
      {
        i++;
      }
      // run paragraph, now layout...
    }
  }

  // go through each run, break into lines as necessary, finished.
  //

  // what about visual reordering? of segments on lines?
  // visual reordering is done in the renderer
}

}        // namespace ash
