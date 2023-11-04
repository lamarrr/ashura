#pragma once
#include <array>
#include <cinttypes>
#include <string_view>

#include "ashura/array.h"
#include "ashura/primitives.h"
#include "ashura/sparse_vec.h"
#include "ashura/utils.h"
#include "stx/enum.h"
#include "stx/fn.h"
#include "stx/result.h"
#include "stx/span.h"

#define ASH_DEFINE_HANDLE(handle) typedef struct handle##_T *handle;

namespace ash
{
namespace gfx
{

constexpr u32 REMAINING_MIP_LEVELS       = ~0U;
constexpr u32 REMAINING_ARRAY_LAYERS     = ~0U;
constexpr u64 WHOLE_SIZE                 = ~0Ui64;
constexpr u32 MAX_COLOR_ATTACHMENTS      = 8;
constexpr u32 MAX_VERTEX_ATTRIBUTES      = 16;
constexpr u32 MAX_PUSH_CONSTANT_SIZE     = 128;
constexpr u32 MAX_MEMORY_HEAP_PROPERTIES = 32;
constexpr u32 MAX_MEMORY_HEAPS           = 16;

ASH_DEFINE_HANDLE(Buffer);
/// format interpretation of a buffer's contents
ASH_DEFINE_HANDLE(BufferView);
ASH_DEFINE_HANDLE(Image);
/// a sub-resource that specifies mips, aspects, and layer of images
ASH_DEFINE_HANDLE(ImageView);
ASH_DEFINE_HANDLE(Sampler);
ASH_DEFINE_HANDLE(Shader);
/// renderpasses are used for selecting tiling strategy and
/// related optimizations
ASH_DEFINE_HANDLE(RenderPass);
ASH_DEFINE_HANDLE(Framebuffer);
ASH_DEFINE_HANDLE(DescriptorSetLayout);
ASH_DEFINE_HANDLE(PipelineCache);
ASH_DEFINE_HANDLE(ComputePipeline);
ASH_DEFINE_HANDLE(GraphicsPipeline);
ASH_DEFINE_HANDLE(CommandBuffer);
ASH_DEFINE_HANDLE(Fence);

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
  None        = 0x000000'000000Ui64,
  VideoEncode = 0x000000'000001Ui64,
  VideoDecode = 0x000000'000002Ui64,
  RayTracing  = 0x000000'000004Ui64
};

STX_DEFINE_ENUM_BIT_OPS(DeviceFeatures)

enum class MemoryProperties : u8
{
  None            = 0x00000000,
  DeviceLocal     = 0x00000001,
  HostVisible     = 0x00000002,
  HostCoherent    = 0x00000004,
  HostCached      = 0x00000008,
  LazilyAllocated = 0x00000010,
  Protected       = 0x00000020
};

STX_DEFINE_ENUM_BIT_OPS(MemoryProperties)

enum class Status : i32
{
  Success              = 0,
  NotReady             = 1,
  Incomplete           = 5,
  OutOfHostMemory      = -1,
  OutOfDeviceMemory    = -2,
  InitializationFailed = -3,
  DeviceLost           = -4,
  MemoryMapFailed      = -5,
  FeatureNotPresent    = -8,
  FormatNotSupported   = -11,
  Unknown              = -13,
  SurfaceLost          = -1000000000
};

enum class FenceStatus : i8
{
  Ready      = 0,
  NotReady   = 1,
  DeviceLost = -4
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

enum class FormatFeatures : u64
{
  None                                                             = 0x00000000Ui64,
  SampledImage                                                     = 0x00000001Ui64,
  StorageImage                                                     = 0x00000002Ui64,
  StorageImageAtomic                                               = 0x00000004Ui64,
  UniformTexelBuffer                                               = 0x00000008Ui64,
  StorageTexelBuffer                                               = 0x00000010Ui64,
  StorageTexelBufferAtomic                                         = 0x00000020Ui64,
  VertexBuffer                                                     = 0x00000040Ui64,
  ColorAttachment                                                  = 0x00000080Ui64,
  ColorAttachmentBlend                                             = 0x00000100Ui64,
  DepthStencilAttachment                                           = 0x00000200Ui64,
  BlitSrc                                                          = 0x00000400Ui64,
  BlitDst                                                          = 0x00000800Ui64,
  SampledImageFilterLinear                                         = 0x00001000Ui64,
  SampledImageFilterCubic                                          = 0x00002000Ui64,
  TransferSrc                                                      = 0x00004000Ui64,
  TransferDst                                                      = 0x00008000Ui64,
  SampledImageFilterMinMax                                         = 0x00010000Ui64,
  MidpointChromaSamples                                            = 0x00020000Ui64,
  SampledImageYCbCrConversionLinearFilter                          = 0x00040000Ui64,
  SampledImageYCbCrConversionSeparateReconstructionFilter          = 0x00080000Ui64,
  SampledImageYCbCrConversionChromaReconstructionExplicit          = 0x00100000Ui64,
  SampledImageYCbCrConversionChromaReconstructionExplicitForceable = 0x00200000Ui64,
  Disjoint                                                         = 0x00400000Ui64,
  CositedChromaSamples                                             = 0x00800000Ui64,
  StorageReadWithoutFormat                                         = 0x80000000Ui64,
  StorageWriteWithoutFormat                                        = 0x100000000Ui64,
  SampledImageDepthComparison                                      = 0x200000000Ui64,
  VideoDecodeOutput                                                = 0x02000000Ui64,
  VideoDecodeDpb                                                   = 0x04000000Ui64,
  VideoDecodeInput                                                 = 0x08000000Ui64,
  VideoEncodeDpb                                                   = 0x10000000Ui64
};

enum class ImageAspects : u8
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

STX_DEFINE_ENUM_BIT_OPS(ImageAspects)

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

STX_DEFINE_ENUM_BIT_OPS(ColorComponents)

/// used for synchronization of state-mutating commands
enum class BufferUsage : u32
{
  None                      = 0x00000000,
  TransferSrc               = 0x00000001,
  TransferDst               = 0x00000002,
  IndirectCommand           = 0x00000004,
  ComputeShaderUniform      = 0x00000008,
  ComputeShaderUniformTexel = 0x00000010,
  ComputeShaderStorage      = 0x00000020,
  ComputeShaderStorageTexel = 0x00000040,
  IndexBuffer               = 0x00000080,
  VertexBuffer              = 0x00000100,
  VertexShaderUniform       = 0x00000200,
  FragmentShaderUniform     = 0x00000400,
  All                       = 0xFFFFFFFF
};

STX_DEFINE_ENUM_BIT_OPS(BufferUsage)

/// used for synchronization of state-mutating commands
/// must provide initial clear value or initial buffer initializer
// images implicitly have TransferDst usage scope
enum class ImageUsage : u32
{
  None                        = 0x00000000,
  TransferSrc                 = 0x00000001,
  TransferDst                 = 0x00000002,
  ComputeShaderSampled        = 0x00000004,
  ComputeShaderStorage        = 0x00000008,
  VertexShaderSampled         = 0x00000010,
  FragmentShaderSampled       = 0x00000020,
  InputAttachment             = 0x00000040,
  ReadColorAttachment         = 0x00000080,
  WriteColorAttachment        = 0x00000100,
  ReadDepthStencilAttachment  = 0x00000200,
  WriteDepthStencilAttachment = 0x00000400,
  PresentSrc                  = 0x00000800,
  All                         = 0xFFFFFFFF
};

STX_DEFINE_ENUM_BIT_OPS(ImageUsage)

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

STX_DEFINE_ENUM_BIT_OPS(ShaderStages)

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
  Type1D      = 0,
  Type2D      = 1,
  Type3D      = 2,
  TypeCube    = 3,
  Type1DArray = 4,
  Type2DArray = 5,
  Type3DArray = 6
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

struct MemoryRange
{
  u64 offset = 0;
  u64 size   = 0;
};

struct Viewport
{
  Rect area;
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

/// [properties] is either of:
///
/// HostVisible | HostCoherent
/// HostVisible | HostCached
/// HostVisible | HostCached | HostCoherent
/// DeviceLocal
/// DeviceLocal | HostVisible | HostCoherent
/// DeviceLocal | HostVisible | HostCached
/// DeviceLocal | HostVisible | HostCached | HostCoherent
struct HeapProperty
{
  MemoryProperties properties = MemoryProperties::None;
  u32              index      = 0;
};

// TODO(lamarrr): write memory allocation strategies, i.e. images should be allocated on this and
// this heap a single heap might have multiple properties
struct DeviceMemoryHeaps
{
  // ordered by performance-tier (MemoryProperties)
  stx::Array<HeapProperty, MAX_MEMORY_HEAP_PROPERTIES> heap_properties = {};
  stx::Array<u64, MAX_MEMORY_HEAPS>                    heap_sizes      = {};

