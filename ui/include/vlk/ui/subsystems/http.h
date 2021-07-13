#pragma once

#include <curl/curl.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#include <chrono>
#include <cstddef>
#include <filesystem>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <queue>
#include <thread>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>

#include "stx/span.h"
#include "vlk/ui/subsystems/async.h"
#include "vlk/utils/limits.h"

#define VLK_CURLE_ENSURE(code_expression, ...) \
  do {                                         \
    if ((code_expression) > 0) {               \
      VLK_PANIC(__VA_ARGS__);                  \
    }                                          \
  } while (false)

#define VLK_CURLM_ENSURE(code_expression, ...) \
  do {                                         \
    if ((code_expression) > 0) {               \
      VLK_PANIC(__VA_ARGS__);                  \
    }                                          \
  } while (false)

namespace vlk {
namespace http {

inline stx::FixedReport operator>>(stx::ReportQuery, CURLcode code) noexcept {
  return stx::FixedReport(fmt::format("{}", static_cast<int>(code)));
}

inline stx::FixedReport operator>>(stx::ReportQuery, CURLMcode code) noexcept {
  return stx::FixedReport(fmt::format("{}", static_cast<int>(code)));
}

// has child object dependencies
enum class Verb { Get, Head };

struct File {
  std::filesystem::path path;
};

struct Bytes {
  std::shared_ptr<uint8_t const[]> bytes;
};

using Content = std::variant<File, Bytes, std::string>;

using Header = std::map<std::string, std::string>;

using Url = std::string;

struct Request {
  Url url = "https://bing.com";
  Header header;
  Verb verb = Verb::Get;
  // ignored if verb is Get or Head
  // response to file or
};

using Task = Request;

enum class ResponseCode : uint32_t {};

struct Response {
  Header header;
  ResponseCode code{};
  std::vector<uint8_t> content;
  std::chrono::nanoseconds total_time{0};

  std::string_view content_as_string_view() const {
    return std::string_view{reinterpret_cast<char const *>(content.data()),
                            content.size()};
  }
};

struct Progress {
  friend struct ProgressMonitorState;

  uint64_t total_bytes_sent = 0;
  uint64_t total_bytes_received = 0;
  uint64_t bytes_sent = 0;
  uint64_t bytes_received = 0;
  uint64_t upload_speed = 0;
  uint64_t download_speed = 0;

  stx::Option<uint64_t> upload_size() const {
    if (upload_size_ == u64_max) return stx::None;
    return stx::Some(uint64_t{upload_size_});
  }
  stx::Option<uint64_t> download_size() const {
    if (download_size_ == u64_max) return stx::None;
    return stx::Some(uint64_t{download_size_});
  }

 private:
  uint64_t upload_size_ = u64_max;
  uint64_t download_size_ = u64_max;
};

struct RawProgress {
  uint64_t bytes_sent = 0;
  uint64_t bytes_received = 0;
  uint64_t upload_speed = 0;
  uint64_t download_speed = 0;
  uint64_t upload_size = u64_max;
  uint64_t download_size = u64_max;
};

struct ProgressMonitorState {
  VLK_DISABLE_COPY(ProgressMonitorState)
  VLK_DISABLE_MOVE(ProgressMonitorState)
  VLK_DEFAULT_CONSTRUCTOR(ProgressMonitorState)

  alignas(alignof(std::max_align_t) *
          2) std::atomic<uint64_t> total_bytes_sent = 0;
  alignas(alignof(std::max_align_t) *
          2) std::atomic<uint64_t> total_bytes_received = 0;
  alignas(alignof(std::max_align_t) * 2) std::atomic<uint64_t> bytes_sent = 0;
  alignas(alignof(std::max_align_t) *
          2) std::atomic<uint64_t> bytes_received = 0;
  alignas(alignof(std::max_align_t) * 2) std::atomic<uint64_t> upload_speed = 0;
  alignas(alignof(std::max_align_t) *
          2) std::atomic<uint64_t> download_speed = 0;
  alignas(alignof(std::max_align_t) * 2) std::atomic<uint64_t> upload_size = 0;
  alignas(alignof(std::max_align_t) *
          2) std::atomic<uint64_t> download_size = 0;

