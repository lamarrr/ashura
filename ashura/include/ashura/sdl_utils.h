#pragma once

#include "SDL.h"
#include "stx/panic.h"

#define ASR_SDL_CHECK(expr, additional_context_message)              \
  do {                                                               \
    if (!(expr))                                                     \
      ::stx::panic(additional_context_message ". SDL's Last Error:", \
                   ::std::string_view(::SDL_GetError()));            \
  } while (false)
