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
/// @param runs  Run-End encoded sequences of the runs
struct RenderText
{
  hash64           hash_;
  bool             use_kerning_   : 1;
  bool             use_ligatures_ : 1;
  TextDirection    direction_     : 2;
  f32              alignment_;
  f32              font_scale_;
  Vec<c32>         text_;
  Vec<u32>         runs_;
  Vec<TextStyle>   styles_;
  Vec<FontStyle>   fonts_;
  Span<char const> language_;
  TextLayout       layout_;
  TextHighlight    highlight_;

  RenderText(AllocatorRef allocator) :
    hash_{0},
    use_kerning_{true},
    use_ligatures_{true},
    direction_{TextDirection::LeftToRight},
    alignment_{-1},
    font_scale_{1},
    text_{allocator},
    runs_{allocator},
    styles_{allocator},
    fonts_{allocator},
    language_{},
    layout_{allocator},
    highlight_{}
  {
    layout_.hash = -1;
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

  RenderText & highlight(TextHighlight const & range);

  RenderText & clear_highlight();

  RenderText & font_scale(f32 scale);

  RenderText & direction(TextDirection direction);

  RenderText & language(Span<char const> language);

  RenderText & alignment(f32 alignment);

  Span<c32 const> get_text() const;

  RenderText & text(Span<c32 const> utf32, TextStyle const & style,
                    FontStyle const & font);

  RenderText & text(Span<c32 const> utf32);

  RenderText & text(Span<c8 const> utf8, TextStyle const & style,
                    FontStyle const & font);

  RenderText & text(Span<c8 const> utf8);

  TextBlock block() const;

  TextBlockStyle block_style(f32 aligned_width) const;

  TextLayout const & layout() const;

  void perform_layout(f32 max_width);

  void render(Canvas & canvas, CRect const & region, CRect const & clip,
              f32 zoom);

  Option<TextHitResult> hit(CRect const & region, Vec2 pos, f32 zoom) const;
};

}    // namespace ash
