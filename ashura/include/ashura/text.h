#pragma once

#include "SBAlgorithm.h"
#include "SBParagraph.h"
#include "ashura/font.h"
#include "ashura/primitives.h"
#include "harfbuzz/hb.h"
#include "stx/span.h"
#include "stx/text.h"
#include "stx/vec.h"
#include <string_view>

namespace ash
{

/// @brief to work around this, replace all tabs with the the number of equivalent spaces you'd like
constexpr u32 DEFAULT_TAB_SIZE = 8;

enum class TextDirection : u8
{
  LeftToRight,
  RightToLeft
};

// TODO(lamarrr): letter and word spacing are presently incorrectly used
struct RunProps
{
  std::string_view                  font                    = {};        // name to use to match the font. if font is not found or empty the fallback fonts are tried.
  stx::Span<std::string_view const> fallback_fonts          = {};        // font to fallback to if specified font is not available. if none of the specified fallback fonts are found the first font in the font bundle will be used
  f32                               font_height             = 20;        // px
  color                             foreground_color        = colors::BLACK;
  color                             background_color        = colors::TRANSPARENT;
  color                             outline_color           = colors::BLACK;
  f32                               outline_thickness       = 0;                                 // px. TODO(lamarrr): outline spread??? we can also scale by px sdf_spread/outline_width
  color                             shadow_color            = colors::TRANSPARENT;
  f32                               shadow_scale            = 1;                                 // relative. multiplied by font_height
  vec2                              shadow_offset           = vec2{0, 0};                        // px. offset from center of glyph
  color                             underline_color         = colors::BLACK;
  f32                               underline_thickness     = 0;                                 // px
  color                             strikethrough_color     = colors::BLACK;
  f32                               strikethrough_thickness = 0;                                 // px
  f32                               letter_spacing          = 0;                                 // px. additional letter spacing, can be negative
  f32                               word_spacing            = 0;                                 // px. additional word spacing, can be negative
  f32                               line_height             = 1.2f;                              // relative. multiplied by font_height
  TextDirection                     direction               = TextDirection::LeftToRight;        // default text direction
  bool                              use_kerning             = true;                              // use provided font kerning
  bool                              use_ligatures           = true;                              // use standard and contextual font ligature substitution
  std::string_view                  language                = {};                                // locale language. if empty, the machine's locale will be retrieved and used
  std::string_view                  script                  = {};                                // script to use for the glyph run. if set this will override script detection
};

/// A text run is a sequence of characters sharing a single property i.e. foreground color, font family etc.
struct TextRun
{
  stx::Span<char const> text;        /// utf-8-encoded text, Span because string view doesnt support non-string types
  stx::Option<RunProps> props;
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
  RunProps                 props;
  TextAlign                align = TextAlign::Left;
  // TextOverflow             overflow = TextOverflow::Wrap;
  // std::string_view         ellipsis = "...";
};

struct TextRunGlyph
{
  u32  index = 0;          // glyph index in font
  vec2 offset;             // context-dependent text shaping offset from normal font glyph position
  f32  advance = 0;        // horizontal advance to use
};

struct TextRunArea
{
  vec2 offset;
  vec2 extent;
  vec2 baseline;
  vec2 line_top;
};

struct GlyphLayout
{
  vec2  offset;
  vec2  extent;
  usize run   = 0;
  usize font  = 0;
  u32   glyph = 0;
};

/// this is part of a word that is styled by a run. i.e. a word: 'Goog', could have 'G' as red, 'oo' as yellow, and 'g' as blue,
/// 'G' will be a run subword, 'oo' is another run subword, and 'g' will be another subword as they have different properties
/// determined by the run they belong to, although part of the same word.
///
struct TextRunSubWord
{
  stx::Span<char const> text;
  usize                 run         = 0;
  usize                 font        = 0;
  f32                   glyph_scale = 0;
  // usize                 nspace_chars   = 0;
  // bool        has_spacing  = false;
  usize       nnewline_chars = 0;
  usize       nline_breaks   = 0;
  f32         width          = 0;        /// width of all the letters excluding the trailing white spaces
  usize       glyphs_begin   = 0;
  usize       nglyphs        = 0;
  bool        is_wrapped     = false;
  TextRunArea area;
};

struct TextLayout
{
  static constexpr hb_tag_t KERNING_FEATURE             = HB_TAG('k', 'e', 'r', 'n');        // kerning operations
  static constexpr hb_tag_t LIGATURE_FEATURE            = HB_TAG('l', 'i', 'g', 'a');        // standard ligature substitution
  static constexpr hb_tag_t CONTEXTUAL_LIGATURE_FEATURE = HB_TAG('c', 'l', 'i', 'g');        // contextual ligature substitution

  stx::Vec<TextRunSubWord> subwords;
  stx::Vec<TextRunGlyph>   glyphs;
  stx::Vec<GlyphLayout>    glyph_layouts;
  vec2                     span;

  // TODO(lamarrr): [future] add bidi
  /// performs layout of the paragraph and returns the width and height of the paragraph
  void layout(Paragraph const &paragraph, stx::Span<BundledFont const> const font_bundle, f32 max_line_width)
  {
    constexpr u32 SPACE    = ' ';
    constexpr u32 TAB      = '\t';
    constexpr u32 NEWLINE  = '\n';
    constexpr u32 RETURN   = '\r';
    constexpr u32 ELLIPSIS = 0x2026;

    subwords.clear();
    glyphs.clear();
    glyph_layouts.clear();
    span = {};

    // there's no layout to perform without a font
    if (font_bundle.is_empty())
    {
      return;
    }

    hb_unicode_funcs_t *hb_funcs = hb_unicode_funcs_get_default();
    ASH_CHECK(hb_funcs != nullptr);

    /** Word Tokenization */
    for (usize i = 0; i < paragraph.runs.size(); i++)
    {
      TextRun const  &run   = paragraph.runs[i];
      RunProps const &props = run.props.as_cref().unwrap_or(paragraph.props);
  // hb_script_get_horizontal_direction

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

    /** Font Resolution and Word Shaping */
    for (TextRunSubWord &subword : subwords)
    {
      RunProps const &props  = paragraph.runs[subword.run].props.as_cref().unwrap_or(paragraph.props);
      stx::Span       font_s = font_bundle.which([&](BundledFont const &f) {
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
        TextRunSubWord *word_begin = iter;
        RunProps const &props      = paragraph.runs[word_begin->run].props.as_cref().unwrap_or(paragraph.props);
        f32             word_width = word_begin->width + word_begin->nspace_chars * props.word_spacing;

        TextRunSubWord *word_end = word_begin + 1;

        if (word_begin->nspace_chars == 0 && word_begin->nnewline_chars == 0)
        {
          for (; word_end < subwords.end();)
          {
            RunProps const &props = paragraph.runs[word_end->run].props.as_cref().unwrap_or(paragraph.props);
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
          RunProps const  &props = paragraph.runs[subword->run].props.as_cref().unwrap_or(paragraph.props);
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
          RunProps const &props = paragraph.runs[subword->run].props.as_cref().unwrap_or(paragraph.props);
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

            RunProps const &props = paragraph.runs[rtl_begin->run].props.as_cref().unwrap_or(paragraph.props);

            rtl_width += rtl_begin->width + rtl_begin->nspace_chars * props.word_spacing;

            for (; rtl_end < line_end; rtl_end++)
            {
              RunProps const &props = paragraph.runs[rtl_end->run].props.as_cref().unwrap_or(paragraph.props);
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
              RunProps const  &props = paragraph.runs[rtl_iter->run].props.as_cref().unwrap_or(paragraph.props);
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