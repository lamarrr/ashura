#pragma once
#include <type_traits>

namespace ash
{

template <typename EnumType>
using enum_ut = std::underlying_type_t<EnumType>;

template <typename EnumType>
[[nodiscard]] constexpr enum_ut<EnumType> enum_uv(EnumType a)
{
  return static_cast<enum_ut<EnumType>>(a);
}

template <typename EnumType>
[[nodiscard]] constexpr enum_ut<EnumType> enum_uv_or(EnumType a, EnumType b)
{
  return static_cast<enum_ut<EnumType>>(enum_uv(a) | enum_uv(b));
}

template <typename EnumType>
[[nodiscard]] constexpr EnumType enum_or(EnumType a, EnumType b)
{
  return static_cast<EnumType>(enum_uv_or(a, b));
}

template <typename EnumType>
[[nodiscard]] constexpr enum_ut<EnumType> enum_uv_and(EnumType a, EnumType b)
{
  return static_cast<enum_ut<EnumType>>(enum_uv(a) & enum_uv(b));
}

template <typename EnumType>
[[nodiscard]] constexpr enum_ut<EnumType> enum_uv_toggle(EnumType a)
{
  return static_cast<enum_ut<EnumType>>(~enum_uv(a));
}

template <typename EnumType>
[[nodiscard]] constexpr EnumType enum_toggle(EnumType a)
{
  return static_cast<EnumType>(enum_uv_toggle(a));
}

template <typename EnumType>
[[nodiscard]] constexpr EnumType enum_and(EnumType a, EnumType b)
{
  return static_cast<EnumType>(enum_uv_and(a, b));
}

#define ASH_DEFINE_ENUM_BIT_OPS(enum_type)                                 \
  [[nodiscard]] constexpr enum_type operator|(enum_type a, enum_type b)    \
  {                                                                        \
    return ::ash::enum_or(a, b);                                           \
  }                                                                        \
                                                                           \
  [[nodiscard]] constexpr enum_type operator~(enum_type a)                 \
  {                                                                        \
    return ::ash::enum_toggle(a);                                          \
  }                                                                        \
                                                                           \
  [[nodiscard]] constexpr enum_type &operator|=(enum_type &a, enum_type b) \
  {                                                                        \
    a = a | b;                                                             \
    return a;                                                              \
  }                                                                        \
                                                                           \
  [[nodiscard]] constexpr enum_type operator&(enum_type a, enum_type b)    \
  {                                                                        \
    return ::ash::enum_and(a, b);                                          \
  }                                                                        \
                                                                           \
  [[nodiscard]] constexpr enum_type &operator&=(enum_type &a, enum_type b) \
  {                                                                        \
    a = a & b;                                                             \
    return a;                                                              \
  }

}        // namespace ash