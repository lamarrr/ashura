#pragma once

#include <chrono>
#include <cstddef>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "curl/curl.h"
#include "fmt/format.h"
#include "stx/async.h"
#include "stx/limits.h"
#include "stx/span.h"
#include "stx/task/priority.h"
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

inline stx::FixedReport operator>>(stx::ReportQuery, CURLcode code) noexcept {
  return stx::FixedReport(fmt::format("{}", static_cast<int>(code)));
}

inline stx::FixedReport operator>>(stx::ReportQuery, CURLMcode code) noexcept {
  return stx::FixedReport(fmt::format("{}", static_cast<int>(code)));
}

enum class Verb : uint8_t { Get, Head };

using Header = std::map<std::string, std::string>;

using Url = std::string;

struct Request {
  Url url = "https://bing.com";
  Header header{};
  Verb verb = Verb::Get;
  uint16_t maximum_redirects = stx::u16_max;
};

enum ResponseCode : uint64_t {};

struct Response {
  ResponseCode code{0};
  std::vector<uint8_t> header;
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
    if (upload_size_ == stx::u64_max) return stx::None;
    return stx::Some(uint64_t{upload_size_});
  }

  stx::Option<uint64_t> download_size() const {
    if (download_size_ == stx::u64_max) return stx::None;
    return stx::Some(uint64_t{download_size_});
  }

 private:
  uint64_t upload_size_ = stx::u64_max;
  uint64_t download_size_ = stx::u64_max;
};

struct RawProgress {
  uint64_t bytes_sent = 0;
  uint64_t bytes_received = 0;
  uint64_t upload_speed = 0;
  uint64_t download_speed = 0;
  uint64_t upload_size = stx::u64_max;
  uint64_t download_size = stx::u64_max;
};

struct ProgressMonitorState {
  STX_DEFAULT_CONSTRUCTOR(ProgressMonitorState)
  STX_MAKE_PINNED(ProgressMonitorState)

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
  Progress get_progress() const { return state.get()->load(); }

  stx::mem::Rc<ProgressMonitorState const> state;
};

struct ProgressUpdateProxy {
  void update(RawProgress const &progress) const {
    state.get()->update(progress);
  }

  stx::mem::Rc<ProgressMonitorState> state;
};

inline std::pair<ProgressMonitor, ProgressUpdateProxy> make_progress_monitor() {
  auto state = stx::mem::make_rc_inplace<ProgressMonitorState>();

  auto progress_monitor = ProgressMonitor{stx::transmute(
      static_cast<ProgressMonitorState const *>(state.get()), state)};
  return std::make_pair(std::move(progress_monitor),
                        ProgressUpdateProxy{std::move(state)});
}

struct PackagedTask {
  Request request;
  stx::Promise<Response> promise;
  // when context is about to shutdown, cancelation is requested so we need to
  // store it
  ProgressUpdateProxy progress_update_proxy;
  stx::TaskPriority priority{0};

  PackagedTask(Request &&task_request, stx::Promise<Response> &&task_promise,
               ProgressUpdateProxy &&task_progress_update_proxy,
               stx::TaskPriority task_priority)
      : request{std::move(task_request)},
        promise{std::move(task_promise)},
        progress_update_proxy{std::move(task_progress_update_proxy)},
        priority{task_priority} {}
};

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

inline stx::mem::Rc<CurlMultiHandle> make_curl_multi_handle() {
  return stx::mem::make_rc_inplace<CurlMultiHandle>(curl_multi_init());
}

struct RunningTaskInfo;

size_t curl_content_write_function(uint8_t const *bytes, size_t unit_size,
                                   size_t nmemb, RunningTaskInfo *task_info);

size_t curl_header_write_function(uint8_t const *bytes, size_t unit_size,
                                  size_t nmemb, RunningTaskInfo *task_info);

struct CurlEasyHandle {
  STX_DISABLE_DEFAULT_CONSTRUCTOR(CurlEasyHandle)
  STX_DISABLE_COPY(CurlEasyHandle)
  STX_DISABLE_MOVE(CurlEasyHandle)

  CURL *easy;
  curl_slist *header;
  stx::mem::Rc<CurlMultiHandle> parent;

