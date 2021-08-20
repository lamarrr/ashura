

#include "stx/vec.h"

#include "gtest/gtest.h"

using stx::Vec;

struct Life {
  Life() { inc(); }

  Life(Life const&) { inc(); }

  Life& operator=(Life const&) {
    clobber();
    return *this;
  }

  Life(Life&&) { inc(); }

  Life& operator=(Life&&) {
    clobber();
    return *this;
  }

  ~Life() { dec(); }

  static int64_t add(int64_t inc) {
    static int64_t count = 0;
    count += inc;
    return count;
  }

  static void inc() { EXPECT_NE(add(1), 0); }

  static void dec() { EXPECT_GE(add(-1), 0); }

  static void clobber() {
    int f = 9;
    *(volatile int*)(&f) = 82;
  }
};

#define EXPECT_VALID_VEC(vec)        \
  EXPECT_GE(vec.end(), vec.begin()); \
  EXPECT_GE(vec.capacity(), vec.size())

TEST(VecTest, Destructor) {
  {
    Vec<int> vec{stx::os_allocator};

    for (size_t i = 0; i < 10000; i++) vec.push_inplace(8).unwrap();

    EXPECT_EQ(vec.size(), 10000);
    EXPECT_VALID_VEC(vec);
  }

  {
    Vec<int> vec{stx::os_allocator};

    EXPECT_VALID_VEC(vec);
  }
}

TEST(VecTest, Resize) {
  {
    Vec<int> vec{stx::os_allocator};

    stx::vec::resize(vec, 10, 69).unwrap();

    EXPECT_VALID_VEC(vec);

    for (auto& el : vec) {
      EXPECT_EQ(el, 69);
    }

    stx::vec::resize(vec, 20, 42).unwrap();

    EXPECT_VALID_VEC(vec);

    EXPECT_EQ(vec.size(), 20);

    for (auto& el : vec.span().subspan(0, 10)) {
      EXPECT_EQ(el, 69);
    }

    for (auto& el : vec.span().subspan(10, 10)) {
      EXPECT_EQ(el, 42);
    }
  }
}

TEST(VecTest, ResizeLifetime) {
  {
    Vec<Life> vec{stx::os_allocator};
    stx::vec::resize(vec, 1).unwrap();
    stx::vec::resize(vec, 5).unwrap();

    EXPECT_VALID_VEC(vec);
  }
}

TEST(VecTest, Noop) {
  stx::Vec<int> vec{stx::os_allocator};

  vec.push(3);

  vec.push_inplace(3);
  vec.reserve(444).unwrap();
  vec.span();
  vec.at(1).unwrap().get() = 0;

  stx::FixedVec<int> g{stx::os_allocator, nullptr, 0};

  EXPECT_DEATH_IF_SUPPORTED(g.push_inplace(4783).expect("unable to push"),
                            ".*?");

  stx::Vec<int> no_vec{stx::noop_allocator};

  EXPECT_DEATH_IF_SUPPORTED(no_vec.push_inplace(4783).expect("unable to push"),
                            ".*?");
}