  constexpr bool has_memory(MemoryProperties properties) const
  {
    for (HeapProperty p : heap_properties)
    {
      if (has_bits(p.properties, properties))
      {
        return true;
      }
    }
    return false;
  }

  constexpr bool has_unified_memory() const
  {
    return has_memory(MemoryProperties::DeviceLocal | MemoryProperties::HostVisible);
  }
};

struct BufferDesc
{
  char const      *label      = nullptr;
  u64              size       = 0;
  MemoryProperties properties = MemoryProperties::None;
  BufferUsage      usage      = BufferUsage::None;
};

struct BufferViewDesc
{
  char const *label  = nullptr;
  Buffer      buffer = nullptr;
  Format      format = Format::Undefined;
  u64         offset = 0;
  u64         size   = 0;
};

struct ImageDesc
{
  char const  *label        = nullptr;
  ImageType    type         = ImageType::Type1D;
  Format       format       = Format::Undefined;
  ImageUsage   usage        = ImageUsage::None;
  ImageAspects aspects      = ImageAspects::None;
  Extent3D     extent       = {};
  u32          mip_levels   = 0;
  u32          array_layers = 0;
};

struct ImageViewDesc
{
  char const      *label             = nullptr;
  Image            image             = nullptr;
  ImageViewType    view_type         = ImageViewType::Type1D;
  Format           view_format       = Format::Undefined;
  ComponentMapping mapping           = ComponentMapping{};
  ImageAspects     aspects           = ImageAspects::None;
  u32              first_mip_level   = 0;
  u32              num_mip_levels    = 0;
  u32              first_array_layer = 0;
  u32              num_array_layers  = 0;
};

struct SamplerDesc
{
  char const        *label                    = nullptr;
  Filter             mag_filter               = Filter::Nearest;
  Filter             min_filter               = Filter::Nearest;
  SamplerMipMapMode  mip_map_mode             = SamplerMipMapMode::Nearest;
  SamplerAddressMode address_mode_u           = SamplerAddressMode::Repeat;
  SamplerAddressMode address_mode_v           = SamplerAddressMode::Repeat;
  SamplerAddressMode address_mode_w           = SamplerAddressMode::Repeat;
  f32                mip_lod_bias             = 0;
  bool               anisotropy_enable        = false;
  f32                max_anisotropy           = 0;
  bool               compare_enable           = false;
  CompareOp          compare_op               = CompareOp::Never;
  f32                min_lod                  = 0;
  f32                max_lod                  = 0;
  BorderColor        border_color             = BorderColor::FloatTransparentBlack;
  bool               unnormalized_coordinates = false;
};

struct ShaderDesc
{
  char const          *label      = nullptr;
  stx::Span<u32 const> spirv_code = {};
};

struct RenderPassAttachment
{
  Format  format           = Format::Undefined;
  LoadOp  load_op          = LoadOp::Load;        // how to use color and depth components
  StoreOp store_op         = StoreOp::Store;
  LoadOp  stencil_load_op  = LoadOp::Load;        // how to use stencil components
  StoreOp stencil_store_op = StoreOp::Store;
};

struct RenderPassDesc
{
  char const                                             *label                    = nullptr;
  stx::Array<RenderPassAttachment, MAX_COLOR_ATTACHMENTS> color_attachments        = {};
  stx::Array<RenderPassAttachment, MAX_COLOR_ATTACHMENTS> input_attachments        = {};
  RenderPassAttachment                                    depth_stencil_attachment = {};
};

struct FramebufferDesc
{
  char const                                  *label                    = nullptr;
  RenderPass                                   renderpass               = nullptr;
  Extent                                       extent                   = {};
  u32                                          layers                   = 0;
  stx::Array<ImageView, MAX_COLOR_ATTACHMENTS> color_attachments        = {};
  ImageView                                    depth_stencil_attachment = nullptr;
};

struct DescriptorBindingDesc
{
  u32            binding = 0;
  DescriptorType type    = DescriptorType::Sampler;
  u32            count   = 0;
  ShaderStages   stages  = ShaderStages::None;
};

struct DescriptorSetLayoutDesc
{
  char const                            *label = nullptr;
  stx::Span<DescriptorBindingDesc const> bindings;
};

struct PipelineCacheDesc
{
  stx::Span<u8 const> initial_data;
};

struct DescriptorSetCount
{
  u32 num_samplers                = 0;
  u32 num_combined_image_samplers = 0;
  u32 num_sampled_images          = 0;
  u32 num_storage_images          = 0;
  u32 num_uniform_texel_buffers   = 0;
  u32 num_storage_texel_buffers   = 0;
  u32 num_uniform_buffers         = 0;
  u32 num_storage_buffers         = 0;
  u32 num_input_attachments       = 0;
};

struct SamplerBinding
{
  u32     binding_id  = 0;
  u32     array_index = 0;
  u32     count       = 0;
  Sampler sampler     = nullptr;
};

struct CombinedImageSamplerBinding
{
  u32       binding_id  = 0;
  u32       array_index = 0;
  u32       count       = 0;
  Sampler   sampler     = nullptr;
  ImageView image_view  = nullptr;
};

struct SampledImageBinding
{
  u32       binding_id  = 0;
  u32       array_index = 0;
  u32       count       = 0;
  ImageView image_view  = nullptr;
};

struct StorageImageBinding
{
  u32       binding_id  = 0;
  u32       array_index = 0;
  u32       count       = 0;
  ImageView image_view  = nullptr;
};

struct UniformTexelBufferBinding
{
  u32        binding_id  = 0;
  u32        array_index = 0;
  u32        count       = 0;
  BufferView buffer_view = nullptr;
};

struct StorageTexelBufferBinding
{
  u32        binding_id  = 0;
  u32        array_index = 0;
  u32        count       = 0;
  BufferView buffer_view = nullptr;
};

struct UniformBufferBinding
{
  u32    binding_id  = 0;
  u32    array_index = 0;
  u32    count       = 0;
  Buffer buffer      = nullptr;
  u64    offset      = 0;
  u64    size        = 0;
};

struct StorageBufferBinding
{
  u32    binding_id  = 0;
  u32    array_index = 0;
  u32    count       = 0;
  Buffer buffer      = nullptr;
  u64    offset      = 0;
  u64    size        = 0;
};

/// used for frame-buffer-local read-operations
struct InputAttachmentBinding
{
  u32       binding_id  = 0;
  u32       array_index = 0;
  u32       count       = 0;
  ImageView image_view  = nullptr;
};

struct DescriptorSetBindings
{
  stx::Span<SamplerBinding const>              samplers                = {};
  stx::Span<CombinedImageSamplerBinding const> combined_image_samplers = {};
  stx::Span<SampledImageBinding const>         sampled_images          = {};
  stx::Span<StorageImageBinding const>         storage_images          = {};
  stx::Span<UniformTexelBufferBinding const>   uniform_texel_buffers   = {};
  stx::Span<StorageTexelBufferBinding const>   storage_texel_buffers   = {};
  stx::Span<UniformBufferBinding const>        uniform_buffers         = {};
  stx::Span<StorageBufferBinding const>        storage_buffers         = {};
  stx::Span<InputAttachmentBinding const>      input_attachments       = {};
};

struct SpecializationConstant
{
  u32   constant_id = 0;
  u32   offset      = 0;
  usize size        = 0;
};

struct ShaderStageDesc
{
  Shader                                  shader                        = nullptr;
  char const                             *entry_point                   = nullptr;
  stx::Span<u8 const>                     specialization_constants_data = {};
  stx::Span<SpecializationConstant const> specialization_constants      = {};
};

struct ComputePipelineDesc
{
  char const         *label                 = nullptr;
  ShaderStageDesc     compute_shader        = {};
  u32                 push_constant_size    = 0;
  DescriptorSetLayout descriptor_set_layout = nullptr;
  PipelineCache       cache                 = nullptr;
};

// Specifies how the binded vertex buffers are iterated and the strides for them
// unique for each binded buffer.
struct VertexInputBinding
{
  // which of the binded vertex buffers
  u32 binding = 0;
  // stride in bytes for each binding advance within the binded buffer
  u32 stride = 0;
  // advance-rate for this binding. on every vertex or every instance
  InputRate input_rate = InputRate::Vertex;
};

// specifies representation/interpretation and shader location mapping of the values in the buffer
// this is a many to one mapping to the input binding.
struct VertexAttribute
{
  u32    binding  = 0;                        // which binding
  u32    location = 0;                        // binding's mapped location
  Format format   = Format::Undefined;        // data format
  u32    offset   = 0;                        // offset of attribute in binding
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
  bool                                                                 logic_op_enable = false;
  LogicOp                                                              logic_op    = LogicOp::Clear;
  stx::Array<PipelineColorBlendAttachmentState, MAX_COLOR_ATTACHMENTS> attachments = {};
  Vec4                                                                 blend_constants;
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

// TODO(lamarrr): figure out lifetimes
struct GraphicsPipelineDesc
{
  char const                                           *label                 = nullptr;
  ShaderStageDesc                                       vertex_shader         = {};
  ShaderStageDesc                                       fragment_shader       = {};
  RenderPass                                            render_pass           = nullptr;
  stx::Array<VertexInputBinding, MAX_VERTEX_ATTRIBUTES> vertex_input_bindings = {};
  stx::Array<VertexAttribute, MAX_VERTEX_ATTRIBUTES>    vertex_attributes     = {};
  u32                                                   push_constant_size    = 0;
  DescriptorSetLayout                                   descriptor_set_layout = nullptr;
  PrimitiveTopology          primitive_topology  = PrimitiveTopology::PointList;
  PipelineRasterizationState rasterization_state = {};
  PipelineDepthStencilState  depth_stencil_state = {};
  PipelineColorBlendState    color_blend_state   = {};
  PipelineCache              cache               = nullptr;
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
  URect3D                image_area          = {};
  ImageSubresourceLayers image_layers        = {};
};

struct ImageCopy
{
  URect3D                src_area   = {};
  ImageSubresourceLayers src_layers = {};
  Offset3D               dst_offset = {};
  ImageSubresourceLayers dst_layers = {};
};

struct ImageBlit
{
  URect3D                src_area   = {};
  ImageSubresourceLayers src_layers = {};
  URect3D                dst_area   = {};
  ImageSubresourceLayers dst_layers = {};
};

union Color
{
  u32 uint32[4] = {0, 0, 0, 0};
  i32 int32[4];
  f32 float32[4];
};

struct DepthStencil
{
  f32 depth   = 0;
  u32 stencil = 0;
};

union ClearValue
{
  Color        color = Color{};
  DepthStencil depth_stencil;
};

struct BufferResource
{
  u32        refcount = 0;
  void      *handle   = nullptr;
  BufferDesc desc     = {};
};

struct BufferViewResource
{
  u32            refcount = 0;
  void          *handle   = nullptr;
  BufferViewDesc desc     = {};
};

struct ImageResource
{
  u32       refcount           = 0;
  void     *handle             = nullptr;
  bool      externally_managed = false;
  ImageDesc desc               = {};
};

struct ImageViewResource
{
  u32           refcount = 0;
  void         *handle   = nullptr;
  ImageViewDesc desc     = {};
};

struct RenderPassResource
{
  u32            refcount = 0;
  void          *handle   = nullptr;
  RenderPassDesc desc     = {};
};

struct FramebufferResource
{
  u32             refcount = 0;
  void           *handle   = nullptr;
  FramebufferDesc desc     = {};
};

struct ShaderResource
{
  u32         refcount = 0;
  void       *handle   = nullptr;
  char const *label    = nullptr;
};

struct PipelineCacheResource
{
  u32   refcount = 0;
  void *handle   = nullptr;
};

struct ComputePipelineResource
{
  u32   refcount = 0;
  void *handle   = nullptr;
};

struct GraphicsPipelineResource
{
  u32   refcount = 0;
  void *handle   = nullptr;
};

struct SamplerResource
{
  u32   refcount = 0;
  void *handle   = nullptr;
};

struct DescriptorSetLayoutResource
{
  u32                refcount = 0;
  void              *handle   = nullptr;
  DescriptorSetCount count    = {};
};

struct FenceResource
{
  u32   refcount = 0;
  void *handle   = nullptr;
};

struct SwapchainResources
{
  stx::Array<Image, 8>     images      = {};
  stx::Array<ImageView, 8> image_views = {};
};

// TODO(lamarrr): formats info properties
// device selection
// instance
struct DeviceInfo
{
  // device name
  // vendor name
  // driver name
  DeviceType        type           = DeviceType::Other;
  DeviceMemoryHeaps memory_heaps   = {};
  f32               max_anisotropy = 1.0f;
  DeviceFeatures    features       = DeviceFeatures::None;
  // FrameInfo -> current display size
  // -> current display format
  // supports hdr?
  // is format hdr
  // dci p3?
  //
};

struct Device;

struct CommandEncoder
{
  virtual ~CommandEncoder()                                                                   = 0;
  virtual void                       begin()                                                  = 0;
  virtual void                       end()                                                    = 0;
  virtual void                       reset()                                                  = 0;
  virtual void                       begin_debug_marker(char const *region_name, Vec4 color)  = 0;
  virtual void                       end_debug_marker()                                       = 0;
  virtual stx::Result<Image, Status> create_image(ImageDesc const &desc, Color initial_color) = 0;
  virtual stx::Result<Image, Status> create_image(ImageDesc const &desc,
                                                  DepthStencil     initial_depth_stencil)         = 0;
  virtual stx::Result<Image, Status> create_image(ImageDesc const &desc, Buffer initial_data,
                                                  stx::Span<BufferImageCopy const> copies)    = 0;
  virtual void                       fill_buffer(Buffer dst, u64 offset, u64 size, u32 data)  = 0;
  virtual void copy_buffer(Buffer src, Buffer dst, stx::Span<BufferCopy const> copies)        = 0;
  virtual void update_buffer(stx::Span<u8 const> src, u64 dst_offset, Buffer dst)             = 0;
  virtual void clear_color_image(Image dst, stx::Span<Color const> clear_colors,
                                 stx::Span<ImageSubresourceRange const> ranges)               = 0;
  virtual void clear_depth_stencil_image(Image                         dst,
                                         stx::Span<DepthStencil const> clear_depth_stencils,
                                         stx::Span<ImageSubresourceRange const> ranges)       = 0;
  virtual void copy_image(Image src, Image dst, stx::Span<ImageCopy const> copies)            = 0;
  virtual void copy_buffer_to_image(Buffer src, Image dst,
                                    stx::Span<BufferImageCopy const> copies)                  = 0;
  virtual void blit_image(Image src, Image dst, stx::Span<ImageBlit const> blits,
                          Filter filter)                                                      = 0;
  virtual void
               begin_render_pass(Framebuffer framebuffer, RenderPass render_pass, IRect render_area,
                                 stx::Span<Color const>        color_attachments_clear_values,
                                 stx::Span<DepthStencil const> depth_stencil_attachments_clear_values) = 0;
  virtual void end_render_pass()                                                    = 0;
  virtual void bind_pipeline(ComputePipeline pipeline, DescriptorSetLayout layout)  = 0;
  virtual void bind_pipeline(GraphicsPipeline pipeline, DescriptorSetLayout layout) = 0;
  virtual void push_descriptors(DescriptorSetBindings const &bindings)              = 0;
  virtual void push_constants(stx::Span<u8 const> push_constants_data)              = 0;
  virtual void dispatch(u32 group_count_x, u32 group_count_y, u32 group_count_z)    = 0;
  virtual void dispatch_indirect(Buffer buffer, u64 offset)                         = 0;
  virtual void set_viewport(Viewport const &viewport)                               = 0;
  virtual void set_scissor(IRect scissor)                                           = 0;
  virtual void set_blend_constants(Vec4 blend_constants)                            = 0;
  virtual void set_stencil_compare_mask(StencilFaces faces, u32 mask)               = 0;
  virtual void set_stencil_reference(StencilFaces faces, u32 reference)             = 0;
  virtual void set_stencil_write_mask(StencilFaces faces, u32 mask)                 = 0;
  virtual void set_vertex_buffers(stx::Span<Buffer const> vertex_buffers)           = 0;
  virtual void draw(Buffer index_buffer, u32 first_index, u32 num_indices, u32 vertex_offset,
                    u32 first_instance, u32 num_instances)                          = 0;
  virtual void draw_indirect(Buffer index_buffer, Buffer buffer, u64 offset, u32 draw_count,
                             u32 stride)                                            = 0;
  virtual void on_execution_complete(stx::UniqueFn<void()> &&fn)                    = 0;
};

struct DeviceResources
{
  stx::SparseVec<BufferResource, Buffer>                           buffers;
  stx::SparseVec<BufferViewResource, BufferView>                   buffer_views;
  stx::SparseVec<ImageResource, Image>                             images;
  stx::SparseVec<ImageViewResource, ImageView>                     image_views;
  stx::SparseVec<SamplerResource, Sampler>                         samplers;
  stx::SparseVec<ShaderResource, Shader>                           shaders;
  stx::SparseVec<RenderPassResource, RenderPass>                   render_passes;
  stx::SparseVec<FramebufferResource, Framebuffer>                 framebuffers;
  stx::SparseVec<DescriptorSetLayoutResource, DescriptorSetLayout> descriptor_set_layouts;
  stx::SparseVec<PipelineCacheResource, PipelineCache>             pipeline_caches;
  stx::SparseVec<ComputePipelineResource, ComputePipeline>         compute_pipelines;
  stx::SparseVec<GraphicsPipelineResource, GraphicsPipeline>       graphics_pipelines;
  stx::SparseVec<FenceResource, Fence>                             fences;
};

// Single-threaded
// Lock????
struct Device
{
  DeviceResources resources;

