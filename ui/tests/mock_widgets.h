#include <iostream>
#include <vector>

#include "stx/option.h"
#include "vlk/ui/widget.h"

using namespace vlk::ui;
using namespace vlk;

struct MockSized : public Widget {
  MockSized(Extent extent, stx::Option<ZIndex> const& z_index = stx::None,
            Padding padding = {})
      : Widget{Type::Render} {
    Widget::init_is_flex(false);
    Widget::update_self_extent(SelfExtent{Constrain::absolute(extent.width),
                                          Constrain::absolute(extent.height)});
    Widget::update_padding(padding);
    Widget::init_z_index(z_index.clone());
  }
  ~MockSized() override {}

  virtual void draw(Canvas& canvas, AssetManager&) override {
    Extent const& extent = canvas.extent();
    std::cout << "[MockSized] draw on extent: Extent{width: " << extent.width
              << ", height: " << extent.height << "}" << std::endl;
  }
};

struct MockFlex : public Widget {
  MockFlex(std::initializer_list<Widget*> const& children,
           stx::Option<ZIndex> const& z_index = stx::None)
      : Widget{Type::Render} {
    children_ = children;
    Widget::init_is_flex(true);
    Widget::update_children(children_);
    Widget::update_flex(Flex{});
    Widget::init_z_index(z_index.clone());
    Widget::update_self_extent(SelfExtent{Constrain{1.0f}, Constrain{1.0f}});
  }

  ~MockFlex() override {
    // no freeing
  }

  std::vector<Widget*> children_;

  virtual void draw(Canvas& canvas, AssetManager&) override {
    Extent const& extent = canvas.extent();
    std::cout << "[MockFlex] draw on extent: Extent{width: " << extent.width
              << ", height: " << extent.height << "}" << std::endl;
  }
};

// this optimization would mean we can't move the widgets since the pointer
// address would change, even if it is a std::array
struct MockView : public Widget {
  MockView(Widget* child) : Widget{Type::View} {
    child_ = child;
    Widget::init_is_flex(true);
    Widget::update_children(stx::Span<Widget*>(&child_, 1));
    Widget::update_flex(Flex{});
    Widget::update_self_extent(SelfExtent{Constrain{1.0f}, Constrain{1.0f}});
    Widget::update_view_extent(ViewExtent{Constrain{1.0f}, Constrain{1.0f}});
    Widget::update_padding(Padding{});
  }

  ~MockView() override {
    // no freeing
  }

  Widget* child_;

  virtual void draw(Canvas& canvas, AssetManager&) override {
    Extent const& extent = canvas.extent();
    std::cout << "[MockView] draw on extent: Extent{width: " << extent.width
              << ", height: " << extent.height << "}" << std::endl;
  }
};