  CurlEasyHandle(CURL *easy_easy, curl_slist *easy_header,
                 stx::mem::Rc<CurlMultiHandle> const &easy_parent)
      : easy{easy_easy}, header{easy_header}, parent{easy_parent} {}

  ~CurlEasyHandle() {
    // remove from the multi interface
    VLK_CURLM_ENSURE(curl_multi_remove_handle(parent.get()->multi, easy));
    curl_easy_cleanup(easy);
    // NOTE: curl accepts nullptr for headers which means empty header
    curl_slist_free_all(header);
  }

  void begin_request(stx::mem::Rc<CurlMultiHandle> const &parent,
                     RunningTaskInfo *info_addr) {
    VLK_CURLE_ENSURE(curl_easy_setopt(easy, CURLOPT_WRITEDATA, info_addr));
    VLK_CURLE_ENSURE(curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION,
                                      curl_content_write_function));
    VLK_CURLE_ENSURE(curl_easy_setopt(easy, CURLOPT_HEADERDATA, info_addr));
    VLK_CURLE_ENSURE(curl_easy_setopt(easy, CURLOPT_HEADERFUNCTION,
                                      curl_header_write_function));
    VLK_CURLM_ENSURE(curl_multi_add_handle(parent.get()->multi, easy));
  }
};

inline stx::mem::Rc<CurlEasyHandle> make_curl_easy_handle(
    stx::mem::Rc<CurlMultiHandle> const &parent, PackagedTask const &task) {
  stx::Rc easy_handle_rc = stx::mem::make_rc_inplace<CurlEasyHandle>(
      curl_easy_init(), nullptr, parent);

  CurlEasyHandle *easy_handle = easy_handle_rc.get();

  Verb verb = task.request.verb;

  switch (verb) {
    case Verb::Get: {
    } break;

    case Verb::Head: {
      VLK_CURLE_ENSURE(curl_easy_setopt(easy_handle->easy, CURLOPT_NOBODY, 1L));
    } break;
  }

  auto const &url = task.request.url;
  auto const &header = task.request.header;

  VLK_CURLE_ENSURE(
      curl_easy_setopt(easy_handle->easy, CURLOPT_URL, url.c_str()));

  for (auto const &[key, value] : header) {
    auto joined = key + ":" + value;

    easy_handle->header =
        curl_slist_append(easy_handle->header, joined.c_str());
  }

  VLK_CURLE_ENSURE(curl_easy_setopt(easy_handle->easy, CURLOPT_HTTPHEADER,
                                    easy_handle->header));

  VLK_CURLE_ENSURE(curl_easy_setopt(easy_handle->easy, CURLOPT_VERBOSE, 1L));

  curl_easy_setopt(easy_handle->easy, CURLOPT_FOLLOWLOCATION, 1);
  curl_easy_setopt(easy_handle->easy, CURLOPT_MAXREDIRS,
                   task.request.maximum_redirects);

  return easy_handle_rc;
}

// we request for cancelation of non-critical tasks once our executor is about
// to shutdown
enum class CancelState : uint8_t { Uncanceled, ExecutorCanceled, UserCanceled };

// we don't force suspension, but we force critical tasks to a resumed state
// once our executor is about to shutdown so they may be canceled
enum class SuspendState : uint8_t { Resumed, Suspended };

struct RunningTaskInfo {
  stx::mem::Rc<CurlEasyHandle> easy;
  stx::RequestProxy request_proxy;
  PackagedTask packaged_task;
  Response response;
  CancelState cancel_state = CancelState::Uncanceled;
  SuspendState suspend_state = SuspendState::Resumed;

  void update_progress() {
    RawProgress progress;
    CURL *easyr = easy.get()->easy;

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
          (size == -1 ? stx::u64_max : static_cast<uint64_t>(size));
    }

    {
      curl_off_t size = 0;
      VLK_CURLE_ENSURE(
          curl_easy_getinfo(easyr, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &size));
      progress.download_size =
          (size == -1 ? stx::u64_max : static_cast<uint64_t>(size));
    }

    packaged_task.progress_update_proxy.update(progress);
  }
};

