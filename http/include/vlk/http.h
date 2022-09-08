#pragma once

#include <chrono>
#include <cstddef>
#include <map>
#include <memory>
#include <mutex>
#include <tuple>
#include <utility>

#include "curl/curl.h"
#include "fmt/format.h"
#include "stx/async.h"
#include "stx/option.h"
#include "stx/spinlock.h"
#include "stx/string.h"
#include "stx/vec.h"
#include "vlk/utils.h"

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

inline std::string operator>>(stx::ReportQuery, CURLcode code) {
  return fmt::format("CURLcode{}", static_cast<int>(code));
}

inline std::string operator>>(stx::ReportQuery, CURLMcode code) {
  return fmt::format("CURLMcode{}", static_cast<int>(code));
}

enum class Verb : uint8_t { Get, Head };

struct Request {
  stx::String url = stx::string::make_static("https://fast.com");
  std::map<stx::String, stx::String> header;
  Verb verb = Verb::Get;
  uint32_t maximum_redirects = 69;
};

using ResponseCode = uint64_t;

struct Response {
  ResponseCode code{0};
  stx::Vec<uint8_t> header;
  stx::Vec<uint8_t> content;
  std::chrono::nanoseconds total_time{0};
  stx::String effective_url{};
  uint64_t uploaded{0};
  uint64_t downloaded{0};
};

struct Progress {
  uint64_t bytes_sent = 0;
  uint64_t bytes_received = 0;
  uint64_t upload_speed = 0;
  uint64_t download_speed = 0;
  stx::Option<uint64_t> content_upload_size = stx::None;
  stx::Option<uint64_t> content_download_size = stx::None;
};

struct ProgressMonitorState {
  STX_DEFAULT_CONSTRUCTOR(ProgressMonitorState)
  STX_MAKE_PINNED(ProgressMonitorState)

  Progress progress;
  stx::SpinLock lock;

  Progress load() {
    stx::LockGuard guard{lock};
    return progress;
  }

  void update(Progress const &progress_) {
    stx::LockGuard guard{lock};
    progress = progress_;
  }
};

struct ProgressMonitor {
  Progress get_progress() const { return state.handle->load(); }

  stx::Rc<ProgressMonitorState *> state;
};

struct ProgressUpdater {
  void update(Progress const &progress) const {
    state.handle->update(progress);
  }

  stx::Rc<ProgressMonitorState *> state;
};

inline stx::Result<std::pair<ProgressMonitor, ProgressUpdater>, stx::AllocError>
make_progress_monitor(stx::Allocator allocator) {
  TRY_OK(state, stx::rc::make_inplace<ProgressMonitorState>(allocator));

  ProgressMonitor progress_monitor{state.share()};

  return stx::Ok(std::make_pair(std::move(progress_monitor),
                                ProgressUpdater{std::move(state)}));
}

namespace impl {
struct CurlMultiHandle {
  STX_DISABLE_DEFAULT_CONSTRUCTOR(CurlMultiHandle)
  STX_DISABLE_COPY(CurlMultiHandle)
  STX_DISABLE_MOVE(CurlMultiHandle)

  CURLM *multi;

  explicit CurlMultiHandle(CURLM *init_multi) : multi{init_multi} {}

  ~CurlMultiHandle() {
    // curl closes connections on destroy
    VLK_CURLM_ENSURE(curl_multi_cleanup(multi));
  }
};

inline auto make_curl_multi_handle(stx::Allocator allocator) {
  CURLM *multi = curl_multi_init();
  if (multi == nullptr) stx::panic("unexpected error from curl");
  return stx::rc::make_inplace<CurlMultiHandle>(allocator, multi);
}

struct TaskInfo;

size_t curl_content_write_function(uint8_t const *bytes, size_t unit_size,
                                   size_t nmemb, TaskInfo *task_info);

size_t curl_header_write_function(uint8_t const *bytes, size_t unit_size,
                                  size_t nmemb, TaskInfo *task_info);

struct CurlEasyHandle {
  STX_DISABLE_DEFAULT_CONSTRUCTOR(CurlEasyHandle)
  STX_DISABLE_COPY(CurlEasyHandle)
  STX_DISABLE_MOVE(CurlEasyHandle)

