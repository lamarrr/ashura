
#pragma once

#include <map>
#include <utility>

#include "ashura/primitives.h"
#include "stx/result.h"
#include "stx/void.h"

namespace ash
{

enum class AssetBundleError : u8
{
  InvalidId
};

template <typename T>
struct AssetBundle
{
  u64 add(T &&asset)
  {
    u64 id = next_id;
    next_id++;
    data.emplace(id, std::move(asset));
    return id;
  }

  stx::Result<stx::Void, AssetBundleError> remove(u64 asset)
  {
    auto it = data.find(asset);
    if (it == data.end())
      return stx::Err(AssetBundleError::InvalidId);
    data.erase(it);
    return stx::Ok(stx::Void{});
  }

  stx::Result<T const *, AssetBundleError> get(u64 asset) const
  {
    auto it = data.find(asset);
    if (it == data.end())
      return stx::Err(AssetBundleError::InvalidId);
    return stx::Ok(&it->second);
  }

  bool has(u64 asset) const
  {
    return data.find(asset) != data.end();
  }

  std::map<u64, T> data;
  u64              next_id = 0;
};

}        // namespace ash