  static constexpr auto memory_order = std::memory_order_relaxed;

  Progress load() const {
    Progress progress;
    progress.total_bytes_sent = total_bytes_sent.load(memory_order);
    progress.total_bytes_received = total_bytes_received.load(memory_order);
    progress.bytes_sent = bytes_sent.load(memory_order);
    progress.bytes_received = bytes_received.load(memory_order);
    progress.upload_speed = upload_speed.load(memory_order);
    progress.download_speed = download_speed.load(memory_order);
    progress.upload_size_ = upload_size.load(memory_order);
    progress.download_size_ = download_size.load(memory_order);

    return progress;
  }

  void push(RawProgress const &progress) {
    auto previous_total_sent = total_bytes_sent.load(memory_order);
    total_bytes_sent.store(previous_total_sent + progress.bytes_sent,
                           memory_order);

    auto previous_total_received = total_bytes_received.load(memory_order);

    total_bytes_received.store(
        previous_total_received + progress.bytes_received, memory_order);

    bytes_sent.store(progress.bytes_sent, memory_order);
    bytes_received.store(progress.bytes_received, memory_order);
    upload_speed.store(progress.upload_speed, memory_order);
    download_speed.store(progress.download_speed, memory_order);
    upload_size.store(progress.upload_size, memory_order);
    download_size.store(progress.download_size, memory_order);
  }
};

struct ProgressMonitor {
  friend struct ProgressMonitorUpdateProxy;

  static auto create() {
    ProgressMonitor monitor;
    monitor.state =
        std::shared_ptr<ProgressMonitorState>(new ProgressMonitorState);

    return std::move(monitor);
  }

  Progress get_progress() const { return state->load(); }

  bool is_valid() const { return state != nullptr; }

 private:
  std::shared_ptr<ProgressMonitorState> state;
};

struct ProgressMonitorUpdateProxy {
  explicit ProgressMonitorUpdateProxy(ProgressMonitor const &monitor) {
    state = monitor.state;
  }

  void update(RawProgress const &progress) const {
    auto shared_state = state.lock();
    if (shared_state == nullptr) {
      // the user is no longer interested in observing the progress of the
      // request
    } else {
      shared_state->push(progress);
    }
  }

 private:
  std::weak_ptr<ProgressMonitorState> state;
};

struct PackagedTask {
  Request request;
  Promise<Response> promise;
  // when context is about to shutdown, cancelation is requested so we need to
  // store it
  CancelationToken cancelation_token;
  CancelationProxy cancelation_proxy;
  SuspendProxy suspend_proxy;
  ProgressMonitorUpdateProxy progress_monitor_update_token;

  PackagedTask(Request &&request, Future<Response> const &future,
               ProgressMonitor const &monitor)
      : request{std::move(request)},
        promise{Promise<Response>(future)},
        cancelation_token{CancelationToken{future}},
        cancelation_proxy{CancelationProxy{future}},
        suspend_proxy{SuspendProxy{future}},
        progress_monitor_update_token{ProgressMonitorUpdateProxy{monitor}} {}
};

struct CurlHeaderDeleter {
  void operator()(curl_slist *slist) { curl_slist_free_all(slist); }
};

struct CurlEasyDeleter {
  void operator()(CURL *easy) { curl_easy_cleanup(easy); }
};

struct CurlMultiDeleter {
  void operator()(CURLM *multi) { curl_multi_cleanup(multi); }
};

using CurlHeader = std::unique_ptr<curl_slist, CurlHeaderDeleter>;
using CurlEasy = std::unique_ptr<CURL, CurlEasyDeleter>;
using CurlMulti = std::unique_ptr<CURL, CurlMultiDeleter>;

struct RunningTaskInfo {
  PackagedTask task;
  CurlEasy easy;
  CurlHeader header;
  Response response;
};

struct RunningTask {
  // we need a stable address for `Response` and `task` to interact with curl
  std::unique_ptr<RunningTaskInfo> info;

