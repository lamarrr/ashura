#include "ashura/engine/text.h"
#include "ashura/std/error.h"
extern "C"
{
#include "SBAlgorithm.h"
#include "SBLine.h"
#include "SBParagraph.h"
#include "SBScriptLocator.h"
}

#include "harfbuzz/hb.h"

namespace ash
{

static void shape_text_hb(Font const &font, hb_font_t *hb_font,
                          f32 render_font_height, u32 not_found_glyph,
                          Span<u32 const> text, hb_buffer_t *shaping_buffer,
                          hb_script_t script, hb_direction_t direction,
                          hb_language_t language, bool use_kerning,
                          bool                             use_ligatures,
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

  hb_buffer_reset(shaping_buffer);
  // invalid character replacement
  hb_buffer_set_replacement_codepoint(shaping_buffer,
                                      HB_BUFFER_REPLACEMENT_CODEPOINT_DEFAULT);
  // default glyphs for characters without defined glyphs
  hb_buffer_set_not_found_glyph(shaping_buffer, not_found_glyph);
  // OpenType (ISO15924) Script Tag. See:
  // https://unicode.org/reports/tr24/#Relation_To_ISO15924
  hb_buffer_set_script(shaping_buffer, script);
  hb_buffer_set_direction(shaping_buffer, direction);
  // OpenType BCP-47 language tag for performing certain shaping operations as
  // defined in the font
  hb_buffer_set_language(shaping_buffer, language);
  hb_font_set_scale(hb_font, (int) (64 * render_font_height),
                    (int) (64 * render_font_height));

  // https://harfbuzz.github.io/harfbuzz-hb-buffer.html#hb-buffer-add-codepoints
  // When shaping part of a larger text (e.g. a run of text from a paragraph),
  // instead of passing just the substring corresponding to the run, it is
  // preferable to pass the whole paragraph and specify the run start and length
  // as item_offset and item_length , respectively, to give HarfBuzz the full
  // context to be able, for example, to do cross-run Arabic shaping or properly
  // handle combining marks at stat of run.
  hb_buffer_add_utf32(shaping_buffer, text.data(), (int) text.size(), 0,
                      (int) text.size());
  hb_shape(hb_font, shaping_buffer, shaping_features,
           (unsigned int) size(shaping_features));

  u32                        nglyph_pos;
  hb_glyph_position_t const *glyph_pos =
      hb_buffer_get_glyph_positions(shaping_buffer, &nglyph_pos);
  CHECK(!(glyph_pos == nullptr && nglyph_pos > 0));

  u32                    nglyph_infos;
  hb_glyph_info_t const *glyph_info =
      hb_buffer_get_glyph_infos(shaping_buffer, &nglyph_infos);
  CHECK(!(glyph_info == nullptr && nglyph_infos > 0));

  CHECK(nglyph_pos == nglyph_infos);

  infos     = Span{glyph_info, nglyph_infos};
  positions = Span{glyph_pos, nglyph_pos};
}

// TODO(lamarrr): remove all uses of char for utf8 text or byte strings
void layout(TextBlock const &block, f32 text_scale_factor, f32 max_line_width,
            TextLayout &layout)
{
  lines.clear();
  run_segments.clear();
  glyph_shapings.clear();

  this->max_line_width    = max_line_width;
  this->text_scale_factor = text_scale_factor;
  span                    = Vec2{0, 0};

  // uses setLocale to get default language from locale which isn't
  // thread-safe
  hb_language_t language_hb =
      block.language.is_empty()() ?
          hb_language_get_default() :
          hb_language_from_string(block.language.data(),
                                  (int) block.language.size());

  hb_buffer_t *const shaping_buffer = hb_buffer_create();
  CHECK(shaping_buffer != nullptr);

  SBCodepointSequence codepoint_sequence{
      .stringEncoding = SBStringEncodingUTF32,
      .stringBuffer   = (void *) block.text.data(),
      .stringLength   = block.text.size()};

  SBAlgorithmRef algorithm = SBAlgorithmCreate(&codepoint_sequence);
  CHECK(algorithm != nullptr);

  SBScriptLocatorRef script_locator = SBScriptLocatorCreate();
  CHECK(script_locator != nullptr);

  SBScriptLocatorLoadCodepoints(script_locator, &codepoint_sequence);

  SBScriptAgent const *script_agent = SBScriptLocatorGetAgent(script_locator);
  CHECK(script_agent != nullptr);

  SBScriptLocatorMoveNext(script_locator);

  TextRun const *run_it            = block.runs.begin();
  usize          style_text_offset = 0;

  /// Paragraph Breaking ///
  for (SBUInteger paragraph_begin = 0; paragraph_begin < block.text.size();)
  {
    SBParagraphRef const sb_paragraph = SBAlgorithmCreateParagraph(
        algorithm, paragraph_begin, UINTPTR_MAX,
        block.direction == TextDirection::LeftToRight ?
            (SBLevel) SBLevelDefaultLTR :
            (SBLevel) SBLevelDefaultRTL);
    CHECK(sb_paragraph != nullptr);
    // number of bytes of the paragraph
    SBUInteger paragraph_length = SBParagraphGetLength(sb_paragraph);
    // base direction, 0 - LTR, 1 - RTL
    SBLevel paragraph_base_level = SBParagraphGetBaseLevel(sb_paragraph);
    // SheenBidi expands to byte level representation of the codepoints
    SBLevel const *paragraph_levels     = SBParagraphGetLevelsPtr(sb_paragraph);
    char const    *paragraph_text_begin = block.text.data() + paragraph_begin;
    char const    *paragraph_text_end = paragraph_text_begin + paragraph_length;
    usize          paragraph_first_segment = run_segments.size();

    for (char const *run_text_begin = paragraph_text_begin;
         run_text_begin < paragraph_text_end;)
    {
      char const *run_text_end = run_text_begin;
      usize       run_block_text_offset =
          (usize) (run_text_begin - block.text.data());
      SBLevel run_level =
          paragraph_levels[run_text_begin - paragraph_text_begin];
      utf8_next((u8 const *&) run_text_end);
      while (!(run_block_text_offset >= script_agent->offset &&
               run_block_text_offset <
                   (script_agent->offset + script_agent->length)))
      {
        CHECK(SBScriptLocatorMoveNext(script_locator));
      }
      SBScript       run_script    = script_agent->script;
      TextDirection  run_direction = (run_level & 0x1) == 0 ?
                                         TextDirection::LeftToRight :
                                         TextDirection::RightToLeft;
      hb_direction_t run_direction_hb =
          (run_level & 0x1) == 0 ? HB_DIRECTION_LTR : HB_DIRECTION_RTL;
      // Note that unicode scripts are different from OpenType (iso15924)
      // scripts though they are similar
      hb_script_t run_script_hb =
          hb_script_from_iso15924_tag(SBScriptGetOpenTypeTag(run_script));
      // find the style intended for this text run (if any, otherwise default)
      usize irun_style = block.styles.size();

      while (run_it < block.runs.end())
      {
        if (run_block_text_offset >= style_text_offset &&
            run_block_text_offset < (style_text_offset + run_it->size))

        {
          irun_style = run_it->style;
          break;
        }
        else
        {
          style_text_offset += run_it->size;
          run_it++;
        }
      }

      TextStyle const &run_style = irun_style >= block.styles.size() ?
                                       block.default_style :
                                       block.styles[irun_style];
      usize            run_font  = resolved_fonts[irun_style];

      // find the last codepoint that belongs to this text run
      while (run_text_end < paragraph_text_end)
      {
        usize   block_text_offset = (usize) (run_text_end - block.text.data());
        SBLevel level = paragraph_levels[run_text_end - paragraph_text_begin];
        char const *p_next_codepoint = run_text_end;
        utf8_next((u8 const *&) p_next_codepoint);
        // find the style intended for this code point (if any, otherwise
        // default)
        usize istyle = block.styles.size();
        while (run_it < block.runs.end())
        {
          if (block_text_offset >= style_text_offset &&
              block_text_offset < (style_text_offset + run_it->size))
          {
            istyle = run_it->style;
            break;
          }
          else
          {
            style_text_offset += run_it->size;
            run_it++;
          }
        }

        bool is_in_script_run =
            (block_text_offset >= script_agent->offset) &&
            (block_text_offset < (script_agent->offset + script_agent->length));

        if (level != run_level || !is_in_script_run || istyle != irun_style)

        {
          // reached end of run
          break;
        }

        // make retrieved codepoint part of run, then advance iterator
        run_text_end = p_next_codepoint;
      }

      /// Text Segmentation ///
      for (char const *segment_text_begin = run_text_begin;
           segment_text_begin < run_text_end;)
      {
        char const *segment_text_end = segment_text_begin;
        bool        has_spacing      = false;

        while (segment_text_end < run_text_end)
        {
          char const *p_next_codepoint = segment_text_end;
          SBCodepoint codepoint = utf8_next((u8 const *&) p_next_codepoint);

          if (codepoint == ' ' || codepoint == '\t')
          {
            // don't break immediately so we can batch multiple tabs/spaces
            // and have them on the same line during line breaking
            has_spacing = true;
          }
          else if (has_spacing)        // has a preceding spacing character
          {
            break;
          }

          segment_text_end = p_next_codepoint;
        }

        Span segment_text{segment_text_begin,
                          (usize) (segment_text_end - segment_text_begin)};

        hb_buffer_reset(shaping_buffer);
        auto [glyph_infos, glyph_positions] = shape_text_harfbuzz(
            *font_bundle[run_font].font,
            font_bundle[run_font].atlas.font_height,
            font_bundle[run_font].atlas.space_glyph, segment_text,
            shaping_buffer, run_script_hb, run_direction_hb, language_hb,
            run_style.use_kerning, run_style.use_ligatures);

        f32 scale = ((f32) run_style.font_height * text_scale_factor) /
                    (f32) font_bundle[run_font].atlas.font_height;

        f32 segment_width = 0;

        usize first_shaping = glyph_shapings.size();

        for (usize i = 0; i < glyph_infos.size(); i++)
        {
          f32  advance = scale * (f32) glyph_positions[i].x_advance / 64.0f;
          Vec2 offset  = Vec2{scale, scale} *
                        Vec2{(f32) glyph_positions[i].x_offset / 64.0f,
                             (f32) glyph_positions[i].y_offset / -64.0f};

          glyph_shapings
              .push(GlyphShaping{.glyph   = glyph_infos[i].codepoint,
                                 .cluster = glyph_infos[i].cluster,
                                 .advance = advance,
                                 .offset  = offset})
              .unwrap();

          segment_width +=
              advance + text_scale_factor * run_style.letter_spacing;
        }

        usize num_shapings = glyph_infos.size();

        segment_width +=
            has_spacing ? (text_scale_factor * run_style.word_spacing) : 0;

        run_segments
            .push(TextRunSegment{.has_spacing   = has_spacing,
                                 .text          = segment_text,
                                 .direction     = run_direction,
                                 .style         = irun_style,
                                 .font          = run_font,
                                 .first_shaping = first_shaping,
                                 .num_shapings  = num_shapings,
                                 .width         = segment_width})
            .unwrap();

        segment_text_begin = segment_text_end;
      }

      run_text_begin = run_text_end;
    }

    SBParagraphRelease(sb_paragraph);

    usize nparagraph_run_segments =
        run_segments.size() - paragraph_first_segment;
    TextRunSegment *paragraph_run_segments_begin =
        run_segments.data() + paragraph_first_segment;
    TextRunSegment *paragraph_run_segments_end =
        paragraph_run_segments_begin + nparagraph_run_segments;

    for (TextRunSegment *line_begin = paragraph_run_segments_begin;
         line_begin < paragraph_run_segments_end;)
    {
      TextRunSegment *line_end   = line_begin;
      f32             line_width = 0;

      while (line_end < paragraph_run_segments_end)
      {
        line_width += line_end->width;
        if (line_end->has_spacing)
        {
          line_end++;
          break;
        }
        else
        {
          line_end++;
        }
      }

      /// Line Breaking ///
      for (TextRunSegment *break_iter = line_end;
           break_iter < paragraph_run_segments_end;)
      {
        f32 word_width = 0;

        while (break_iter < paragraph_run_segments_end)
        {
          word_width += break_iter->width;
          if (break_iter->has_spacing)
          {
            break_iter++;
            break;
          }
          else
          {
            break_iter++;
          }
        }

        if (line_width + word_width > max_line_width)
        {
          break;
        }
        else
        {
          line_end = break_iter;
          line_width += word_width;
        }
      }

      /// Line Metrics and Visual Re-ordering of Text Segments ///
      f32 line_ascent  = 0;
      f32 line_descent = 0;
      f32 line_height  = 0;
      for (TextRunSegment *direction_run_begin = line_begin;
           direction_run_begin < line_end;)
      {
        TextDirection   direction         = direction_run_begin->direction;
        TextRunSegment *direction_run_end = direction_run_begin + 1;

        for (; direction_run_end < line_end; direction_run_end++)
        {
          if (direction_run_end->direction != direction)
          {
            break;
          }
        }

        if (direction == TextDirection::RightToLeft)
        {
          // re-order consecutive RTL segments on the line to match visual
          // reading direction
          Span{direction_run_begin,
               (usize) (direction_run_end - direction_run_begin)}
              .reverse();
        }

        for (TextRunSegment const &run_segment :
             Span{direction_run_begin,
                  (usize) (direction_run_end - direction_run_begin)})
        {
          TextStyle const &style  = run_segment.style >= block.styles.size() ?
                                        block.default_style :
                                        block.styles[run_segment.style];
          f32              ascent = text_scale_factor *
                       font_bundle[run_segment.font].atlas.ascent *
                       style.font_height;
          f32 descent = text_scale_factor *
                        font_bundle[run_segment.font].atlas.descent *
                        style.font_height;
          f32 height   = ascent + descent;
          line_height  = max(line_height, style.line_height * height);
          line_ascent  = max(line_ascent, ascent);
          line_descent = max(line_descent, descent);
        }

        direction_run_begin = direction_run_end;
      }

      lines
          .push_inplace(LineMetrics{
              .width          = line_width,
              .ascent         = line_ascent,
              .descent        = line_descent,
              .line_height    = line_height,
              .base_direction = (paragraph_base_level & 0x1) == 0 ?
                                    TextDirection::LeftToRight :
                                    TextDirection::RightToLeft,
              .first_segment  = (usize) (line_begin - run_segments.data()),
              .num_segments   = (usize) (line_end - line_begin)})
          .unwrap();

      span.y += line_height;
      span.x = max(span.x, line_width);

      line_begin = line_end;
    }

    paragraph_begin += paragraph_length;
  }

  SBAlgorithmRelease(algorithm);
  hb_buffer_destroy(shaping_buffer);
}
};
}
