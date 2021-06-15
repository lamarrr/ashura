#pragma once

#include "vlk/ui/font.h"
#include "vlk/ui/font_asset.h"

namespace vlk {
namespace ui {

using FontSource = std::variant<SystemFont, FileFont, MemoryFont,
                                FileTypefaceSource, MemoryTypefaceSource>;

}
}  // namespace vlk
