#pragma once

#include <chrono>
#include <map>

#include "ashura/primitives.h"
#include "ashura/subsystem.h"
#include "ashura/utils.h"
#include "fmt/format.h"
#include "stx/async.h"
#include "stx/string.h"
#include "stx/vec.h"

#ifndef NOMINMAX
#  define NOMINMAX
#  include "curl/curl.h"
#  undef NOMINMAX
#else
#  include "curl/curl.h"
#endif

#define ASH_CURLE_CHECK(code_expression, ...) \
  do                                          \
  {                                           \
    if ((code_expression) > 0)                \
    {                                         \
      ASH_PANIC(__VA_ARGS__);                 \
    }                                         \
  } while (false)

#define ASH_CURLM_CHECK(code_expression, ...) \
  do                                          \
  {                                           \
    if ((code_expression) > 0)                \
    {                                         \
      ASH_PANIC(__VA_ARGS__);                 \
    }                                         \
  } while (false)

namespace ash
{

// Overloading the operator>> to convert CURLcode to a string representation
inline std::string operator>>(stx::ReportQuery, CURLcode code)
{
  return fmt::format("CURLcode{}", static_cast<int>(code));
}

// Overloading the operator>> to convert CURLMcode to a string representation
inline std::string operator>>(stx::ReportQuery, CURLMcode code)
{
  return fmt::format("CURLMcode{}", AS(int, code));
}

// Enum to represent HTTP request methods
enum class HttpMethod : u8
{
  Get,
  Head
};

// Struct to represent an HTTP request
struct HttpRequest
{
  stx::String                        url = "https://fast.com";
  std::map<stx::String, stx::String> headers;
  HttpMethod                         method           = HttpMethod::Get;
  u32                                maximumRedirects = CURLOPT_MAXREDIRS;
};

// Struct to represent an HTTP response
struct HttpResponse
{
  u64                      code = 0;
  stx::Vec<u8>             header;
  stx::Vec<u8>             content;
  std::chrono::nanoseconds totalTime{0};
  stx::String              effectiveUrl;
  u64                      uploaded{0};
  u64                      downloaded{0};
};

struct HttpProgress
{
  u64              bytesSent     = 0;
  u64              bytesReceived = 0;
  u64              uploadSpeed   = 0;
  u64              downloadSpeed = 0;
  stx::Option<u64> contentUploadSize;
  stx::Option<u64> contentDownloadSize;
};

struct HttpProgressMonitorState
{
  STX_DEFAULT_CONSTRUCTOR(HttpProgressMonitorState)
  STX_MAKE_PINNED(HttpProgressMonitorState)

  HttpProgress  progress;
  stx::SpinLock lock;

  HttpProgress load()
  {
    stx::LockGuard guard{lock};
    return progress;
  }

  void update(const HttpProgress &progress_)
  {
    stx::LockGuard guard{lock};
    progress = progress_;
  }
};

struct HttpProgressMonitor
{
  HttpProgress getProgress() const
  {
    return state->load();
  }
  stx::Rc<HttpProgressMonitorState *> state;
};

struct HttpProgressUpdater
{
  void update(const HttpProgress &progress) const
  {
    state->update(progress);
  }

  stx::Rc<HttpProgressMonitorState *> state;
};

inline stx::Result<std::pair<HttpProgressMonitor, HttpProgressUpdater>, stx::AllocError>
    makeProgressMonitor(stx::Allocator allocator)
{
  TRY_OK(state, stx::rc::make_inplace<HttpProgressMonitorState>(allocator));

  HttpProgressMonitor progressMonitor{state.share()};

  return stx::Ok(std::make_pair(std::move(progressMonitor), HttpProgressUpdater{std::move(state)}));
}

struct HttpCurlMultiHandleImpl;

struct HttpCurlMultiHandle
{
  STX_DISABLE_DEFAULT_CONSTRUCTOR(HttpCurlMultiHandle)
  STX_DISABLE_COPY(HttpCurlMultiHandle)
  STX_DISABLE_MOVE(HttpCurlMultiHandle)

  explicit HttpCurlMultiHandle(CURLM *init_multi);        // Declaration of constructor
  ~HttpCurlMultiHandle();                                 // Declaration of destructor

