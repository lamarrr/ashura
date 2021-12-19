#pragma once

#include "stx/async.h"
#include "stx/result.h"
#include "vlk/font_asset.h"
#include "vlk/font_source.h"
#include "vlk/image_asset.h"
#include "vlk/image_source.h"
#include "vlk/scheduler.h"
#include "vlk/scheduler/scheduling/schedule.h"

namespace vlk {

struct AssetLoader : public SubsystemImpl {
  virtual void link(SubsystemsContext const& context) override {
    Rc<Subsystem*> scheduler_subsystem =
        context.get("TaskScheduler")
            .expect("Unable to find task scheduler subsystem");

    scheduler = stx::Some(
        stx::transmute(scheduler_subsystem.handle->as<TaskScheduler>().unwrap(),
                       std::move(scheduler_subsystem)));
  }

  stx::Future<stx::Result<ImageAsset, ImageLoadError>> load_image(
      FileImageSource source) {
    stx::Rc<FileImageSourceData const*>& data = source.data;

    return vlk::sched::fn(
        *scheduler.value().handle,
        [data_ = data.share()]() -> Result<ImageAsset, ImageLoadError> {
          return impl::StbiImageBuffer::load_from_file(
                     data_.handle->path, data_.handle->target_format)
              .map([](impl::StbiImageBuffer buffer) {
                return ImageAsset{impl::make_sk_image(buffer)};
              });
        },
        stx::NORMAL_PRIORITY,
        TaskTraceInfo{
            stx::string::rc::make_static_view("AssetLoader"),
            stx::transmute(std::string_view{data.handle->tag}, data.share())});
  }

  stx::Future<stx::Result<ImageAsset, ImageLoadError>> load_image(
      MemoryImageSource source) {
    stx::Rc<MemoryImageSourceData const*>& data = source.data;

    return vlk::sched::fn(
        *scheduler.value().handle,
        [data_ = data.share()]() -> Result<ImageAsset, ImageLoadError> {
          return Ok{ImageAsset{impl::make_sk_image(
              data_.handle->info, stx::Span{data_.handle->bytes.data(),
                                            data_.handle->bytes.size()})}};
        },
        stx::NORMAL_PRIORITY,
        TaskTraceInfo{
            stx::string::rc::make_static_view("AssetLoader"),
            stx::transmute(std::string_view{data.handle->tag}, data.share())});
  }

  stx::Future<Result<FontAsset, FontLoadError>> load_font(
      MemoryTypefaceSource source) {
    auto& data = source.data;
    return vlk::sched::fn(
        *scheduler.value().handle,
        [source_ = source]() {
          return impl::load_typeface_from_memory(source_.get_bytes().handle)
              .map([](sk_sp<SkTypeface>&& typeface) {
                return FontAsset{std::move(typeface)};
              });
        },
        stx::NORMAL_PRIORITY,
        TaskTraceInfo{
            stx::string::rc::make_static_view("AssetLoader"),
            stx::transmute(std::string_view{data.handle->tag}, data.share())});
  }

  stx::Future<Result<FontAsset, FontLoadError>> load_font(
      FileTypefaceSource source) {
    auto& data = source.data;
    return vlk::sched::fn(
        *scheduler.value().handle,
        [source_ = source]() {
          return impl::load_typeface_from_file(source_.get_path())
              .map([](sk_sp<SkTypeface>&& typeface) {
                return FontAsset{std::move(typeface)};
              });
        },
        stx::NORMAL_PRIORITY,
        TaskTraceInfo{
            stx::string::rc::make_static_view("AssetLoader"),
            stx::transmute(std::string_view{data.handle->tag}, data.share())});
  }
  stx::Future<Result<FontAsset, FontLoadError>> load_font(
      MemoryFontSource source);
  stx::Future<Result<FontAsset, FontLoadError>> load_font(
      FileFontSource source);
  stx::Future<Result<FontAsset, FontLoadError>> load_font(SystemFont font);

  stx::Option<stx::Rc<TaskScheduler*>> scheduler = stx::None;
};

}  // namespace vlk
