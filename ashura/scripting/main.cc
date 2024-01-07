#include "ashura/span.h"

struct Module
{
  void (*init)(void *ctx)   = nullptr;
  void (*deinit)(void *ctx) = nullptr;
  void (*tick)(void *ctx)   = nullptr;
};

void shader0_init(void *ctx);
void shader0_deinit(void *ctx);
void shader0_tick(void *ctx);

Module const shader0_module{
    .init = shader0_init, .deinit = shader0_deinit, .tick = shader0_tick};

extern "C" ash::Span<Module> get_pack_modules();

// these don't need to use any os-optimized routines
// targets: x86-64-windows, x86-windows, arm64-linux, arm-linux
// needs to be able to schedule and call engine-functionalities
// must be independently shippable and modular
//
// what if they use different standard libraries, different calling conventions
// and different
//
// It shouldn't be OS-dependent nor use any environment-specific functionality.
// only those provided by the engine, thus making it isolated.
//
// this would require all engine's module-facing functionalities to use a
// standard calling convention, else dll-linking would fail.
//
//
//
// pass authoring and preprocessing, game and app logic
//
//
//
//
//
//
//
//
//
//