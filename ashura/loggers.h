#pragma once

#include <memory>

#include "spdlog/logger.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

#define ASH_DECLARE_LOGGER(identifier) \
  ::spdlog::logger *get_logger_##identifier()

#define ASH_GET_LOGGER(identifier) get_logger_##identifier()

// TODO(lamarrr): use log path specified in config?
#define ASH_DEFINE_LOGGER(identifier)                                       \
  ::spdlog::logger *get_logger_##identifier()                               \
  {                                                                         \
    static ::spdlog::sink_ptr __sinks[] = {                                 \
        ::std::make_shared<::spdlog::sinks::basic_file_sink_mt>("log.txt"), \
        ::std::make_shared<::spdlog::sinks::stdout_color_sink_mt>()};       \
                                                                            \
    static ::spdlog::logger __logger{                                       \
        #identifier, __sinks,                                               \
        __sinks + sizeof(__sinks) / sizeof(::spdlog::sink_ptr)};            \
    return &__logger;                                                       \
  }

#define ASH_LOG_TRACE(logger, ...) ASH_GET_LOGGER(logger)->trace(__VA_ARGS__)
#define ASH_LOG_INFO(logger, ...) ASH_GET_LOGGER(logger)->info(__VA_ARGS__)
#define ASH_LOG_ERR(logger, ...) ASH_GET_LOGGER(logger)->error(__VA_ARGS__)
#define ASH_LOG_WARN(logger, ...) ASH_GET_LOGGER(logger)->warn(__VA_ARGS__)

namespace ash
{

ASH_DECLARE_LOGGER(Init);
ASH_DECLARE_LOGGER(Vulkan);
ASH_DECLARE_LOGGER(Vulkan_ValidationLayer);
ASH_DECLARE_LOGGER(Vulkan_RenderResourceManager);
ASH_DECLARE_LOGGER(Vulkan_CanvasPipelineManager);
ASH_DECLARE_LOGGER(Window);
ASH_DECLARE_LOGGER(Context);
ASH_DECLARE_LOGGER(ImageLoader);
ASH_DECLARE_LOGGER(MediaPlayer);
ASH_DECLARE_LOGGER(FontRenderer);

}        // namespace ash
