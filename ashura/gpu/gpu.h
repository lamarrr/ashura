/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/allocator.h"
#include "ashura/std/dyn.h"
#include "ashura/std/enum.h"
#include "ashura/std/math.h"
#include "ashura/std/option.h"
#include "ashura/std/result.h"
#include "ashura/std/types.h"
#include "ashura/std/vec.h"

namespace ash
{
namespace gpu
{

inline constexpr u32 REMAINING_MIP_LEVELS   = U32_MAX;
inline constexpr u32 REMAINING_ARRAY_LAYERS = U32_MAX;
inline constexpr u64 WHOLE_SIZE             = U64_MAX;

typedef struct Buffer_T *              Buffer;
typedef struct BufferView_T *          BufferView;
typedef struct Image_T *               Image;
typedef struct ImageView_T *           ImageView;
typedef struct MemoryGroup_T *         MemoryGroup;
typedef struct Sampler_T *             Sampler;
typedef struct Shader_T *              Shader;
typedef struct DescriptorSetLayout_T * DescriptorSetLayout;
typedef struct DescriptorSet_T *       DescriptorSet;
typedef struct PipelineCache_T *       PipelineCache;
typedef struct ComputePipeline_T *     ComputePipeline;
typedef struct GraphicsPipeline_T *    GraphicsPipeline;
typedef struct TimestampQuery_T *      TimestampQuery;
typedef struct StatisticsQuery_T *     StatisticsQuery;
typedef struct Surface_T *             Surface;
typedef struct Swapchain_T *           Swapchain;
typedef struct QueueScope_T *          QueueScope;
typedef struct CommandEncoder *        CommandEncoderPtr;
typedef struct CommandBuffer *         CommandBufferPtr;
typedef struct Device *                DevicePtr;
typedef struct Instance *              InstancePtr;

enum class ObjectType : u32
{
  None                = 0,
  Instance            = 1,
  Device              = 2,
  CommandEncoder      = 3,
  Buffer              = 4,
  BufferView          = 5,
  Image               = 6,
  ImageView           = 7,
  Sampler             = 8,
  Shader              = 9,
  DescriptorSetLayout = 10,
  DescriptorSet       = 11,
  PipelineCache       = 12,
  ComputePipeline     = 13,
  GraphicsPipeline    = 14,
  TimestampQuery      = 15,
  StatisticsQuery     = 16,
  Surface             = 17,
  Swapchain           = 18
};

enum class Backend : u8
{
  Stub    = 0,
  Vulkan  = 1,
  OpenGL  = 2,
  DirectX = 3,
  Metal   = 4
};

enum class DeviceType : u8
{
  Other         = 0,
  IntegratedGpu = 1,
  DiscreteGpu   = 2,
  VirtualGpu    = 3,
  Cpu           = 4
};

enum class MemoryProperties : u8
{
  None            = 0x00,
  DeviceLocal     = 0x01,
  HostVisible     = 0x02,
  HostCoherent    = 0x04,
  HostCached      = 0x08,
  LazilyAllocated = 0x10
};

ASH_BIT_ENUM_OPS(MemoryProperties)

enum class PresentMode : u8
{
  Immediate   = 0,
  Mailbox     = 1,
  Fifo        = 2,
  FifoRelaxed = 3
};

enum class [[nodiscard]] Status : i32
{
  Success              = 0,
  NotReady             = 1,
  TimeOut              = 2,
  Incomplete           = 5,
  OutOfHostMemory      = -1,
  OutOfDeviceMemory    = -2,
  InitializationFailed = -3,
  DeviceLost           = -4,
  MemoryMapFailed      = -5,
  LayerNotPresent      = -6,
  ExtensionNotPresent  = -7,
  FeatureNotPresent    = -8,
  TooManyObjects       = -10,
  FormatNotSupported   = -11,
  Unknown              = -13,
  SurfaceLost          = -1'000'000'000
};

constexpr Str to_str(Status status)
{
  switch (status)
  {
    case Status::Success:
      return "Success"_str;
    case Status::NotReady:
      return "NotReady"_str;
    case Status::TimeOut:
      return "TimeOut"_str;
    case Status::Incomplete:
      return "Incomplete"_str;
    case Status::OutOfHostMemory:
      return "OutOfHostMemory"_str;
    case Status::OutOfDeviceMemory:
      return "OutOfDeviceMemory"_str;
    case Status::InitializationFailed:
      return "InitializationFailed"_str;
    case Status::DeviceLost:
      return "DeviceLost"_str;
    case Status::MemoryMapFailed:
      return "MemoryMapFailed"_str;
    case Status::LayerNotPresent:
      return "LayerNotPresent"_str;
    case Status::ExtensionNotPresent:
      return "ExtensionNotPresent"_str;
    case Status::FeatureNotPresent:
      return "FeatureNotPresent"_str;
    case Status::TooManyObjects:
      return "TooManyObjects"_str;
    case Status::FormatNotSupported:
      return "FormatNotSupported"_str;
    case Status::SurfaceLost:
      return "SurfaceLost"_str;
    default:
      return "<Unrecognized Status>"_str;
  }
}

enum class Format : i32
{
  Undefined                  = 0,
  R4G4_UNORM_PACK8           = 1,
  R4G4B4A4_UNORM_PACK16      = 2,
  B4G4R4A4_UNORM_PACK16      = 3,
  R5G6B5_UNORM_PACK16        = 4,
  B5G6R5_UNORM_PACK16        = 5,
  R5G5B5A1_UNORM_PACK16      = 6,
  B5G5R5A1_UNORM_PACK16      = 7,
  A1R5G5B5_UNORM_PACK16      = 8,
  R8_UNORM                   = 9,
  R8_SNORM                   = 10,
  R8_USCALED                 = 11,
  R8_SSCALED                 = 12,
  R8_UINT                    = 13,
  R8_SINT                    = 14,
  R8_SRGB                    = 15,
  R8G8_UNORM                 = 16,
  R8G8_SNORM                 = 17,
  R8G8_USCALED               = 18,
  R8G8_SSCALED               = 19,
  R8G8_UINT                  = 20,
  R8G8_SINT                  = 21,
  R8G8_SRGB                  = 22,
  R8G8B8_UNORM               = 23,
  R8G8B8_SNORM               = 24,
  R8G8B8_USCALED             = 25,
  R8G8B8_SSCALED             = 26,
  R8G8B8_UINT                = 27,
  R8G8B8_SINT                = 28,
  R8G8B8_SRGB                = 29,
  B8G8R8_UNORM               = 30,
  B8G8R8_SNORM               = 31,
  B8G8R8_USCALED             = 32,
  B8G8R8_SSCALED             = 33,
  B8G8R8_UINT                = 34,
  B8G8R8_SINT                = 35,
  B8G8R8_SRGB                = 36,
  R8G8B8A8_UNORM             = 37,
  R8G8B8A8_SNORM             = 38,
  R8G8B8A8_USCALED           = 39,
  R8G8B8A8_SSCALED           = 40,
  R8G8B8A8_UINT              = 41,
  R8G8B8A8_SINT              = 42,
  R8G8B8A8_SRGB              = 43,
  B8G8R8A8_UNORM             = 44,
  B8G8R8A8_SNORM             = 45,
  B8G8R8A8_USCALED           = 46,
  B8G8R8A8_SSCALED           = 47,
  B8G8R8A8_UINT              = 48,
  B8G8R8A8_SINT              = 49,
  B8G8R8A8_SRGB              = 50,
  A8B8G8R8_UNORM_PACK32      = 51,
  A8B8G8R8_SNORM_PACK32      = 52,
  A8B8G8R8_USCALED_PACK32    = 53,
  A8B8G8R8_SSCALED_PACK32    = 54,
  A8B8G8R8_UINT_PACK32       = 55,
  A8B8G8R8_SINT_PACK32       = 56,
  A8B8G8R8_SRGB_PACK32       = 57,
  A2R10G10B10_UNORM_PACK32   = 58,
  A2R10G10B10_SNORM_PACK32   = 59,
  A2R10G10B10_USCALED_PACK32 = 60,
  A2R10G10B10_SSCALED_PACK32 = 61,
  A2R10G10B10_UINT_PACK32    = 62,
  A2R10G10B10_SINT_PACK32    = 63,
  A2B10G10R10_UNORM_PACK32   = 64,
  A2B10G10R10_SNORM_PACK32   = 65,
  A2B10G10R10_USCALED_PACK32 = 66,
  A2B10G10R10_SSCALED_PACK32 = 67,
  A2B10G10R10_UINT_PACK32    = 68,
  A2B10G10R10_SINT_PACK32    = 69,
  R16_UNORM                  = 70,
  R16_SNORM                  = 71,
  R16_USCALED                = 72,
  R16_SSCALED                = 73,
  R16_UINT                   = 74,
  R16_SINT                   = 75,
  R16_SFLOAT                 = 76,
  R16G16_UNORM               = 77,
  R16G16_SNORM               = 78,
  R16G16_USCALED             = 79,
  R16G16_SSCALED             = 80,
  R16G16_UINT                = 81,
  R16G16_SINT                = 82,
  R16G16_SFLOAT              = 83,
  R16G16B16_UNORM            = 84,
  R16G16B16_SNORM            = 85,
  R16G16B16_USCALED          = 86,
  R16G16B16_SSCALED          = 87,
  R16G16B16_UINT             = 88,
  R16G16B16_SINT             = 89,
  R16G16B16_SFLOAT           = 90,
  R16G16B16A16_UNORM         = 91,
  R16G16B16A16_SNORM         = 92,
  R16G16B16A16_USCALED       = 93,
  R16G16B16A16_SSCALED       = 94,
  R16G16B16A16_UINT          = 95,
  R16G16B16A16_SINT          = 96,
  R16G16B16A16_SFLOAT        = 97,
  R32_UINT                   = 98,
  R32_SINT                   = 99,
  R32_SFLOAT                 = 100,
  R32G32_UINT                = 101,
  R32G32_SINT                = 102,
  R32G32_SFLOAT              = 103,
  R32G32B32_UINT             = 104,
  R32G32B32_SINT             = 105,
  R32G32B32_SFLOAT           = 106,
  R32G32B32A32_UINT          = 107,
  R32G32B32A32_SINT          = 108,
  R32G32B32A32_SFLOAT        = 109,
  R64_UINT                   = 110,
  R64_SINT                   = 111,
  R64_SFLOAT                 = 112,
  R64G64_UINT                = 113,
  R64G64_SINT                = 114,
  R64G64_SFLOAT              = 115,
  R64G64B64_UINT             = 116,
  R64G64B64_SINT             = 117,
  R64G64B64_SFLOAT           = 118,
  R64G64B64A64_UINT          = 119,
  R64G64B64A64_SINT          = 120,
  R64G64B64A64_SFLOAT        = 121,
  B10G11R11_UFLOAT_PACK32    = 122,
  E5B9G9R9_UFLOAT_PACK32     = 123,
  D16_UNORM                  = 124,
  X8_D24_UNORM_PACK32        = 125,
  D32_SFLOAT                 = 126,
  S8_UINT                    = 127,
  D16_UNORM_S8_UINT          = 128,
  D24_UNORM_S8_UINT          = 129,
  D32_SFLOAT_S8_UINT         = 130,
  BC1_RGB_UNORM_BLOCK        = 131,
  BC1_RGB_SRGB_BLOCK         = 132,
  BC1_RGBA_UNORM_BLOCK       = 133,
  BC1_RGBA_SRGB_BLOCK        = 134,
  BC2_UNORM_BLOCK            = 135,
  BC2_SRGB_BLOCK             = 136,
  BC3_UNORM_BLOCK            = 137,
  BC3_SRGB_BLOCK             = 138,
  BC4_UNORM_BLOCK            = 139,
  BC4_SNORM_BLOCK            = 140,
  BC5_UNORM_BLOCK            = 141,
  BC5_SNORM_BLOCK            = 142,
  BC6H_UFLOAT_BLOCK          = 143,
  BC6H_SFLOAT_BLOCK          = 144,
  BC7_UNORM_BLOCK            = 145,
  BC7_SRGB_BLOCK             = 146,
  ETC2_R8G8B8_UNORM_BLOCK    = 147,
  ETC2_R8G8B8_SRGB_BLOCK     = 148,
  ETC2_R8G8B8A1_UNORM_BLOCK  = 149,
  ETC2_R8G8B8A1_SRGB_BLOCK   = 150,
  ETC2_R8G8B8A8_UNORM_BLOCK  = 151,
  ETC2_R8G8B8A8_SRGB_BLOCK   = 152,
  EAC_R11_UNORM_BLOCK        = 153,
  EAC_R11_SNORM_BLOCK        = 154,
  EAC_R11G11_UNORM_BLOCK     = 155,
  EAC_R11G11_SNORM_BLOCK     = 156,
  ASTC_4x4_UNORM_BLOCK       = 157,
  ASTC_4x4_SRGB_BLOCK        = 158,
  ASTC_5x4_UNORM_BLOCK       = 159,
  ASTC_5x4_SRGB_BLOCK        = 160,
  ASTC_5x5_UNORM_BLOCK       = 161,
  ASTC_5x5_SRGB_BLOCK        = 162,
  ASTC_6x5_UNORM_BLOCK       = 163,
  ASTC_6x5_SRGB_BLOCK        = 164,
  ASTC_6x6_UNORM_BLOCK       = 165,
  ASTC_6x6_SRGB_BLOCK        = 166,
  ASTC_8x5_UNORM_BLOCK       = 167,
  ASTC_8x5_SRGB_BLOCK        = 168,
  ASTC_8x6_UNORM_BLOCK       = 169,
  ASTC_8x6_SRGB_BLOCK        = 170,
  ASTC_8x8_UNORM_BLOCK       = 171,
  ASTC_8x8_SRGB_BLOCK        = 172,
  ASTC_10x5_UNORM_BLOCK      = 173,
  ASTC_10x5_SRGB_BLOCK       = 174,
  ASTC_10x6_UNORM_BLOCK      = 175,
  ASTC_10x6_SRGB_BLOCK       = 176,
  ASTC_10x8_UNORM_BLOCK      = 177,
  ASTC_10x8_SRGB_BLOCK       = 178,
  ASTC_10x10_UNORM_BLOCK     = 179,
  ASTC_10x10_SRGB_BLOCK      = 180,
  ASTC_12x10_UNORM_BLOCK     = 181,
  ASTC_12x10_SRGB_BLOCK      = 182,
  ASTC_12x12_UNORM_BLOCK     = 183,
  ASTC_12x12_SRGB_BLOCK      = 184
};

enum class ColorSpace : i32
{
  SRGB_NONLINEAR          = 0,
  DISPLAY_P3_NONLINEAR    = 1'000'104'001,
  EXTENDED_SRGB_LINEAR    = 1'000'104'002,
  DISPLAY_P3_LINEAR       = 1'000'104'003,
  DCI_P3_NONLINEAR        = 1'000'104'004,
  BT709_LINEAR            = 1'000'104'005,
  BT709_NONLINEAR         = 1'000'104'006,
  BT2020_LINEAR           = 1'000'104'007,
  HDR10_ST2084            = 1'000'104'008,
  DOLBYVISION             = 1'000'104'009,
  HDR10_HLG               = 1'000'104'010,
  ADOBERGB_LINEAR         = 1'000'104'011,
  ADOBERGB_NONLINEAR      = 1'000'104'012,
  PASS_THROUGH            = 1'000'104'013,
  EXTENDED_SRGB_NONLINEAR = 1'000'104'014
};

enum class FormatFeatures : u32
{
  None                     = 0x0000U,
  SampledImage             = 0x0001U,
  StorageImage             = 0x0002U,
  StorageImageAtomic       = 0x0004U,
  UniformTexelBuffer       = 0x0008U,
  StorageTexelBuffer       = 0x0010U,
  StorageTexelBufferAtomic = 0x0020U,
  VertexBuffer             = 0x0040U,
  ColorAttachment          = 0x0080U,
  ColorAttachmentBlend     = 0x0100U,
  DepthStencilAttachment   = 0x0200U,
  BlitSrc                  = 0x0400U,
  BlitDst                  = 0x0800U,
  SampledImageFilterLinear = 0x1000U
};

ASH_BIT_ENUM_OPS(FormatFeatures)

enum class ImageAspects : u8
{
  None    = 0x00U,
  Color   = 0x01U,
  Depth   = 0x02U,
  Stencil = 0x04U
};

ASH_BIT_ENUM_OPS(ImageAspects)

enum class SampleCount : u8
{
  None = 0x00U,
  C1   = 0x01U,
  C2   = 0x02U,
  C4   = 0x04U,
  C8   = 0x08U,
  C16  = 0x10U,
  C32  = 0x20U,
  C64  = 0x40U
};

ASH_BIT_ENUM_OPS(SampleCount)

enum class LoadOp : u8
{
  Load     = 0,
  Clear    = 1,
  DontCare = 2
};

enum class StoreOp : u32
{
  Store    = 0,
  DontCare = 1,
  None     = 1'000'301'000
};

enum class BlendFactor : u8
{
  Zero                  = 0,
  One                   = 1,
  SrcColor              = 2,
  OneMinusSrcColor      = 3,
  DstColor              = 4,
  OneMinusDstColor      = 5,
  SrcAlpha              = 6,
  OneMinusSrcAlpha      = 7,
  DstAlpha              = 8,
  OneMinusDstAlpha      = 9,
  ConstantColor         = 10,
  OneMinusConstantColor = 11,
  ConstantAlpha         = 12,
  OneMinusConstantAlpha = 13,
  SrcAlphaSaturate      = 14,
  Src1Color             = 15,
  OneMinusSrc1Color     = 16,
  Src1Alpha             = 17,
  OneMinusSrc1Alpha     = 18
};

enum class BlendOp : u8
{
  Add             = 0,
  Subtract        = 1,
  ReverseSubtract = 2,
  Min             = 3,
  Max             = 4
};

enum class CompareOp : u8
{
  Never          = 0,
  Less           = 1,
  Equal          = 2,
  LessOrEqual    = 3,
  Greater        = 4,
  NotEqual       = 5,
  GreaterOrEqual = 6,
  Always         = 7
};

enum class StencilOp : u8
{
  Keep              = 0,
  Zero              = 1,
  Replace           = 2,
  IncrementAndClamp = 3,
  DecrementAndClamp = 4,
  Invert            = 5,
  IncrementAndWrap  = 6,
  DecrementAndWrap  = 7
};

enum class LogicOp : u8
{
  Clear        = 0,
  And          = 1,
  AndReverse   = 2,
  Copy         = 3,
  AndInverted  = 4,
  NoOp         = 5,
  Xor          = 6,
  Or           = 7,
  Nor          = 8,
  Equivalent   = 9,
  Invert       = 10,
  OrReverse    = 11,
  CopyInverted = 12,
  OrInverted   = 13,
  Nand         = 14,
  Set          = 15
};

enum class SamplerAddressMode : u8
{
  Repeat            = 0,
  MirroredRepeat    = 1,
  ClampToEdge       = 2,
  ClampToBorder     = 3,
  MirrorClampToEdge = 4
};

enum class SamplerMipMapMode : u8
{
  Nearest = 0,
  Linear  = 1
};

enum class Filter : u8
{
  Nearest = 0,
  Linear  = 1
};

enum class CullMode : u8
{
  None         = 0,
  Front        = 1,
  Back         = 2,
  FrontAndBack = Front | Back
};

enum class FrontFace : u8
{
  CounterClockWise = 0,
  ClockWise        = 1
};

enum class StencilFaces : u8
{
  None         = 0,
  Front        = 1,
  Back         = 2,
  FrontAndBack = 3
};

enum class ComponentSwizzle : u8
{
  Identity   = 0,
  Zero       = 1,
  One        = 2,
  ComponentR = 3,
  ComponentG = 4,
  ComponentB = 5,
  ComponentA = 6
};

enum class ColorComponents : u8
{
  None = 0x00,
  R    = 0x01,
  G    = 0x02,
  B    = 0x04,
  A    = 0x08,
  All  = R | G | B | A
};

ASH_BIT_ENUM_OPS(ColorComponents)

enum class BufferUsage : u32
{
  None               = 0x0000U,
  TransferSrc        = 0x0001U,
  TransferDst        = 0x0002U,
  UniformTexelBuffer = 0x0004U,
  StorageTexelBuffer = 0x0008U,
  UniformBuffer      = 0x0010U,
  StorageBuffer      = 0x0020U,
  IndexBuffer        = 0x0040U,
  VertexBuffer       = 0x0080U,
  IndirectBuffer     = 0x0100U
};

ASH_BIT_ENUM_OPS(BufferUsage)

enum class ImageUsage : u32
{
  None                   = 0x00U,
  TransferSrc            = 0x01U,
  TransferDst            = 0x02U,
  Sampled                = 0x04U,
  Storage                = 0x08U,
  ColorAttachment        = 0x10U,
  DepthStencilAttachment = 0x20U,
  InputAttachment        = 0x80U
};

ASH_BIT_ENUM_OPS(ImageUsage)

enum class InputRate : u8
{
  Vertex   = 0,
  Instance = 1
};

enum class ShaderStages : u8
{
  None        = 0x00U,
  Vertex      = 0x01U,
  Fragment    = 0x10U,
  Compute     = 0x20U,
  AllGraphics = 0x1FU,
  All         = Vertex | Fragment | Compute | AllGraphics
};

ASH_BIT_ENUM_OPS(ShaderStages)

enum class PipelineStages : u64
{
  None                  = 0x0000'0000,
  TopOfPipe             = 0x0000'0001,
  DrawIndirect          = 0x0000'0002,
  VertexInput           = 0x0000'0004,
  VertexShader          = 0x0000'0008,
  GeometryShader        = 0x0000'0040,
  FragmentShader        = 0x0000'0080,
  EarlyFragmentTests    = 0x0000'0100,
  LateFragmentTests     = 0x0000'0200,
  ColorAttachmentOutput = 0x0000'0400,
  ComputeShader         = 0x0000'0800,
  Transfer              = 0x0000'1000,
  BottomOfPipe          = 0x0000'2000,
  Host                  = 0x0000'4000,
  AllGraphics           = 0x0000'8000,
  AllCommands           = 0x0001'0000
};

ASH_BIT_ENUM_OPS(PipelineStages)

enum class BorderColor : u8
{
  FloatTransparentBlack = 0,
  IntTransparentBlack   = 1,
  FloatOpaqueBlack      = 2,
  IntOpaqueBlack        = 3,
  FloatOpaqueueWhite    = 4,
  IntOpaqueueWhite      = 5,
};

enum class PolygonMode : u8
{
  Fill  = 0,
  Line  = 1,
  Point = 2
};

enum class PrimitiveTopology : u8
{
  PointList     = 0,
  LineList      = 1,
  LineStrip     = 2,
  TriangleList  = 3,
  TriangleStrip = 4,
  TriangleFan   = 5
};

enum class ImageType : u8
{
  Type1D = 0,
  Type2D = 1,
  Type3D = 2
};

enum class ImageViewType : u8
{
  Type1D        = 0,
  Type2D        = 1,
  Type3D        = 2,
  TypeCube      = 3,
  Type1DArray   = 4,
  Type2DArray   = 5,
  TypeCubeArray = 6
};

enum class DescriptorType : u8
{
  Sampler              = 0,
  CombinedImageSampler = 1,
  SampledImage         = 2,
  StorageImage         = 3,
  UniformTexelBuffer   = 4,
  StorageTexelBuffer   = 5,
  UniformBuffer        = 6,
  ReadStorageBuffer    = 7,
  RWStorageBuffer      = 8,
  DynUniformBuffer     = 9,
  DynReadStorageBuffer = 10,
  DynRWStorageBuffer   = 11,
  InputAttachment      = 12
};

constexpr u8 NUM_DESCRIPTOR_TYPES = 13;

enum class IndexType : u8
{
  U16 = 0,
  U32 = 1
};

enum class CompositeAlpha : u32
{
  None           = 0x00U,
  Opaque         = 0x01U,
  PreMultiplied  = 0x02U,
  PostMultiplied = 0x04U,
  Inherit        = 0x08U
};

ASH_BIT_ENUM_OPS(CompositeAlpha)

enum class ResolveModes : u32
{
  None       = 0x00,
  SampleZero = 0x01,
  Average    = 0x02,
  Min        = 0x04,
  Max        = 0x08
};

ASH_BIT_ENUM_OPS(ResolveModes)

enum class MemoryType : u8
{
  /// the resource is the sole owner
  Unique = 0,
  /// the resource's memory is grouped with other resources
  Group  = 1
};

using Object = Enum<InstancePtr, DevicePtr, CommandEncoderPtr, CommandBufferPtr,
                    Buffer, BufferView, Image, ImageView, MemoryGroup, Sampler,
                    Shader, DescriptorSetLayout, DescriptorSet, PipelineCache,
                    ComputePipeline, GraphicsPipeline, TimestampQuery,
                    StatisticsQuery, Surface, Swapchain, QueueScope>;

struct SurfaceFormat
{
  Format     format      = Format::Undefined;
  ColorSpace color_space = ColorSpace::SRGB_NONLINEAR;
};

/// @brief Describes the region of the framebuffer the coordinates gotten from the shaders
/// will be translated to. The shader coordinates are in range [0, 1].
/// The [0, 1] shader coordinates will be transformed to where this viewport points to.
/// If either extent.x or extent.y are negative the axis is inverted.
struct Viewport
{
  f32x2 offset    = {};
  f32x2 extent    = {};
  f32   min_depth = 0;
  f32   max_depth = 0;
};

struct StencilState
{
  StencilOp fail_op       = StencilOp::Keep;
  StencilOp pass_op       = StencilOp::Keep;
  StencilOp depth_fail_op = StencilOp::Keep;
  CompareOp compare_op    = CompareOp::Never;
  u32       compare_mask  = 0;
  u32       write_mask    = 0;
  u32       reference     = 0;
};

struct ComponentMapping
{
  ComponentSwizzle r = ComponentSwizzle::Identity;
  ComponentSwizzle g = ComponentSwizzle::Identity;
  ComponentSwizzle b = ComponentSwizzle::Identity;
  ComponentSwizzle a = ComponentSwizzle::Identity;
};

struct FormatProperties
{
  FormatFeatures linear_tiling_features  = FormatFeatures::None;
  FormatFeatures optimal_tiling_features = FormatFeatures::None;
  FormatFeatures buffer_features         = FormatFeatures::None;
};

struct ImageSubresourceRange
{
  ImageAspects aspects      = ImageAspects::None;
  Slice32      mip_levels   = {};
  Slice32      array_layers = {};
};

struct ImageSubresourceLayers
{
  ImageAspects aspects      = ImageAspects::None;
  u32          mip_level    = 0;
  Slice32      array_layers = {};
};

struct MemoryGroupInfo
{
  Span<Enum<Buffer, Image> const> resources = {};
  Span<u32 const>                 aliases   = {};
};

struct BufferInfo
{
  Str         label       = {};
  u64         size        = 0;
  BufferUsage usage       = BufferUsage::None;
  MemoryType  memory_type = MemoryType::Unique;
  bool        host_mapped = false;
};

/// format interpretation of a buffer's contents
struct BufferViewInfo
{
  Str     label  = {};
  Buffer  buffer = nullptr;
  Format  format = Format::Undefined;
  Slice64 slice  = {};
};

struct ImageInfo
{
  Str          label        = {};
  ImageType    type         = ImageType::Type1D;
  Format       format       = Format::Undefined;
  ImageUsage   usage        = ImageUsage::None;
  ImageAspects aspects      = ImageAspects::None;
  u32x3        extent       = {};
  u32          mip_levels   = 0;
  u32          array_layers = 0;
  SampleCount  sample_count = SampleCount::None;
  MemoryType   memory_type  = MemoryType::Unique;
};

/// a sub-resource that specifies mips, aspects, layer, and component mapping of
/// images. typically for reference in shaders.
///
/// @param mapping mapping of the components in the shader. i.e. for
/// R8G8B8_UNORM the non-existent Alpha component is always 0. To set it to 1 we
/// set its component mapping (mapping.a) to ComponentSwizzle::One.
///
struct ImageViewInfo
{
  Str              label        = {};
  Image            image        = nullptr;
  ImageViewType    view_type    = ImageViewType::Type1D;
  Format           view_format  = Format::Undefined;
  ComponentMapping mapping      = {};
  ImageAspects     aspects      = ImageAspects::None;
  Slice32          mip_levels   = {};
  Slice32          array_layers = {};
};

struct SamplerInfo
{
  Str                label             = {};
  Filter             mag_filter        = Filter::Nearest;
  Filter             min_filter        = Filter::Nearest;
  SamplerMipMapMode  mip_map_mode      = SamplerMipMapMode::Nearest;
  SamplerAddressMode address_mode_u    = SamplerAddressMode::Repeat;
  SamplerAddressMode address_mode_v    = SamplerAddressMode::Repeat;
  SamplerAddressMode address_mode_w    = SamplerAddressMode::Repeat;
  f32                mip_lod_bias      = 0;
  bool               anisotropy_enable = false;
  f32                max_anisotropy    = 1.0;
  bool               compare_enable    = false;
  CompareOp          compare_op        = CompareOp::Never;
  f32                min_lod           = 0;
  f32                max_lod           = 0;
  BorderColor        border_color      = BorderColor::FloatTransparentBlack;
  bool               unnormalized_coordinates = false;
};

struct ShaderInfo
{
  Str             label      = {};
  Span<u32 const> spirv_code = {};
};

/// @param count represents maximum count of the binding if
/// `is_variable_length` is true.
/// @param is_variable_length if it is a dynamically sized binding
struct DescriptorBindingInfo
{
  DescriptorType type               = DescriptorType::Sampler;
  u32            count              = 0;
  bool           is_variable_length = false;
};

struct DescriptorSetLayoutInfo
{
  Str                               label    = {};
  Span<DescriptorBindingInfo const> bindings = {};
};

struct DescriptorSetInfo
{
  Str                 label            = {};
  DescriptorSetLayout layout           = nullptr;
  Span<u32 const>     variable_lengths = {};
};

struct PipelineCacheInfo
{
  Str            label        = {};
  Span<u8 const> initial_data = {};
};

struct ImageBinding
{
  Sampler   sampler    = nullptr;
  ImageView image_view = nullptr;
};

struct BufferBinding
{
  Buffer  buffer = nullptr;
  Slice64 range  = {};
};

struct DescriptorSetUpdate
{
  DescriptorSet             set           = nullptr;
  u32                       binding       = 0;
  u32                       first_element = 0;
  Span<ImageBinding const>  images        = {};
  Span<BufferView const>    texel_buffers = {};
  Span<BufferBinding const> buffers       = {};
};

struct SpecializationConstant
{
  u32   constant_id = 0;
  u32   offset      = 0;
  usize size        = 0;
};

struct ShaderStageInfo
{
  Shader                             shader                        = nullptr;
  Str                                entry_point                   = {};
  Span<SpecializationConstant const> specialization_constants      = {};
  Span<u8 const>                     specialization_constants_data = {};
};

struct ComputePipelineInfo
{
  Str                             label                  = {};
  ShaderStageInfo                 compute_shader         = {};
  u32                             push_constants_size    = 0;
  Span<DescriptorSetLayout const> descriptor_set_layouts = {};
  PipelineCache                   cache                  = nullptr;
};

/// Specifies how the binded vertex buffers are iterated and the strides for
/// them unique for each binded buffer.
/// @param binding binding id this structure represents
/// @param stride stride in bytes for each binding advance within the binded
/// buffer
/// @param input_rate advance-rate for this binding. on every vertex or every
/// instance
struct VertexInputBinding
{
  u32       binding    = 0;
  u32       stride     = 0;
  InputRate input_rate = InputRate::Vertex;
};

/// specifies representation/interpretation and shader location mapping of the
/// values in the buffer this is a many to one mapping to the input binding.
/// @param binding which binding this attribute binds to
/// @param location binding's mapped location in the shader
/// @param format data format interpretation
/// @param offset offset of attribute in binding
struct VertexAttribute
{
  u32    binding  = 0;
  u32    location = 0;
  Format format   = Format::Undefined;
  u32    offset   = 0;
};

struct DepthStencilState
{
  bool         depth_test_enable        = false;
  bool         depth_write_enable       = false;
  CompareOp    depth_compare_op         = CompareOp::Never;
  bool         depth_bounds_test_enable = false;
  bool         stencil_test_enable      = false;
  StencilState front_stencil            = {};
  StencilState back_stencil             = {};
  f32          min_depth_bounds         = 0;
  f32          max_depth_bounds         = 0;
};

struct ColorBlendAttachmentState
{
  bool            blend_enable           = false;
  BlendFactor     src_color_blend_factor = BlendFactor::Zero;
  BlendFactor     dst_color_blend_factor = BlendFactor::Zero;
  BlendOp         color_blend_op         = BlendOp::Add;
  BlendFactor     src_alpha_blend_factor = BlendFactor::Zero;
  BlendFactor     dst_alpha_blend_factor = BlendFactor::Zero;
  BlendOp         alpha_blend_op         = BlendOp::Add;
  ColorComponents color_write_mask       = ColorComponents::None;
};

struct ColorBlendState
{
  bool                                  logic_op_enable = false;
  LogicOp                               logic_op        = LogicOp::Clear;
  Span<ColorBlendAttachmentState const> attachments     = {};
  f32x4                                 blend_constant  = {};
};

struct RasterizationState
{
  bool             depth_clamp_enable         = false;
  PolygonMode      polygon_mode               = PolygonMode::Fill;
  CullMode         cull_mode                  = CullMode::None;
  FrontFace        front_face                 = FrontFace::CounterClockWise;
  bool             depth_bias_enable          = false;
  f32              depth_bias_constant_factor = 0;
  f32              depth_bias_clamp           = 0;
  f32              depth_bias_slope_factor    = 0;
  gpu::SampleCount sample_count               = gpu::SampleCount::C1;
};

struct GraphicsState
{
  RectU        scissor                  = {};
  Viewport     viewport                 = {};
  f32x4        blend_constant           = {};
  bool         stencil_test_enable      = false;
  StencilState front_face_stencil       = {};
  StencilState back_face_stencil        = {};
  CullMode     cull_mode                = CullMode::None;
  FrontFace    front_face               = FrontFace::CounterClockWise;
  bool         depth_test_enable        = false;
  CompareOp    depth_compare_op         = CompareOp::Never;
  bool         depth_write_enable       = false;
  bool         depth_bounds_test_enable = false;
};

/// @param color_format, depth_format, stencil_format: with Format::Undefined
/// means the attachment is unused.
struct GraphicsPipelineInfo
{
  Str                             label                  = {};
  ShaderStageInfo                 vertex_shader          = {};
  ShaderStageInfo                 fragment_shader        = {};
  Span<Format const>              color_formats          = {};
  Option<Format>                  depth_format           = none;
  Option<Format>                  stencil_format         = none;
  Span<VertexInputBinding const>  vertex_input_bindings  = {};
  Span<VertexAttribute const>     vertex_attributes      = {};
  u32                             push_constants_size    = 0;
  Span<DescriptorSetLayout const> descriptor_set_layouts = {};
  PrimitiveTopology  primitive_topology  = PrimitiveTopology::PointList;
  RasterizationState rasterization_state = {};
  DepthStencilState  depth_stencil_state = {};
  ColorBlendState    color_blend_state   = {};
  PipelineCache      cache               = nullptr;
};

struct SwapchainInfo
{
  Str            label               = {};
  Surface        surface             = nullptr;
  SurfaceFormat  format              = {};
  ImageUsage     usage               = ImageUsage::None;
  u32            preferred_buffering = 0;
  PresentMode    present_mode        = PresentMode::Immediate;
  u32x2          preferred_extent    = {};
  CompositeAlpha composite_alpha     = CompositeAlpha::None;
};

struct QueueScopeInfo
{
  Str label     = {};
  u32 buffering = 0;
};

struct StatisticsQueryInfo
{
  Str label = {};
  u32 count = 0;
};

struct TimestampQueryInfo
{
  Str label = {};
  u32 count = 0;
};

struct CommandBufferInfo
{
  Str label = {};
};

struct CommandEncoderInfo
{
  Str label = {};
};

struct DispatchCommand
{
  u32 x = 0;
  u32 y = 0;
  u32 z = 0;
};

struct DrawCommand
{
  u32 vertex_count   = 0;
  u32 instance_count = 0;
  u32 first_vertex   = 0;
  u32 first_instance = 0;
};

struct DrawIndexedCommand
{
  u32 index_count    = 0;
  u32 instance_count = 0;
  u32 first_index    = 0;
  i32 vertex_offset  = 0;
  u32 first_instance = 0;
};

struct BufferCopy
{
  Slice64 src_range{};
  u64     dst_offset = 0;
};

struct BufferImageCopy
{
  u64                    buffer_offset       = 0;
  u32                    buffer_row_length   = 0;
  u32                    buffer_image_height = 0;
  ImageSubresourceLayers image_layers        = {};
  BoxU                   image_area          = {};
};

struct ImageCopy
{
  ImageSubresourceLayers src_layers = {};
  BoxU                   src_area   = {};
  ImageSubresourceLayers dst_layers = {};
  u32x3                  dst_offset = {};
};

struct ImageBlit
{
  ImageSubresourceLayers src_layers = {};
  BoxU                   src_area   = {};
  ImageSubresourceLayers dst_layers = {};
  BoxU                   dst_area   = {};
};

struct ImageResolve
{
  ImageSubresourceLayers src_layers = {};
  BoxU                   src_area   = {};
  ImageSubresourceLayers dst_layers = {};
  u32x3                  dst_offset = {};
};

/// x, y, z, w => R, G, B, A
union Color
{
  u32x4 u32 = {0, 0, 0, 0};
  i32x4 i32;
  f32x4 f32;
};

struct DepthStencil
{
  f32 depth   = 0;
  u32 stencil = 0;
};

union ClearValue
{
  Color        color = {};
  DepthStencil depth_stencil;
};

struct SurfaceCapabilities
{
  ImageUsage     image_usage     = ImageUsage::None;
  CompositeAlpha composite_alpha = CompositeAlpha::None;
};

struct PipelineStatistics
{
  u64 input_assembly_vertices     = 0;
  u64 input_assembly_primitives   = 0;
  u64 vertex_shader_invocations   = 0;
  u64 clipping_invocations        = 0;
  u64 clipping_primitives         = 0;
  u64 fragment_shader_invocations = 0;
  u64 compute_shader_invocations  = 0;
};

/// @param timestamp_period number of timestamp ticks equivalent to 1
/// nanosecond
struct DeviceProperties
{
  u32        api_version                        = 0;
  u32        driver_version                     = 0;
  u32        vendor_id                          = 0;
  u32        device_id                          = 0;
  Str        device_name                        = {};
  DeviceType type                               = DeviceType::Other;
  bool       has_unified_memory                 = false;
  bool       has_non_solid_fill_mode            = false;
  u64        texel_buffer_offset_alignment      = 0;
  u64        uniform_buffer_offset_alignment    = 0;
  u64        storage_buffer_offset_alignment    = 0;
  f32        timestamp_period                   = 0;
  u32        max_compute_work_group_count[3]    = {};
  u32        max_compute_work_group_size[3]     = {};
  u32        max_compute_work_group_invocations = 0;
  u32        max_compute_shared_memory_size     = 0;
};

/// @param generation increases everytime the swapchain for the surface is
/// recreated or re-configured
/// @param images swapchain images, calling ref or unref on them will cause a
/// panic as they are only meant to exist for the lifetime of the frame. avoid
/// storing pointers to its data members.
struct SwapchainState
{
  u32x2             extent          = {};
  SurfaceFormat     format          = {};
  PresentMode       present_mode    = PresentMode::Immediate;
  CompositeAlpha    composite_alpha = CompositeAlpha::None;
  Span<Image const> images          = {};
  Option<u32>       current_image   = none;
};

struct QueueScopeState
{
  u64 tail_frame    = 0;
  u64 current_frame = 0;
  u64 ring_index    = 0;
  u64 buffering     = 0;
};

struct RenderingAttachment
{
  ImageView    view         = nullptr;
  ImageView    resolve      = nullptr;
  ResolveModes resolve_mode = ResolveModes::None;
  LoadOp       load_op      = LoadOp::Load;
  StoreOp      store_op     = StoreOp::Store;
  ClearValue   clear        = {};
};

struct RenderingInfo
{
  RectU                           render_area        = {};
  u32                             num_layers         = 0;
  Span<RenderingAttachment const> color_attachments  = {};
  Option<RenderingAttachment>     depth_attachment   = none;
  Option<RenderingAttachment>     stencil_attachment = none;
};

struct CommandEncoder
{
  virtual void begin() = 0;

