

namespace ash
{
// TODO:
// tooltip
// repeat click hold down
// input int32, int64 with (- +)
// input vec4, vec3, vec2, f32, mat4, mat3
// drag *
// slider *
// tooltips with widget rendering onto them
// progress bar
// color pickers
//
// GRADIENT!
//
struct IntInput
{
  // + slider
  // signed, unsigned
  // int64
  // bounded, unbounded
};

struct FloatInput
{
  // + slider
  // signed, unsigned
  // float64
  // bounded, unbounded
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
  // on updated
  // on updating/ on typing with timeout
};

}        // namespace ash
