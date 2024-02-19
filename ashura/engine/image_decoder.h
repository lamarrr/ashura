#pragma once
#include <utility>

#include "ashura/engine/error.h"
#include "ashura/std/image.h"
#include "ashura/std/result.h"
#include "ashura/std/types.h"

namespace ash
{

Result<ImageBuffer, Error> decode_webp(AllocatorImpl const &allocator,
                                       Span<u8 const>       bytes);

Result<ImageBuffer, Error> decode_jpg(AllocatorImpl const &allocator,
                                      Span<u8 const>       bytes);

Result<ImageBuffer, LoadError> decode_png(AllocatorImpl const &allocator,
                                          Span<u8 const>       bytes);

Result<ImageBuffer, Error> decode_image(AllocatorImpl const &allocator,
                                        Span<u8 const>       bytes);

}        // namespace ash
