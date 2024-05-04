#pragma once
#include "ashura/std/allocator.h"
#include "ashura/std/enum.h"
#include "ashura/std/error.h"
#include "ashura/std/log.h"
#include "ashura/std/mem.h"
#include "ashura/std/option.h"
#include "ashura/std/range.h"
#include "ashura/std/result.h"
#include "ashura/std/source_location.h"
#include "ashura/std/types.h"
#include "ashura/std/vec.h"
#include <cstddef>

namespace ash
{
namespace gfx
{
constexpr u32 REMAINING_MIP_LEVELS           = ~0U;
constexpr u32 REMAINING_ARRAY_LAYERS         = ~0U;
constexpr u64 WHOLE_SIZE                     = ~0ULL;
constexpr u32 MAX_COLOR_ATTACHMENTS          = 8;
constexpr u32 MAX_INPUT_ATTACHMENTS          = 8;
constexpr u32 MAX_VERTEX_ATTRIBUTES          = 16;
constexpr u32 MAX_PUSH_CONSTANT_SIZE         = 128;
constexpr u32 MAX_PIPELINE_DESCRIPTOR_SETS   = 8;
constexpr u32 MAX_DESCRIPTOR_DYNAMIC_BUFFERS = 4;
constexpr u32 MAX_BINDINGS_PER_SET           = 8;
constexpr u32 MAX_DESCRIPTORS_PER_BINDING    = 1024;
constexpr u32 MAX_DESCRIPTORS_PER_SET        = 1024;
constexpr u32 MAX_COMPUTE_GROUP_COUNT_X      = 1024;
constexpr u32 MAX_COMPUTE_GROUP_COUNT_Y      = 1024;
constexpr u32 MAX_COMPUTE_GROUP_COUNT_Z      = 1024;
constexpr u32 MAX_SWAPCHAIN_IMAGES           = 4;

typedef Vec2U                         Offset;
typedef Vec2U                         Extent;
typedef Vec3U                         Offset3D;
typedef Vec3U                         Extent3D;
typedef u64                           FrameId;
typedef struct Buffer_T              *Buffer;
typedef struct BufferView_T          *BufferView;
typedef struct Image_T               *Image;
typedef struct ImageView_T           *ImageView;
typedef struct Sampler_T             *Sampler;
typedef struct Shader_T              *Shader;
typedef struct RenderPass_T          *RenderPass;
typedef struct Framebuffer_T         *Framebuffer;
typedef struct DescriptorSetLayout_T *DescriptorSetLayout;
typedef struct DescriptorSet_T       *DescriptorSet;
typedef struct PipelineCache_T       *PipelineCache;
typedef struct ComputePipeline_T     *ComputePipeline;
typedef struct GraphicsPipeline_T    *GraphicsPipeline;
typedef struct CommandEncoder_T      *CommandEncoder;
typedef struct Surface_T             *Surface;
typedef struct Swapchain_T           *Swapchain;
typedef struct Device_T              *Device;
typedef struct Instance_T            *Instance;

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

enum class Format : u8
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

enum class ColorSpace : u32
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

enum class FormatFeatures : u16
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
  None     = 0x00U,
  Color    = 0x01U,
  Depth    = 0x02U,
  Stencil  = 0x04U,
  MetaData = 0x08U
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

enum class BufferUsage : u16
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

enum class ImageUsage : u8
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

enum class CompositeAlpha : u8
{
  None           = 0x00U,
  Opaque         = 0x01U,
  PreMultiplied  = 0x02U,
  PostMultiplied = 0x04U,
  Inherit        = 0x08U
};

ASH_DEFINE_ENUM_BIT_OPS(CompositeAlpha)

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

/// @extent: can be negative to flip
struct Viewport
{
  Vec2 offset;
  Vec2 extent;
  f32  min_depth = 0;
  f32  max_depth = 0;
};

struct StencilOpState
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
/// @mapping: mapping of the components in the shader. i.e. for R8G8B8_UNORM the
/// non-existent Alpha component is always 0. To set it to 1 we set its
/// component mapping (mapping.a) to ComponentSwizzle::One.
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
  f32                max_anisotropy    = 0;
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

/// @load_op: how to load color or depth component
/// @store_op: how to store color or depth component
/// @stencil_load_op: how to load stencil component
/// @stencil_store_op: how to store stencil component
struct RenderPassAttachment
{
  Format  format           = Format::Undefined;
  LoadOp  load_op          = LoadOp::Load;
  StoreOp store_op         = StoreOp::Store;
  LoadOp  stencil_load_op  = LoadOp::Load;
  StoreOp stencil_store_op = StoreOp::Store;
};

/// render_passes are used for selecting tiling strategy and
/// related optimizations
struct RenderPassDesc
{
  Span<char const>                 label                    = {};
  Span<RenderPassAttachment const> color_attachments        = {};
  Span<RenderPassAttachment const> input_attachments        = {};
  RenderPassAttachment             depth_stencil_attachment = {};
};

struct FramebufferDesc
{
  Span<char const>      label                    = {};
  RenderPass            render_pass              = nullptr;
  Extent                extent                   = {};
  Span<ImageView const> color_attachments        = {};
  ImageView             depth_stencil_attachment = nullptr;
  u32                   layers                   = 0;
};

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

struct DescriptorUpdate
{
  gfx::DescriptorSet        set           = 0;
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
  u32                             push_constant_size     = 0;
  Span<DescriptorSetLayout const> descriptor_set_layouts = {};
  PipelineCache                   cache                  = nullptr;
};

/// Specifies how the binded vertex buffers are iterated and the strides for
/// them unique for each binded buffer.
/// @binding: binding id this structure represents
/// @stride: stride in bytes for each binding advance within the binded buffer
/// @input_rate: advance-rate for this binding. on every vertex or every
/// instance
struct VertexInputBinding
{
  u32       binding    = 0;
  u32       stride     = 0;
  InputRate input_rate = InputRate::Vertex;
};

/// specifies representation/interpretation and shader location mapping of the
/// values in the buffer this is a many to one mapping to the input binding.
/// @binding: which binding this attribute binds to
/// @location: binding's mapped location in the shader
/// @format: data format interpretation
/// @offset: offset of attribute in binding
struct VertexAttribute
{
  u32    binding  = 0;
  u32    location = 0;
  Format format   = Format::Undefined;
  u32    offset   = 0;
};

struct PipelineDepthStencilState
{
  bool           depth_test_enable        = false;
  bool           depth_write_enable       = false;
  CompareOp      depth_compare_op         = CompareOp::Never;
  bool           depth_bounds_test_enable = false;
  bool           stencil_test_enable      = false;
  StencilOpState front_stencil            = {};
  StencilOpState back_stencil             = {};
  f32            min_depth_bounds         = 0;
  f32            max_depth_bounds         = 0;
};

struct PipelineColorBlendAttachmentState
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

struct PipelineColorBlendState
{
  bool                                          logic_op_enable = false;
  LogicOp                                       logic_op       = LogicOp::Clear;
  Span<PipelineColorBlendAttachmentState const> attachments    = {};
  Vec4                                          blend_constant = {};
};

struct PipelineRasterizationState
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

struct GraphicsPipelineDesc
{
  Span<char const>                label                  = {};
  ShaderStageDesc                 vertex_shader          = {};
  ShaderStageDesc                 fragment_shader        = {};
  RenderPass                      render_pass            = nullptr;
  Span<VertexInputBinding const>  vertex_input_bindings  = {};
  Span<VertexAttribute const>     vertex_attributes      = {};
  u32                             push_constant_size     = 0;
  Span<DescriptorSetLayout const> descriptor_set_layouts = {};
  PrimitiveTopology          primitive_topology  = PrimitiveTopology::PointList;
  PipelineRasterizationState rasterization_state = {};
  PipelineDepthStencilState  depth_stencil_state = {};
  PipelineColorBlendState    color_blend_state   = {};
  PipelineCache              cache               = nullptr;
};

struct DescriptorHeapDesc
{
  DescriptorSetLayout layout            = nullptr;
  u32                 num_sets_per_pool = 0;
  AllocatorImpl       allocator         = default_allocator;
};

struct DispatchCommand
{
  u32 x = 0;
  u32 y = 0;
  u32 z = 0;
};

struct DrawIndexedCommand
{
  u32 index_count    = 0;
  u32 instance_count = 0;
  u32 first_index    = 0;
  i32 vertex_offset  = 0;
  u32 first_instance = 0;
};

struct DrawCommand
{
  u32 vertex_count   = 0;
  u32 instance_count = 0;
  u32 first_vertex   = 0;
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

/// @generation: increases everytime the swapchain for the surface is recreated
/// or re-configured
/// @images: swapchain images, calling ref or unref on them will cause a panic
/// as they are only meant to exist for the lifetime of the frame.
/// avoid storing pointers to its data members.
struct SwapchainState
{
  Extent            extent        = {};
  SurfaceFormat     format        = {};
  Span<Image const> images        = {};
  Option<u32>       current_image = None;
};

struct DeviceLimits
{
  u32         max_image_dimension1D                      = 0;
  u32         max_image_dimension2D                      = 0;
  u32         max_image_dimension3D                      = 0;
  u32         max_image_dimension_cube                   = 0;
  u32         max_image_array_layers                     = 0;
  u32         max_texel_buffer_elements                  = 0;
  u32         max_uniform_buffer_range                   = 0;
  u32         max_storage_buffer_range                   = 0;
  u32         max_push_constants_size                    = 0;
  u32         max_bound_descriptor_sets                  = 0;
  u32         max_per_stage_descriptor_samplers          = 0;
  u32         max_per_stage_descriptor_uniform_buffers   = 0;
  u32         max_per_stage_descriptor_storage_buffers   = 0;
  u32         max_per_stage_descriptor_sampled_images    = 0;
  u32         max_per_stage_descriptor_storage_images    = 0;
  u32         max_per_stage_descriptor_input_attachments = 0;
  u32         max_per_stage_resources                    = 0;
  u32         max_descriptor_set_samplers                = 0;
  u32         max_descriptor_set_uniform_buffers         = 0;
  u32         max_descriptor_set_uniform_buffers_dynamic = 0;
  u32         max_descriptor_set_storage_buffers         = 0;
  u32         max_descriptor_set_storage_buffers_dynamic = 0;
  u32         max_descriptor_set_sampled_images          = 0;
  u32         max_descriptor_set_storage_images          = 0;
  u32         max_descriptor_set_input_attachments       = 0;
  u32         max_vertex_input_attributes                = 0;
  u32         max_vertex_input_bindings                  = 0;
  u32         max_vertex_input_attribute_offset          = 0;
  u32         max_vertex_input_binding_stride            = 0;
  u32         max_vertex_output_components               = 0;
  u32         max_fragment_input_components              = 0;
  u32         max_fragment_output_attachments            = 0;
  u32         max_fragment_dual_src_attachments          = 0;
  u32         max_fragment_combined_output_resources     = 0;
  u32         max_compute_shared_memory_size             = 0;
  u32         max_compute_work_group_count[3]            = {};
  u32         max_compute_work_group_invocations         = 0;
  u32         max_compute_work_group_size[3]             = {};
  u32         max_draw_indexed_index_value               = 0;
  u32         max_draw_indirect_count                    = 0;
  f32         max_sampler_lod_bias                       = 0;
  f32         max_sampler_anisotropy                     = 0;
  u32         max_viewports                              = 0;
  u32         max_viewport_dimensions[2]                 = {};
  f32         viewport_bounds_range[2]                   = {};
  u32         viewport_sub_pixel_bits                    = 0;
  usize       min_memory_map_alignment                   = 0;
  u64         min_texel_buffer_offset_alignment          = 0;
  u64         min_uniform_buffer_offset_alignment        = 0;
  u64         min_storage_buffer_offset_alignment        = 0;
  u32         max_framebuffer_width                      = 0;
  u32         max_framebuffer_height                     = 0;
  u32         max_framebuffer_layers                     = 0;
  SampleCount framebuffer_color_sample_counts            = SampleCount::None;
  SampleCount framebuffer_depth_sample_counts            = SampleCount::None;
  SampleCount framebuffer_stencil_sample_counts          = SampleCount::None;
  SampleCount framebuffer_no_attachments_sample_counts   = SampleCount::None;
  u32         max_color_attachments                      = 0;
  SampleCount sampled_image_color_sample_counts          = SampleCount::None;
  SampleCount sampled_image_integer_sample_counts        = SampleCount::None;
  SampleCount sampled_image_depth_sample_counts          = SampleCount::None;
  SampleCount sampled_image_stencil_sample_counts        = SampleCount::None;
  SampleCount storage_image_sample_counts                = SampleCount::None;
  u32         max_clip_distances                         = 0;
  u32         max_cull_distances                         = 0;
  u32         max_combined_clip_and_cull_distances       = 0;
};

struct DeviceProperties
{
  u32              api_version        = 0;
  u32              driver_version     = 0;
  u32              vendor_id          = 0;
  u32              device_id          = 0;
  Span<char const> api_name           = {};
  Span<char const> device_name        = {};
  DeviceType       type               = DeviceType::Other;
  bool             has_unified_memory = false;
  DeviceLimits     limits             = {};
};

// TODO(lamarrrr): maintaining multiple heaps is impractical and could lead to
// fragmentation we need to use only one global heap. the descriptor set layout
// can be used to point to a table containing binding metadata or whatnot.
//
// HOW TO MANAGE ALLOCATION, DEALLOCATION, USAGE TRACKING, RELEASE TRACKING,
// STATISTICS
//
// create memory for each descriptor set that will be updated and used for
// tracking entries, size and spec sourced from layout.
// set will contain the size of each binding descriptor.
//
//
// large descriptor pools
// max descriptorcount per binding=> 1024
// max bindings per set => 8
//
//
//
// for allocation, go through each pool, whichever has enough to contain the
// request, select it and allocate from it, update the stats.
//
//
// for deallocation find the pool it belongs to, deallocate from it, update its
// stats.
//
// multiple descriptor pools of size 4096
// allocation strategy? TO TRACK AND MANAGE FRAGMENTATION
// HOW TO HANDLE DYNAMICALLY SIZED DESCRIPTORS
// USAGE/RELEASE TRACKING CAN BE DONE BY THE USER. UPON DEALLOCATION, UPDATE
// STATS
//
//
// TRACK each RENDER PASS/COMPUTE PASS, count the pass id and use it to generate
// the name, i.e. Pass:0 - Compute
// How to retrieve?
//
//
//

/// to execute tasks at end of frame. use the tail frame index.
struct CommandEncoderInterface
{
  // TODO(lamarrrr): timestamp query contexts, pER-command-buffer reset at
  // beginning of frame, collected for
  // u32 (*begin_statistics)(CommandEncoder self);
  // void (*end_statistics)(CommandEncoder self, u32 query_idx);
  // Stats | Pending get_statistics(u32 query_idx, u32 frames_ago);
  // where frames_ago < max_frames_in_flight
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
  void (*begin_render_pass)(
      CommandEncoder self, Framebuffer framebuffer, RenderPass render_pass,
      Offset render_offset, Extent render_extent,
      Span<Color const>        color_attachments_clear_values,
      Span<DepthStencil const> depth_stencil_attachment_clear_value)  = nullptr;
  void (*end_render_pass)(CommandEncoder self)                        = nullptr;
  void (*bind_compute_pipeline)(CommandEncoder  self,
                                ComputePipeline pipeline)             = nullptr;
  void (*bind_graphics_pipeline)(CommandEncoder   self,
                                 GraphicsPipeline pipeline)           = nullptr;
  void (*bind_descriptor_sets)(CommandEncoder            self,
                               Span<DescriptorSet const> descriptor_sets,
                               Span<u32 const> dynamic_offsets)       = nullptr;
  void (*push_constants)(CommandEncoder self,
                         Span<u8 const> push_constants_data)          = nullptr;
  void (*dispatch)(CommandEncoder self, u32 group_count_x, u32 group_count_y,
                   u32 group_count_z)                                 = nullptr;
  void (*dispatch_indirect)(CommandEncoder self, Buffer buffer,
                            u64 offset)                               = nullptr;
  void (*set_viewport)(CommandEncoder self, Viewport const &viewport) = nullptr;
  void (*set_scissor)(CommandEncoder self, Offset scissor_offset,
                      Extent scissor_extent)                          = nullptr;
  void (*set_blend_constants)(CommandEncoder self,
                              Vec4           blend_constant)                    = nullptr;
  void (*set_stencil_compare_mask)(CommandEncoder self, StencilFaces faces,
                                   u32 mask)                          = nullptr;
  void (*set_stencil_reference)(CommandEncoder self, StencilFaces faces,
                                u32 reference)                        = nullptr;
  void (*set_stencil_write_mask)(CommandEncoder self, StencilFaces faces,
                                 u32 mask)                            = nullptr;
  void (*bind_vertex_buffers)(CommandEncoder     self,
                              Span<Buffer const> vertex_buffers,
                              Span<u64 const>    offsets)                = nullptr;
  void (*bind_index_buffer)(CommandEncoder self, Buffer index_buffer,
                            u64 offset, IndexType index_type)         = nullptr;
  void (*draw)(CommandEncoder self, u32 vertex_count, u32 instance_count,
               u32 first_vertex_id, u32 first_instance_id)            = nullptr;
  void (*draw_indexed)(CommandEncoder self, u32 first_index, u32 num_indices,
                       i32 vertex_offset, u32 first_instance_id,
                       u32 num_instances)                             = nullptr;
  void (*draw_indirect)(CommandEncoder self, Buffer buffer, u64 offset,
                        u32 draw_count, u32 stride)                   = nullptr;
  void (*draw_indexed_indirect)(CommandEncoder self, Buffer buffer, u64 offset,
                                u32 draw_count, u32 stride)           = nullptr;
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
  u32                            max_frames_in_flight = 0;
  FrameId                        tail                 = 0;
  FrameId                        current              = 0;
  Span<CommandEncoderImpl const> encoders             = {};
  u32                            ring_index           = 0;
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
  Result<RenderPass, Status> (*create_render_pass)(
      Device self, RenderPassDesc const &desc) = nullptr;
  Result<Framebuffer, Status> (*create_framebuffer)(
      Device self, FramebufferDesc const &desc) = nullptr;
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
  FrameContext (*get_frame_context)(Device self)     = nullptr;
  Result<Swapchain, Status> (*create_swapchain)(
      Device self, Surface surface, SwapchainDesc const &desc)      = nullptr;
  void (*destroy_buffer)(Device self, Buffer buffer)                = nullptr;
  void (*destroy_buffer_view)(Device self, BufferView buffer_view)  = nullptr;
  void (*destroy_image)(Device self, Image image)                   = nullptr;
  void (*destroy_image_view)(Device self, ImageView image_view)     = nullptr;
  void (*destroy_sampler)(Device self, Sampler sampler)             = nullptr;
  void (*destroy_shader)(Device self, Shader shader)                = nullptr;
  void (*destroy_render_pass)(Device self, RenderPass render_pass)  = nullptr;
  void (*destroy_framebuffer)(Device self, Framebuffer framebuffer) = nullptr;
  void (*destroy_descriptor_set_layout)(Device              self,
                                        DescriptorSetLayout layout) = nullptr;
  void (*destroy_descriptor_set)(Device self, DescriptorSet set)    = nullptr;
  void (*destroy_pipeline_cache)(Device self, PipelineCache cache)  = nullptr;
  void (*destroy_compute_pipeline)(Device          self,
                                   ComputePipeline pipeline)        = nullptr;
  void (*destroy_graphics_pipeline)(Device           self,
                                    GraphicsPipeline pipeline)      = nullptr;
  void (*destroy_swapchain)(Device self, Swapchain swapchain)       = nullptr;
  Result<void *, Status> (*get_buffer_memory_map)(Device self,
                                                  Buffer buffer)    = nullptr;
  Result<Void, Status> (*invalidate_buffer_memory_map)(
      Device self, Buffer buffer, MemoryRange range)                 = nullptr;
  Result<Void, Status> (*flush_buffer_memory_map)(Device self, Buffer buffer,
                                                  MemoryRange range) = nullptr;
  Result<usize, Status> (*get_pipeline_cache_size)(
      Device self, PipelineCache cache)                          = nullptr;
  Result<usize, Status> (*get_pipeline_cache_data)(Device        self,
                                                   PipelineCache cache,
                                                   Span<u8>      out) = nullptr;
  Result<Void, Status> (*merge_pipeline_cache)(
      Device self, PipelineCache dst, Span<PipelineCache const> srcs) = nullptr;
  void (*update_descriptor_set)(Device                  self,
                                DescriptorUpdate const &update)       = nullptr;
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
  void (*destroy)(Instance self) = nullptr;
  Result<DeviceImpl, Status> (*create_device)(
      Instance self, Span<DeviceType const> preferred_types,
      Span<Surface const> compatible_surfaces,
      AllocatorImpl       allocator)                            = nullptr;
  Backend (*get_backend)(Instance self)                   = nullptr;
  void (*destroy_device)(Instance self, Device device)    = nullptr;
  void (*destroy_surface)(Instance self, Surface surface) = nullptr;
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

Result<InstanceImpl, Status>
    create_vulkan_instance(AllocatorImpl allocator, Logger *logger,
                           bool enable_validation_layer);

}        // namespace gfx

/// @name: parameter name
/// @type: only valid if is not uniform
/// @count: element count of the binding
/// @current_count: current element count of the binding, only used if
/// `is_variable_length` is true
/// @member_offset: offset of this member in the whole struct
/// @is_variable_length: if binding is a variable length binding
struct ShaderBindingMetaData
{
  Span<char const>    name;
  gfx::DescriptorType type               = gfx::DescriptorType::Sampler;
  u32                 member_offset      = 0;
  u32                 count              = 0;
  bool                is_variable_length = false;
  u32                 current_count      = 0;
};

// change binding length for example
// CHECK LIMITS!!
// count variable length using recursion in macro
// TODO(lamarrr): check descriptor set layout compatibility
// TODO(lamarrr): encode dynamic? count should represent maximum count.
#define BEGIN_SHADER_PARAMETER(Name)             \
  struct Name                                    \
  {                                              \
    typedef Name _METAThisType;                  \
                                                 \
    struct _METAMemberBegin                      \
    {                                            \
      static constexpr u32 _METAMemberIndex = 0; \
    };                                           \
                                                 \
    typedef _METAMemberBegin
// TODO(lamarrr): define push function above

#define SHADER_BINDING(BindingName, StructType, BindingType, Count,              \
                       IsVariableLength, InitialCount)                           \
  _METAMember_##BindingName;                                                     \
                                                                                 \
  static constexpr void _METApush(_METAMember_##BindingName,                     \
                                  ::ash::ShaderBindingMetaData *meta,            \
                                  u32                           _METAThisOffset) \
  {                                                                              \
    *meta = ::ash::ShaderBindingMetaData{                                        \
        .name = ::ash::to_span(#BindingName),                                    \
        .type = ::ash::gfx::DescriptorType::BindingType,                         \
        .member_offset =                                                         \
            (u32) (_METAThisOffset + offsetof(_METAThisType, BindingName)),      \
        .count              = (u32) (Count),                                     \
        .is_variable_length = (bool) (IsVariableLength),                         \
        .current_count      = (u32) (InitialCount)};                                  \
    _METApush(_METAMemberAfter_##BindingName{}, meta + 1, _METAThisOffset);      \
  }                                                                              \
                                                                                 \
  ::ash::gfx::StructType BindingName[(u32) (Count)];                             \
  u32                    BindingName##_count = (u32) (InitialCount);             \
                                                                                 \
  struct _METAMemberAfter_##BindingName                                          \
  {                                                                              \
    static constexpr u32 _METAMemberIndex =                                      \
        _METAMember_##BindingName::_METAMemberIndex + 1;                         \
  };                                                                             \
                                                                                 \
  typedef _METAMemberAfter_##BindingName

#define SAMPLER(BindingName, Count) \
  SHADER_BINDING(BindingName, ImageBinding, Sampler, Count, false, Count)
#define VAR_SAMPLER(BindingName, Count, InitialCount) \
  SHADER_BINDING(BindingName, ImageBinding, Sampler, Count, true, InitialCount)

#define COMBINED_IMAGE_SAMPLER(BindingName, Count)                       \
  SHADER_BINDING(BindingName, ImageBinding, CombinedImageSampler, Count, \
                 false, Count)
#define VAR_COMBINED_IMAGE_SAMPLER(BindingName, Count, InitialCount)           \
  SHADER_BINDING(BindingName, ImageBinding, CombinedImageSampler, Count, true, \
                 InitialCount)

#define SAMPLED_IMAGE(BindingName, Count) \
  SHADER_BINDING(BindingName, ImageBinding, SampledImage, Count, false, Count)
#define VAR_SAMPLED_IMAGE(BindingName, Count, InitialCount)            \
  SHADER_BINDING(BindingName, ImageBinding, SampledImage, Count, true, \
                 InitialCount)

#define STORAGE_IMAGE(BindingName, Count) \
  SHADER_BINDING(BindingName, ImageBinding, StorageImage, Count, false, Count)
#define VAR_STORAGE_IMAGE(BindingName, Count, InitialCount)            \
  SHADER_BINDING(BindingName, ImageBinding, StorageImage, Count, true, \
                 InitialCount)

#define UNIFORM_TEXEL_BUFFER(BindingName, Count) \
  SHADER_BINDING(BindingName, BufferView, BufferView, Count, false, Count)
#define VAR_UNIFORM_TEXEL_BUFFER(BindingName, Count, InitialCount) \
  SHADER_BINDING(BindingName, BufferView, BufferView, Count, true, InitialCount)

#define STORAGE_TEXEL_BUFFER(BindingName, Count) \
  SHADER_BINDING(BindingName, BufferView, BufferView, Count, false, Count)
#define VAR_STORAGE_TEXEL_BUFFER(BindingName, Count, InitialCount) \
  SHADER_BINDING(BindingName, BufferView, BufferView, Count, true, InitialCount)

#define UNIFORM_BUFFER(BindingName, Count) \
  SHADER_BINDING(BindingName, BufferBinding, UniformBuffer, Count, false, Count)
#define VAR_UNIFORM_BUFFER(BindingName, Count, InitialCount)             \
  SHADER_BINDING(BindingName, BufferBinding, UniformBuffer, Count, true, \
                 InitialCount)

#define STORAGE_BUFFER(BindingName, Count) \
  SHADER_BINDING(BindingName, BufferBinding, StorageBuffer, Count, false, Count)
#define VAR_STORAGE_BUFFER(BindingName, Count, InitialCount)             \
  SHADER_BINDING(BindingName, BufferBinding, StorageBuffer, Count, true, \
                 InitialCount)

#define DYNAMIC_UNIFORM_BUFFER(BindingName, Count)                        \
  SHADER_BINDING(BindingName, BufferBinding, DynamicUniformBuffer, Count, \
                 false, Count)
#define VAR_DYNAMIC_UNIFORM_BUFFER(BindingName, Count, InitialCount)      \
  SHADER_BINDING(BindingName, BufferBinding, DynamicUniformBuffer, Count, \
                 true, InitialCount)

#define DYNAMIC_STORAGE_BUFFER(BindingName, Count)                        \
  SHADER_BINDING(BindingName, BufferBinding, DynamicStorageBuffer, Count, \
                 false, Count)
#define VAR_DYNAMIC_STORAGE_BUFFER(BindingName, Count, InitialCount)      \
  SHADER_BINDING(BindingName, BufferBinding, DynamicStorageBuffer, Count, \
                 true, InitialCount)

#define INPUT_ATTACHMENT(BindingName, Count)                               \
  SHADER_BINDING(BindingName, ImageBinding, InputAttachment, Count, false, \
                 Count)
#define VAR_INPUT_ATTACHMENT(BindingName, Count, InitialCount)            \
  SHADER_BINDING(BindingName, ImageBinding, InputAttachment, Count, true, \
                 InitialCount)

#define SHADER_PARAMETER_INLINE(BindingName, ParameterType)                      \
  _METAMember_##BindingName;                                                     \
                                                                                 \
  static constexpr void _METApush(_METAMember_##BindingName,                     \
                                  ::ash::ShaderBindingMetaData *meta,            \
                                  u32                           _METAThisOffset) \
  {                                                                              \
    ParameterType::_METApush(                                                    \
        ParameterType::_METAMemberBegin{}, meta,                                 \
        (u32) (_METAThisOffset + offsetof(_METAThisType, BindingName)));         \
    _METApush(_METAMemberAfter_##BindingName{},                                  \
              meta + ParameterType::NUM_BINDINGS, _METAThisOffset);              \
  }                                                                              \
                                                                                 \
  ParameterType BindingName;                                                     \
                                                                                 \
  struct _METAMemberAfter_##BindingName                                          \
  {                                                                              \
    static constexpr u32 _METAMemberIndex =                                      \
        _METAMember_##BindingName::_METAMemberIndex +                            \
        ParameterType::NUM_BINDINGS;                                             \
  };                                                                             \
                                                                                 \
  typedef _METAMemberAfter_##BindingName

#define END_SHADER_PARAMETER(Name)                                             \
  _METAMemberEnd;                                                              \
                                                                               \
  static constexpr void _METApush(_METAMemberEnd,                              \
                                  ::ash::ShaderBindingMetaData *, u32)         \
  {                                                                            \
  }                                                                            \
                                                                               \
  static constexpr char const NAME[]       = #Name;                            \
  static constexpr u32        NUM_BINDINGS = _METAMemberEnd::_METAMemberIndex; \
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
    for (u32 i = 0; i < NUM_BINDINGS; i++)                                     \
    {                                                                          \
      descs[i] = gfx::DescriptorBindingDesc{                                   \
          .type               = bindings[i].type,                              \
          .count              = bindings[i].count,                             \
          .is_variable_length = bindings[i].is_variable_length};               \
    }                                                                          \
    return descs;                                                              \
  }                                                                            \
  }                                                                            \
  ;

BEGIN_SHADER_PARAMETER(UniformShaderParameter)
DYNAMIC_UNIFORM_BUFFER(buffer, 1)
END_SHADER_PARAMETER(UniformShaderParameter)

template <typename Param>
gfx::DescriptorSetLayout
    create_shader_parameter_layout(gfx::DeviceImpl const &device)
{
  constexpr Array descs = Param::GET_BINDINGS_DESC();
  return device
      ->create_descriptor_set_layout(
          device.self,
          gfx::DescriptorSetLayoutDesc{.label    = to_span(Param::NAME),
                                       .bindings = to_span(descs)})
      .unwrap();
}

template <typename Param>
gfx::DescriptorSet create_shader_parameter(gfx::DeviceImpl const   &device,
                                           gfx::DescriptorSetLayout layout)
{
  constexpr Array descs = Param::GET_BINDINGS_DESC();

  return device
      ->create_descriptor_set(
          device.self, layout,
          descs[size(descs) - 1].is_variable_length ?
              Span<u32 const>{descs[size(descs) - 1].current_count} :
              Span<u32 const>{})
      .unwrap();
}

template <typename Param>
void update_shader_parameter(gfx::DeviceImpl const &device,
                             gfx::DescriptorSet set, Param const &param)
{
  Span metadata = to_span(Param::BINDINGS);

  for (u32 i = 0; i < metadata.size(); i++)
  {
    gfx::DescriptorUpdate        update{.set = set, .binding = i, .element = 0};
    ShaderBindingMetaData const &member = metadata[i];
    switch (member.type)
    {
      case gfx::DescriptorType::CombinedImageSampler:
      case gfx::DescriptorType::InputAttachment:
      case gfx::DescriptorType::SampledImage:
      case gfx::DescriptorType::Sampler:
      case gfx::DescriptorType::StorageImage:
        update.images =
            Span{(gfx::ImageBinding const *) (((u8 const *) &param) +
                                              member.member_offset),
                 member.count};
        break;

      case gfx::DescriptorType::DynamicStorageBuffer:
      case gfx::DescriptorType::DynamicUniformBuffer:
      case gfx::DescriptorType::StorageBuffer:
      case gfx::DescriptorType::UniformBuffer:
        update.buffers =
            Span{(gfx::BufferBinding const *) (((u8 const *) &param) +
                                               member.member_offset),
                 member.count};
        break;
      case gfx::DescriptorType::StorageTexelBuffer:
      case gfx::DescriptorType::UniformTexelBuffer:
      {
        update.texel_buffers =
            Span{(gfx::BufferView const *) (((u8 const *) &param) +
                                            member.member_offset),
                 member.count};
      }
      break;
      default:
        break;
    }

    device->update_descriptor_set(device.self, update);
  }
}

struct FreeTable
{
};

//
// needed for high-freq data.
// would separating set allocation from ssbo and ubo make sense?
//
// TODO(lamarrr): perhaps we should just make it easy to persist. i.e. if we
// repeatedly call a rrect pass with same parameters but different draw offsets
// and indices. we wouldn't need to frequently update descriptor sets or what
// not. no caching as well.
//
// StorageBufferSpan(buffer_size, max_suballocations = U32_MAX)::
// .offset, .num_allocations, min_alignment
// - advance(u64) -> offset
// - reset()
//
//
// check UBO size limits!
// check UBO range limits!
// UniformBufferSpan (buffer_size, max_allocations = U32_MAX):: -
//  .offset, .num_allocations, min_alignment
// - push() -> (set, buffer) | error on resize needed
// - reset()
//
//
// - num set allocations -> max_num_uniforms
// - uniform buffer size -> buffer_size
//
// - how will offsets work for uboheap?
//
// we can thus natively support pushing storage/uniform buffers (deferred)
// commit buffers/readback
//
//
// - review set layot compat checks
//
// immediate - non/non-persistent non-allocating usage
// TODO(lamarrr): rename to buffer and allow usage with SSBO
// should contain both SSBO and UBO sets, take usage type and return appropriate
// descriptor set for it, and use that for allocating descriptors if needed.
// ALLOW both usages and align to the greater alignment.
//
//
// TODO(lamarrr): we also need alignment for read/write across multiple
// allocations.
//
//
// TODO(lamarrr): consider using VMA? search vma reserved memory, need to track
// last used frame of each section. if not used in a while, can free.
//
//
// TODO(lamarrr): how to manage texture heaps for dispatch
//
// THE assumption is that usage would be similar across a buffering
//
// WE NEED STORAGE BUFFERS TO BE ABLE TO ACCESS A LARGE RANGE FOR HUGE
// DISPATCHES, SIZE CLASSES WILL NOT DO.
//
// USE HASHMAPS for descriptor allocation and lookup? key => [BUFFER INDEX +
// ACCESS_RANGE + STORAGE_BUFFER | UNIFORM_BUFFER]
//
// KEEP FREE DESCRIPTOR SETS LIST? to enable stealing - inefficient
//
//
// VK_WHOLE_RANGE sizes can thus be used with any descriptor set provided offset
// is 0
//
//
// BUFFER TYPES: per-frame buffers, persistent buffers
//
//
// BEST SOLUTION:???XXXX
// ALLOW THE PASS to rotate across uniforms, pass might be called multiple times
// per frame and we'd need to allocate anyway
//
//
// allocate descriptor set with null buffer bindings, for each allocation.
// collect uniform data into temporary CPU buffer, at end of frame, upload to
// GPU, then create buffer with proper alignment. along with proper read/write.
//
// cache descriptor sets with ranges => any range can be used as long as it is
//  larger than the max range for that size and, it is larger or equal to the
//  size of the intended access range. and its access range is within the bounds
//  of the buffer.
//
// if buffer is recreated by recreation criteria:
//  update all descriptor sets to point to new buffer and the correct offset +
//  size. update descriptor cache.
//
//
//
//
// since we are performing batch allocations
//
//
// defer destruction of the buffer to completion of frame. buffer must be
// destroyed and memory released before beginning of frame.
//
//
// when pushing, it must be pushed along with all parameters atomically as alloc
// may fail.
//
//
// struct CommandBlock{
//   u32 type = 0;
//   u32 num_parameters = 0;
//   u32 parameters_size;
// };
//
// struct ParameterHeader{
// u32 size;
// };
//
// ...// data
//
// u64 buffer_data[];
// Vec<[u64, u64]> buffers;
// T[] consume(u32 count);
// u8[] consume_buffer();
//
//
//
//
//
// EMULATE COMMAND BUFFER!
//
//
//
// SHOULD BINDING MODEL CHANGE? HASHMAPS? OF SET LAYOUTS?
//
//
//
// collect commands
// - should binding updates be performed in command buffer?
//
// - collect accessed buffers/images in each pass
// - collect bindings and restructure in a way that will make it easy to view
// accessed buffers and whatnot
//
//
// - after collecting all commands, allocate appropriate buffers
// - create or destroy buffer. sync-safe.
// - copy data to buffer and ensure alignment
// - allocate appropriate descriptor sets and re-use those in cache and update
// - update descriptor sets to point to buffers
//
//
//
// - insert per-pass barriers, perhaps w
//
//
//
// # PENDING
// - How to re-use storage buffers in another stage, i.e. compute -> graphics ->
// compute -> graphics. maybe return command encoder id
//
//
inline constexpr u8  NUM_UNIFORM_SIZE_CLASSES   = 6;
inline constexpr u32 DEFAULT_UNIFORM_BATCH_SIZE = 4096;
static constexpr Array<u32, NUM_UNIFORM_SIZE_CLASSES>
    DEFAULT_UNIFORM_SIZE_CLASSES{64, 128, 256, 512, 1024, 4096};

struct UniformHeapBatch
{
  gfx::Buffer                                         buffer      = nullptr;
  Array<gfx::DescriptorSet, NUM_UNIFORM_SIZE_CLASSES> descriptors = {};
};

struct Uniform
{
  gfx::DescriptorSet set           = {};
  gfx::Buffer        buffer        = nullptr;
  u32                buffer_offset = 0;
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
  Array<u32, NUM_UNIFORM_SIZE_CLASSES> size_classes_ =
      DEFAULT_UNIFORM_SIZE_CLASSES;
  u32                      batch_buffer_size_                   = 0;
  u32                      min_uniform_buffer_offset_alignment_ = 0;
  u32                      batch_                               = 0;
  u32                      batch_buffer_offset_                 = 0;
  Vec<UniformHeapBatch>    batches_                             = {};
  gfx::DescriptorSetLayout descriptor_set_layout_               = {};
  gfx::DescriptorHeapImpl  descriptor_heap_                     = {};
  gfx::DeviceImpl          device_;

