#pragma once
#include <string_view>
#include "ashura/primitives.h"
#include "ashura/utils.h"
#include "stx/fn.h"
#include "stx/vec.h"
// #include "Volk/volk.h" TODO(lamarrr): use Volk to avoid table dispatch overhead https://gpuopen.com/learn/reducing-vulkan-api-call-overhead/

namespace ash
{
namespace lgfx
{

enum class Buffer : u64
{
  None = 0
};

enum class Image : u64
{
  None = 0
};

/// a sub-resource that specifies regions, mips, aspects, and layer of images
enum class ImageView : u64
{
  None = 0
};

enum class RenderPass : u64
{
  None = 0
};

enum class Framebuffer : u64
{
  None = 0
};

enum class Resource : u64
{
  None = 0
};

// HIGH-LEVEL RENDER & COMPUTE PIPELINE COMPONENTS (ABSTRACTION)
// EFFECTS & POST-PROCESSING
// MESH MANAGEMENT
// MESH BATCHING & INSTANCING
// MATERIAL MANAGEMENT
// RESOURCE MANAGEMENT
// CAMERA MANAGEMENT
// LIGHT MANAGEMENT
// SCENE GRAPH (SORTING, CULLING)

// MID-LEVEL RENDER & COMPUTE PIPELINE COMPONENTS
// RESOURCE SYNCHRONIZATION & MANAGEMENT (I.E. BARRIERS)
// TASK GRAPHS

// LOW-LEVEL RENDER & COMPUTE PIPELINE COMPONENTS (PLATFORM-SPECIFIC)
// RENDER PASSES
// COMPUTE PASSES
// PIPELINES
// SHADERS
// PSO & PSO CACHES

/**
 * HANDLES:
 * - Resource state tracking and transition (barriers)
 * - Resource creation, recreation, and management
 */

enum class MemoryProperties : u32
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

struct HeapProperty
{
  // properties is either of:
  //
  // HostVisible | HostCoherent
  // HostVisible | HostCached
  // HostVisible | HostCached | HostCoherent
  // DeviceLocal
  // DeviceLocal | HostVisible | HostCoherent
  // DeviceLocal | HostVisible | HostCached
  // DeviceLocal | HostVisible | HostCached | HostCoherent
  MemoryProperties properties = MemoryProperties::None;
  u32              index      = 0;
};

// a single heap might have multiple properties
struct DeviceMemoryHeaps
{
  static constexpr u8 MAX_HEAP_PROPERTIES = 32;
  static constexpr u8 MAX_HEAPS           = 16;

  // ordered by performance-tier (MemoryProperties)
  HeapProperty heap_properties[MAX_HEAP_PROPERTIES];
  u32          num_properties = 0;
  u64          heap_sizes[MAX_HEAPS];
  u32          num_heaps = 0;