struct RunningTask {
  // we need a stable address for `Response` and `task` to interact with curl
  // TODO(lamarrr): use Unique
  std::unique_ptr<RunningTaskInfo> info;

  explicit RunningTask(PackagedTask &&task,
                       stx::mem::Rc<CurlMultiHandle> const &parent)
      : info{std::make_unique<RunningTaskInfo>(RunningTaskInfo{
            make_curl_easy_handle(parent, task),
            stx::RequestProxy{task.promise}, std::move(task), Response{}})} {
    info.get()->easy.get()->begin_request(parent, info.get());
  }
};

struct TaskQueue {
  enum class Mode : uint8_t { Accept, Reject };

  void start_reject() { mode_.store(Mode::Reject, std::memory_order_relaxed); }

  void force_cancel_and_remove_scheduled_noncritical() {
    // cancel all pending tasks
    std::lock_guard guard{tasks_mutex_};

    // we need to not accept incoming tasks?
    // or just mark now-incoming tasks as canceled immediately
    for (PackagedTask &task : tasks_) {
      if (task.priority < stx::TaskPriority::Critical) {
        task.promise.notify_force_canceled();
      }
    }

    auto new_end = std::remove_if(
        tasks_.begin(), tasks_.end(), [](PackagedTask const &task) {
          return task.priority < stx::TaskPriority::Critical;
        });

    tasks_.erase(new_end, tasks_.end());
  }

  std::tuple<stx::Future<Response>, ProgressMonitor> submit_task(
      Request &&request, stx::TaskPriority priority) {
    auto [future, promise] = stx::make_future<Response>();
    auto [progress_monitor, progress_update_proxy] = make_progress_monitor();

    auto mode = mode_.load(std::memory_order_relaxed);

    if (mode == Mode::Reject) {
      promise.notify_force_canceled();
    } else {
      // the task submitter just has to wait until the queue is free.
      // this is usually a very short period of time.
      std::lock_guard guard{tasks_mutex_};
      promise.notify_scheduled();
      tasks_.push_back(PackagedTask{std::move(request), std::move(promise),
                                    std::move(progress_update_proxy),
                                    priority});
      num_scheduled_critical_.fetch_add(1, std::memory_order_relaxed);
    }

    return std::make_tuple(std::move(future), std::move(progress_monitor));
  }

  // pop task from the task queue, if it is not presently in use.
  // this ensures the executor thread is not blocked and the submitting thread
  // is not blocked for too long. once this method returns None, either from the
  // mutex aready being locked or no task being available, it must call it at a
  // later point in time.
  stx::Option<PackagedTask> try_pop_task() {
    if (tasks_mutex_.try_lock()) {
      std::lock_guard guard{tasks_mutex_, std::adopt_lock};
      if (tasks_.empty()) {
        return stx::None;
      } else {
        PackagedTask task = std::move(tasks_.front());
        tasks_.erase(tasks_.begin());
        num_scheduled_critical_.fetch_sub(1, std::memory_order_relaxed);
        return stx::Some(std::move(task));
      }
    } else {
      return stx::None;
    }
  }

  uint64_t fetch_num_scheduled_critical() const {
    return num_scheduled_critical_.load(std::memory_order_relaxed);
  }

 private:
  STX_CACHELINE_ALIGNED std::atomic<Mode> mode_{Mode::Accept};
  STX_CACHELINE_ALIGNED std::atomic<uint64_t> num_scheduled_critical_ = 0;
  std::mutex tasks_mutex_;
  std::vector<PackagedTask> tasks_;
};

// on shutdown request, we will not accept any new request.
// we will cancel all pending non-critical tasks.
// we will ensure all already scheduled or executing critical tasks are run to
// completion before finishing shutdown.
struct ExecutionContextHandle {
  enum class State : uint8_t {
    // all incoming tasks are accepted and executed
    Active,
    // all incoming tasks are rejected. already scheduled or executing
    // non-critical tasks are canceled.
    // scheduled or running critical tasks are completed.
    UserShuttingDown,
    // all incoming tasks are rejected. all scheduled critical tasks have
    // finished
    // executing
    Shutdown
  };

  STX_MAKE_PINNED(ExecutionContextHandle)