  void init(gfx::DeviceImpl device,
            u32             batch_buffer_size    = DEFAULT_UNIFORM_BATCH_SIZE,
            u32             descriptor_pool_size = 16,
            Array<u32, NUM_UNIFORM_SIZE_CLASSES> const &size_classes =
                DEFAULT_UNIFORM_SIZE_CLASSES)
  {
    gfx::DeviceProperties properties =
        device->get_device_properties(device.self);
    min_uniform_buffer_offset_alignment_ =
        properties.limits.min_uniform_buffer_offset_alignment;

    CHECK(batch_buffer_size >= size_classes[NUM_UNIFORM_SIZE_CLASSES - 1]);
    CHECK(batch_buffer_size >= min_uniform_buffer_offset_alignment_);
    for (u8 i = 0; i < NUM_UNIFORM_SIZE_CLASSES - 1; i++)
    {
      CHECK(size_classes[i] < size_classes[i + 1]);
    }

    size_classes_                 = size_classes;
    batch_buffer_size_            = batch_buffer_size;
    device_                       = device;
    batch_                        = 0;
    batch_buffer_offset_          = 0;
    constexpr Array bindings_desc = UniformShaderParameter::GET_BINDINGS_DESC();
    descriptor_set_layout_        = device
                                 ->create_descriptor_set_layout(
                                     device.self,
                                     gfx::DescriptorSetLayoutDesc{
                                         .label    = "Uniform Buffer"_span,
                                         .bindings = to_span(bindings_desc)})
                                 .unwrap();

    descriptor_heap_ = device
                           ->create_descriptor_heap(
                               device.self,
                               gfx::DescriptorHeapDesc{
                                   .layout            = descriptor_set_layout_,
                                   .num_sets_per_pool = descriptor_pool_size,
                                   .allocator         = default_allocator})
                           .unwrap();
  }

