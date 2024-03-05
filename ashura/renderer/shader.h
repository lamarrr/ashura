#pragma once
#include "ashura/gfx/gfx.h"
#include "ashura/std/mem.h"
#include "ashura/std/option.h"
#include "ashura/std/result.h"
#include "ashura/std/types.h"
#include <cstddef>

namespace ash
{

/// @name: parameter name
/// @type: only valid if is not uniform
/// @count: element count of the binding
/// @member_offset: offset of this member in the whole struct
/// @uniform_size: only valid if is uniform
/// @uniform_alignment: only valid if is uniform
struct ShaderBindingMetaData
{
  Span<char const>    name;
  gfx::DescriptorType type          = gfx::DescriptorType::Sampler;
  u16                 count         = 0;
  u16                 member_offset = 0;
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

#define SHADER_SAMPLER(BindingName, Count)                            \
  _METAMember_##BindingName;                                          \
                                                                      \
  static constexpr void _METApush(_METAMember_##BindingName,          \
                                  ::ash::ShaderBindingMetaData *meta) \
  {                                                                   \
    *meta = ::ash::ShaderBindingMetaData{                             \
        .name          = ::ash::to_span(#BindingName),                \
        .type          = ::ash::gfx::DescriptorType::Sampler,         \
        .count         = (u16) Count,                                 \
        .member_offset = (u16) offsetof(_METAThisType, BindingName)}; \
    _METApush(_METAMemberAfter_##BindingName{}, meta + 1);            \
  }                                                                   \
                                                                      \
  ::ash::gfx::SamplerBinding BindingName[Count];                      \
                                                                      \
  struct _METAMemberAfter_##BindingName                               \
  {                                                                   \
    static constexpr u16 _METAMemberIndex =                           \
        _METAMember_##BindingName::_METAMemberIndex + 1;              \
  };                                                                  \
                                                                      \
  typedef _METAMemberAfter_##BindingName

#define SHADER_COMBINED_IMAGE_SAMPLER(BindingName, Count)                  \
  _METAMember_##BindingName;                                               \
                                                                           \
  static constexpr void _METApush(_METAMember_##BindingName,               \
                                  ::ash::ShaderBindingMetaData *meta)      \
  {                                                                        \
    *meta = ::ash::ShaderBindingMetaData{                                  \
        .name          = ::ash::to_span(#BindingName),                     \
        .type          = ::ash::gfx::DescriptorType::CombinedImageSampler, \
        .count         = (u16) Count,                                      \
        .member_offset = (u16) offsetof(_METAThisType, BindingName)};      \
    _METApush(_METAMemberAfter_##BindingName{}, meta + 1);                 \
  }                                                                        \
                                                                           \
  ::ash::gfx::CombinedImageSamplerBinding BindingName[Count];              \
                                                                           \
  struct _METAMemberAfter_##BindingName                                    \
  {                                                                        \
    static constexpr u16 _METAMemberIndex =                                \
        _METAMember_##BindingName::_METAMemberIndex + 1;                   \
  };                                                                       \
                                                                           \
  typedef _METAMemberAfter_##BindingName

#define SHADER_SAMPLED_IMAGE(BindingName, Count)                      \
  _METAMember_##BindingName;                                          \
                                                                      \
  static constexpr void _METApush(_METAMember_##BindingName,          \
                                  ::ash::ShaderBindingMetaData *meta) \
  {                                                                   \
    *meta = ::ash::ShaderBindingMetaData{                             \
        .name          = ::ash::to_span(#BindingName),                \
        .type          = ::ash::gfx::DescriptorType::SampledImage,    \
        .count         = (u16) Count,                                 \
        .member_offset = (u16) offsetof(_METAThisType, BindingName)}; \
    _METApush(_METAMemberAfter_##BindingName{}, meta + 1);            \
  }                                                                   \
                                                                      \
  ::ash::gfx::SampledImageBinding BindingName[Count];                 \
                                                                      \
  struct _METAMemberAfter_##BindingName                               \
  {                                                                   \
    static constexpr u16 _METAMemberIndex =                           \
        _METAMember_##BindingName::_METAMemberIndex + 1;              \
  };                                                                  \
                                                                      \
  typedef _METAMemberAfter_##BindingName

#define SHADER_UNIFORM_TEXEL_BUFFER(BindingName, Count)                  \
  _METAMember_##BindingName;                                             \
                                                                         \
  static constexpr void _METApush(_METAMember_##BindingName,             \
                                  ::ash::ShaderBindingMetaData *meta)    \
  {                                                                      \
    *meta = ::ash::ShaderBindingMetaData{                                \
        .name          = ::ash::to_span(#BindingName),                   \
        .type          = ::ash::gfx::DescriptorType::UniformTexelBuffer, \
        .count         = (u16) Count,                                    \
        .member_offset = (u16) offsetof(_METAThisType, BindingName)};    \
    _METApush(_METAMemberAfter_##BindingName{}, meta + 1);               \
  }                                                                      \
                                                                         \
  ::ash::gfx::UniformTexelBufferBinding BindingName[Count];              \
                                                                         \
  struct _METAMemberAfter_##BindingName                                  \
  {                                                                      \
    static constexpr u16 _METAMemberIndex =                              \
        _METAMember_##BindingName::_METAMemberIndex + 1;                 \
  };                                                                     \
                                                                         \
  typedef _METAMemberAfter_##BindingName

#define SHADER_STORAGE_TEXEL_BUFFER(BindingName, Count)                  \
  _METAMember_##BindingName;                                             \
                                                                         \
  static constexpr void _METApush(_METAMember_##BindingName,             \
                                  ::ash::ShaderBindingMetaData *meta)    \
  {                                                                      \
    *meta = ::ash::ShaderBindingMetaData{                                \
        .name          = ::ash::to_span(#BindingName),                   \
        .type          = ::ash::gfx::DescriptorType::StorageTexelBuffer, \
        .count         = (u16) Count,                                    \
        .member_offset = (u16) offsetof(_METAThisType, BindingName)};    \
    _METApush(_METAMemberAfter_##BindingName{}, meta + 1);               \
  }                                                                      \
                                                                         \
  ::ash::gfx::StorageTexelBufferBinding BindingName[Count];              \
                                                                         \
  struct _METAMemberAfter_##BindingName                                  \
  {                                                                      \
    static constexpr u16 _METAMemberIndex =                              \
        _METAMember_##BindingName::_METAMemberIndex + 1;                 \
  };                                                                     \
                                                                         \
  typedef _METAMemberAfter_##BindingName

#define SHADER_UNIFORM_BUFFER(BindingName, Count)                     \
  _METAMember_##BindingName;                                          \
                                                                      \
  static constexpr void _METApush(_METAMember_##BindingName,          \
                                  ::ash::ShaderBindingMetaData *meta) \
  {                                                                   \
    *meta = ::ash::ShaderBindingMetaData{                             \
        .name          = ::ash::to_span(#BindingName),                \
        .type          = ::ash::gfx::DescriptorType::UniformBuffer,   \
        .count         = (u16) Count,                                 \
        .member_offset = (u16) offsetof(_METAThisType, BindingName)}; \
    _METApush(_METAMemberAfter_##BindingName{}, meta + 1);            \
  }                                                                   \
                                                                      \
  ::ash::gfx::UniformBufferBinding BindingName[Count];                \
                                                                      \
  struct _METAMemberAfter_##BindingName                               \
  {                                                                   \
    static constexpr u16 _METAMemberIndex =                           \
        _METAMember_##BindingName::_METAMemberIndex + 1;              \
  };                                                                  \
                                                                      \
  typedef _METAMemberAfter_##BindingName

#define SHADER_STORAGE_BUFFER(BindingName, Count)                     \
  _METAMember_##BindingName;                                          \
                                                                      \
  static constexpr void _METApush(_METAMember_##BindingName,          \
                                  ::ash::ShaderBindingMetaData *meta) \
  {                                                                   \
    *meta = ::ash::ShaderBindingMetaData{                             \
        .name          = ::ash::to_span(#BindingName),                \
        .type          = ::ash::gfx::DescriptorType::StorageBuffer,   \
        .count         = (u16) Count,                                 \
        .member_offset = (u16) offsetof(_METAThisType, BindingName)}; \
    _METApush(_METAMemberAfter_##BindingName{}, meta + 1);            \
  }                                                                   \
                                                                      \
  ::ash::gfx::StorageBufferBinding BindingName[Count];                \
                                                                      \
  struct _METAMemberAfter_##BindingName                               \
  {                                                                   \
    static constexpr u16 _METAMemberIndex =                           \
        _METAMember_##BindingName::_METAMemberIndex + 1;              \
  };                                                                  \
                                                                      \
  typedef _METAMemberAfter_##BindingName

#define SHADER_DYNAMIC_UNIFORM_BUFFER(BindingName, Count)                  \
  _METAMember_##BindingName;                                               \
                                                                           \
  static constexpr void _METApush(_METAMember_##BindingName,               \
                                  ::ash::ShaderBindingMetaData *meta)      \
  {                                                                        \
    *meta = ::ash::ShaderBindingMetaData{                                  \
        .name          = ::ash::to_span(#BindingName),                     \
        .type          = ::ash::gfx::DescriptorType::DynamicUniformBuffer, \
        .count         = (u16) Count,                                      \
        .member_offset = (u16) offsetof(_METAThisType, BindingName)};      \
    _METApush(_METAMemberAfter_##BindingName{}, meta + 1);                 \
  }                                                                        \
                                                                           \
  ::ash::gfx::DynamicUniformBufferBinding BindingName[Count];              \
                                                                           \
  struct _METAMemberAfter_##BindingName                                    \
  {                                                                        \
    static constexpr u16 _METAMemberIndex =                                \
        _METAMember_##BindingName::_METAMemberIndex + 1;                   \
  };                                                                       \
                                                                           \
  typedef _METAMemberAfter_##BindingName

#define SHADER_DYNAMIC_STORAGE_BUFFER(BindingName, Count)                  \
  _METAMember_##BindingName;                                               \
                                                                           \
  static constexpr void _METApush(_METAMember_##BindingName,               \
                                  ::ash::ShaderBindingMetaData *meta)      \
  {                                                                        \
    *meta = ::ash::ShaderBindingMetaData{                                  \
        .name          = ::ash::to_span(#BindingName),                     \
        .type          = ::ash::gfx::DescriptorType::DynamicStorageBuffer, \
        .count         = (u16) Count,                                      \
        .member_offset = (u16) offsetof(_METAThisType, BindingName)};      \
    _METApush(_METAMemberAfter_##BindingName{}, meta + 1);                 \
  }                                                                        \
                                                                           \
  ::ash::gfx::DynamicStorageBufferBinding BindingName[Count];              \
                                                                           \
  struct _METAMemberAfter_##BindingName                                    \
  {                                                                        \
    static constexpr u16 _METAMemberIndex =                                \
        _METAMember_##BindingName::_METAMemberIndex + 1;                   \
  };                                                                       \
                                                                           \
  typedef _METAMemberAfter_##BindingName

#define SHADER_INPUT_ATTACHMENT(BindingName, Count)                   \
  _METAMember_##BindingName;                                          \
                                                                      \
  static constexpr void _METApush(_METAMember_##BindingName,          \
                                  ::ash::ShaderBindingMetaData *meta) \
  {                                                                   \
    *meta = ::ash::ShaderBindingMetaData{                             \
        .name          = ::ash::to_span(#BindingName),                \
        .type          = ::ash::gfx::DescriptorType::InputAttachment, \
        .count         = (u16) Count,                                 \
        .member_offset = (u16) offsetof(_METAThisType, BindingName)}; \
    _METApush(_METAMemberAfter_##BindingName{}, meta + 1);            \
  }                                                                   \
                                                                      \
  ::ash::gfx::InputAttachmentBinding BindingName[Count];              \
                                                                      \
  struct _METAMemberAfter_##BindingName                               \
  {                                                                   \
    static constexpr u16 _METAMemberIndex =                           \
        _METAMember_##BindingName::_METAMemberIndex + 1;              \
  };                                                                  \
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
END_SHADER_PARAMETER(PBRParameter)

// TODO(lamarrr): use shader setter and getter instead, get layout, get bindings
// TODO(lamarrr): !!!uniform buffer setup?????
//
// TODO(lamarrr): global deletion queue in render context?
// TODO(lamarrr): automatic uniform buffer setup?
// get buffer descriptionsm and batch all parameters together into a single
// buffer
//

// MULTIPLE SETS? i.e. per-pass parameter INLINE SHADER PARAMETERS?
// - create parameter manager/allocator
template <typename Param>
struct ShaderParameterHeap
{
  static constexpr Array BINDINGS = PBRParameter::get_bindings();

  void init(gfx::DeviceImpl device, u32 batch_size)
  {
    gfx::DescriptorBindingDesc descs[PBRParameter::NUM_BINDINGS];
    for (u16 i = 0; i < PBRParameter::NUM_BINDINGS; i++)
    {
      descs[i] = gfx::DescriptorBindingDesc{.type  = members[i].type,
                                            .count = members[i].count};
    }
    gfx::DescriptorSetLayout layout =
        device_
            ->create_descriptor_set_layout(
                device_.self,
                gfx::DescriptorSetLayoutDesc{.label    = PBRParameter::NAME,
                                             .bindings = to_span(descs)})
            .unwrap();
    heap_ = device_
                ->create_descriptor_heap(device_.self, {&layout, 1}, batch_size,
                                         heap_allocator)
                .unwrap();
  }

  void deinit();

  Option<gfx::DescriptorSet> create_descriptor(PBRParameter const &param)
  {
    // allocate uniform buffer slot
    // allocate descriptor group
    u32 group = heap_->add_group(heap_.self, 0).unwrap();
    return Some{
        gfx::DescriptorSet{.heap = heap_.self, .group = group, .set = 0}};
  }

  void update_bindings(u32 id, PBRParameter const &param)
  {
    for (ShaderBindingMetaData const &member : BINDINGS)
    {
      switch (member.type)
      {
        case gfx::DescriptorType::CombinedImageSampler:
        {
          heap_->combined_image_sampler(
              heap_.self, desc.set.group, desc.set.set, 0,
              Span{(gfx::CombinedImageSampler const *) (((u8 const *) &param) +
                                                        member.member_offset),
                   member.count});
        }
        break;
        case gfx::DescriptorType::DynamicStorageBuffer:
        {
          heap_->dynamic_storage_buffer(
              heap_.self, desc.set.group, desc.set.set, 0,
              Span{(gfx::DynamicStorageBufferBinding const
                        *) (((u8 const *) &param) + member.member_offset),
                   member.count});
        }
        break;
        case gfx::DescriptorType::DynamicUniformBuffer:
        {
          heap_->dynamic_uniform_buffer(
              heap_.self, desc.set.group, desc.set.set, 0,
              Span{(gfx::DynamicUniformBufferBinding const
                        *) (((u8 const *) &param) + member.member_offset),
                   member.count});
        }
        break;
        case gfx::DescriptorType::InputAttachment:
        {
          heap_->input_attachment(
              heap_.self, desc.set.group, desc.set.set, 0,
              Span{
                  (gfx::InputAttachmentBinding const *) (((u8 const *) &param) +
                                                         member.member_offset),
                  member.count});
        }
        break;
        case gfx::DescriptorType::SampledImage:
        {
          heap_->sampled_image(
              heap_.self, desc.set.group, desc.set.set, 0,
              Span{(gfx::SampledImageBinding const *) (((u8 const *) &param) +
                                                       member.member_offset),
                   member.count});
        }
        break;
        case gfx::DescriptorType::Sampler:
        {
          heap_->sampler(
              heap_.self, desc.set.group, desc.set.set, 0,
              Span{(gfx::SamplerBinding const *) (((u8 const *) &param) +
                                                  member.member_offset),
                   member.count});
        }
        break;
        case gfx::DescriptorType::StorageBuffer:
        {
          heap_->storage_buffer(
              heap_.self, desc.set.group, desc.set.set, 0,
              Span{(gfx::StorageBufferBinding const *) (((u8 const *) &param) +
                                                        member.member_offset),
                   member.count});
        }
        break;
        case gfx::DescriptorType::StorageImage:
        {
          heap_->storage_image(
              heap_.self, desc.set.group, desc.set.set, 0,
              Span{(gfx::StorageImageBinding const *) (((u8 const *) &param) +
                                                       member.member_offset),
                   member.count});
        }
        break;
        case gfx::DescriptorType::StorageTexelBuffer:
        {
          heap_->storage_texel_buffer(
              heap_.self, desc.set.group, desc.set.set, 0,
              Span{(gfx::StorageTexelBufferBinding const
                        *) (((u8 const *) &param) + member.member_offset),
                   member.count});
        }
        break;
        case gfx::DescriptorType::UniformBuffer:
        {
          heap_->uniform_buffer(
              heap_.self, desc.set.group, desc.set.set, 0,
              Span{(gfx::UniformBufferBinding const *) (((u8 const *) &param) +
                                                        member.member_offset),
                   member.count});
        }
        break;
        case gfx::DescriptorType::UniformTexelBuffer:
        {
          heap_->uniform_texel_buffer(
              heap_.self, desc.set.group, desc.set.set, 0,
              Span{(gfx::UniformTexelBufferBinding const
                        *) (((u8 const *) &param) + member.member_offset),
                   member.count});
        }
        break;
        default:
          break;
      }
    }
  }

  void release_descriptor(gfx::DescriptorSet set)
  {
    // UniformBuffers list, packed, aligned
    // use dynamic uniform buffer for uniforms
    heap_->release(heap_.self, set.group);
    // release slot
  }

  gfx::DescriptorHeapImpl heap_;
  gfx::DeviceImpl         device_;
};

struct Shader;

}        // namespace ash
