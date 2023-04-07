#pragma once
#include "ashura/primitives.h"
#include <chrono>
#include <string_view>

namespace ash
{

struct Context;

struct Plugin
{
  template <typename T>
  T *as()
  {
    T *ptr = dynamic_cast<T *>(this);
    ASH_CHECK(ptr != nullptr, "Invalid Plugin Type provided");
    return ptr;
  }

  virtual constexpr void on_startup(Context &context)
  {}

  virtual constexpr void tick(Context &context, std::chrono::nanoseconds interval)
  {}

  virtual constexpr void on_exit(Context &context)
  {}

  virtual constexpr std::string_view get_name()
  {
    return "Plugin";
  }

  virtual constexpr ~Plugin()
  {}

};

}        // namespace ash
