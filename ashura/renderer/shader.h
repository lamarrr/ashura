#pragma once
#include "ashura/gfx/gfx.h"
#include "ashura/std/option.h"
#include "ashura/std/result.h"
#include "ashura/std/types.h"
#include <cstddef>

namespace ash
{

// TODO(lamarrr): use shader setter and getter instead, get layout, get bindings
// TODO(lamarrr): !!!uniform buffer setup?????
//
// TODO(lamarrr): global deletion queue in render context?
// TODO(lamarrr): automatic uniform buffer setup?
// get buffer descriptionsm and batch all parameters together into a single
// buffer
//

/// @name: parameter name
/// @type: only valid if is not uniform
/// @count: element count of the binding
/// @member_offset: offset of this member in the whole struct
/// @uniform_size: only valid if is uniform
/// @uniform_alignment: only valid if is uniform
struct ShaderBindingMetaData
{
  Span<char const>    name;
  gfx::DescriptorType type              = gfx::DescriptorType::Sampler;
  u16                 count             = 0;
  u16                 member_offset     = 0;
  u16                 uniform_size      = 0;
  u16                 uniform_alignment = 0;
};

// Span<gfx::DescriptorBindingDesc const> bindings;
// dynamic offsets

struct ShaderParameterDescriptor
{
  gfx::DescriptorSet set            = {};
  Span<u64 const>    buffer_offsets = {};
  u32                uniform_index  = -1;
};

// MULTIPLE SETS? i.e. per-pass parameter INLINE SHADER PARAMETERS?
// - create parameter manager/allocator
template <typename Param>
struct ShaderParameterHeap
{
  // no stalling
  void                              init(u32 batch_size);
  void                              deinit();
  Option<ShaderParameterDescriptor> create_descriptor(Param const &param);
  void update(Param const *param, ShaderParameterDescriptor const &desc);
  void release_descriptor(ShaderParameterDescriptor const &desc);
  // UniformBuffers list, packed, aligned
  // use dynamic uniform buffer for uniforms
  gfx::DescriptorHeapImpl heap_;
  gfx::Buffer             uniform_buffers_[2];
};

#define BEGIN_SHADER_PARAMETER(Name)             \
  struct Name                                    \
  {                                              \
    typedef Name _METAThisType;                  \
                                                 \
    struct _METAMemberBegin                      \
    {                                            \
      static constexpr u16 _METAMemberIndex = 0; \
    };                                           \
                                                 \
    typedef _METAMemberBegin

#define SHADER_UNIFORM(StructType, BindingName, Count)                         \
  _METAMember_##BindingName;                                                   \
  static constexpr void _METApush(_METAMember_##BindingName,                   \
                                  ::ash::ShaderBindingMetaData *meta)          \
  {                                                                            \
    *meta = ::ash::ShaderBindingMetaData{                                      \
        .name              = ::ash::to_span(#BindingName),                     \
        .type              = ::ash::gfx::DescriptorType::DynamicUniformBuffer, \
        .count             = (u16) Count,                                      \
        .member_offset     = (u16) offsetof(_METAThisType, BindingName),       \
        .uniform_size      = sizeof(StructType),                               \
        .uniform_alignment = alignof(StructType)};                             \
    _METApush(_METAMemberAfter_##BindingName{}, meta + 1);                     \
  }                                                                            \
                                                                               \
  StructType BindingName[Count];                                               \
                                                                               \
  struct _METAMemberAfter_##BindingName                                        \
  {                                                                            \
    static constexpr u16 _METAMemberIndex =                                    \
        _METAMember_##BindingName::_METAMemberIndex + 1;                       \
  };                                                                           \
                                                                               \
  typedef _METAMemberAfter_##BindingName

#define SHADER_SAMPLER(BindingName, Count)                               \
  _METAMember_##BindingName;                                             \
                                                                         \
  static constexpr void _METApush(_METAMember_##BindingName,             \
                                  ::ash::ShaderBindingMetaData *meta)    \
  {                                                                      \
    *meta = ::ash::ShaderBindingMetaData{                                \
        .name              = ::ash::to_span(#BindingName),               \
        .type              = ::ash::gfx::DescriptorType::Sampler,        \
        .count             = (u16) Count,                                \
        .member_offset     = (u16) offsetof(_METAThisType, BindingName), \
        .uniform_size      = 0,                                          \
        .uniform_alignment = 0};                                         \
    _METApush(_METAMemberAfter_##BindingName{}, meta + 1);               \
  }                                                                      \
                                                                         \
  ::ash::gfx::SamplerBinding BindingName[Count];                         \
                                                                         \
  struct _METAMemberAfter_##BindingName                                  \
  {                                                                      \
    static constexpr u16 _METAMemberIndex =                              \
        _METAMember_##BindingName::_METAMemberIndex + 1;                 \
  };                                                                     \
                                                                         \
  typedef _METAMemberAfter_##BindingName

#define SHADER_COMBINED_IMAGE_SAMPLER(BindingName, Count)                      \
  _METAMember_##BindingName;                                                   \
                                                                               \
  static constexpr void _METApush(_METAMember_##BindingName,                   \
                                  ::ash::ShaderBindingMetaData *meta)          \
  {                                                                            \
    *meta = ::ash::ShaderBindingMetaData{                                      \
        .name              = ::ash::to_span(#BindingName),                     \
        .type              = ::ash::gfx::DescriptorType::CombinedImageSampler, \
        .count             = (u16) Count,                                      \
        .member_offset     = (u16) offsetof(_METAThisType, BindingName),       \
        .uniform_size      = 0,                                                \
        .uniform_alignment = 0};                                               \
    _METApush(_METAMemberAfter_##BindingName{}, meta + 1);                     \
  }                                                                            \
                                                                               \
  ::ash::gfx::CombinedImageSamplerBinding BindingName[Count];                  \
                                                                               \
  struct _METAMemberAfter_##BindingName                                        \
  {                                                                            \
    static constexpr u16 _METAMemberIndex =                                    \
        _METAMember_##BindingName::_METAMemberIndex + 1;                       \
  };                                                                           \
                                                                               \
  typedef _METAMemberAfter_##BindingName

#define SHADER_SAMPLED_IMAGE(BindingName, Count)                         \
  _METAMember_##BindingName;                                             \
                                                                         \
  static constexpr void _METApush(_METAMember_##BindingName,             \
                                  ::ash::ShaderBindingMetaData *meta)    \
  {                                                                      \
    *meta = ::ash::ShaderBindingMetaData{                                \
        .name              = ::ash::to_span(#BindingName),               \
        .type              = ::ash::gfx::DescriptorType::SampledImage,   \
        .count             = (u16) Count,                                \
        .member_offset     = (u16) offsetof(_METAThisType, BindingName), \
        .uniform_size      = 0,                                          \
        .uniform_alignment = 0};                                         \
    _METApush(_METAMemberAfter_##BindingName{}, meta + 1);               \
  }                                                                      \
                                                                         \
  ::ash::gfx::SampledImageBinding BindingName[Count];                    \
                                                                         \
  struct _METAMemberAfter_##BindingName                                  \
  {                                                                      \
    static constexpr u16 _METAMemberIndex =                              \
        _METAMember_##BindingName::_METAMemberIndex + 1;                 \
  };                                                                     \
                                                                         \
  typedef _METAMemberAfter_##BindingName

#define SHADER_UNIFORM_TEXEL_BUFFER(BindingName, Count)                      \
  _METAMember_##BindingName;                                                 \
                                                                             \
  static constexpr void _METApush(_METAMember_##BindingName,                 \
                                  ::ash::ShaderBindingMetaData *meta)        \
  {                                                                          \
    *meta = ::ash::ShaderBindingMetaData{                                    \
        .name              = ::ash::to_span(#BindingName),                   \
        .type              = ::ash::gfx::DescriptorType::UniformTexelBuffer, \
        .count             = (u16) Count,                                    \
        .member_offset     = (u16) offsetof(_METAThisType, BindingName),     \
        .uniform_size      = 0,                                              \
        .uniform_alignment = 0};                                             \
    _METApush(_METAMemberAfter_##BindingName{}, meta + 1);                   \
  }                                                                          \
                                                                             \
  ::ash::gfx::UniformTexelBufferBinding BindingName[Count];                  \
                                                                             \
  struct _METAMemberAfter_##BindingName                                      \
  {                                                                          \
    static constexpr u16 _METAMemberIndex =                                  \
        _METAMember_##BindingName::_METAMemberIndex + 1;                     \
  };                                                                         \
                                                                             \
  typedef _METAMemberAfter_##BindingName

#define SHADER_STORAGE_TEXEL_BUFFER(BindingName, Count)                      \
  _METAMember_##BindingName;                                                 \
                                                                             \
  static constexpr void _METApush(_METAMember_##BindingName,                 \
                                  ::ash::ShaderBindingMetaData *meta)        \
  {                                                                          \
    *meta = ::ash::ShaderBindingMetaData{                                    \
        .name              = ::ash::to_span(#BindingName),                   \
        .type              = ::ash::gfx::DescriptorType::StorageTexelBuffer, \
        .count             = (u16) Count,                                    \
        .member_offset     = (u16) offsetof(_METAThisType, BindingName),     \
        .uniform_size      = 0,                                              \
        .uniform_alignment = 0};                                             \
    _METApush(_METAMemberAfter_##BindingName{}, meta + 1);                   \
  }                                                                          \
                                                                             \
  ::ash::gfx::StorageTexelBufferBinding BindingName[Count];                  \
                                                                             \
  struct _METAMemberAfter_##BindingName                                      \
  {                                                                          \
    static constexpr u16 _METAMemberIndex =                                  \
        _METAMember_##BindingName::_METAMemberIndex + 1;                     \
  };                                                                         \
                                                                             \
  typedef _METAMemberAfter_##BindingName

#define SHADER_UNIFORM_BUFFER(BindingName, Count)                        \
  _METAMember_##BindingName;                                             \
                                                                         \
  static constexpr void _METApush(_METAMember_##BindingName,             \
                                  ::ash::ShaderBindingMetaData *meta)    \
  {                                                                      \
    *meta = ::ash::ShaderBindingMetaData{                                \
        .name              = ::ash::to_span(#BindingName),               \
        .type              = ::ash::gfx::DescriptorType::UniformBuffer,  \
        .count             = (u16) Count,                                \
        .member_offset     = (u16) offsetof(_METAThisType, BindingName), \
        .uniform_size      = 0,                                          \
        .uniform_alignment = 0};                                         \
    _METApush(_METAMemberAfter_##BindingName{}, meta + 1);               \
  }                                                                      \
                                                                         \
  ::ash::gfx::UniformBufferBinding BindingName[Count];                   \
                                                                         \
  struct _METAMemberAfter_##BindingName                                  \
  {                                                                      \
    static constexpr u16 _METAMemberIndex =                              \
        _METAMember_##BindingName::_METAMemberIndex + 1;                 \
  };                                                                     \
                                                                         \
  typedef _METAMemberAfter_##BindingName

#define SHADER_STORAGE_BUFFER(BindingName, Count)                        \
  _METAMember_##BindingName;                                             \
                                                                         \
  static constexpr void _METApush(_METAMember_##BindingName,             \
                                  ::ash::ShaderBindingMetaData *meta)    \
  {                                                                      \
    *meta = ::ash::ShaderBindingMetaData{                                \
        .name              = ::ash::to_span(#BindingName),               \
        .type              = ::ash::gfx::DescriptorType::StorageBuffer,  \
        .count             = (u16) Count,                                \
        .member_offset     = (u16) offsetof(_METAThisType, BindingName), \
        .uniform_size      = 0,                                          \
        .uniform_alignment = 0};                                         \
    _METApush(_METAMemberAfter_##BindingName{}, meta + 1);               \
  }                                                                      \
                                                                         \
  ::ash::gfx::StorageBufferBinding BindingName[Count];                   \
                                                                         \
  struct _METAMemberAfter_##BindingName                                  \
  {                                                                      \
    static constexpr u16 _METAMemberIndex =                              \
        _METAMember_##BindingName::_METAMemberIndex + 1;                 \
  };                                                                     \
                                                                         \
  typedef _METAMemberAfter_##BindingName

#define SHADER_DYNAMIC_UNIFORM_BUFFER(BindingName, Count)                      \
  _METAMember_##BindingName;                                                   \
                                                                               \
  static constexpr void _METApush(_METAMember_##BindingName,                   \
                                  ::ash::ShaderBindingMetaData *meta)          \
  {                                                                            \
    *meta = ::ash::ShaderBindingMetaData{                                      \
        .name              = ::ash::to_span(#BindingName),                     \
        .type              = ::ash::gfx::DescriptorType::DynamicUniformBuffer, \
        .count             = (u16) Count,                                      \
        .member_offset     = (u16) offsetof(_METAThisType, BindingName),       \
        .uniform_size      = 0,                                                \
        .uniform_alignment = 0};                                               \
    _METApush(_METAMemberAfter_##BindingName{}, meta + 1);                     \
  }                                                                            \
                                                                               \
  ::ash::gfx::DynamicUniformBufferBinding BindingName[Count];                  \
                                                                               \
  struct _METAMemberAfter_##BindingName                                        \
  {                                                                            \
    static constexpr u16 _METAMemberIndex =                                    \
        _METAMember_##BindingName::_METAMemberIndex + 1;                       \
  };                                                                           \
                                                                               \
  typedef _METAMemberAfter_##BindingName

#define SHADER_DYNAMIC_STORAGE_BUFFER(BindingName, Count)                      \
  _METAMember_##BindingName;                                                   \
                                                                               \
  static constexpr void _METApush(_METAMember_##BindingName,                   \
                                  ::ash::ShaderBindingMetaData *meta)          \
  {                                                                            \
    *meta = ::ash::ShaderBindingMetaData{                                      \
        .name              = ::ash::to_span(#BindingName),                     \
        .type              = ::ash::gfx::DescriptorType::DynamicStorageBuffer, \
        .count             = (u16) Count,                                      \
        .member_offset     = (u16) offsetof(_METAThisType, BindingName),       \
        .uniform_size      = 0,                                                \
        .uniform_alignment = 0};                                               \
    _METApush(_METAMemberAfter_##BindingName{}, meta + 1);                     \
  }                                                                            \
                                                                               \
  ::ash::gfx::DynamicStorageBufferBinding BindingName[Count];                  \
                                                                               \
  struct _METAMemberAfter_##BindingName                                        \
  {                                                                            \
    static constexpr u16 _METAMemberIndex =                                    \
        _METAMember_##BindingName::_METAMemberIndex + 1;                       \
  };                                                                           \
                                                                               \
  typedef _METAMemberAfter_##BindingName

#define SHADER_INPUT_ATTACHMENT(BindingName, Count)                       \
  _METAMember_##BindingName;                                              \
                                                                          \
  static constexpr void _METApush(_METAMember_##BindingName,              \
                                  ::ash::ShaderBindingMetaData *meta)     \
  {                                                                       \
    *meta = ::ash::ShaderBindingMetaData{                                 \
        .name              = ::ash::to_span(#BindingName),                \
        .type              = ::ash::gfx::DescriptorType::InputAttachment, \
        .count             = (u16) Count,                                 \
        .member_offset     = (u16) offsetof(_METAThisType, BindingName),  \
        .uniform_size      = 0,                                           \
        .uniform_alignment = 0};                                          \
    _METApush(_METAMemberAfter_##BindingName{}, meta + 1);                \
  }                                                                       \
                                                                          \
  ::ash::gfx::InputAttachmentBinding BindingName[Count];                  \
                                                                          \
  struct _METAMemberAfter_##BindingName                                   \
  {                                                                       \
    static constexpr u16 _METAMemberIndex =                               \
        _METAMember_##BindingName::_METAMemberIndex + 1;                  \
  };                                                                      \
                                                                          \
  typedef _METAMemberAfter_##BindingName

#define END_SHADER_PARAMETER(Name)                                             \
  _METAMemberEnd;                                                              \
                                                                               \
  static constexpr void _METApush(_METAMemberEnd,                              \
                                  ::ash::ShaderBindingMetaData *)              \
  {                                                                            \
  }                                                                            \
                                                                               \
  static constexpr char const NAME[]       = #Name;                            \
  static constexpr u16        NUM_BINDINGS = _METAMemberEnd::_METAMemberIndex; \
  static constexpr auto       get_bindings()                                   \
  {                                                                            \
    ::ash::Array<::ash::ShaderBindingMetaData, NUM_BINDINGS> bindings;         \
    _METApush(_METAMemberBegin{}, bindings);                                   \
    return bindings;                                                           \
  };                                                                           \
  }                                                                            \
  ;

BEGIN_SHADER_PARAMETER(PBRParameter)
SHADER_SAMPLER(Metallic, 1)
SHADER_SAMPLER(Albedo, 1)
SHADER_SAMPLER(AO, 1)
SHADER_UNIFORM(Vec4, MetallicFactor, 1)
SHADER_UNIFORM(Vec4, AlbedoFactor, 1)
SHADER_UNIFORM(Vec4, AOFactor, 1)
END_SHADER_PARAMETER(PBRParameter)

constexpr u16  x = PBRParameter::NUM_BINDINGS;
constexpr auto b = PBRParameter::get_bindings();
struct Shader;

}        // namespace ash
