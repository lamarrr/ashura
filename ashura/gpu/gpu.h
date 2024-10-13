/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/allocator.h"
#include "ashura/std/math.h"
#include "ashura/std/option.h"
#include "ashura/std/result.h"
#include "ashura/std/types.h"

namespace ash
{
namespace gpu
{

constexpr u32 REMAINING_MIP_LEVELS   = ~0U;
constexpr u32 REMAINING_ARRAY_LAYERS = ~0U;
constexpr u64 WHOLE_SIZE             = ~0ULL;

constexpr u32 MAX_IMAGE_EXTENT_1D                  = 8192;
constexpr u32 MAX_IMAGE_EXTENT_2D                  = 8192;
constexpr u32 MAX_IMAGE_EXTENT_3D                  = 2048;
constexpr u32 MAX_IMAGE_EXTENT_CUBE                = 8192;
constexpr u32 MAX_IMAGE_ARRAY_LAYERS               = 1024;
constexpr u32 MAX_VIEWPORT_EXTENT                  = 8192;
constexpr u32 MAX_FRAMEBUFFER_EXTENT               = 8192;
constexpr u32 MAX_FRAMEBUFFER_LAYERS               = 1024;
constexpr u32 MAX_VERTEX_ATTRIBUTES                = 16;
constexpr u32 MAX_PUSH_CONSTANTS_SIZE              = 128;
constexpr u32 MAX_UPDATE_BUFFER_SIZE               = 65536;
constexpr u32 MAX_PIPELINE_DESCRIPTOR_SETS         = 8;
constexpr u32 MAX_PIPELINE_DYNAMIC_UNIFORM_BUFFERS = 8;
constexpr u32 MAX_PIPELINE_DYNAMIC_STORAGE_BUFFERS = 8;
constexpr u32 MAX_PIPELINE_INPUT_ATTACHMENTS       = 8;
constexpr u32 MAX_PIPELINE_COLOR_ATTACHMENTS       = 8;
constexpr u32 MAX_DESCRIPTOR_SET_DESCRIPTORS       = 4096;
constexpr u32 MAX_BINDING_DESCRIPTORS              = 4096;
constexpr u32 MAX_DESCRIPTOR_SET_BINDINGS          = 16;
constexpr u32 MAX_FRAME_BUFFERING                  = 4;
constexpr u32 MAX_SWAPCHAIN_IMAGES                 = 4;
constexpr u64 MAX_UNIFORM_BUFFER_RANGE             = 65536;
constexpr f32 MAX_SAMPLER_ANISOTROPY               = 16;
constexpr u32 MAX_CLIP_DISTANCES                   = 8;
constexpr u32 MAX_CULL_DISTANCES                   = 8;
constexpr u32 MAX_COMBINED_CLIP_AND_CULL_DISTANCES = 8;

typedef Vec2U Offset;
typedef Vec2U Extent;
typedef RectU Rect;
typedef Vec3U Offset3D;
typedef Vec3U Extent3D;
typedef u64   FrameId;

typedef struct Buffer_T              *Buffer;
typedef struct BufferView_T          *BufferView;
typedef struct Image_T               *Image;
typedef struct ImageView_T           *ImageView;
typedef struct Sampler_T             *Sampler;
typedef struct Shader_T              *Shader;
typedef struct DescriptorSetLayout_T *DescriptorSetLayout;
typedef struct DescriptorSet_T       *DescriptorSet;
typedef struct PipelineCache_T       *PipelineCache;
typedef struct ComputePipeline_T     *ComputePipeline;
typedef struct GraphicsPipeline_T    *GraphicsPipeline;
typedef struct TimestampQuery_T      *TimeStampQuery;
typedef struct StatisticsQuery_T     *StatisticsQuery;
typedef struct CommandEncoder_T      *CommandEncoder;
typedef struct Surface_T             *Surface;
typedef struct Swapchain_T           *Swapchain;
typedef struct Device_T              *Device;
typedef struct Instance_T            *Instance;

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
  TimeStampQuery      = 15,
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

ASH_DEFINE_ENUM_BIT_OPS(MemoryProperties)

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
  SurfaceLost          = -1000000000
};

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
  DISPLAY_P3_NONLINEAR    = 1000104001,
  EXTENDED_SRGB_LINEAR    = 1000104002,
  DISPLAY_P3_LINEAR       = 1000104003,
  DCI_P3_NONLINEAR        = 1000104004,
  BT709_LINEAR            = 1000104005,
  BT709_NONLINEAR         = 1000104006,
  BT2020_LINEAR           = 1000104007,
  HDR10_ST2084            = 1000104008,
  DOLBYVISION             = 1000104009,
  HDR10_HLG               = 1000104010,
  ADOBERGB_LINEAR         = 1000104011,
  ADOBERGB_NONLINEAR      = 1000104012,
  PASS_THROUGH            = 1000104013,
  EXTENDED_SRGB_NONLINEAR = 1000104014
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

ASH_DEFINE_ENUM_BIT_OPS(FormatFeatures)

enum class ImageAspects : u8
{
  None    = 0x00U,
  Color   = 0x01U,
  Depth   = 0x02U,
  Stencil = 0x04U
};

ASH_DEFINE_ENUM_BIT_OPS(ImageAspects)

enum class SampleCount : u8
{
  None    = 0x00U,
  Count1  = 0x01U,
  Count2  = 0x02U,
  Count4  = 0x04U,
  Count8  = 0x08U,
  Count16 = 0x10U,
  Count32 = 0x20U,
  Count64 = 0x40U
};

ASH_DEFINE_ENUM_BIT_OPS(SampleCount)

enum class LoadOp : u8
{
  Load     = 0,
  Clear    = 1,
  DontCare = 2
};

enum class StoreOp : u8
{
  Store    = 0,
  DontCare = 1
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

ASH_DEFINE_ENUM_BIT_OPS(ColorComponents)

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

ASH_DEFINE_ENUM_BIT_OPS(BufferUsage)

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

ASH_DEFINE_ENUM_BIT_OPS(ImageUsage)

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

ASH_DEFINE_ENUM_BIT_OPS(ShaderStages)

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
  StorageBuffer        = 7,
  DynamicUniformBuffer = 8,
  DynamicStorageBuffer = 9,
  InputAttachment      = 10
};

enum class IndexType : u8
{
  Uint16 = 0,
  Uint32 = 1
};

enum class CompositeAlpha : u32
{
  None           = 0x00U,
  Opaque         = 0x01U,
  PreMultiplied  = 0x02U,
  PostMultiplied = 0x04U,
  Inherit        = 0x08U
};

ASH_DEFINE_ENUM_BIT_OPS(CompositeAlpha)

enum class ResolveModes : u32
{
  None       = 0x00,
  SampleZero = 0x01,
  Average    = 0x02,
  Min        = 0x04,
  Max        = 0x08
};

ASH_DEFINE_ENUM_BIT_OPS(ResolveModes)

struct Object
{
  union
  {
    void               *handle = nullptr;
    Instance            instance;
    Device              device;
    CommandEncoder      command_encoder;
    Buffer              buffer;
    BufferView          buffer_view;
    Image               image;
    ImageView           image_view;
    Sampler             sampler;
    Shader              shader;
    DescriptorSetLayout descriptor_set_layout;
    DescriptorSet       descriptor_set;
    PipelineCache       pipeline_cache;
    ComputePipeline     compute_pipeline;
    GraphicsPipeline    graphics_pipeline;
    TimeStampQuery      timestamp_query;
    StatisticsQuery     statistics_query;
    Surface             surface;
    Swapchain           swapchain;
  };
  ObjectType type = ObjectType::None;
};

struct SurfaceFormat
{
  Format     format      = Format::Undefined;
  ColorSpace color_space = ColorSpace::SRGB_NONLINEAR;
};

struct MemoryRange
{
  u64 offset = 0;
  u64 size   = 0;
};

/// @param extent can be negative to flip
struct Viewport
{
  Vec2 offset    = {};
  Vec2 extent    = {};
  f32  min_depth = 0;
  f32  max_depth = 0;
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
  ImageAspects aspects           = ImageAspects::None;
  u32          first_mip_level   = 0;
  u32          num_mip_levels    = 0;
  u32          first_array_layer = 0;
  u32          num_array_layers  = 0;
};

struct ImageSubresourceLayers
{
  ImageAspects aspects           = ImageAspects::None;
  u32          mip_level         = 0;
  u32          first_array_layer = 0;
  u32          num_array_layers  = 0;
};

struct BufferDesc
{
  Span<char const> label       = {};
  u64              size        = 0;
  bool             host_mapped = false;
  BufferUsage      usage       = BufferUsage::None;
};

/// format interpretation of a buffer's contents
struct BufferViewDesc
{
  Span<char const> label  = {};
  Buffer           buffer = nullptr;
  Format           format = Format::Undefined;
  u64              offset = 0;
  u64              size   = 0;
};

struct ImageDesc
{
  Span<char const> label        = {};
  ImageType        type         = ImageType::Type1D;
  Format           format       = Format::Undefined;
  ImageUsage       usage        = ImageUsage::None;
  ImageAspects     aspects      = ImageAspects::None;
  Extent3D         extent       = {};
  u32              mip_levels   = 0;
  u32              array_layers = 0;
  SampleCount      sample_count = SampleCount::None;
};

/// a sub-resource that specifies mips, aspects, layer, and component mapping of
/// images. typically for reference in shaders.
///
/// @param mapping mapping of the components in the shader. i.e. for
/// R8G8B8_UNORM the non-existent Alpha component is always 0. To set it to 1 we
/// set its component mapping (mapping.a) to ComponentSwizzle::One.
///
struct ImageViewDesc
{
  Span<char const> label             = {};
  Image            image             = nullptr;
  ImageViewType    view_type         = ImageViewType::Type1D;
  Format           view_format       = Format::Undefined;
  ComponentMapping mapping           = {};
  ImageAspects     aspects           = ImageAspects::None;
  u32              first_mip_level   = 0;
  u32              num_mip_levels    = 0;
  u32              first_array_layer = 0;
  u32              num_array_layers  = 0;
};

struct SamplerDesc
{
  Span<char const>   label             = {};
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

struct ShaderDesc
{
  Span<char const> label      = {};
  Span<u32 const>  spirv_code = {};
};

/// @param count represents maximum count of the binding if
/// `is_variable_length` is true.
/// @param is_variable_length if it is a dynamically sized binding
struct DescriptorBindingDesc
{
  DescriptorType type               = DescriptorType::Sampler;
  u32            count              = 0;
  bool           is_variable_length = false;
};

struct DescriptorSetLayoutDesc
{
  Span<char const>                  label    = {};
  Span<DescriptorBindingDesc const> bindings = {};
};

struct PipelineCacheDesc
{
  Span<char const> label        = {};
  Span<u8 const>   initial_data = {};
};

struct ImageBinding
{
  Sampler   sampler    = nullptr;
  ImageView image_view = nullptr;
};

struct BufferBinding
{
  Buffer buffer = nullptr;
  u64    offset = 0;
  u64    size   = 0;
};

struct DescriptorSetUpdate
{
  DescriptorSet             set           = nullptr;
  u32                       binding       = 0;
  u32                       element       = 0;
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

struct ShaderStageDesc
{
  Shader                             shader                        = nullptr;
  Span<char const>                   entry_point                   = {};
  Span<SpecializationConstant const> specialization_constants      = {};
  Span<u8 const>                     specialization_constants_data = {};
};

struct ComputePipelineDesc
{
  Span<char const>                label                  = {};
  ShaderStageDesc                 compute_shader         = {};
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
  Vec4                                  blend_constant  = {};
};

struct RasterizationState
{
  bool        depth_clamp_enable         = false;
  PolygonMode polygon_mode               = PolygonMode::Fill;
  CullMode    cull_mode                  = CullMode::None;
  FrontFace   front_face                 = FrontFace::CounterClockWise;
  bool        depth_bias_enable          = false;
  f32         depth_bias_constant_factor = 0;
  f32         depth_bias_clamp           = 0;
  f32         depth_bias_slope_factor    = 0;
};

struct GraphicsState
{
  Rect         scissor                  = {};
  Viewport     viewport                 = {};
  Vec4         blend_constant           = {};
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
struct GraphicsPipelineDesc
{
  Span<char const>                label                  = {};
  ShaderStageDesc                 vertex_shader          = {};
  ShaderStageDesc                 fragment_shader        = {};
  Span<Format const>              color_formats          = {};
  Span<Format const>              depth_format           = {};
  Span<Format const>              stencil_format         = {};
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
  u64 src_offset = 0;
  u64 dst_offset = 0;
  u64 size       = 0;
};

struct BufferImageCopy
{
  u64                    buffer_offset       = 0;
  u32                    buffer_row_length   = 0;
  u32                    buffer_image_height = 0;
  ImageSubresourceLayers image_layers        = {};
  Offset3D               image_offset        = {};
  Extent3D               image_extent        = {};
};

struct ImageCopy
{
  ImageSubresourceLayers src_layers = {};
  Offset3D               src_offset = {};
  ImageSubresourceLayers dst_layers = {};
  Offset3D               dst_offset = {};
  Extent3D               extent     = {};
};

struct ImageBlit
{
  ImageSubresourceLayers src_layers     = {};
  Offset3D               src_offsets[2] = {};
  ImageSubresourceLayers dst_layers     = {};
  Offset3D               dst_offsets[2] = {};
};

struct ImageResolve
{
  ImageSubresourceLayers src_layers = {};
  Offset3D               src_offset = {};
  ImageSubresourceLayers dst_layers = {};
  Offset3D               dst_offset = {};
  Extent3D               extent     = {};
};

/// x, y, z, w => R, G, B, A
union Color
{
  Vec4U uint32 = {0, 0, 0, 0};
  Vec4I int32;
  Vec4  float32;
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

struct SwapchainDesc
{
  Span<char const> label               = {};
  SurfaceFormat    format              = {};
  ImageUsage       usage               = ImageUsage::None;
  u32              preferred_buffering = 0;
  PresentMode      present_mode        = PresentMode::Immediate;
  Extent           preferred_extent    = {};
  CompositeAlpha   composite_alpha     = CompositeAlpha::None;
};

/// @param generation increases everytime the swapchain for the surface is
/// recreated or re-configured
/// @param images swapchain images, calling ref or unref on them will cause a
/// panic as they are only meant to exist for the lifetime of the frame. avoid
/// storing pointers to its data members.
struct SwapchainState
{
  Extent            extent        = {};
  SurfaceFormat     format        = {};
  Span<Image const> images        = {};
  Option<u32>       current_image = None;
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
  u32              api_version                        = 0;
  u32              driver_version                     = 0;
  u32              vendor_id                          = 0;
  u32              device_id                          = 0;
  Span<char const> device_name                        = {};
  DeviceType       type                               = DeviceType::Other;
  bool             has_unified_memory                 = false;
  bool             has_non_solid_fill_mode            = false;
  u64              texel_buffer_offset_alignment      = 0;
  u64              uniform_buffer_offset_alignment    = 0;
  u64              storage_buffer_offset_alignment    = 0;
  f32              timestamp_period                   = 0;
  u32              max_compute_work_group_count[3]    = {};
  u32              max_compute_work_group_size[3]     = {};
  u32              max_compute_work_group_invocations = 0;
  u32              max_compute_shared_memory_size     = 0;
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
  Rect                            render_area        = {};
  u32                             num_layers         = 0;
  Span<RenderingAttachment const> color_attachments  = {};
  Span<RenderingAttachment const> depth_attachment   = {};
  Span<RenderingAttachment const> stencil_attachment = {};
};

/// to execute tasks at end of frame. use the tail frame index.
struct CommandEncoderInterface
{
  void (*reset_timestamp_query)(CommandEncoder self,
                                TimeStampQuery query)                 = nullptr;
  void (*reset_statistics_query)(CommandEncoder  self,
                                 StatisticsQuery query)               = nullptr;
  void (*write_timestamp)(CommandEncoder self, TimeStampQuery query)  = nullptr;
  void (*begin_statistics)(CommandEncoder  self,
                           StatisticsQuery query)                     = nullptr;
  void (*end_statistics)(CommandEncoder self, StatisticsQuery query)  = nullptr;
  void (*begin_debug_marker)(CommandEncoder self, Span<char const> region_name,
                             Vec4 color)                              = nullptr;
  void (*end_debug_marker)(CommandEncoder self)                       = nullptr;
  void (*fill_buffer)(CommandEncoder self, Buffer dst, u64 offset, u64 size,
                      u32 data)                                       = nullptr;
  void (*copy_buffer)(CommandEncoder self, Buffer src, Buffer dst,
                      Span<BufferCopy const> copies)                  = nullptr;
  void (*update_buffer)(CommandEncoder self, Span<u8 const> src, u64 dst_offset,
                        Buffer dst)                                   = nullptr;
  void (*clear_color_image)(CommandEncoder self, Image dst, Color clear_color,
                            Span<ImageSubresourceRange const> ranges) = nullptr;
  void (*clear_depth_stencil_image)(
      CommandEncoder self, Image dst, DepthStencil clear_depth_stencil,
      Span<ImageSubresourceRange const> ranges)                    = nullptr;
  void (*copy_image)(CommandEncoder self, Image src, Image dst,
                     Span<ImageCopy const> copies)                 = nullptr;
  void (*copy_buffer_to_image)(CommandEncoder self, Buffer src, Image dst,
                               Span<BufferImageCopy const> copies) = nullptr;
  void (*blit_image)(CommandEncoder self, Image src, Image dst,
                     Span<ImageBlit const> blits, Filter filter)   = nullptr;
  void (*resolve_image)(CommandEncoder self, Image src, Image dst,
                        Span<ImageResolve const> resolves)         = nullptr;
  void (*begin_compute_pass)(CommandEncoder self)                  = nullptr;
  void (*end_compute_pass)(CommandEncoder self)                    = nullptr;
  void (*begin_rendering)(CommandEncoder       self,
                          RenderingInfo const &info)               = nullptr;
  void (*end_rendering)(CommandEncoder self)                       = nullptr;
  void (*bind_compute_pipeline)(CommandEncoder  self,
                                ComputePipeline pipeline)          = nullptr;
  void (*bind_graphics_pipeline)(CommandEncoder   self,
                                 GraphicsPipeline pipeline)        = nullptr;
  void (*bind_descriptor_sets)(CommandEncoder            self,
                               Span<DescriptorSet const> descriptor_sets,
                               Span<u32 const> dynamic_offsets)    = nullptr;
  void (*push_constants)(CommandEncoder self,
                         Span<u8 const> push_constants_data)       = nullptr;
  void (*dispatch)(CommandEncoder self, u32 group_count_x, u32 group_count_y,
                   u32 group_count_z)                              = nullptr;
  void (*dispatch_indirect)(CommandEncoder self, Buffer buffer,
                            u64 offset)                            = nullptr;
  void (*set_graphics_state)(CommandEncoder       self,
                             GraphicsState const &state)           = nullptr;
  void (*bind_vertex_buffers)(CommandEncoder     self,
                              Span<Buffer const> vertex_buffers,
                              Span<u64 const>    offsets)             = nullptr;
  void (*bind_index_buffer)(CommandEncoder self, Buffer index_buffer,
                            u64 offset, IndexType index_type)      = nullptr;
  void (*draw)(CommandEncoder self, u32 vertex_count, u32 instance_count,
               u32 first_vertex_id, u32 first_instance_id)         = nullptr;
  void (*draw_indexed)(CommandEncoder self, u32 first_index, u32 num_indices,
                       i32 vertex_offset, u32 first_instance_id,
                       u32 num_instances)                          = nullptr;
  void (*draw_indirect)(CommandEncoder self, Buffer buffer, u64 offset,
                        u32 draw_count, u32 stride)                = nullptr;
  void (*draw_indexed_indirect)(CommandEncoder self, Buffer buffer, u64 offset,
                                u32 draw_count, u32 stride)        = nullptr;
};

struct CommandEncoderImpl
{
  CommandEncoder                 self      = nullptr;
  CommandEncoderInterface const *interface = nullptr;

