#pragma once
#include "ashura/std/types.h"
#include <atomic>

namespace ash
{

struct StopToken
{
  std::atomic<bool> req_ = false;

  bool is_stop_requested() const
  {
    return req_.load(std::memory_order_relaxed) &&
           req_.load(std::memory_order_acquire);
  }

  void request_stop()
  {
    req_.store(true, std::memory_order_release);
  }
};

}        // namespace ash