#pragma once

#include <chrono>
#include <cinttypes>
#include <functional>
#include <map>
#include <utility>
#include <variant>

#include "stx/async.h"
#include "stx/mem.h"
#include "stx/result.h"
#include "vlk/asset.h"
#include "vlk/asset_tag.h"
#include "vlk/subsystem/impl.h"
#include "vlk/utils.h"

namespace vlk {

enum class AssetCacheError : uint8_t { InvalidTag };

using PendingAsset = stx::Future<stx::Rc<Asset *>>;
using LoadedAsset = stx::Rc<Asset *>;
struct CanceledAsset {};

// not thread or async safe
struct AssetCache : public SubsystemImpl {
  // NOTE: the asset_future must not be moved from
  void update(AssetTag tag, stx::Future<stx::Rc<Asset *>> asset_future) {
    data_.insert_or_assign(std::move(tag), AssetInfo{std::move(asset_future)});
  }

  void update(AssetTag tag, stx::Rc<Asset *> asset) {
    data_.insert_or_assign(std::move(tag), AssetInfo{asset});
  }

  auto discard(AssetTag const &tag)
      -> stx::Result<stx::NoneType, AssetCacheError> {
    auto pos = data_.find(tag);
    if (pos == data_.end()) {
      return stx::Err(AssetCacheError::InvalidTag);
    } else {
      data_.erase(pos);
    }
  }

  auto query(AssetTag tag) const
      -> stx::Result<std::variant<PendingAsset, LoadedAsset, CanceledAsset>,
                     AssetCacheError> {
    auto pos = data_.find(tag);
    if (pos == data_.end()) {
      return stx::Err(AssetCacheError::InvalidTag);
    } else {
      return stx::Ok(std::variant{pos->second.asset});
    }
  }

  void link(SubsystemsContext const &) final {}

  void tick(std::chrono::nanoseconds) final;

 private:
  struct AssetInfo {
    std::variant<PendingAsset, LoadedAsset, CanceledAsset> asset;
  };

  uint64_t total_size_ = 0;
  std::map<AssetTag, AssetInfo, std::less<>> data_;
};

}  // namespace vlk