  virtual ~Device() = 0;

  virtual stx::Result<FormatProperties, Status> get_format_properties(Format format)            = 0;
  virtual stx::Result<Buffer, Status>           create_buffer(BufferDesc const &desc)           = 0;
  virtual stx::Result<BufferView, Status>       create_buffer_view(BufferViewDesc const &desc)  = 0;
  virtual stx::Result<ImageView, Status>        create_image_view(ImageViewDesc const &desc)    = 0;
  virtual stx::Result<Sampler, Status>          create_sampler(SamplerDesc const &desc)         = 0;
  virtual stx::Result<Shader, Status>           create_shader(ShaderDesc const &desc)           = 0;
  virtual stx::Result<RenderPass, Status>       create_render_pass(RenderPassDesc const &desc)  = 0;
  virtual stx::Result<Framebuffer, Status>      create_framebuffer(FramebufferDesc const &desc) = 0;
  virtual stx::Result<DescriptorSetLayout, Status>
      create_descriptor_set_layout(DescriptorSetLayoutDesc const &desc) = 0;
  virtual stx::Result<PipelineCache, Status>
      create_pipeline_cache(PipelineCacheDesc const &desc) = 0;
  virtual stx::Result<ComputePipeline, Status>
      create_compute_pipeline(ComputePipelineDesc const &desc) = 0;
  virtual stx::Result<GraphicsPipeline, Status>
                                     create_graphics_pipeline(GraphicsPipelineDesc const &desc) = 0;
  virtual stx::Result<Fence, Status> create_fence(bool signaled)                                = 0;
  virtual stx::Result<CommandEncoder *, Status> create_command_encoder()                        = 0;

