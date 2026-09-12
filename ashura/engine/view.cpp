/// SPDX-License-Identifier: MIT
#include "ashura/engine/view.hpp"
#include "ashura/engine/color.hpp"

namespace ash
{
namespace ui
{

Option<void *> ViewSysScope::get_user_data(Span<u8 const> tag) const
{
    return user_data_map_.try_get(tag).map([](auto & d) { return d.get(); });
}

}    // namespace ui
}    // namespace ash
