/// SPDX-License-Identifier: MIT
/// Meta-Generated Source Code
// clang-format off
#pragma once

namespace ash
{

inline constexpr unsigned int MAX_PACK_SIZE = 32;

namespace intr
{

template <unsigned int I, typename... T>
requires((I < sizeof...(T)) && (sizeof...(T) <= MAX_PACK_SIZE))
struct index_pack;

template<typename E0, typename... E>
struct index_pack<0, E0, E...>
{
  using Type = E0;
};

template<typename E0, typename E1, typename... E>
struct index_pack<1, E0, E1, E...>
{
  using Type = E1;
};

template<typename E0, typename E1, typename E2, typename... E>
struct index_pack<2, E0, E1, E2, E...>
{
  using Type = E2;
};

template<typename E0, typename E1, typename E2, typename E3, typename... E>
struct index_pack<3, E0, E1, E2, E3, E...>
{
  using Type = E3;
};

template<typename E0, typename E1, typename E2, typename E3, typename E4, typename... E>
struct index_pack<4, E0, E1, E2, E3, E4, E...>
{
  using Type = E4;
};

template<typename E0, typename E1, typename E2, typename E3, typename E4, typename E5, typename... E>
struct index_pack<5, E0, E1, E2, E3, E4, E5, E...>
{
  using Type = E5;
};

template<typename E0, typename E1, typename E2, typename E3, typename E4, typename E5, typename E6, typename... E>
struct index_pack<6, E0, E1, E2, E3, E4, E5, E6, E...>
{
  using Type = E6;
};

template<typename E0, typename E1, typename E2, typename E3, typename E4, typename E5, typename E6, typename E7, typename... E>
struct index_pack<7, E0, E1, E2, E3, E4, E5, E6, E7, E...>
{
  using Type = E7;
};

template<typename E0, typename E1, typename E2, typename E3, typename E4, typename E5, typename E6, typename E7, typename E8, typename... E>
struct index_pack<8, E0, E1, E2, E3, E4, E5, E6, E7, E8, E...>
{
  using Type = E8;
};

template<typename E0, typename E1, typename E2, typename E3, typename E4, typename E5, typename E6, typename E7, typename E8, typename E9, typename... E>
struct index_pack<9, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E...>
{
  using Type = E9;
};

template<typename E0, typename E1, typename E2, typename E3, typename E4, typename E5, typename E6, typename E7, typename E8, typename E9, typename E10, typename... E>
struct index_pack<10, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E...>
{
  using Type = E10;
};

template<typename E0, typename E1, typename E2, typename E3, typename E4, typename E5, typename E6, typename E7, typename E8, typename E9, typename E10, typename E11, typename... E>
struct index_pack<11, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11, E...>
{
  using Type = E11;
};

template<typename E0, typename E1, typename E2, typename E3, typename E4, typename E5, typename E6, typename E7, typename E8, typename E9, typename E10, typename E11, typename E12, typename... E>
struct index_pack<12, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11, E12, E...>
{
  using Type = E12;
};

template<typename E0, typename E1, typename E2, typename E3, typename E4, typename E5, typename E6, typename E7, typename E8, typename E9, typename E10, typename E11, typename E12, typename E13, typename... E>
struct index_pack<13, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11, E12, E13, E...>
{
  using Type = E13;
};

template<typename E0, typename E1, typename E2, typename E3, typename E4, typename E5, typename E6, typename E7, typename E8, typename E9, typename E10, typename E11, typename E12, typename E13, typename E14, typename... E>
struct index_pack<14, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11, E12, E13, E14, E...>
{
  using Type = E14;
};

template<typename E0, typename E1, typename E2, typename E3, typename E4, typename E5, typename E6, typename E7, typename E8, typename E9, typename E10, typename E11, typename E12, typename E13, typename E14, typename E15, typename... E>
struct index_pack<15, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11, E12, E13, E14, E15, E...>
{
  using Type = E15;
};

template<typename E0, typename E1, typename E2, typename E3, typename E4, typename E5, typename E6, typename E7, typename E8, typename E9, typename E10, typename E11, typename E12, typename E13, typename E14, typename E15, typename E16, typename... E>
struct index_pack<16, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11, E12, E13, E14, E15, E16, E...>
{
  using Type = E16;
};

template<typename E0, typename E1, typename E2, typename E3, typename E4, typename E5, typename E6, typename E7, typename E8, typename E9, typename E10, typename E11, typename E12, typename E13, typename E14, typename E15, typename E16, typename E17, typename... E>
struct index_pack<17, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11, E12, E13, E14, E15, E16, E17, E...>
{
  using Type = E17;
};

template<typename E0, typename E1, typename E2, typename E3, typename E4, typename E5, typename E6, typename E7, typename E8, typename E9, typename E10, typename E11, typename E12, typename E13, typename E14, typename E15, typename E16, typename E17, typename E18, typename... E>
struct index_pack<18, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11, E12, E13, E14, E15, E16, E17, E18, E...>
{
  using Type = E18;
};

template<typename E0, typename E1, typename E2, typename E3, typename E4, typename E5, typename E6, typename E7, typename E8, typename E9, typename E10, typename E11, typename E12, typename E13, typename E14, typename E15, typename E16, typename E17, typename E18, typename E19, typename... E>
struct index_pack<19, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11, E12, E13, E14, E15, E16, E17, E18, E19, E...>
{
  using Type = E19;
};

template<typename E0, typename E1, typename E2, typename E3, typename E4, typename E5, typename E6, typename E7, typename E8, typename E9, typename E10, typename E11, typename E12, typename E13, typename E14, typename E15, typename E16, typename E17, typename E18, typename E19, typename E20, typename... E>
struct index_pack<20, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11, E12, E13, E14, E15, E16, E17, E18, E19, E20, E...>
{
  using Type = E20;
};

template<typename E0, typename E1, typename E2, typename E3, typename E4, typename E5, typename E6, typename E7, typename E8, typename E9, typename E10, typename E11, typename E12, typename E13, typename E14, typename E15, typename E16, typename E17, typename E18, typename E19, typename E20, typename E21, typename... E>
struct index_pack<21, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11, E12, E13, E14, E15, E16, E17, E18, E19, E20, E21, E...>
{
  using Type = E21;
};

template<typename E0, typename E1, typename E2, typename E3, typename E4, typename E5, typename E6, typename E7, typename E8, typename E9, typename E10, typename E11, typename E12, typename E13, typename E14, typename E15, typename E16, typename E17, typename E18, typename E19, typename E20, typename E21, typename E22, typename... E>
struct index_pack<22, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11, E12, E13, E14, E15, E16, E17, E18, E19, E20, E21, E22, E...>
{
  using Type = E22;
};

template<typename E0, typename E1, typename E2, typename E3, typename E4, typename E5, typename E6, typename E7, typename E8, typename E9, typename E10, typename E11, typename E12, typename E13, typename E14, typename E15, typename E16, typename E17, typename E18, typename E19, typename E20, typename E21, typename E22, typename E23, typename... E>
struct index_pack<23, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11, E12, E13, E14, E15, E16, E17, E18, E19, E20, E21, E22, E23, E...>
{
  using Type = E23;
};

template<typename E0, typename E1, typename E2, typename E3, typename E4, typename E5, typename E6, typename E7, typename E8, typename E9, typename E10, typename E11, typename E12, typename E13, typename E14, typename E15, typename E16, typename E17, typename E18, typename E19, typename E20, typename E21, typename E22, typename E23, typename E24, typename... E>
struct index_pack<24, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11, E12, E13, E14, E15, E16, E17, E18, E19, E20, E21, E22, E23, E24, E...>
{
  using Type = E24;
};

template<typename E0, typename E1, typename E2, typename E3, typename E4, typename E5, typename E6, typename E7, typename E8, typename E9, typename E10, typename E11, typename E12, typename E13, typename E14, typename E15, typename E16, typename E17, typename E18, typename E19, typename E20, typename E21, typename E22, typename E23, typename E24, typename E25, typename... E>
struct index_pack<25, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11, E12, E13, E14, E15, E16, E17, E18, E19, E20, E21, E22, E23, E24, E25, E...>
{
  using Type = E25;
};

template<typename E0, typename E1, typename E2, typename E3, typename E4, typename E5, typename E6, typename E7, typename E8, typename E9, typename E10, typename E11, typename E12, typename E13, typename E14, typename E15, typename E16, typename E17, typename E18, typename E19, typename E20, typename E21, typename E22, typename E23, typename E24, typename E25, typename E26, typename... E>
struct index_pack<26, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11, E12, E13, E14, E15, E16, E17, E18, E19, E20, E21, E22, E23, E24, E25, E26, E...>
{
  using Type = E26;
};

template<typename E0, typename E1, typename E2, typename E3, typename E4, typename E5, typename E6, typename E7, typename E8, typename E9, typename E10, typename E11, typename E12, typename E13, typename E14, typename E15, typename E16, typename E17, typename E18, typename E19, typename E20, typename E21, typename E22, typename E23, typename E24, typename E25, typename E26, typename E27, typename... E>
struct index_pack<27, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11, E12, E13, E14, E15, E16, E17, E18, E19, E20, E21, E22, E23, E24, E25, E26, E27, E...>
{
  using Type = E27;
};

template<typename E0, typename E1, typename E2, typename E3, typename E4, typename E5, typename E6, typename E7, typename E8, typename E9, typename E10, typename E11, typename E12, typename E13, typename E14, typename E15, typename E16, typename E17, typename E18, typename E19, typename E20, typename E21, typename E22, typename E23, typename E24, typename E25, typename E26, typename E27, typename E28, typename... E>
struct index_pack<28, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11, E12, E13, E14, E15, E16, E17, E18, E19, E20, E21, E22, E23, E24, E25, E26, E27, E28, E...>
{
  using Type = E28;
};

template<typename E0, typename E1, typename E2, typename E3, typename E4, typename E5, typename E6, typename E7, typename E8, typename E9, typename E10, typename E11, typename E12, typename E13, typename E14, typename E15, typename E16, typename E17, typename E18, typename E19, typename E20, typename E21, typename E22, typename E23, typename E24, typename E25, typename E26, typename E27, typename E28, typename E29, typename... E>
struct index_pack<29, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11, E12, E13, E14, E15, E16, E17, E18, E19, E20, E21, E22, E23, E24, E25, E26, E27, E28, E29, E...>
{
  using Type = E29;
};

template<typename E0, typename E1, typename E2, typename E3, typename E4, typename E5, typename E6, typename E7, typename E8, typename E9, typename E10, typename E11, typename E12, typename E13, typename E14, typename E15, typename E16, typename E17, typename E18, typename E19, typename E20, typename E21, typename E22, typename E23, typename E24, typename E25, typename E26, typename E27, typename E28, typename E29, typename E30, typename... E>
struct index_pack<30, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11, E12, E13, E14, E15, E16, E17, E18, E19, E20, E21, E22, E23, E24, E25, E26, E27, E28, E29, E30, E...>
{
  using Type = E30;
};

template<typename E0, typename E1, typename E2, typename E3, typename E4, typename E5, typename E6, typename E7, typename E8, typename E9, typename E10, typename E11, typename E12, typename E13, typename E14, typename E15, typename E16, typename E17, typename E18, typename E19, typename E20, typename E21, typename E22, typename E23, typename E24, typename E25, typename E26, typename E27, typename E28, typename E29, typename E30, typename E31, typename... E>
struct index_pack<31, E0, E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11, E12, E13, E14, E15, E16, E17, E18, E19, E20, E21, E22, E23, E24, E25, E26, E27, E28, E29, E30, E31, E...>
{
  using Type = E31;
};


} // namespace intr
} // namespace ash

  