  CURL *easy;
  curl_slist *header;
  stx::Rc<CurlMultiHandle *> parent;

  CurlEasyHandle(CURL *easy_easy, curl_slist *easy_header,
                 stx::Rc<CurlMultiHandle *> easy_parent)
      : easy{easy_easy}, header{easy_header}, parent{std::move(easy_parent)} {}

  ~CurlEasyHandle() {
    // remove from the multi interface
    VLK_CURLM_ENSURE(curl_multi_remove_handle(parent.handle->multi, easy));
    curl_easy_cleanup(easy);
    // NOTE: curl accepts nullptr for headers which means empty header
    curl_slist_free_all(header);
  }
};

struct TaskInfo {
  stx::Rc<CurlEasyHandle *> easy;
  stx::Vec<uint8_t> header;
  stx::Vec<uint8_t> content;
  stx::Promise<Response> promise;
  ProgressUpdater updater;
  stx::FutureStatus last_status_poll = stx::FutureStatus::Executing;
};

struct Task {
  // we need a stable address for `Response` and `task` to interact with curl.
  // i.e. the callback data passed to it
  stx::Unique<TaskInfo *> info;

  static inline stx::Result<stx::Rc<CurlEasyHandle *>, stx::AllocError>
  prepare_request(stx::Allocator allocator,
                  stx::Rc<CurlMultiHandle *> const &parent,
                  Request const &request) {
    CURL *easy = curl_easy_init();

    if (easy == nullptr) stx::panic("unexpected error from CURL");

    TRY_OK(easy_handle_rc, stx::rc::make_inplace<CurlEasyHandle>(
                               allocator, easy, nullptr, parent.share()));

    CurlEasyHandle *easy_handle = easy_handle_rc.handle;

    Verb verb = request.verb;

    switch (verb) {
      case Verb::Get: {
      } break;

      case Verb::Head: {
        VLK_CURLE_ENSURE(
            curl_easy_setopt(easy_handle->easy, CURLOPT_NOBODY, 1L));
      } break;
    }

    auto const &url = request.url;
    auto const &header = request.header;

    VLK_CURLE_ENSURE(
        curl_easy_setopt(easy_handle->easy, CURLOPT_URL, url.c_str()));

    for (auto const &[key, value] : header) {
      stx::String joined =
          stx::string::join(allocator, "", key, ":", value).unwrap();

      auto *new_header = curl_slist_append(easy_handle->header, joined.c_str());

      if (new_header == nullptr) stx::panic();

      easy_handle->header = new_header;
    }

    VLK_CURLE_ENSURE(curl_easy_setopt(easy_handle->easy, CURLOPT_HTTPHEADER,
                                      easy_handle->header));

    VLK_CURLE_ENSURE(curl_easy_setopt(easy_handle->easy, CURLOPT_VERBOSE, 1L));

    curl_easy_setopt(easy_handle->easy, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(easy_handle->easy, CURLOPT_MAXREDIRS,
                     request.maximum_redirects);

    return stx::Ok(std::move(easy_handle_rc));
  }

  static void begin_request(CURL *easy, CURLM *multi, TaskInfo *info_addr) {
    VLK_CURLE_ENSURE(curl_easy_setopt(easy, CURLOPT_WRITEDATA, info_addr));
    VLK_CURLE_ENSURE(curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION,
                                      impl::curl_content_write_function));
    VLK_CURLE_ENSURE(curl_easy_setopt(easy, CURLOPT_HEADERDATA, info_addr));
    VLK_CURLE_ENSURE(curl_easy_setopt(easy, CURLOPT_HEADERFUNCTION,
                                      impl::curl_header_write_function));
    VLK_CURLM_ENSURE(curl_multi_add_handle(multi, easy));
  }

