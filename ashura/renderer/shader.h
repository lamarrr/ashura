#pragma once
#include "ashura/gfx/gfx.h"
#include "ashura/std/mem.h"
#include "ashura/std/option.h"
#include "ashura/std/panic.h"
#include "ashura/std/result.h"
#include "ashura/std/source_location.h"
#include "ashura/std/types.h"
#include "ashura/std/vec.h"
#include <cstddef>

#define CHECK(logger, description, ...)                                        \
  if (!(__VA_ARGS__))                                                          \
  {                                                                            \
    (logger).panic(description, " (expression: " #__VA_ARGS__,                 \
                   ") [function: ", ::ash::SourceLocation::current().function, \
                   ", file: ", ::ash::SourceLocation::current().file, ":",     \
                   ::ash::SourceLocation::current().line, ":",                 \
                   ::ash::SourceLocation::current().column, "]");              \
  }

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

#define SHADER_SAMPLER(BindingName, Count)                                       \
  _METAMember_##BindingName;                                                     \
                                                                                 \
  static constexpr void _METApush(_METAMember_##BindingName,                     \
                                  ::ash::ShaderBindingMetaData *meta,            \
                                  u16                           _METAThisOffset) \
  {                                                                              \
    *meta = ::ash::ShaderBindingMetaData{                                        \
        .name  = ::ash::to_span(#BindingName),                                   \
        .type  = ::ash::gfx::DescriptorType::Sampler,                            \
        .count = (u16) Count,                                                    \
        .member_offset =                                                         \
            (u16) (_METAThisOffset + offsetof(_METAThisType, BindingName))};     \
    _METApush(_METAMemberAfter_##BindingName{}, meta + 1, _METAThisOffset);      \
  }                                                                              \
                                                                                 \
  ::ash::gfx::SamplerBinding BindingName[Count];                                 \
                                                                                 \
  struct _METAMemberAfter_##BindingName                                          \
  {                                                                              \
    static constexpr u16 _METAMemberIndex =                                      \
        _METAMember_##BindingName::_METAMemberIndex + 1;                         \
  };                                                                             \
                                                                                 \
  typedef _METAMemberAfter_##BindingName

#define SHADER_COMBINED_IMAGE_SAMPLER(BindingName, Count)                        \
  _METAMember_##BindingName;                                                     \
                                                                                 \
  static constexpr void _METApush(_METAMember_##BindingName,                     \
                                  ::ash::ShaderBindingMetaData *meta,            \
                                  u16                           _METAThisOffset) \
  {                                                                              \
    *meta = ::ash::ShaderBindingMetaData{                                        \
        .name  = ::ash::to_span(#BindingName),                                   \
        .type  = ::ash::gfx::DescriptorType::CombinedImageSampler,               \
        .count = (u16) Count,                                                    \
        .member_offset =                                                         \
            (u16) (_METAThisOffset + offsetof(_METAThisType, BindingName))};     \
    _METApush(_METAMemberAfter_##BindingName{}, meta + 1, _METAThisOffset);      \
  }                                                                              \
                                                                                 \
  ::ash::gfx::CombinedImageSamplerBinding BindingName[Count];                    \
                                                                                 \
  struct _METAMemberAfter_##BindingName                                          \
  {                                                                              \
    static constexpr u16 _METAMemberIndex =                                      \
        _METAMember_##BindingName::_METAMemberIndex + 1;                         \
  };                                                                             \
                                                                                 \
  typedef _METAMemberAfter_##BindingName

#define SHADER_SAMPLED_IMAGE(BindingName, Count)                                 \
  _METAMember_##BindingName;                                                     \
                                                                                 \
  static constexpr void _METApush(_METAMember_##BindingName,                     \
                                  ::ash::ShaderBindingMetaData *meta,            \
                                  u16                           _METAThisOffset) \
  {                                                                              \
    *meta = ::ash::ShaderBindingMetaData{                                        \
        .name  = ::ash::to_span(#BindingName),                                   \
        .type  = ::ash::gfx::DescriptorType::SampledImage,                       \
        .count = (u16) Count,                                                    \
        .member_offset =                                                         \
            (u16) (_METAThisOffset + offsetof(_METAThisType, BindingName))};     \
    _METApush(_METAMemberAfter_##BindingName{}, meta + 1, _METAThisOffset);      \
  }                                                                              \
                                                                                 \
  ::ash::gfx::SampledImageBinding BindingName[Count];                            \
                                                                                 \
  struct _METAMemberAfter_##BindingName                                          \
  {                                                                              \
    static constexpr u16 _METAMemberIndex =                                      \
        _METAMember_##BindingName::_METAMemberIndex + 1;                         \
  };                                                                             \
                                                                                 \
  typedef _METAMemberAfter_##BindingName

#define SHADER_UNIFORM_TEXEL_BUFFER(BindingName, Count)                          \
  _METAMember_##BindingName;                                                     \
                                                                                 \
  static constexpr void _METApush(_METAMember_##BindingName,                     \
                                  ::ash::ShaderBindingMetaData *meta,            \
                                  u16                           _METAThisOffset) \
  {                                                                              \
    *meta = ::ash::ShaderBindingMetaData{                                        \
        .name  = ::ash::to_span(#BindingName),                                   \
        .type  = ::ash::gfx::DescriptorType::UniformTexelBuffer,                 \
        .count = (u16) Count,                                                    \
        .member_offset =                                                         \
            (u16) (_METAThisOffset + offsetof(_METAThisType, BindingName))};     \
    _METApush(_METAMemberAfter_##BindingName{}, meta + 1, _METAThisOffset);      \
  }                                                                              \
                                                                                 \
  ::ash::gfx::UniformTexelBufferBinding BindingName[Count];                      \
                                                                                 \
  struct _METAMemberAfter_##BindingName                                          \
  {                                                                              \
    static constexpr u16 _METAMemberIndex =                                      \
        _METAMember_##BindingName::_METAMemberIndex + 1;                         \
  };                                                                             \
                                                                                 \
  typedef _METAMemberAfter_##BindingName

#define SHADER_STORAGE_TEXEL_BUFFER(BindingName, Count)                          \
  _METAMember_##BindingName;                                                     \
                                                                                 \
  static constexpr void _METApush(_METAMember_##BindingName,                     \
                                  ::ash::ShaderBindingMetaData *meta,            \
                                  u16                           _METAThisOffset) \
  {                                                                              \
    *meta = ::ash::ShaderBindingMetaData{                                        \
        .name  = ::ash::to_span(#BindingName),                                   \
        .type  = ::ash::gfx::DescriptorType::StorageTexelBuffer,                 \
        .count = (u16) Count,                                                    \
        .member_offset =                                                         \
            (u16) (_METAThisOffset + offsetof(_METAThisType, BindingName))};     \
    _METApush(_METAMemberAfter_##BindingName{}, meta + 1, _METAThisOffset);      \
  }                                                                              \
                                                                                 \
  ::ash::gfx::StorageTexelBufferBinding BindingName[Count];                      \
                                                                                 \
  struct _METAMemberAfter_##BindingName                                          \
  {                                                                              \
    static constexpr u16 _METAMemberIndex =                                      \
        _METAMember_##BindingName::_METAMemberIndex + 1;                         \
  };                                                                             \
                                                                                 \
  typedef _METAMemberAfter_##BindingName

#define SHADER_UNIFORM_BUFFER(BindingName, Count)                                \
  _METAMember_##BindingName;                                                     \
                                                                                 \
  static constexpr void _METApush(_METAMember_##BindingName,                     \
                                  ::ash::ShaderBindingMetaData *meta,            \
                                  u16                           _METAThisOffset) \
  {                                                                              \
    *meta = ::ash::ShaderBindingMetaData{                                        \
        .name  = ::ash::to_span(#BindingName),                                   \
        .type  = ::ash::gfx::DescriptorType::UniformBuffer,                      \
        .count = (u16) Count,                                                    \
        .member_offset =                                                         \
            (u16) (_METAThisOffset + offsetof(_METAThisType, BindingName))};     \
    _METApush(_METAMemberAfter_##BindingName{}, meta + 1, _METAThisOffset);      \
  }                                                                              \
                                                                                 \
  ::ash::gfx::UniformBufferBinding BindingName[Count];                           \
                                                                                 \
  struct _METAMemberAfter_##BindingName                                          \
  {                                                                              \
    static constexpr u16 _METAMemberIndex =                                      \
        _METAMember_##BindingName::_METAMemberIndex + 1;                         \
  };                                                                             \
                                                                                 \
  typedef _METAMemberAfter_##BindingName

#define SHADER_STORAGE_BUFFER(BindingName, Count)                                \
  _METAMember_##BindingName;                                                     \
                                                                                 \
  static constexpr void _METApush(_METAMember_##BindingName,                     \
                                  ::ash::ShaderBindingMetaData *meta,            \
                                  u16                           _METAThisOffset) \
  {                                                                              \
    *meta = ::ash::ShaderBindingMetaData{                                        \
        .name  = ::ash::to_span(#BindingName),                                   \
        .type  = ::ash::gfx::DescriptorType::StorageBuffer,                      \
        .count = (u16) Count,                                                    \
        .member_offset =                                                         \
            (u16) (_METAThisOffset + offsetof(_METAThisType, BindingName))};     \
    _METApush(_METAMemberAfter_##BindingName{}, meta + 1, _METAThisOffset);      \
  }                                                                              \
                                                                                 \
  ::ash::gfx::StorageBufferBinding BindingName[Count];                           \
                                                                                 \
  struct _METAMemberAfter_##BindingName                                          \
  {                                                                              \
    static constexpr u16 _METAMemberIndex =                                      \
        _METAMember_##BindingName::_METAMemberIndex + 1;                         \
  };                                                                             \
                                                                                 \
  typedef _METAMemberAfter_##BindingName

#define SHADER_DYNAMIC_UNIFORM_BUFFER(BindingName, Count)                        \
  _METAMember_##BindingName;                                                     \
                                                                                 \
  static constexpr void _METApush(_METAMember_##BindingName,                     \
                                  ::ash::ShaderBindingMetaData *meta,            \
                                  u16                           _METAThisOffset) \
  {                                                                              \
    *meta = ::ash::ShaderBindingMetaData{                                        \
        .name  = ::ash::to_span(#BindingName),                                   \
        .type  = ::ash::gfx::DescriptorType::DynamicUniformBuffer,               \
        .count = (u16) Count,                                                    \
        .member_offset =                                                         \
            (u16) (_METAThisOffset + offsetof(_METAThisType, BindingName))};     \
    _METApush(_METAMemberAfter_##BindingName{}, meta + 1, _METAThisOffset);      \
  }                                                                              \
                                                                                 \
  ::ash::gfx::DynamicUniformBufferBinding BindingName[Count];                    \
                                                                                 \
  struct _METAMemberAfter_##BindingName                                          \
  {                                                                              \
    static constexpr u16 _METAMemberIndex =                                      \
        _METAMember_##BindingName::_METAMemberIndex + 1;                         \
  };                                                                             \
                                                                                 \
  typedef _METAMemberAfter_##BindingName

#define SHADER_DYNAMIC_STORAGE_BUFFER(BindingName, Count)                        \
  _METAMember_##BindingName;                                                     \
                                                                                 \
  static constexpr void _METApush(_METAMember_##BindingName,                     \
                                  ::ash::ShaderBindingMetaData *meta,            \
                                  u16                           _METAThisOffset) \
  {                                                                              \
    *meta = ::ash::ShaderBindingMetaData{                                        \
        .name  = ::ash::to_span(#BindingName),                                   \
        .type  = ::ash::gfx::DescriptorType::DynamicStorageBuffer,               \
        .count = (u16) Count,                                                    \
        .member_offset =                                                         \
            (u16) (_METAThisOffset + offsetof(_METAThisType, BindingName))};     \
    _METApush(_METAMemberAfter_##BindingName{}, meta + 1, _METAThisOffset);      \
  }                                                                              \
                                                                                 \
  ::ash::gfx::DynamicStorageBufferBinding BindingName[Count];                    \
                                                                                 \
  struct _METAMemberAfter_##BindingName                                          \
  {                                                                              \
    static constexpr u16 _METAMemberIndex =                                      \
        _METAMember_##BindingName::_METAMemberIndex + 1;                         \
  };                                                                             \
                                                                                 \
  typedef _METAMemberAfter_##BindingName

#define SHADER_INPUT_ATTACHMENT(BindingName, Count)                              \
  _METAMember_##BindingName;                                                     \
                                                                                 \
  static constexpr void _METApush(_METAMember_##BindingName,                     \
                                  ::ash::ShaderBindingMetaData *meta,            \
                                  u16                           _METAThisOffset) \
  {                                                                              \
    *meta = ::ash::ShaderBindingMetaData{                                        \
        .name  = ::ash::to_span(#BindingName),                                   \
        .type  = ::ash::gfx::DescriptorType::InputAttachment,                    \
        .count = (u16) Count,                                                    \
        .member_offset =                                                         \
            (u16) (_METAThisOffset + offsetof(_METAThisType, BindingName))};     \
    _METApush(_METAMemberAfter_##BindingName{}, meta + 1, _METAThisOffset);      \
  }                                                                              \
                                                                                 \
  ::ash::gfx::InputAttachmentBinding BindingName[Count];                         \
                                                                                 \
  struct _METAMemberAfter_##BindingName                                          \
  {                                                                              \
    static constexpr u16 _METAMemberIndex =                                      \
        _METAMember_##BindingName::_METAMemberIndex + 1;                         \
  };                                                                             \
                                                                                 \
  typedef _METAMemberAfter_##BindingName

#define SHADER_PARAMETER_INLINE(BindingName, ParameterType)                      \
  _METAMember_##BindingName;                                                     \
                                                                                 \
  static constexpr void _METApush(_METAMember_##BindingName,                     \
                                  ::ash::ShaderBindingMetaData *meta,            \
                                  u16                           _METAThisOffset) \
  {                                                                              \
    ParameterType::_METApush(                                                    \
        ParameterType::_METAMemberBegin{}, meta,                                 \
        (u16) (_METAThisOffset + offsetof(_METAThisType, BindingName)));         \
    _METApush(_METAMemberAfter_##BindingName{},                                  \
              meta + ParameterType::NUM_BINDINGS, _METAThisOffset);              \
  }                                                                              \
                                                                                 \
  ParameterType BindingName;                                                     \
                                                                                 \
  struct _METAMemberAfter_##BindingName                                          \
  {                                                                              \
    static constexpr u16 _METAMemberIndex =                                      \
        _METAMember_##BindingName::_METAMemberIndex +                            \
        ParameterType::NUM_BINDINGS;                                             \
  };                                                                             \
                                                                                 \
  typedef _METAMemberAfter_##BindingName

#define END_SHADER_PARAMETER(Name)                                             \
  _METAMemberEnd;                                                              \
                                                                               \
  static constexpr void _METApush(_METAMemberEnd,                              \
                                  ::ash::ShaderBindingMetaData *, u16)         \
  {                                                                            \
  }                                                                            \
                                                                               \
  static constexpr char const NAME[]       = #Name;                            \
  static constexpr u16        NUM_BINDINGS = _METAMemberEnd::_METAMemberIndex; \
  static constexpr auto       GET_BINDINGS()                                   \
  {                                                                            \
    ::ash::Array<::ash::ShaderBindingMetaData, NUM_BINDINGS> bindings;         \
    _METApush(_METAMemberBegin{}, bindings, 0);                                \
    return bindings;                                                           \
  };                                                                           \
                                                                               \
  static constexpr auto GET_BINDINGS_DESC()                                    \
  {                                                                            \
    ::ash::Array<::ash::ShaderBindingMetaData, NUM_BINDINGS> bindings =        \
        GET_BINDINGS();                                                        \
    ::ash::Array<::ash::gfx::DescriptorBindingDesc, NUM_BINDINGS> descs;       \
    for (u16 i = 0; i < NUM_BINDINGS; i++)                                     \
    {                                                                          \
      descs[i] = gfx::DescriptorBindingDesc{.type  = bindings[i].type,         \
                                            .count = bindings[i].count};       \
    }                                                                          \
    return descs;                                                              \
  }                                                                            \
  }                                                                            \
  ;

BEGIN_SHADER_PARAMETER(UniformShaderParameter)
SHADER_DYNAMIC_UNIFORM_BUFFER(Data, 1)
END_SHADER_PARAMETER(UniformShaderParameter)

template <typename Param>
struct ShaderParameterHeap
{
  static constexpr Array BINDINGS = Param::GET_BINDINGS();

  gfx::DeviceImpl          device_;
  gfx::DescriptorHeapImpl  heap_;
  gfx::DescriptorSetLayout layout_;

  void init(gfx::DeviceImpl device, u32 batch_size)
  {
    auto descs = Param::GET_BINDINGS_DESC();
    layout_    = device
                  ->create_descriptor_set_layout(
                      device_.self,
                      gfx::DescriptorSetLayoutDesc{.label    = Param::NAME,
                                                   .bindings = to_span(descs)})
                  .unwrap();
    heap_ = device
                ->create_descriptor_heap(device_.self, {&layout_, 1},
                                         batch_size, heap_allocator)
                .unwrap();
    device_ = device;
  }

  void deinit()
  {
    device_->unref_descriptor_heap(device_.self, heap_);
  }

  Option<gfx::DescriptorSet> create(Param const &param)
  {
    u32 group = heap_->add_group(heap_.self).unwrap();
    return Some{
        gfx::DescriptorSet{.heap = heap_.self, .group = group, .set = 0}};
  }

  void update(gfx::DescriptorSet set, Param const &param)
  {
    for (ShaderBindingMetaData const &member : BINDINGS)
    {
      switch (member.type)
      {
        case gfx::DescriptorType::CombinedImageSampler:
        {
          heap_->combined_image_sampler(
              heap_.self, set.group, set.set, 0,
              Span{(gfx::CombinedImageSamplerBinding const
                        *) (((u8 const *) &param) + member.member_offset),
                   member.count});
        }
        break;
        case gfx::DescriptorType::DynamicStorageBuffer:
        {
          heap_->dynamic_storage_buffer(
              heap_.self, set.group, set.set, 0,
              Span{(gfx::DynamicStorageBufferBinding const
                        *) (((u8 const *) &param) + member.member_offset),
                   member.count});
        }
        break;
        case gfx::DescriptorType::DynamicUniformBuffer:
        {
          heap_->dynamic_uniform_buffer(
              heap_.self, set.group, set.set, 0,
              Span{(gfx::DynamicUniformBufferBinding const
                        *) (((u8 const *) &param) + member.member_offset),
                   member.count});
        }
        break;
        case gfx::DescriptorType::InputAttachment:
        {
          heap_->input_attachment(
              heap_.self, set.group, set.set, 0,
              Span{
                  (gfx::InputAttachmentBinding const *) (((u8 const *) &param) +
                                                         member.member_offset),
                  member.count});
        }
        break;
        case gfx::DescriptorType::SampledImage:
        {
          heap_->sampled_image(
              heap_.self, set.group, set.set, 0,
              Span{(gfx::SampledImageBinding const *) (((u8 const *) &param) +
                                                       member.member_offset),
                   member.count});
        }
        break;
        case gfx::DescriptorType::Sampler:
        {
          heap_->sampler(
              heap_.self, set.group, set.set, 0,
              Span{(gfx::SamplerBinding const *) (((u8 const *) &param) +
                                                  member.member_offset),
                   member.count});
        }
        break;
        case gfx::DescriptorType::StorageBuffer:
        {
          heap_->storage_buffer(
              heap_.self, set.group, set.set, 0,
              Span{(gfx::StorageBufferBinding const *) (((u8 const *) &param) +
                                                        member.member_offset),
                   member.count});
        }
        break;
        case gfx::DescriptorType::StorageImage:
        {
          heap_->storage_image(
              heap_.self, set.group, set.set, 0,
              Span{(gfx::StorageImageBinding const *) (((u8 const *) &param) +
                                                       member.member_offset),
                   member.count});
        }
        break;
        case gfx::DescriptorType::StorageTexelBuffer:
        {
          heap_->storage_texel_buffer(
              heap_.self, set.group, set.set, 0,
              Span{(gfx::StorageTexelBufferBinding const
                        *) (((u8 const *) &param) + member.member_offset),
                   member.count});
        }
        break;
        case gfx::DescriptorType::UniformBuffer:
        {
          heap_->uniform_buffer(
              heap_.self, set.group, set.set, 0,
              Span{(gfx::UniformBufferBinding const *) (((u8 const *) &param) +
                                                        member.member_offset),
                   member.count});
        }
        break;
        case gfx::DescriptorType::UniformTexelBuffer:
        {
          heap_->uniform_texel_buffer(
              heap_.self, set.group, set.set, 0,
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

  void release(gfx::DescriptorSet set)
  {
    heap_->release(heap_.self, set.group);
  }
};

constexpr u64 DEFAULT_UNIFORM_HEAP_BATCH_SIZE = 16384;

struct UniformHeapBatch
{
  gfx::DescriptorSet set    = {};
  gfx::Buffer        buffer = nullptr;
};

struct Uniform
{
  UniformHeapBatch batch;
  u64              buffer_offset = 0;
};

/// per-frame uniform buffer heap.
/// allocate multiple large uniform buffers along with descriptor sets
/// since we are buffering (using a single uniform heap per-frame in flight),
/// once we get to this frame's next cycle, we would be able to write directly
/// to the memory-mapped gpu memory then at bind-time, use dynamic offsets to
/// point to the intended region of the batched uniform. alignment is taken care
/// of.
///
struct UniformHeap
{
  u64                      batch_buffer_size_                   = 0;
  u64                      min_uniform_buffer_offset_alignment_ = 0;
  u32                      batch_                               = 0;
  u64                      batch_buffer_offset_                 = 0;
  Vec<UniformHeapBatch>    batches_                             = {};
  gfx::DescriptorSetLayout descriptor_set_layout_               = {};
  gfx::DescriptorHeapImpl  descriptor_heap_                     = {};
  gfx::DeviceImpl          device_;

  void init(gfx::DeviceImpl device,
            u64             batch_buffer_size = DEFAULT_UNIFORM_HEAP_BATCH_SIZE)
  {
    CHECK(panic_logger, "", batch_buffer_size > 32);
    gfx::DeviceProperties properties =
        device->get_device_properties(device.self);
    min_uniform_buffer_offset_alignment_ =
        properties.limits.min_uniform_buffer_offset_alignment;
    batch_buffer_size_           = batch_buffer_size;
    device_                      = device;
    batch_                       = 0;
    batch_buffer_offset_         = 0;
    constexpr auto bindings_desc = UniformShaderParameter::GET_BINDINGS_DESC();
    descriptor_set_layout_       = device
                                 ->create_descriptor_set_layout(
                                     device.self,
                                     gfx::DescriptorSetLayoutDesc{
                                         .label    = "Uniform Buffer",
                                         .bindings = to_span(bindings_desc)})
                                 .unwrap();
    descriptor_heap_ =
        device
            ->create_descriptor_heap(device.self, {&descriptor_set_layout_, 1},
                                     16, heap_allocator)
            .unwrap();
  }

  void deinit();

  template <typename UniformType>
  Uniform push(UniformType const &uniform)
  {
    return push_range({&uniform, 1});
  }

  template <typename UniformType>
  Uniform push_range(Span<UniformType const> uniform)
  {
    static_assert(alignof(UniformType) <= 16);
    CHECK(panic_logger, "", uniform.size_bytes() <= batch_buffer_size_);
    u64 alignment =
        max(alignof(UniformType), min_uniform_buffer_offset_alignment_);
    u64 buffer_offset = mem::align_offset(alignment, batch_buffer_offset_);
    u64 batch_index   = batch_;
    if ((buffer_offset + uniform.size_bytes()) > batch_buffer_size_)
    {
      batch_index++;
      buffer_offset = 0;
    }

    if (batch_index > batches_.size())
    {
      gfx::Buffer buffer =
          device_
              ->create_buffer(
                  device_.self,
                  gfx::BufferDesc{.label       = "UniformHeap batch buffer",
                                  .size        = batch_buffer_size_,
                                  .host_mapped = true,
                                  .usage = gfx::BufferUsage::UniformBuffer})
              .unwrap();
      u32 group = descriptor_heap_->add_group(descriptor_heap_.self).unwrap();

      descriptor_heap_->dynamic_uniform_buffer(
          descriptor_heap_.self, group, 0, 0,
          to_span<gfx::DynamicUniformBufferBinding>(
              {{.buffer = buffer, .offset = 0, .size = gfx::WHOLE_SIZE}}));

      CHECK(panic_logger, "",
            batches_.push(
                UniformHeapBatch{.set = {.group = group}, .buffer = buffer}));
    }

    UniformHeapBatch const &batch = batches_[batch_index];
    void                   *map =
        device_->get_buffer_memory_map(device_.self, batch.buffer).unwrap();
    mem::copy(uniform, (UniformType *) (((u8 *) map) + buffer_offset));
    device_
        ->flush_buffer_memory_map(device_.self, batch.buffer,
                                  gfx::MemoryRange{0, gfx::WHOLE_SIZE})
        .unwrap();

    batch_               = batch_index;
    batch_buffer_offset_ = buffer_offset + uniform.size_bytes();

    return Some{Uniform{.batch = batch, .buffer_offset = buffer_offset}};
  }

  void clear()
  {
    batch_               = 0;
    batch_buffer_offset_ = 0;
  }
};

}        // namespace ash