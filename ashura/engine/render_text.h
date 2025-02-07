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
                   u32 first = 0, u32 count = U32_MAX)
  {
    if (count == 0)
    {
      return *this;
    }

    if (runs_.is_empty())
    {
      runs_.push(U32_MAX).unwrap();
      styles_.push(style).unwrap();
      fonts_.push(font).unwrap();
      hash_ = 0;
      return *this;
    }

    u32 const end = sat_add(first, count);

    Span const first_run_span = binary_find(runs_.view(), gt, first);

    /// should never happen since there's always a U32_MAX run end
    CHECK(!first_run_span.is_empty(), "");

    Span const last_run_span = binary_find(first_run_span, geq, end);

    /// should never happen since there's always a U32_MAX run end
    CHECK(!last_run_span.is_empty(), "");

    u32 first_run = (u32) (first_run_span.pbegin() - runs_.view().pbegin());
    u32 last_run  = (u32) (last_run_span.pbegin() - runs_.view().pbegin());

    u32 const first_run_begin = (first_run == 0) ? 0 : runs_[first_run - 1];
    u32 const last_run_end    = runs_[last_run];

    /// run merging

    /// merge middle

    if (last_run > (first_run + 1))
    {
      u32 const first_erase = first_run + 1;
      u32 const num_erase   = last_run - first_erase;
      runs_.erase(first_erase, num_erase);
      styles_.erase(first_erase, num_erase);
      fonts_.erase(first_erase, num_erase);
      last_run -= num_erase;
    }

    /// merge left
    if (first_run_begin == first)
    {
      u32 const first_erase = first_run;
      u32 const num_erase   = last_run - first_run;
      runs_.erase(first_erase, num_erase);
      styles_.erase(first_erase, num_erase);
      fonts_.erase(first_erase, num_erase);
      last_run -= num_erase;
    }

    /// merge right
    if (last_run_end == end)
    {
      u32 const first_erase = first_run + 1;
      u32 const num_erase   = (last_run + 1) - first_erase;
      runs_.erase(first_erase, num_erase);
      styles_.erase(first_erase, num_erase);
      fonts_.erase(first_erase, num_erase);
      last_run -= num_erase;
    }

    (void) last_run;

    /// run splitting
    if (first_run_begin == first && last_run_end == end)
    {
      styles_[first_run] = style;
      fonts_[first_run]  = font;
    }
    else
    {
      if (first_run_begin == first)
      {
        // split with new on left
        runs_.insert(first_run, end).unwrap();
        styles_.insert(first_run, style).unwrap();
        fonts_.insert(first_run, font).unwrap();
      }
      else if (last_run_end == end)
      {
        // split with new on right
        runs_[first_run] = first;
        runs_.insert(first_run + 1, end).unwrap();
        styles_.insert(first_run + 1, style).unwrap();
        fonts_.insert(first_run + 1, font).unwrap();
      }
      else
      {
        // split with new in the middle of the run
        runs_[first_run] = first;
        runs_.insert(first_run + 1, end).unwrap();
        styles_.insert(first_run + 1, style).unwrap();
        fonts_.insert(first_run + 1, font).unwrap();
        runs_.insert(first_run + 2, last_run_end).unwrap();
        styles_.insert(first_run + 2, styles_[first_run]).unwrap();
        fonts_.insert(first_run + 2, fonts_[first_run]).unwrap();
      }
    }

    hash_ = 0;

    return *this;
  }

  RenderText & flush_text()
  {
    hash_ = 0;
    return *this;
  }

  RenderText & highlight(TextHighlight const & range)
  {
    highlight_ = range;
    return *this;
  }

  RenderText & clear_highlight()
  {
    highlight_ = TextHighlight{};
    return *this;
  }

  RenderText & font_scale(f32 scale)
  {
    font_scale_ = scale;
    return *this;
  }

  RenderText & direction(TextDirection direction)
  {
    if (direction_ == direction)
    {
      return *this;
    }
    direction_ = direction;
    flush_text();
    return *this;
  }

  RenderText & language(Span<char const> language)
  {
    if (range_eq(language_, language))
    {
      return *this;
    }
    language_ = language;
    flush_text();
    return *this;
  }

  RenderText & alignment(f32 alignment)
  {
    if (alignment_ == alignment)
    {
      return *this;
    }
    alignment_ = alignment;
    flush_text();
    return *this;
  }

  Span<c32 const> get_text() const
  {
    return text_;
  }

  RenderText & text(Span<c32 const> utf32, TextStyle const & style,
                    FontStyle const & font)
  {
    text(utf32);
    run(style, font);
    flush_text();
    return *this;
  }

  RenderText & text(Span<c32 const> utf32)
  {
    text_.clear();
    text_.extend(utf32).unwrap();
    flush_text();
    return *this;
  }

  RenderText & text(Span<c8 const> utf8, TextStyle const & style,
                    FontStyle const & font)
  {
    run(style, font);
    text(utf8);
    flush_text();
    return *this;
  }

  RenderText & text(Span<c8 const> utf8)
  {
    text_.clear();
    utf8_decode(utf8, text_).unwrap();
    flush_text();
    return *this;
  }

  TextBlock block() const
  {
    return TextBlock{.text          = text_,
                     .hash          = hash_,
                     .runs          = runs_,
                     .fonts         = fonts_,
                     .font_scale    = font_scale_,
                     .direction     = direction_,
                     .language      = language_,
                     .use_kerning   = use_kerning_,
                     .use_ligatures = use_ligatures_};
  }

  TextBlockStyle block_style(f32 aligned_width) const
  {
    return TextBlockStyle{.runs        = styles_,
                          .alignment   = alignment_,
                          .align_width = aligned_width,
                          .highlight   = highlight_};
  }

  TextLayout const & layout() const
  {
    return layout_;
  }

  void perform_layout(f32 max_width)
  {
    if (hash_ == layout_.hash && max_width == layout_.max_width)
    {
      return;
    }

    hash_ = -1;
    sys->font.layout_text(block(), max_width, layout_);
  }

  void render(Canvas & canvas, CRect const & region, CRect const & clip,
              f32 zoom)
  {
    canvas.text(
      {.center = region.center, .transform = scale3d(Vec3::splat(zoom))},
      block(), layout_, block_style(region.extent.x), clip);
  }

  Option<TextHitResult> hit(CRect const & region, Vec2 pos, f32 zoom) const
  {
    Vec2 const pos_local = (region.begin() - pos) / zoom;
    return layout_.hit(block(), block_style(region.extent.x), pos_local);
  }
};

}    // namespace ash
