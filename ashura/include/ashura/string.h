#include <string_view>

#include "ashura/primitives.h"

namespace ash
{

using ascii_codepoint = u8;
using utf8_codepoint  = u32;

enum class StringEncoding : u8
{
  Ascii = 0,
  Utf8 = 1
};

struct ASCIIString
{};
struct ASCIIStringView
{};

struct StringView
{
  StringView(char const *static_string_literal);
  usize          ncodepoints = 0;
  usize          size        = 0;
  StringEncoding encoding    = StringEncoding::Ascii;
};

struct String
{
  String(char const *static_string_literal)
  {}
  usize ncodepoints = 0;
  usize size        = 0;
};

/// gets the unicode codepoint at iter and then advances iter to the next codepoint
///
constexpr uint32_t utf8_next(uint8_t const *&iter)
{
  if ((*iter & 0xF8) == 0xF0)
  {
    uint32_t c1 = *iter;
    iter++;
    uint32_t c2 = *iter;
    iter++;
    uint32_t c3 = *iter;
    iter++;
    uint32_t c4 = *iter;
    iter++;
    return c1 << 24 | c2 << 16 | c3 << 8 | c4;
  }
  else if ((*iter & 0xF0) == 0xE0)
  {
    uint32_t c1 = *iter;
    iter++;
    uint32_t c2 = *iter;
    iter++;
    uint32_t c3 = *iter;
    iter++;
    return c1 << 16 | c2 << 8 | c3;
  }
  else if ((*iter & 0xE0) == 0xC0)
  {
    uint32_t c1 = *iter;
    iter++;
    uint32_t c2 = *iter;
    iter++;
    return c1 << 8 | c2;
  }
  else
  {
    uint32_t c1 = *iter;
    iter++;
    return c1;
  }
}
//   /// Returns whether the given code unit represents an ASCII scalar
//   @_alwaysEmitIntoClient
//   @inline(__always)
//   public static func isASCII(_ x: CodeUnit) -> Bool {
//     return x & 0b1000_0000 == 0
//   }

//  public static func width(_ x: Unicode.Scalar) -> Int {
//     switch x.value {
//       case 0..<0x80: return 1
//       case 0x80..<0x0800: return 2
//       case 0x0800..<0x1_0000: return 3
//       default: return 4
//     }
//   }
//   @inline(__always)
//   @inlinable
//   public static func encode(
//     _ source: Unicode.Scalar
//   ) -> EncodedScalar? {
//     var c = source.value
//     if _fastPath(c < (1&<<7)) {
//       return EncodedScalar(_containing: UInt8(c))
//     }
//     var o = c & 0b0__0011_1111
//     c &>>= 6
//     o &<<= 8
//     if _fastPath(c < (1&<<5)) {
//       return EncodedScalar(_biasedBits: (o | c) &+ 0b0__1000_0001__1100_0001)
//     }
//     o |= c & 0b0__0011_1111
//     c &>>= 6
//     o &<<= 8
//     if _fastPath(c < (1&<<4)) {
//       return EncodedScalar(
//         _biasedBits: (o | c) &+ 0b0__1000_0001__1000_0001__1110_0001)
//     }
//     o |= c & 0b0__0011_1111
//     c &>>= 6
//     o &<<= 8
//     return EncodedScalar(
//       _biasedBits: (o | c ) &+ 0b0__1000_0001__1000_0001__1000_0001__1111_0001)
//   }

//   @inline(__always)
//   @inlinable
//   public static func decode(_ source: EncodedScalar) -> Unicode.Scalar {
//     switch source.count {
//     case 1:
//       return Unicode.Scalar(_unchecked: source._biasedBits &- 0x01)
//     case 2:
//       let bits = source._biasedBits &- 0x0101
//       var value = (bits & 0b0_______________________11_1111__0000_0000) &>> 8
//       value    |= (bits & 0b0________________________________0001_1111) &<< 6
//       return Unicode.Scalar(_unchecked: value)
//     case 3:
//       let bits = source._biasedBits &- 0x010101
//       var value = (bits & 0b0____________11_1111__0000_0000__0000_0000) &>> 16
//       value    |= (bits & 0b0_______________________11_1111__0000_0000) &>> 2
//       value    |= (bits & 0b0________________________________0000_1111) &<< 12
//       return Unicode.Scalar(_unchecked: value)
//     default:
//       _internalInvariant(source.count == 4)
//       let bits = source._biasedBits &- 0x01010101
//       var value = (bits & 0b0_11_1111__0000_0000__0000_0000__0000_0000) &>> 24
//       value    |= (bits & 0b0____________11_1111__0000_0000__0000_0000) &>> 10
//       value    |= (bits & 0b0_______________________11_1111__0000_0000) &<< 4
//       value    |= (bits & 0b0________________________________0000_0111) &<< 18
//       return Unicode.Scalar(_unchecked: value)
//     }
//   }

}        // namespace ash