  ExecutionContextHandle(stx::Promise<void> &&promise)
      : multi_{make_curl_multi_handle()},
        promise_{std::move(promise)},
        request_proxy_{promise_} {}

  // submittable to from multiple threads.
  // the submitting thread has to wait until the task queue is free for tasks to
  // be added on.
  //
  // tasks can be submitted from multiple threads
  std::tuple<stx::Future<Response>, ProgressMonitor> submit_task(
      Request &&request, stx::TaskPriority priority) {
    return task_queue_.submit_task(std::move(request), priority);
  }

  // only called on one thread
  void tick() {
    // critical task enters while shutting down? or just immediately after
    // shutdown?

    // we need to still tick until all tasks have been canceled
    //
    // ignore shutdown request until we have no critical tasks in the schedule
    // and execution queue
    //
    //
    auto cancel_request = request_proxy_.fetch_cancel_request();

    if (cancel_request.state == stx::RequestedCancelState::Canceled) {
      if (state_ == State::Active) {
        promise_.notify_user_cancel_begin();
        state_ = State::UserShuttingDown;
        task_queue_.start_reject();
        task_queue_.force_cancel_and_remove_scheduled_noncritical();

        for (auto &task : running_tasks_) {
          auto *info = task.info.get();
          //  the critical ones must be run to completion and
          // the non-critical ones immediately canceled.
          info->packaged_task.promise.request_force_resume();

          if (info->packaged_task.priority < stx::TaskPriority::Critical) {
            info->packaged_task.promise.request_force_cancel();
          }
        }
      } else if (state_ == State::UserShuttingDown &&
                 task_queue_.fetch_num_scheduled_critical() == 0 &&
                 count_num_running_critical() == 0) {
        // log
        promise_.notify_user_canceled();
        state_ = State::Shutdown;
      }
    }

    if (state_ == State::Active || state_ == State::UserShuttingDown) {
      // try to begin execution of all scheduled tasks without blocking this
      // thread.
      //
      // NOTE that some tasks may be left in the queue and we'd have to attend
      // to them at another call of `tick()`.
      while (task_queue_.try_pop_task().match(
          [this](PackagedTask &&task) {
            running_tasks_.push_back(RunningTask{std::move(task), multi_});
            return true;
          },
          [] { return false; })) {
      }

      // we need a tick frequency
      // if running
      // ensure progression of tasks `curl_multi_perform` and `curl_poll`
      // observe and forward cancelation requests
      for (auto &task : running_tasks_) {
        auto *info = task.info.get();
        auto &promise = info->packaged_task.promise;
        auto *easy = info->easy.get()->easy;

        info->update_progress();

        if (state_ == State::UserShuttingDown &&
            info->packaged_task.priority < stx::TaskPriority::Critical) {
          promise.request_force_cancel();
        }

        if (state_ == State::UserShuttingDown) {
          promise.request_force_resume();
        }

        auto cancel_state = info->cancel_state;
        auto suspend_state = info->suspend_state;
        auto &request_proxy = info->request_proxy;
        auto cancel_request = request_proxy.fetch_cancel_request();
        auto suspend_request = request_proxy.fetch_suspend_request();

        if (cancel_request.state == stx::RequestedCancelState::Canceled &&
            suspend_state == SuspendState::Suspended) {
          // for cancelation to happen in CURL, the task must first be put in a
          // resumed state
          promise.request_force_resume();
        }

        // attend to suspension, cancelation and resumption requests
        if (cancel_state == CancelState::Uncanceled) {
          if (suspend_request.state == stx::RequestedSuspendState::Suspended &&
              suspend_state == SuspendState::Resumed) {
            if (suspend_request.source == stx::RequestSource::Executor) {
              promise.notify_force_suspend_begin();
              VLK_CURLE_ENSURE(curl_easy_pause(easy, CURLPAUSE_ALL));
              info->suspend_state = SuspendState::Suspended;
              promise.notify_force_suspended();
            } else {
              promise.notify_user_suspend_begin();
              VLK_CURLE_ENSURE(curl_easy_pause(easy, CURLPAUSE_ALL));
              info->suspend_state = SuspendState::Suspended;
              promise.notify_user_suspended();
            }
          } else if (suspend_request.state ==
                         stx::RequestedSuspendState::Resumed &&
                     suspend_state == SuspendState::Suspended) {
            if (suspend_request.source == stx::RequestSource::User) {
              promise.notify_user_resume_begin();
              VLK_CURLE_ENSURE(curl_easy_pause(easy, CURLPAUSE_CONT));
              info->suspend_state = SuspendState::Resumed;
              promise.notify_user_resumed();
            } else {
              promise.notify_force_resume_begin();
              VLK_CURLE_ENSURE(curl_easy_pause(easy, CURLPAUSE_CONT));
              info->suspend_state = SuspendState::Resumed;
              promise.notify_force_resumed();
            }
          }
        } else if (cancel_state == CancelState::UserCanceled) {
          promise.notify_user_canceled();
        } else if (cancel_state == CancelState::ExecutorCanceled) {
          promise.notify_force_canceled();
        }

        {
          // remove canceled tasks from the list
          auto new_end =
              std::remove_if(running_tasks_.begin(), running_tasks_.end(),
                             [](RunningTask const &task) {
                               return task.info.get()->cancel_state ==
                                          CancelState::UserCanceled ||
                                      task.info.get()->cancel_state ==
                                          CancelState::ExecutorCanceled;
                             });
          running_tasks_.erase(new_end, running_tasks_.end());
        }

        VLK_CURLM_ENSURE(
            curl_multi_perform(multi_.get()->multi, &num_running_handles_));

        // one or more tasks have finished executing
        int num_messages_in_queue = 0;
        CURLMsg const *messages =
            curl_multi_info_read(multi_.get()->multi, &num_messages_in_queue);

        // CURL can return 0 along with a valid pointer which would mean
        // there's a single message on the queue
        if (num_messages_in_queue == 0 && messages != nullptr) {
          num_messages_in_queue = 1;
        }

        for (CURLMsg const &msg :
             stx::Span<CURLMsg const>(messages, num_messages_in_queue)) {
          if (msg.msg == CURLMSG_DONE) {
            auto task_pos =
                std::find_if(running_tasks_.begin(), running_tasks_.end(),
                             [easy = msg.easy_handle](RunningTask const &task) {
                               return task.info.get()->easy.get()->easy == easy;
                             });

            VLK_ENSURE(task_pos != running_tasks_.end());

            auto *info = task_pos->info.get();
            CURL *easy = info->easy.get()->easy;

            // get status and more completion info
            char const *effective_url = nullptr;
            VLK_CURLE_ENSURE(curl_easy_getinfo(easy, CURLINFO_EFFECTIVE_URL,
                                               &effective_url));

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
            VLK_CURLE_ENSURE(curl_easy_getinfo(easy, CURLINFO_RESPONSE_CODE,
                                               &response_code));
            // TODO(check for negative)
            info->response.code = ResponseCode{response_code};

            // notify of completion
            info->packaged_task.promise.notify_completed(
                std::move(info->response));

            running_tasks_.erase(task_pos);
          }
        }
      }
    }
  }

  uint64_t count_num_running_critical() const {
    return std::count_if(running_tasks_.begin(), running_tasks_.end(),
                         [](RunningTask const &task) {
                           return task.info.get()->packaged_task.priority ==
                                  stx::TaskPriority::Critical;
                         });
  }

  stx::mem::Rc<CurlMultiHandle> multi_;
  stx::Promise<void> promise_;
  stx::RequestProxy request_proxy_;

  int num_running_handles_ = 0;
  TaskQueue task_queue_{};
  std::vector<RunningTask> running_tasks_{};
  State state_ = State::Active;
};

/*
struct ExecutionContext {
  static std::pair<stx::Future<void>, ExecutionContext> create() {
    auto [future, promise] = stx::make_future<void>();

    return std::make_pair(
        std::move(future),
        ExecutionContext{std::make_unique<ExecutionContextHandle>(
            CurlMulti::create(), std::move(promise))});
  }

  stx::mem::Unique<ExecutionContextHandle> handle;
};
*/

}  // namespace http
}  // namespace vlk
