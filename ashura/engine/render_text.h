/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/canvas.h"
#include "ashura/engine/text.h"
#include "ashura/std/error.h"
#include "ashura/std/text.h"
#include "ashura/std/types.h"

namespace ash
{

/// @brief Controls and manages GUI text state for rendering
/// - manages runs and run styling
/// - manages and checks for text layout invalidation
/// - recalculate text layout when it changes if necessary
/// - renders the text using the computed style information
struct RenderText
{
  struct
  {
    bool             dirty : 1              = true;
    bool             use_kerning : 1        = true;
    bool             use_ligatures : 1      = true;
    TextDirection    direction : 2          = TextDirection::LeftToRight;
    f32              alignment              = -1;
    Vec<u32>         text                   = {};
    Vec<u32>         runs                   = {};
    Vec<TextStyle>   styles                 = {};
    Vec<FontStyle>   fonts                  = {};
    Span<char const> language               = {};
    TextLayout       layout                 = {};
    Vec<Slice32>     highlights             = {};
    ColorGradient    highlight_color        = {};
    Vec4             highlight_corner_radii = {};

    /// @brief  Styles specified runs of text, performing run merging and
    /// splitting in the process. If there's previously no runs, the first added
    /// run will be the default and span the whole of the text.
    /// @param first first codepoint index to be patched
    /// @param count range of the number of codepoints to be patched
    /// @param style font style to be applied
    /// @param font font configuration to be applied
    void style(u32 first, u32 count, TextStyle const &style,
               FontStyle const &font)
    {
      if (count == 0)
      {
        return;
      }

      if (runs.is_empty())
      {
        runs.push(U32_MAX).unwrap();
        styles.push(style).unwrap();
        fonts.push(font).unwrap();
        dirty = true;
        return;
      }

      u32 const end = sat_add(first, count);

      // if this becomes a bottleneck, might be improvable by using a binary
      // search. although more complex.
      u32 first_run       = 0;
      u32 first_run_begin = 0;
      for (; first_run < runs.size32(); first_run++)
      {
        if (runs[first_run] > first)
        {
          break;
        }
        first_run_begin = runs[first_run];
      }

      /// should never happen since there's always a U32_MAX run end
      CHECK(first_run < runs.size32());

      u32 last_run     = first_run;
      u32 last_run_end = 0;

      for (; last_run < runs.size32(); last_run++)
      {
        last_run_end = runs[last_run];
        if (last_run_end >= end)
        {
          break;
        }
      }

      /// should never happen since there's always a U32_MAX run end
      CHECK(last_run < runs.size32());

      /// run merging

      /// merge middle
      if (last_run > (first_run + 1))
      {
        u32 const first_erase = first_run + 1;
        u32 const num_erase   = last_run - first_erase;
        runs.erase(first_erase, num_erase);
        styles.erase(first_erase, num_erase);
        fonts.erase(first_erase, num_erase);
        last_run -= num_erase;
      }

      /// merge left
      if (first_run_begin == first)
      {
        u32 const first_erase = first_run;
        u32 const num_erase   = last_run - first_run;
        runs.erase(first_erase, num_erase);
        styles.erase(first_erase, num_erase);
        fonts.erase(first_erase, num_erase);
        last_run -= num_erase;
      }

      /// merge right
      if (last_run_end == end)
      {
        u32 const first_erase = first_run + 1;
        u32 const num_erase   = (last_run + 1) - first_erase;
        runs.erase(first_erase, num_erase);
        styles.erase(first_erase, num_erase);
        fonts.erase(first_erase, num_erase);
        last_run -= num_erase;
      }

      (void) last_run;

      /// run splitting
      if (!(first_run_begin == first && last_run_end == end))
      {
        if (first_run_begin == first)
        {
          // split with new on left
          runs.insert(first_run, end).unwrap();
          styles.insert(first_run, style).unwrap();
          fonts.insert(first_run, font).unwrap();
        }
        else if (last_run_end == end)
        {
          // split with new on right
          runs[first_run] = first;
          runs.insert(first_run + 1, end).unwrap();
          styles.insert(first_run + 1, style).unwrap();
          fonts.insert(first_run + 1, font).unwrap();
        }
        else
        {
          // split with new in the middle of the run
          runs[first_run] = first;
          runs.insert(first_run + 1, end).unwrap();
          styles.insert(first_run + 1, style).unwrap();
          fonts.insert(first_run + 1, font).unwrap();
          runs.insert(first_run + 2, last_run_end).unwrap();
          styles.insert(first_run + 2, styles[first_run]).unwrap();
          fonts.insert(first_run + 2, fonts[first_run]).unwrap();
        }
      }

      dirty = true;
    }
  } inner = {};

