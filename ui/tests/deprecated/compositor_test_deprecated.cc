#include "vlk/ui/compositor.h"
#include <fstream>
#include "gtest/gtest.h"
#include "vlk/assets/image.h"
#include "vlk/ui/surface_provider.h"
#include "vlk/ui/widget.h"
#include "vlk/ui/widgets/row.h"

using namespace vlk::ui;

using namespace vlk;

struct TestImage : public Widget {
  TestImage(char const *path) : image_{}, path_{path} {
    auto im_desc =
        vlk::desc::Image2D{path, vlk::desc::Image2D::Format::RGBA, false};
    image_ = vlk::data::Image2D::load(im_desc).expect("unable to load image");
  }

  virtual Type get_type() const noexcept override { return Type::Render; }

  virtual bool is_stateful() const noexcept override { return false; }

  virtual bool is_dirty() const noexcept override { return false; }
  virtual void mark_clean() noexcept override {}

  virtual std::string_view get_name() const noexcept override { return path_; }

  virtual std::string_view get_type_hint() const noexcept override {
    return "TestImage";
  }

  virtual stx::Span<Widget *const> get_children() const noexcept override {
    return {};
  }

  virtual Rect compute_area(
      [[maybe_unused]] Extent const &allotted_extent,
      [[maybe_unused]] stx::Span<Rect> const &children_area) override {
    return Rect{Offset{0, 0}, allotted_extent};
  }

  virtual void draw([[maybe_unused]] Canvas &canvas,
                    [[maybe_unused]] Extent const &requested_extent) override {
    SkCanvas *sk_canvas = canvas.as_skia();
    sk_sp<SkData> sk_data =
        SkData::MakeWithoutCopy(image_.bytes().data(), image_.size());
    VLK_ENSURE(sk_data != nullptr);
    sk_sp<SkImage> image = SkImage::MakeRasterData(
        SkImageInfo::Make(image_.width(), image_.height(),
                          SkColorType::kRGBA_8888_SkColorType,
                          SkAlphaType::kPremul_SkAlphaType),
        sk_data, image_.width() * 4);
    VLK_ENSURE(image != nullptr);
    sk_canvas->drawImage(image, 0, 0);
  }

 private:
  vlk::data::Image2D image_;
  char const *path_;
};

TEST(CompositorTest, SimpleComposition) {
  CpuSurfaceProvider surface_provider;
  auto row = Row({new TestImage("/home/lamar/Pictures/IMG_0127.JPG"),
                  new TestImage("/home/lamar/Pictures/IMG_0187.PNG")});
  Extent screen{1920, 1080};
  impl::Compositor compostior{surface_provider, screen,
                              Rect{Offset{0, 0}, screen}, row};

  auto image = compostior.tick(std::chrono::nanoseconds(10));

  for (auto &entry : compostior.get_stateless_cache()) {
    auto area = entry.snapshot.area();
    std::cout << entry.snapshot.widget()->get_name() << " Offset{"
              << area.offset.x << ", " << area.offset.y << "}"
              << " Extent{" << area.extent.width << ", " << area.extent.height
              << "}\n";
  }
}

#include "vlk/ui/widgets/box.h"
#include "vlk/ui/widgets/image.h"
#include "vlk/ui/widgets/margin.h"
#include "vlk/ui/widgets/text.h"

TEST(TextRenderingTest, SimpleParagraph) {
  CpuSurfaceProvider surface_provider;

  auto row = Row(
      {new Margin(10, new TestImage("/home/lamar/Desktop/batman.jpg")),
       new Margin{
           100, 20,
           new Text{
               "It’s not who I am underneath, but what I do that defines me.",
               TextProperties()
                   .font_family("TImes New Roman")
                   .font_size(150)
                   .color(colors::Black)
                   .align(TextAlign::Center)}}});
  Extent screen{1920, 1080};
  impl::Compositor compostior{surface_provider, screen,
                              Rect{Offset{0, 0}, screen}, row};

  auto start = std::chrono::high_resolution_clock::now();

  auto image = compostior.tick(std::chrono::nanoseconds(10));
}

