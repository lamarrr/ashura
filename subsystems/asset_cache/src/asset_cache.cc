#include "vlk/subsystems/asset_cache.h"

#include "fmt/format.h"

namespace {
inline std::string format_bytes_unit(uint64_t bytes) {
  static constexpr uint64_t kb_bytes = 1000UL;
  static constexpr uint64_t mb_bytes = kb_bytes * 1000UL;
  static constexpr uint64_t gb_bytes = mb_bytes * 1000UL;
  static constexpr uint64_t tb_bytes = gb_bytes * 1000UL;

  if (bytes >= (tb_bytes / 10)) {
    return fmt::format("{:.2f} TeraBytes",
                       bytes / static_cast<float>(tb_bytes));
  } else if (bytes >= (gb_bytes / 10)) {
    return fmt::format("{:.2f} GigaBytes",
                       bytes / static_cast<float>(gb_bytes));
  } else if (bytes >= (mb_bytes / 10)) {
    return fmt::format("{:.2f} MegaBytes",
                       bytes / static_cast<float>(mb_bytes));
  } else if (bytes >= (kb_bytes / 10)) {
    return fmt::format("{:.2f} KiloBytes",
                       bytes / static_cast<float>(kb_bytes));
  } else {
    return fmt::format("{} Bytes", bytes);
  }
}

}  // namespace

namespace vlk {

void AssetCache::tick(std::chrono::nanoseconds) {
  for (auto &[tag, asset_info] : data_) {
    if (std::holds_alternative<PendingAsset>(asset_info.asset)) {
      auto &future = std::get<PendingAsset>(asset_info.asset);
      future.copy().match(
          [&](stx::Rc<Asset*> &&asset) {
            uint64_t asset_size = asset.get()->size_bytes();
            total_size_ += asset_size;
            VLK_LOG(
                "Asset with tag '{}' and size: {} has finished loading "
                "and added to asset cache",
                tag.as_str(), format_bytes_unit(asset_size));
            asset_info.asset = std::move(asset);
          },
          [&](stx::FutureError error) {
            if (error == stx::FutureError::Canceled) {
              asset_info.asset = CanceledAsset{};
              VLK_LOG("Loading of asset with tag '{}' has been canceled",
                      tag.as_str());
            }
          });
    } else {
    }
  }
}

}  // namespace vlk
