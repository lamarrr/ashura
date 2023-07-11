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
  gfx::image                        foreground_image        = gfx::WHITE_IMAGE;
  color                             background_color        = colors::TRANSPARENT;        //
  color                             outline_color           = colors::BLACK;              //
  f32                               outline_thickness       = 0;                          // px. TODO(lamarrr): outline spread??? we can also scale by px sdf_spread/outline_width
  color                             shadow_color            = colors::BLACK;              //
  f32                               shadow_scale            = 0;                          // relative. multiplied by font_height
  vec2                              shadow_offset           = vec2{0, 0};                 // px. offset from center of glyph
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

/// A text run is a sequence of characters sharing a single property i.e. foreground color, font etc.
/// This is a user-level run
struct TextRun
{
  usize size  = 0;        // byte size coverage of this run
  usize style = 0;        // run properties. if not set the paragraph's run properties are used instead
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
  std::string_view           text;                              // utf-8-encoded text, Span because string view doesnt support non-string types
  stx::Span<TextRun const>   runs;                              // parts of text not styled by a run will use the paragraphs run style
  TextStyle                  default_style;                     // default run styling
  stx::Span<TextStyle const> styles;
  TextAlign                  align    = TextAlign::Left;        // text alignment
  TextOverflow               overflow = TextOverflow::Wrap;
};

/// Area occupied by a text run
/// All coordinates are relative to the paragraph
struct TextRunArea
{
  vec2 offset;
  vec2 extent;
  vec2 baseline;
  vec2 line_top;
};

/// Placement of glyph.
/// All coordinates are relative to the paragraph
struct GlyphLayout
{
  vec2  offset;
  vec2  extent;
  usize run   = 0;
  usize font  = 0;
  u32   glyph = 0;
};

// TODO(lamarrr): include trailing whitespace width
struct TextRunGlyph
{
  u32  index = 0;          // glyph index in font
  vec2 offset;             // context-dependent text shaping offset from normal font glyph position
  f32  advance = 0;        // horizontal-layout advance
};

/// this is part of a word that is styled by a run. i.e. a word: 'Goog', could have 'G' as red, 'oo' as yellow, and 'g' as blue,
/// 'G' will be a run subword, 'oo' is another run subword, and 'g' will be another subword as they have different properties
/// determined by the run they belong to, although part of the same word.
///
struct TextRunSubWord
{
  stx::Span<char const> text;                          // slice of the text contents of this subword
  usize                 run            = 0;            // the text run this subword belongs to
  usize                 font           = 0;            // resolved font index in the font bundle
  f32                   width          = 0;            // px. width of all the letters including trailing tabs and spaces
  bool                  has_spacing    = false;        // whether this run subword has spacing at its end, this means it has a number of trailing tabs and spaces
  usize                 nnewline_chars = 0;            // number of newline characters
  usize                 nline_breaks   = 0;            // resolved line breaks (after wrapping)
  usize                 glyphs_begin   = 0;            // begin offset of this subword's glyphs in the global glyph array. enables us to not have to have a Vec for each subword
  usize                 nglyphs        = 0;            // number of glyphs belonging to this subword
  bool                  is_wrapped     = false;        // if subword is wrapped (i.e. it would exceed the max line width if it is not placed on a new line)
  TextRunArea           area;                          // laid out area of this run subword
};

/// tabs or spaces
/// spacings are also break opportunities
struct TextSpacing
{
  char const *begin = nullptr;        // byte offset on line
  usize       size  = 0;              // byte size of total number of spaces on line
};

struct RunSubword
{
  char const *begin       = nullptr;        // byte offset on line
  usize       size        = 0;              // byte size of total number of spaces on line
  SBLevel     level       = 0;
  SBScript    script      = SBScriptNil;
  usize       spacing     = 0;
  bool        has_spacing = false;
};

struct TextLayout
{
  struct TextSubRun
  {
    char const      *text_begin           = nullptr;
    char const      *text_end             = nullptr;
    TextStyle const *style                = nullptr;
    SBScript         script               = SBScriptNil;
    SBLevel          direction            = 0;
    bool             is_break_opportunity = false;
    f32              width                = 0;
  };

  // LineMetrics?

  stx::Vec<TextRunSubWord> subwords;
  stx::Vec<TextRunGlyph>   glyphs;
  stx::Vec<GlyphLayout>    glyph_layouts;        // laid out glyphs
  vec2                     span;                 // normal extent the text spans. might be more than the provided max_line_width at layout time