  friend struct HttpTask;
  friend struct HttpClient;

private:
  HttpCurlMultiHandleImpl *impl;        // Pointer to implementation
};

inline auto make_curl_multi_handle(stx::Allocator allocator)
{
  CURLM *multi = curl_multi_init();
  if (multi == nullptr)
  {
    stx::panic("unexpected error from curl");        // Panic if initialization fails
  }
  return stx::rc::make_inplace<HttpCurlMultiHandle>(
      allocator, multi);        // Create and return the HttpCurlMultiHandle
}

struct HttpCurlEasyHandleImpl;

struct HttpCurlEasyHandle
{
  STX_DISABLE_DEFAULT_CONSTRUCTOR(HttpCurlEasyHandle)
  STX_DISABLE_COPY(HttpCurlEasyHandle)
  STX_DISABLE_MOVE(HttpCurlEasyHandle)

  HttpCurlEasyHandle(CURL *easy_easy, curl_slist *easy_header,
                     stx::Rc<HttpCurlMultiHandle *> easy_parent);

  friend struct HttpTask;
  friend struct HttpClient;

private:
  HttpCurlEasyHandleImpl *impl;        // Pointer to implementation class
};

struct HttpTaskInfo
{
  stx::Rc<HttpCurlEasyHandle *> easy;
  stx::Vec<u8>                  header;
  stx::Vec<u8>                  content;
  stx::Promise<HttpResponse>    promise;
  HttpProgressUpdater           updater;
  stx::FutureStatus             last_status_poll = stx::FutureStatus::Executing;
};

struct HttpTaskImpl;

struct HttpTask
{
  stx::Unique<HttpTaskInfo *> info;

  static stx::Result<stx::Rc<HttpCurlEasyHandle *>, stx::AllocError>
      prepare_request(stx::Allocator allocator, stx::Rc<HttpCurlMultiHandle *> const &parent,
                      HttpRequest const &request);

  static void begin_request(CURL *easy, CURLM *multi, HttpTaskInfo *info_addr);

  void retrieve_progress_info(CURL *easy, CURLINFO info, u64 &value);

  void retrieve_optional_progress_info(CURL *easy, CURLINFO info, stx::Option<u64> &value);

  void update_progress();

  static stx::Result<std::tuple<HttpTask, HttpProgressMonitor, stx::Future<HttpResponse>>,
                     stx::AllocError>
      launch(stx::Allocator allocator, HttpRequest const &request,
             stx::Rc<HttpCurlMultiHandle *> const &parent);

  void finish(stx::Allocator allocator);
};

struct HttpClient : public Subsystem
{
  STX_MAKE_PINNED(HttpClient)

  explicit HttpClient(stx::Allocator allocator) :
      multi_{make_curl_multi_handle(allocator).unwrap()},
      tasks_{allocator},
      lock_{},
      allocator_{allocator}
  {
  }

  std::tuple<stx::Future<HttpResponse>, HttpProgressMonitor>
      get(stx::String url, std::map<stx::String, stx::String> header = {}, u32 max_redirects = 69)
  {
    stx::LockGuard guard{lock_};
    auto [task, monitor, future] =
        HttpTask::launch(
            allocator_,
            HttpRequest{std::move(url), std::move(header), HttpMethod::Get, max_redirects}, multi_)
            .unwrap();

    tasks_.push(std::move(task)).unwrap();

    return std::make_tuple(std::move(future), std::move(monitor));
  }

  std::tuple<stx::Future<HttpResponse>, HttpProgressMonitor>
      head(stx::String url, std::map<stx::String, stx::String> header = {}, u32 max_redirects = 69)
  {
    stx::LockGuard guard{lock_};
    auto [task, monitor, future] =
        HttpTask::launch(
            allocator_,
            HttpRequest{std::move(url), std::move(header), HttpMethod::Head, max_redirects}, multi_)
            .unwrap();

    tasks_.push(std::move(task)).unwrap();

    return std::make_tuple(std::move(future), std::move(monitor));
  }

  virtual constexpr void on_startup(Context &ctx) override
  {
  }

  virtual void tick(Context &ctx, std::chrono::nanoseconds interval) override;

  virtual std::string_view get_name() override
  {
    return "HttpClientSubsystem";
  }

  virtual constexpr void on_exit(Context &ctx) override
  {
  }

  stx::Rc<HttpCurlMultiHandle *> multi_;
  stx::Vec<HttpTask>             tasks_;
  stx::SpinLock                  lock_;
  stx::Allocator                 allocator_;
};

}        // namespace ash