#pragma once

#include <string>
#include <utility>

#include "ashura/primitives.h"

namespace ash
{

constexpr char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                "abcdefghijklmnopqrstuvwxyz"
                                "0123456789+/";

inline bool is_base64(char c)
{
  return isalnum(c) || (c == '+') || (c == '/');
}

std::string base64_encode(Span<char const> data)
{
  usize       in_len          = data.size();
  char const *bytes_to_encode = data.data();

  char char_array_3[3];
  char char_array_4[4];
  i64  i = 0;

  std::string ret;

  while (in_len--)
  {
    char_array_3[i++] = *(bytes_to_encode++);
    if (i == 3)
    {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] =
          ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] =
          ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for (i = 0; (i < 4); i++)
      {
        ret.push_back(base64_chars[char_array_4[i]]);
      }
      i = 0;
    }
  }

  if (i != 0)
  {
    for (i64 j = i; j < 3; j++)
    {
      char_array_3[j] = '\0';
    }

    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] =
        ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] =
        ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
    char_array_4[3] = char_array_3[2] & 0x3f;

    for (i64 j = 0; (j < i + 1); j++)
    {
      ret += base64_chars[char_array_4[j]];
    }

    while ((i++ < 3))
    {
      ret.push_back('=');
    }
  }

  return ret;
}

std::string base64_decode(Span<char const> enc)
{
  char const *encoded_string = enc.data();
  usize       in_len         = enc.size();

  char char_array_4[4];
  char char_array_3[3];
  i64  i  = 0;
  i64  in = 0;

  std::string ret;

  while (in_len-- && (encoded_string[in] != '=') &&
         is_base64(encoded_string[in]))
  {
    char_array_4[i++] = encoded_string[in];
    in++;
    if (i == 4)
    {
      for (i = 0; i < 4; i++)
      {
        char_array_4[i] = static_cast<char>(
            strchr(base64_chars, char_array_4[i]) - base64_chars);
      }

      char_array_3[0] =
          (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] =
          ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

      for (i = 0; (i < 3); i++)
      {
        ret.push_back(char_array_3[i]);
      }
      i = 0;
    }
  }

  if (i != 0)
  {
    for (i64 j = i; j < 4; j++)
    {
      char_array_4[j] = 0;
    }

    for (i64 j = 0; j < 4; j++)
    {
      char_array_4[j] = static_cast<char>(
          strchr(base64_chars, char_array_4[j]) - base64_chars);
    }

    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
    char_array_3[1] =
        ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

    for (i64 j = 0; (j < i - 1); j++)
    {
      ret.push_back(char_array_3[j]);
    }
  }

  return ret;
}

}        // namespace ash
