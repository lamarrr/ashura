#include "ashura/ecs.h"
#include "ashura/gfx.h"
#include "ashura/primitives.h"

namespace ash
{

namespace gfx
{

typedef struct PassEncoder_T *PassEncoder;
typedef struct PassCommand_T *PassCommand;

// can codegen work with this?
struct RenderEncoder
{
  struct Entity
  {
    f32   position;
    f32   velocity;
    f32   pulsation;
    Image texture;
    u32   descriptor;
  } entities[100];
  DescriptorHeap descriptor_heap;

  void init();

  // multiple batches of resources
  // add entity
  // remove entity
  // add render attribute to entity
  // remove render attribute from entity
  // modify attribute

  u32     add();
  void    remove(u32);
  Entity &get(u32);

  void tick(CommandEncoder, uint64_t frame_index)
  {
  }
};
}        // namespace gfx
}        // namespace ash