  explicit RunningTask(PackagedTask &&task)
      : info{new RunningTaskInfo{std::move(task), CurlEasy{curl_easy_init()},
                                 CurlHeader{nullptr}, Response{}}} {}
};

struct ExecutionContextHandle {
  // no move
  // no copy

  ExecutionContextHandle() {}

  ~ExecutionContextHandle() {
    // cancel all pending tasks
    // notify futures of cancelation
    // ...
    // remove curl easy handles
    // destroy pending curl easy handles
    // destroy multi_init
  }

  // submittable to from multiple threads
  std::tuple<Future<Response>, ProgressMonitor> submit(Request &&request) {
    std::lock_guard guard{task_queue_mutex_};
    auto future = Future<Response>::create();
    auto progress_monitor = ProgressMonitor::create();

    task_queue_.push_back(
        PackagedTask{std::move(request), future, progress_monitor});

    return std::tuple{std::move(future), std::move(progress_monitor)};
  }

  static size_t curl_content_write_function(void *bytes, size_t unit_size,
                                            size_t nmemb, void *task_info_ptr) {
    auto task_info = static_cast<RunningTaskInfo *>(task_info_ptr);
    auto bytes_u8 = static_cast<uint8_t *>(bytes);
    auto total_size = nmemb * unit_size;

    if (task_info->task.cancelation_proxy.try_acknowledge_cancel()) {
      // signals to libcurl that we want to cancel the request
      // TODO(lamarrr): this makes curl propagate a CURL_WRITE_ERROR return
      //
      return 0;
    }

    try {
      task_info->response.content.insert(task_info->response.content.end(),
                                         bytes_u8, bytes_u8 + total_size);
    } catch (...) {
      VLK_PANIC("Ran out of memory");
    }

    return total_size;
  }

