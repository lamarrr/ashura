#pragma once
#include <chrono>
#include <string_view>

namespace ash
{

struct Plugin
{
  template <typename T>
  T *as()
  {
    T *ptr = dynamic_cast<T *>(this);
    ASH_CHECK(ptr != nullptr, "Invalid Plugin Type provided");
    return ptr;
  }

  virtual constexpr void on_startup()
  {}

  virtual constexpr void tick(std::chrono::nanoseconds interval)
  {}

  virtual constexpr void on_exit()
  {}

  virtual constexpr std::string_view get_id()
  {
    return "Plugin";
  }

  virtual constexpr ~Plugin()
  {}
};

}        // namespace ash