  constexpr CommandEncoderInterface const *operator->() const
  {
    return interface;
  }
};

struct FrameContext
{
  u32                            buffering  = 0;
  FrameId                        tail       = 0;
  FrameId                        current    = 0;
  Span<CommandEncoderImpl const> encoders   = {};
  u32                            ring_index = 0;
};

struct DeviceInterface
{
  DeviceProperties (*get_device_properties)(Device self) = nullptr;
  Result<FormatProperties, Status> (*get_format_properties)(
      Device self, Format format)                                 = nullptr;
  Result<Buffer, Status> (*create_buffer)(Device            self,
                                          BufferDesc const &desc) = nullptr;
  Result<BufferView, Status> (*create_buffer_view)(
      Device self, BufferViewDesc const &desc)                 = nullptr;
  Result<Image, Status> (*create_image)(Device           self,
                                        ImageDesc const &desc) = nullptr;
  Result<ImageView, Status> (*create_image_view)(
      Device self, ImageViewDesc const &desc)                        = nullptr;
  Result<Sampler, Status> (*create_sampler)(Device             self,
                                            SamplerDesc const &desc) = nullptr;
  Result<Shader, Status> (*create_shader)(Device            self,
                                          ShaderDesc const &desc)    = nullptr;
  Result<DescriptorSetLayout, Status> (*create_descriptor_set_layout)(
      Device self, DescriptorSetLayoutDesc const &desc) = nullptr;
  Result<DescriptorSet, Status> (*create_descriptor_set)(
      Device self, DescriptorSetLayout layout,
      Span<u32 const> variable_lengths) = nullptr;
  Result<PipelineCache, Status> (*create_pipeline_cache)(
      Device self, PipelineCacheDesc const &desc) = nullptr;
  Result<ComputePipeline, Status> (*create_compute_pipeline)(
      Device self, ComputePipelineDesc const &desc) = nullptr;
  Result<GraphicsPipeline, Status> (*create_graphics_pipeline)(
      Device self, GraphicsPipelineDesc const &desc) = nullptr;
  Result<Swapchain, Status> (*create_swapchain)(
      Device self, Surface surface, SwapchainDesc const &desc) = nullptr;
  Result<TimeStampQuery, Status> (*create_timestamp_query)(Device self) =
      nullptr;
  Result<StatisticsQuery, Status> (*create_statistics_query)(Device self) =
      nullptr;
  void (*uninit_buffer)(Device self, Buffer buffer)                   = nullptr;
  void (*uninit_buffer_view)(Device self, BufferView buffer_view)     = nullptr;
  void (*uninit_image)(Device self, Image image)                      = nullptr;
  void (*uninit_image_view)(Device self, ImageView image_view)        = nullptr;
  void (*uninit_sampler)(Device self, Sampler sampler)                = nullptr;
  void (*uninit_shader)(Device self, Shader shader)                   = nullptr;
  void (*uninit_descriptor_set_layout)(Device              self,
                                       DescriptorSetLayout layout)    = nullptr;
  void (*uninit_descriptor_set)(Device self, DescriptorSet set)       = nullptr;
  void (*uninit_pipeline_cache)(Device self, PipelineCache cache)     = nullptr;
  void (*uninit_compute_pipeline)(Device          self,
                                  ComputePipeline pipeline)           = nullptr;
  void (*uninit_graphics_pipeline)(Device           self,
                                   GraphicsPipeline pipeline)         = nullptr;
  void (*uninit_swapchain)(Device self, Swapchain swapchain)          = nullptr;
  void (*uninit_timestamp_query)(Device self, TimeStampQuery query)   = nullptr;
  void (*uninit_statistics_query)(Device self, StatisticsQuery query) = nullptr;
  FrameContext (*get_frame_context)(Device self)                      = nullptr;
  Result<void *, Status> (*map_buffer_memory)(Device self,
                                              Buffer buffer)          = nullptr;
  void (*unmap_buffer_memory)(Device self, Buffer buffer)             = nullptr;
  Result<Void, Status> (*invalidate_mapped_buffer_memory)(
      Device self, Buffer buffer, MemoryRange range) = nullptr;
  Result<Void, Status> (*flush_mapped_buffer_memory)(
      Device self, Buffer buffer, MemoryRange range) = nullptr;
  Result<usize, Status> (*get_pipeline_cache_size)(
      Device self, PipelineCache cache)                          = nullptr;
  Result<usize, Status> (*get_pipeline_cache_data)(Device        self,
                                                   PipelineCache cache,
                                                   Span<u8>      out) = nullptr;
  Result<Void, Status> (*merge_pipeline_cache)(
      Device self, PipelineCache dst, Span<PipelineCache const> srcs) = nullptr;
  void (*update_descriptor_set)(Device                     self,
                                DescriptorSetUpdate const &update)    = nullptr;
  Result<Void, Status> (*wait_idle)(Device self)                      = nullptr;
  Result<Void, Status> (*wait_queue_idle)(Device self)                = nullptr;
  Result<u32, Status> (*get_surface_formats)(
      Device self, Surface surface, Span<SurfaceFormat> formats) = nullptr;
  Result<u32, Status> (*get_surface_present_modes)(
      Device self, Surface surface, Span<PresentMode> modes) = nullptr;
  Result<SurfaceCapabilities, Status> (*get_surface_capabilities)(
      Device self, Surface surface) = nullptr;
  Result<SwapchainState, Status> (*get_swapchain_state)(
      Device self, Swapchain swapchain) = nullptr;
  Result<Void, Status> (*invalidate_swapchain)(
      Device self, Swapchain swapchain, SwapchainDesc const &desc) = nullptr;
  Result<Void, Status> (*begin_frame)(Device    self,
                                      Swapchain swapchain)         = nullptr;
  Result<Void, Status> (*submit_frame)(Device    self,
                                       Swapchain swapchain)        = nullptr;
  Result<u64, Status> (*get_timestamp_query_result)(
      Device self, TimeStampQuery query) = nullptr;
  Result<PipelineStatistics, Status> (*get_statistics_query_result)(
      Device self, StatisticsQuery query) = nullptr;
};

struct DeviceImpl
{
  Device                 self      = nullptr;
  DeviceInterface const *interface = nullptr;

  constexpr DeviceInterface const *operator->() const
  {
    return interface;
  }
};

struct InstanceInterface
{
  void (*uninit)(Instance self) = nullptr;
  Result<DeviceImpl, Status> (*create_device)(
      Instance self, AllocatorImpl allocator,
      Span<DeviceType const> preferred_types,
      Span<Surface const> compatible_surfaces, u32 buffering) = nullptr;
  Backend (*get_backend)(Instance self)                       = nullptr;
  void (*uninit_device)(Instance self, Device device)         = nullptr;
  void (*uninit_surface)(Instance self, Surface surface)      = nullptr;
};

struct InstanceImpl
{
  Instance                 self      = nullptr;
  InstanceInterface const *interface = nullptr;

  constexpr InstanceInterface const *operator->() const
  {
    return interface;
  }
};

Result<InstanceImpl, Status> create_vulkan_instance(AllocatorImpl allocator,
                                                    bool enable_validation);

}        // namespace gpu

}        // namespace ash
