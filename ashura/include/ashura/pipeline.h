#pragma once
#include "ashura/primitives.h"

namespace ash
{

namespace gfx
{
using pipeline = u32;

};        // namespace gfx

constexpr std::string_view DEFAULT_SHAPE_PIPELINE = "BuiltinPipeline:Shape2D";
constexpr std::string_view DEFAULT_GLYPH_PIPELINE = "BuiltinPipeline:Glyph2D";

struct CanvasPipelineSpec
{
  std::string_view     name;                   /// name to use to recognise this pipeline
  stx::Span<u32 const> vertex_shader;          /// compiled SPIRV vertex shader
  stx::Span<u32 const> fragment_shader;        /// compiled SPIRV fragment shader
};

}        // namespace ash