  virtual Status end() = 0;

  virtual void reset() = 0;

  virtual void reset_timestamp_query(TimestampQuery query, Slice32 range) = 0;

  virtual void reset_statistics_query(StatisticsQuery query, Slice32 range) = 0;

  virtual void write_timestamp(TimestampQuery query, PipelineStages stage,
                               u32 index) = 0;

  virtual void begin_statistics(StatisticsQuery query, u32 index) = 0;

  virtual void end_statistics(StatisticsQuery query, u32 index) = 0;

  virtual void begin_debug_marker(Str region_name, f32x4 color) = 0;

  virtual void end_debug_marker() = 0;

  virtual void fill_buffer(Buffer dst, Slice64 range, u32 data) = 0;

  virtual void copy_buffer(Buffer src, Buffer dst,
                           Span<BufferCopy const> copies) = 0;

  virtual void update_buffer(Span<u8 const> src, u64 dst_offset,
                             Buffer dst) = 0;

  virtual void clear_color_image(Image dst, Color value,
                                 Span<ImageSubresourceRange const> ranges) = 0;

  virtual void
    clear_depth_stencil_image(Image dst, DepthStencil value,
                              Span<ImageSubresourceRange const> ranges) = 0;

  virtual void copy_image(Image src, Image dst,
                          Span<ImageCopy const> copies) = 0;

