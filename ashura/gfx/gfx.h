#pragma once
#include "ashura/std/allocator.h"
#include "ashura/std/enum.h"
#include "ashura/std/log.h"
#include "ashura/std/option.h"
#include "ashura/std/result.h"
#include "ashura/std/types.h"

namespace ash
{
namespace gfx
{
constexpr u32 REMAINING_MIP_LEVELS         = ~0U;
constexpr u32 REMAINING_ARRAY_LAYERS       = ~0U;
constexpr u64 WHOLE_SIZE                   = ~0ULL;
constexpr u32 MAX_COLOR_ATTACHMENTS        = 8;
constexpr u32 MAX_INPUT_ATTACHMENTS        = 8;
constexpr u32 MAX_VERTEX_ATTRIBUTES        = 16;
constexpr u32 MAX_PUSH_CONSTANT_SIZE       = 128;
constexpr u32 MAX_MEMORY_HEAP_PROPERTIES   = 32;
constexpr u32 MAX_MEMORY_HEAPS             = 16;
constexpr u32 MAX_PIPELINE_DESCRIPTOR_SETS = 8;
constexpr u32 MAX_COMPUTE_GROUP_COUNT_X    = 1024;
constexpr u32 MAX_COMPUTE_GROUP_COUNT_Y    = 1024;
constexpr u32 MAX_COMPUTE_GROUP_COUNT_Z    = 1024;
constexpr u32 MAX_SWAPCHAIN_IMAGES         = 8;

typedef Vec2U                          Offset;
typedef Vec2U                          Extent;
typedef Vec3U                          Offset3D;
typedef Vec3U                          Extent3D;
typedef u64                            FrameId;
typedef struct Buffer_T               *Buffer;
typedef struct BufferView_T           *BufferView;
typedef struct Image_T                *Image;
typedef struct ImageView_T            *ImageView;
typedef struct Sampler_T              *Sampler;
typedef struct CombinedImageSampler    CombinedImageSampler;
typedef CombinedImageSampler           CombinedImageSamplerBinding;
typedef struct Shader_T               *Shader;
typedef struct RenderPass_T           *RenderPass;
typedef struct Framebuffer_T          *Framebuffer;
typedef struct DescriptorSetLayout_T  *DescriptorSetLayout;
typedef struct DescriptorHeap_T       *DescriptorHeap;
typedef struct DescriptorSet           DescriptorSet;
typedef struct PipelineCache_T        *PipelineCache;
typedef struct ComputePipeline_T      *ComputePipeline;
typedef struct GraphicsPipeline_T     *GraphicsPipeline;
typedef struct Fence_T                *Fence;
typedef struct CommandEncoder_T       *CommandEncoder;
typedef struct Surface_T              *Surface;
typedef struct Swapchain_T            *Swapchain;
typedef struct FrameContext_T         *FrameContext;
typedef struct Device_T               *Device;
typedef struct Instance_T             *Instance;
typedef struct DescriptorHeapInterface DescriptorHeapInterface;
typedef struct CommandEncoderInterface CommandEncoderInterface;
typedef struct DeviceInterface         DeviceInterface;
typedef struct InstanceInterface       InstanceInterface;
typedef struct DescriptorHeapImpl      DescriptorHeapImpl;
typedef struct CommandEncoderImpl      CommandEncoderImpl;
typedef struct DeviceImpl              DeviceImpl;
typedef struct InstanceImpl            InstanceImpl;

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

enum class DeviceFeatures : u64
{
  Basic       = 0x000000ULL,
  VideoEncode = 0x000001ULL,
  VideoDecode = 0x000002ULL,
  RayTracing  = 0x000004ULL
};

ASH_DEFINE_ENUM_BIT_OPS(DeviceFeatures)

enum class MemoryProperties : u32
{
  None            = 0x00000000,
  DeviceLocal     = 0x00000001,
  HostVisible     = 0x00000002,
  HostCoherent    = 0x00000004,
  HostCached      = 0x00000008,
  LazilyAllocated = 0x00000010,
  Protected       = 0x00000020
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

enum class Format : u32
{
  Undefined                                  = 0,
  R4G4_UNORM_PACK8                           = 1,
  R4G4B4A4_UNORM_PACK16                      = 2,
  B4G4R4A4_UNORM_PACK16                      = 3,
  R5G6B5_UNORM_PACK16                        = 4,
  B5G6R5_UNORM_PACK16                        = 5,
  R5G5B5A1_UNORM_PACK16                      = 6,
  B5G5R5A1_UNORM_PACK16                      = 7,
  A1R5G5B5_UNORM_PACK16                      = 8,
  R8_UNORM                                   = 9,
  R8_SNORM                                   = 10,
  R8_USCALED                                 = 11,
  R8_SSCALED                                 = 12,
  R8_UINT                                    = 13,
  R8_SINT                                    = 14,
  R8_SRGB                                    = 15,
  R8G8_UNORM                                 = 16,
  R8G8_SNORM                                 = 17,
  R8G8_USCALED                               = 18,
  R8G8_SSCALED                               = 19,
  R8G8_UINT                                  = 20,
  R8G8_SINT                                  = 21,
  R8G8_SRGB                                  = 22,
  R8G8B8_UNORM                               = 23,
  R8G8B8_SNORM                               = 24,
  R8G8B8_USCALED                             = 25,
  R8G8B8_SSCALED                             = 26,
  R8G8B8_UINT                                = 27,
  R8G8B8_SINT                                = 28,
  R8G8B8_SRGB                                = 29,
  B8G8R8_UNORM                               = 30,
  B8G8R8_SNORM                               = 31,
  B8G8R8_USCALED                             = 32,
  B8G8R8_SSCALED                             = 33,
  B8G8R8_UINT                                = 34,
  B8G8R8_SINT                                = 35,
  B8G8R8_SRGB                                = 36,
  R8G8B8A8_UNORM                             = 37,
  R8G8B8A8_SNORM                             = 38,
  R8G8B8A8_USCALED                           = 39,
  R8G8B8A8_SSCALED                           = 40,
  R8G8B8A8_UINT                              = 41,
  R8G8B8A8_SINT                              = 42,
  R8G8B8A8_SRGB                              = 43,
  B8G8R8A8_UNORM                             = 44,
  B8G8R8A8_SNORM                             = 45,
  B8G8R8A8_USCALED                           = 46,
  B8G8R8A8_SSCALED                           = 47,
  B8G8R8A8_UINT                              = 48,
  B8G8R8A8_SINT                              = 49,
  B8G8R8A8_SRGB                              = 50,
  A8B8G8R8_UNORM_PACK32                      = 51,
  A8B8G8R8_SNORM_PACK32                      = 52,
  A8B8G8R8_USCALED_PACK32                    = 53,
  A8B8G8R8_SSCALED_PACK32                    = 54,
  A8B8G8R8_UINT_PACK32                       = 55,
  A8B8G8R8_SINT_PACK32                       = 56,
  A8B8G8R8_SRGB_PACK32                       = 57,
  A2R10G10B10_UNORM_PACK32                   = 58,
  A2R10G10B10_SNORM_PACK32                   = 59,
  A2R10G10B10_USCALED_PACK32                 = 60,
  A2R10G10B10_SSCALED_PACK32                 = 61,
  A2R10G10B10_UINT_PACK32                    = 62,
  A2R10G10B10_SINT_PACK32                    = 63,
  A2B10G10R10_UNORM_PACK32                   = 64,
  A2B10G10R10_SNORM_PACK32                   = 65,
  A2B10G10R10_USCALED_PACK32                 = 66,
  A2B10G10R10_SSCALED_PACK32                 = 67,
  A2B10G10R10_UINT_PACK32                    = 68,
  A2B10G10R10_SINT_PACK32                    = 69,
  R16_UNORM                                  = 70,
  R16_SNORM                                  = 71,
  R16_USCALED                                = 72,
  R16_SSCALED                                = 73,
  R16_UINT                                   = 74,
  R16_SINT                                   = 75,
  R16_SFLOAT                                 = 76,
  R16G16_UNORM                               = 77,
  R16G16_SNORM                               = 78,
  R16G16_USCALED                             = 79,
  R16G16_SSCALED                             = 80,
  R16G16_UINT                                = 81,
  R16G16_SINT                                = 82,
  R16G16_SFLOAT                              = 83,
  R16G16B16_UNORM                            = 84,
  R16G16B16_SNORM                            = 85,
  R16G16B16_USCALED                          = 86,
  R16G16B16_SSCALED                          = 87,
  R16G16B16_UINT                             = 88,
  R16G16B16_SINT                             = 89,
  R16G16B16_SFLOAT                           = 90,
  R16G16B16A16_UNORM                         = 91,
  R16G16B16A16_SNORM                         = 92,
  R16G16B16A16_USCALED                       = 93,
  R16G16B16A16_SSCALED                       = 94,
  R16G16B16A16_UINT                          = 95,
  R16G16B16A16_SINT                          = 96,
  R16G16B16A16_SFLOAT                        = 97,
  R32_UINT                                   = 98,
  R32_SINT                                   = 99,
  R32_SFLOAT                                 = 100,
  R32G32_UINT                                = 101,
  R32G32_SINT                                = 102,
  R32G32_SFLOAT                              = 103,
  R32G32B32_UINT                             = 104,
  R32G32B32_SINT                             = 105,
  R32G32B32_SFLOAT                           = 106,
  R32G32B32A32_UINT                          = 107,
  R32G32B32A32_SINT                          = 108,
  R32G32B32A32_SFLOAT                        = 109,
  R64_UINT                                   = 110,
  R64_SINT                                   = 111,
  R64_SFLOAT                                 = 112,
  R64G64_UINT                                = 113,
  R64G64_SINT                                = 114,
  R64G64_SFLOAT                              = 115,
  R64G64B64_UINT                             = 116,
  R64G64B64_SINT                             = 117,
  R64G64B64_SFLOAT                           = 118,
  R64G64B64A64_UINT                          = 119,
  R64G64B64A64_SINT                          = 120,
  R64G64B64A64_SFLOAT                        = 121,
  B10G11R11_UFLOAT_PACK32                    = 122,
  E5B9G9R9_UFLOAT_PACK32                     = 123,
  D16_UNORM                                  = 124,
  X8_D24_UNORM_PACK32                        = 125,
  D32_SFLOAT                                 = 126,
  S8_UINT                                    = 127,
  D16_UNORM_S8_UINT                          = 128,
  D24_UNORM_S8_UINT                          = 129,
  D32_SFLOAT_S8_UINT                         = 130,
  BC1_RGB_UNORM_BLOCK                        = 131,
  BC1_RGB_SRGB_BLOCK                         = 132,
  BC1_RGBA_UNORM_BLOCK                       = 133,
  BC1_RGBA_SRGB_BLOCK                        = 134,
  BC2_UNORM_BLOCK                            = 135,
  BC2_SRGB_BLOCK                             = 136,
  BC3_UNORM_BLOCK                            = 137,
  BC3_SRGB_BLOCK                             = 138,
  BC4_UNORM_BLOCK                            = 139,
  BC4_SNORM_BLOCK                            = 140,
  BC5_UNORM_BLOCK                            = 141,
  BC5_SNORM_BLOCK                            = 142,
  BC6H_UFLOAT_BLOCK                          = 143,
  BC6H_SFLOAT_BLOCK                          = 144,
  BC7_UNORM_BLOCK                            = 145,
  BC7_SRGB_BLOCK                             = 146,
  ETC2_R8G8B8_UNORM_BLOCK                    = 147,
  ETC2_R8G8B8_SRGB_BLOCK                     = 148,
  ETC2_R8G8B8A1_UNORM_BLOCK                  = 149,
  ETC2_R8G8B8A1_SRGB_BLOCK                   = 150,
  ETC2_R8G8B8A8_UNORM_BLOCK                  = 151,
  ETC2_R8G8B8A8_SRGB_BLOCK                   = 152,
  EAC_R11_UNORM_BLOCK                        = 153,
  EAC_R11_SNORM_BLOCK                        = 154,
  EAC_R11G11_UNORM_BLOCK                     = 155,
  EAC_R11G11_SNORM_BLOCK                     = 156,
  ASTC_4x4_UNORM_BLOCK                       = 157,
  ASTC_4x4_SRGB_BLOCK                        = 158,
  ASTC_5x4_UNORM_BLOCK                       = 159,
  ASTC_5x4_SRGB_BLOCK                        = 160,
  ASTC_5x5_UNORM_BLOCK                       = 161,
  ASTC_5x5_SRGB_BLOCK                        = 162,
  ASTC_6x5_UNORM_BLOCK                       = 163,
  ASTC_6x5_SRGB_BLOCK                        = 164,
  ASTC_6x6_UNORM_BLOCK                       = 165,
  ASTC_6x6_SRGB_BLOCK                        = 166,
  ASTC_8x5_UNORM_BLOCK                       = 167,
  ASTC_8x5_SRGB_BLOCK                        = 168,
  ASTC_8x6_UNORM_BLOCK                       = 169,
  ASTC_8x6_SRGB_BLOCK                        = 170,
  ASTC_8x8_UNORM_BLOCK                       = 171,
  ASTC_8x8_SRGB_BLOCK                        = 172,
  ASTC_10x5_UNORM_BLOCK                      = 173,
  ASTC_10x5_SRGB_BLOCK                       = 174,
  ASTC_10x6_UNORM_BLOCK                      = 175,
  ASTC_10x6_SRGB_BLOCK                       = 176,
  ASTC_10x8_UNORM_BLOCK                      = 177,
  ASTC_10x8_SRGB_BLOCK                       = 178,
  ASTC_10x10_UNORM_BLOCK                     = 179,
  ASTC_10x10_SRGB_BLOCK                      = 180,
  ASTC_12x10_UNORM_BLOCK                     = 181,
  ASTC_12x10_SRGB_BLOCK                      = 182,
  ASTC_12x12_UNORM_BLOCK                     = 183,
  ASTC_12x12_SRGB_BLOCK                      = 184,
  G8B8G8R8_422_UNORM                         = 1000156000,
  B8G8R8G8_422_UNORM                         = 1000156001,
  G8_B8_R8_3PLANE_420_UNORM                  = 1000156002,
  G8_B8R8_2PLANE_420_UNORM                   = 1000156003,
  G8_B8_R8_3PLANE_422_UNORM                  = 1000156004,
  G8_B8R8_2PLANE_422_UNORM                   = 1000156005,
  G8_B8_R8_3PLANE_444_UNORM                  = 1000156006,
  R10X6_UNORM_PACK16                         = 1000156007,
  R10X6G10X6_UNORM_2PACK16                   = 1000156008,
  R10X6G10X6B10X6A10X6_UNORM_4PACK16         = 1000156009,
  G10X6B10X6G10X6R10X6_422_UNORM_4PACK16     = 1000156010,
  B10X6G10X6R10X6G10X6_422_UNORM_4PACK16     = 1000156011,
  G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16 = 1000156012,
  G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16  = 1000156013,
  G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16 = 1000156014,
  G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16  = 1000156015,
  G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16 = 1000156016,
  R12X4_UNORM_PACK16                         = 1000156017,
  R12X4G12X4_UNORM_2PACK16                   = 1000156018,
  R12X4G12X4B12X4A12X4_UNORM_4PACK16         = 1000156019,
  G12X4B12X4G12X4R12X4_422_UNORM_4PACK16     = 1000156020,
  B12X4G12X4R12X4G12X4_422_UNORM_4PACK16     = 1000156021,
  G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16 = 1000156022,
  G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16  = 1000156023,
  G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16 = 1000156024,
  G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16  = 1000156025,
  G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16 = 1000156026,
  G16B16G16R16_422_UNORM                     = 1000156027,
  B16G16R16G16_422_UNORM                     = 1000156028,
  G16_B16_R16_3PLANE_420_UNORM               = 1000156029,
  G16_B16R16_2PLANE_420_UNORM                = 1000156030,
  G16_B16_R16_3PLANE_422_UNORM               = 1000156031,
  G16_B16R16_2PLANE_422_UNORM                = 1000156032,
  G16_B16_R16_3PLANE_444_UNORM               = 1000156033,
  G8_B8R8_2PLANE_444_UNORM                   = 1000330000,
  G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16  = 1000330001,
  G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16  = 1000330002,
  G16_B16R16_2PLANE_444_UNORM                = 1000330003,
  A4R4G4B4_UNORM_PACK16                      = 1000340000,
  A4B4G4R4_UNORM_PACK16                      = 1000340001,
  ASTC_4x4_SFLOAT_BLOCK                      = 1000066000,
  ASTC_5x4_SFLOAT_BLOCK                      = 1000066001,
  ASTC_5x5_SFLOAT_BLOCK                      = 1000066002,
  ASTC_6x5_SFLOAT_BLOCK                      = 1000066003,
  ASTC_6x6_SFLOAT_BLOCK                      = 1000066004,
  ASTC_8x5_SFLOAT_BLOCK                      = 1000066005,
  ASTC_8x6_SFLOAT_BLOCK                      = 1000066006,
  ASTC_8x8_SFLOAT_BLOCK                      = 1000066007,
  ASTC_10x5_SFLOAT_BLOCK                     = 1000066008,
  ASTC_10x6_SFLOAT_BLOCK                     = 1000066009,
  ASTC_10x8_SFLOAT_BLOCK                     = 1000066010,
  ASTC_10x10_SFLOAT_BLOCK                    = 1000066011,
  ASTC_12x10_SFLOAT_BLOCK                    = 1000066012,
  ASTC_12x12_SFLOAT_BLOCK                    = 1000066013,
  PVRTC1_2BPP_UNORM_BLOCK_IMG                = 1000054000,
  PVRTC1_4BPP_UNORM_BLOCK_IMG                = 1000054001,
  PVRTC2_2BPP_UNORM_BLOCK_IMG                = 1000054002,
  PVRTC2_4BPP_UNORM_BLOCK_IMG                = 1000054003,
  PVRTC1_2BPP_SRGB_BLOCK_IMG                 = 1000054004,
  PVRTC1_4BPP_SRGB_BLOCK_IMG                 = 1000054005,
  PVRTC2_2BPP_SRGB_BLOCK_IMG                 = 1000054006,
  PVRTC2_4BPP_SRGB_BLOCK_IMG                 = 1000054007,
  R16G16_S10_5                               = 1000464000,
  A1B5G5R5_UNORM_PACK16                      = 1000470000,
  A8_UNORM                                   = 1000470001
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

enum class FormatFeatures : u64
{
  None                                                    = 0x00000000ULL,
  SampledImage                                            = 0x00000001ULL,
  StorageImage                                            = 0x00000002ULL,
  StorageImageAtomic                                      = 0x00000004ULL,
  UniformTexelBuffer                                      = 0x00000008ULL,
  StorageTexelBuffer                                      = 0x00000010ULL,
  StorageTexelBufferAtomic                                = 0x00000020ULL,
  VertexBuffer                                            = 0x00000040ULL,
  ColorAttachment                                         = 0x00000080ULL,
  ColorAttachmentBlend                                    = 0x00000100ULL,
  DepthStencilAttachment                                  = 0x00000200ULL,
  BlitSrc                                                 = 0x00000400ULL,
  BlitDst                                                 = 0x00000800ULL,
  SampledImageFilterLinear                                = 0x00001000ULL,
  SampledImageFilterCubic                                 = 0x00002000ULL,
  TransferSrc                                             = 0x00004000ULL,
  TransferDst                                             = 0x00008000ULL,
  SampledImageFilterMinMax                                = 0x00010000ULL,
  MidpointChromaSamples                                   = 0x00020000ULL,
  SampledImageYCbCrConversionLinearFilter                 = 0x00040000ULL,
  SampledImageYCbCrConversionSeparateReconstructionFilter = 0x00080000ULL,
  SampledImageYCbCrConversionChromaReconstructionExplicit = 0x00100000ULL,
  SampledImageYCbCrConversionChromaReconstructionExplicitForceable =
      0x00200000ULL,
  Disjoint                    = 0x00400000ULL,
  CositedChromaSamples        = 0x00800000ULL,
  StorageReadWithoutFormat    = 0x80000000ULL,
  StorageWriteWithoutFormat   = 0x100000000ULL,
  SampledImageDepthComparison = 0x200000000ULL,
  VideoDecodeOutput           = 0x02000000ULL,
  VideoDecodeDpb              = 0x04000000ULL,
  VideoDecodeInput            = 0x08000000ULL,
  VideoEncodeDpb              = 0x10000000ULL
};

ASH_DEFINE_ENUM_BIT_OPS(FormatFeatures)

enum class ImageAspects : u32
{
  None     = 0x00000000,
  Color    = 0x00000001,
  Depth    = 0x00000002,
  Stencil  = 0x00000004,
  MetaData = 0x00000008,
  Plane0   = 0x00000010,
  Plane1   = 0x00000020,
  Plane2   = 0x00000040
};

ASH_DEFINE_ENUM_BIT_OPS(ImageAspects)

enum class SampleCount : u8
{
  None    = 0x00000000,
  Count1  = 0x00000001,
  Count2  = 0x00000002,
  Count4  = 0x00000004,
  Count8  = 0x00000008,
  Count16 = 0x00000010,
  Count32 = 0x00000020,
  Count64 = 0x00000040
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
  None                                    = 0x00000000,
  TransferSrc                             = 0x00000001,
  TransferDst                             = 0x00000002,
  UniformTexelBuffer                      = 0x00000004,
  StorageTexelBuffer                      = 0x00000008,
  UniformBuffer                           = 0x00000010,
  StorageBuffer                           = 0x00000020,
  IndexBuffer                             = 0x00000040,
  VertexBuffer                            = 0x00000080,
  IndirectBuffer                          = 0x00000100,
  VideoDecodeSrc                          = 0x00002000,
  VideoDecodeDst                          = 0x00004000,
  AccelerationStructureBuildInputReadOnly = 0x00080000,
  AccelerationStructureStorage            = 0x00100000,
  ShaderBindingTable                      = 0x00000400,
  VideoEncodeDst                          = 0x00008000,
  VideoEncodeSrc                          = 0x00010000,
  RayTracing                              = ShaderBindingTable
};

ASH_DEFINE_ENUM_BIT_OPS(BufferUsage)

enum class ImageUsage : u32
{
  None                   = 0x00000000,
  TransferSrc            = 0x00000001,
  TransferDst            = 0x00000002,
  Sampled                = 0x00000004,
  Storage                = 0x00000008,
  ColorAttachment        = 0x00000010,
  DepthStencilAttachment = 0x00000020,
  InputAttachment        = 0x00000080,
  VideoDecodeDst         = 0x00000400,
  VideoDecodeSrc         = 0x00000800,
  VideoDecodeDpb         = 0x00001000,
  VideoEncodeDst         = 0x00002000,
  VideoEncodeSrc         = 0x00004000,
  VideoEncodeDpb         = 0x00008000
};

ASH_DEFINE_ENUM_BIT_OPS(ImageUsage)

enum class InputRate : u8
{
  Vertex   = 0,
  Instance = 1
};

enum class ShaderStages : u32
{
  None         = 0x00000000,
  Vertex       = 0x00000001,
  Fragment     = 0x00000010,
  Compute      = 0x00000020,
  AllGraphics  = 0x0000001F,
  All          = 0x7FFFFFFF,
  RayGen       = 0x00000100,
  AnyHit       = 0x00000200,
  ClosestHit   = 0x00000400,
  Miss         = 0x00000800,
  Intersection = 0x00001000,
  Callable     = 0x00002000
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
  None           = 0x00000000,
  Opaque         = 0x00000001,
  PreMultiplied  = 0x00000002,
  PostMultiplied = 0x00000004,
  Inherit        = 0x00000008
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
  DescriptorType type  = DescriptorType::Sampler;
  u32            count = 0;
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

struct SamplerBinding
{
  Sampler sampler = nullptr;
};

struct CombinedImageSampler
{
  Sampler   sampler    = nullptr;
  ImageView image_view = nullptr;
};

struct SampledImageBinding
{
  ImageView image_view = nullptr;
};

struct StorageImageBinding
{
  ImageView image_view = nullptr;
};

struct UniformTexelBufferBinding
{
  BufferView buffer_view = nullptr;
};

struct StorageTexelBufferBinding
{
  BufferView buffer_view = nullptr;
};

struct UniformBufferBinding
{
  Buffer buffer = nullptr;
  u64    offset = 0;
  u64    size   = 0;
};

struct StorageBufferBinding
{
  Buffer buffer = nullptr;
  u64    offset = 0;
  u64    size   = 0;
};

struct DynamicUniformBufferBinding
{
  Buffer buffer = nullptr;
  u64    offset = 0;
  u64    size   = 0;
};

struct DynamicStorageBufferBinding
{
  Buffer buffer = nullptr;
  u64    offset = 0;
  u64    size   = 0;
};

/// used for frame-buffer-local read-operations
struct InputAttachmentBinding
{
  ImageView image_view = nullptr;
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

struct FrameContextDesc
{
  Span<char const> label                = {};
  u32              max_frames_in_flight = 0;
  AllocatorImpl    allocator            = default_allocator;
};

struct DescriptorHeapDesc
{
  Span<DescriptorSetLayout const> descriptor_set_layouts = {};
  u32                             groups_per_pool        = 0;
  AllocatorImpl                   allocator              = default_allocator;
};

struct IndirectDispatchCommand
{
  u32 x = 0;
  u32 y = 0;
  u32 z = 0;
};

struct IndirectDrawCommand
{
  u32 index_count    = 0;
  u32 instance_count = 0;
  u32 first_index    = 0;
  i32 vertex_offset  = 0;
  u32 first_instance = 0;
};

struct IndirectUnindexedDrawCommand
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

/// should be assumed to change from frame to frame.
/// avoid storing pointers to this struct.
struct FrameInfo
{
  FrameId                        tail       = 0;
  FrameId                        current    = 0;
  Span<CommandEncoderImpl const> encoders   = {};
  u32                            ring_index = 0;
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
  DeviceFeatures   features           = DeviceFeatures::Basic;
  DeviceLimits     limits             = {};
};

struct DescriptorSet
{
  DescriptorHeap heap  = nullptr;
  u32            group = 0;
  u32            set   = 0;
};

/// @num_allocated_groups: number of alive group allocations
/// @num_free_groups: number of released and reclaimable desciptor groups
/// @num_released_groups: number of released but non-reclaimable descriptor
/// groups. possibly still in use by the device.
struct DescriptorHeapStats
{
  u32 num_allocated_groups = 0;
  u32 num_free_groups      = 0;
  u32 num_released_groups  = 0;
  u32 num_pools            = 0;
};

struct DescriptorHeapInterface
{
  Result<u32, Status> (*add_group)(DescriptorHeap self)    = nullptr;
  void (*collect)(DescriptorHeap self, FrameId tail_frame) = nullptr;
  void (*mark_in_use)(DescriptorHeap self, u32 group,
                      FrameId current_frame)               = nullptr;
  bool (*is_in_use)(DescriptorHeap self, u32 group,
                    FrameId tail_frame)                    = nullptr;
  void (*release)(DescriptorHeap self, u32 group)          = nullptr;
  DescriptorHeapStats (*get_stats)(DescriptorHeap self)    = nullptr;
  void (*sampler)(DescriptorHeap self, u32 group, u32 set, u32 binding,
                  Span<SamplerBinding const> elements)     = nullptr;
  void (*combined_image_sampler)(
      DescriptorHeap self, u32 group, u32 set, u32 binding,
      Span<CombinedImageSamplerBinding const> elements)           = nullptr;
  void (*sampled_image)(DescriptorHeap self, u32 group, u32 set, u32 binding,
                        Span<SampledImageBinding const> elements) = nullptr;
  void (*storage_image)(DescriptorHeap self, u32 group, u32 set, u32 binding,
                        Span<StorageImageBinding const> elements) = nullptr;
  void (*uniform_texel_buffer)(
      DescriptorHeap self, u32 group, u32 set, u32 binding,
      Span<UniformTexelBufferBinding const> elements) = nullptr;
  void (*storage_texel_buffer)(
      DescriptorHeap self, u32 group, u32 set, u32 binding,
      Span<StorageTexelBufferBinding const> elements)               = nullptr;
  void (*uniform_buffer)(DescriptorHeap self, u32 group, u32 set, u32 binding,
                         Span<UniformBufferBinding const> elements) = nullptr;
  void (*storage_buffer)(DescriptorHeap self, u32 group, u32 set, u32 binding,
                         Span<StorageBufferBinding const> elements) = nullptr;
  void (*dynamic_uniform_buffer)(
      DescriptorHeap self, u32 group, u32 set, u32 binding,
      Span<DynamicUniformBufferBinding const> elements) = nullptr;
  void (*dynamic_storage_buffer)(
      DescriptorHeap self, u32 group, u32 set, u32 binding,
      Span<DynamicStorageBufferBinding const> elements) = nullptr;
  void (*input_attachment)(DescriptorHeap self, u32 group, u32 set, u32 binding,
                           Span<InputAttachmentBinding const> elements) =
      nullptr;
};

struct DescriptorHeapImpl
{
  DescriptorHeap                 self      = nullptr;
  DescriptorHeapInterface const *interface = nullptr;

