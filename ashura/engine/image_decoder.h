#pragma once
#include <utility>

#include "ashura/std/image.h"
#include "ashura/std/types.h"

namespace ash
{

Result<ImageBuffer, LoadError> decode_webp(Span<u8 const> data);
Result<ImageBuffer, LoadError> decode_jpg(Span<u8 const> bytes);
Result<ImageBuffer, LoadError> decode_image(Span<u8 const> bytes);

}        // namespace ash
