#pragma once
#include "ashura/allocator.h"

namespace ash
{
namespace gfx
{

template <typename T, typename SizeT, typename IdT = SizeT, typename Pool>
[[nodiscard]] bool sparse_set_add_entity(AllocatorImpl const &allocator, Pool *pools,
                           SizeT num_pools, SizeT *&free_ids,
                           SizeT &num_free_ids, SizeT *&id_mapping)
{
}

}        // namespace gfx

}        // namespace ash