  virtual void ref(Buffer buffer)                             = 0;
  virtual void ref(BufferView buffer_view)                    = 0;
  virtual void ref(Image image)                               = 0;
  virtual void ref(ImageView image_view)                      = 0;
  virtual void ref(Sampler sampler)                           = 0;
  virtual void ref(Shader shader)                             = 0;
  virtual void ref(RenderPass render_pass)                    = 0;
  virtual void ref(Framebuffer framebuffer)                   = 0;
  virtual void ref(DescriptorSetLayout descriptor_set_layout) = 0;
  virtual void ref(PipelineCache cache)                       = 0;
  virtual void ref(ComputePipeline pipeline)                  = 0;
  virtual void ref(GraphicsPipeline pipeline)                 = 0;
  virtual void ref(Fence fence)                               = 0;
  virtual void ref(CommandEncoder *command_encoder)           = 0;

  virtual void unref(Buffer buffer)                             = 0;
  virtual void unref(BufferView buffer_view)                    = 0;
  virtual void unref(Image image)                               = 0;
  virtual void unref(ImageView image_view)                      = 0;
  virtual void unref(Sampler sampler)                           = 0;
  virtual void unref(Shader shader)                             = 0;
  virtual void unref(RenderPass render_pass)                    = 0;
  virtual void unref(Framebuffer framebuffer)                   = 0;
  virtual void unref(DescriptorSetLayout descriptor_set_layout) = 0;
  virtual void unref(PipelineCache cache)                       = 0;
  virtual void unref(ComputePipeline pipeline)                  = 0;
  virtual void unref(GraphicsPipeline pipeline)                 = 0;
  virtual void unref(Fence fence)                               = 0;
  virtual void unref(CommandEncoder *command_encoder)           = 0;

