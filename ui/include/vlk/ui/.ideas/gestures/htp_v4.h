#pragma once

#include <chrono>
#include <cstddef>
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "curl/curl.h"
#include "stx/span.h"
#include "vlk/async.h"
#include "vlk/task_priority.h"
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

enum class Verb { Get, Head };

using Header = std::map<std::string, std::string>;

using Url = std::string;

struct Request {
  Url url = "https://bing.com";
  Header header{};
  Verb verb = Verb::Get;
};

enum ResponseCode : uint64_t {};

struct Response {
  Header header;
  ResponseCode code{0};
  std::vector<uint8_t> content;
  std::chrono::nanoseconds total_time{0};
  Url effective_url;
  uint64_t uploaded = 0;
  uint64_t downloaded = 0;

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
  STX_DISABLE_COPY(ProgressMonitorState)
  VLK_DISABLE_MOVE(ProgressMonitorState)
  VLK_DEFAULT_CONSTRUCTOR(ProgressMonitorState)

  STX_CACHELINE_ALIGNED std::atomic<uint64_t> total_bytes_sent = 0;
  STX_CACHELINE_ALIGNED std::atomic<uint64_t> total_bytes_received = 0;
  STX_CACHELINE_ALIGNED std::atomic<uint64_t> bytes_sent = 0;
  STX_CACHELINE_ALIGNED std::atomic<uint64_t> bytes_received = 0;
  STX_CACHELINE_ALIGNED std::atomic<uint64_t> upload_speed = 0;
  STX_CACHELINE_ALIGNED std::atomic<uint64_t> download_speed = 0;
  STX_CACHELINE_ALIGNED std::atomic<uint64_t> upload_size = 0;
  STX_CACHELINE_ALIGNED std::atomic<uint64_t> download_size = 0;

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

  void update(RawProgress const &progress) {
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
    ProgressMonitor monitor{};
    monitor.state = std::make_shared<ProgressMonitorState>();
    return std::move(monitor);
  }

  Progress get_progress() const { return state->load(); }

  bool is_valid() const { return state != nullptr; }

 private:
  std::shared_ptr<ProgressMonitorState> state;
};

struct ProgressMonitorUpdateProxy {
  explicit ProgressMonitorUpdateProxy(ProgressMonitor const &monitor)
      : state{monitor.state} {}

  VLK_DEFAULT_CONSTRUCTOR(ProgressMonitorUpdateProxy)

