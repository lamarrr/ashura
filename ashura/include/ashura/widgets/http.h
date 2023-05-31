#pragma once

#include <chrono>
#include <map>

#include "ashura/utils.h"
#include "curl/curl.h"
#include "fmt/format.h"
#include "stx/async.h"
#include "stx/option.h"
#include "stx/span.h"
#include "stx/spinlock.h"
#include "stx/string.h"
#include "stx/task/priority.h"
#include "stx/vec.h"

#define ASH_CURLE_ENSURE(code_expression, ...) \
  do                                           \
  {                                            \
    if ((code_expression) > 0)                 \
    {                                          \
      ASH_PANIC(__VA_ARGS__);                  \
    }                                          \
  } while (false)

#define ASH_CURLM_ENSURE(code_expression, ...) \
  do                                           \
  {                                            \
    if ((code_expression) > 0)                 \
    {                                          \
      ASH_PANIC(__VA_ARGS__);                  \
    }                                          \
  } while (false)

namespace ash
{
namespace http
{
// Overloading the operator>> to convert CURLcode to a string representation
inline std::string operator>>(stx::ReportQuery, CURLcode code)
{
  return fmt::format("CURLcode{}", static_cast<int>(code));
}

// Overloading the operator>> to convert CURLMcode to a string representation
inline std::string operator>>(stx::ReportQuery, CURLMcode code)
{
  return fmt::format("CURLMcode{}", static_cast<int>(code));
}

// Enum to represent HTTP request methods
enum class HttpMethod : uint8_t
{
  Get,
  Head
};

// Struct to represent an HTTP request
struct HttpRequest
{
  stx::String                        url = stx::string::make_static("https://fast.com");
  std::map<stx::String, stx::String> headers;
  HttpMethod                         method           = HttpMethod::Get;
  uint32_t                           maximumRedirects = 69;
};

// Struct to represent an HTTP response
struct HttpResponse
{
  uint64_t                 code{0};
  stx::Vec<uint8_t>        header;
  stx::Vec<uint8_t>        content;
  std::chrono::nanoseconds totalTime{0};
  stx::String              effectiveUrl;
  uint64_t                 uploaded{0};
  uint64_t                 downloaded{0};
};

struct Progress
{
  uint64_t              bytesSent           = 0;
  uint64_t              bytesReceived       = 0;
  uint64_t              uploadSpeed         = 0;
  uint64_t              downloadSpeed       = 0;
  stx::Option<uint64_t> contentUploadSize   = stx::None;
  stx::Option<uint64_t> contentDownloadSize = stx::None;
};

struct ProgressMonitorState
{
  STX_DEFAULT_CONSTRUCTOR(ProgressMonitorState)
  STX_MAKE_PINNED(ProgressMonitorState)

  Progress      progress;
  stx::SpinLock lock;

  Progress load()
  {
    stx::LockGuard guard{lock};
    return progress;
  }

  void update(const Progress &progress_)
  {
    stx::LockGuard guard{lock};
    progress = progress_;
  }
};

struct ProgressMonitor
{
  Progress getProgress() const
  {
    return state->load();
  }
  stx::Rc<ProgressMonitorState *> state;
};

struct ProgressUpdater
{
  void update(const Progress &progress) const
  {
    state->update(progress);
  }

  stx::Rc<ProgressMonitorState *> state;
};

inline stx::Result<std::pair<ProgressMonitor, ProgressUpdater>, stx::AllocError>
    makeProgressMonitor(stx::Allocator allocator)
{
  TRY_OK(state, stx::rc::make_inplace<ProgressMonitorState>(allocator));

  ProgressMonitor progressMonitor{state.share()};

  return stx::Ok(std::make_pair(std::move(progressMonitor),
                                ProgressUpdater{std::move(state)}));
}

namespace impl
{
// The CurlMultiHandle struct represents a multi-handle for managing multiple HTTP transfers concurrently.
// It encapsulates the underlying CURLM* multi-handle from the libcurl library.
struct CurlMultiHandle
{
  STX_DISABLE_DEFAULT_CONSTRUCTOR(CurlMultiHandle)
  STX_DISABLE_COPY(CurlMultiHandle)
  STX_DISABLE_MOVE(CurlMultiHandle)

  CURLM *multi;        // Pointer to the multi-handle