  void uninit()
  {
    for (UniformHeapBatch const &batch : batches_)
    {
      device_->destroy_buffer(device_.self, batch.buffer);
    }
    device_->destroy_descriptor_set_layout(device_.self,
                                           descriptor_set_layout_);
    device_->destroy_descriptor_heap(device_.self, descriptor_heap_);
    batches_.reset();
  }

  template <typename UniformType>
  Uniform push(UniformType const &uniform)
  {
    return push_range(Span{&uniform, 1});
  }

  template <typename UniformType>
  Uniform push_range(Span<UniformType const> uniform)
  {
    return push_bytes(uniform.as_u8(), alignof(UniformType));
  }

  Uniform push_bytes(Span<u8 const> uniform, u32 alignment)
  {
    CHECK(alignment <= batch_buffer_size_);
    CHECK(uniform.size_bytes() <= batch_buffer_size_);
    CHECK(uniform.size_bytes() <= size_classes_[NUM_UNIFORM_SIZE_CLASSES - 1]);

    u8 size_class = 0;
    for (; size_class < NUM_UNIFORM_SIZE_CLASSES; size_class++)
    {
      if (size_classes_[size_class] >= uniform.size_bytes())
      {
        break;
      }
    }

    u32 const classed_size = size_classes_[size_class];
    u32       buffer_offset =
        mem::align_offset(max(alignment, min_uniform_buffer_offset_alignment_),
                          batch_buffer_offset_);
    u32 batch_index = batch_;
    if ((buffer_offset + classed_size) > batch_buffer_size_)
    {
      batch_index++;
      buffer_offset = 0;
    }

    if (batch_index >= batches_.size())
    {
      gfx::Buffer buffer =
          device_
              ->create_buffer(
                  device_.self,
                  gfx::BufferDesc{.label = "UniformHeap batch buffer"_span,
                                  .size  = batch_buffer_size_,
                                  .host_mapped = true,
                                  .usage = gfx::BufferUsage::UniformBuffer |
                                           gfx::BufferUsage::TransferDst |
                                           gfx::BufferUsage::TransferSrc})
              .unwrap();

      UniformHeapBatch batch{.buffer = buffer, .descriptors = {}};

      for (u32 i = 0; i < NUM_UNIFORM_SIZE_CLASSES; i++)
      {
        u32 set = descriptor_heap_->allocate(descriptor_heap_.self).unwrap();
        batch.descriptors[i] =
            gfx::DescriptorSet{.heap = descriptor_heap_.self, .index = set};
        descriptor_heap_->update(
            descriptor_heap_.self,
            gfx::DescriptorUpdate{.set     = set,
                                  .binding = 0,
                                  .element = 0,
                                  .buffers = to_span<gfx::BufferBinding>(
                                      {{.buffer = buffer,
                                        .offset = 0,
                                        .size   = size_classes_[i]}})});
      }

      CHECK(batches_.push(batch));
    }

    UniformHeapBatch const &batch = batches_[batch_index];
    u8 *map = (u8 *) device_->get_buffer_memory_map(device_.self, batch.buffer)
                  .unwrap();
    mem::copy(uniform, map + buffer_offset);
    device_
        ->flush_buffer_memory_map(device_.self, batch.buffer,
                                  gfx::MemoryRange{0, gfx::WHOLE_SIZE})
        .unwrap();

    batch_               = batch_index;
    batch_buffer_offset_ = buffer_offset + uniform.size_bytes();

    return Uniform{.set           = batch.descriptors[size_class],
                   .buffer        = batch.buffer,
                   .buffer_offset = buffer_offset};
  }

  void reset()
  {
    batch_               = 0;
    batch_buffer_offset_ = 0;
  }
};

// TODO(lamarrr): we need SSBO heap as well, maybe use pages instead.
//
// we also need to be able to sparsely update descriptor set bindings, i.e. when
// one or few out of thousands of the array elements of our bindings change.
//
// buffers
// images
// storage_texels
//

}        // namespace ash
