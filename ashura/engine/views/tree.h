/// SPDX-License-Identifier: MIT
#include "ashura/engine/views/text.h"

namespace ash
{
namespace ui
{

/// @brief Contains multiple views.
struct Tree
{
    Allocator allocator_;
    // TODO: object memory & layout + destructors

    auto & button(...);
    auto & check_box(...);
    auto & color_picker();
    auto & combo(...);
    auto & flex(...);
    auto & focus();
    auto & icon(...);
    auto & image(...);
    auto & input(...);
    auto & list(...);
    auto & modal(...);
    auto & plot(...);
    auto & radio(...);
    auto & scalar_box(...);
    auto & scroll_view(...);
    auto & slider(...);
    auto & space(...);
    auto & stack(...);
    auto & switch_box();
    auto & table(...);
    auto & text(...);

    template <typename ViewType, typename... Args>
    auto & view(...);
};

}    // namespace ui
}    // namespace ash