  // Constructor that initializes the CurlMultiHandle with the given multi-handle.
  explicit CurlMultiHandle(CURLM *init_multi) :
      multi{init_multi}
  {}

  // Destructor that cleans up the multi-handle by closing connections.
  ~CurlMultiHandle()
  {
    // curl_multi_cleanup closes connections associated with the multi-handle
    ASH_CURLM_ENSURE(curl_multi_cleanup(multi));
  }
};

// Function to create a CurlMultiHandle using the curl_multi_init function.
inline auto make_curl_multi_handle(stx::Allocator allocator)
{
  CURLM *multi = curl_multi_init();
  if (multi == nullptr)
  {
    stx::panic("unexpected error from curl");                             // Panic if initialization fails
  }
  return stx::rc::make_inplace<CurlMultiHandle>(allocator, multi);        // Create and return the CurlMultiHandle
}

struct CurlEasyHandle
{
  STX_DISABLE_DEFAULT_CONSTRUCTOR(CurlEasyHandle)
  STX_DISABLE_COPY(CurlEasyHandle)
  STX_DISABLE_MOVE(CurlEasyHandle)

  CURL                      *easy;
  curl_slist                *header;
  stx::Rc<CurlMultiHandle *> parent;

  CurlEasyHandle(CURL *easy_easy, curl_slist *easy_header,
                 stx::Rc<CurlMultiHandle *> easy_parent) :
      easy{easy_easy}, header{easy_header}, parent{std::move(easy_parent)}
  {}

  ~CurlEasyHandle()
  {
    ASH_CURLM_ENSURE(curl_multi_remove_handle(parent.handle->multi, easy));
    curl_easy_cleanup(easy);
    curl_slist_free_all(header);
  }
};
struct TaskInfo
{
  stx::Rc<CurlEasyHandle *>  easy;
  stx::Vec<uint8_t>          header;
  stx::Vec<uint8_t>          content;
  stx::Promise<HttpResponse> promise;
  ProgressUpdater            updater;
  stx::FutureStatus          last_status_poll = stx::FutureStatus::Executing;
};

size_t curl_header_write_function(uint8_t const *bytes, size_t unit_size,
                                  size_t nmemb, TaskInfo *task_info)
{
  size_t total_size = nmemb * unit_size;

  for (size_t i = 0; i < total_size; i++)
  {
    task_info->header.push(static_cast<uint8_t>(bytes[i])).unwrap();
  }

  return total_size;
}

size_t curl_content_write_function(uint8_t const *bytes, size_t unit_size,
                                   size_t nmemb, TaskInfo *task_info)
{
  size_t total_size = nmemb * unit_size;

  auto             &promise = task_info->promise;
  stx::RequestProxy request_proxy{promise};

  auto cancel_request  = request_proxy.fetch_cancel_request();
  auto suspend_request = request_proxy.fetch_suspend_request();

  if (cancel_request == stx::CancelState::Canceled)
  {
    promise.notify_canceled();
    return 0;
  }

  if (suspend_request == stx::SuspendState::Suspended)
  {
    promise.notify_suspended();
    return CURL_WRITEFUNC_PAUSE;
  }

  for (size_t i = 0; i < total_size; i++)
  {
    task_info->content.push(static_cast<uint8_t>(bytes[i])).unwrap();
  }

  return total_size;
}

struct Task
{
  stx::Unique<TaskInfo *> info;