  virtual void copy_buffer_to_image(Buffer src, Image dst,
                                    Span<BufferImageCopy const> copies) = 0;

  virtual void blit_image(Image src, Image dst, Span<ImageBlit const> blits,
                          Filter filter) = 0;

  virtual void resolve_image(Image src, Image dst,
                             Span<ImageResolve const> resolves) = 0;

  virtual void begin_compute_pass() = 0;

  virtual void end_compute_pass() = 0;

  virtual void begin_rendering(RenderingInfo const & info) = 0;

  virtual void end_rendering() = 0;

  virtual void bind_compute_pipeline(ComputePipeline pipeline) = 0;

  virtual void bind_graphics_pipeline(GraphicsPipeline pipeline) = 0;

  virtual void bind_descriptor_sets(Span<DescriptorSet const> descriptor_sets,
                                    Span<u32 const> dynamic_offsets) = 0;

  virtual void push_constants(Span<u8 const> push_constants_data) = 0;

  virtual void dispatch(u32x3 group_count) = 0;

  virtual void dispatch_indirect(Buffer buffer, u64 offset) = 0;

  virtual void set_graphics_state(GraphicsState const & state) = 0;

  virtual void bind_vertex_buffers(Span<Buffer const> vertex_buffers,
                                   Span<u64 const>    offsets) = 0;

