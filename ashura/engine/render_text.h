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
  Vec<u32>           runs_;
  Vec<TextStyle>     styles_;
  Vec<FontStyle>     fonts_;
  Str                language_;
  TextLayout         layout_;
  TextHighlightStyle highlight_style_;
  CaretStyle         caret_style_;
  Vec<isize>         carets_;
  Vec<Slice>         highlights_;

  RenderText(AllocatorRef allocator) :
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
  RenderText(RenderText &&)                  = delete;
  RenderText & operator=(RenderText &&)      = delete;
  ~RenderText()                              = default;

  /// @brief  Styles specified runs of text, performing run merging and
  /// splitting in the process. If there's previously no runs, the first added
  /// run will be the default and span the whole of the text.
  /// @param first first codepoint index to be patched
  /// @param count range of the number of codepoints to be patched
  /// @param style font style to be applied
  /// @param font font configuration to be applied
  RenderText & run(TextStyle const & style, FontStyle const & font,
                   u32 first = 0, u32 count = U32_MAX);

  RenderText & flush_text();

  RenderText & wrap(bool wrap);

  RenderText & highlight_style(Option<TextHighlightStyle> style);

  RenderText & caret_style(Option<CaretStyle> caret);

  RenderText & add_highlight(Slice range);

  RenderText & clear_highlights();

  RenderText & add_caret(isize carey);

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

  void render(Canvas & canvas, CRect const & region, CRect const & clip,
              f32 zoom);

  Tuple<isize, CaretLocation> hit(CRect const & region, Vec2 pos,
                                  f32 zoom) const;
};

}    // namespace ash