  static inline stx::Result<stx::Rc<CurlEasyHandle *>, stx::AllocError>
      prepare_request(stx::Allocator                    allocator,
                      stx::Rc<CurlMultiHandle *> const &parent,
                      HttpRequest const                &request)
  {
    CURL *easy = curl_easy_init();
    if (easy == nullptr)
      stx::panic("unexpected error from CURL");
    TRY_OK(easy_handle_rc, stx::rc::make_inplace<CurlEasyHandle>(
                               allocator, easy, nullptr, parent.share()));

    CurlEasyHandle *easy_handle = easy_handle_rc.handle;

    HttpMethod verb = request.method;

    switch (verb)
    {
      case HttpMethod::Get:
        break;

      case HttpMethod::Head:
        ASH_CURLE_ENSURE(curl_easy_setopt(easy_handle->easy, CURLOPT_NOBODY, 1L));
        break;
    }
    auto const &url    = request.url;
    auto const &header = request.headers;

    ASH_CURLE_ENSURE(curl_easy_setopt(easy_handle->easy, CURLOPT_URL, url.c_str()));

    for (const auto &[key, value] : header)
    {
      stx::String joined = stx::string::join(allocator, "", key, ":", value).unwrap();

      auto *new_header = curl_slist_append(easy_handle->header, joined.c_str());
      if (new_header == nullptr)
        stx::panic();

      easy_handle->header = new_header;
    }

    ASH_CURLE_ENSURE(curl_easy_setopt(easy_handle->easy, CURLOPT_HTTPHEADER,
                                      easy_handle->header));

    ASH_CURLE_ENSURE(curl_easy_setopt(easy_handle->easy, CURLOPT_VERBOSE, 1L));

    curl_easy_setopt(easy_handle->easy, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(easy_handle->easy, CURLOPT_MAXREDIRS, request.maximumRedirects);

    return stx::Ok(std::move(easy_handle_rc));
  }

  static void begin_request(CURL *easy, CURLM *multi, TaskInfo *info_addr)
  {
    ASH_CURLE_ENSURE(curl_easy_setopt(easy, CURLOPT_WRITEDATA, info_addr));
    ASH_CURLE_ENSURE(curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION,
                                      impl::curl_content_write_function));
    ASH_CURLE_ENSURE(curl_easy_setopt(easy, CURLOPT_HEADERDATA, info_addr));
    ASH_CURLE_ENSURE(curl_easy_setopt(easy, CURLOPT_HEADERFUNCTION,
                                      impl::curl_header_write_function));
    ASH_CURLM_ENSURE(curl_multi_add_handle(multi, easy));
  }

  void retrieve_progress_info(CURL *easy, CURLINFO info, uint64_t &value)
  {
    curl_off_t curl_value;
    ASH_CURLE_ENSURE(curl_easy_getinfo(easy, info, &curl_value));
    value = static_cast<uint64_t>(curl_value);
  }

  void retrieve_optional_progress_info(CURL *easy, CURLINFO info, stx::Option<uint64_t> &value)
  {
    curl_off_t curl_value;
    ASH_CURLE_ENSURE(curl_easy_getinfo(easy, info, &curl_value));
    if (curl_value == -1)
    {
      value = stx::None;
    }
    else
    {
      value = stx::Some(static_cast<uint64_t>(curl_value));
    }
  }

  void update_progress()
  {
    Progress progress;
    CURL    *easyr = info.handle->easy.handle->easy;

    retrieve_progress_info(easyr, CURLINFO_SIZE_UPLOAD_T, progress.bytesSent);
    retrieve_progress_info(easyr, CURLINFO_SIZE_DOWNLOAD_T, progress.bytesReceived);
    retrieve_progress_info(easyr, CURLINFO_SPEED_UPLOAD_T, progress.uploadSpeed);
    retrieve_progress_info(easyr, CURLINFO_SPEED_DOWNLOAD_T, progress.downloadSpeed);
    retrieve_optional_progress_info(easyr, CURLINFO_CONTENT_LENGTH_UPLOAD_T, progress.contentUploadSize);
    retrieve_optional_progress_info(easyr, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, progress.contentDownloadSize);

    info.handle->updater.update(progress);
  }

  static stx::Result<std::tuple<Task, ProgressMonitor, stx::Future<HttpResponse>>,
                     stx::AllocError>
      launch(stx::Allocator allocator, HttpRequest const &request,
             stx::Rc<CurlMultiHandle *> const &parent)
  {
    TRY_OK(easy, prepare_request(allocator, parent, request));
    TRY_OK(updater, makeProgressMonitor(allocator));
    TRY_OK(promise, stx::make_promise<HttpResponse>(allocator));

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

  void finish(stx::Allocator allocator)
  {
    HttpResponse response;

    CURL *easy = info.handle->easy.handle->easy;

    // get status and more completion info
    char const *effective_url = nullptr;
    ASH_CURLE_ENSURE(
        curl_easy_getinfo(easy, CURLINFO_EFFECTIVE_URL, &effective_url));

    if (effective_url != nullptr)
    {
      response.effectiveUrl =
          stx::string::make(allocator, effective_url).unwrap();
    }

    curl_off_t total_time = 0;
    ASH_CURLE_ENSURE(
        curl_easy_getinfo(easy, CURLINFO_TOTAL_TIME_T, &total_time));
    response.totalTime = std::chrono::microseconds(total_time);

    curl_off_t total_downloaded = 0;
    curl_off_t total_uploaded   = 0;

    ASH_CURLE_ENSURE(curl_easy_getinfo(easy, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T,
                                       &total_downloaded));
    ASH_CURLE_ENSURE(curl_easy_getinfo(easy, CURLINFO_CONTENT_LENGTH_UPLOAD_T,
                                       &total_uploaded));

    response.downloaded = total_downloaded;
    response.uploaded   = total_uploaded;

    long response_code = 0;
    ASH_CURLE_ENSURE(
        curl_easy_getinfo(easy, CURLINFO_RESPONSE_CODE, &response_code));

    response.code = uint64_t{static_cast<uint64_t>(response_code)};

    response.header  = std::move(info.handle->header);
    response.content = std::move(info.handle->content);

    // notify of completion
    info.handle->promise.notify_completed(std::move(response));
  }
};
}        // namespace impl

struct Client
{
  STX_MAKE_PINNED(Client)

