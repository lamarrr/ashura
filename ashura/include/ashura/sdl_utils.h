#pragma once

#include "SDL.h"
#include "stx/panic.h"

#define ASH_SDL_CHECK(expr, ...)                                 \
  do                                                             \
  {                                                              \
    if (!(expr))                                                 \
      ::stx::panic(#expr " failed. " #__VA_ARGS__ " SDL Error:", \
                   ::std::string_view(::SDL_GetError()));        \
  } while (false)
