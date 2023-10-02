#pragma once
#include "ashura/primitives.h"
#include <cstdio>

namespace ash
{
namespace gui
{
// TODO:
// tooltip
// repeat click hold down
// input Vec4, vec3, vec2, f32, mat4, mat3
// tooltips with widget rendering onto them
// progress bar
// color pickers
//
// GRADIENT!
//

// TODO(lamarrr): IME and Controller support???

#define ASH_DECLARE_NUM_INPUT_SPEC(TypeName, type, scanfmt, displayfmt) \
  struct TypeName##InputSpec                                            \
  {                                                                     \
    char const *scan_format    = scanfmt;                               \
    char const *display_format = displayfmt;                            \
    alignas(8) type value      = 0;                                     \
    alignas(8) type step       = 1;                                     \
    alignas(8) type min        = std::numeric_limits<type>::lowest();   \
    alignas(8) type max        = std::numeric_limits<type>::max();      \
  };

ASH_DECLARE_NUM_INPUT_SPEC(U8, u8, "%" PRIu8, "%" PRIu8)
ASH_DECLARE_NUM_INPUT_SPEC(U16, u16, "%" PRIu16, "%" PRIu16)
ASH_DECLARE_NUM_INPUT_SPEC(U32, u32, "%" PRIu32, "%" PRIu32)
ASH_DECLARE_NUM_INPUT_SPEC(U64, u64, "%" PRIu64, "%" PRIu64)
ASH_DECLARE_NUM_INPUT_SPEC(I8, i8, "%" PRIi8, "%" PRIi8)
ASH_DECLARE_NUM_INPUT_SPEC(I16, i16, "%" PRIi16, "%" PRIi16)
ASH_DECLARE_NUM_INPUT_SPEC(I32, i32, "%" PRIi32, "%" PRIi32)
ASH_DECLARE_NUM_INPUT_SPEC(I64, i64, "%" PRIi64, "%" PRIi64)
ASH_DECLARE_NUM_INPUT_SPEC(F32, f32, "%.2f", "%.2f")
ASH_DECLARE_NUM_INPUT_SPEC(F64, f64, "%.2lf", "%.2lf")

enum class NumInputFlags : u8
{
  None           = 0,
  AddStepButtons = 1,
  AddDragger     = 2,
  Disabled       = 16,
  Default        = AddStepButtons | AddDragger
};

STX_DEFINE_ENUM_BIT_OPS(NumInputFlags)

enum class NumType : u8
{
  F32,
  F64,
  I8,
  I16,
  I32,
  I64,
  U8,
  U16,
  U32,
  U64
};

// width??? min?, will translate and crop on typing
// 2px padding for highlighting
struct NumInputProps
{
  NumInputFlags flags           = NumInputFlags::Default;
  f32           font_height     = 0;
  f32           padding         = 0;
  Color         button_color    = colors::TRANSPARENT;
  Color         symbol_color    = material::GRAY_300;
  Color         highlight_color = material::GRAY_300;
  Color         text_color      = material::GRAY_300;
  Color         text_box_color  = colors::TRANSPARENT;
};

enum class NumInputState
{
  Stale,
  FocusingInc,
  FocusingDec,
  FocusingText,
  Editing
};

// TODO(lamarrr): accept focus, esc to cancel, enter to increase
struct NumInput : public Widget
{
  NumInput(U8InputSpec spec, NumInputProps iprops) :
      u8spec{spec}, type{NumType::U8}, props{iprops}
  {}

  NumInput(U16InputSpec spec, NumInputProps iprops) :
      u16spec{spec}, type{NumType::U16}, props{iprops}
  {}

  NumInput(U32InputSpec spec, NumInputProps iprops) :
      u32spec{spec}, type{NumType::U32}, props{iprops}
  {}

  NumInput(U64InputSpec spec, NumInputProps iprops) :
      u64spec{spec}, type{NumType::U64}, props{iprops}
  {}

  NumInput(I8InputSpec spec, NumInputProps iprops) :
      i8spec{spec}, type{NumType::I8}, props{iprops}
  {}