  constexpr bool has_memory(MemoryProperties properties) const
  {
    for (u32 i = 0; i < num_properties; i++)
    {
      if ((heap_properties[i].properties & properties) == properties)
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

// TODO(lamarrr): write memory allocation strategies, i.e. images should be allocated on this and this heap

enum class Format : u32
{
  Undefined                                      = 0,
  R4G4_UNORM_PACK8                               = 1,
  R4G4B4A4_UNORM_PACK16                          = 2,
  B4G4R4A4_UNORM_PACK16                          = 3,
  R5G6B5_UNORM_PACK16                            = 4,
  B5G6R5_UNORM_PACK16                            = 5,
  R5G5B5A1_UNORM_PACK16                          = 6,
  B5G5R5A1_UNORM_PACK16                          = 7,
  A1R5G5B5_UNORM_PACK16                          = 8,
  R8_UNORM                                       = 9,
  R8_SNORM                                       = 10,
  R8_USCALED                                     = 11,
  R8_SSCALED                                     = 12,
  R8_UINT                                        = 13,
  R8_SINT                                        = 14,
  R8_SRGB                                        = 15,
  R8G8_UNORM                                     = 16,
  R8G8_SNORM                                     = 17,
  R8G8_USCALED                                   = 18,
  R8G8_SSCALED                                   = 19,
  R8G8_UINT                                      = 20,
  R8G8_SINT                                      = 21,
  R8G8_SRGB                                      = 22,
  R8G8B8_UNORM                                   = 23,
  R8G8B8_SNORM                                   = 24,
  R8G8B8_USCALED                                 = 25,
  R8G8B8_SSCALED                                 = 26,
  R8G8B8_UINT                                    = 27,
  R8G8B8_SINT                                    = 28,
  R8G8B8_SRGB                                    = 29,
  B8G8R8_UNORM                                   = 30,
  B8G8R8_SNORM                                   = 31,
  B8G8R8_USCALED                                 = 32,
  B8G8R8_SSCALED                                 = 33,
  B8G8R8_UINT                                    = 34,
  B8G8R8_SINT                                    = 35,
  B8G8R8_SRGB                                    = 36,
  R8G8B8A8_UNORM                                 = 37,
  R8G8B8A8_SNORM                                 = 38,
  R8G8B8A8_USCALED                               = 39,
  R8G8B8A8_SSCALED                               = 40,
  R8G8B8A8_UINT                                  = 41,
  R8G8B8A8_SINT                                  = 42,
  R8G8B8A8_SRGB                                  = 43,
  B8G8R8A8_UNORM                                 = 44,
  B8G8R8A8_SNORM                                 = 45,
  B8G8R8A8_USCALED                               = 46,
  B8G8R8A8_SSCALED                               = 47,
  B8G8R8A8_UINT                                  = 48,
  B8G8R8A8_SINT                                  = 49,
  B8G8R8A8_SRGB                                  = 50,
  A8B8G8R8_UNORM_PACK32                          = 51,
  A8B8G8R8_SNORM_PACK32                          = 52,
  A8B8G8R8_USCALED_PACK32                        = 53,
  A8B8G8R8_SSCALED_PACK32                        = 54,
  A8B8G8R8_UINT_PACK32                           = 55,
  A8B8G8R8_SINT_PACK32                           = 56,
  A8B8G8R8_SRGB_PACK32                           = 57,
  A2R10G10B10_UNORM_PACK32                       = 58,
  A2R10G10B10_SNORM_PACK32                       = 59,
  A2R10G10B10_USCALED_PACK32                     = 60,
  A2R10G10B10_SSCALED_PACK32                     = 61,
  A2R10G10B10_UINT_PACK32                        = 62,
  A2R10G10B10_SINT_PACK32                        = 63,
  A2B10G10R10_UNORM_PACK32                       = 64,
  A2B10G10R10_SNORM_PACK32                       = 65,
  A2B10G10R10_USCALED_PACK32                     = 66,
  A2B10G10R10_SSCALED_PACK32                     = 67,
  A2B10G10R10_UINT_PACK32                        = 68,
  A2B10G10R10_SINT_PACK32                        = 69,
  R16_UNORM                                      = 70,
  R16_SNORM                                      = 71,
  R16_USCALED                                    = 72,
  R16_SSCALED                                    = 73,
  R16_UINT                                       = 74,
  R16_SINT                                       = 75,
  R16_SFLOAT                                     = 76,
  R16G16_UNORM                                   = 77,
  R16G16_SNORM                                   = 78,
  R16G16_USCALED                                 = 79,
  R16G16_SSCALED                                 = 80,
  R16G16_UINT                                    = 81,
  R16G16_SINT                                    = 82,
  R16G16_SFLOAT                                  = 83,
  R16G16B16_UNORM                                = 84,
  R16G16B16_SNORM                                = 85,
  R16G16B16_USCALED                              = 86,
  R16G16B16_SSCALED                              = 87,
  R16G16B16_UINT                                 = 88,
  R16G16B16_SINT                                 = 89,
  R16G16B16_SFLOAT                               = 90,
  R16G16B16A16_UNORM                             = 91,
  R16G16B16A16_SNORM                             = 92,
  R16G16B16A16_USCALED                           = 93,
  R16G16B16A16_SSCALED                           = 94,
  R16G16B16A16_UINT                              = 95,
  R16G16B16A16_SINT                              = 96,
  R16G16B16A16_SFLOAT                            = 97,
  R32_UINT                                       = 98,
  R32_SINT                                       = 99,
  R32_SFLOAT                                     = 100,
  R32G32_UINT                                    = 101,
  R32G32_SINT                                    = 102,
  R32G32_SFLOAT                                  = 103,
  R32G32B32_UINT                                 = 104,
  R32G32B32_SINT                                 = 105,
  R32G32B32_SFLOAT                               = 106,
  R32G32B32A32_UINT                              = 107,
  R32G32B32A32_SINT                              = 108,
  R32G32B32A32_SFLOAT                            = 109,
  R64_UINT                                       = 110,
  R64_SINT                                       = 111,
  R64_SFLOAT                                     = 112,
  R64G64_UINT                                    = 113,
  R64G64_SINT                                    = 114,
  R64G64_SFLOAT                                  = 115,
  R64G64B64_UINT                                 = 116,
  R64G64B64_SINT                                 = 117,
  R64G64B64_SFLOAT                               = 118,
  R64G64B64A64_UINT                              = 119,
  R64G64B64A64_SINT                              = 120,
  R64G64B64A64_SFLOAT                            = 121,
  B10G11R11_UFLOAT_PACK32                        = 122,
  E5B9G9R9_UFLOAT_PACK32                         = 123,
  D16_UNORM                                      = 124,
  X8_D24_UNORM_PACK32                            = 125,
  D32_SFLOAT                                     = 126,
  S8_UINT                                        = 127,
  D16_UNORM_S8_UINT                              = 128,
  D24_UNORM_S8_UINT                              = 129,
  D32_SFLOAT_S8_UINT                             = 130,
  BC1_RGB_UNORM_BLOCK                            = 131,
  BC1_RGB_SRGB_BLOCK                             = 132,
  BC1_RGBA_UNORM_BLOCK                           = 133,
  BC1_RGBA_SRGB_BLOCK                            = 134,
  BC2_UNORM_BLOCK                                = 135,
  BC2_SRGB_BLOCK                                 = 136,
  BC3_UNORM_BLOCK                                = 137,
  BC3_SRGB_BLOCK                                 = 138,
  BC4_UNORM_BLOCK                                = 139,
  BC4_SNORM_BLOCK                                = 140,
  BC5_UNORM_BLOCK                                = 141,
  BC5_SNORM_BLOCK                                = 142,
  BC6H_UFLOAT_BLOCK                              = 143,
  BC6H_SFLOAT_BLOCK                              = 144,
  BC7_UNORM_BLOCK                                = 145,
  BC7_SRGB_BLOCK                                 = 146,
  ETC2_R8G8B8_UNORM_BLOCK                        = 147,
  ETC2_R8G8B8_SRGB_BLOCK                         = 148,
  ETC2_R8G8B8A1_UNORM_BLOCK                      = 149,
  ETC2_R8G8B8A1_SRGB_BLOCK                       = 150,
  ETC2_R8G8B8A8_UNORM_BLOCK                      = 151,
  ETC2_R8G8B8A8_SRGB_BLOCK                       = 152,
  EAC_R11_UNORM_BLOCK                            = 153,
  EAC_R11_SNORM_BLOCK                            = 154,
  EAC_R11G11_UNORM_BLOCK                         = 155,
  EAC_R11G11_SNORM_BLOCK                         = 156,
  ASTC_4x4_UNORM_BLOCK                           = 157,
  ASTC_4x4_SRGB_BLOCK                            = 158,
  ASTC_5x4_UNORM_BLOCK                           = 159,
  ASTC_5x4_SRGB_BLOCK                            = 160,
  ASTC_5x5_UNORM_BLOCK                           = 161,
  ASTC_5x5_SRGB_BLOCK                            = 162,
  ASTC_6x5_UNORM_BLOCK                           = 163,
  ASTC_6x5_SRGB_BLOCK                            = 164,
  ASTC_6x6_UNORM_BLOCK                           = 165,
  ASTC_6x6_SRGB_BLOCK                            = 166,
  ASTC_8x5_UNORM_BLOCK                           = 167,
  ASTC_8x5_SRGB_BLOCK                            = 168,
  ASTC_8x6_UNORM_BLOCK                           = 169,
  ASTC_8x6_SRGB_BLOCK                            = 170,
  ASTC_8x8_UNORM_BLOCK                           = 171,
  ASTC_8x8_SRGB_BLOCK                            = 172,
  ASTC_10x5_UNORM_BLOCK                          = 173,
  ASTC_10x5_SRGB_BLOCK                           = 174,
  ASTC_10x6_UNORM_BLOCK                          = 175,
  ASTC_10x6_SRGB_BLOCK                           = 176,
  ASTC_10x8_UNORM_BLOCK                          = 177,
  ASTC_10x8_SRGB_BLOCK                           = 178,
  ASTC_10x10_UNORM_BLOCK                         = 179,
  ASTC_10x10_SRGB_BLOCK                          = 180,
  ASTC_12x10_UNORM_BLOCK                         = 181,
  ASTC_12x10_SRGB_BLOCK                          = 182,
  ASTC_12x12_UNORM_BLOCK                         = 183,
  ASTC_12x12_SRGB_BLOCK                          = 184,
  G8B8G8R8_422_UNORM                             = 1000156000,
  B8G8R8G8_422_UNORM                             = 1000156001,
  G8_B8_R8_3PLANE_420_UNORM                      = 1000156002,
  G8_B8R8_2PLANE_420_UNORM                       = 1000156003,
  G8_B8_R8_3PLANE_422_UNORM                      = 1000156004,
  G8_B8R8_2PLANE_422_UNORM                       = 1000156005,
  G8_B8_R8_3PLANE_444_UNORM                      = 1000156006,
  R10X6_UNORM_PACK16                             = 1000156007,
  R10X6G10X6_UNORM_2PACK16                       = 1000156008,
  R10X6G10X6B10X6A10X6_UNORM_4PACK16             = 1000156009,
  G10X6B10X6G10X6R10X6_422_UNORM_4PACK16         = 1000156010,
  B10X6G10X6R10X6G10X6_422_UNORM_4PACK16         = 1000156011,
  G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16     = 1000156012,
  G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16      = 1000156013,
  G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16     = 1000156014,
  G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16      = 1000156015,
  G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16     = 1000156016,
  R12X4_UNORM_PACK16                             = 1000156017,
  R12X4G12X4_UNORM_2PACK16                       = 1000156018,
  R12X4G12X4B12X4A12X4_UNORM_4PACK16             = 1000156019,
  G12X4B12X4G12X4R12X4_422_UNORM_4PACK16         = 1000156020,
  B12X4G12X4R12X4G12X4_422_UNORM_4PACK16         = 1000156021,
  G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16     = 1000156022,
  G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16      = 1000156023,
  G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16     = 1000156024,
  G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16      = 1000156025,
  G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16     = 1000156026,
  G16B16G16R16_422_UNORM                         = 1000156027,
  B16G16R16G16_422_UNORM                         = 1000156028,
  G16_B16_R16_3PLANE_420_UNORM                   = 1000156029,
  G16_B16R16_2PLANE_420_UNORM                    = 1000156030,
  G16_B16_R16_3PLANE_422_UNORM                   = 1000156031,
  G16_B16R16_2PLANE_422_UNORM                    = 1000156032,
  G16_B16_R16_3PLANE_444_UNORM                   = 1000156033,
  G8_B8R8_2PLANE_444_UNORM                       = 1000330000,
  G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16      = 1000330001,
  G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16      = 1000330002,
  G16_B16R16_2PLANE_444_UNORM                    = 1000330003,
  A4R4G4B4_UNORM_PACK16                          = 1000340000,
  A4B4G4R4_UNORM_PACK16                          = 1000340001,
  ASTC_4x4_SFLOAT_BLOCK                          = 1000066000,
  ASTC_5x4_SFLOAT_BLOCK                          = 1000066001,
  ASTC_5x5_SFLOAT_BLOCK                          = 1000066002,
  ASTC_6x5_SFLOAT_BLOCK                          = 1000066003,
  ASTC_6x6_SFLOAT_BLOCK                          = 1000066004,
  ASTC_8x5_SFLOAT_BLOCK                          = 1000066005,
  ASTC_8x6_SFLOAT_BLOCK                          = 1000066006,
  ASTC_8x8_SFLOAT_BLOCK                          = 1000066007,
  ASTC_10x5_SFLOAT_BLOCK                         = 1000066008,
  ASTC_10x6_SFLOAT_BLOCK                         = 1000066009,
  ASTC_10x8_SFLOAT_BLOCK                         = 1000066010,
  ASTC_10x10_SFLOAT_BLOCK                        = 1000066011,
  ASTC_12x10_SFLOAT_BLOCK                        = 1000066012,
  ASTC_12x12_SFLOAT_BLOCK                        = 1000066013,
  PVRTC1_2BPP_UNORM_BLOCK_IMG                    = 1000054000,
  PVRTC1_4BPP_UNORM_BLOCK_IMG                    = 1000054001,
  PVRTC2_2BPP_UNORM_BLOCK_IMG                    = 1000054002,
  PVRTC2_4BPP_UNORM_BLOCK_IMG                    = 1000054003,
  PVRTC1_2BPP_SRGB_BLOCK_IMG                     = 1000054004,
  PVRTC1_4BPP_SRGB_BLOCK_IMG                     = 1000054005,
  PVRTC2_2BPP_SRGB_BLOCK_IMG                     = 1000054006,
  PVRTC2_4BPP_SRGB_BLOCK_IMG                     = 1000054007,
  R16G16_S10_5_NV                                = 1000464000,
  A1B5G5R5_UNORM_PACK16_KHR                      = 1000470000,
  A8_UNORM_KHR                                   = 1000470001,
  ASTC_4x4_SFLOAT_BLOCK_EXT                      = ASTC_4x4_SFLOAT_BLOCK,
  ASTC_5x4_SFLOAT_BLOCK_EXT                      = ASTC_5x4_SFLOAT_BLOCK,
  ASTC_5x5_SFLOAT_BLOCK_EXT                      = ASTC_5x5_SFLOAT_BLOCK,
  ASTC_6x5_SFLOAT_BLOCK_EXT                      = ASTC_6x5_SFLOAT_BLOCK,
  ASTC_6x6_SFLOAT_BLOCK_EXT                      = ASTC_6x6_SFLOAT_BLOCK,
  ASTC_8x5_SFLOAT_BLOCK_EXT                      = ASTC_8x5_SFLOAT_BLOCK,
  ASTC_8x6_SFLOAT_BLOCK_EXT                      = ASTC_8x6_SFLOAT_BLOCK,
  ASTC_8x8_SFLOAT_BLOCK_EXT                      = ASTC_8x8_SFLOAT_BLOCK,
  ASTC_10x5_SFLOAT_BLOCK_EXT                     = ASTC_10x5_SFLOAT_BLOCK,
  ASTC_10x6_SFLOAT_BLOCK_EXT                     = ASTC_10x6_SFLOAT_BLOCK,
  ASTC_10x8_SFLOAT_BLOCK_EXT                     = ASTC_10x8_SFLOAT_BLOCK,
  ASTC_10x10_SFLOAT_BLOCK_EXT                    = ASTC_10x10_SFLOAT_BLOCK,
  ASTC_12x10_SFLOAT_BLOCK_EXT                    = ASTC_12x10_SFLOAT_BLOCK,
  ASTC_12x12_SFLOAT_BLOCK_EXT                    = ASTC_12x12_SFLOAT_BLOCK,
  G8B8G8R8_422_UNORM_KHR                         = G8B8G8R8_422_UNORM,
  B8G8R8G8_422_UNORM_KHR                         = B8G8R8G8_422_UNORM,
  G8_B8_R8_3PLANE_420_UNORM_KHR                  = G8_B8_R8_3PLANE_420_UNORM,
  G8_B8R8_2PLANE_420_UNORM_KHR                   = G8_B8R8_2PLANE_420_UNORM,
  G8_B8_R8_3PLANE_422_UNORM_KHR                  = G8_B8_R8_3PLANE_422_UNORM,
  G8_B8R8_2PLANE_422_UNORM_KHR                   = G8_B8R8_2PLANE_422_UNORM,
  G8_B8_R8_3PLANE_444_UNORM_KHR                  = G8_B8_R8_3PLANE_444_UNORM,
  R10X6_UNORM_PACK16_KHR                         = R10X6_UNORM_PACK16,
  R10X6G10X6_UNORM_2PACK16_KHR                   = R10X6G10X6_UNORM_2PACK16,
  R10X6G10X6B10X6A10X6_UNORM_4PACK16_KHR         = R10X6G10X6B10X6A10X6_UNORM_4PACK16,
  G10X6B10X6G10X6R10X6_422_UNORM_4PACK16_KHR     = G10X6B10X6G10X6R10X6_422_UNORM_4PACK16,
  B10X6G10X6R10X6G10X6_422_UNORM_4PACK16_KHR     = B10X6G10X6R10X6G10X6_422_UNORM_4PACK16,
  G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16_KHR = G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16,
  G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16_KHR  = G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16,
  G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16_KHR = G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16,
  G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16_KHR  = G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16,
  G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16_KHR = G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16,
  R12X4_UNORM_PACK16_KHR                         = R12X4_UNORM_PACK16,
  R12X4G12X4_UNORM_2PACK16_KHR                   = R12X4G12X4_UNORM_2PACK16,
  R12X4G12X4B12X4A12X4_UNORM_4PACK16_KHR         = R12X4G12X4B12X4A12X4_UNORM_4PACK16,
  G12X4B12X4G12X4R12X4_422_UNORM_4PACK16_KHR     = G12X4B12X4G12X4R12X4_422_UNORM_4PACK16,
  B12X4G12X4R12X4G12X4_422_UNORM_4PACK16_KHR     = B12X4G12X4R12X4G12X4_422_UNORM_4PACK16,
  G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16_KHR = G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16,
  G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16_KHR  = G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16,
  G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16_KHR = G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16,
  G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16_KHR  = G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16,
  G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16_KHR = G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16,
  G16B16G16R16_422_UNORM_KHR                     = G16B16G16R16_422_UNORM,
  B16G16R16G16_422_UNORM_KHR                     = B16G16R16G16_422_UNORM,
  G16_B16_R16_3PLANE_420_UNORM_KHR               = G16_B16_R16_3PLANE_420_UNORM,
  G16_B16R16_2PLANE_420_UNORM_KHR                = G16_B16R16_2PLANE_420_UNORM,
  G16_B16_R16_3PLANE_422_UNORM_KHR               = G16_B16_R16_3PLANE_422_UNORM,
  G16_B16R16_2PLANE_422_UNORM_KHR                = G16_B16R16_2PLANE_422_UNORM,
  G16_B16_R16_3PLANE_444_UNORM_KHR               = G16_B16_R16_3PLANE_444_UNORM,
  G8_B8R8_2PLANE_444_UNORM_EXT                   = G8_B8R8_2PLANE_444_UNORM,
  G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16_EXT  = G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16,
  G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16_EXT  = G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16,
  G16_B16R16_2PLANE_444_UNORM_EXT                = G16_B16R16_2PLANE_444_UNORM,
  A4R4G4B4_UNORM_PACK16_EXT                      = A4R4G4B4_UNORM_PACK16,
  A4B4G4R4_UNORM_PACK16_EXT                      = A4B4G4R4_UNORM_PACK16,
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

struct FormatProperties
{
  FormatFeatures linear_tiling_features  = FormatFeatures::None;
  FormatFeatures optimal_tiling_features = FormatFeatures::None;
  FormatFeatures buffer_features         = FormatFeatures::None;
};

struct DeviceInfo
{
  DeviceMemoryHeaps memory_heaps;
  // formats info properties
};

enum class ImageAspect : u32
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

STX_DEFINE_ENUM_BIT_OPS(ImageAspect)

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
  PresentSource                 = 1000001002
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

enum class PipelineStages : u64
{
  None                    = 0x00000000ULL,
  TopOfPipe               = 0x00000001ULL,
  VertexShader            = 0x00000008ULL,
  FragmentShader          = 0x00000080ULL,
  EarlyFragmentTests      = 0x00000100ULL,
  LateFragmentTests       = 0x00000200ULL,
  ColorAttachmentOutput   = 0x00000400ULL,
  ComputeShader           = 0x00000800ULL,
  Transfer                = 0x00001000ULL,
  BottomOfPipe            = 0x00002000ULL,
  Host                    = 0x00004000ULL,
  AllGraphics             = 0x00008000ULL,
  AllCommands             = 0x00010000ULL,
  Copy                    = 0x100000000ULL,
  Resolve                 = 0x200000000ULL,
  Blit                    = 0x400000000ULL,
  Clear                   = 0x800000000ULL,
  IndexInput              = 0x1000000000ULL,
  VertexAttributeInput    = 0x2000000000ULL,
  PreRasterizationShaders = 0x4000000000ULL,
  RayTracingShader        = 0x00200000ULL,
  VideoDecode             = 0x04000000ULL,
  VideoEncode             = 0x08000000ULL
};

STX_DEFINE_ENUM_BIT_OPS(PipelineStages)

enum class BufferUsages : u32
{
  Undefined                 = 0x00000000,
  TransferSrc               = 0x00000001,
  TransferDst               = 0x00000002,
  UniformTexelBuffer        = 0x00000004,
  UniformStorageTexelBuffer = 0x00000008,
  UniformBuffer             = 0x00000010,
  StorageBuffer             = 0x00000020,
  IndexBuffer               = 0x00000040,
  VertexBuffer              = 0x00000080
};

STX_DEFINE_ENUM_BIT_OPS(BufferUsages)

enum class ImageUsages : u32
{
  Undefined              = 0x00000000,
  TransferSrc            = 0x00000001,
  TransferDst            = 0x00000002,
  Sampled                = 0x00000004,
  Storage                = 0x00000008,
  ColorAttachment        = 0x00000010,
  DepthStencilAttachment = 0x00000020
};

STX_DEFINE_ENUM_BIT_OPS(ImageUsages)

enum class ResourceType : u8
{
  None        = 0,
  Image       = 1,
  ImageView   = 2,
  Buffer      = 3,
  RenderPass  = 4,
  Framebuffer = 5
};

enum class Access : u64
{
  None                               = 0x00000000ULL,
  IndirectCommandRead                = 0x00000001ULL,
  IndexRead                          = 0x00000002ULL,
  VertexAttributeRead                = 0x00000004ULL,
  UniformRead                        = 0x00000008ULL,
  InputAttachmentRead                = 0x00000010ULL,
  ShaderRead                         = 0x00000020ULL,
  ShaderWrite                        = 0x00000040ULL,
  ColorAttachmentRead                = 0x00000080ULL,
  ColorAttachmentWrite               = 0x00000100ULL,
  DepthStencilAttachmentRead         = 0x00000200ULL,
  DepthStencilAttachmentWrite        = 0x00000400ULL,
  TransferRead                       = 0x00000800ULL,
  TransferWrite                      = 0x00001000ULL,
  HostRead                           = 0x00002000ULL,
  HostWrite                          = 0x00004000ULL,
  MemoryRead                         = 0x00008000ULL,
  MemoryWrite                        = 0x00010000ULL,
  ShaderSampledRead                  = 0x100000000ULL,
  ShaderSampledWrite                 = 0x200000000ULL,
  ShaderStorageWrite                 = 0x400000000ULL,
  VideoDecodeRead_KHR                = 0x800000000ULL,
  VideoDecodeWrite_KHR               = 0x1000000000ULL,
  VideoEncodeRead_KHR                = 0x2000000000ULL,
  VideoEncodeWrite_KHR               = 0x4000000000ULL,
  AccelerationStructureRead_KHR      = 0x00200000ULL,
  AccelerationStructureWrite_KHR     = 0x00400000ULL,
  AccelerationStructureRead_NV       = 0x00200000ULL,
  AccelerationStructureWrite_NV      = 0x00400000ULL,
  FragmentDensityMapRead_EXT         = 0x01000000ULL,
  ColorAttachmentReadNonCoherent_EXT = 0x00080000ULL,
  DescriptorBufferRead_EXT           = 0x20000000000ULL,
  ShaderBindingTableRead_KHR         = 0x10000000000ULL
};

STX_DEFINE_ENUM_BIT_OPS(Access)

struct ComponentMapping
{
  ComponentSwizzle r = ComponentSwizzle::Identity;
  ComponentSwizzle g = ComponentSwizzle::Identity;
  ComponentSwizzle b = ComponentSwizzle::Identity;
  ComponentSwizzle a = ComponentSwizzle::Identity;

  static constexpr ComponentMapping identity()
  {
    return ComponentMapping{
        .r = ComponentSwizzle::Identity,
        .g = ComponentSwizzle::Identity,
        .b = ComponentSwizzle::Identity,
        .a = ComponentSwizzle::Identity};
  }
};

enum class PipelineType : u8
{
  Graphics      = 0,
  Compute       = 1,
  VideoDecoding = 2,
  VideoEncoding = 3,
  RayTracing    = 4
};

// TODO(lamarrr): since we are performing transfers on the same queue family, we can just insert upload barriers instead of waiting on uploads to finish??
// but since we are writing to a possibly in-use memory, we will need to sync up or check if an upload is already in progress with the buffer?
// or use vulkan events to let us know when uploads are in progress and when they are done

// A command queue can't execute multiple commands at once, but it can reorder the commands given to it if no synchronization is put in place

// Action and synchronization commands recorded to a command buffer execute the
// VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT pipeline stage in submission order - forming an implicit
// execution dependency between this stage in each command.
//
// each command executes VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT in the order they were submitted, this is the only execution order guarantee on the queue
//
// https://github.com/KhronosGroup/Vulkan-Docs/issues/552
//
//
// A Queue is an out-of-order execution unit that executes a specific set of tasks
//
//
// TODO(lamarrr): each task might have different synchronization tasks to perform
//
//
// we need to keep track of:
// where it was last used, what it was last used for and how
// where it will next be used at, what it will next be used for
//
//
//
struct BufferBinding
{
  Buffer         buffer = Buffer::None;
  Access         access = Access::None;
  PipelineStages stages = PipelineStages::None;
};

struct ImageViewBinding
{
  ImageView      image_view = ImageView::None;
  Access         access     = Access::None;
  PipelineStages stages     = PipelineStages::None;
};

enum class ResourceBindingType : u8
{
  BufferBinding,
  ImageViewBinding
};

struct ResourceBinding
{
  constexpr ResourceBinding(BufferBinding resource) :
      buffer{resource}, type{ResourceBindingType::BufferBinding}
  {}
  constexpr ResourceBinding(ImageViewBinding resource) :
      image_view{resource}, type{ResourceBindingType::ImageViewBinding}
  {}

  union
  {
    BufferBinding    buffer;
    ImageViewBinding image_view;
  };
  ResourceBindingType type;
};

struct QueueMemoryBarrier
{
  PipelineStages src_stage_mask  = PipelineStages::None;
  PipelineStages dst_stage_mask  = PipelineStages::None;
  Access         src_access_mask = Access::None;
  Access         dst_access_mask = Access::None;
};

struct QueueBufferMemoryBarrier
{
  Buffer         buffer          = Buffer::None;
  u64            offset          = 0;
  u64            size            = 0;
  PipelineStages src_stage_mask  = PipelineStages::None;
  PipelineStages dst_stage_mask  = PipelineStages::None;
  Access         src_access_mask = Access::None;
  Access         dst_access_mask = Access::None;
};

struct QueueImageMemoryBarrier
{
  Image          image           = Image::None;
  u32            first_mip_level = 0;
  u32            num_mip_levels  = 0;
  ImageAspect    aspect          = ImageAspect::None;
  ImageLayout    old_layout      = ImageLayout::Undefined;
  ImageLayout    new_layout      = ImageLayout::Undefined;
  PipelineStages src_stage_mask  = PipelineStages::None;
  PipelineStages dst_stage_mask  = PipelineStages::None;
  Access         src_access_mask = Access::None;
  Access         dst_access_mask = Access::None;
};

enum class QueueBarrierType : u8
{
  None         = 0,
  Memory       = 1,
  BufferMemory = 2,
  ImageMemory  = 3
};

struct QueueBarrier
{
  constexpr QueueBarrier() :
      type{QueueBarrierType::None}
  {}
  constexpr QueueBarrier(QueueMemoryBarrier const &barrier) :
      memory{barrier}, type{QueueBarrierType::Memory}
  {}
  constexpr QueueBarrier(QueueBufferMemoryBarrier const &barrier) :
      buffer{barrier}, type{QueueBarrierType::BufferMemory}
  {}
  constexpr QueueBarrier(QueueImageMemoryBarrier const &barrier) :
      image{barrier}, type{QueueBarrierType::ImageMemory}
  {}

  union
  {
    QueueMemoryBarrier       memory;
    QueueBufferMemoryBarrier buffer;
    QueueImageMemoryBarrier  image;
  };
  QueueBarrierType type;
};

struct MemoryState
{
  PipelineStages stage       = PipelineStages::None;
  Access         access_mask = Access::None;
};

struct BufferState
{
  PipelineStages stage       = PipelineStages::None;
  Access         access_mask = Access::None;
};

struct ImageState
{
  PipelineStages stage       = PipelineStages::None;
  Access         access_mask = Access::None;
  ImageLayout    layout      = ImageLayout::Undefined;
};

enum class ResourceStateType : u8
{
  MemoryState,
  BufferState,
  ImageState
};

struct ResourceState
{
  constexpr ResourceState(MemoryState const &state) :
      memory{state}, type{ResourceStateType::MemoryState}
  {}
  constexpr ResourceState(BufferState const &state) :
      buffer{state}, type{ResourceStateType::BufferState}
  {}
  constexpr ResourceState(ImageState const &state) :
      image{state}, type{ResourceStateType::ImageState}
  {}

  union
  {
    MemoryState memory;
    BufferState buffer;
    ImageState  image;
  };
  ResourceStateType type;
};

struct BufferDesc
{
  std::string_view pass       = "undefined";
  std::string_view name       = "undefined";
  u64              size       = 0;
  MemoryProperties properties = MemoryProperties::None;
  BufferUsages     usages     = BufferUsages::Undefined;
};

struct ImageDesc
{
  std::string_view pass   = "undefined";
  std::string_view name   = "undefined";
  Format           format = Format::R8_UNORM;
  ImageUsages      usages = ImageUsages::Sampled;
  Extent           extent;
  u32              mips = 1;
};

struct ImageViewDesc
{
  std::string_view pass            = "undefined";
  std::string_view name            = "undefined";
  Image            image           = Image::None;
  Format           view_format     = Format::Undefined;
  ComponentMapping mapping         = ComponentMapping::identity();
  u32              first_mip_level = 0;
  u32              num_mip_levels  = 0;
  ImageAspect      aspect          = ImageAspect::None;
};

struct RenderPassAttachment
{
  Format  format   = Format::Undefined;
  LoadOp  load_op  = LoadOp::DontCare;
  StoreOp store_op = StoreOp::DontCare;
};

/**slots description to be fed to pipeline and allow pipeline structure itself*/
/**combination of images to feed to pipeline along with renderpass*/
// we can hash the frame buffer description and store it somewhere and then re-use it for the gpu?
// render passes are used for computing tiling strategy on the GPU. we can create the render passes along with the framebuffer object
// it is a property of the passes we create, and it should be enough
//
// render pass just needs to be a compatible render pass with pre-computed tiling strategy
// we can cache renderpasses on a per-pass basis or cache the data it computes and just use a compatible renderpass that has the same operations and format
//
//
// TODO(lamarrr): how do we perform: beginrenderpass and endrenderpass then?
//
//
// on framebuffer creation we can use a different renderpass than the originally created one
//
// renderpasses are cached ATTACHMENT_UNUSED slots
//
struct RenderPassDesc
{
  std::string_view                      pass = "undefined";
  std::string_view                      name = "undefined";
  stx::Span<RenderPassAttachment const> color_attachments;
  stx::Span<RenderPassAttachment const> depth_stencil_attachments;
};

// we can cache framebuffers as they can be dynamic for some stype of passes
// TODO(lamarrr): how do we know when a renderpass and framebuffer can be destroyed?
struct FramebufferDesc
{
  std::string_view           pass       = "undefined";
  std::string_view           name       = "undefined";
  RenderPass                 renderpass = RenderPass::None;
  stx::Span<ImageView const> color_attachments;
  stx::Span<ImageView const> depth_stencil_attachments;
};

struct ResourceDesc
{
  constexpr ResourceDesc() :
      type{ResourceType::None}
  {}
  constexpr ResourceDesc(BufferDesc const &desc) :
      buffer{desc}, type{ResourceType::Buffer}
  {}
  constexpr ResourceDesc(ImageDesc const &desc) :
      image{desc}, type{ResourceType::Image}
  {}
  constexpr ResourceDesc(ImageViewDesc const &desc) :
      image_view{desc}, type{ResourceType::ImageView}
  {}
  constexpr ResourceDesc(RenderPassDesc const &desc) :
      render_pass{desc}, type{ResourceType::RenderPass}
  {}
  constexpr ResourceDesc(FramebufferDesc const &desc) :
      framebuffer{desc}, type{ResourceType::Framebuffer}
  {}

  union
  {
    BufferDesc      buffer;
    ImageDesc       image;
    ImageViewDesc   image_view;
    RenderPassDesc  render_pass;
    FramebufferDesc framebuffer;
  };
  ResourceType type;
};

struct BufferCopy
{
  u64 src_offset = 0;
  u64 dst_offset = 0;
  u64 size       = 0;
};

struct BufferImageCopy
{
  u64         buffer_offset       = 0;
  u32         buffer_row_length   = 0;
  u32         buffer_image_height = 0;
  URect       image_area;
  u32         image_mip_level = 0;
  ImageAspect image_aspect    = ImageAspect::None;
};

struct ImageCopy
{
  URect       src_area;
  u32         src_mip_level = 0;
  ImageAspect src_aspect    = ImageAspect::None;
  URect       dst_area;
  u32         dst_mip_level = 0;
  ImageAspect dst_aspect    = ImageAspect::None;
};

struct ImageBlit
{
  URect       src_area;
  u32         src_mip_level = 0;
  ImageAspect src_aspect    = ImageAspect::None;
  URect       dst_area;
  u32         dst_mip_level = 0;
  ImageAspect dst_aspect    = ImageAspect::None;
};

union Color
{
  f32 float32[4];
  i32 int32[4];
  u32 uint32[4];
};

struct DepthStencil
{
  f32 depth   = 0;
  u32 stencil = 0;
};

struct ClearValue
{
  union
  {
    Color        color;
    DepthStencil depth_stencil;
  };
};

namespace cmd
{
struct CopyBuffer
{
  Buffer            src        = Buffer::None;
  Buffer            dst        = Buffer::None;
  BufferCopy const *copies     = nullptr;
  u32               num_copies = 0;
};

// will cause a device idle wait if in use unless newly created
struct MutateBuffer
{
  Buffer                     dst       = Buffer::None;
  stx::Fn<void(void *, u64)> operation = stx::fn::make_static([](void *, u64) {});
};

struct CopyImage
{
  Image                      src = Image::None;
  Image                      dst = Image::None;
  stx::Span<ImageCopy const> copies;
};

struct CopyBufferToImage
{
  Buffer                           src = Buffer::None;
  Image                            dst = Image::None;
  stx::Span<BufferImageCopy const> copies;
};

struct BlitImage
{
  Image                      src = Image::None;
  Image                      dst = Image::None;
  stx::Span<ImageBlit const> blits;
  Filter                     filter = Filter::Nearest;
};

// TODO(lamarrr): Note: the same resource can be used in many ways
// framebuffers are cached by the system on a per-pass basis
// this signifies a draw call/compute call reception command
//
// this is for the FXPass receiver to use the information to perform draw calls
// the FXPass receiver decides which shaders to use, what shader parameters and the inputs, which would also require a command receiver
//
//
// or should we receive draw calls???? this will cause a lot of churn and logic within the EffectPass???
// what if we need vertices computed externally or something???
//
//
//
struct DispatchTask
{
  u64                              index = 0;        // task index is expected to contain a list of subcommands that don't need separate passes
  PipelineType                     type  = PipelineType::Graphics;
  stx::Span<ResourceBinding const> bindings;
  Framebuffer                      framebuffer = Framebuffer::None;        // only valid for PipelineType::Graphics graphics operations
};

// TODO(lamarrr): we will cache and create new renderpasses every time
//
struct BeginRenderPass
{
  Framebuffer                   framebuffer = Framebuffer::None;
  RenderPass                    render_pass = RenderPass::None;
  IRect                         render_area;
  stx::Span<Color const>        color_attachments_clear_values;
  stx::Span<DepthStencil const> depth_stencil_attachments_clear_values;
};

struct EndRenderPass
{
};

};        // namespace cmd

enum class CmdType : u32
{
  None              = 0,
  CopyBuffer        = 1,
  MutateBuffer      = 2,
  CopyImage         = 3,
  CopyBufferToImage = 4,
  BlitImage         = 5,
  DispatchTask      = 6,
  BeginRenderPass   = 7,
  EndRenderPass     = 8,
};

struct Cmd
{
  union
  {
    cmd::CopyBuffer        copy_buffer;
    cmd::MutateBuffer      mutate_buffer;
    cmd::CopyImage         copy_image;
    cmd::CopyBufferToImage copy_buffer_to_image;
    cmd::BlitImage         blit_image;
    cmd::DispatchTask      dispatch_task;
    cmd::BeginRenderPass   begin_render_pass;
    cmd::EndRenderPass     end_render_pass;
  };
  CmdType type = CmdType::None;

  constexpr Cmd() :
      type{CmdType::None}
  {}
  constexpr Cmd(cmd::CopyBuffer const &cmd) :
      copy_buffer{cmd}, type{CmdType::CopyBuffer}
  {}
  constexpr Cmd(cmd::MutateBuffer const &cmd) :
      mutate_buffer{cmd}, type{CmdType::MutateBuffer}
  {}
  constexpr Cmd(cmd::CopyImage const &cmd) :
      copy_image{cmd}, type{CmdType::CopyImage}
  {}
  constexpr Cmd(cmd::CopyBufferToImage const &cmd) :
      copy_buffer_to_image{cmd}, type{CmdType::CopyBufferToImage}
  {}
  constexpr Cmd(cmd::BlitImage const &cmd) :
      blit_image{cmd}, type{CmdType::BlitImage}
  {}
  constexpr Cmd(cmd::DispatchTask const &cmd) :
      dispatch_task{cmd}, type{CmdType::DispatchTask}
  {}
  constexpr Cmd(cmd::BeginRenderPass const &cmd) :
      begin_render_pass{cmd}, type{CmdType::BeginRenderPass}
  {}
  constexpr Cmd(cmd::EndRenderPass const &cmd) :
      end_render_pass{cmd}, type{CmdType::EndRenderPass}
  {}
};

constexpr usize CMD_SIZE = sizeof(Cmd);

struct GraphCtx
{
  DeviceInfo device_info;
};

// renderpass cache
//
// RESOURCE CREATION
// TODO(lamarrr): we need to check for image aliasing for the renderpasses
struct Graph
{
  GraphCtx                ctx;
  stx::Vec<u64>           free_indices;
  stx::Vec<ResourceDesc>  resources;
  stx::Vec<ResourceState> resource_states;
  Buffer                  create_buffer(BufferDesc const &);
  Image                   create_image(ImageDesc const &);
  ImageView               create_image_view(ImageViewDesc const &);
  RenderPass              create_render_pass(RenderPassDesc const &);
  Framebuffer             create_framebuffer(FramebufferDesc const &);
  BufferDesc              get_desc(Buffer);
  ImageDesc               get_desc(Image);
  ImageViewDesc           get_desc(ImageView);
  RenderPassDesc          get_desc(RenderPass);
  FramebufferDesc         get_desc(Framebuffer);
  void                    release(Buffer);
  void                    release(Image);
  void                    release(ImageView);
  void                    release(RenderPass);
  void                    release(Framebuffer);
};

// RESOURCE ACCESS DESCRIPTIONS

// WE NEED TO:
// - Automate synchronization. image, memory, barrier creation, cmdcopy, cmdblit, cmdtransfer
struct CmdBuffer
{
  void add(Cmd cmd);

  stx::Vec<Cmd> cmds;
};

struct ScreenPassCtx
{
  Extent extent;
  Format format;
  bool   suboptimal  = false;        // updated by vulkan
  u32    num_buffers = 1;
};

struct ScreenPassResources
{
  Image       color_images[16];        // screen has implicit pass to present the screen_color_image
  Image       depth_stencil_images[16];
  RenderPass  render_passes[16];
  Framebuffer framebuffers[16];
};

struct ScreenPassBindings
{
  u32 image_index = 0;
};

struct ScreenPass
{
  ScreenPassCtx       ctx;
  ScreenPassResources resources;
  ScreenPassBindings  bindings;
};

struct ScreenPass;

inline void onscreen_draw_pass(Graph &graph)
{
  // RENDER
  // transition color attachment layout from presentation optimal to color attachment optimal
  //
  //
  // perform intermediate rendering operations
  //
  //
  // transition color attachment layout from color_attachment optimal to presentation optimal
  // THIS IS POINTLESSSS, it is on-screen
  // TODO(lamarrr): graph check?

  // ASH_CHECK( ctx.screen_pass.ctx.num_buffers <= 16);

  // for (u32 i = 0; i < ctx.screen_pass.ctx.num_buffers; i++)
  // {
  //   rid                   color_image = graph.create_image(ImageDesc{.format = ctx.screen_pass.ctx.format,
  //                                                                    .usages = ImageUsages::ColorAttachment,
  //                                                                    .size   = ctx.screen_pass.ctx.extent,
  //                                                                    .mips   = 1});
  //   FramebufferAttachment color_attachment{.image    = color_image,
  //                                          .load_op  = LoadOp::Clear,
  //                                          .store_op = StoreOp::Store};

  //   rid depth_stencil_image = graph.create_image(ImageDesc{.format = Format::D16_Unorm,
  //                                                          .usages = ImageUsages::DepthStencilAttachment,
  //                                                          .size   = ctx.screen_pass.ctx.extent,
  //                                                          .mips   = 1});

  //   FramebufferAttachment depth_stencil_attachment{.image    = depth_stencil_image,
  //                                                  .load_op  = LoadOp::Clear,
  //                                                  .store_op = StoreOp::Store};

  //   rid framebuffer                                   = graph.create_framebuffer(RenderPassDesc{.render_pass   = render_pass,
  //                                                                                               .color         = color_image,
  //                                                                                               .depth_stencil = depth_stencil_image});
  //   ctx.screen_pass.resources.color_images[i]         = color_image;
  //   ctx.screen_pass.resources.depth_stencil_images[i] = depth_stencil_image;
  //   ctx.screen_pass.resources.render_passes[i]        = render_pass;
  //   ctx.screen_pass.resources.framebuffers[i]         = framebuffer;
  // }
  // record render ops
}

inline void onscreen_draw_pass_update(Graph &graph, GraphCtx &ctx)
{
}

struct OffscreenPass
{
  // if these changes, the resources need to be recreated
  struct Arguments
  {
    ImageDesc color_attachment_desc;
    ImageDesc depth_stencil_attachment_desc;
    LoadOp    color_load_op          = LoadOp::DontCare;
    LoadOp    depth_stencil_load_op  = LoadOp::DontCare;
    StoreOp   color_store_op         = StoreOp::DontCare;
    StoreOp   depth_stencil_store_op = StoreOp::DontCare;
  } arguments;

  struct Resources
  {
    Image       color_images[1]              = {Image::None};
    ImageView   color_image_views[1]         = {ImageView::None};
    Image       depth_stencil_images[1]      = {Image::None};
    ImageView   depth_stencil_image_views[1] = {ImageView::None};
    RenderPass  render_pass                  = RenderPass::None;
    Framebuffer framebuffer                  = Framebuffer::None;
  } resources;

  struct State
  {
    RenderPassAttachment color_attachments[1];
    RenderPassAttachment depth_stencil_attachments[1];
    Color                clear_colors[1]         = {Color{.int32 = {0, 0, 0, 0}}};
    DepthStencil         clear_depth_stencils[1] = {DepthStencil{.depth = 0, .stencil = 0}};
  } state;

  // bindings don't require changes to the resources, and can change for every task execution
  // these are input or output bindings
  struct Bindings
  {
  } bindings;

  // to check if to recreate resources
  bool diff(Graph const &graph, Arguments const &new_args)
  {
    return false;
  }

  void init(Graph &graph, CmdBuffer &cmd_buffer)
  {
    // SETUP
    //
    // get the number maximum number of offscreen draw passes in the scene = N
    //
    // create N color output render targets with undefined layout
    // optionally create N depth stencil output render targets with undefined layout
    // left to the pipeline to determine the inputs???
    //

    bool has_color         = arguments.color_attachment_desc.format != Format::Undefined;
    bool has_depth_stencil = arguments.depth_stencil_attachment_desc.format != Format::Undefined;

    if (has_color)
    {
      resources.color_images[0]      = graph.create_image(arguments.color_attachment_desc);
      resources.color_image_views[0] = graph.create_image_view(ImageViewDesc{.image           = resources.color_images[0],
                                                                             .view_format     = arguments.color_attachment_desc.format,
                                                                             .mapping         = ComponentMapping::identity(),
                                                                             .first_mip_level = 0,
                                                                             .num_mip_levels  = 1,
                                                                             .aspect          = ImageAspect::Color});
    }

    if (has_depth_stencil)
    {
      resources.depth_stencil_images[0]      = graph.create_image(arguments.depth_stencil_attachment_desc);
      resources.depth_stencil_image_views[0] = graph.create_image_view(ImageViewDesc{.image           = resources.depth_stencil_images[0],
                                                                                     .view_format     = arguments.depth_stencil_attachment_desc.format,
                                                                                     .mapping         = ComponentMapping::identity(),
                                                                                     .first_mip_level = 0,
                                                                                     .num_mip_levels  = 1,
                                                                                     .aspect          = ImageAspect::Depth | ImageAspect::Stencil});
    }

    state.color_attachments[0]         = RenderPassAttachment{.format   = arguments.color_attachment_desc.format,
                                                              .load_op  = arguments.color_load_op,
                                                              .store_op = arguments.color_store_op};
    state.depth_stencil_attachments[0] = RenderPassAttachment{.format   = arguments.depth_stencil_attachment_desc.format,
                                                              .load_op  = arguments.depth_stencil_load_op,
                                                              .store_op = arguments.depth_stencil_store_op};
    resources.render_pass              = graph.create_render_pass(RenderPassDesc{.color_attachments         = has_color ? stx::Span{state.color_attachments} : stx::Span<RenderPassAttachment>{},
                                                                                 .depth_stencil_attachments = has_depth_stencil ? stx::Span{state.depth_stencil_attachments} : stx::Span<RenderPassAttachment>{}});
    resources.framebuffer              = graph.create_framebuffer(FramebufferDesc{.renderpass                = resources.render_pass,
                                                                                  .color_attachments         = has_color ? stx::Span{resources.color_image_views} : stx::Span<ImageView>{},
                                                                                  .depth_stencil_attachments = has_depth_stencil ? stx::Span{resources.depth_stencil_image_views} : stx::Span<ImageView>{}});
  }

  void execute(Graph &graph, CmdBuffer &cmd_buffer)
  {
    //
    // for all N outputs insert barrier to convert from used or newly created layout to color attachment output layout
    //
    // for each N batch:
    //
    // for each z-sorted offscreen render pass:
    //
    //
    // RENDER
    // perform all intermediate rendering operations
    //
    // transition layout of color render target to shader read or transfer src or dst
    //
    // render to target
    // insert barrier to convert layout back to color attachment output
    //
    // we might want to leave the final image layout or state until completion of the pipeline as we don't know how exactly the will be used
    //

    bool has_color         = arguments.color_attachment_desc.format != Format::Undefined;
    bool has_depth_stencil = arguments.depth_stencil_attachment_desc.format != Format::Undefined;

    cmd_buffer.add(cmd::BeginRenderPass{.framebuffer                            = resources.framebuffer,
                                        .render_pass                            = resources.render_pass,
                                        .render_area                            = IRect{.offset = {0, 0}, .extent = arguments.color_attachment_desc.extent},
                                        .color_attachments_clear_values         = has_color ? stx::Span{state.clear_colors} : stx::Span<Color>{},
                                        .depth_stencil_attachments_clear_values = has_depth_stencil ? stx::Span{state.clear_depth_stencils} : stx::Span<DepthStencil>{}});
    cmd_buffer.add(cmd::DispatchTask{.index       = 0,
                                     .type        = PipelineType::Graphics,
                                     .bindings    = {},
                                     .framebuffer = resources.framebuffer});        // TODO(lamarrr): what are we using framebuffer for here?>
    cmd_buffer.add(cmd::EndRenderPass{});
  }
};

inline void clipped_draw_pass()
{
}

struct BlurCapturePass
{
  struct Arguments
  {
    Extent blur_radius;
    Extent input_image_subregion_extent;
    Format input_image_format = Format::R8G8B8A8_UNORM;
  } arguments;

  struct Resources
  {
    Buffer kernel_buffer           = Buffer::None;
    Image  sample_image            = Image::None;
    u32    sample_image_mip_levels = 0;
    Extent sample_image_extent;
    Buffer sample_buffer = Buffer::None;
    Buffer result_buffer = Buffer::None;
  } resources;

  struct State
  {
    ImageBlit       mip_down_blits[6];
    ImageBlit       mip_up_blits[6];
    ResourceBinding pipeline_bindings[32];
  } state;

  struct Bindings
  {
    Image  input_image     = Image::None;
    u32    input_image_mip = 0;
    Offset input_image_offset;
  } bindings;

  constexpr u8 pixel_byte_size(Format)
  {
    return 1;
  }

  void init(Graph &graph, CmdBuffer &cmd_buffer)
  {
    // SETUP
    // TODO(lamarrr): how do we handle alpha??
    // TODO(lamarrr): we can just blit from the source image down to mips directly
    resources.sample_image_mip_levels = std::min(arguments.input_image_subregion_extent.max_mip_levels(), 6U);
    resources.sample_image_extent     = arguments.input_image_subregion_extent;
    Extent downsampled_input_extent   = arguments.input_image_subregion_extent.at_mip_level(resources.sample_image_mip_levels - 1);
    resources.kernel_buffer           = graph.create_buffer(BufferDesc{.size       = arguments.blur_radius.area(),
                                                                       .properties = graph.ctx.device_info.memory_heaps.has_unified_memory() ? (MemoryProperties::DeviceLocal | MemoryProperties::HostVisible) : MemoryProperties::HostVisible,
                                                                       .usages     = BufferUsages::UniformBuffer});
    resources.sample_image            = graph.create_image(ImageDesc{.format = arguments.input_image_format,
                                                                     .usages = ImageUsages::Sampled,
                                                                     .extent = arguments.input_image_subregion_extent,
                                                                     .mips   = resources.sample_image_mip_levels});
    resources.sample_buffer           = graph.create_buffer(BufferDesc{.size       = downsampled_input_extent.area() * pixel_byte_size(arguments.input_image_format),
                                                                       .properties = MemoryProperties::DeviceLocal,
                                                                       .usages     = BufferUsages::TransferDst | BufferUsages::TransferSrc | BufferUsages::StorageBuffer});
    resources.result_buffer           = graph.create_buffer(BufferDesc{.size       = downsampled_input_extent.area() * pixel_byte_size(arguments.input_image_format),
                                                                       .properties = MemoryProperties::DeviceLocal,
                                                                       .usages     = BufferUsages::TransferDst | BufferUsages::TransferSrc | BufferUsages::StorageBuffer});
    // resource initialization stage
    // resource pre-render setup
    //
    //
    // TODO(lamarrr): mips up to a certain extent, sanity check the mip level
    // prepare constants
    // RENDER
    //
    // for each blur pass
    // for vert and horz pass
    //
    // transition src image layout to transfer src
    // copy from src image to the buffer
    // use buffer as SSBO
    // perform seperable vert and horz gaussian blur in compute shader
    //
    // hand over image to the next user and let them decide how to use them? only valid usage is sampled image
    //
    //
    // copy image from input to sample mip level 0
    // blit image to image mip level N-1
    // what if we want to execute compute shader at creation time?
    // this should be done externally, we are only concerned with pass resource management????
    // even if done externally, we still need to track resources?
    //
    cmd_buffer.add(cmd::MutateBuffer{.dst       = resources.kernel_buffer,
                                     .operation = stx::fn::make_static([](void *data, u64 size) {
                                       std::memset(data, 0, size);
                                     })});
  }

  // to check if to recreate resources
  bool diff(Graph const &graph, Arguments const &new_args)
  {
    return false;
  }

  void execute(Graph &graph, CmdBuffer &cmd_buffer)
  {
    // TODO(lamarrr): assume this is correct
    state.mip_up_blits[resources.sample_image_mip_levels - 1] = state.mip_down_blits[0] = ImageBlit{
        .src_area      = URect{.offset = {0, 0}, .extent = resources.sample_image_extent},
        .src_mip_level = 0,
        .src_aspect    = ImageAspect::Color,
        .dst_area      = URect{.offset = {0, 0}, .extent = resources.sample_image_extent},
        .dst_mip_level = 0,
        .dst_aspect    = ImageAspect::Color};

    for (u32 i = 1; i < resources.sample_image_mip_levels; i++)
    {
      state.mip_up_blits[resources.sample_image_mip_levels - 1 - i] = state.mip_down_blits[i] = ImageBlit{
          .src_area      = URect{.offset = {0, 0}, .extent = resources.sample_image_extent.at_mip_level(i - 1)},
          .src_mip_level = i - 1,
          .src_aspect    = ImageAspect::Color,
          .dst_area      = URect{.offset = {0, 0}, .extent = resources.sample_image_extent.at_mip_level(i)},
          .dst_mip_level = i,
          .dst_aspect    = ImageAspect::Color};
    }

    cmd_buffer.add(cmd::BlitImage{.src    = bindings.input_image,
                                  .dst    = resources.sample_image,
                                  .blits  = stx::Span{state.mip_down_blits, resources.sample_image_mip_levels},
                                  .filter = Filter::Nearest});

    // TODO(lamarrr): what if pipeline clears depth stencil multiple times?
    //
    //
    // a concerning usage would be:
    //
    // perform pass
    // render draw commands//
    // render draw commands// here, we would need to generate multiple barriers depending on the number of tasks being performed, we can make it a part of the binding argument
    // render draw commands//
    // render draw commands//
    // perform pass
    //
    //
    //
    //
    // TODO(lamarrr): what about intermediate drawing passes? will this be handled by the executor??? will this worsen resource management?

    // TODO(lamarrr): what about specifying multiple commands??? with inputs??

    state.pipeline_bindings[0] = BufferBinding{.buffer = resources.kernel_buffer, .access = Access::ShaderRead, .stages = PipelineStages::ComputeShader};
    state.pipeline_bindings[1] = BufferBinding{.buffer = resources.sample_buffer, .access = Access::ShaderRead, .stages = PipelineStages::ComputeShader};
    state.pipeline_bindings[2] = BufferBinding{.buffer = resources.result_buffer, .access = Access::ShaderStorageWrite, .stages = PipelineStages::ComputeShader};

    // for graphics passes that write to the framebuffer, it will generate sync primitives for them
    cmd_buffer.add(cmd::BeginRenderPass{.render_pass = RenderPass::None});
    cmd_buffer.add(cmd::DispatchTask{.index    = 0,
                                     .type     = PipelineType::Compute,
                                     .bindings = stx::Span{state.pipeline_bindings, 2}});
    cmd_buffer.add(cmd::EndRenderPass{});

    // each render task execution will need to wait on the framebuffer
    cmd_buffer.add(cmd::DispatchTask{.index    = 0,
                                     .type     = PipelineType::Compute,
                                     .bindings = {}});

    cmd_buffer.add(cmd::BlitImage{.src    = bindings.input_image,
                                  .dst    = resources.sample_image,
                                  .blits  = stx::Span{state.mip_up_blits, resources.sample_image_mip_levels},
                                  .filter = Filter::Nearest});
  }
};

struct BloomCapturePassExecutor
{
  // we need to sync each generated frame?
  void init(BlurCapturePass);
  void execute()
  {
    // add_task(TaskType::Compute, [](){});
    // add_task(TaskType::Compute, [](){});
    // add_task(TaskType::Compute, [](){});
    // add_task(TaskType::Compute, [](){});
    // take draw lists
  }
};

inline void bloom3d_pass()
{
  // SETUP
  // RENDER
}

inline void outline3d_pass()
{
  // SETUP
  // create depth attachment
  // RENDER
  // clear depth attachment
  // disable depth test and depth buffer
  // draw commands using colors only
  // enable depth test and depth buffer
  // draw object
}

inline void chromatic_aberration_pass()
{
  // https://www.shadertoy.com/view/Mds3zn
  // SETUP
  // RENDER
}

inline void effect_pass()
{
  // SETUP
  // RENDER
}

//
// DeviceLocal::HostVisible when available
//
// or AccessType::OnDeviceOnly? this will enable it to write directly
// AccessType::Host AccessType::DeviceAndHost
//
// or request UsageHints
//

// TODO(lamarrr): some states in the graphics ctx might change, we need to diff them for the passes
// Resource utilization optimization passes
//
// the graphics api already knows how to optimize and multi-thread accesses, we just need to insert barriers appropriately
//
// each operation will have a number of barriers that need to be inserted before it executes
//
inline void generate_sync_primitives(Graph const &graph, stx::Span<Cmd const> cmds, stx::Vec<QueueBarrier> &queue_barriers, stx::Vec<u32> &cmd_barriers)
{
  // accumulate states until they are no longer needed
  // use previous frame's states and barriers states
  // detect unused resources
  // insert fences, barriers, and whatnot
  // smart aliasing image memory barriers with aliasing will enable us to perform better
  // store current usage so the next usage will know how to access
  //
  // render passes perform layout and transitions neccessary
  //
  //

  for (Cmd const &cmd : cmds)
  {
    switch (cmd.type)
    {
      case CmdType::None:
        break;
      case CmdType::BlitImage:
      {
        // check RID
        // check current layout and usage, and all accessors
        // check all memory aliasing
        // convert layout to transfer dst and write access with whatever access type is needed for blitting
        // update barrier tracker
        // on next usage in command buffer, get the last usage and update accordingly
        ImageState &src_state = graph.resource_states[(u64) cmd.blit_image.src].image;
        ImageState &dst_state = graph.resource_states[(u64) cmd.blit_image.dst].image;

        for (ImageBlit const &blit : cmd.blit_image.blits)
        {
          QueueImageMemoryBarrier src_barrier{.image           = cmd.blit_image.src,
                                              .first_mip_level = blit.src_mip_level,
                                              .num_mip_levels  = 1,
                                              .aspect          = blit.src_aspect,
                                              .old_layout      = src_state.layout,
                                              .new_layout      = ImageLayout::TransferSrcOptimal,
                                              .src_stage_mask  = src_state.stage,
                                              .dst_stage_mask  = PipelineStages::Transfer,
                                              .src_access_mask = src_state.access_mask,
                                              .dst_access_mask = Access::TransferRead};

          QueueImageMemoryBarrier dst_barrier{.image           = cmd.blit_image.dst,
                                              .first_mip_level = blit.dst_mip_level,
                                              .num_mip_levels  = 1,
                                              .aspect          = blit.dst_aspect,
                                              .old_layout      = dst_state.layout,
                                              .new_layout      = ImageLayout::TransferDstOptimal,
                                              .src_stage_mask  = dst_state.stage,
                                              .dst_stage_mask  = PipelineStages::Transfer,
                                              .src_access_mask = dst_state.access_mask,
                                              .dst_access_mask = Access::TransferWrite};

          queue_barriers.push_inplace(src_barrier).unwrap();
          queue_barriers.push_inplace(dst_barrier).unwrap();

          src_state.access_mask = Access::TransferRead;
          dst_state.access_mask = Access::TransferWrite;
          src_state.stage       = PipelineStages::Transfer;
          dst_state.stage       = PipelineStages::Transfer;
          src_state.layout      = ImageLayout::TransferSrcOptimal;
          dst_state.layout      = ImageLayout::TransferDstOptimal;
        }

        cmd_barriers.push(cmd.blit_image.blits.size() * 2).unwrap();
      }
      break;
      case CmdType::CopyBufferToImage:
      {
        // get latest expected state
        // convert to transfer dst layout and transfer write access with transfer stage
        // leave as-is. the next usage should conver it back to how it is needed if necessary
        BufferState &src_state = graph.resource_states[(u64) cmd.copy_buffer_to_image.src].buffer;
        ImageState  &dst_state = graph.resource_states[(u64) cmd.copy_buffer_to_image.dst].image;

        for (BufferImageCopy const &copy : cmd.copy_buffer_to_image.copies)
        {
          QueueBufferMemoryBarrier src_barrier{.buffer          = cmd.copy_buffer_to_image.src,
                                               .offset          = copy.buffer_offset,
                                               .size            = copy.buffer_row_length * copy.buffer_image_height,
                                               .src_stage_mask  = src_state.stage,
                                               .dst_stage_mask  = PipelineStages::Transfer,
                                               .src_access_mask = src_state.access_mask,
                                               .dst_access_mask = Access::TransferRead};

          QueueImageMemoryBarrier dst_barrier{.image           = cmd.copy_buffer_to_image.dst,
                                              .first_mip_level = copy.image_mip_level,
                                              .num_mip_levels  = 1,
                                              .aspect          = copy.image_aspect,
                                              .old_layout      = dst_state.layout,
                                              .new_layout      = ImageLayout::TransferDstOptimal,
                                              .src_stage_mask  = dst_state.stage,
                                              .dst_stage_mask  = PipelineStages::Transfer,
                                              .src_access_mask = dst_state.access_mask,
                                              .dst_access_mask = Access::TransferWrite};

          queue_barriers.push_inplace(src_barrier).unwrap();
          queue_barriers.push_inplace(dst_barrier).unwrap();

          src_state.access_mask = Access::TransferRead;
          dst_state.access_mask = Access::TransferWrite;
          src_state.stage       = PipelineStages::Transfer;
          dst_state.stage       = PipelineStages::Transfer;
          dst_state.layout      = ImageLayout::TransferDstOptimal;
        }

        cmd_barriers.push(cmd.copy_buffer_to_image.copies.size() * 2).unwrap();
      }

      break;

      case CmdType::BeginRenderPass:
      {
        cmd.begin_render_pass.render_pass;
        cmd.begin_render_pass.framebuffer;
        cmd.begin_render_pass.color_attachments_clear_values;
        cmd.begin_render_pass.depth_stencil_attachments_clear_values;
      }
      break;

      case CmdType::EndRenderPass:
      {
      }
      break;

      case CmdType::DispatchTask:
      {
        for (ResourceBinding const &binding : cmd.dispatch_task.bindings)
        {
          switch (binding.type)
          {
            case ResourceBindingType::BufferBinding:
            {
              BufferState &state = graph.resource_states[(u64) binding.buffer.buffer].buffer;

              QueueBufferMemoryBarrier barrier{.buffer          = binding.buffer.buffer,
                                               .offset          = 0,
                                               .size            = stx::U64_MAX,
                                               .src_stage_mask  = state.stage,
                                               .dst_stage_mask  = binding.buffer.stages,
                                               .src_access_mask = state.access_mask,
                                               .dst_access_mask = binding.buffer.access};

              queue_barriers.push_inplace(barrier).unwrap();
              state.access_mask = binding.buffer.access;
              state.stage       = binding.buffer.stages;
            }
            break;
            case ResourceBindingType::ImageViewBinding:
            {
              ImageViewDesc const &sub_desc = graph.resources[(u64) binding.image_view.image_view].image_view;
              ImageDesc const     &desc     = graph.resources[(u64) sub_desc.image].image;
              ImageState          &state    = graph.resource_states[(u64) sub_desc.image].image;

              QueueImageMemoryBarrier barrier{.image           = sub_desc.image,
                                              .first_mip_level = sub_desc.first_mip_level,
                                              .num_mip_levels  = sub_desc.num_mip_levels,
                                              .aspect          = sub_desc.aspect,
                                              .old_layout      = state.layout,
                                              .new_layout      = ImageLayout::ShaderReadOnlyOptimal,
                                              .src_stage_mask  = state.stage,
                                              .dst_stage_mask  = binding.image_view.stages,
                                              .src_access_mask = state.access_mask,
                                              .dst_access_mask = binding.image_view.access};

              queue_barriers.push_inplace(barrier).unwrap();
              state.access_mask = binding.image_view.access;
              state.stage       = binding.image_view.stages;
            }
            break;

            default:
              break;
          }
        }
        cmd_barriers.push(2).unwrap();        // TODO(lamarrr)
      }
      break;

      default:
        break;
    }
  }
}

struct Pipeline
{};
struct ShaderMatrix
{};
// TODO(lamarrr): dynamic shaders / baking
struct DynamicShader
{
  // bindings
  // script
};

}        // namespace lgfx
}        // namespace ash
