#pragma once

#include "SDL.h"
#include "stx/panic.h"

#define ASH_SDL_CHECK(...)                                                                    \
  do                                                                                          \
  {                                                                                           \
    if (!(__VA_ARGS__))                                                                       \
      ::stx::panic(#__VA_ARGS__ " failed. SDL Error:", ::std::string_view(::SDL_GetError())); \
  } while (false)
