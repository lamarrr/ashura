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
/// - recalculates text layout when it changes and if necessary
/// - renders the text using the computed style information
/// @param runs  Run-End encoded sequences of the runs
struct RenderText
{
  static constexpr hash64 HASH_CLEAN = U64_MAX;
  static constexpr hash64 HASH_DIRTY = 0;

  hash64             hash_;
  bool               wrap_;
  bool               use_kerning_   : 1;
  bool               use_ligatures_ : 1;
  TextDirection      direction_     : 2;
  f32                alignment_;
  f32                font_scale_;
  Vec<c32>           text_;
  Vec<usize>         runs_;
  Vec<TextStyle>     styles_;
  Vec<FontStyle>     fonts_;
  Str                language_;
  TextLayout         layout_;
  TextHighlightStyle highlight_style_;
  CaretStyle         caret_style_;
  Vec<usize>         carets_;
  Vec<Slice>         highlights_;

  RenderText(Allocator allocator) :
    hash_{HASH_DIRTY},
    wrap_{true},
    use_kerning_{true},
    use_ligatures_{true},
    direction_{TextDirection::LeftToRight},
    alignment_{ALIGNMENT_LEFT},
    font_scale_{1},
    text_{allocator},
    runs_{allocator},
    styles_{allocator},
    fonts_{allocator},
    language_{},
    layout_{allocator},
    highlight_style_{},
    caret_style_{},
    carets_{allocator},
    highlights_{allocator}
  {
  }

  RenderText(RenderText const &)             = delete;
  RenderText & operator=(RenderText const &) = delete;
  RenderText(RenderText &&)                  = default;
  RenderText & operator=(RenderText &&)      = default;
  ~RenderText()                              = default;

  /// @brief  Styles specified runs of text, performing run merging and
  /// splitting in the process. If there's previously no runs, the first added
  /// run will be the default and span the whole of the text.
  /// @param first first codepoint index to be patched
  /// @param count range of the number of codepoints to be patched
  /// @param style font style to be applied
  /// @param font font configuration to be applied
  RenderText & run(TextStyle const & style, FontStyle const & font,
                   usize first = 0, usize count = USIZE_MAX);

  RenderText & flush_text();

  RenderText & wrap(bool wrap);

  RenderText & highlight_style(Option<TextHighlightStyle> style);

  RenderText & caret_style(Option<CaretStyle> caret);

  RenderText & add_highlight(Slice range);

  RenderText & clear_highlights();

  RenderText & add_caret(usize carey);

  RenderText & clear_carets();

  RenderText & font_scale(f32 scale);

  RenderText & direction(TextDirection direction);

  RenderText & language(Str language);

  RenderText & alignment(f32 alignment);

  Str32 get_text() const;

  RenderText & text(Str32 utf32, TextStyle const & style,
                    FontStyle const & font);

  RenderText & text(Str32 utf32);

  RenderText & text(Str8 utf8, TextStyle const & style, FontStyle const & font);

  RenderText & text(Str8 utf8);

  usize size() const
  {
    return text_.size();
  }

  TextBlock block() const;

  TextBlockStyle block_style(f32 aligned_width) const;

  TextLayout const & get_layout() const;

  void layout(f32 max_width);

  /// @brief Render the laid out text
  /// @param center canvas-space region of the text to place the text on
  /// @param align_width the width to align the text to
  /// @param clip the canvas-space clip rectangle
  /// @param zoom the zoom to apply to the text
  void render(TextRenderer renderer, f32x2 center, f32 align_width,
              f32x4x4 const & transform, CRect const & clip = MAX_CLIP,
              Allocator allocator = default_allocator);

  /// @brief Perform hit test on the laid-out text
  /// @param center canvas-space region the text was placed on
  /// @param align_width the width the text was aligned to
  /// @param zoom the zoom that was applied to the text
  /// @param pos the canvas-space text position to hit
  /// @returns .v0: caret index, .v1: caret location
  Tuple<isize, CaretAlignment> hit(f32x2 center, f32 align_width,
                                   f32x4x4 const & transform,
                                   f32x2           transformed_pos) const;
};

}    // namespace ash