  Client(stx::Allocator allocator) :
      multi_{impl::make_curl_multi_handle(allocator).unwrap()},
      tasks_{allocator},
      lock_{},
      allocator_{allocator}
  {}

  // submittable to from multiple threads.
  // the submitting thread has to wait until the task queue is free for tasks to
  // be added on.
  //
  // tasks can be submitted from multiple threads
  std::tuple<stx::Future<HttpResponse>, ProgressMonitor> get(
      stx::String url, std::map<stx::String, stx::String> header = {},
      uint32_t max_redirects = 69)
  {
    stx::LockGuard guard{lock_};
    auto [task, monitor, future] =
        impl::Task::launch(allocator_,
                           HttpRequest{std::move(url), std::move(header), HttpMethod::Get,
                                       max_redirects},
                           multi_)
            .unwrap();

    tasks_.push(std::move(task)).unwrap();

    return std::make_tuple(std::move(future), std::move(monitor));
  }

  std::tuple<stx::Future<HttpResponse>, ProgressMonitor> head(
      stx::String url, std::map<stx::String, stx::String> header = {},
      uint32_t max_redirects = 69)
  {
    stx::LockGuard guard{lock_};
    auto [task, monitor, future] =
        impl::Task::launch(allocator_,
                           HttpRequest{std::move(url), std::move(header),
                                       HttpMethod::Head, max_redirects},
                           multi_)
            .unwrap();

    tasks_.push(std::move(task)).unwrap();

    return std::make_tuple(std::move(future), std::move(monitor));
  }

  void tick()
  {
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
      tasks_.erase(to_erase);
    }

    for (impl::Task &task : tasks_)
    {
      task.update_progress();

      if (task.info.handle->last_status_poll == stx::FutureStatus::Suspended &&
          task.info.handle->promise.fetch_suspend_request() ==
              stx::SuspendState::Executing)
      {
        ASH_CURLE_ENSURE(curl_easy_pause(task.info.handle->easy.handle->easy,
                                         CURLPAUSE_CONT));
        task.info.handle->promise.notify_executing();
      }
    }

    int num_running_handles = 0;
    ASH_CURLM_ENSURE(
        curl_multi_perform(multi_.handle->multi, &num_running_handles));

    int            num_messages_in_queue = 0;
    CURLMsg const *messages =
        curl_multi_info_read(multi_.handle->multi, &num_messages_in_queue);

    // CURL can return 0 along with a valid pointer which would mean
    // there's a single message on the queue
    if (num_messages_in_queue == 0 && messages != nullptr)
    {
      num_messages_in_queue = 1;
    }

    for (CURLMsg const &msg :
         stx::Span<CURLMsg const>(messages, num_messages_in_queue))
    {
      if (msg.msg == CURLMSG_DONE)
      {
        stx::Span task = tasks_.span().which(
            [easy = msg.easy_handle](impl::Task const &task) {
              return task.info.handle->easy.handle->easy == easy;
            });

        ASH_CHECK(!task.is_empty());

        task[0].finish(allocator_);
      }
    }
  }

  stx::Rc<impl::CurlMultiHandle *> multi_;
  stx::Vec<impl::Task>             tasks_;
  stx::SpinLock                    lock_;
  stx::Allocator                   allocator_;
};

}        // namespace http
}        // namespace ash