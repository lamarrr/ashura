#pragma once

#include <variant>

#include "vlk/ui/image_asset.h"

namespace vlk {
namespace ui {

using ImageSource = std::variant<MemoryImageSource, FileImageSource>;

}
}  // namespace vlk
