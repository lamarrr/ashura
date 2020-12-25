#pragma once

#include <utility>
#include "vulkan/vulkan.h"

// TODO: explore other vulkan headers

namespace vlk {
enum class AtrrType {
  f32,
  f32x1 = f32,
  f32x2,
  f32x3,
  f32x4,
  i32x3,
  u32x3,
  // TODO(lamarrr): implement more dtypes
  Unimplemented
};

constexpr VkFormat to_vulkan_dtype(AtrrType type) noexcept {
  // float (vec1): VK_FORMAT_R32_SFLOAT
  // vec2: VK_FORMAT_R32G32_SFLOAT
  // vec3: VK_FORMAT_R32G32B32_SFLOAT
  // vec4: VK_FORMAT_R32G32B32A32_SFLOAT

  // TODO(lamarrr): implement more dtypes

  switch (type) {
    case AtrrType::f32x1:
      return VK_FORMAT_R32_SFLOAT;
    case AtrrType::f32x2:
      return VK_FORMAT_R32G32_SFLOAT;
    case AtrrType::f32x3:
      return VK_FORMAT_R32G32B32_SFLOAT;
    case AtrrType::f32x4:
      return VK_FORMAT_R32G32B32A32_SFLOAT;
    case AtrrType::i32x3:
      return VK_FORMAT_R32G32B32_SINT;
    case AtrrType::u32x3:
      return VK_FORMAT_R32G32B32_UINT;
    default:
      return static_cast<VkFormat>(-1);
  }
}

template <typename T>
struct vk_attr_type {
  static constexpr auto type = AtrrType::Unimplemented;
};

template <>
struct vk_attr_type<float> {
  static constexpr auto type = AtrrType::f32;
};

template <>
struct vk_attr_type<float[1]> {
  static constexpr auto type = AtrrType::f32x1;
};

template <>
struct vk_attr_type<float[2]> {
  static constexpr auto type = AtrrType::f32x2;
};

template <>
struct vk_attr_type<float[3]> {
  static constexpr auto type = AtrrType::f32x3;
};

template <>
struct vk_attr_type<float[4]> {
  static constexpr auto type = AtrrType::f32x4;
};

constexpr size_t dtype_size(AtrrType type) noexcept {
  switch (type) {
    case AtrrType::f32x1:
      return sizeof(float);
    case AtrrType::f32x2:
      return sizeof(float[2]);
    case AtrrType::f32x3:
      return sizeof(float[3]);
    case AtrrType::f32x4:
      return sizeof(float[4]);
    case AtrrType::i32x3:
      return sizeof(int32_t[3]);
    case AtrrType::u32x3:
      return sizeof(uint32_t[3]);
    default:
      return 0;
  }
}

constexpr size_t location_increment(AtrrType type) noexcept {
  switch (type) {
    case AtrrType::f32x1:
      return 1;
    case AtrrType::f32x2:
      return 1;
    case AtrrType::f32x3:
      return 1;
    case AtrrType::f32x4:
      return 1;
    case AtrrType::i32x3:
      return 1;
    case AtrrType::u32x3:
      return 1;
    default:
      return 0;
  }
  // TODO(lamarrr): not valid for matrices, double (f64) and others.
}

constexpr void fill_packed_vertex_input_attributes_description(
    [[maybe_unused]] stx::Span<VkVertexInputAttributeDescription> const&
        attribute_descriptions,
    [[maybe_unused]] uint32_t binding, [[maybe_unused]] uint32_t location,
    [[maybe_unused]] uint32_t bytes_offset,
    [[maybe_unused]] uint32_t index) noexcept {}

template <typename Head_AtrrType, typename... Tail_AtrrTypes>
constexpr void fill_packed_vertex_input_attributes_description(
    stx::Span<VkVertexInputAttributeDescription> const& attribute_descriptions,
    uint32_t binding, uint32_t location, uint32_t bytes_offset, uint32_t index,
    Head_AtrrType type, Tail_AtrrTypes... tail_types) noexcept {
  attribute_descriptions[index].binding = binding;
  attribute_descriptions[index].location = location;
  attribute_descriptions[index].format = to_vulkan_dtype(type);
  attribute_descriptions[index].offset = bytes_offset;

  fill_packed_vertex_input_attributes_description(
      attribute_descriptions, binding, location + location_increment(type),
      bytes_offset + dtype_size(type), index + 1, tail_types...);
}

template <typename... AttributeTypes>
constexpr std::array<VkVertexInputAttributeDescription,
                     sizeof...(AttributeTypes)>
make_packed_vertex_input_attributes_description(
    uint32_t binding, uint32_t start_location = 0) noexcept {
  std::array<VkVertexInputAttributeDescription, sizeof...(AttributeTypes)>
      descriptions{};

  fill_packed_vertex_input_attributes_description(
      descriptions, binding, start_location, 0, 0,
      vk_attr_type<AttributeTypes>::type...);
  return descriptions;
}

static constexpr VkVertexInputBindingDescription
make_vertex_input_binding_description(uint32_t binding,
                                      VkVertexInputRate input_rate,
                                      uint32_t stride) noexcept {
  VkVertexInputBindingDescription binding_desciption{};

  binding_desciption.binding = binding;
  // VK_VERTEX_INPUT_RATE_VERTEX: Move to the next data entry after each
  // vertex (per-vertex data)
  // VK_VERTEX_INPUT_RATE_INSTANCE: Move to the next data entry after each
  // instance (per-instance data)
  binding_desciption.inputRate = input_rate;
  binding_desciption.stride = stride;

  return binding_desciption;
}

template <typename T>
inline constexpr size_t vlk_size() noexcept {
  return sizeof(T);
}

template <typename... T>
inline constexpr size_t packed_bytes_size() noexcept {
  return (0 + ... + vlk_size<T>());
}

template <typename... AttributeTypes>
struct [[nodiscard]] PackedVertexInput {
  constexpr PackedVertexInput(uint32_t binding, VkVertexInputRate input_rate,
                              uint32_t start_location = 0,
                              size_t bytes_to_skip = 0)
      : attributes_description_{make_packed_vertex_input_attributes_description<
            AttributeTypes...>(binding, start_location)},
        binding_description_{make_vertex_input_binding_description(
            binding, input_rate,
            packed_bytes_size<AttributeTypes...>() + bytes_to_skip)} {}