  void update(RawProgress const &progress) const {
    auto shared_state = state.lock();
    if (shared_state == nullptr) {
      // the user is no longer interested in observing the progress of the
      // request
    } else {
      shared_state->update(progress);
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
  ProgressMonitorUpdateProxy progress_monitor_update_token;
  TaskPriority priority{0};

  PackagedTask(Request &&request, Promise<Response> &&init_promise,
               ProgressMonitor const &monitor, TaskPriority task_priority)
      : request{std::move(request)},
        promise{std::move(init_promise)},
        progress_monitor_update_token{ProgressMonitorUpdateProxy{monitor}},
        priority{task_priority} {}
};

struct CurlMultiHandle {
  VLK_MAKE_HANDLE(CurlMultiHandle)

  CURLM *multi = nullptr;

  ~CurlMultiHandle() {
    if (multi != nullptr) {
      VLK_CURLM_ENSURE(curl_multi_cleanup(multi));
    }
  }
};

struct CurlMulti {
  static CurlMulti create() {
    return CurlMulti{std::shared_ptr<CurlMultiHandle>{
        new CurlMultiHandle{curl_multi_init()}}};
  }

  bool is_valid() const { return handle != nullptr; }

  std::shared_ptr<CurlMultiHandle> handle;
};

struct CurlEasyHandle {
  VLK_MAKE_HANDLE(CurlEasyHandle)

  ~CurlEasyHandle() {
    if (easy != nullptr) {
      if (parent.is_valid()) {
        // remove from the multi interface
        VLK_CURLM_ENSURE(curl_multi_remove_handle(parent.handle->multi, easy));
      }

      curl_easy_cleanup(easy);
    }
    if (header != nullptr) {
      curl_slist_free_all(header);
    }
  }

  CURL *easy = nullptr;
  curl_slist *header = nullptr;
  CurlMulti parent;
};

struct RunningTaskInfo;

static size_t content_write_function(uint8_t const *bytes, size_t unit_size,
                                     size_t nmemb, RunningTaskInfo *task_info);

struct CurlEasy {
  static CurlEasy create(PackagedTask const &task, CurlMulti const &multi,
                         RunningTaskInfo *info) {
    CurlEasy easy{std::make_shared<CurlEasyHandle>()};
    easy.handle->easy = curl_easy_init();

    auto const &url = task.request.url;
    auto const &header = task.request.header;
    Verb verb = task.request.verb;

    switch (verb) {
      case Verb::Get: {
      } break;

      case Verb::Head: {
        VLK_CURLE_ENSURE(
            curl_easy_setopt(easy.handle->easy, CURLOPT_NOBODY, 1L));
      } break;
    }

    VLK_CURLE_ENSURE(
        curl_easy_setopt(easy.handle->easy, CURLOPT_URL, url.c_str()));

    for (auto const &[key, value] : header) {
      auto joined = key + ":" + value;

      easy.handle->header =
          curl_slist_append(easy.handle->header, joined.c_str());
    }

    VLK_CURLE_ENSURE(curl_easy_setopt(easy.handle->easy, CURLOPT_HTTPHEADER,
                                      easy.handle->header));

    VLK_CURLE_ENSURE(curl_easy_setopt(easy.handle->easy, CURLOPT_VERBOSE, 1L));
    VLK_CURLE_ENSURE(
        curl_easy_setopt(easy.handle->easy, CURLOPT_WRITEDATA, info));
    VLK_CURLE_ENSURE(curl_easy_setopt(easy.handle->easy, CURLOPT_WRITEFUNCTION,
                                      content_write_function));

    VLK_CURLM_ENSURE(
        curl_multi_add_handle(multi.handle->multi, easy.handle->easy));

    easy.handle->parent = multi;

    return std::move(easy);
  }

  bool is_valid() const { return handle != nullptr; }

  std::shared_ptr<CurlEasyHandle> handle;
};

enum class CancelState { Uncanceled, UserCanceled, ExecutorCanceled };
enum class SuspendState { Resumed, UserSuspended };

struct RunningTaskInfo {
  PackagedTask packaged_task;
  CurlEasy easy;
  Response response;
  CancelState cancel_state = CancelState::Uncanceled;
  SuspendState suspend_state = SuspendState::Resumed;

  void update_progress() {
    RawProgress progress;
    CURL *easyr = easy.handle->easy;

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
      progress.upload_size =
          (size == -1 ? u64_max : static_cast<uint64_t>(size));
    }

    {
      curl_off_t size = 0;
      VLK_CURLE_ENSURE(
          curl_easy_getinfo(easyr, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &size));
      progress.download_size =
          (size == -1 ? u64_max : static_cast<uint64_t>(size));
    }

    packaged_task.progress_monitor_update_token.update(progress);
  }
};

static size_t content_write_function(uint8_t const *bytes, size_t unit_size,
                                     size_t nmemb, RunningTaskInfo *task_info) {
  auto total_size = nmemb * unit_size;

  auto &promise = task_info->packaged_task.promise;

  switch (promise.fetch_executor_requested_cancel_state()) {
    case RequestedCancelState::Canceled: {
      promise.notify_force_cancel_begin();
      task_info->cancel_state = CancelState::ExecutorCanceled;

      // signals to libcurl that we want to cancel the request
      return 0;
    } break;

    case RequestedCancelState::Uncanceled: {
      switch (promise.fetch_user_requested_cancel_state()) {
        case RequestedCancelState::Canceled: {
          promise.notify_user_cancel_begin();
          task_info->cancel_state = CancelState::UserCanceled;

          // signals to libcurl that we want to cancel the request
          return 0;
        } break;

        case RequestedCancelState::Uncanceled: {
        }
      }
    } break;
  }

  task_info->response.content.insert(task_info->response.content.end(), bytes,
                                     bytes + total_size);

  return total_size;
}

struct RunningTask {
  // we need a stable address for `Response` and `task` to interact with curl
  std::unique_ptr<RunningTaskInfo> info;

  explicit RunningTask(PackagedTask &&task, CurlMulti const &multi) {
    info = std::unique_ptr<RunningTaskInfo>{
        new RunningTaskInfo{std::move(task), CurlEasy{}, Response{}}};

    info->easy = CurlEasy::create(info->packaged_task, multi, info.get());
    info->packaged_task.promise.notify_scheduled();
  }
};

struct TaskQueue {
  enum class Mode { AcceptNonCritical, RejectNonCritical };

  void start_reject_noncritical() {
    mode_.store(Mode::RejectNonCritical, std::memory_order_relaxed);
  }

  void force_cancel_scheduled_noncritical() {
    // cancel all pending tasks
    std::lock_guard guard{tasks_mutex_};
    // we need to not accept incoming tasks?
    // or just mark now-incoming tasks as canceled immediately
    for (PackagedTask &task : tasks_) {
      task.promise.notify_force_canceled();
    }

    auto new_end = std::remove_if(
        tasks_.begin(), tasks_.end(), [](PackagedTask const &task) {
          return task.priority < TaskPriority::Critical;
        });

    tasks_.erase(new_end, tasks_.end());
  }

