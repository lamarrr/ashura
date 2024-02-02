#pragma once
#include "ashura/std/types.h"

namespace ash
{

typedef struct System_T       *System;
typedef struct SystemInterface SystemInterface;
typedef struct SystemImpl      SystemImpl;

struct SystemInterface
{
  void (*init)(System)      = nullptr;
  void (*deinit)(System)    = nullptr;
  void (*tick)(System, u64) = nullptr;
};

struct SystemImpl
{
  char const            *name      = nullptr;
  u64                    version   = 0;
  System                 system    = nullptr;
  SystemInterface const *interface = nullptr;
};

}        // namespace ash
