#include "ashura/ecs.h"

#include <cinttypes>
#include <span>

typedef struct CommandEncoder_T *CommandEncoder;
struct PassEncoder;
typedef struct PassCommand_T         *PassCommand;
typedef struct DescriptorSetLayout_T *DescriptorSetLayout;
typedef struct DescriptorSet_T       *DescriptorSet;
typedef struct DescriptorPool_T      *DescriptorPool;
typedef struct Image_T               *Image;

struct DescriptorHeap
{
  DescriptorSetLayout layout;
  DescriptorPool     *pools;
  uint32_t            num_pools;
  uint32_t            pool_capacity = 1024;
  DescriptorSet      *sets;
  uint64_t           *last_use_ticks;        // < ctx.trailing_frame_tick
  uint32_t            num_sets;
  uint32_t           *free_sets;
  uint32_t            num_free_sets;
  uint32_t           *released_indices;
  uint32_t            num_released;

  // pop index from pool_free_sets if any, otherwise create new pool and add and allocate new free sets from that
  uint32_t add();
  // fetch from sets list, each set is ordered by the pool index
  DescriptorSet get(uint32_t set);
  // add to released indices
  void release(uint32_t set);
  // set last used tick for descriptor set
  void update_tick(uint32_t set, uint64_t frame_index);
  // for all sets in released indices if last used tick < trailing_frame_tick, move to free indices
  // descriptor set can't be reused, destroyed or modified until its no longer in use
  void tick(uint64_t trailing_frame_tick);
  void destroy();
};

// can codegen work with this?
struct RenderEncoder
{
  struct Entity
  {
    float    position;
    float    velocity;
    float    pulsation;
    Image    texture;
    uint32_t descriptor_set;
  } entities[100];
  DescriptorManager descriptor_manager;

  // multiple batches of resources
  // add entity
  // remove entity
  // add render attribute to entity
  // remove render attribute from entity
  // modify attribute

  uint32_t add();
  void     remove(uint32_t);
  Entity  &get(uint32_t);

  void tick(CommandEncoder)
  {
  }
};