  // called on every tick
  void update_progress() {
    Progress progress;
    CURL *easyr = info.handle->easy.handle->easy;

    {
      curl_off_t size = 0;
      VLK_CURLE_ENSURE(curl_easy_getinfo(easyr, CURLINFO_SIZE_UPLOAD_T, &size));
      progress.bytes_sent = size;
    }

    {
      curl_off_t size = 0;
      VLK_CURLE_ENSURE(
          curl_easy_getinfo(easyr, CURLINFO_SIZE_DOWNLOAD_T, &size));
      progress.bytes_received = size;
    }

    {
      curl_off_t size = 0;

      VLK_CURLE_ENSURE(
          curl_easy_getinfo(easyr, CURLINFO_SPEED_UPLOAD_T, &size));
      progress.upload_speed = size;
    }

    {
      curl_off_t size = 0;
      VLK_CURLE_ENSURE(
          curl_easy_getinfo(easyr, CURLINFO_SPEED_DOWNLOAD_T, &size));
      progress.download_speed = size;
    }

    {
      curl_off_t size = 0;

      VLK_CURLE_ENSURE(
          curl_easy_getinfo(easyr, CURLINFO_CONTENT_LENGTH_UPLOAD_T, &size));

      if (size == -1) {
        progress.content_upload_size = stx::None;
      } else {
        progress.content_upload_size = stx::Some(static_cast<uint64_t>(size));
      }
    }

    {
      curl_off_t size = 0;
      VLK_CURLE_ENSURE(
          curl_easy_getinfo(easyr, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &size));
      if (size == -1) {
        progress.content_download_size = stx::None;
      } else {
        progress.content_download_size = stx::Some(static_cast<uint64_t>(size));
      }
    }

    info.handle->updater.update(progress);
  }

  static stx::Result<std::tuple<Task, ProgressMonitor, stx::Future<Response>>,
                     stx::AllocError>
  launch(stx::Allocator allocator, Request const &request,
         stx::Rc<CurlMultiHandle *> const &parent) {
    TRY_OK(easy, prepare_request(allocator, parent, request));
    TRY_OK(updater, make_progress_monitor(allocator));
    TRY_OK(promise, stx::make_promise<Response>(allocator));

    auto future = promise.get_future();

    TRY_OK(task_info,
           stx::rc::make_unique_inplace<TaskInfo>(
               allocator, std::move(easy), stx::Vec<uint8_t>{allocator},
               stx::Vec<uint8_t>{allocator}, std::move(promise),
               std::move(updater.second)));

    begin_request(task_info.handle->easy.handle->easy,
                  task_info.handle->easy.handle->parent.handle->multi,
                  task_info.handle);

    return stx::Ok(std::make_tuple(Task{std::move(task_info)},
                                   std::move(updater.first),
                                   std::move(future)));
  }

  void finish(stx::Allocator allocator) {
    Response response;

    CURL *easy = info.handle->easy.handle->easy;

    // get status and more completion info
    char const *effective_url = nullptr;
    VLK_CURLE_ENSURE(
        curl_easy_getinfo(easy, CURLINFO_EFFECTIVE_URL, &effective_url));

    if (effective_url != nullptr) {
      response.effective_url =
          stx::string::make(allocator, effective_url).unwrap();
    }

    curl_off_t total_time = 0;
    VLK_CURLE_ENSURE(
        curl_easy_getinfo(easy, CURLINFO_TOTAL_TIME_T, &total_time));
    response.total_time = std::chrono::microseconds(total_time);

    curl_off_t total_downloaded = 0;
    curl_off_t total_uploaded = 0;

    VLK_CURLE_ENSURE(curl_easy_getinfo(easy, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T,
                                       &total_downloaded));
    VLK_CURLE_ENSURE(curl_easy_getinfo(easy, CURLINFO_CONTENT_LENGTH_UPLOAD_T,
                                       &total_uploaded));

    response.downloaded = total_downloaded;
    response.uploaded = total_uploaded;

    long response_code = 0;
    VLK_CURLE_ENSURE(
        curl_easy_getinfo(easy, CURLINFO_RESPONSE_CODE, &response_code));

    response.code = ResponseCode{static_cast<uint64_t>(response_code)};

    response.header = std::move(info.handle->header);
    response.content = std::move(info.handle->content);

    // notify of completion
    info.handle->promise.notify_completed(std::move(response));
  }
};

}  // namespace impl