  constexpr DescriptorHeapInterface const *operator->() const
  {
    return interface;
  }
};

/// to execute tasks at end of frame. use the tail frame index.
struct CommandEncoderInterface
{
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
  void (*draw)(CommandEncoder self, u32 first_index, u32 num_indices,
               i32 vertex_offset, u32 first_instance,
               u32 num_instances)                                     = nullptr;
  void (*draw_indirect)(CommandEncoder self, Buffer buffer, u64 offset,
                        u32 draw_count, u32 stride)                   = nullptr;
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
  Result<DescriptorHeapImpl, Status> (*create_descriptor_heap)(
      Device self, DescriptorHeapDesc const &desc) = nullptr;
  Result<PipelineCache, Status> (*create_pipeline_cache)(
      Device self, PipelineCacheDesc const &desc) = nullptr;
  Result<ComputePipeline, Status> (*create_compute_pipeline)(
      Device self, ComputePipelineDesc const &desc) = nullptr;
  Result<GraphicsPipeline, Status> (*create_graphics_pipeline)(
      Device self, GraphicsPipelineDesc const &desc)                = nullptr;
  Result<Fence, Status> (*create_fence)(Device self, bool signaled) = nullptr;
  Result<FrameContext, Status> (*create_frame_context)(
      Device self, FrameContextDesc const &desc) = nullptr;
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
  void (*destroy_descriptor_heap)(Device             self,
                                  DescriptorHeapImpl heap)          = nullptr;
  void (*destroy_pipeline_cache)(Device self, PipelineCache cache)  = nullptr;
  void (*destroy_compute_pipeline)(Device          self,
                                   ComputePipeline pipeline)        = nullptr;
  void (*destroy_graphics_pipeline)(Device           self,
                                    GraphicsPipeline pipeline)      = nullptr;
  void (*destroy_fence)(Device self, Fence fence)                   = nullptr;
  void (*destroy_frame_context)(Device       self,
                                FrameContext frame_context)         = nullptr;
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
  Result<Void, Status> (*wait_for_fences)(Device self, Span<Fence const> fences,
                                          bool all, u64 timeout)      = nullptr;
  Result<Void, Status> (*reset_fences)(Device            self,
                                       Span<Fence const> fences)      = nullptr;
  Result<bool, Status> (*get_fence_status)(Device self, Fence fence)  = nullptr;
  Result<Void, Status> (*wait_idle)(Device self)                      = nullptr;
  Result<Void, Status> (*wait_queue_idle)(Device self)                = nullptr;
  FrameInfo (*get_frame_info)(Device self, FrameContext frame_context);
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
  Result<Void, Status> (*begin_frame)(Device self, FrameContext frame_context,
                                      Swapchain swapchain)         = nullptr;
  Result<Void, Status> (*submit_frame)(Device self, FrameContext frame_context,
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
  Result<InstanceImpl, Status> (*create)(
      AllocatorImpl allocator, Logger *logger,
      bool enable_validation_layer) = nullptr;
  void (*destroy)(Instance self)    = nullptr;
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

}        // namespace gfx
}        // namespace ash
