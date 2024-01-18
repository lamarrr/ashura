#pragma once
#include "ashura/allocator.h"

namespace ash
{
namespace gfx
{

template <typename K, typename V>
struct MapEntry
{
  K key;
  V value;
};

// linear-probing hash map
template <typename K, typename V>
void flat_map(MapEntry);

}        // namespace gfx
}        // namespace ash
