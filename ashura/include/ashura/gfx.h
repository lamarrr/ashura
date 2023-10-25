#pragma once
#include <array>
#include <cinttypes>

#include "ashura/primitives.h"
#include "ashura/utils.h"
#include "stx/enum.h"
#include "stx/span.h"

#define ASH_DEFINE_HANDLE(handle) \
  typedef struct handle##_T;      \
  typedef handle##_T *handle;

namespace ash
{
namespace gfx
{

constexpr u32 REMAINING_MIP_LEVELS   = ~0U;
constexpr u32 REMAINING_ARRAY_LAYERS = ~0U;
constexpr u64 WHOLE_SIZE             = ~0ULL;
// TODO(lamarrr): we have to check these against usage and device info
constexpr u8 MAX_DESCRIPTOR_SET_BINDINGS = 16;
constexpr u8 MAX_COLOR_ATTACHMENTS       = 8;
constexpr u8 MAX_VERTEX_ATTRIBUTES       = 16;
constexpr u8 MAX_PUSH_CONSTANT_SIZE      = 128;

ASH_DEFINE_HANDLE(Buffer);
ASH_DEFINE_HANDLE(BufferView);
/// format interpretation of a buffer's contents
ASH_DEFINE_HANDLE(Image);
ASH_DEFINE_HANDLE(ImageView);
/// a sub-resource that specifies mips, aspects, and layer of images
ASH_DEFINE_HANDLE(Sampler);
ASH_DEFINE_HANDLE(Shader);
ASH_DEFINE_HANDLE(RenderPass);
/// renderpasses are used for selecting tiling strategy and
/// related optimizations
ASH_DEFINE_HANDLE(Framebuffer);
ASH_DEFINE_HANDLE(DescriptorSetLayout);
ASH_DEFINE_HANDLE(ComputePipeline);
ASH_DEFINE_HANDLE(GraphicsPipeline);
ASH_DEFINE_HANDLE(CommandBuffer);

enum class MemoryProperties : u8
{
  None            = 0x00000000,
  DeviceLocal     = 0x00000001,
  HostVisible     = 0x00000002,
  HostCoherent    = 0x00000004,
  HostCached      = 0x00000008,
  LazilyAllocated = 0x00000010,
  Protected       = 0x00000020,
};

STX_DEFINE_ENUM_BIT_OPS(MemoryProperties)

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
  None                                                             = 0x00000000ULL,
  SampledImage                                                     = 0x00000001ULL,
  StorageImage                                                     = 0x00000002ULL,
  StorageImageAtomic                                               = 0x00000004ULL,
  UniformTexelBuffer                                               = 0x00000008ULL,
  StorageTexelBuffer                                               = 0x00000010ULL,
  StorageTexelBufferAtomic                                         = 0x00000020ULL,
  VertexBuffer                                                     = 0x00000040ULL,
  ColorAttachment                                                  = 0x00000080ULL,
  ColorAttachmentBlend                                             = 0x00000100ULL,
  DepthStencilAttachment                                           = 0x00000200ULL,
  BlitSrc                                                          = 0x00000400ULL,
  BlitDst                                                          = 0x00000800ULL,
  SampledImageFilterLinear                                         = 0x00001000ULL,
  SampledImageFilterCubic                                          = 0x00002000ULL,
  TransferSrc                                                      = 0x00004000ULL,
  TransferDst                                                      = 0x00008000ULL,
  SampledImageFilterMinMax                                         = 0x00010000ULL,
  MidpointChromaSamples                                            = 0x00020000ULL,
  SampledImageYCbCrConversionLinearFilter                          = 0x00040000ULL,
  SampledImageYCbCrConversionSeparateReconstructionFilter          = 0x00080000ULL,
  SampledImageYCbCrConversionChromaReconstructionExplicit          = 0x00100000ULL,
  SampledImageYCbCrConversionChromaReconstructionExplicitForceable = 0x00200000ULL,
  Disjoint                                                         = 0x00400000ULL,
  CositedChromaSamples                                             = 0x00800000ULL,
  StorageReadWithoutFormat                                         = 0x80000000ULL,
  StorageWriteWithoutFormat                                        = 0x100000000ULL,
  SampledImageDepthComparison                                      = 0x200000000ULL,
  VideoDecodeOutput                                                = 0x02000000ULL,
  VideoDecodeDpb                                                   = 0x04000000ULL,
  VideoDecodeInput                                                 = 0x08000000ULL,
  VideoEncodeDpb                                                   = 0x10000000ULL
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

enum class ImageLayout : u32
{
  Undefined                     = 0,
  General                       = 1,
  ColorAttachmentOptimal        = 2,
  DepthStencilAttachmentOptimal = 3,
  DepthStencilReadOnlyOptimal   = 4,
  ShaderReadOnlyOptimal         = 5,
  TransferSrcOptimal            = 6,
  TransferDstOptimal            = 7,
  PresentSrc                    = 1000001002,
  VideoDecodeDst                = 1000024000,
  VideoDecodeSrc                = 1000024001,
  VideoDecodeDpb                = 1000024002,
  VideoEncodeDst                = 1000299000,
  VideoEncodeSrc                = 1000299001,
  EncodeDpb                     = 1000299002
};

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

enum class PipelineStages : u32
{
  None                         = 0x00000000,
  TopOfPipe                    = 0x00000001,
  DrawIndirect                 = 0x00000002,
  VertexInput                  = 0x00000004,
  VertexShader                 = 0x00000008,
  TessellationControlShader    = 0x00000010,
  TessellationEvaluationShader = 0x00000020,
  GeometryShader               = 0x00000040,
  FragmentShader               = 0x00000080,
  EarlyFragmentTests           = 0x00000100,
  LateFragmentTests            = 0x00000200,
  ColorAttachmentOutput        = 0x00000400,
  ComputeShader                = 0x00000800,
  Transfer                     = 0x00001000,
  BottomOfPipe                 = 0x00002000,
  Host                         = 0x00004000,
  AllGraphics                  = 0x00008000,
  AllCommands                  = 0x00010000
};

STX_DEFINE_ENUM_BIT_OPS(PipelineStages)

enum class BufferUsage : u8
{
  None                      = 0x00000000,
  TransferSrc               = 0x00000001,
  TransferDst               = 0x00000002,
  UniformTexelBuffer        = 0x00000004,
  UniformStorageTexelBuffer = 0x00000008,
  UniformBuffer             = 0x00000010,
  StorageBuffer             = 0x00000020,
  IndexBuffer               = 0x00000040,
  VertexBuffer              = 0x00000080
};

STX_DEFINE_ENUM_BIT_OPS(BufferUsage)

enum class ImageUsage : u8
{
  None                   = 0x00000000,
  TransferSrc            = 0x00000001,
  TransferDst            = 0x00000002,
  Sampled                = 0x00000004,
  Storage                = 0x00000008,
  ColorAttachment        = 0x00000010,
  DepthStencilAttachment = 0x00000020,
  InputAttachment        = 0x00000080
};

STX_DEFINE_ENUM_BIT_OPS(ImageUsage)

// USED TO DETERMINE HOW TO INSERT BARRIERS BEFORE AND AFTER MUTATING SCOPES
enum class BufferUsageScope : u32
{
  None                      = 0x00000000,
  TransferSrc               = 0x00000001,
  TransferDst               = 0x00000002,
  ComputeShaderUniform      = 0x00000004,
  ComputeShaderStorage      = 0x00000008,
  ComputeShaderUniformTexel = 0x00000010,
  ComputeShaderStorageTexel = 0x00000020,
  IndexBuffer               = 0x00000040,
  VertexBuffer              = 0x00000080,
  VertexShaderUniform       = 0x00000100,
  FragmentShaderUniform     = 0x00000200,
  All                       = 0xFFFFFFFF
};

STX_DEFINE_ENUM_BIT_OPS(BufferUsageScope)

// BASE LAYOUT IS ALWAYS  SHADER-READ-ONLY???
//
//
//
// used for: -> base layout, layout transitions
// TransferSrc is implicitly supported?
//
// read from color attachment in compute shader
// write from color attachment in compute shader
// read but with transitions in compute  shader
//
// tramsition from base layout to new layout storage|sampled
// transition back to base layout
//
//
// base layout is determined by relative frequency of operation
//
//
//
enum class ImageUsageScope : u32
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
  Present                     = 0x00000800,
  All                         = 0xFFFFFFFF
};

STX_DEFINE_ENUM_BIT_OPS(ImageUsageScope)

constexpr PipelineStages stages_from_usage(BufferUsage usage)
{
}

constexpr PipelineStages stages_from_usage(ImageUsage usage)
{
}

// compute pipelines

enum class InputRate : u8
{
  Vertex   = 0,
  Instance = 1
};

// write access -> wait on all previous reads, transient reads, and transient writes
// transient write access -> wait on all previous reads, writes, transient reads, and transient writes
// transient read access  -> wait on all previous reads, writes, transient reads, and transient writes
// read-only access -> wait only all previous writes, transient reads, and transient writes
//
// buffers have no transient accesses
//
enum class MemoryAccess : u32
{
  None  = 0,
  Read  = 0x00008000,
  Write = 0x00010000
};

STX_DEFINE_ENUM_BIT_OPS(MemoryAccess)

enum class Access : u32
{
  None                        = 0x00000000,
  IndirectCommandRead         = 0x00000001,
  IndexRead                   = 0x00000002,
  VertexAttributeRead         = 0x00000004,
  UniformRead                 = 0x00000008,
  InputAttachmentRead         = 0x00000010,
  ShaderRead                  = 0x00000020,
  ShaderWrite                 = 0x00000040,
  ColorAttachmentRead         = 0x00000080,
  ColorAttachmentWrite        = 0x00000100,
  DepthStencilAttachmentRead  = 0x00000200,
  DepthStencilAttachmentWrite = 0x00000400,
  TransferRead                = 0x00000800,
  TransferWrite               = 0x00001000,
  HostRead                    = 0x00002000,
  HostWrite                   = 0x00004000,
  MemoryRead                  = 0x00008000,
  MemoryWrite                 = 0x00010000
};

STX_DEFINE_ENUM_BIT_OPS(Access)

enum class ShaderStages : u32
{
  None         = 0x00000000,
  Vertex       = 0x00000001,
  Geometry     = 0x00000008,
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

// TODO(lamarrr): aliasing of the same resources in the same descriptor binding group
//
// TODO(lamarrr): write in vertex shader, read in fragment shader, possible pattern? can be synced
// via glsl ordering is non-deterministic
struct BufferDesc
{
  u64              size       = 0;
  MemoryProperties properties = MemoryProperties::None;
  BufferUsage      usage      = BufferUsage::None;
  BufferUsageScope scope      = BufferUsageScope::None;
};

struct BufferViewDesc
{
  Buffer buffer      = nullptr;
  Format view_format = Format::Undefined;
  u64    offset      = 0;
  u64    size        = 0;
};

struct ImageDesc
{
  ImageType       type         = ImageType::Type1D;
  Format          format       = Format::Undefined;
  ImageUsage      usage        = ImageUsage::None;
  ImageUsageScope scope        = ImageUsageScope::None;
  ImageAspects    aspects      = ImageAspects::None;
  Extent3D        extent       = {};
  u32             mips         = 0;
  u32             array_layers = 0;
};

struct ImageViewDesc
{
  Image            image             = nullptr;
  ImageViewType    view_type         = ImageViewType::Type1D;
  Format           view_format       = Format::Undefined;
  ComponentMapping mapping           = ComponentMapping{};
  u32              first_mip_level   = 0;
  u32              num_mip_levels    = 0;
  u32              first_array_layer = 0;
  u32              num_array_layers  = 0;
  ImageAspects     aspects           = ImageAspects::None;
};

struct SamplerDesc
{
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

struct RenderPassAttachment
{
  Format  format           = Format::Undefined;
  LoadOp  load_op          = LoadOp::Load;        // how to use color and depth components
  StoreOp store_op         = StoreOp::Store;
  LoadOp  stencil_load_op  = LoadOp::Load;        // how to use stencil components
  StoreOp stencil_store_op = StoreOp::Store;