  NumInput(I16InputSpec spec, NumInputProps iprops) :
      i16spec{spec}, type{NumType::I16}, props{iprops}
  {}

  NumInput(I32InputSpec spec, NumInputProps iprops) :
      i32spec{spec}, type{NumType::I32}, props{iprops}
  {}

  NumInput(I64InputSpec spec, NumInputProps iprops) :
      i64spec{spec}, type{NumType::I64}, props{iprops}
  {}

  NumInput(F32InputSpec spec, NumInputProps iprops) :
      f32spec{spec}, type{NumType::F32}, props{iprops}
  {}

  NumInput(F64InputSpec spec, NumInputProps iprops) :
      f64spec{spec}, type{NumType::F64}, props{iprops}
  {}

  STX_DISABLE_COPY(NumInput)
  STX_DEFAULT_MOVE(NumInput)

  virtual ~NumInput() override
  {}

  virtual Vec2 fit(Context &ctx, Vec2 allocated_size, stx::Span<Vec2 const> children_allocations, stx::Span<Vec2 const> children_sizes, stx::Span<Vec2> children_positions) override
  {
    char buff[256];
    int  written = std::snprintf(buff, 256, u8spec.display_format, (void *) &u8spec.value);
    // layout.layout();
  }

  virtual void draw(Context &ctx, gfx::Canvas &canvas) override
  {
  }

  virtual void tick(Context &ctx, std::chrono::nanoseconds interval) override
  {
  }

  virtual bool hit_test(Context &ctx, Vec2 mouse_position) override
  {
    return true;
  }

  virtual stx::Option<DragData> on_drag_start(Context &ctx, Vec2 mouse_position) override
  {
  }

  virtual void on_drag_update(Context &ctx, Vec2 mouse_position, Vec2 translation, DragData const &drag_data) override
  {
  }

  virtual void on_mouse_enter(Context &ctx, Vec2 mouse_position) override
  {
  }

  virtual void on_mouse_leave(Context &ctx, stx::Option<Vec2> mouse_position) override
  {
  }

  void change_spec(U8InputSpec spec)
  {
    u8spec = spec;
    type   = NumType::U8;
  }

  void change_spec(U16InputSpec spec)
  {
    u16spec = spec;
    type    = NumType::U16;
  }

  void change_spec(U32InputSpec spec)
  {
    u32spec = spec;
    type    = NumType::U32;
  }

  void change_spec(U64InputSpec spec)
  {
    u64spec = spec;
    type    = NumType::U64;
  }

  void change_spec(I8InputSpec spec)
  {
    i8spec = spec;
    type   = NumType::I8;
  }

  void change_spec(I16InputSpec spec)
  {
    i16spec = spec;
    type    = NumType::I16;
  }

  void change_spec(I32InputSpec spec)
  {
    i32spec = spec;
    type    = NumType::I32;
  }

  void change_spec(I64InputSpec spec)
  {
    i64spec = spec;
    type    = NumType::I64;
  }

  void change_spec(F32InputSpec spec)
  {
    f32spec = spec;
    type    = NumType::F32;
  }

  void change_spec(F64InputSpec spec)
  {
    f64spec = spec;
    type    = NumType::F64;
  }

  union
  {
    U8InputSpec  u8spec;
    U16InputSpec u16spec;
    U32InputSpec u32spec;
    U64InputSpec u64spec;
    I8InputSpec  i8spec;
    I16InputSpec i16spec;
    I32InputSpec i32spec;
    I64InputSpec i64spec;
    F32InputSpec f32spec;
    F64InputSpec f64spec;
  };
  NumType       type;
  NumInputProps props;
  TextLayout    layout;
};

struct VecInput
{
};

struct MatInput
{
};

struct ColorInput
{
  // HSL, YUV, RGBA, CMYK
};

struct TextInput
{
  // single line
  // secret
  // disabled
  // on updated
  // on updating/ on typing with timeout
};

}        // namespace gui
}        // namespace ash
