#pragma once
#include "ashura/primitives.h"
#include <chrono>
#include <string_view>

namespace ash
{

struct Context;

struct Subsystem
{
  template <typename T>
  T *as()
  {
    T *ptr = dynamic_cast<T *>(this);
    ASH_CHECK(ptr != nullptr, "Invalid Subsystem Type provided");
    return ptr;
  }

  virtual constexpr void on_startup(Context &ctx)
  {
  }

  virtual constexpr void tick(Context &ctx, std::chrono::nanoseconds interval)
  {
  }

  virtual constexpr void on_exit(Context &ctx)
  {
  }

  virtual constexpr std::string_view get_name()
  {
    return "Subsystem";
  }

  virtual constexpr ~Subsystem()
  {
  }
};

}        // namespace ash
