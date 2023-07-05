#include "ashura/subsystems/http_client.h"

namespace ash
{
namespace HttpClient
{

struct CurlMultiHandleImpl
{
  CURLM *multi;        // Pointer to the multi-handle

  CurlMultiHandleImpl(CURLM *init_multi) :
      multi{init_multi}
  {}

  ~CurlMultiHandleImpl()
  {
    ASH_CURLM_CHECK(curl_multi_cleanup(multi));
  }
};

CurlMultiHandle::CurlMultiHandle(CURLM *init_multi) :
    impl(nullptr)
{
  if (init_multi)
    impl = new CurlMultiHandleImpl(init_multi);
  else
    throw std::runtime_error("Invalid multi-handle provided.");
}

CurlMultiHandle::~CurlMultiHandle()
{
  delete impl;
}

// Implementation class definition
struct CurlEasyHandleImpl
{
  CURL                      *easy;
  curl_slist                *header;
  stx::Rc<CurlMultiHandle *> parent;

  CurlEasyHandleImpl(CURL *easy_easy, curl_slist *easy_header, stx::Rc<CurlMultiHandle *> easy_parent) :
      easy(easy_easy), header(easy_header), parent(std::move(easy_parent))
  {}

  ~CurlEasyHandleImpl()
  {
    ASH_CURLM_CHECK(curl_multi_remove_handle(parent.handle, easy));
    curl_easy_cleanup(easy);
    curl_slist_free_all(header);
  }
};

CurlEasyHandle::CurlEasyHandle(CURL *easy, curl_slist *header, stx::Rc<CurlMultiHandle *> parent) :
    impl(new CurlEasyHandleImpl(easy, header, std::move(parent)))
{
  if (!impl->easy)
    throw std::runtime_error("Failed to initialize CurlEasyHandle.");
}

inline size_t curl_header_write_function(u8 const *bytes, size_t unit_size,
                                         size_t nmemb, TaskInfo *task_info)
{
  size_t total_size = nmemb * unit_size;

  for (size_t i = 0; i < total_size; i++)
  {
    task_info->header.push_inplace(bytes[i]).unwrap();
  }

  return total_size;
}

inline size_t curl_content_write_function(u8 const *bytes, size_t unit_size,
                                          size_t nmemb, TaskInfo *task_info)
{
  size_t total_size = nmemb * unit_size;

  stx::Promise<HttpResponse> &promise = task_info->promise;
  stx::RequestProxy           request_proxy{promise};

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
    task_info->content.push(static_cast<u8>(bytes[i])).unwrap();
  }

  return total_size;
}

stx::Result<stx::Rc<CurlEasyHandle *>, stx::AllocError> Task::prepare_request(
    stx::Allocator allocator, stx::Rc<CurlMultiHandle *> const &parent, HttpRequest const &request)
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
      ASH_CURLE_CHECK(curl_easy_setopt(easy_handle->impl->easy, CURLOPT_NOBODY, 1L));
      break;
  }
  stx::String const                        &url    = request.url;
  std::map<stx::String, stx::String> const &header = request.headers;

  ASH_CURLE_CHECK(curl_easy_setopt(easy_handle->impl->easy, CURLOPT_URL, url.c_str()));

  for (const auto &[key, value] : header)
  {
    stx::String joined = stx::string::join(allocator, "", key, ":", value).unwrap();

    curl_slist *new_header = curl_slist_append(easy_handle->impl->header, joined.c_str());
    ASH_CHECK(new_header);

    easy_handle->impl->header = new_header;
  }

  ASH_CURLE_CHECK(curl_easy_setopt(easy_handle->impl->easy, CURLOPT_HTTPHEADER,
                                   easy_handle->impl->header));

  ASH_CURLE_CHECK(curl_easy_setopt(easy_handle->impl->easy, CURLOPT_VERBOSE, 1L));

  curl_easy_setopt(easy_handle->impl->easy, CURLOPT_FOLLOWLOCATION, 1);
  curl_easy_setopt(easy_handle->impl->easy, CURLOPT_MAXREDIRS, request.maximumRedirects);

  return stx::Ok(std::move(easy_handle_rc));
}

void Task::begin_request(CURL *easy, CURLM *multi, TaskInfo *info_addr)
{
  ASH_CURLE_CHECK(curl_easy_setopt(easy, CURLOPT_WRITEDATA, info_addr));
  ASH_CURLE_CHECK(curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION,
                                   curl_content_write_function));
  ASH_CURLE_CHECK(curl_easy_setopt(easy, CURLOPT_HEADERDATA, info_addr));
  ASH_CURLE_CHECK(curl_easy_setopt(easy, CURLOPT_HEADERFUNCTION,
                                   curl_header_write_function));
  ASH_CURLM_CHECK(curl_multi_add_handle(multi, easy));
}

void Task::retrieve_progress_info(CURL *easy, CURLINFO info, u64 &value)
{
  curl_off_t curl_value;
  AS_U64(curl_easy_getinfo(easy, info, &curl_value));
  value = static_cast<u64>(curl_value);
}

void Task::retrieve_optional_progress_info(CURL *easy, CURLINFO info, stx::Option<u64> &value)
{
  curl_off_t curl_value;
  ASH_CURLE_CHECK(curl_easy_getinfo(easy, info, &curl_value));
  if (curl_value == -1)
  {
    value = stx::None;
  }
  else
  {
    value = stx::Some(AS_U64(curl_value));
  }
}

