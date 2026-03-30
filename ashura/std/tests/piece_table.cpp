/// SPDX-License-Identifier: MIT
#include "gtest/gtest.h"

#include "ashura/std/piece_table.hpp"

using namespace ash;

TEST(PieceTableTest, Extend)
{
  PieceTable8 piece{default_allocator};

  piece.append(static_rc(u8"AB 12"_s)).unwrap();
  piece.append(static_rc(u8"\n"_s)).unwrap();
  piece.append(static_rc(u8"676967 12345"_s)).unwrap();

  Vec<c8> result{default_allocator};

  piece.compact(Slice::all(), result).unwrap();

  EXPECT_EQ(piece.size(), 18);
  EXPECT_EQ(result.size(), 18);
  EXPECT_TRUE(mem::eq(result.view(), u8"AB 12\n676967 12345"_s));
}

TEST(PieceTableTest, Insert)
{
  PieceTable8 piece{default_allocator};

  piece.insert(0, static_rc(u8"AB 12"_s)).unwrap();

  {
    Vec<c8> result{default_allocator};

    piece.compact(Slice::all(), result).unwrap();

    EXPECT_TRUE(mem::eq(result.view(), u8"AB 12"_s));
  }

  piece.insert(5, static_rc(u8"3456"_s)).unwrap();

  {
    Vec<c8> result{default_allocator};
    piece.compact(Slice::all(), result).unwrap();
    EXPECT_TRUE(mem::eq(result.view(), u8"AB 123456"_s));
  }

  piece.insert(2, static_rc(u8"CDEFGH"_s)).unwrap();

  {
    Vec<c8> result{default_allocator};

    piece.compact(Slice::all(), result).unwrap();
    EXPECT_TRUE(mem::eq(result.view(), u8"ABCDEFGH 123456"_s));
  }
}

TEST(PieceTableTest, Erase)
{
  PieceTable8 piece{default_allocator};

  piece.append(static_rc(u8"AB"_s)).unwrap();
  piece.append(static_rc(u8"CDEFGH"_s)).unwrap();
  piece.append(static_rc(u8"IJKLM"_s)).unwrap();

  {
    PieceTable8 p{default_allocator};

    piece.clone(Slice::all(), p).unwrap();

    p.erase(Slice::slice(3, 2));
    Vec<c8> result{default_allocator};

    p.compact(Slice::all(), result).unwrap();
    EXPECT_TRUE(mem::eq(result.view(), u8"ABCFGHIJKLM"_s));
  }

  {
    PieceTable8 p{default_allocator};

    piece.clone(Slice::all(), p).unwrap();

    p.erase(Slice::slice(1, 2));
    Vec<c8> result{default_allocator};

    p.compact(Slice::all(), result).unwrap();
    EXPECT_TRUE(mem::eq(result.view(), u8"ADEFGHIJKLM"_s));
  }

  {
    PieceTable8 p{default_allocator};

    piece.clone(Slice::all(), p).unwrap();

    p.erase(Slice::slice(1, 7));
    Vec<c8> result{default_allocator};

    p.compact(Slice::all(), result).unwrap();
    EXPECT_TRUE(mem::eq(result.view(), u8"AIJKLM"_s));
  }

  {
    PieceTable8 p{default_allocator};

    piece.clone(Slice::all(), p).unwrap();

    p.erase(Slice::slice(1, 8));
    Vec<c8> result{default_allocator};

    p.compact(Slice::all(), result).unwrap();
    EXPECT_TRUE(mem::eq(result.view(), u8"AJKLM"_s));
  }

  {
    PieceTable8 p{default_allocator};

    piece.clone(Slice::all(), p).unwrap();

    p.erase(Slice::slice(2, 6));
    Vec<c8> result{default_allocator};

    p.compact(Slice::all(), result).unwrap();
    EXPECT_TRUE(mem::eq(result.view(), u8"ABIJKLM"_s));
  }

  {
    PieceTable8 p{default_allocator};

    piece.clone(Slice::all(), p).unwrap();

    p.erase(Slice::all());
    Vec<c8> result{default_allocator};

    p.compact(Slice::all(), result).unwrap();
    EXPECT_TRUE(mem::eq(result.view(), u8""_s));
  }
}

TEST(PieceTableTest, Compact)
{
  PieceTable8 piece{default_allocator};

  piece.append(static_rc(u8"AB"_s)).unwrap();
  piece.append(static_rc(u8"CDEFGH"_s)).unwrap();
  piece.append(static_rc(u8"IJKLM"_s)).unwrap();
  piece.append(static_rc(u8"NOPQR"_s)).unwrap();
  piece.append(static_rc(u8"STUVW"_s)).unwrap();
  piece.append(static_rc(u8"XYZ"_s)).unwrap();
  piece.append(static_rc(u8" 012"_s)).unwrap();
  piece.append(static_rc(u8"3456"_s)).unwrap();
  piece.append(static_rc(u8"789"_s)).unwrap();

  {
    Vec<c8> result{default_allocator};

    piece.compact(Slice::slice(0, 5), result).unwrap();
    EXPECT_TRUE(mem::eq(result.view(), u8"ABCDE"_s));
  }

  {
    Vec<c8> result{default_allocator};

    piece.compact(Slice::slice(4, 5), result).unwrap();
    EXPECT_TRUE(mem::eq(result.view(), u8"EFGHI"_s));
  }
}

TEST(PieceTableTest, Clone)
{
  PieceTable8 piece{default_allocator};

  piece.append(static_rc(u8"AB"_s)).unwrap();
  piece.append(static_rc(u8"CDEFGH"_s)).unwrap();
  piece.append(static_rc(u8"IJKLM"_s)).unwrap();
  piece.append(static_rc(u8"NOPQR"_s)).unwrap();
  piece.append(static_rc(u8"STUVW"_s)).unwrap();
  piece.append(static_rc(u8"XYZ"_s)).unwrap();
  piece.append(static_rc(u8" 012"_s)).unwrap();
  piece.append(static_rc(u8"3456"_s)).unwrap();
  piece.append(static_rc(u8"789"_s)).unwrap();

  {
    PieceTable8 clone{default_allocator};
    piece.clone(Slice::slice(4, 10), clone).unwrap();

    Vec<c8> result{default_allocator};
    clone.compact(Slice::all(), result).unwrap();
    EXPECT_TRUE(mem::eq(result.view(), u8"EFGHIJKLMN"_s));
  }
}
