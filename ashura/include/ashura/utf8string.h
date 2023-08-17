#include <string_view>

#include "ashura/primitives.h"

namespace ash
{

struct Utf8StringView
{
  Utf8StringView(   );
  size_t ncodepoints = 0;
};

struct Utf8String
{
  Utf8String(char8_t const *static_string_literal)
  {}

  operator std::string_view() const
  {}

  size_t ncodepoints = 0;
  size_t size        = 0;
};

}        // namespace ash
