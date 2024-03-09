#pragma once
#include <utility>

#include "ashura/engine/error.h"
#include "ashura/std/image.h"
#include "ashura/std/result.h"
#include "ashura/std/types.h"
#include "ashura/std/vec.h"

namespace ash
{

Result<ImageSpan<u8>, DecodeError> decode_webp(Span<u8 const> bytes,
                                               Vec<u8>       &vec);

Result<ImageSpan<u8>, DecodeError> decode_jpg(Span<u8 const> bytes,
                                              Vec<u8>       &vec);

Result<ImageSpan<u8>, DecodeError> decode_png(Span<u8 const> bytes,
                                              Vec<u8>       &vec);

Result<ImageSpan<u8>, DecodeError> decode_image(Span<u8 const> bytes,
                                                Vec<u8>       &vec);

}        // namespace ash
