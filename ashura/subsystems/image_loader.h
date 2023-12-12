#pragma once
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <string_view>

#include "ashura/context.h"
#include "ashura/image.h"
#include "ashura/image_decoder.h"
#include "ashura/primitives.h"
#include "ashura/subsystem.h"
#include "ashura/utils.h"
#include "stx/async.h"
#include "stx/memory.h"
#include "stx/result.h"
#include "stx/scheduler.h"
#include "stx/scheduler/scheduling/schedule.h"

namespace ash
{

struct ImageLoader : public Subsystem
{
  ImageLoader() = default;

  virtual constexpr void on_startup(Context &ctx) override
  {
    task_scheduler = ctx.task_scheduler;
  }

  virtual constexpr void tick(Context                 &ctx,
                              std::chrono::nanoseconds interval) override
  {
  }

  virtual constexpr void on_exit(Context &ctx) override
  {
  }

  virtual constexpr std::string_view get_name() override
  {
    return "ImageLoader";
  }

  virtual constexpr ~ImageLoader() override
  {
  }

  stx::Future<stx::Result<ImageBuffer, ImageLoadError>>
      load_from_file(std::string_view path)
  {
    ASH_LOG_INFO(ImageLoader, "Loading image from path: {}", path);
    return stx::sched::fn(
        *task_scheduler,
        [path_ = stx::string::make(stx::os_allocator, path).unwrap()]()
            -> stx::Result<ImageBuffer, ImageLoadError> {
          if (!std::filesystem::exists(path_.view()))
          {
            ASH_LOG_ERR(
                ImageLoader,
                "Failed to load image from path: {}, path does not exist",
                path_.view());
            return stx::Err(ImageLoadError::InvalidPath);
          }

          std::FILE *file = std::fopen(path_.c_str(), "rb");
          ASH_CHECK(file != nullptr);

          ASH_CHECK(std::fseek(file, 0, SEEK_END) == 0);

          long file_size = std::ftell(file);
          ASH_CHECK(file_size >= 0);

          stx::Memory memory =
              stx::mem::allocate(stx::os_allocator, file_size).unwrap();

          ASH_CHECK(std::fseek(file, 0, SEEK_SET) == 0);

          ASH_CHECK(std::fread(memory.handle, 1, file_size, file) == file_size);

          ASH_CHECK(std::fclose(file) == 0);

          stx::Result result = decode_image(
              stx::Span{(u8 *) memory.handle, static_cast<usize>(file_size)});

          if (result.is_ok())
          {
            ASH_LOG_INFO(
                ImageLoader,
                "Loaded and decoded {}x{} image at path: {} with size={} bytes",
                result.value().extent.width, result.value().extent.height,
                path_.view(), result.value().span().size());
          }
          else
          {
            ASH_LOG_ERR(ImageLoader,
                        "Failed to decode image at path: {}, error: {}",
                        path_.view(), (i64) result.err());
          }

          return result;
        },
        stx::NORMAL_PRIORITY, stx::TaskTraceInfo{});
  }

  stx::TaskScheduler *task_scheduler = nullptr;
};

}        // namespace ash
