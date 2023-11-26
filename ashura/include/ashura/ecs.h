#pragma once
#include "ashura/primitives.h"

namespace ash
{
namespace ecs
{

using entity_id            = u64;
using component_id         = u64;
using component_tag        = u64;
using component_destructor = void (*)(u64 const *free_mask, void *reps, u64 count);
using component_relocator  = void (*)(u64 const *free_mask, void *reps, u64 count);

struct SampleComponent
{
  static constexpr component_id COMPONENT_ID = 0;
};

// guaranteed to be contigous in memory both component-wise and entity-wise
//
// system can decide to only process batches
// should registry require batches???
//
// they are thus not disjoint in memory
struct entity_batch
{
  // must all share the same components, and freed together
  // data may change differently
  // can share signals?
  entity_id first_entity = 0;
  u64       nentities    = 0;
};

struct component_batch
{
  component_id first_component = 0;
};

// xxxxx systems are thus free to re-organize the components

// TODO(lamarrr):
// - cache preloading multiple components into memory
// - non-aliasing specification
// - iterator/cursor
struct ComponentTable
{
  // how the type tag is defined or used is left to the application
  component_tag tag = 0;

  component_destructor destructor = nullptr;

  u64 unit_size     = 0;
  u64 count         = 0;
  u64 capacity      = 0;
  u64 nsignals      = 0;
  u64 nfree_indices = 0;

  void *data = nullptr;

  u64 *freed_mask = nullptr;

  // capacity = count
  // indices of available slots
  // freeing: destroy(data[i]); free_indices[nfree_indices] = i; freed_mask[i >> 6] ^= (i & 63);
  // nfree_indices++; reclamation: i = free_indices[nfree_indices - 1]; freed_mask[i >> 6] ^= (i &
  // 63); data[i] = component_data;  nfree_indices--;
  u64 *free_indices = nullptr;

  // default state for the signals
  u64 *default_signal_states = nullptr;

  // signals to-and-from the systems.
  // it is left to the component and the system to interpret and use these signals.
  // example signals are: component_disabled, component_updated, component_has_request, etc.
  u64 **signals = nullptr;
};

// THE GOAL IS FOR FAST ACCESS OF COMPONENTS, NOT OF ENTITIES
// SYSTEMS OPERATE ON COMPONENTS, NOT ENTITIES
// COMPONENTS can only be disabled, not removed
/// encourages memory re-use
struct Registry
{
  ComponentTable *components          = nullptr;
  u64             components_capacity = 0;
  u64             ncomponents         = 0;

  // maps entities to the indices of its components, U64_MAX if entity does not have a component
  // check entity has component: entity_component_map[component_id][entity_id] != U64_MAX;
  // get entity component: components[component_id][entity_component_map[component_id][entity_id]]
  // check entity validity: (entity_id < nentities) && ((entity_free_mask[entity_id >> 6] >>
  // (entity_id & 63)) & 1) free entity: free components && entity_free_mask[]
  u64 **entity_component_map = nullptr;
  u64  *entity_free_mask =
      nullptr;        // we don't want to waste indices and have to update indirections
  u64 *free_entity_indices = nullptr;
  u64  entities_capacity   = 0;
  u64  nentities           = 0;
  u64  nfree_entities      = 0;
};

// say for example we want a system that processes elements in batches
// batches may not be speci

// entity/registry batches are somewhat useless?
// disjoint-entity component batching are useful and what we need as some systems i.e. particle
// processing systems won't function properly without them we can have reserves of components
}        // namespace ecs
}        // namespace ash
