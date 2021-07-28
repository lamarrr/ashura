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

auto c() {
  auto h = stx::mem::make_unique(4);
  return h;
}

char const* str = "hello world";

auto g() { stx::mem::make_static_string_rc(str); }

int main() {
  b();
  bb();
  c();
}

void xlaunder(void*) {}
