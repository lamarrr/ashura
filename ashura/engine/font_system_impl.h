#pragma once
#include "ashura/engine/font_impl.h"
#include "ashura/engine/systems.h"

namespace ash
{

struct FontSystemImpl : FontSystem
{
  AllocatorRef                allocator_;
  SparseVec<Vec<Dyn<Font *>>> fonts_;
  hb_buffer_t *               hb_buffer_;

  explicit FontSystemImpl(AllocatorRef allocator, hb_buffer_t * hb_buffer) :
    allocator_{allocator},
    fonts_{allocator},
    hb_buffer_{hb_buffer}
  {
  }

  FontSystemImpl(FontSystemImpl const &)             = delete;
  FontSystemImpl(FontSystemImpl &&)                  = default;
  FontSystemImpl & operator=(FontSystemImpl const &) = delete;
  FontSystemImpl & operator=(FontSystemImpl &&)      = default;
  virtual ~FontSystemImpl() override;

  virtual void shutdown() override;

  Result<Dyn<Font *>, FontLoadErr>
    decode_(Span<char const> label, Span<u8 const> encoded, u32 face = 0);

  virtual Result<> rasterize(Font & font, u32 font_height) override;

  FontId upload_(Dyn<Font *> font);

  virtual void layout_text(TextBlock const & block, f32 max_width,
                           TextLayout & layout) override;

  virtual Future<Result<FontId, FontLoadErr>>
    load_from_memory(Vec<char> label, Vec<u8> encoded, u32 font_height,
                     u32 face = 0) override;

  virtual Future<Result<FontId, FontLoadErr>>
    load_from_path(Vec<char> label, Span<char const> path, u32 font_height,
                   u32 face = 0) override;

  virtual FontInfo get(FontId id) override;

  virtual FontInfo get(Span<char const> label) override;

  virtual void unload(FontId id) override;
};

}    // namespace ash
