

#include "vlk/font_asset.h"

#include <utility>
#include <vector>

#include "include/core/SkTypeface.h"

namespace vlk {

namespace {
inline uint64_t estimate_typeface_size(sk_sp<SkTypeface> const& typeface) {
  uint64_t byte_size = 0U;
  uint64_t num_tables = typeface->countTables();
  std::vector<SkFontTableTag> table_tags;
  table_tags.resize(num_tables);

  for (uint64_t i = 0; i < num_tables; i++) {
    typeface->getTableTags(table_tags.data());
  }

  for (auto const& table_tag : table_tags) {
    byte_size += typeface->getTableSize(table_tag);
  }

  return byte_size;
}
}  // namespace

FontAsset::FontAsset(sk_sp<SkTypeface> raw_typeface)
    : Asset{}, raw_{std::move(raw_typeface)} {
  Asset::size_bytes_ = estimate_typeface_size(raw_);
}

}  // namespace vlk

//#include "vlk/ui/font_asset.h"

//#include <atomic>

// #include "include/core/SkFontMgr.h"
// #include "include/core/SkFontStyle.h"
/*
namespace vlk {
namespace ui {


namespace impl {


std::unique_ptr<Asset> TypefaceLoader::load(AssetLoadArgs const& args) const {
  auto const& load_args =
      upcast<TypefaceLoadArgs const>(args).expect("Upcast failed").get();
  if (load_args.is_mem()) {
    auto const& source_data =
        std::get<std::shared_ptr<MemoryTypefaceSourceData const>>(
            load_args.data_ref());
    return std::make_unique<TypefaceAsset>(
        TypefaceAsset{load_typeface_from_memory(source_data->bytes)});
  } else if (load_args.is_file()) {
    auto const& source_data =
        std::get<std::shared_ptr<FileTypefaceSourceData const>>(
            load_args.data_ref());
    return std::make_unique<TypefaceAsset>(
        TypefaceAsset{load_typeface_from_file(source_data->path)});
  } else if (load_args.is_system()) {
    auto const& source_data =
        std::get<std::shared_ptr<SystemFontData const>>(load_args.data_ref());
    return std::make_unique<TypefaceAsset>(TypefaceAsset{
        load_system_typeface(source_data->family, source_data->style)});
  } else {
    VLK_PANIC();
  }
}

std::shared_ptr<TypefaceLoader const> TypefaceLoader::get_default() {
  static std::shared_ptr<TypefaceLoader const> loader =
      std::make_shared<TypefaceLoader const>(TypefaceLoader{});

  return loader;
}

}  // namespace impl

}  // namespace ui
}  // namespace vlk

struct QueueMutex {
  // use a contention metric to determine if to spin for a period of time or
  // sleep
  std::atomic<uint64_t> num_accesses;
};
*/
