#pragma once

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
inline std::string operator>>(ReportQuery, CURLcode code)
{
  return fmt::format("CURLcode{}", static_cast<int>(code));
}

// Overloading the operator>> to convert CURLMcode to a string representation
inline std::string operator>>(ReportQuery, CURLMcode code)
{
  return fmt::format("CURLMcode{}", static_cast<int>(code));
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
  String                        url = "https://fast.com";
  std::map<String, String> headers;
  HttpMethod                         method           = HttpMethod::Get;
  u32                                maximumRedirects = CURLOPT_MAXREDIRS;
};

// Struct to represent an HTTP response
struct HttpResponse
{
  u64                      code = 0;
  Vec<u8>             header;
  Vec<u8>             content;
  std::chrono::nanoseconds totalTime{0};
  String              effectiveUrl;
  u64                      uploaded{0};
  u64                      downloaded{0};
};

struct HttpProgress
{
  u64              bytesSent     = 0;
  u64              bytesReceived = 0;
  u64              uploadSpeed   = 0;
  u64              downloadSpeed = 0;
  Option<u64> contentUploadSize;
  Option<u64> contentDownloadSize;
};

struct HttpProgressMonitorState
{
  STX_DEFAULT_CONSTRUCTOR(HttpProgressMonitorState)
  STX_MAKE_PINNED(HttpProgressMonitorState)

  HttpProgress  progress;
  SpinLock lock;

  HttpProgress load()
  {
    LockGuard guard{lock};
    return progress;
  }

  void update(const HttpProgress &progress_)
  {
    LockGuard guard{lock};
    progress = progress_;
  }
};

struct HttpProgressMonitor
{
  HttpProgress getProgress() const
  {
    return state->load();
  }
  Rc<HttpProgressMonitorState *> state;
};

struct HttpProgressUpdater
{
  void update(const HttpProgress &progress) const
  {
    state->update(progress);
  }

  Rc<HttpProgressMonitorState *> state;
};

inline Result<std::pair<HttpProgressMonitor, HttpProgressUpdater>,
                   AllocError>
    makeProgressMonitor(Allocator allocator)
{
  TRY_OK(state, rc::make_inplace<HttpProgressMonitorState>(allocator));

  HttpProgressMonitor progressMonitor{state.share()};

  return Ok(std::make_pair(std::move(progressMonitor),
                                HttpProgressUpdater{std::move(state)}));
}

struct HttpCurlMultiHandleImpl;

struct HttpCurlMultiHandle
{
  STX_DISABLE_DEFAULT_CONSTRUCTOR(HttpCurlMultiHandle)
  STX_DISABLE_COPY(HttpCurlMultiHandle)
  STX_DISABLE_MOVE(HttpCurlMultiHandle)

  explicit HttpCurlMultiHandle(
      CURLM *init_multi);        // Declaration of constructor
  ~HttpCurlMultiHandle();        // Declaration of destructor

  friend struct HttpTask;
  friend struct HttpClient;

private:
  HttpCurlMultiHandleImpl *impl;        // Pointer to implementation
};

inline auto make_curl_multi_handle(Allocator allocator)
{
  CURLM *multi = curl_multi_init();
  if (multi == nullptr)
  {
    panic(
        "unexpected error from curl");        // Panic if initialization fails
  }
  return rc::make_inplace<HttpCurlMultiHandle>(
      allocator, multi);        // Create and return the HttpCurlMultiHandle
}

struct HttpCurlEasyHandleImpl;

struct HttpCurlEasyHandle
{
  STX_DISABLE_DEFAULT_CONSTRUCTOR(HttpCurlEasyHandle)
  STX_DISABLE_COPY(HttpCurlEasyHandle)
  STX_DISABLE_MOVE(HttpCurlEasyHandle)

  HttpCurlEasyHandle(CURL *easy_easy, curl_slist *easy_header,
                     Rc<HttpCurlMultiHandle *> easy_parent);

  friend struct HttpTask;
  friend struct HttpClient;

private:
  HttpCurlEasyHandleImpl *impl;        // Pointer to implementation class
};

struct HttpTaskInfo
{
  Rc<HttpCurlEasyHandle *> easy;
  Vec<u8>                  header;
  Vec<u8>                  content;
  Promise<HttpResponse>    promise;
  HttpProgressUpdater           updater;
  FutureStatus             last_status_poll = FutureStatus::Executing;
};

struct HttpTaskImpl;

struct HttpTask
{
  Unique<HttpTaskInfo *> info;

  static Result<Rc<HttpCurlEasyHandle *>, AllocError>
      prepare_request(Allocator                        allocator,
                      Rc<HttpCurlMultiHandle *> const &parent,
                      HttpRequest const                    &request);

  static void begin_request(CURL *easy, CURLM *multi, HttpTaskInfo *info_addr);

  void retrieve_progress_info(CURL *easy, CURLINFO info, u64 &value);

  void retrieve_optional_progress_info(CURL *easy, CURLINFO info,
                                       Option<u64> &value);

  void update_progress();

  static Result<
      std::tuple<HttpTask, HttpProgressMonitor, Future<HttpResponse>>,
      AllocError>
      launch(Allocator allocator, HttpRequest const &request,
             Rc<HttpCurlMultiHandle *> const &parent);

  void finish(Allocator allocator);
};

struct HttpClient : public Subsystem
{
  STX_MAKE_PINNED(HttpClient)

  explicit HttpClient(Allocator allocator) :
      multi_{make_curl_multi_handle(allocator).unwrap()},
      tasks_{allocator},
      lock_{},
      allocator_{allocator}
  {
  }

  std::tuple<Future<HttpResponse>, HttpProgressMonitor>
      get(String url, std::map<String, String> header = {},
          u32 max_redirects = 69)
  {
    LockGuard guard{lock_};
    auto [task, monitor, future] =
        HttpTask::launch(allocator_,
                         HttpRequest{std::move(url), std::move(header),
                                     HttpMethod::Get, max_redirects},
                         multi_)
            .unwrap();

    tasks_.push(std::move(task)).unwrap();

    return std::make_tuple(std::move(future), std::move(monitor));
  }

  std::tuple<Future<HttpResponse>, HttpProgressMonitor>
      head(String url, std::map<String, String> header = {},
           u32 max_redirects = 69)
  {
    LockGuard guard{lock_};
    auto [task, monitor, future] =
        HttpTask::launch(allocator_,
                         HttpRequest{std::move(url), std::move(header),
                                     HttpMethod::Head, max_redirects},
                         multi_)
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

  Rc<HttpCurlMultiHandle *> multi_;
  Vec<HttpTask>             tasks_;
  SpinLock                  lock_;
  Allocator                 allocator_;
};

}        // namespace ash