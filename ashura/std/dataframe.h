/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/enum.h"
#include "ashura/std/option_span.h"
#include "ashura/std/types.h"
#include "ashura/std/vec.h"

namespace ash
{
namespace experimental
{

namespace df
{

/// @brief Slicing type of the contained data
enum class Slicing : u8
{
  /// @brief no slicing is required to unpack the data
  None     = 0,
  /// @brief non-contiguous slice segments with pair of indices indicating the offset and size
  Segments = 1,
  /// @brief run-end encoding with the first index being 0 and the rest being the run size up to that point
  Runs     = 2
};

inline constexpr u8 SLICING_COUNT = 3;

enum class Type : u8
{
  None       = 0,
  Bool       = 1,
  Uint       = 2,
  Bits       = Uint,
  Int        = 3,
  Float      = 4,
  Bytes      = 5,
  Utf8       = 6,
  Utf16      = 7,
  Utf32      = 8,
  Timestamp  = 9,
  Duration   = 10,
  Decimal    = 11,
  List       = 12,
  Struct     = 13,
  Union      = 14,
  Dictionary = 15,
  Map        = 16,
  Custom     = 31,
};

inline constexpr u8 TYPE_COUNT = 32;

struct ArrayMetaData
{
};

/// @brief A cross-ABI data-interchange format for columnar data.
struct ArrayInfo
{
  static constexpr u64 NULL_MASK_INDEX          = 0;
  static constexpr u64 PREFERRED_DATA_ALIGNMENT = SIMD_ALIGNMENT;

  /// @brief data type of the array
  Type type = Type::None;

  /// @brief slicing method of the array,
  /// i.e. for strings: scattered indices or run-end encoded indices
  Slicing slicing = Slicing::None;

  /// @brief bit-width of the stored element type,
  /// i.e. u64 is 64, and bits are 1
  u16 bit_size = 0;

  /// @brief atom-width of the stored type,
  /// i.e bits can be stored in u64 types. the atom-bit-size will be 64
  u16 atom_bit_size = 0;

  /// @brief the number of component arrays
  u16 num_components = 0;

  /// @brief the number of elements
  u64 size = 0;

  /// @brief the number of elements the array is capable of holding
  u64 capacity = 0;

  /// @brief the total amount of used memory for this array
  u64 size_bytes = 0;

  /// @brief the base alignment of the allocated memory
  u64 alignment_bytes = 0;

  /// @brief the total amount of memory allocated for this array
  u64 capacity_bytes = 0;

  /// @brief the component arrays, they do not have children
  ArrayInfo * ASH_RESTRICT * ASH_RESTRICT components = nullptr;

  /// @brief the storage data
  void * ASH_RESTRICT data = nullptr;
};

struct Array
{
  ArrayInfo * info_;
};

}    // namespace df
}    // namespace experimental
}    // namespace ash
