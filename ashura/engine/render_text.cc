/// SPDX-License-Identifier: MIT
#include "ashura/engine/render_text.h"
#include "ashura/engine/systems.h"

namespace ash
{

RenderText & RenderText::run(TextStyle const & style, FontStyle const & font,
                             u32 first, u32 count)
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

RenderText & RenderText::flush_text()
{
  hash_ = 0;
  return *this;
}

RenderText & RenderText::highlight(TextHighlight const & range)
{
  highlight_ = range;
  return *this;
}

RenderText & RenderText::clear_highlight()
{
  highlight_ = TextHighlight{};
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
                   .hash          = hash_,
                   .runs          = runs_,
                   .fonts         = fonts_,
                   .font_scale    = font_scale_,
                   .direction     = direction_,
                   .language      = language_,
                   .use_kerning   = use_kerning_,
                   .use_ligatures = use_ligatures_};
}

TextBlockStyle RenderText::block_style(f32 aligned_width) const
{
  return TextBlockStyle{.runs        = styles_,
                        .alignment   = alignment_,
                        .align_width = aligned_width,
                        .highlight   = highlight_};
}

TextLayout const & RenderText::layout() const
{
  return layout_;
}

void RenderText::perform_layout(f32 max_width)
{
  if (hash_ == layout_.hash && max_width == layout_.max_width)
  {
    return;
  }

  hash_ = -1;
  sys->font.layout_text(block(), max_width, layout_);
}

void RenderText::render(Canvas & canvas, CRect const & region,
                        CRect const & clip, f32 zoom)
{
  canvas.text(
    {.center = region.center, .transform = scale3d(Vec3::splat(zoom))}, block(),
    layout_, block_style(region.extent.x), clip);
}

Option<TextHitResult> RenderText::hit(CRect const & region, Vec2 pos,
                                      f32 zoom) const
{
  Vec2 const local_pos = (pos - region.begin() - 0.5F * region.extent) / zoom;
  return layout_.hit(block(), block_style(region.extent.x), local_pos);
}

}    // namespace ash