  virtual void *get_buffer_memory_map(Buffer buffer)                                            = 0;
  virtual void invalidate_buffer_memory_map(Buffer buffer, stx::Span<MemoryRange const> ranges) = 0;
  virtual void flush_buffer_memory_map(Buffer buffer, stx::Span<MemoryRange const> ranges)      = 0;
  virtual usize       get_pipeline_cache_size(PipelineCache cache)                              = 0;
  virtual void        get_pipeline_cache_data(PipelineCache cache, stx::Span<u8> out)           = 0;
  virtual void        wait_for_fences(stx::Span<Fence const> fences, bool all, u64 timeout)     = 0;
  virtual void        reset_fences(stx::Span<Fence const> fences)                               = 0;
  virtual FenceStatus get_fence_status(Fence fence)                                             = 0;
  virtual void        submit(CommandEncoder *encoder, Fence signal_fence)                       = 0;
  virtual void        wait_idle()                                                               = 0;
};

// MT? how will mt even work?
template <typename Handle>
struct Rc
{
  Device *dev    = nullptr;
  Handle  handle = nullptr;

  constexpr Rc(Device *idev, Handle ihandle) : dev{idev}, handle{ihandle}
  {
  }

  constexpr Rc(Rc const &other) : dev{other.dev}, handle{other.handle}
  {
    dev->ref(handle);
  }

