

#include <cinttypes>
#include <memory>

namespace vlk {
namespace ui {

enum class ImageFormat : uint8_t {
  Rgba8888 = 0,
  Rgb888,
  Bgra8888,
  Gray8,
};

enum class BackendImageFormat : uint8_t {
  Rgba8888 = 0,  //!< pixel with 8 bits for red, green, blue, alpha; in 32-bit
                 //!< word
  Rgbx8888,  //!< pixel with 8 bits each for red, green, blue; in 32-bit word,
  Bgra8888,  //!< pixel with 8 bits for blue, green, red, alpha;
             //!< in 32-bit word
  Gray8,     //!< pixel with grayscale level in 8-bit word
};

struct AssetUID {
  AssetUID() = delete;

  constexpr uint64_t value() const { return value_; }

 private:
  uint64_t value_ = 0;
};

//
//
//
// rename this
//
//
//
// Bundle must be part of context
//
//
//
struct StaticImageBundleData {
  StaticImageBundleData(sk_sp<SkImage> const &gpu_image, Extent const &extent,
                        BackendImageFormat image_format)
      : gpu_image_{gpu_image}, extent_{extent}, image_format_{image_format} {}

  // ensure extent is not zero
  size_t bytes() const;

  StaticImageBundleData(StaticImageBundleData const &) = delete;
  StaticImageBundleData &operator=(StaticImageBundleData const &) = delete;

  // void draw();?
 private:
  // TODO(lamarrr): use sk_sp skimage for this (WithoutCopy) and store on the
  // GPU
  sk_sp<Image> gpu_image_;  // non-null
  Extent extent_;           // must be .visible()
  // test with rgbx8888
  BackendImageFormat image_format_ = BackendImageFormat::Rgba8888;
};

// TODO(lamarrr): log evictions for asyncimagebundle
// a delegate for uploading images to the GPU. The images are always kept in
// memory.
// TODO(lamarrr): repeated images?
struct StaticImageBundle : public Bundle {
  // users should be able to add images at the beginning of the program?
  //
  //
  // Algorithm:
  // we need to load image data asynchronously and also avoid loading it
  // multiple times in an instance of the application
  //
  //
  //
  //
  //
  //
  //
  // we need to add to the canvas to draw image using name or uid

  std::shared_ptr<StaticImageBundleData> get_image(StaticImageUID uid) {
    // o is for None and we never assign that as a uid
    VLK_ENSURE(uid != StaticImageUID::None, "Static image UID is `None`");
    VLK_ENSURE(uid <= data_.size(), "Invalid static image UID");
    return data_[uid.value() - 1];
  }

  StaticImageUID add_image(stx::Span<uint8_t const> data, Extent const &extent,
                           ImageFormat image_format) {
    VLK_ENSURE(!data.empty(), "Image data is empty");
    VLK_ENSURE(extent.visible(),
               "Image extent is not visible (extent == Extent{0, 0})");

    uint64_t new_uid = data_.size() + 1;

    // create a new u32 vector named data_aligned
    // SkImage::MakeCrossContextFromPixmap(SkPixMap::MakeWithoutCopy(data_aligned));

    return StaticImageUID{new_uid};
  }

  void tick(std::chrono::nanoseconds) {}

 private:
  // no replication
  std::vector<std::shared_ptr<StaticImageBundleData>> data_;
};


// ReservationUID reserve_tag(); used for async loading

}  // namespace ui
}  // namespace vlk