  constexpr size_t size_bytes() const noexcept {
    return packed_bytes_size<AttributeTypes...>();
  }

  constexpr auto get_binding_description() const noexcept {
    return binding_description_;
  }

  constexpr auto get_attributes_description() const noexcept {
    return attributes_description_;
  }

  template <typename T>
  constexpr bool size_matches() const noexcept {
    return sizeof(T) == size_bytes();
  }

  std::array<VkVertexInputAttributeDescription, sizeof...(AttributeTypes)>
      attributes_description_;
  VkVertexInputBindingDescription binding_description_;
};

#define VLK_VEC1_F32_ALIGN alignas(4)
#define VLK_VEC2_F32_ALIGN alignas(4 * 2)
#define VLK_VEC3_F32_ALIGN alignas(4 * 3)
#define VLK_VEC4_F32_ALIGN alignas(4 * 4)
#define VLK_MAT4_F32_ALIGN VLK_VEC4_F32_ALIGN

#define VLK_MAT4_F32(field_name) VLK_MAT4_F32_ALIGN float field_name[4 * 4]
#define VLK_VEC2_F32(field_name) VLK_VEC2_F32_ALIGN float field_name[2]
#define VLK_VEC3_F32(field_name) VLK_VEC3_F32_ALIGN float field_name[3]

struct ProjectionParameters {
  VLK_VEC2_F32(reserved);
  VLK_MAT4_F32(model);
  VLK_MAT4_F32(view);
  VLK_MAT4_F32(projection);
};

}  // namespace vlk