  constexpr Rc(Rc &&other) : dev{other.dev}, handle{other.handle}
  {
    other.dev    = nullptr;
    other.handle = nullptr;
  }

  constexpr Rc &operator=(Rc const &other)
  {
    this->~Rc();        //self-assign
    new (this) Rc{other};
    return *this;
  }

  constexpr Rc &operator=(Rc &&other)
  {
    std::swap(dev, other.dev);
    std::swap(handle, other.handle);
    return *this;
  }

  constexpr ~Rc()
  {
    if (handle != nullptr && dev != nullptr)
    {
      dev->unref(handle);
    }
  }

  constexpr Handle leak()
  {
    Handle out = dev;
    dev        = nullptr;
    handle     = nullptr;
    return out;
  }
};

/*


namespace ash
{
namespace rcg
{

constexpr u8 MAX_FRAMES_IN_FLIGHT = 4;

// we'll support GLSL->SPIRV and Shader Editor -> C++ -> GLSL -> SPIRV
// contains all loaded shaders
//
//
// TODO(lamarrr): multi-pass dependency? knowing when to free resources
//
struct ShaderMap
{
  // shaders are always compiled and loaded at startup time
  // select shader
  // shader will have vendor, context, and name to compare to
  // if none was given then how?
  // we don't allow shaders to change at runtime, they must be baked and compiled AOT
  //
  // TODO(lamarrr): PSO caches
  //
  //
};

struct PipelineCacheMap
{
  // vendor id, pass name, name
  // frag shader id, vert shader id
};

struct Graph;

// used for: validation layer, logging, warning, and driver dispatch
struct CommandBuffer
{
  rhi::Driver                    *driver            = nullptr;
  CommandBuffer              rhi               = nullptr;
  Graph                          *graph             = nullptr;
  CommandBufferHook              *hook              = nullptr;
  RenderPass                 render_pass       = nullptr;
  stx::Vec<stx::UniqueFn<void()>> completion_tasks  = {};        // MUST be run in reverse order
};

// TODO(lamarrr): how do we enable features like raytracing dynamically at runtime?
// each pass declares flags?
//
// TODO(lamarrr): async shader compilation and loading, how?
//
//
// for each creation and unref commands, we'll insert optional hooks that check that the
// parameters are correct we need to check the graph for information regarding the type and its
// dependencies
//
//
// on scheduled frame_fence sent check if any resource has been requested to be unrefd, if so,
// unref
//
//
*/

}        // namespace gfx
}        // namespace ash