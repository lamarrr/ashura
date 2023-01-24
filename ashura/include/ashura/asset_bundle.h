
#pragma once

#include <map>

#include "ashura/primitives.h"
#include "stx/result.h"
#include "stx/spinlock.h"
#include "stx/void.h"

namespace asr {

enum class AssetBundleError : u8 { InvalidId };

template <typename T>
struct AssetBundle {
  u64 add(T&& asset);

  stx::Result<stx::Void, AssetBundleError> remove(u64 asset);

  stx::Result<T*, AssetBundleError> get(u64 asset) const;

  bool has(u64 asset) const;

  std::map<u64, T> data_;
  u64 next_ = 0;
  stx::SpinLock lock_;
};

}  // namespace asr
