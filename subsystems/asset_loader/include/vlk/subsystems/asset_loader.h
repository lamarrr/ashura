#pragma once

#include <utility>

#include "stx/async.h"
#include "stx/result.h"
#include "stx/scheduler.h"
#include "stx/scheduler/scheduling/schedule.h"
#include "vlk/font_asset.h"
#include "vlk/font_source.h"
#include "vlk/image_asset.h"
#include "vlk/image_source.h"
#include "vlk/subsystem.h"
#include "vlk/subsystem/context.h"
#include "vlk/subsystems/scheduler.h"

namespace vlk {

struct AssetLoader : public Subsystem {
  AssetLoader() = default;
  AssetLoader(AssetLoader const&) = default;
  AssetLoader(AssetLoader&&) = default;
  AssetLoader& operator=(AssetLoader const&) = default;
  AssetLoader& operator=(AssetLoader&&) = default;
  ~AssetLoader() override {}

  virtual void link(SubsystemsContext const& context) override {
    stx::Rc<Subsystem*> scheduler_subsystem =
        context.get("VLK_TaskScheduler")
            .expect("Unable to find task scheduler subsystem");

    scheduler = stx::Some(
        stx::transmute(scheduler_subsystem.handle->as<TaskScheduler>().unwrap(),
                       std::move(scheduler_subsystem)));
  }

  virtual void tick(std::chrono::nanoseconds) override {}

  virtual stx::FutureAny get_future() override {
    return stx::FutureAny{
        stx::make_promise<void>(stx::os_allocator).unwrap().get_future()};
  }

  stx::Future<stx::Result<ImageAsset, ImageLoadError>> load_image(
      FileImageSource source) {
    stx::Rc<FileImageSourceData const*>& data = source.data;

    return stx::sched::fn(
        scheduler.value().handle->scheduler,
        [data_ = data.share()]() -> stx::Result<ImageAsset, ImageLoadError> {
          return impl::StbiImageBuffer::load_from_file(
                     data_.handle->path, data_.handle->target_format)
              .map([](impl::StbiImageBuffer buffer) {
                return ImageAsset{impl::make_sk_image(buffer)};
              });
        },
        stx::NORMAL_PRIORITY,
        stx::TaskTraceInfo{
            stx::string::rc::make_static_view("AssetLoader"),
            stx::transmute(std::string_view{data.handle->tag}, data.share())});
  }

  stx::Future<stx::Result<ImageAsset, ImageLoadError>> load_image(
      MemoryImageSource source) {
    stx::Rc<MemoryImageSourceData const*>& data = source.data;

    return stx::sched::fn(
        scheduler.value().handle->scheduler,
        [data_ = data.share()]() -> stx::Result<ImageAsset, ImageLoadError> {
          return stx::Ok{ImageAsset{impl::make_sk_image(
              data_.handle->info, stx::Span{data_.handle->bytes.data(),
                                            data_.handle->bytes.size()})}};
        },
        stx::NORMAL_PRIORITY,
        stx::TaskTraceInfo{
            stx::string::rc::make_static_view("AssetLoader"),
            stx::transmute(std::string_view{data.handle->tag}, data.share())});
  }

  stx::Future<stx::Result<FontAsset, FontLoadError>> load_font(
      MemoryTypefaceSource source) {
    auto& data = source.data;
    return stx::sched::fn(
        scheduler.value().handle->scheduler,
        [source_ = source]() {
          return impl::load_typeface_from_memory(source_.get_bytes().handle)
              .map([](sk_sp<SkTypeface>&& typeface) {
                return FontAsset{std::move(typeface)};
              });
        },
        stx::NORMAL_PRIORITY,
        stx::TaskTraceInfo{
            stx::string::rc::make_static_view("AssetLoader"),
            stx::transmute(std::string_view{data.handle->tag}, data.share())});
  }

  stx::Future<stx::Result<FontAsset, FontLoadError>> load_font(
      FileTypefaceSource source) {
    auto& data = source.data;
    return stx::sched::fn(
        scheduler.value().handle->scheduler,
        [source_ = source]() {
          return impl::load_typeface_from_file(source_.get_path())
              .map([](sk_sp<SkTypeface>&& typeface) {
                return FontAsset{std::move(typeface)};
              });
        },
        stx::NORMAL_PRIORITY,
        stx::TaskTraceInfo{
            stx::string::rc::make_static_view("AssetLoader"),
            stx::transmute(std::string_view{data.handle->tag}, data.share())});
  }

  stx::Future<stx::Result<FontAsset, FontLoadError>> load_font(
      SystemFont font) {
    stx::Rc<SystemFontData const*> font_data = font.data.share();
    std::string_view tag{font_data.handle->tag};
    return stx::sched::fn(
        scheduler.value().handle->scheduler,
        [font_ = std::move(font)]() {
          return impl::load_system_typeface(font_.get_family(),
                                            font_.get_style())
              .map([](sk_sp<SkTypeface>&& typeface) {
                return FontAsset{std::move(typeface)};
              });
        },
        stx::NORMAL_PRIORITY,
        stx::TaskTraceInfo{
            stx::string::rc::make_static_view("AssetLoader"),
            stx::transmute(std::string_view{tag}, std::move(font_data))});
  }

  stx::Future<stx::Result<FontAsset, FontLoadError>> load_font(
      MemoryFontSource source);
  stx::Future<stx::Result<FontAsset, FontLoadError>> load_font(
      FileFontSource source);

  stx::Option<stx::Rc<vlk::TaskScheduler*>> scheduler = stx::None;
};

}  // namespace vlk