  std::tuple<Future<Response>, ProgressMonitor> submit_task(
      Request &&request, TaskPriority priority) {
    auto [future, promise] = Async::create<Response>();
    auto progress_monitor = ProgressMonitor::create();

    if (mode_.load(std::memory_order_relaxed) == Mode::RejectNonCritical &&
        priority < TaskPriority::Critical) {
      // force canceled
      promise.notify_force_canceled();
    } else {
      std::lock_guard guard{tasks_mutex_};
      tasks_.push_back(PackagedTask{std::move(request), std::move(promise),
                                    progress_monitor, priority});
    }

    return std::make_tuple(std::move(future), std::move(progress_monitor));
  }

  // pop task from the task queue, if it is not presently in use.
  // this ensures the executor thread is not blocked and the submitting thread
  // is not blocked for too long.
  stx::Option<PackagedTask> try_pop_task() {
    if (tasks_mutex_.try_lock()) {
      std::lock_guard guard{tasks_mutex_, std::adopt_lock};
      if (tasks_.empty()) {
        return stx::None;
      } else {
        PackagedTask task = std::move(tasks_.front());
        tasks_.erase(tasks_.begin());
        return stx::Some(std::move(task));
      }
    } else {
      return stx::None;
    }
  }

 private:
  STX_CACHELINE_ALIGNED std::atomic<Mode> mode_{Mode::AcceptNonCritical};
  std::mutex tasks_mutex_;
  std::vector<PackagedTask> tasks_;
};

struct ExecutionContextHandle {
  enum class State { Active, ShuttingDown, Shutdown };

  VLK_MAKE_HANDLE(ExecutionContextHandle)

  ExecutionContextHandle(CurlMulti &&multi, Promise<void> &&promise)
      : multi_{std::move(multi)}, promise_{std::move(promise)} {}

  ~ExecutionContextHandle() {
    // cancel all tasks both critical and non-critical
    // close multi connection
    // destroy objects
    //
    // notify futures of cancelation
    // ...
    // remove curl easy handles
    // destroy pending curl easy handles
    // destroy multi_init
    //
  }

  void try_schedule_pending_tasks() {
    while (task_queue_.try_pop_task().match(
        [&](PackagedTask &&task) {
          running_tasks_.push_back(RunningTask{std::move(task), multi_});
          return true;
        },
        [] { return false; })) {
    }
  }

  void begin_shutdown() {
    task_queue_.start_reject_noncritical();
    task_queue_.force_cancel_all();
    // make curl cancel all present requests
    // for all tasks: if not already completed request promise force cancel
    // run tick again?
    // await them getting to the canceled state
    //
    // discard running task list
    // probably by running tick()? if multiperform is ok to call or something
    //
    //
    // request force-cancel of running non-critical tasks
    //
    //
    //
  }

  // submittable to from multiple threads.
  // the submitting thread has to wait until the task queue is free for tasks to
  // be added on.
  //
  // tasks can be submitted from multiple threads
  std::tuple<Future<Response>, ProgressMonitor> submit_task(
      Request &&request, TaskPriority priority) {
    // if shutdown, force cancel
    return task_queue_.submit_task(std::move(request), priority);
  }

