#include "stx/vec.h"
static void f() {
  stx::Vec<int> vec{stx::os_allocator};

  vec.push(3);

  vec.push_inplace(3);
  vec.reserve(444).unwrap();
  vec.span();
  vec.at(1).unwrap().get() = 0;

  stx::FixedVec<int> g{stx::os_allocator, nullptr, 0};

  g.push_inplace(4783).unwrap();
}
