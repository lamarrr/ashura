#pragma once
#include <utility>

#include "ashura/engine/error.h"
#include "ashura/std/image.h"
#include "ashura/std/result.h"
#include "ashura/std/types.h"

namespace ash
{

// TODO(lamarrr): use vec instead?
Result<Allocation, DecodeError> decode_webp(AllocatorImpl const &allocator,
                                            Span<u8 const>       bytes,
                                            ImageSpan<u8>       &span);

Result<Allocation, DecodeError> decode_jpg(AllocatorImpl const &allocator,
                                           Span<u8 const>       bytes,
                                           ImageSpan<u8>       &span);

Result<Allocation, DecodeError> decode_png(AllocatorImpl const &allocator,
                                           Span<u8 const>       bytes,
                                           ImageSpan<u8>       &span);

Result<Allocation, DecodeError> decode_image(AllocatorImpl const &allocator,
                                             Span<u8 const>       bytes,
                                             ImageSpan<u8>       &span);

}        // namespace ash
