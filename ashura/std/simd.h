#pragma once
#include "ashura/std/types.h"

namespace ash
{
// any
// all
// none
// minimum
// maximum
// sum
// product
// dot product
// group/split
// combine/splice
// splatted reduction
// cumsum
// sort
// shuffle
// interleaving
// deinterleaving
// reverse
// broadcast
// mask-operations, must use unsigned types

/// REQUIREMENTS
/// - automatic dispatch or algorithm specialization based on data alignment and
/// the number of valid elements in the specified data.
/// - allow interoperation with plain float pointers
/// - create simd adapter allocator that always allocates simd-aligned memory of
/// a specified minimum alignment.
/// - default over-alignment is always 1, unspecified will invoke dynamic
/// dispatch
/// - choose between runtime and comptime (most pref) dispatch. avoid using a
/// separate type for simd elements

}        // namespace ash