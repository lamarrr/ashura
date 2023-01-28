
#pragma once

#include <map>
#include <utility>

#include "ashura/primitives.h"
#include "stx/result.h"
#include "stx/spinlock.h"
#include "stx/void.h"

namespace asr {

enum class AssetBundleError : u8 { InvalidId };

template <typename T>
struct AssetBundle {
  u64 add(T&& asset) {
    stx::LockGuard guard{lock_};
    u64 id = next_;
    next_++;
    data_.emplace(id, std::move(asset));
    return id;
  }

  stx::Result<stx::Void, AssetBundleError> remove(u64 asset) {
    stx::LockGuard guard{lock_};
    auto it = data_.find(asset);
    if (it == data_.end()) return stx::Err(AssetBundleError::InvalidId);
    data_.erase(it);
    return stx::Ok(stx::Void{});
  }

  stx::Result<T*, AssetBundleError> get(u64 asset) {
    stx::LockGuard guard{lock_};
    auto it = data_.find(asset);
    if (it == data_.end()) return stx::Err(AssetBundleError::InvalidId);
    return stx::Ok(&it->second);
  }

  bool has(u64 asset) {
    stx::LockGuard guard{lock_};
    return data_.find(asset) != data_.end();
  }

  std::map<u64, T> data_;
  u64 next_ = 0;
  stx::SpinLock lock_;
};

}  // namespace asr