  virtual void bind_index_buffer(Buffer index_buffer, u64 offset,
                                 IndexType index_type) = 0;

  virtual void draw(Slice32 vertices, Slice32 instances) = 0;

  virtual void draw_indexed(Slice32 indices, Slice32 instances,
                            i32 vertex_offset) = 0;

  virtual void draw_indirect(Buffer buffer, u64 offset, u32 draw_count,
                             u32 stride) = 0;

  virtual void draw_indexed_indirect(Buffer buffer, u64 offset, u32 draw_count,
                                     u32 stride) = 0;

  virtual void present(Swapchain swapchain) = 0;
};

struct CommandBuffer
{
  virtual void begin() = 0;

  virtual Status end() = 0;

  virtual void reset() = 0;

  virtual void record(CommandEncoder & encoder) = 0;
};

struct Device
{
  virtual Result<Buffer, Status> create_buffer(BufferInfo const & info) = 0;

  virtual Result<BufferView, Status>
    create_buffer_view(BufferViewInfo const & info) = 0;

  virtual Result<Image, Status> create_image(ImageInfo const & info) = 0;

  virtual Result<ImageView, Status>
    create_image_view(ImageViewInfo const & info) = 0;

  virtual Result<MemoryGroup, Status>
    create_memory_group(MemoryGroupInfo const & info) = 0;

