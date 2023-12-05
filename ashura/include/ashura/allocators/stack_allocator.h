#pragma once
#include "ashura/primitives.h"

namespace ash
{

struct StackAllocator
{
  ~StackAllocator()
  {
  }
  void grow();
  void shrink();
};