#pragma once

#include <algorithm>
#include <string_view>

extern "C"
{
#include "SBAlgorithm.h"
#include "SBLine.h"
#include "SBParagraph.h"
#include "SBScriptLocator.h"
}

#include "ashura/font.h"
#include "ashura/primitives.h"
#include "ashura/unicode.h"

#include "harfbuzz/hb.h"
#include "stx/span.h"
#include "stx/text.h"
#include "stx/vec.h"

namespace ash
{

enum class TextDirection : u8
{
  LeftToRight,
  RightToLeft
};

struct TextStyle
{
  std::string_view font =
      {};        // name to use to match the font. if font is not found or empty
                 // the fallback fonts are tried.
  stx::Span<std::string_view const> fallback_fonts =
      {};        // font to fallback to if {font} is not available. if none of
                 // the specified fallback fonts are found the first font in the
                 // font bundle will be used
  f32   font_height       = 20;                   // px
  Color foreground_color  = colors::BLACK;        //
  Color outline_color     = colors::BLACK;        //
  f32   outline_thickness = 0;                    //
  Color shadow_color      = colors::BLACK;        //
  f32   shadow_scale      = 0;        // relative. multiplied by font_height
  Vec2  shadow_offset    = Vec2{0, 0};        // px. offset from center of glyph
  Color background_color = colors::TRANSPARENT;         //
  Color underline_color  = colors::BLACK;               //
  f32   underline_thickness     = 0;                    // px
  Color strikethrough_color     = colors::BLACK;        //
  f32   strikethrough_thickness = 0;                    // px
  f32   letter_spacing =
      0;        // px. additional letter spacing, can be negative
  f32  word_spacing = 0;        // px. additional word spacing, can be negative
  f32  line_height  = 1.2f;        // relative. multiplied by font_height
  bool use_kerning  = true;        // use provided font kerning
  bool use_ligatures =
      true;        // use standard and contextual font ligature substitution
};

/// A text run is a sequence of characters sharing a single property.
/// i.e. foreground color, font etc.
struct TextRun
{
  usize size = 0;        // byte size coverage of this run. i.e. for the first
                         // run with size 20 all text within [0, 20] bytes range
                         // of the text will be styled using this run
  usize style = 0;        // run style to use
};

enum class TextAlign : u8
{
  Start,
  Center,
  End
};

enum class TextOverflow
{
  Wrap,
  Ellipsis
};

struct TextBlock
{
  std::string_view text;        // utf-8-encoded text, Span because string view
                                // doesnt support non-string types
  stx::Span<TextRun const> runs;        // parts of text not styled by a run
                                        // will use the paragraphs run style
  stx::Span<TextStyle const>
                styles;               // styles for the text block's contents
  TextStyle     default_style;        // default run styling
  TextAlign     align = TextAlign::Start;        // text alignment
  TextDirection direction =
      TextDirection::LeftToRight;        // base text direction
  std::string_view language =
      {};        // base language to use for selecting opentype features to
                 // used on the text, uses default if not set
};

/// RunSegment is a part of a text run split by groups of spacing characters
/// word contained in a run. The spacing characters translate to break
/// opportunities.
struct TextRunSegment
{
  bool has_spacing =
      false;        // if it has trailing spacing characters (tabs and spaces)
                    // where we can break the text, this corresponds to the
                    // unicode Break-After (BA)
  stx::Span<char const> text = {};        // utf8 text of segment
  TextDirection         direction =
      TextDirection::LeftToRight;         // direction of text
  usize style                 = 0;        // resolved run text styling
  usize font                  = 0;        // resolved font index in font bundle
  usize glyph_shapings_offset = 0;
  usize nglyph_shapings       = 0;
  f32   width = 0;        // sum of advances, letter spacing & word spacing
};

struct LineMetrics
{
  f32 width       = 0;        // width of the line
  f32 ascent      = 0;        // maximum ascent of all the runs on the line
  f32 descent     = 0;        // maximum descent of all the runs on the line
  f32 line_height = 0;        // maximum line height of all the runs on the line
  TextDirection base_direction =
      TextDirection::LeftToRight;        // base direction of the line
  usize run_segments_offset = 0;         // begin index of line's segments
  usize nrun_segments       = 0;         // number of segments
};

struct GlyphShaping
{
  u32  glyph   = 0;
  u32  cluster = 0;
  f32  advance = 0;        // context-dependent horizontal-layout advance
  Vec2 offset;        // context-dependent text shaping offset from normal font
                      // glyph position
};

struct TextLayout
{
  stx::Vec<LineMetrics>    lines;
  stx::Vec<TextRunSegment> run_segments;
  stx::Vec<GlyphShaping>   glyph_shapings;
  f32                      max_line_width = 0;
  Vec2                     span;
  f32                      text_scale_factor = 0;

