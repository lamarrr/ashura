#include "gtest/gtest.h"
#include "vlk/ui/pipeline.h"

using namespace vlk;
using namespace vlk::ui;

static_assert((IOffset{20, 30} + IOffset{30, 20}) == IOffset{50, 50});
static_assert((IOffset{20, 30} - IOffset{30, 20}) == IOffset{-10, 10});
static_assert(IOffset{20, 30} == IOffset{20, 30});
static_assert(IOffset{20, 30} != IOffset{21, 30});

static_assert((Offset{20, 30} + Offset{30, 20}) == Offset{50, 50});
static_assert(Offset{20, 30} == Offset{20, 30});
static_assert(Offset{20, 30} != Offset{21, 30});

static_assert(!Extent{0, 0}.visible());
static_assert(Extent{20, 30}.constrain(Extent{30, 50}) == Extent{20, 30});
static_assert(Extent{30, 50}.constrain(Extent{20, 30}) == Extent{20, 30});
static_assert((Extent{20, 20} + Extent{20, 50}) == Extent{40, 70});
static_assert(Extent{20, 60} == Extent{20, 60});
static_assert(Extent{60, 20} != Extent{20, 60});

static_assert(aspect_ratio_trim(Extent{4, 1}, Extent{1000, 100}) ==
              Extent{400, 100});
static_assert(aspect_ratio_trim(Extent{4, 1}, Extent{100, 1000}) ==
              Extent{100, 25});
static_assert(aspect_ratio_trim(Extent{1, 4}, Extent{1000, 100}) ==
              Extent{25, 100});
static_assert(aspect_ratio_trim(Extent{1, 4}, Extent{100, 1000}) ==
              Extent{100, 400});
static_assert(aspect_ratio_trim(Extent{2, 3}, Extent{300, 400}) ==
              Extent{266, 400});

TEST(PrimitivesTests, BasicTest) {}