void Task::update_progress()
{
  Progress progress;
  CURL    *easyr = info.handle->easy.handle->impl->easy;

  retrieve_progress_info(easyr, CURLINFO_SIZE_UPLOAD_T, progress.bytesSent);
  retrieve_progress_info(easyr, CURLINFO_SIZE_DOWNLOAD_T, progress.bytesReceived);
  retrieve_progress_info(easyr, CURLINFO_SPEED_UPLOAD_T, progress.uploadSpeed);
  retrieve_progress_info(easyr, CURLINFO_SPEED_DOWNLOAD_T, progress.downloadSpeed);
  retrieve_optional_progress_info(easyr, CURLINFO_CONTENT_LENGTH_UPLOAD_T, progress.contentUploadSize);
  retrieve_optional_progress_info(easyr, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, progress.contentDownloadSize);

  info.handle->updater.update(progress);
}

stx::Result<std::tuple<Task, ProgressMonitor, stx::Future<HttpResponse>>,
            stx::AllocError>
    Task::launch(stx::Allocator allocator, HttpRequest const &request,
                 stx::Rc<CurlMultiHandle *> const &parent)
{
  TRY_OK(easy, prepare_request(allocator, parent, request));
  TRY_OK(updater, makeProgressMonitor(allocator));
  TRY_OK(promise, stx::make_promise<HttpResponse>(allocator));

  auto future = promise.get_future();

  TRY_OK(task_info,
         stx::rc::make_unique_inplace<TaskInfo>(
             allocator, std::move(easy), stx::Vec<u8>{allocator},
             stx::Vec<u8>{allocator}, std::move(promise),
             std::move(updater.second)));

  begin_request(task_info.handle->easy.handle->impl->easy,
                task_info.handle->easy.handle->impl->parent.handle->impl->multi,
                task_info.handle);

  return stx::Ok(std::make_tuple(Task{std::move(task_info)},
                                 std::move(updater.first),
                                 std::move(future)));
}

void Task::finish(stx::Allocator allocator)
{
  HttpResponse response;

  CURL *easy = info.handle->easy.handle->impl->easy;

  // get status and more completion info
  char const *effective_url = nullptr;
  ASH_CURLE_CHECK(
      curl_easy_getinfo(easy, CURLINFO_EFFECTIVE_URL, &effective_url));

  if (effective_url != nullptr)
  {
    response.effectiveUrl =
        stx::string::make(allocator, effective_url).unwrap();
  }

  curl_off_t total_time = 0;
  ASH_CURLE_CHECK(
      curl_easy_getinfo(easy, CURLINFO_TOTAL_TIME_T, &total_time));
  response.totalTime = std::chrono::microseconds(total_time);

  curl_off_t total_downloaded = 0;
  curl_off_t total_uploaded   = 0;

  ASH_CURLE_CHECK(curl_easy_getinfo(easy, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T,
                                    &total_downloaded));
  ASH_CURLE_CHECK(curl_easy_getinfo(easy, CURLINFO_CONTENT_LENGTH_UPLOAD_T,
                                    &total_uploaded));

  response.downloaded = total_downloaded;
  response.uploaded   = total_uploaded;

  long response_code = 0;
  ASH_CURLE_CHECK(
      curl_easy_getinfo(easy, CURLINFO_RESPONSE_CODE, &response_code));

  response.code = u64{static_cast<u64>(response_code)};

  response.header  = std::move(info.handle->header);
  response.content = std::move(info.handle->content);

  // notify of completion
  info.handle->promise.notify_completed(std::move(response));
}

void Client::tick()
{
  stx::LockGuard guard{lock_};

  {
    // poll statuses
    tasks_.span().for_each([](Task &task) {
      task.info.handle->last_status_poll =
          task.info.handle->promise.fetch_status();
    });

    auto to_erase = tasks_.span()
                        .partition([](Task const &task) {
                          return task.info.handle->last_status_poll !=
                                     stx::FutureStatus::Canceled ||
                                 task.info.handle->last_status_poll ==
                                     stx::FutureStatus::Completed;
                        })
                        .second;
    tasks_.erase(to_erase);
  }

  for (Task &task : tasks_)
  {
    task.update_progress();

    if (task.info.handle->last_status_poll == stx::FutureStatus::Suspended &&
        task.info.handle->promise.fetch_suspend_request() ==
            stx::SuspendState::Executing)
    {
      ASH_CURLE_CHECK(curl_easy_pause(task.info.handle->easy.handle->impl->easy,
                                      CURLPAUSE_CONT));
      task.info.handle->promise.notify_executing();
    }
  }

  int num_running_handles = 0;
  ASH_CURLM_CHECK(
      curl_multi_perform(multi_.handle->impl->multi, &num_running_handles));

  int            num_messages_in_queue = 0;
  CURLMsg const *messages =
      curl_multi_info_read(multi_.handle->impl->multi, &num_messages_in_queue);

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
          [easy = msg.easy_handle](Task const &task) {
            return task.info.handle->easy.handle->impl->easy == easy;
          });

      ASH_CHECK(!task.is_empty());

      task[0].finish(allocator_);
    }
  }
}

}        // namespace HttpClient
}        // namespace ash
