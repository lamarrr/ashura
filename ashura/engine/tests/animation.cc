/// SPDX-License-Identifier: MIT
#include "ashura/engine/animation.h"

#include "gtest/gtest.h"

TEST(AnimationEngine, Basic)
{
  using namespace ash;

  StaggeredAnimation<f32> animation =
      StaggeredAnimation<f32>::make(6, 36, RippleStagger{});

  auto & timeline = animation.timelines().v0;

  f32         frames[]    = {20, 30};
  nanoseconds durations[] = {1ms};

  Easing easings[] = {easing::linear()};

  timeline.key_frame(frames, durations, easings);

  EXPECT_EQ(timeline.duration(), 1ms);

  EXPECT_EQ(animation.animate(0).v0, 20);
  
  animation.tick(500us);

  EXPECT_EQ(animation.animate(0).v0, 25);

  animation.tick(2ms);

  EXPECT_EQ(animation.animate(0).v0, 30);
}