  virtual Result<Sampler, Status> create_sampler(SamplerInfo const & info) = 0;

  virtual Result<Shader, Status> create_shader(ShaderInfo const & info) = 0;

  virtual Result<DescriptorSetLayout, Status>
    create_descriptor_set_layout(DescriptorSetLayoutInfo const & info) = 0;

  virtual Result<DescriptorSet, Status>
    create_descriptor_set(DescriptorSetInfo const & info) = 0;

  virtual Result<PipelineCache, Status>
    create_pipeline_cache(PipelineCacheInfo const & info) = 0;

  virtual Result<ComputePipeline, Status>
    create_compute_pipeline(ComputePipelineInfo const & info) = 0;

  virtual Result<GraphicsPipeline, Status>
    create_graphics_pipeline(GraphicsPipelineInfo const & info) = 0;

  virtual Result<Swapchain, Status>
    create_swapchain(SwapchainInfo const & info) = 0;

  virtual Result<TimestampQuery, Status>
    create_timestamp_query(TimestampQueryInfo const & info) = 0;

  virtual Result<StatisticsQuery, Status>
    create_statistics_query(StatisticsQueryInfo const & info) = 0;

  virtual Result<CommandEncoderPtr, Status>
    create_command_encoder(CommandEncoderInfo const & info) = 0;

