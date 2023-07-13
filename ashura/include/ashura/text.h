#pragma once

#include <string_view>

extern "C"
{
#include "SBAlgorithm.h"
#include "SBLine.h"
#include "SBParagraph.h"
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

// TODO(lamarrr): letter and word spacing are presently incorrectly used
struct TextStyle
{
  std::string_view                  font                    = {};                         // name to use to match the font. if font is not found or empty the fallback fonts are tried.
  stx::Span<std::string_view const> fallback_fonts          = {};                         // font to fallback to if {font} is not available. if none of the specified fallback fonts are found the first font in the font bundle will be used
  f32                               font_height             = 20;                         // px
  color                             foreground_color        = colors::BLACK;              //
  color                             outline_color           = colors::BLACK;              //
  f32                               outline_thickness       = 0;                          // px. TODO(lamarrr): outline spread??? we can also scale by px sdf_spread/outline_width
  color                             shadow_color            = colors::BLACK;              //
  f32                               shadow_scale            = 0;                          // relative. multiplied by font_height
  vec2                              shadow_offset           = vec2{0, 0};                 // px. offset from center of glyph
  color                             background_color        = colors::TRANSPARENT;        //
  color                             underline_color         = colors::BLACK;              //
  f32                               underline_thickness     = 0;                          // px
  color                             strikethrough_color     = colors::BLACK;              //
  f32                               strikethrough_thickness = 0;                          // px
  f32                               letter_spacing          = 0;                          // px. additional letter spacing, can be negative
  f32                               word_spacing            = 0;                          // px. additional word spacing, can be negative
  f32                               line_height             = 1.2f;                       // relative. multiplied by font_height
  bool                              use_kerning             = true;                       // use provided font kerning
  bool                              use_ligatures           = true;                       // use standard and contextual font ligature substitution
};

/// A text run is a sequence of characters sharing a single property.
/// i.e. foreground color, font etc.
struct TextRun
{
  usize size  = 0;        // byte size coverage of this run. i.e. for the first run with size 20 all text within [0, 20] bytes range of the text will be styled using this run
  usize style = 0;        // run style to use
};

enum class TextAlign : u8
{
  Left,
  Center,
  Right
};

enum class TextOverflow
{
  Wrap,
  Ellipsis
};

struct TextBlock
{
  std::string_view           text;                                          // utf-8-encoded text, Span because string view doesnt support non-string types
  stx::Span<TextRun const>   runs;                                          // parts of text not styled by a run will use the paragraphs run style
  stx::Span<TextStyle const> styles;
  TextStyle                  default_style;                                 // default run styling
  TextAlign                  align     = TextAlign::Left;                   // text alignment
  TextOverflow               overflow  = TextOverflow::Wrap;
  TextDirection              direction = TextDirection::LeftToRight;        // base text direction
  std::string_view           language  = {};                                // base language to use for selecting opentype features to used on the text, uses default if not set
};

/// Placement of glyph.
/// All coordinates are relative to the paragraph
struct GlyphLayout
{
  vec2             offset;                   // context-dependent text shaping offset from normal font glyph position
  f32              advance = 0;              // horizontal-layout advance
  usize            font    = 0;              // resolved font index
  u32              glyph   = 0;              // glyph index in font
  TextStyle const *style   = nullptr;        // text styling information
};

/// RunSegment is a part of a text run split by groups of spacing characters word contained in a run.
/// The spacing characters translate to break opportunities.
struct TextRunSegment
{
  bool                  is_break_opportunity = false;        // if it has trailing spacing characters (tabs and spaces) where we can break the text, this corresponds to the unicode Break-After (BA)
  stx::Span<char const> text                 = {};
  hb_direction_t        direction            = HB_DIRECTION_INVALID;
  hb_script_t           script               = HB_SCRIPT_INVALID;
  hb_language_t         language             = HB_LANGUAGE_INVALID;
  TextStyle const      *style                = nullptr;
  f32                   width                = 0;        // sum of all advances + letter spacing + word spacing
};

struct LineMetrics
{
  f32 width       = 0;        // width of the line
  f32 ascent      = 0;        // maximum ascent of all the runs on the line
  f32 descent     = 0;        // maximum descent of all the runs on the line
  f32 line_height = 0;        // maximum line height of all the runs on the line
};

struct TextLayout
{
  stx::Span<hb_glyph_position_t const> shape_text_harfbuzz(Font const &font, stx::Span<char const> text, hb_script_t script, hb_direction_t direction, hb_language_t language, TextStyle const &style)
  {
    hb_feature_t const shaping_features[] = {{.tag   = HB_TAG('k', 'e', 'r', 'n'),        // kerning operations
                                              .value = style.use_kerning,
                                              .start = HB_FEATURE_GLOBAL_START,
                                              .end   = HB_FEATURE_GLOBAL_END},
                                             {.tag   = HB_TAG('l', 'i', 'g', 'a'),        // standard ligature glyph substitution
                                              .value = style.use_ligatures,
                                              .start = HB_FEATURE_GLOBAL_START,
                                              .end   = HB_FEATURE_GLOBAL_END},
                                             {.tag   = HB_TAG('c', 'l', 'i', 'g'),        // contextual ligature glyph substitution
                                              .value = style.use_ligatures,
                                              .start = HB_FEATURE_GLOBAL_START,
                                              .end   = HB_FEATURE_GLOBAL_END}};

    hb_buffer_reset(font.hb_buffer);
    hb_buffer_set_replacement_codepoint(font.hb_buffer, HB_BUFFER_REPLACEMENT_CODEPOINT_DEFAULT);        // invalid character replacement
    hb_buffer_set_not_found_glyph(font.hb_buffer, 0);                                                    // default glyphs for characters without defined glyphs
    hb_buffer_set_script(font.hb_buffer, script);
    hb_buffer_set_direction(font.hb_buffer, direction);
    hb_buffer_set_language(font.hb_buffer, language);
    hb_font_set_scale(font.hb_font, (int) (64 * style.font_height), (int) (64 * style.font_height));
    hb_buffer_add_utf8(font.hb_buffer, text.data(), text.size(), 0, text.size());
    hb_shape(font.hb_font, font.hb_buffer, shaping_features, std::size(shaping_features));

    uint                       nglyphs;
    hb_glyph_position_t const *glyph_pos = hb_buffer_get_glyph_positions(font.hb_buffer, &nglyphs);
    ASH_CHECK(!(glyph_pos == nullptr && nglyphs > 0));

    return stx::Span{glyph_pos, nglyphs};
  }

  // Not thread-safe. it mutates the supplied font's data
  void layout(TextBlock const &block, stx::Span<BundledFont const> const font_bundle, f32 const max_line_width)
  {
    constexpr u32 ELLIPSIS_CODEPOINT = 0x2026;

    subwords.clear();
    glyphs.clear();
    glyph_layouts.clear();
    span = {};

    // there's no layout to perform without a font
    if (font_bundle.is_empty())
    {
      return;
    }

    // resolved fonts for each style specification
    stx::Vec<usize> resolved_fonts;

    hb_language_t const language_hb = block.language.empty() ? hb_language_get_default() : hb_language_from_string(block.language.data(), block.language.size());        // uses setLocale to get default language from locale which isn't thread-safe

    for (TextStyle const &style : block.styles)
    {
      usize font = match_font(style.font, style.fallback_fonts, font_bundle);
      // use first font in the font bundle if specified font and fallback fonts are not found
      resolved_fonts
          .push(font < font_bundle.size() ? font : 0)
          .unwrap();
    }

    {
      usize font = match_font(block.default_style.font, block.default_style.fallback_fonts, font_bundle);
      resolved_fonts
          .push(font < font_bundle.size() ? font : 0)
          .unwrap();
    }

    SBCodepointSequence codepoint_sequence = {SBStringEncodingUTF8, (void *) block.text.data(), block.text.size()};
    SBAlgorithmRef      algorithm          = SBAlgorithmCreate(&codepoint_sequence);
    ASH_CHECK(algorithm != nullptr);
    char const    *p_style_text_begin = block.text.data();
    TextRun const *style_it           = block.runs.begin();
    f32            line_top           = 0;

    stx::Vec<TextRunSegment> segments;

    // TODO(lamarrr): let's perform text layout line by line and reset memory allocations instead of storing all of them
    // and only add run areas that have specific styling requirements
    for (SBUInteger paragraph_begin = 0; paragraph_begin < block.text.size();)
    {
      SBParagraphRef sb_paragraph = SBAlgorithmCreateParagraph(algorithm, paragraph_begin, UINTPTR_MAX, block.direction == TextDirection::LeftToRight ? SBLevelDefaultLTR : SBLevelDefaultRTL);
      ASH_CHECK(sb_paragraph != nullptr);
      SBUInteger const     paragraph_length     = SBParagraphGetLength(sb_paragraph);           // number of bytes of the paragraph
      SBLevel const        paragraph_base_level = SBParagraphGetBaseLevel(sb_paragraph);        // base direction, 0 - LTR, 1 - RTL
      SBLevel const *const levels               = SBParagraphGetLevelsPtr(sb_paragraph);        // SheenBidi expands to byte level representation of the codepoints
      char const *const    p_paragraph_begin    = block.text.data() + paragraph_begin;
      char const *const    p_paragraph_end      = p_paragraph_begin + paragraph_length;

      for (char const *run_text_it = p_paragraph_begin; run_text_it < p_paragraph_end;)
      {
        char const *const    p_run_begin         = run_text_it;
        SBLevel const        run_level           = levels[p_run_begin - p_paragraph_begin];
        SBCodepoint const    run_first_codepoint = stx::utf8_next(run_text_it);
        SBScript const       run_script          = SBCodepointGetScript(run_first_codepoint);
        TextStyle const     *p_run_style         = &block.default_style;
        hb_script_t const    run_script_hb       = hb_script_from_iso15924_tag(SBScriptGetOpenTypeTag(run_script));        // Note that unicode scripts are different from OpenType (iso15924) scripts though they have similarities
        hb_direction_t const run_direction_hb    = (run_level & 0x1) == 0 ? HB_DIRECTION_LTR : HB_DIRECTION_RTL;

        // find the style configuration intended for this text run (if any)
        while (style_it < block.runs.end())
        {
          if (p_run_begin >= p_style_text_begin && p_run_begin < (p_style_text_begin + style_it->size))
          {
            p_run_style = &block.styles[style_it->style];
            break;
          }
          else
          {
            p_style_text_begin += style_it->size;
            style_it++;
          }
        }

        usize const irun_font = (p_run_style == &block.default_style) ? resolved_fonts[resolved_fonts.size() - 1] : resolved_fonts[p_run_style - block.styles.begin()];
        Font const &run_font  = *font_bundle[irun_font].font;

        // find the last codepoint that belongs to this text run
        while (run_text_it < p_paragraph_end)
        {
          char const       *p_next_codepoint = run_text_it;
          SBLevel const     level            = levels[p_next_codepoint - p_paragraph_begin];
          SBCodepoint const codepoint        = stx::utf8_next(p_next_codepoint);
          SBScript const    script           = SBCodepointGetScript(codepoint);
          TextStyle const  *p_style          = &block.default_style;

          // find the style configuration intended for this code point (if any)
          while (style_it < block.runs.end())
          {
            if (run_text_it >= p_style_text_begin && run_text_it < (p_style_text_begin + style_it->size))
            {
              p_style = &block.styles[style_it->style];
              break;
            }
            else
            {
              p_style_text_begin += style_it->size;
              p_style++;
            }
          }

          // inherited scripts inherit the preceding codepoint's script.
          // common scripts can be used with any script.
          bool const is_matching_script = (script == run_script) || (script == SBScriptZINH || script == SBScriptZYYY);

          if (level != run_level || !is_matching_script || p_style != p_run_style)
          {
            // reached end of run
            break;
          }

          // make retrieved codepoint part of run, then advance iterator
          run_text_it = p_next_codepoint;
        }

        // split runs into segments and perform text layout
        // we just need blocks. alignment can happen at the very end.
        // we can remove the absolute positioning property of the TextRun areas and use metrics instead
        //
        // TODO(lamarrr): this would mean text shaping can't span across multiple runs
        // its important that we can shape across multiple runs
        //

        // Text Segmentation
        for (char const *p_run_segment_it = p_run_begin; p_run_segment_it < run_text_it;)
        {
          char const *const p_run_segment_begin = p_run_segment_it;
          bool              has_spacing         = false;

          while (p_run_segment_it < run_text_it)
          {
            char const       *p_next_codepoint = p_run_segment_it;
            SBCodepoint const codepoint        = stx::utf8_next(p_next_codepoint);
            if (codepoint == ' ' || codepoint == '\t')
            {
              // don't break immediately so we can batch multiple tabs/spaces
              has_spacing = true;
            }
            else if (has_spacing)        // has a preceding spacing character
            {
              break;
            }
            p_run_segment_it = p_next_codepoint;
          }

          stx::Span const run_segment_text{p_run_segment_begin, p_run_segment_it - p_run_segment_begin};
          stx::Span const glyph_positions = shape_text_harfbuzz(run_font, run_segment_text, run_script_hb, run_direction_hb, language_hb, *p_run_style);

          f32 segment_width = 0;

          for (hb_glyph_position_t const glyph_position : glyph_positions)
          {
            segment_width += (glyph_position.x_advance >> 6) + p_run_style->letter_spacing;
          }

          segment_width += has_spacing ? p_run_style->word_spacing : 0;

          segments.push(TextRunSegment{.is_break_opportunity = has_spacing,
                                       .text                 = run_segment_text,
                                       .direction            = run_direction_hb,
                                       .script               = run_script_hb,
                                       .language             = language_hb,
                                       .style                = p_run_style,
                                       .width                = segment_width})
              .unwrap();
        }
      }

      // Line Breaking

      SBParagraphRelease(sb_paragraph);

      paragraph_begin += paragraph_length;
    }

    SBAlgorithmRelease(algorithm);

    /** Font Resolution and Word Shaping */
    for (TextRunSubWord &subword : subwords)
    {
      TextStyle const &props  = paragraph.runs[subword.run].props.as_cref().unwrap_or(paragraph.props);
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

      // if no font or fallback font is found use the first font in the bundle. NOTE that we already ensured there's at least one font in the bundle
      if (font_s.is_empty())
      {
        font_s = font_bundle.slice(0, 1);
      }

      usize            font_index = AS(usize, font_s.begin() - font_bundle.begin());
      Font const      &font       = *font_bundle[font_index].font;
      FontAtlas const &atlas      = font_bundle[font_index].atlas;

      hb_feature_t shaping_features[] = {{.tag = KERNING_FEATURE, .value = props.use_kerning, .start = HB_FEATURE_GLOBAL_START, .end = HB_FEATURE_GLOBAL_END},
                                         {.tag = LIGATURE_FEATURE, .value = props.use_ligatures, .start = HB_FEATURE_GLOBAL_START, .end = HB_FEATURE_GLOBAL_END},
                                         {.tag = CONTEXTUAL_LIGATURE_FEATURE, .value = props.use_ligatures, .start = HB_FEATURE_GLOBAL_START, .end = HB_FEATURE_GLOBAL_END}};

      hb_font_set_scale(font.hb_font, 64 * props.font_height, 64 * props.font_height);

      hb_buffer_reset(font.hb_buffer);
      hb_buffer_set_script(font.hb_buffer, AS(hb_script_t, props.script));

      if (props.direction == TextDirection::LeftToRight)
      {
        hb_buffer_set_direction(font.hb_buffer, HB_DIRECTION_LTR);
      }
      else
      {
        hb_buffer_set_direction(font.hb_buffer, HB_DIRECTION_RTL);
      }

      // TODO(lamarrr): actually use the spaces in text shaping
      hb_buffer_set_language(font.hb_buffer, props.language.empty() ? hb_language_get_default() : hb_language_from_string(props.language.data(), AS(int, props.language.size())));
      hb_buffer_add_utf8(font.hb_buffer, subword.text.begin(), AS(int, subword.text.size()), 0, AS(int, subword.text.size()));
      hb_shape(font.hb_font, font.hb_buffer, shaping_features, AS(uint, std::size(shaping_features)));

      uint                 nglyphs;
      hb_glyph_info_t     *glyph_info = hb_buffer_get_glyph_infos(font.hb_buffer, &nglyphs);
      uint                 nglyph_pos;
      hb_glyph_position_t *glyph_pos = hb_buffer_get_glyph_positions(font.hb_buffer, &nglyph_pos);
      ASH_CHECK(!(glyph_info == nullptr && nglyphs > 0));
      ASH_CHECK(!(glyph_pos == nullptr && nglyph_pos > 0));
      ASH_CHECK(nglyph_pos == nglyphs);

      f32 width       = 0;
      f32 glyph_scale = props.font_height / atlas.font_height;

      subword.font         = font_index;
      subword.glyphs_begin = glyphs.size();

      // TODO(lamarrr): invalid glyphs might still have advances
      for (usize i = 0; i < nglyphs; i++)
      {
        u32       glyph_index = glyph_info[i].codepoint;
        vec2      offset{glyph_pos[i].x_offset / 64.0f, -glyph_pos[i].y_offset / 64.0f};
        f32       advance = glyph_pos[i].x_advance / 64.0f;
        stx::Span glyph   = atlas.get(glyph_index);

        if (!glyph.is_empty())
        {
          width += advance + props.letter_spacing;
          glyphs.push_inplace(TextRunGlyph{
                                  .index   = glyph_index,
                                  .offset  = offset,
                                  .advance = advance})
              .unwrap();
          subword.nglyphs++;
        }
        // use glyph at index 0 as replacement glyph, this is usually the invalid character replacement glyph
        else if (!atlas.glyphs.is_empty())
        {
          // HB_BUFFER_REPLACEMENT_CODEPOINT_DEFAULT
          width += advance + props.letter_spacing;
          glyphs.push_inplace(TextRunGlyph{
                                  .index   = 0,
                                  .offset  = offset,
                                  .advance = advance})
              .unwrap();
          subword.nglyphs++;
        }
        else
        {
          // can't find a replacement glyph, we'll pretend as if there's nothing there
        }
      }

      subword.glyph_scale = glyph_scale;
      subword.width       = width;
    }

    /** Word Wrapping and Line Breaking */
    {
      f32 cursor_x = 0;
      for (TextRunSubWord *iter = subwords.begin(); iter < subwords.end();)
      {
        TextRunSubWord  *word_begin = iter;
        TextStyle const &props      = paragraph.runs[word_begin->run].props.as_cref().unwrap_or(paragraph.props);
        f32              word_width = word_begin->width + word_begin->nspace_chars * props.word_spacing;

        TextRunSubWord *word_end = word_begin + 1;

        if (word_begin->nspace_chars == 0 && word_begin->nnewline_chars == 0)
        {
          for (; word_end < subwords.end();)
          {
            TextStyle const &props = paragraph.runs[word_end->run].props.as_cref().unwrap_or(paragraph.props);
            word_width += word_end->width + word_end->nspace_chars * props.word_spacing;

            // if at last subword
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

        // wrap word to new line if its width exceeds the maximum line width
        if (cursor_x + word_width > max_line_width)
        {
          word_begin->is_wrapped = true;
          cursor_x               = word_width;
        }
        else if ((word_end - 1)->nnewline_chars > 0)
        {
          cursor_x = 0;
        }
        else
        {
          cursor_x += word_width;
        }

        iter = word_end;
      }
    }

    {
      // resolve line breaks using word wrapping and newline breaks. if it has a newline, the wrapping doesn't count as a line break
      for (TextRunSubWord *iter = subwords.begin(); iter < subwords.end(); iter++)
      {
        if (iter->nnewline_chars > 0)
        {
          iter->nline_breaks = iter->nnewline_chars;
        }

        if (iter->is_wrapped)
        {
          TextRunSubWord *previous = iter - 1;
          if (previous >= subwords.begin() && previous->nnewline_chars == 0)
          {
            previous->nline_breaks = 1;
          }
        }
      }
    }

    /** Line Layout and Glyph Placement */
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

        // TODO(lamarrr): when positioning the glyphs we also need to use the spread factor
        // scale the spread by the

        for (TextRunSubWord const *subword = line_begin; subword < line_end; subword++)
        {
          TextStyle const &props = paragraph.runs[subword->run].props.as_cref().unwrap_or(paragraph.props);
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
          TextStyle const &props = paragraph.runs[subword->run].props.as_cref().unwrap_or(paragraph.props);
          if (props.direction == TextDirection::LeftToRight)
          {
            FontAtlas const &atlas = font_bundle[subword->font].atlas;

            subword->area.offset   = vec2{line_alignment_x + cursor_x, line_top};
            subword->area.extent   = vec2{subword->width + subword->nspace_chars * props.word_spacing, line_height};
            subword->area.baseline = vec2{line_alignment_x + cursor_x, baseline_y};
            subword->area.line_top = vec2{line_alignment_x + cursor_x, baseline_y - subword->glyph_scale * atlas.ascent};

            for (TextRunGlyph const &run_glyph : glyphs.span().slice(subword->glyphs_begin, subword->nglyphs))
            {
              Glyph const &glyph  = atlas.glyphs[run_glyph.index];
              vec2         offset = vec2{line_alignment_x + cursor_x + subword->glyph_scale * glyph.bearing.x, baseline_y - subword->glyph_scale * glyph.bearing.y};
              offset              = offset + run_glyph.offset;

              glyph_layouts.push(GlyphLayout{
                                     .offset = offset,
                                     .extent = subword->glyph_scale * glyph.extent.to_vec(),
                                     .run    = subword->run,
                                     .font   = subword->font,
                                     .glyph  = run_glyph.index})
                  .unwrap();

              cursor_x += run_glyph.advance + props.letter_spacing;
            }

            cursor_x += subword->nspace_chars * props.word_spacing;
            subword++;
          }
          else
          {
            f32             rtl_width = 0;
            TextRunSubWord *rtl_begin = subword;
            TextRunSubWord *rtl_end   = subword + 1;

            TextStyle const &props = paragraph.runs[rtl_begin->run].props.as_cref().unwrap_or(paragraph.props);

            rtl_width += rtl_begin->width + rtl_begin->nspace_chars * props.word_spacing;

            for (; rtl_end < line_end; rtl_end++)
            {
              TextStyle const &props = paragraph.runs[rtl_end->run].props.as_cref().unwrap_or(paragraph.props);
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
              TextStyle const &props = paragraph.runs[rtl_iter->run].props.as_cref().unwrap_or(paragraph.props);
              FontAtlas const &atlas = font_bundle[rtl_iter->font].atlas;

              rtl_cursor_x -= rtl_iter->width + rtl_iter->nspace_chars * props.word_spacing;

              rtl_iter->area.offset   = vec2{line_alignment_x + rtl_cursor_x, line_top};
              rtl_iter->area.extent   = vec2{rtl_iter->width + rtl_iter->nspace_chars * props.word_spacing, line_height};
              rtl_iter->area.baseline = vec2{line_alignment_x + rtl_cursor_x, baseline_y};
              rtl_iter->area.line_top = vec2{line_alignment_x + cursor_x, baseline_y - rtl_iter->glyph_scale * atlas.ascent};

              f32 glyph_cursor_x = rtl_cursor_x;

              for (TextRunGlyph const &run_glyph : glyphs.span().slice(rtl_iter->glyphs_begin, rtl_iter->nglyphs))
              {
                Glyph const &glyph  = atlas.glyphs[run_glyph.index];
                vec2         offset = vec2{line_alignment_x + glyph_cursor_x + rtl_iter->glyph_scale * glyph.bearing.x, baseline_y - rtl_iter->glyph_scale * glyph.bearing.y};
                offset              = offset + run_glyph.offset;

                glyph_layouts.push(GlyphLayout{
                                       .offset = offset,
                                       .extent = rtl_iter->glyph_scale * glyph.extent.to_vec(),
                                       .run    = rtl_iter->run,
                                       .font   = rtl_iter->font,
                                       .glyph  = run_glyph.index})
                    .unwrap();

                glyph_cursor_x += run_glyph.advance + props.letter_spacing;
              }
            }

            cursor_x += rtl_width;
            subword = rtl_end;
          }
        }

        span.y = line_top + line_height + (nline_breaks > 1 ? (nline_breaks - 1) * line_height : 0.0f);

        line_top += nline_breaks * line_height;

        span.x = std::max(span.x, line_alignment_x + line_width);

        iter = line_end;
      }
    }

    glyphs.clear();
  }
};

}        // namespace ash