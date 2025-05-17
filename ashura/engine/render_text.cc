/// SPDX-License-Identifier: MIT
#include "ashura/engine/render_text.h"
#include "ashura/engine/systems.h"

namespace ash
{

RenderText & RenderText::run(TextStyle const & style, FontStyle const & font,
                             usize first, usize count)
{
  if (count == 0)
  {
    return *this;
  }

  if (runs_.is_empty())
  {
    runs_.push(USIZE_MAX).unwrap();
    styles_.push(style).unwrap();
    fonts_.push(font).unwrap();
    hash_ = 0;
    return *this;
  }

  auto const end = sat_add(first, count);

  auto const first_run_span = binary_find(runs_.view(), gt, first);

  /// should never happen since there's always a USIZE_MAX run end
  CHECK(!first_run_span.is_empty(), "");

  auto const last_run_span = binary_find(first_run_span, geq, end);

  /// should never happen since there's always a USIZE_MAX run end
  CHECK(!last_run_span.is_empty(), "");

  auto first_run = (usize) (first_run_span.pbegin() - runs_.view().pbegin());
  auto last_run  = (usize) (last_run_span.pbegin() - runs_.view().pbegin());

  auto const first_run_begin = (first_run == 0) ? 0 : runs_[first_run - 1];
  auto const last_run_end    = runs_[last_run];

  /// run merging

  /// merge middle

  if (last_run > (first_run + 1))
  {
    auto const first_erase = first_run + 1;
    auto const num_erase   = last_run - first_erase;
    runs_.erase(first_erase, num_erase);
    styles_.erase(first_erase, num_erase);
    fonts_.erase(first_erase, num_erase);
    last_run -= num_erase;
  }

  /// merge left
  if (first_run_begin == first)
  {
    auto const first_erase = first_run;
    auto const num_erase   = last_run - first_run;
    runs_.erase(first_erase, num_erase);
    styles_.erase(first_erase, num_erase);
    fonts_.erase(first_erase, num_erase);
    last_run -= num_erase;
  }

  /// merge right
  if (last_run_end == end)
  {
    auto const first_erase = first_run + 1;
    auto const num_erase   = (last_run + 1) - first_erase;
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

  hash_ = HASH_DIRTY;

  return *this;
}

RenderText & RenderText::flush_text()
{
  hash_ = HASH_DIRTY;
  return *this;
}

RenderText & RenderText::wrap(bool wrap)
{
  wrap_ = wrap;
  return *this;
}

RenderText & RenderText::highlight_style(Option<TextHighlightStyle> style)
{
  highlight_style_ = style.unwrap_or();
  return *this;
}

RenderText & RenderText::caret_style(Option<CaretStyle> caret)
{
  caret_style_ = caret.unwrap_or();
  return *this;
}

RenderText & RenderText::add_highlight(Slice range)
{
  highlights_.push(range).unwrap();
  return *this;
}

RenderText & RenderText::clear_highlights()
{
  highlights_.clear();
  return *this;
}

RenderText & RenderText::add_caret(usize caret)
{
  carets_.push(caret).unwrap();
  return *this;
}

RenderText & RenderText::clear_carets()
{
  carets_.clear();
  return *this;
}

RenderText & RenderText::font_scale(f32 scale)
{
  font_scale_ = scale;
  return *this;
}

RenderText & RenderText::direction(TextDirection direction)
{
  if (direction_ == direction)
  {
    return *this;
  }
  direction_ = direction;
  flush_text();
  return *this;
}

RenderText & RenderText::language(Str language)
{
  if (range_eq(language_, language))
  {
    return *this;
  }
  language_ = language;
  flush_text();
  return *this;
}

RenderText & RenderText::alignment(f32 alignment)
{
  if (alignment_ == alignment)
  {
    return *this;
  }
  alignment_ = alignment;
  flush_text();
  return *this;
}

Str32 RenderText::get_text() const
{
  return text_;
}

RenderText & RenderText::text(Str32 utf32, TextStyle const & style,
                              FontStyle const & font)
{
  text(utf32);
  run(style, font);
  flush_text();
  return *this;
}

RenderText & RenderText::text(Str32 utf32)
{
  text_.clear();
  text_.extend(utf32).unwrap();
  flush_text();
  return *this;
}

RenderText & RenderText::text(Str8 utf8, TextStyle const & style,
                              FontStyle const & font)
{
  run(style, font);
  text(utf8);
  flush_text();
  return *this;
}

RenderText & RenderText::text(Str8 utf8)
{
  text_.clear();
  utf8_decode(utf8, text_).unwrap();
  flush_text();
  return *this;
}

TextBlock RenderText::block() const
{
  return TextBlock{.text          = text_,
                   .runs          = runs_,
                   .fonts         = fonts_,
                   .font_scale    = font_scale_,
                   .direction     = direction_,
                   .language      = language_,
                   .wrap          = wrap_,
                   .use_kerning   = use_kerning_,
                   .use_ligatures = use_ligatures_};
}

TextBlockStyle RenderText::block_style(f32 aligned_width) const
{
  return TextBlockStyle{.runs        = styles_,
                        .alignment   = alignment_,
                        .align_width = aligned_width,
                        .highlight   = highlight_style_,
                        .caret       = caret_style_};
}

TextLayout const & RenderText::get_layout() const
{
  return layout_;
}

void RenderText::layout(f32 max_width)
{
  if (hash_ == HASH_CLEAN && max_width == layout_.max_width)
  {
    return;
  }

  sys->font.layout_text(block(), max_width, layout_);
  hash_ = HASH_CLEAN;
}

void RenderText::render(TextRenderer renderer, Vec2 center, f32 align_width,
                        Mat4 const & transform, CRect const & clip,
                        AllocatorRef allocator)
{
  layout_.render(renderer, {.area{.center = center}, .transform = transform},
                 block(), block_style(align_width), highlights_, carets_, clip,
                 allocator);
}

Tuple<isize, CaretAlignment> RenderText::hit(Vec2 center, f32 align_width,
                                             Mat4 const & t, Vec2 pos) const
{
  auto s     = Vec2{t[0][0], t[1][1]};
  auto local = (pos - center) / s;
  return layout_.hit(block(), block_style(align_width), local);
}

}    // namespace ash
