/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/errors.hpp"
#include "ashura/gpu/gpu.h"
#include "ashura/std/result.hpp"
#include "ashura/std/types.hpp"
#include "ashura/std/vec.hpp"

namespace ash
{

struct DecodedImageInfo
{
    u32x2       extent{1, 1};
    gpu::Format format = gpu::Format::Undefined;
};

Result<DecodedImageInfo, SysErr> decode_webp(Span<u8 const> bytes, Vec<u8> & channels);

Result<DecodedImageInfo, SysErr> decode_png(Span<u8 const> bytes, Vec<u8> & channels);

Result<DecodedImageInfo, SysErr> decode_jpg(Span<u8 const> bytes, Vec<u8> & channels);

Result<DecodedImageInfo, SysErr> decode_image(Span<u8 const> bytes, Vec<u8> & channels);

}    // namespace ash
