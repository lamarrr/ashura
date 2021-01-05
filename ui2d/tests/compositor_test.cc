#include "vlk/ui2d/compositor.h"
#include <fstream>
#include "gtest/gtest.h"
#include "vlk/assets/image.h"
#include "vlk/ui2d/surface_provider.h"
#include "vlk/ui2d/widget.h"
using namespace vlk::ui2d;

using namespace vlk;

struct TestImage : public Widget {
  TestImage(char const *path) : path_{path}, image_{} {
    auto im_desc =
        vlk::desc::Image2D{path, vlk::desc::Image2D::Format::RGBA, false};
    image_ = vlk::data::Image2D::load(im_desc).expect("unable to load image");
  }

  virtual bool is_layout_type() const noexcept override { return false; }

  virtual bool is_stateful() const noexcept override { return false; }

  virtual bool is_dirty() const noexcept override { return false; }
  virtual void mark_clean() noexcept override {}

  virtual std::string_view get_name() const noexcept override { return path_; }

  virtual stx::Span<Widget *const> get_children() const noexcept override {
    return {};
  }

  virtual Rect compute_area(
      [[maybe_unused]] Extent const &allotted_extent,
      [[maybe_unused]] stx::Span<Rect> const &children_area) const
      noexcept override {
    return Rect{Offset{0, 0}, allotted_extent};
  }

  virtual void draw(
      [[maybe_unused]] Canvas &canvas,
      [[maybe_unused]] Extent const &requested_extent) const override {
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
  auto column = Column({new TestImage("/home/lamar/Pictures/IMG_0127.JPG"),
                        new TestImage("/home/lamar/Pictures/IMG_0187.PNG")});
  Extent screen{1920, 1080};
  impl::Compositor compostior{surface_provider, screen,
                              Rect{Offset{0, 0}, screen}, column};

  auto image = compostior.tick(std::chrono::nanoseconds(10));

  for (auto &entry : compostior.get_stateless_cache()) {
    auto area = entry.snapshot.area();
    std::cout << entry.snapshot.widget()->get_name() << " Offset{"
              << area.offset.x << ", " << area.offset.y << "}"
              << " Extent{" << area.extent.width << ", " << area.extent.height
              << "}\n";
  }

  std::ofstream file("./compositor.dump", std::ios_base::app);

  std::vector<uint8_t> buff;
  buff.resize(screen.width * screen.height * 4);

  image->readPixels(SkImageInfo::Make(screen.width, screen.height,
                                      SkColorType::kRGBA_8888_SkColorType,
                                      SkAlphaType::kPremul_SkAlphaType),
                    buff.data(), screen.width * 4, 0, 0);

  for (uint8_t c : buff) {
    file << static_cast<int>(c) << ", ";
  }
}
