#include "ashura/engine/engine.h"

int main()
{
  using namespace ash;
  Logger::init();
  CHECK(logger->add_sink(&stdio_sink));
  Scheduler::init(default_allocator, std::this_thread::get_id(),
                  span({0ns, 0ns}), span({0ns, 0ns, 0ns, 0ns, 0ns, 0ns}));

  Engine::init(
      default_allocator, nullptr,
      R"(C:\Users\rlama\Documents\workspace\oss\ashura\ashura\config.json)"_span,
      R"(C:\Users\rlama\Documents\workspace\oss\ashura\assets)"_span);
}