struct Client {
  STX_MAKE_PINNED(Client)

  explicit Client(stx::Allocator allocator)
      : multi_{impl::make_curl_multi_handle(allocator).unwrap()},
        tasks_{allocator},
        lock_{},
        allocator_{allocator} {}

  // submittable to from multiple threads.
  // the submitting thread has to wait until the task queue is free for tasks to
  // be added on.
  //
  // tasks can be submitted from multiple threads
  std::tuple<stx::Future<Response>, ProgressMonitor> get(
      stx::String url, std::map<stx::String, stx::String> header = {},
      uint32_t max_redirects = 69) {
    stx::LockGuard guard{lock_};
    auto [task, monitor, future] =
        impl::Task::launch(allocator_,
                           Request{std::move(url), std::move(header), Verb::Get,
                                   max_redirects},
                           multi_)
            .unwrap();

    tasks_ = stx::vec::push(std::move(tasks_), std::move(task)).unwrap();

    return std::make_tuple(std::move(future), std::move(monitor));
  }

  std::tuple<stx::Future<Response>, ProgressMonitor> head(
      stx::String url, std::map<stx::String, stx::String> header = {},
      uint32_t max_redirects = 69) {
    stx::LockGuard guard{lock_};
    auto [task, monitor, future] =
        impl::Task::launch(allocator_,
                           Request{std::move(url), std::move(header),
                                   Verb::Head, max_redirects},
                           multi_)
            .unwrap();

    tasks_ = stx::vec::push(std::move(tasks_), std::move(task)).unwrap();

    return std::make_tuple(std::move(future), std::move(monitor));
  }

  void tick() {
    stx::LockGuard guard{lock_};

    {
      // poll statuses
      tasks_.span().for_each([](impl::Task &task) {
        task.info.handle->last_status_poll =
            task.info.handle->promise.fetch_status();
      });

      // remove canceled tasks
      auto to_erase = tasks_.span()
                          .partition([](impl::Task const &task) {
                            return task.info.handle->last_status_poll !=
                                       stx::FutureStatus::Canceled ||
                                   task.info.handle->last_status_poll ==
                                       stx::FutureStatus::Completed;
                          })
                          .second;

      tasks_ = stx::vec::erase(std::move(tasks_), to_erase);
    }

    for (impl::Task &task : tasks_) {
      task.update_progress();

      if (task.info.handle->last_status_poll == stx::FutureStatus::Suspended &&
          task.info.handle->promise.fetch_suspend_request() ==
              stx::SuspendState::Executing) {
        VLK_CURLE_ENSURE(curl_easy_pause(task.info.handle->easy.handle->easy,
                                         CURLPAUSE_CONT));
        task.info.handle->promise.notify_executing();
      }
    }

    int num_running_handles = 0;
    VLK_CURLM_ENSURE(
        curl_multi_perform(multi_.handle->multi, &num_running_handles));

    int num_messages_in_queue = 0;
    CURLMsg const *messages =
        curl_multi_info_read(multi_.handle->multi, &num_messages_in_queue);

    // CURL can return 0 along with a valid pointer which would mean
    // there's a single message on the queue
    if (num_messages_in_queue == 0 && messages != nullptr) {
      num_messages_in_queue = 1;
    }

    for (CURLMsg const &msg :
         stx::Span<CURLMsg const>(messages, num_messages_in_queue)) {
      if (msg.msg == CURLMSG_DONE) {
        stx::Span task = tasks_.span().which(
            [easy = msg.easy_handle](impl::Task const &task) {
              return task.info.handle->easy.handle->easy == easy;
            });

        VLK_ENSURE(!task.is_empty());

        task[0].finish(allocator_);
      }
    }
  }

  stx::Rc<impl::CurlMultiHandle *> multi_;
  stx::Vec<impl::Task> tasks_;
  stx::SpinLock lock_;
  stx::Allocator allocator_;
};

}  // namespace http
}  // namespace vlk
