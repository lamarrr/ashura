

#include "stx/flex.h"

#include "gtest/gtest.h"

using stx::Flex;

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

#define EXPECT_VALID_FLEX(flex)                                \
  EXPECT_GE(flex.iterator____end(), flex.iterator____begin()); \
  EXPECT_GE(flex.capacity(), flex.size())

TEST(FlexTest, Destructor) {
  {
    Flex<int> flex{stx::Memory{stx::os_allocator, nullptr}, 0, 0};

    for (size_t i = 0; i < 10000; i++)
      flex = stx::flex::push_inplace(std::move(flex), 8).unwrap();

    EXPECT_EQ(flex.size(), 10000);
    EXPECT_VALID_FLEX(flex);
  }

  {
    Flex<int> flex{stx::os_allocator};

    EXPECT_VALID_FLEX(flex);
  }
}

TEST(FlexTest, Resize) {
  {
    Flex<int> flex{stx::os_allocator};

    flex = stx::flex::resize(std::move(flex), 10, 69).unwrap();

    EXPECT_VALID_FLEX(flex);

    for (auto& el : flex.span()) {
      EXPECT_EQ(el, 69);
    }

    flex = stx::flex::resize(std::move(flex), 20, 42).unwrap();

    EXPECT_VALID_FLEX(flex);

    EXPECT_EQ(flex.size(), 20);

    for (auto& el : flex.span().subspan(0, 10)) {
      EXPECT_EQ(el, 69);
    }

    for (auto& el : flex.span().subspan(10, 10)) {
      EXPECT_EQ(el, 42);
    }
  }
}

TEST(FlexTest, ResizeLifetime) {
  {
    Flex<Life> flex{stx::os_allocator};
    flex = stx::flex::resize(std::move(flex), 1).unwrap();
    flex = stx::flex::resize(std::move(flex), 5).unwrap();

    EXPECT_VALID_FLEX(flex);
  }
}

TEST(FlexTest, Noop) {
  stx::Flex<int> flex{stx::os_allocator};

  flex = stx::flex::push(std::move(flex), 3).unwrap();

  flex = stx::flex::push_inplace(std::move(flex), 3).unwrap();
  flex = stx::flex::reserve(std::move(flex), 444).unwrap();
  flex.span();
  flex.span().at(1).unwrap().get() = 0;

  stx::FixedFlex<int> g{stx::os_allocator};

  EXPECT_DEATH_IF_SUPPORTED((g = stx::flex::push_inplace(std::move(g), 4783)
                                     .expect("unable to push")),
                            ".*?");

  stx::Flex<int> no_flex{stx::noop_allocator};

  EXPECT_DEATH_IF_SUPPORTED((g = stx::flex::push_inplace(std::move(g), 4783)
                                     .expect("unable to push")),
                            ".*?");
}