  virtual Result<CommandBufferPtr, Status>
    create_command_buffer(CommandBufferInfo const & info) = 0;

  virtual Result<QueueScope, Status>
    create_queue_scope(QueueScopeInfo const & info) = 0;

  virtual void uninit(Buffer buffer) = 0;

  virtual void uninit(BufferView buffer_view) = 0;

  virtual void uninit(Image image) = 0;

  virtual void uninit(ImageView image_view) = 0;

  virtual void uninit(MemoryGroup memory_group) = 0;

  virtual void uninit(Sampler sampler) = 0;

  virtual void uninit(Shader shader) = 0;

  virtual void uninit(DescriptorSetLayout layout) = 0;

  virtual void uninit(DescriptorSet set) = 0;

  virtual void uninit(PipelineCache cache) = 0;

  virtual void uninit(ComputePipeline pipeline) = 0;

  virtual void uninit(GraphicsPipeline pipeline) = 0;

  virtual void uninit(Swapchain swapchain) = 0;

  virtual void uninit(TimestampQuery query) = 0;

  virtual void uninit(StatisticsQuery query) = 0;

  virtual void uninit(CommandEncoderPtr encoder) = 0;

  virtual void uninit(CommandBufferPtr buffer) = 0;

  virtual void uninit(QueueScope scope) = 0;