  // TODO(lamarrr): [future] add bidi
  /// performs layout of the paragraph
  void layout(TextBlock const &block, stx::Span<BundledFont const> const font_bundle, f32 const max_line_width)
  {
    static constexpr hb_tag_t KERNING_FEATURE             = HB_TAG('k', 'e', 'r', 'n');        // kerning operations
    static constexpr hb_tag_t LIGATURE_FEATURE            = HB_TAG('l', 'i', 'g', 'a');        // standard ligature substitution
    static constexpr hb_tag_t CONTEXTUAL_LIGATURE_FEATURE = HB_TAG('c', 'l', 'i', 'g');        // contextual ligature substitution
    constexpr u32             ELLIPSIS_CODEPOINT          = 0x2026;

    subwords.clear();
    glyphs.clear();
    glyph_layouts.clear();
    span = {};

    // there's no layout to perform without a font
    if (font_bundle.is_empty())
    {
      return;
    }

    stx::Vec<usize> resolved_fonts;

    // for(), resolve fonts

    SBCodepointSequence codepoint_sequence = {SBStringEncodingUTF8, (void *) block.text.data(), block.text.size()};
    SBAlgorithmRef      algorithm          = SBAlgorithmCreate(&codepoint_sequence);
    ASH_CHECK(algorithm != nullptr);
    char const    *p_style_text_begin = block.text.data();
    TextRun const *style_it           = block.runs.begin();
    f32            line_top           = 0;
    // stx::Vec<usize> line_break_opportunities;

    // TODO(lamarrr): let's perform text layout line by line and reset memory allocations instead of storing all of them
    // and only add run areas that have specific styling requirements
    for (SBUInteger paragraph_begin = 0; paragraph_begin < block.text.size();)
    {
      SBParagraphRef sb_paragraph = SBAlgorithmCreateParagraph(algorithm, paragraph_begin, UINTPTR_MAX, SBLevelDefaultLTR);
      ASH_CHECK(sb_paragraph != nullptr);
      SBUInteger const     paragraph_length     = SBParagraphGetLength(sb_paragraph);           // number of bytes of the paragraph
      SBLevel const        paragraph_base_level = SBParagraphGetBaseLevel(sb_paragraph);        // base direction, 0 - LTR, 1 - RTL
      SBLevel const *const levels               = SBParagraphGetLevelsPtr(sb_paragraph);        // SheenBidi expands to byte level representation of the codepoints
      char const *const    p_paragraph_begin    = block.text.data() + paragraph_begin;
      char const *const    p_paragraph_end      = p_paragraph_begin + paragraph_length;

      // line_break_opportunities.clear();
      // line_break_opportunities.unsafe_resize_uninitialized(paragraph_length).unwrap();
      // we have to check from back for line break opportunity
      // if we find one and it still doesn't fit on the line break opportunity

      // lay out till end of line break opportunity. if it doesn't fit, shrink by 1 break point
      // lay out again and check if it fits, continue until we can fit onto the line.
      //
      //

      // for (char const *line_break_opportunity_begin = p_paragraph_begin; line_break_finder < p_paragraph_end;)
      // {
      //   SBCodepoint const codepoint = stx::utf8_next(line_break_finder);
      //   if (codepoint == ' ' || codepoint == '\t')
      //   {
      //   }
      // }

      stx::Vec<TextSpacing> spacings;
      stx::Vec<RunSubword>  subwords;

      // process text runs on the present paragraph
      for (char const *run_text_it = p_paragraph_begin; run_text_it < p_paragraph_end;)
      {
        char const *const p_run_begin         = run_text_it;
        SBLevel const     run_level           = levels[p_run_begin - p_paragraph_begin];
        SBCodepoint const run_first_codepoint = stx::utf8_next(run_text_it);
        SBScript const    run_script          = SBCodepointGetScript(run_first_codepoint);
        TextStyle const  *p_run_style         = &block.default_style;

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

        // find the last codepoint belongs to this text run
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

          if (level != run_level || script != run_script || p_style != p_run_style)
          {
            // mark as end of run
            break;
          }
          else
          {
            // make retrieved codepoint part of run and advance
            run_text_it = p_next_codepoint;
          }
        }

        // split into subruns and perform text layout
        // we just need blocks. alignment can happen at the very end.
        // we can remove the absolute positioning property of the TextRun areas and use metrics instead
        //
        // TODO(lamarrr): this would mean text shaping can't span across multiple runs
        // its important that we can shape across multiple runs
        //
        for (char const *p_word_it = p_run_begin; p_word_it < run_text_it;)
        {
          char const *p_word_end    = p_word_it;
          char const *spacing_begin = nullptr;
          char const *spacing_end   = nullptr;

          while (p_word_end < run_text_it)
          {
            char const       *p_next_codepoint = p_word_end;
            SBCodepoint const codepoint        = stx::utf8_next(p_next_codepoint);
            if (codepoint == ' ' || codepoint == '\t')
            {
              spacing_begin = p_word_end;
              // add break point after codepoint
              // find last spacing character
              for (; p_next_codepoint < run_text_it;)
              {
                // find end of spacing
                // make sure we don't iterate over this codepoint next time
              }
              break;
            }
            else if (codepoint == '\n' || codepoint == '\r')
            {
              // already end of paragraph
              break;
            }
            else
            {
              p_word_end = p_next_codepoint;
            }
          }

          bool const has_spacing = spacing_end > spacing_begin && spacing_begin != nullptr;

          usize spacing = 0;

          if (has_spacing)
          {
            spacing = spacings.size();
            spacings.push(TextSpacing{.begin = spacing_begin,
                                      .size  = (usize) (spacing_end - spacing_begin)})
                .unwrap();
          }

          subwords.push(RunSubword{
                            .begin       = p_word_it,
                            .size        = (usize) (p_word_end - p_word_it),
                            .level       = run_level,
                            .script      = run_script,
                            .spacing     = spacing,
                            .has_spacing = has_spacing})
              .unwrap();
        }
      }


      // perform text shaping

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