  // only called on one thread?
  void progress_tasks() {
    // critical task enters while shutting down? or just immediately after
    // shutdown?

    // we need to still tick until all tasks have been canceled
    if (promise_.fetch_user_requested_cancel_state() ==
            RequestedCancelState::Canceled &&
        state_ == State::Active) {
      // TODO(lamarrr): if shutdown sequence not already begun
      state_ = State::ShuttingDown;
      promise_.notify_user_cancel_begin();
      begin_shutdown();
      // log
    }

    // has no already executing critical tasks
    if (state_ == State::ShuttingDown && running_tasks_.empty()) {
      state_ = State::Shutdown;
      // log
      // what about suspended tasks?
      //
      // notify user
      promise_.notify_user_canceled();
    }

    if (state_ != State::Shutdown) {
      try_schedule_pending_tasks();
      // we need a tick frequency
      // if running
      // ensure progression of tasks curl_multi_perform and curl_poll
      // observe and forward cancelation requests

      for (auto &task : running_tasks_) {
        auto &promise = task.info->packaged_task.promise;
        auto easy = task.info->easy.handle->easy;

        task.info->update_progress();

        if (state_ == State::ShuttingDown &&
            task.info->packaged_task.priority < TaskPriority::Critical) {
          promise.request_cancel();
        }

        // TODO(lamarrr): swith to if statements here
        // and force suspended tasks to resumption so we can cancel them

        auto cancel_state = task.info->cancel_state;
        auto suspend_state = task.info->suspend_state;
        auto requested_suspend_state =
            promise.fetch_user_requested_suspend_state();

        switch (task.info->cancel_state) {
          case CancelState::ExecutorCanceled: {
            promise.notify_force_canceled();
          } break;

            // if task hasn't been force canceled, then check suspension states
          case CancelState::Uncanceled: {
            // we don't request for suspension, but the user can
            switch (promise.fetch_user_requested_suspend_state()) {
              case RequestedSuspendState::Resumed: {
                switch (task.info->suspend_state) {
                  case SuspendState::Resumed: {
                    task.info->packaged_task.promise.notify_executing();
                  } break;

                  case SuspendState::UserSuspended: {
                    promise.notify_user_resume_begin();
                    VLK_CURLE_ENSURE(curl_easy_pause(easy, CURLPAUSE_CONT));
                    promise.notify_user_resumed();
                    task.info->suspend_state = SuspendState::Resumed;
                  } break;
                }
              } break;

              case RequestedSuspendState::Suspended: {
                switch (task.info->suspend_state) {
                  case SuspendState::Resumed: {
                    promise.notify_user_suspend_begin();
                    VLK_CURLE_ENSURE(curl_easy_pause(easy, CURLPAUSE_ALL));
                    promise.notify_user_suspended();
                    task.info->suspend_state = SuspendState::UserSuspended;
                  } break;

                  case SuspendState::UserSuspended: {
                  } break;
                }
                break;
              }
            }
            break;
          }

          case CancelState::UserCanceled: {
            promise.notify_user_canceled();
          } break;
        }
      }

      {
        // remove canceled tasks from the list
        auto new_end = std::remove_if(
            running_tasks_.begin(), running_tasks_.end(),
            [](RunningTask const &task) {
              return task.info->cancel_state != CancelState::Uncanceled;
            });
        running_tasks_.erase(new_end, running_tasks_.end());
      }

      VLK_CURLM_ENSURE(
          curl_multi_perform(multi_.handle->multi, &num_running_handles_));

      // one or more tasks have finished executing
      int num_messages_in_queue = 0;
      CURLMsg const *messages =
          curl_multi_info_read(multi_.handle->multi, &num_messages_in_queue);

      // fuck this.
      // CURL can return 0 along with a valid pointer which would mean there's a
      // single message on the queue
      if (num_messages_in_queue == 0 && messages != nullptr) {
        num_messages_in_queue = 1;
      }

      for (CURLMsg const &msg :
           stx::Span<CURLMsg const>(messages, num_messages_in_queue)) {
        if (msg.msg == CURLMSG_DONE) {
          auto task_pos =
              std::find_if(running_tasks_.begin(), running_tasks_.end(),
                           [easy = msg.easy_handle](RunningTask const &task) {
                             return task.info->easy.handle->easy == easy;
                           });

          VLK_ENSURE(task_pos != running_tasks_.end());

          auto &info = task_pos->info;
          CURL *easy = info->easy.handle->easy;

          // get status and more completion info
          char const *effective_url = nullptr;
          VLK_CURLE_ENSURE(
              curl_easy_getinfo(easy, CURLINFO_EFFECTIVE_URL, &effective_url));

          if (effective_url != nullptr) {
            info->response.effective_url = effective_url;
          }

          curl_off_t total_time = 0;
          VLK_CURLE_ENSURE(
              curl_easy_getinfo(easy, CURLINFO_TOTAL_TIME_T, &total_time));
          info->response.total_time = std::chrono::microseconds(total_time);

          curl_off_t total_downloaded = 0;
          curl_off_t total_uploaded = 0;

          VLK_CURLE_ENSURE(curl_easy_getinfo(
              easy, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &total_downloaded));
          VLK_CURLE_ENSURE(curl_easy_getinfo(
              easy, CURLINFO_CONTENT_LENGTH_UPLOAD_T, &total_uploaded));

          info->response.downloaded = total_downloaded;
          info->response.uploaded = total_uploaded;

          curl_off_t response_code = 0;
          VLK_CURLE_ENSURE(
              curl_easy_getinfo(easy, CURLINFO_RESPONSE_CODE, &response_code));
          info->response.code = ResponseCode{response_code};

          // notify of completion
          task_pos->info->packaged_task.promise.notify_completed(
              std::move(task_pos->info->response));

          running_tasks_.erase(task_pos);
        }
      }
    }
  }

  CurlMulti multi_;
  Promise<void> promise_;

  int num_running_handles_ = 0;
  TaskQueue task_queue_{};
  std::vector<RunningTask> running_tasks_{};
  State state_ = State::Active;
};

struct ExecutionContext {
  static std::pair<Future<void>, ExecutionContext> create() {
    auto [future, promise] = Async::create<void>();

    return std::make_pair(
        std::move(future),
        ExecutionContext{std::make_unique<ExecutionContextHandle>(
            CurlMulti::create(), std::move(promise))});
  }

  std::unique_ptr<ExecutionContextHandle> handle;
};

}  // namespace http
}  // namespace vlk
