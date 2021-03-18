
#include "gtest/gtest.h"

#include "vlk/ui/layout.h"

using namespace vlk::ui;

/*
TEST(ConstraintSolver, Resolution) {
  {
    IndependentParameters p;
    p.bias = 0;
    p.clamp = OutputClamp{0.0f, 1.0f};
    p.high = vlk::u32_max;
    p.low = 0;
    p.scale = 1.0f;

    EXPECT_EQ(resolve_eqn(1000, p.scale, p.bias, p.low, p.high, p.clamp, true),
              1000);
    EXPECT_EQ(resolve_eqn(1000, p.scale, p.bias, p.low, p.high, p.clamp, false),
              1000);

    EXPECT_EQ(resolve_eqn(1000, 2.0f, p.bias, p.low, p.high, p.clamp, true),
              1000);
    EXPECT_EQ(resolve_eqn(1000, 2.0f, p.bias, p.low, p.high, p.clamp, false),
              1000);

    EXPECT_EQ(resolve_eqn(1000, 0.5f, p.bias, p.low, p.high, p.clamp, true),
              500);
    EXPECT_EQ(resolve_eqn(1000, 0.5f, p.bias, p.low, p.high, p.clamp, false),
              500);

    EXPECT_EQ(resolve_eqn(1000, p.scale, 20, 40, 700, p.clamp, true), 700);
    EXPECT_EQ(resolve_eqn(1000, p.scale, 20, 40, 700, p.clamp, false), 700);

    EXPECT_EQ(resolve_eqn(1000, p.scale, 20, p.low, p.high, p.clamp, true),
              1000);
    EXPECT_EQ(resolve_eqn(1000, p.scale, 20, p.low, p.high, p.clamp, false),
              1000);

    EXPECT_DEATH(resolve_eqn(1000, p.scale, 20, p.low, p.high,
                             OutputClamp{0.0f, 2.0f}, true),
                 ".*");
    EXPECT_EQ(resolve_eqn(1000, p.scale, 20, p.low, p.high,
                          OutputClamp{0.0f, 2.0f}, false),
              1020);
  }
}

TEST(ConstraintSolver, ResolutionDependent) {
  {
    IndependentParameters p;

    p.bias = 0;
    p.clamp = OutputClamp{0.0f, 1.0f};
    p.high = vlk::u32_max;
    p.low = 0;
    p.scale = 1.0f;

    EXPECT_EQ(resolve_eqn_dependent(1000, 250, p.scale, p.bias, p.low, p.high,
                                    p.clamp, true),
              250);
    EXPECT_EQ(resolve_eqn_dependent(1000, 250, p.scale, p.bias, p.low, p.high,
                                    p.clamp, false),
              250);

    EXPECT_EQ(resolve_eqn_dependent(1000, 250, p.scale, p.bias, p.low, p.high,
                                    p.clamp, true),
              250);
    EXPECT_EQ(resolve_eqn_dependent(1000, 250, p.scale, p.bias, p.low, p.high,
                                    p.clamp, false),
              250);

    EXPECT_EQ(resolve_eqn_dependent(1000, 250, 2.0f, p.bias, p.low, p.high,
                                    p.clamp, true),
              250);
    EXPECT_EQ(resolve_eqn_dependent(1000, 250, 2.0f, p.bias, p.low, p.high,
                                    p.clamp, false),
              250);

    EXPECT_EQ(resolve_eqn_dependent(1000, 250, 0.5f, p.bias, p.low, p.high,
                                    p.clamp, true),
              250);
    EXPECT_EQ(resolve_eqn_dependent(1000, 250, 0.5f, p.bias, p.low, p.high,
                                    p.clamp, false),
              250);

    EXPECT_EQ(
        resolve_eqn_dependent(1000, 250, p.scale, 20, 40, 700, p.clamp, true),
        250);
    EXPECT_EQ(
        resolve_eqn_dependent(1000, 250, p.scale, 20, 40, 700, p.clamp, false),
        250);

    EXPECT_EQ(resolve_eqn_dependent(500, 250, p.scale, 20, p.low, p.high,
                                    p.clamp, true),
              250);
    EXPECT_EQ(resolve_eqn_dependent(500, 250, p.scale, 20, p.low, p.high,
                                    p.clamp, false),
              250);

    EXPECT_DEATH(resolve_eqn_dependent(250, 250, p.scale, 20, p.low, p.high,
                                       OutputClamp{0.0f, 2.0f}, true),
                 ".*");
    EXPECT_EQ(resolve_eqn_dependent(250, 250, p.scale, 20, p.low, p.high,
                                    OutputClamp{0.0f, 2.0f}, false),
              270);
  }
}
*/
