#pragma once
#include "ashura/vulkan_context.h"

namespace ash
{

// TODO(lamarrr): how do we allocate the descriptor set inputs?
struct CanvasPipelineSpec
{
  std::string_view                            name;                           // name to use to recognise this pipeline
  stx::Span<u32 const>                        vertex_shader;                  // compiled SPIRV vertex shader
  stx::Span<u32 const>                        fragment_shader;                // compiled SPIRV fragment shader
  stx::Vec<ash::vk::DescriptorSetSpec>        descriptor_sets_spec;           // description of the layout of the descriptor sets
  stx::Vec<VkVertexInputAttributeDescription> vertex_input_attributes;        // description of the vertex inputs to the pipeline's vertex shader
  u32                                         push_constant_size = 0;         // size of the push constant used in the fragment and vertex shader
};

void add_pipeline(CanvasPipelineSpec spec);

}        // namespace ash