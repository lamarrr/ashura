

#include "vlk/widget.h"

#include "gtest/gtest.h"

TEST(WidgetConstructionTest, Widget) {
  vlk::WidgetGraph graph;

  graph.add_stateful_child<vlk::Button>()
      .add_stateful_child<vlk::Button>()
      .add_stateful_child<vlk::Button>();
}