  static std::pair<stx::Span<hb_glyph_info_t const>,
                   stx::Span<hb_glyph_position_t const>>
      shape_text_harfbuzz(Font const &font, f32 render_font_height,
                          u32 not_found_glyph, stx::Span<char const> text,
                          hb_buffer_t *shaping_buffer, hb_script_t script,
                          hb_direction_t direction, hb_language_t language,
                          bool use_kerning, bool use_ligatures)
  {
    // tags are opentype feature tags
    hb_feature_t const shaping_features[] = {
        {.tag   = HB_TAG('k', 'e', 'r', 'n'),        // kerning operations
         .value = use_kerning,
         .start = HB_FEATURE_GLOBAL_START,
         .end   = HB_FEATURE_GLOBAL_END},
        {.tag   = HB_TAG('l', 'i', 'g',
                         'a'),        // standard ligature glyph substitution
         .value = use_ligatures,
         .start = HB_FEATURE_GLOBAL_START,
         .end   = HB_FEATURE_GLOBAL_END},
        {.tag   = HB_TAG('c', 'l', 'i',
                         'g'),        // contextual ligature glyph substitution
         .value = use_ligatures,
         .start = HB_FEATURE_GLOBAL_START,
         .end   = HB_FEATURE_GLOBAL_END}};

    hb_buffer_reset(shaping_buffer);
    hb_buffer_set_replacement_codepoint(
        shaping_buffer,
        HB_BUFFER_REPLACEMENT_CODEPOINT_DEFAULT);        // invalid character
                                                         // replacement
    hb_buffer_set_not_found_glyph(
        shaping_buffer,
        not_found_glyph);        // default glyphs for characters without
                                 // defined glyphs
    hb_buffer_set_script(
        shaping_buffer,
        script);        // OpenType (ISO15924) Script Tag. See:
                        // https://unicode.org/reports/tr24/#Relation_To_ISO15924
    hb_buffer_set_direction(shaping_buffer, direction);
    hb_buffer_set_language(
        shaping_buffer,
        language);        // OpenType BCP-47 language tag for performing certain
                          // shaping operations as defined in the font
    hb_font_set_scale(font.hb_font, (int) (64 * render_font_height),
                      (int) (64 * render_font_height));
    hb_buffer_add_utf8(shaping_buffer, text.data(), (int) text.size(), 0,
                       (int) text.size());
    hb_shape(font.hb_font, shaping_buffer, shaping_features,
             (uint) std::size(shaping_features));

    uint                       nglyph_pos;
    hb_glyph_position_t const *glyph_pos =
        hb_buffer_get_glyph_positions(shaping_buffer, &nglyph_pos);
    ASH_CHECK(!(glyph_pos == nullptr && nglyph_pos > 0));

    uint                   nglyph_infos;
    hb_glyph_info_t const *glyph_info =
        hb_buffer_get_glyph_infos(shaping_buffer, &nglyph_infos);
    ASH_CHECK(!(glyph_info == nullptr && nglyph_infos > 0));

    ASH_CHECK(nglyph_pos == nglyph_infos);

    return std::make_pair(stx::Span{glyph_info, nglyph_infos},
                          stx::Span{glyph_pos, nglyph_pos});
  }