  void flush_text()
  {
    inner.dirty = true;
  }

  void set_highlights(Span<Slice32 const> highlight)
  {
    inner.highlights.clear();
    inner.highlights.extend_copy(highlight).unwrap();
  }

  void set_highlight(Slice32 highlight)
  {
    set_highlights(span({highlight}));
  }

  void set_highlight_style(ColorGradient color, Vec4 corner_radii)
  {
    inner.highlight_color        = color;
    inner.highlight_corner_radii = corner_radii;
  }

  void set_direction(TextDirection direction)
  {
    inner.direction = direction;
    flush_text();
  }

  void set_language(Span<char const> language)
  {
    inner.language = language;
    flush_text();
  }

  void set_alignment(f32 alignment)
  {
    inner.alignment = alignment;
    flush_text();
  }

  Span<u32 const> get_text() const
  {
    return span(inner.text);
  }

  void set_text(Span<u32 const> utf32, TextStyle const &style,
                FontStyle const &font)
  {
    inner.style(0, U32_MAX, style, font);
    set_text(utf32);
    flush_text();
  }

  void set_text(Span<u32 const> utf32)
  {
    inner.text.clear();
    inner.text.extend_copy(utf32).unwrap();
    flush_text();
  }

  void set_text(Span<u8 const> utf8, TextStyle const &style,
                FontStyle const &font)
  {
    inner.style(0, U32_MAX, style, font);
    set_text(utf8);
    flush_text();
  }

  void set_text(Span<u8 const> utf8)
  {
    inner.text.clear();
    utf8_decode(utf8, inner.text).unwrap();
    flush_text();
  }

  void style(u32 first, u32 count, TextStyle const &style,
             FontStyle const &font)
  {
    inner.style(first, count, style, font);
    flush_text();
  }

  TextBlock block() const
  {
    return TextBlock{.text          = span(inner.text),
                     .runs          = span(inner.runs),
                     .fonts         = span(inner.fonts),
                     .direction     = inner.direction,
                     .language      = inner.language,
                     .use_kerning   = inner.use_kerning,
                     .use_ligatures = inner.use_ligatures};
  }

  TextBlockStyle block_style(f32 aligned_width) const
  {
    return TextBlockStyle{.runs        = span(inner.styles),
                          .alignment   = inner.alignment,
                          .align_width = aligned_width};
  }

  void calculate_layout(f32 max_width)
  {
    if (!inner.dirty && max_width == inner.layout.extent.x)
    {
      return;
    }
    layout_text(block(), max_width, inner.layout);
  }

  void render(CRect const &region, CRect const &clip, Canvas &canvas) const
  {
    canvas.text(ShapeDesc{.center = region.center}, block(), inner.layout,
                block_style(region.extent.x), clip);
    // [ ] render
    // [ ] are the cursor indexes correct?
    // [ ] use overlays on intersecting graphemes
  }

  void reset()
  {
    inner.text.reset();
    inner.runs.reset();
    inner.styles.reset();
    inner.fonts.reset();
    inner.layout.reset();
  }
};

}        // namespace ash