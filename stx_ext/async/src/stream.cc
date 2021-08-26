#include "stx/stream.h"

void ff() {
  stx::StreamState<int> stream;
  stream.user____poll(0);
  stream.generator____yield(nullptr, true);
  stream.generator____yield(nullptr, true);
  stream.generator____yield(nullptr, true);
  stream.generator____yield(nullptr, true);
  stream.generator____yield(nullptr, true);
  stream.generator____yield(nullptr, true);
  stream.generator____yield(nullptr, true);
}