TEST(TextRenderingTest, SimpleBox) {
  CpuSurfaceProvider surface_provider;

  // TODO(lamarrr): fix these enums BoxLayout
  // TODO(lamarrr): custom sizes for the boxes

  //  TextShadow shadows[] = {TextShadow{colors::Black, {}, 1.0}};
  // VerticalSeparator
  TextProperties text_properties = TextProperties{}
                                       .font_family("Times New Roman")
                                       .font_size(30)
                                       .color(colors::Black)
                                       .align(TextAlign::Center);

  auto row = Row(
      {new Box{
           new Text{"左線読設 後碁給能上目秘使約。満毎冠行来昼本可必図将発確"
                    "年。今属場育"
                    "図情闘陰野高備込制詩西校客。審対江置講今固残必託地集済決維"
                    "駆年策。立得",
                    TextProperties()
                        .font_family("Roboto Mono")
                        .font_size(40)
                        .color(colors::White)
                        .align(TextAlign::Left)},
           BoxProperties{}
               .padding(100)
               .layout(BoxLayout::Fit)
               .border_radius(5000),
           BoxDecoration{}
               .color(colors::Black.with_alpha(0xAA))
               .image(data::Image2D::load("/home/lamar/Pictures/bhound.jpg",
                                          desc::Image2D::Format::RGBA)
                          .unwrap(),
                      1.0f, Sizing::relative(0.25f, 0.0f, 1.0f, 1.0f),
                      Stretch::None)
               .blur(50.0f)},
       new Margin{
           100,
           new Text{
               "Lorem ipsum dolor sit amet, consectetur adipiscing elit. "
               "Fusce ac laoreet neque. Ut commodo sapien libero, a facilisis "
               "enim fermentum accumsan. Aliquam ultricies leo ut felis "
               "lobortis eleifend. Pellentesque libero felis, venenatis nec "
               "accumsan vel, fermentum non nulla. Aliquam commodo, magna sit "
               "amet condimentum vulputate, turpis libero convallis est, in "
               "consequat mi mi eu orci. Curabitur sapien arcu, tincidunt ac "
               "nisl a, tempus varius purus. Integer tristique nisl quis "
               "magna pellentesque, at venenatis est varius. Mauris "
               "pellentesque velit et egestas blandit. Nulla sit amet nisi "
               "mollis, lacinia diam ac, hendrerit lorem.",
               text_properties}},
       new Margin{
           100,
           new Text{
               "Lorem ipsum dolor sit amet, consectetur adipiscing elit. "
               "Fusce ac laoreet neque. Ut commodo sapien libero, a facilisis "
               "enim fermentum accumsan. Aliquam ultricies leo ut felis "
               "lobortis eleifend. Pellentesque libero felis, venenatis nec "
               "accumsan vel, fermentum non nulla. Aliquam commodo, magna sit "
               "amet condimentum vulputate, turpis libero convallis est, in "
               "consequat mi mi eu orci. Curabitur sapien arcu, tincidunt ac "
               "nisl a, tempus varius purus. Integer tristique nisl quis "
               "magna pellentesque, at venenatis est varius. Mauris "
               "pellentesque velit et egestas blandit. Nulla sit amet nisi "
               "mollis, lacinia diam ac, hendrerit lorem.",
               text_properties}}});
  Extent screen{1920, 1080};
  impl::Compositor compostior{surface_provider, screen,
                              Rect{Offset{0, 0}, screen}, row};

  auto start = std::chrono::high_resolution_clock::now();

  auto image = compostior.tick(std::chrono::nanoseconds(10));

  std::ofstream file("./compositor.dump", std::ios_base::app);

  std::vector<uint8_t> buff;
  buff.resize(screen.width * screen.height * 4);

  image->readPixels(SkImageInfo::Make(screen.width, screen.height,
                                      SkColorType::kRGBA_8888_SkColorType,
                                      SkAlphaType::kPremul_SkAlphaType),
                    buff.data(), screen.width * 4, 0, 0);

  for (uint8_t c : buff) {
    file << static_cast<uint32_t>(c) << ", ";
  }
}