  constexpr ImageAccess get_color_image_access() const
  {
    ImageLayout  layout    = ImageLayout::ColorAttachmentOptimal;
    bool         has_write = false;
    bool         has_read  = false;
    MemoryAccess access    = MemoryAccess::None;

    if (load_op == LoadOp::Clear || load_op == LoadOp::DontCare || store_op == StoreOp::Store)
    {
      has_write = true;
    }

    if (load_op == LoadOp::Load)
    {
      has_read = true;
    }

    if (has_write)
    {
      access |= MemoryAccess::Write;
    }

    if (has_read)
    {
      access |= MemoryAccess::Read;
    }

    return ImageAccess{
        .layout = layout, .stages = PipelineStages::ColorAttachmentOutput, .access = access};
  }

  constexpr ImageAccess get_depth_stencil_image_access() const
  {
    ImageLayout  layout    = ImageLayout::Undefined;
    bool         has_write = false;
    bool         has_read  = false;
    MemoryAccess access    = MemoryAccess::None;

    if (load_op == LoadOp::Clear || load_op == LoadOp::DontCare || store_op == StoreOp::Store ||
        store_op == StoreOp::DontCare || stencil_load_op == LoadOp::Clear ||
        stencil_load_op == LoadOp::DontCare || stencil_store_op == StoreOp::Store ||
        stencil_store_op == StoreOp::DontCare)
    {
      has_write = true;
    }

    if (load_op == LoadOp::Load || stencil_load_op == LoadOp::Load)
    {
      has_read = true;
    }

    if (has_write)
    {
      access |= MemoryAccess::Write;
    }

    if (has_read)
    {
      access |= MemoryAccess::Read;
    }

    if (has_write)
    {
      layout = ImageLayout::DepthStencilAttachmentOptimal;
    }
    else
    {
      layout = ImageLayout::DepthStencilReadOnlyOptimal;
    }

    return ImageAccess{.layout = layout,
                       .stages = PipelineStages::EarlyFragmentTests |
                                 PipelineStages::LateFragmentTests |
                                 PipelineStages::ColorAttachmentOutput,
                       .access = access};
  }
};

struct RenderPassDesc
{
  RenderPassAttachment color_attachments[MAX_COLOR_ATTACHMENTS] = {};
  u32                  num_color_attachments                    = 0;
  RenderPassAttachment input_attachments[MAX_COLOR_ATTACHMENTS] = {};
  u32                  num_input_attachments                    = 0;
  RenderPassAttachment depth_stencil_attachment                 = {};
};

struct FramebufferDesc
{
  RenderPass renderpass                               = nullptr;
  Extent     extent                                   = {};
  u32        layers                                   = 0;
  ImageView  color_attachments[MAX_COLOR_ATTACHMENTS] = {};
  u32        num_color_attachments                    = 0;
  ImageView  input_attachments[MAX_COLOR_ATTACHMENTS] = {};
  u32        num_input_attachments                    = 0;
  ImageView  depth_stencil_attachment                 = nullptr;
};

struct DescriptorBindingDesc
{
  u32            binding = 0;
  DescriptorType type    = DescriptorType::Sampler;
  u32            count   = 0;
  ShaderStages   stages  = ShaderStages::None;
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
  ShaderStageDesc     compute_shader        = {};
  DescriptorSetLayout descriptor_set_layout = nullptr;
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
  StencilOpState front_stencil_state      = {};
  StencilOpState back_stencil_state       = {};
  f32            min_depth_bounds         = 0;
  f32            max_depth_bounds         = 0;
};

struct PipelineColorBlendAttachmentState
{
  bool            color_blend_enable     = false;
  BlendFactor     src_color_blend_factor = BlendFactor::Zero;
  BlendFactor     dst_color_blend_factor = BlendFactor::Zero;
  BlendOp         color_blend_op         = BlendOp::Add;
  BlendFactor     src_alpha_blend_factor = BlendFactor::Zero;
  BlendFactor     dst_alpha_blend_factor = BlendFactor::Zero;
  BlendOp         alpha_blend_op         = BlendOp::Add;
  ColorComponents color_outputs          = ColorComponents::None;
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
  bool        color_logic_op_enable      = false;
  LogicOp     color_logic_op             = LogicOp::Clear;
};

struct GraphicsPipelineDesc
{
  ShaderStageDesc                   vertex_shader_stage                          = {};
  ShaderStageDesc                   fragment_shader_stage                        = {};
  RenderPass                        render_pass                                  = nullptr;
  VertexInputBinding                vertex_input_bindings[MAX_VERTEX_ATTRIBUTES] = {};
  u32                               num_vertex_input_bindings                    = 0;
  VertexAttribute                   vertex_attributes[MAX_VERTEX_ATTRIBUTES]     = {};
  u32                               num_vertex_attributes                        = 0;
  DescriptorSetLayout               descriptor_set_layout                        = nullptr;
  PrimitiveTopology                 primitive_topology   = PrimitiveTopology::PointList;
  PipelineRasterizationState        rasterization_state  = {};
  PipelineDepthStencilState         depth_stencil_state  = {};
  Vec4                              color_blend_constant = {};
  PipelineColorBlendAttachmentState color_blend_states[MAX_COLOR_ATTACHMENTS] = {};
  u32                               num_color_attachments                     = 0;
};

struct BufferCopy
{
  u64 src_offset = 0;
  u64 dst_offset = 0;
  u64 size       = 0;
};

struct BufferImageCopy
{
  u64          buffer_offset       = 0;
  u32          buffer_row_length   = 0;
  u32          buffer_image_height = 0;
  URect3D      image_area          = {};
  u32          image_mip_level     = 0;
  u32          first_array_layer   = 0;
  u32          num_array_layers    = 0;
  ImageAspects image_aspects       = ImageAspects::None;
};

struct ImageCopy
{
  URect3D      src_area              = {};
  u32          src_mip_level         = 0;
  u32          src_first_array_layer = 0;
  u32          src_num_array_layers  = 0;
  ImageAspects src_aspects           = ImageAspects::None;
  Offset3D     dst_offset            = {};
  u32          dst_mip_level         = 0;
  u32          dst_first_array_layer = 0;
  u32          dst_num_array_layers  = 0;
  ImageAspects dst_aspects           = ImageAspects::None;
};

struct ImageBlit
{
  URect3D      src_area              = {};
  u32          src_mip_level         = 0;
  u32          src_first_array_layer = 0;
  u32          src_num_array_layers  = 0;
  ImageAspects src_aspects           = ImageAspects::None;
  URect3D      dst_area              = {};
  u32          dst_mip_level         = 0;
  u32          dst_first_array_layer = 0;
  u32          dst_num_array_layers  = 0;
  ImageAspects dst_aspects           = ImageAspects::None;
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

struct QueueBufferMemoryBarrier
{
  Buffer         buffer     = nullptr;
  u64            offset     = 0;
  u64            size       = 0;
  PipelineStages src_stages = PipelineStages::None;
  PipelineStages dst_stages = PipelineStages::None;
  Access         src_access = Access::None;
  Access         dst_access = Access::None;
};

struct QueueImageMemoryBarrier
{
  Image          image             = nullptr;
  u32            first_mip_level   = 0;
  u32            num_mip_levels    = 0;
  u32            first_array_layer = 0;
  u32            num_array_layers  = 0;
  ImageAspects   aspects           = ImageAspects::None;
  ImageLayout    old_layout        = ImageLayout::Undefined;
  ImageLayout    new_layout        = ImageLayout::Undefined;
  PipelineStages src_stages        = PipelineStages::None;
  PipelineStages dst_stages        = PipelineStages::None;
  Access         src_access        = Access::None;
  Access         dst_access        = Access::None;
};

struct StencilFaceState
{
  u32 compare_mask = 0;
  u32 reference    = 0;
  u32 write_mask   = 0;
};

struct RenderState
{
  Viewport         viewport;
  IRect            scissor;
  Vec4             blend_constants;
  StencilFaceState front_stencil;
  StencilFaceState back_stencil;
};

// RESOURCES hold the backend/RHI handles

struct BufferResource
{
  BufferDesc desc;
  Buffer     handle = nullptr;
};

struct BufferViewResource
{
  BufferViewDesc desc;
  BufferView     handle = nullptr;
};

struct ImageResource
{
  ImageDesc desc;
  Image     handle = nullptr;
};

struct ImageViewResource
{
  ImageViewDesc desc;
  ImageView     handle = nullptr;
};

struct RenderPassResource
{
  RenderPassDesc desc;
  RenderPass     handle = nullptr;
};

struct FramebufferResource
{
  FramebufferDesc desc;
  Framebuffer     handle = nullptr;
};

struct ShaderResource
{
  Shader handle = nullptr;
};

struct ComputePipelineResource
{
  ComputePipeline handle = nullptr;
};

struct GraphicsPipelineResource
{
  GraphicsPipeline handle = nullptr;
};

struct SamplerResource
{
  Sampler handle = nullptr;
};

struct DescriptorSetLayoutResource
{
  DescriptorSetLayout handle = nullptr;
};

struct CommandBufferResource
{
  CommandBuffer handle = nullptr;
};

}        // namespace gfx
}        // namespace ash
         // NOTE: renderpass attachments MUST not be accessed in shaders within that renderpass