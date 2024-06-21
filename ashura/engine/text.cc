#include "ashura/engine/text.h"
#include "ashura/engine/font_impl.h"
#include "ashura/std/vec.h"

extern "C"
{
#include "SBAlgorithm.h"
#include "SBLine.h"
#include "SBParagraph.h"
#include "SBScriptLocator.h"
#include "hb.h"
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
    for (u32 i = 0; i < (u32) agent->length; i++)
    {
      segments[agent->offset + i].script = TextScript{agent->script};
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
    u32 first = i++;
    while (i < text_size && !segments[i].paragraph)
    {
      i++;
    }

    u32 const      length    = i - first;
    SBParagraphRef paragraph = SBAlgorithmCreateParagraph(
        algorithm, first, length,
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
    for (u32 idx = first; idx < i; idx++)
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
                       u16 style, FontMetrics const &font_metrics,
                       TextDirection direction, TextDirection base_direction,
                       bool paragraph, bool breakable,
                       Span<hb_glyph_info_t const>     infos,
                       Span<hb_glyph_position_t const> positions)
{
  u32 num_glyphs  = (u32) infos.size();
  u32 first_glyph = (u32) l.glyphs.size();
  i32 advance     = 0;

  for (u32 i = 0; i < num_glyphs; i++)
  {
    hb_glyph_info_t const     &info = infos[i];
    hb_glyph_position_t const &pos  = positions[i];
    GlyphShape                 g{.glyph   = info.codepoint,
                                 .cluster = info.cluster,
                                 .advance = {pos.x_advance, pos.y_advance},
                                 .offset  = {pos.x_offset, pos.y_offset}};
    CHECK(l.glyphs.push(g));

    advance += pos.x_advance;
  }

  CHECK(l.runs.push(
      TextRun{.first          = first,
              .count          = count,
              .style          = style,
              .font_height    = s.font_height,
              .line_height    = s.line_height * s.font_height,
              .first_glyph    = first_glyph,
              .num_glyphs     = num_glyphs,
              .metrics        = TextRunMetrics{.advance = advance,
                                               .ascent  = font_metrics.ascent,
                                               .descent = font_metrics.descent},
              .base_direction = base_direction,
              .direction      = direction,
              .paragraph      = paragraph,
              .breakable      = breakable}));
}

/// see:
/// https://stackoverflow.com/questions/62374506/how-do-i-align-glyphs-along-the-baseline-with-freetype
///
void layout_text(TextBlock const &block, f32 max_width, TextLayout &layout)
{
  layout.clear();
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
    u32 prev_run_end = 0;
    for (u32 irun = 0; irun < (u32) block.runs.size(); irun++)
    {
      u32 const run_end = min(block.runs[irun], text_size);
      CHECK(prev_run_end <= block.text.size());
      CHECK(prev_run_end <= run_end);
      for (u32 i = prev_run_end; i < run_end; i++)
      {
        segments[i].style = irun;
      }
      prev_run_end = run_end;
    }
  }

  layout.max_width = max_width;

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
    u32 const          first   = i++;
    TextSegment const &segment = segments[first];
    while (i < text_size && segment.style == segments[i].style &&
           segment.script == segments[i].script && !segments[i].paragraph &&
           segment.direction == segments[i].direction && !segments[i].breakable)
    {
      i++;
    }

    FontStyle const                &s         = block.fonts[segment.style];
    FontImpl const                 *f         = (FontImpl const *) s.font;
    Span<hb_glyph_info_t const>     infos     = {};
    Span<hb_glyph_position_t const> positions = {};
    shape(f->hb_font, buffer, block.text, first, i - first,
          hb_script_from_iso15924_tag(
              SBScriptGetOpenTypeTag(SBScript{(u8) segment.script})),
          (segment.direction == TextDirection::LeftToRight) ? HB_DIRECTION_LTR :
                                                              HB_DIRECTION_RTL,
          language, block.use_kerning, block.use_ligatures, infos, positions);

    insert_run(layout, s, first, i - first, segment.style, f->metrics,
               segment.direction, segment.base_direction, segment.paragraph,
               segment.breakable, infos, positions);
  }

  u32 const num_runs = (u32) layout.runs.size();
  for (u32 i = 0; i < num_runs;)
  {
    u32 const           first          = i++;
    TextRun const      &first_run      = layout.runs[first];
    TextDirection const base_direction = first_run.base_direction;
    bool const          paragraph      = first_run.paragraph;
    f32 width   = pt_to_px(first_run.metrics.advance, first_run.font_height);
    f32 ascent  = pt_to_px(first_run.metrics.ascent, first_run.font_height);
    f32 descent = pt_to_px(first_run.metrics.descent, first_run.font_height);
    f32 line_height = first_run.line_height;

    while (
        i < num_runs && !layout.runs[i].paragraph &&
        !(layout.runs[i].breakable && (pt_to_px(layout.runs[i].metrics.advance,
                                                layout.runs[i].font_height) +
                                       width) > max_width))
    {
      TextRun const        &r = layout.runs[i];
      TextRunMetrics const &m = r.metrics;
      width += pt_to_px(m.advance, r.font_height);
      ascent      = max(ascent, pt_to_px(m.ascent, r.font_height));
      descent     = max(descent, pt_to_px(m.descent, r.font_height));
      line_height = max(line_height, r.line_height);
      i++;
    }

    Line line{.first_run = first,
              .num_runs  = (i - first),
              .metrics   = LineMetrics{.line_height    = line_height,
                                       .ascent         = ascent,
                                       .descent        = descent,
                                       .width          = width,
                                       .base_direction = base_direction},
              .paragraph = paragraph};

    CHECK(layout.lines.push(line));
  }
}

}        // namespace ash
