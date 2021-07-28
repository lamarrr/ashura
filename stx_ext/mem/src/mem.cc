#include "stx/mem.h"

#include <iostream>

void xlaunder(void*);

auto b() {
  auto y = stx::mem::make_rc(2);
  xlaunder(&y);
  return y;
}

struct Easy {
  int a = 7;
  int b = 6;
} easy;

auto bb() {
  auto y = stx::mem::make_rc_for_static(easy);
  xlaunder(&y);
  std::cout << y.get()->a << std::endl;
  return y;
}




void xlaunder(void*){}