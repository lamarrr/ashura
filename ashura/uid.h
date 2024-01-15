#pragma once
#include "ashura/types.h"

namespace ash
{

typedef u8  uid8;
typedef u16 uid16;
typedef u32 uid32;
typedef u64 uid64;

constexpr uid8  INVALID_UID8  = U8_MAX;
constexpr uid16 INVALID_UID16 = U16_MAX;
constexpr uid32 INVALID_UID32 = U32_MAX;
constexpr uid64 INVALID_UID64 = U64_MAX;

}        // namespace ash
