#pragma once
#include "ashura/std/types.h"

namespace ash
{

typedef struct System_T       *System;
typedef struct SystemInterface SystemInterface;
typedef struct SystemImpl      SystemImpl;

struct SystemInterface
{
  void (*init)(System self)      = nullptr;
  void (*deinit)(System self)    = nullptr;
  void (*tick)(System self, u64) = nullptr;
};

struct SystemImpl
{
  char const            *name      = nullptr;
  u64                    version   = 0;
  System                 self      = nullptr;
  SystemInterface const *interface = nullptr;
};

}        // namespace ash