  virtual DeviceProperties get_properties() = 0;

  virtual Result<FormatProperties, Status>
    get_format_properties(Format format) = 0;

  virtual Result<Span<u8>, Status> get_memory_map(Buffer buffer) = 0;

  virtual Result<Void, Status> invalidate_mapped_memory(Buffer  buffer,
                                                        Slice64 range) = 0;

  virtual Result<Void, Status> flush_mapped_memory(Buffer  buffer,
                                                   Slice64 range) = 0;

  virtual Result<usize, Status>
    get_pipeline_cache_size(PipelineCache cache) = 0;

  virtual Result<Void, Status> get_pipeline_cache_data(PipelineCache cache,
                                                       Vec<u8> &     out) = 0;

  virtual Result<Void, Status>
    merge_pipeline_cache(PipelineCache dst, Span<PipelineCache const> srcs) = 0;

  virtual void update_descriptor_set(DescriptorSetUpdate const & update) = 0;

  virtual QueueScopeState get_queue_scope_state(QueueScope scope) = 0;

  virtual Result<Void, Status> wait_idle() = 0;

  virtual Result<Void, Status> wait_queue_idle() = 0;

  virtual Result<Void, Status>
    get_surface_formats(Surface surface, Vec<SurfaceFormat> & formats) = 0;

