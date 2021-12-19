
#include "vlk/image_asset.h"

#include "include/core/SkImage.h"
#include "vlk/utils.h"

namespace vlk {

ImageAsset::ImageAsset(sk_sp<SkImage> raw_image) : raw_{std::move(raw_image)} {
  VLK_ENSURE(raw_ != nullptr);
  Asset::size_bytes_ = raw_->imageInfo().computeMinByteSize();
}
}  // namespace vlk

/*
    : load_result_{std::move(load_result)} {
  load_result_.as_cref().match(
      [&](sk_sp<SkImage> const& image) {

      },
      [&](ImageLoadError) { Asset::update_size(0); });
}



}


// TODO(lamarrr): WEBP support

}  // namespace ui
}  // namespace vlk

*/