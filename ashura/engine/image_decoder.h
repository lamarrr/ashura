#pragma once
#include <utility>

#include "ashura/engine/error.h"
#include "ashura/std/image.h"
#include "ashura/std/result.h"
#include "ashura/std/types.h"

namespace ash
{

Result<Allocation, Error> decode_webp(AllocatorImpl const &allocator,
                                      Span<u8 const>       bytes,
                                      ImageSpan<u8>       &span);

Result<Allocation, Error> decode_jpg(AllocatorImpl const &allocator,
                                     Span<u8 const> bytes, ImageSpan<u8> &span);

Result<Allocation, Error> decode_png(AllocatorImpl const &allocator,
                                     Span<u8 const> bytes, ImageSpan<u8> &span);

Result<Allocation, Error> decode_image(AllocatorImpl const &allocator,
                                       Span<u8 const>       bytes,
                                       ImageSpan<u8>       &span);

}        // namespace ash