  virtual Result<Void, Status>
    get_surface_present_modes(Surface surface, Vec<PresentMode> & modes) = 0;

  virtual Result<SurfaceCapabilities, Status>
    get_surface_capabilities(Surface surface) = 0;

  virtual Result<SwapchainState, Status>
    get_swapchain_state(Swapchain swapchain) = 0;

  virtual Result<Void, Status>
    get_timestamp_query_result(TimestampQuery query, Slice32 range,
                               Vec<u64> & timestamps) = 0;

  virtual Result<Void, Status>
    get_statistics_query_result(StatisticsQuery query, Slice32 range,
                                Vec<PipelineStatistics> & statistics) = 0;

  virtual Result<Void, Status> acquire_next(Swapchain swapchain) = 0;

  virtual Result<Void, Status> submit(CommandBufferPtr buffer,
                                      QueueScope       scope) = 0;
};

struct Instance
{
  virtual ~Instance() = default;

  virtual Result<DevicePtr, Status>
    create_device(AllocatorRef           allocator,
                  Span<DeviceType const> preferred_types) = 0;

  virtual Backend get_backend() = 0;

  virtual void uninit(DevicePtr device) = 0;

  virtual void uninit(Surface surface) = 0;
};

Result<Dyn<InstancePtr>, Status> create_vulkan_instance(AllocatorRef allocator,
                                                        bool enable_validation);

/// REQUIRED LIMITS AND PROPERTIES

inline constexpr u32         MAX_IMAGE_EXTENT_1D                  = 8'192;
inline constexpr u32         MAX_IMAGE_EXTENT_2D                  = 8'192;
inline constexpr u32         MAX_IMAGE_EXTENT_3D                  = 2'048;
inline constexpr u32         MAX_IMAGE_EXTENT_CUBE                = 8'192;
inline constexpr u32         MAX_IMAGE_ARRAY_LAYERS               = 1'024;
inline constexpr u32         MAX_VIEWPORT_EXTENT                  = 8'192;
inline constexpr u32         MAX_FRAMEBUFFER_EXTENT               = 8'192;
inline constexpr u32         MAX_FRAMEBUFFER_LAYERS               = 1'024;
inline constexpr u32         MAX_VERTEX_ATTRIBUTES                = 16;
inline constexpr u32         MAX_PUSH_CONSTANTS_SIZE              = 128;
inline constexpr u32         MAX_UPDATE_BUFFER_SIZE               = 65'536;
inline constexpr u32         MAX_PIPELINE_DESCRIPTOR_SETS         = 8;
inline constexpr u32         MAX_PIPELINE_DYNAMIC_UNIFORM_BUFFERS = 8;
inline constexpr u32         MAX_PIPELINE_DYNAMIC_STORAGE_BUFFERS = 8;
inline constexpr u32         MAX_PIPELINE_INPUT_ATTACHMENTS       = 8;
inline constexpr u32         MAX_PIPELINE_COLOR_ATTACHMENTS       = 8;
inline constexpr u32         MAX_DESCRIPTOR_SET_DESCRIPTORS       = 4'096;
inline constexpr u32         MAX_BINDING_DESCRIPTORS              = 4'096;
inline constexpr u32         MAX_DESCRIPTOR_SET_BINDINGS          = 16;
inline constexpr u32         MAX_FRAME_BUFFERING                  = 4;
inline constexpr u32         MAX_SWAPCHAIN_IMAGES                 = 4;
inline constexpr u64         MAX_UNIFORM_BUFFER_RANGE             = 65'536;
inline constexpr f32         MAX_SAMPLER_ANISOTROPY               = 16;
inline constexpr u32         MAX_CLIP_DISTANCES                   = 8;
inline constexpr u32         MAX_CULL_DISTANCES                   = 8;
inline constexpr u32         MAX_COMBINED_CLIP_AND_CULL_DISTANCES = 8;
inline constexpr u32         BUFFER_OFFSET_ALIGNMENT              = 512;
inline constexpr SampleCount REQUIRED_COLOR_SAMPLE_COUNTS =
  SampleCount::C1 | SampleCount::C2 | SampleCount::C4;
inline constexpr SampleCount REQUIRED_DEPTH_SAMPLE_COUNTS =
  SampleCount::C1 | SampleCount::C2 | SampleCount::C4;

}    // namespace gpu

inline void format(fmt::Sink sink, fmt::Spec, gpu::Status const & status)
{
  sink(gpu::to_str(status));
}

}    // namespace ash