  // only called on one thread?
  void tick() {
    if (!task_queue_.empty()) {
      std::lock_guard guard{task_queue_mutex_};

      for (PackagedTask &task : task_queue_) {
        // on suspend, we still read the requested data but only the part that
        // was read

        // the task might have been requested for cancelation before it got to
        // us
        if (task.cancelation_proxy.try_acknowledge_cancel()) {
          // don't submit the task
          continue;
        } else {
          RunningTask running_task{std::move(task)};

          auto const &url = running_task.info->task.request.url;
          auto const &header = running_task.info->task.request.header;
          auto const &easy = running_task.info->easy;
          auto &curl_header = running_task.info->header;

          VLK_CURLE_ENSURE(
              curl_easy_setopt(easy.get(), CURLOPT_URL, url.c_str()));

          for (auto const &[key, value] : header) {
            auto joined = key + ":" + value;
            curl_slist *raw_curl_header = curl_header.release();

            raw_curl_header =
                curl_slist_append(raw_curl_header, joined.c_str());

            curl_header.reset(raw_curl_header);
          }

          VLK_CURLE_ENSURE(curl_easy_setopt(easy.get(), CURLOPT_HTTPHEADER,
                                            curl_header.get()));

          switch (running_task.info->task.request.verb) {
            case Verb::Get: {
            } break;

            case Verb::Head: {
              VLK_CURLE_ENSURE(
                  curl_easy_setopt(easy.get(), CURLOPT_NOBODY, 1L));
            } break;
          }

          VLK_CURLE_ENSURE(curl_easy_setopt(easy.get(), CURLOPT_WRITEDATA,
                                            running_task.info.get()));
          VLK_CURLE_ENSURE(curl_easy_setopt(easy.get(), CURLOPT_WRITEFUNCTION,
                                            curl_content_write_function));

          // must remove once done with request
          //
          // considered to be running at this point
          VLK_CURLM_ENSURE(curl_multi_add_handle(multi_.get(), easy.get()));

          running_tasks_.push_back(std::move(running_task));
        }
      }
    }

    // we need a tick frequency
    // if running
    // ensure progression of tasks curl_multi_perform and curl_poll
    // observe and forward cancelation requests

    int present_num_running_handles = 0;
    VLK_CURLM_ENSURE(
        curl_multi_perform(multi_.get(), &present_num_running_handles));

    for (auto &task : running_tasks_) {
      if (task.info->task.cancelation_proxy.try_acknowledge_cancel()) {
        // cancel
      } else {
        RawProgress progress;

        {
          curl_off_t size = 0;
          curl_easy_getinfo(task.info->easy.get(), CURLINFO_SIZE_UPLOAD_T,
                            &size);
          progress.bytes_sent = size;
        }

        {
          curl_off_t size = 0;
          curl_easy_getinfo(task.info->easy.get(), CURLINFO_SIZE_DOWNLOAD_T,
                            &size);
          progress.bytes_received = size;
        }

        {
          curl_off_t size = 0;

          curl_easy_getinfo(task.info->easy.get(), CURLINFO_SPEED_UPLOAD_T,
                            &size);
          progress.upload_speed = size;
        }

        {
          curl_off_t size = 0;
          curl_easy_getinfo(task.info->easy.get(), CURLINFO_SPEED_DOWNLOAD_T,
                            &size);
          progress.download_speed = size;
        }

        {
          curl_off_t size = 0;

          curl_easy_getinfo(task.info->easy.get(),
                            CURLINFO_CONTENT_LENGTH_UPLOAD_T, &size);
          progress.upload_size =
              (size == -1 ? u64_max : static_cast<uint64_t>(size));
        }

        {
          curl_off_t size = 0;
          curl_easy_getinfo(task.info->easy.get(),
                            CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &size);
          progress.download_size =
              (size == -1 ? u64_max : static_cast<uint64_t>(size));
        }

        task.info->task.progress_monitor_update_token.update(progress);

        if (task.info->task.suspend_proxy.try_acknowledge_suspend()) {
          return CURL_WRITEFUNC_PAUSE;
        }

        if (task.info->task.suspend_proxy.try_acknowledge_resume()) {
        }
      }
    }

    // one or more tasks has finished executing
    if (previous_num_running_handles != present_num_running_handles) {
      int num_messages_in_queue = 0;
      CURLMsg const *messages =
          curl_multi_info_read(multi_.get(), &num_messages_in_queue);

      for (CURLMsg const &msg :
           stx::Span<CURLMsg const>(messages, num_messages_in_queue)) {
        if (msg.msg == CURLMSG_DONE) {
          // remove from the multi interface
          curl_multi_remove_handle(multi_.get(), msg.easy_handle);

          auto pos =
              std::find_if(running_tasks_.begin(), running_tasks_.end(),
                           [easy = msg.easy_handle](RunningTask const &task) {
                             return task.info->easy.get() == easy;
                           });

          VLK_ENSURE(pos != running_tasks_.end());

          pos->info->task.promise.finish(
              pos->info->task.cancelation_token.get_status(),
              std::move(pos->info->response));

          running_tasks_.erase(pos);
        }
      }
    }

    previous_num_running_handles = present_num_running_handles;

    // cancel individually
  }

  // tasks will be submitted from multiple threads
  std::mutex task_queue_mutex_;
  std::vector<PackagedTask> task_queue_;

  std::vector<RunningTask> running_tasks_;

  CancelationToken cancelation_token_;

  CurlMulti multi_{curl_multi_init()};
  int previous_num_running_handles = 0;
};

}  // namespace http
}  // namespace vlk