  // TODO(lamarrr): remove all uses of char for utf8 text or byte strings
  void layout(TextBlock const &block, f32 text_scale_factor,
              stx::Span<BundledFont const> const font_bundle,
              f32 const                          max_line_width)
  {
    lines.clear();
    run_segments.clear();
    glyph_shapings.clear();

    this->max_line_width    = max_line_width;
    this->text_scale_factor = text_scale_factor;
    span                    = Vec2{0, 0};

    // there's no layout to perform without a font
    if (font_bundle.is_empty() || block.text.empty())
    {
      return;
    }

    // uses setLocale to get default language from locale which isn't
    // thread-safe
    hb_language_t const language_hb =
        block.language.empty() ?
            hb_language_get_default() :
            hb_language_from_string(block.language.data(),
                                    (int) block.language.size());

    hb_buffer_t *const shaping_buffer = hb_buffer_create();
    ASH_CHECK(shaping_buffer != nullptr);

    /// Text Style Font Resolution ///
    stx::Vec<usize> resolved_fonts;

    for (TextStyle const &style : block.styles)
    {
      usize font = match_font(style.font, style.fallback_fonts, font_bundle);
      // use first font in the font bundle if specified font and fallback fonts
      // are not found
      resolved_fonts.push(font < font_bundle.size() ? font : 0).unwrap();
    }

    {
      usize font = match_font(block.default_style.font,
                              block.default_style.fallback_fonts, font_bundle);
      resolved_fonts.push(font < font_bundle.size() ? font : 0).unwrap();
    }

    SBCodepointSequence const codepoint_sequence{
        .stringEncoding = SBStringEncodingUTF8,
        .stringBuffer   = (void *) block.text.data(),
        .stringLength   = block.text.size()};

    SBAlgorithmRef const algorithm = SBAlgorithmCreate(&codepoint_sequence);
    ASH_CHECK(algorithm != nullptr);

    SBScriptLocatorRef const script_locator = SBScriptLocatorCreate();
    ASH_CHECK(script_locator != nullptr);

    SBScriptLocatorLoadCodepoints(script_locator, &codepoint_sequence);

    SBScriptAgent const *script_agent = SBScriptLocatorGetAgent(script_locator);
    ASH_CHECK(script_agent != nullptr);

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
      ASH_CHECK(sb_paragraph != nullptr);
      SBUInteger const paragraph_length = SBParagraphGetLength(
          sb_paragraph);        // number of bytes of the paragraph
      SBLevel const paragraph_base_level = SBParagraphGetBaseLevel(
          sb_paragraph);        // base direction, 0 - LTR, 1 - RTL
      SBLevel const *const paragraph_levels = SBParagraphGetLevelsPtr(
          sb_paragraph);        // SheenBidi expands to byte level
                                // representation of the codepoints
      char const *const paragraph_text_begin =
          block.text.data() + paragraph_begin;
      char const *const paragraph_text_end =
          paragraph_text_begin + paragraph_length;
      usize const paragraph_run_segments_offset = run_segments.size();

      for (char const *run_text_begin = paragraph_text_begin;
           run_text_begin < paragraph_text_end;)
      {
        char const *run_text_end = run_text_begin;
        usize const run_block_text_offset =
            (usize) (run_text_begin - block.text.data());
        SBLevel const run_level =
            paragraph_levels[run_text_begin - paragraph_text_begin];
        stx::utf8_next((u8 const *&) run_text_end);
        while (!(run_block_text_offset >= script_agent->offset &&
                 run_block_text_offset < (script_agent->offset +
                                          script_agent->length))) [[unlikely]]
        {
          ASH_CHECK(SBScriptLocatorMoveNext(script_locator));
        }
        SBScript const       run_script    = script_agent->script;
        TextDirection const  run_direction = (run_level & 0x1) == 0 ?
                                                 TextDirection::LeftToRight :
                                                 TextDirection::RightToLeft;
        hb_direction_t const run_direction_hb =
            (run_level & 0x1) == 0 ? HB_DIRECTION_LTR : HB_DIRECTION_RTL;
        hb_script_t const run_script_hb =
            hb_script_from_iso15924_tag(SBScriptGetOpenTypeTag(
                run_script));        // Note that unicode scripts are different
                                     // from OpenType (iso15924) scripts though
                                     // they are similar
        usize irun_style =
            block.styles.size();        // find the style intended for this text
                                        // run (if any, otherwise default)

        while (run_it < block.runs.end())
        {
          if (run_block_text_offset >= style_text_offset &&
              run_block_text_offset < (style_text_offset + run_it->size))
              [[likely]]
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
        usize const      run_font  = resolved_fonts[irun_style];

        // find the last codepoint that belongs to this text run
        while (run_text_end < paragraph_text_end)
        {
          usize const block_text_offset =
              (usize) (run_text_end - block.text.data());
          SBLevel const level =
              paragraph_levels[run_text_end - paragraph_text_begin];
          char const *p_next_codepoint = run_text_end;
          stx::utf8_next((u8 const *&) p_next_codepoint);
          usize istyle =
              block.styles
                  .size();        // find the style intended for this code point
                                  // (if any, otherwise default)

          while (run_it < block.runs.end())
          {
            if (block_text_offset >= style_text_offset &&
                block_text_offset < (style_text_offset + run_it->size))
                [[likely]]
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

          bool const is_in_script_run =
              (block_text_offset >= script_agent->offset) &&
              (block_text_offset <
               (script_agent->offset + script_agent->length));

          if (level != run_level || !is_in_script_run || istyle != irun_style)
              [[unlikely]]
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
            char const       *p_next_codepoint = segment_text_end;
            SBCodepoint const codepoint =
                stx::utf8_next((u8 const *&) p_next_codepoint);

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

          stx::Span const segment_text{
              segment_text_begin,
              (usize) (segment_text_end - segment_text_begin)};

          auto const [glyph_infos, glyph_positions] = shape_text_harfbuzz(
              *font_bundle[run_font].font,
              font_bundle[run_font].atlas.font_height,
              font_bundle[run_font].atlas.space_glyph, segment_text,
              shaping_buffer, run_script_hb, run_direction_hb, language_hb,
              run_style.use_kerning, run_style.use_ligatures);

          f32 const scale = ((f32) run_style.font_height * text_scale_factor) /
                            (f32) font_bundle[run_font].atlas.font_height;

          f32 segment_width = 0;

          usize const glyph_shapings_offset = glyph_shapings.size();

          for (usize i = 0; i < glyph_infos.size(); i++)
          {
            f32 const advance =
                scale * (f32) glyph_positions[i].x_advance / 64.0f;
            Vec2 const offset =
                scale * Vec2{(f32) glyph_positions[i].x_offset / 64.0f,
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

          usize const nglyph_shapings = glyph_infos.size();

          segment_width +=
              has_spacing ? (text_scale_factor * run_style.word_spacing) : 0;

          run_segments
              .push(
                  TextRunSegment{.has_spacing           = has_spacing,
                                 .text                  = segment_text,
                                 .direction             = run_direction,
                                 .style                 = irun_style,
                                 .font                  = run_font,
                                 .glyph_shapings_offset = glyph_shapings_offset,
                                 .nglyph_shapings       = nglyph_shapings,
                                 .width                 = segment_width})
              .unwrap();

          segment_text_begin = segment_text_end;
        }

        run_text_begin = run_text_end;
      }

      SBParagraphRelease(sb_paragraph);

      usize const nparagraph_run_segments =
          run_segments.size() - paragraph_run_segments_offset;
      TextRunSegment *const paragraph_run_segments_begin =
          run_segments.data() + paragraph_run_segments_offset;
      TextRunSegment *const paragraph_run_segments_end =
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
          TextDirection const direction = direction_run_begin->direction;
          TextRunSegment     *direction_run_end = direction_run_begin + 1;

          for (; direction_run_end < line_end; direction_run_end++)
          {
            if (direction_run_end->direction != direction) [[unlikely]]
            {
              break;
            }
          }

          if (direction == TextDirection::RightToLeft)
          {
            // re-order consecutive RTL segments on the line to match visual
            // reading direction
            stx::Span{direction_run_begin,
                      (usize) (direction_run_end - direction_run_begin)}
                .reverse();
          }

          for (TextRunSegment const &run_segment :
               stx::Span{direction_run_begin,
                         (usize) (direction_run_end - direction_run_begin)})
          {
            TextStyle const &style  = run_segment.style >= block.styles.size() ?
                                          block.default_style :
                                          block.styles[run_segment.style];
            f32 const        ascent = text_scale_factor *
                               font_bundle[run_segment.font].atlas.ascent *
                               style.font_height;
            f32 const descent = text_scale_factor *
                                font_bundle[run_segment.font].atlas.descent *
                                style.font_height;
            f32 const height = ascent + descent;
            line_height  = std::max(line_height, style.line_height * height);
            line_ascent  = std::max(line_ascent, ascent);
            line_descent = std::max(line_descent, descent);
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
                .run_segments_offset =
                    (usize) (line_begin - run_segments.data()),
                .nrun_segments = (usize) (line_end - line_begin)})
            .unwrap();

        span.y += line_height;
        span.x = std::max(span.x, line_width);

        line_begin = line_end;
      }

      paragraph_begin += paragraph_length;
    }

    SBAlgorithmRelease(algorithm);
    hb_buffer_destroy(shaping_buffer);
  }
};

}        // namespace ash