#pragma once

#include "SDL.h"
#include "stx/panic.h"

#define VLK_SDL_ENSURE(expr, additional_context_message)             \
  do {                                                               \
    if (!(expr))                                                     \
      ::stx::panic(additional_context_message ". SDL's Last Error:", \
                   SDL_GetError());                                  \
  } while (false)
