#include "ashura/subsystems/http_client.h"
#include "ashura/utils.h"
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

namespace ash
{

struct HttpCurlMultiHandleImpl
{
  CURLM *multi;        // Pointer to the multi-handle

  HttpCurlMultiHandleImpl(CURLM *init_multi) : multi{init_multi}
  {
  }

  ~HttpCurlMultiHandleImpl()
  {
    ASH_CURLM_CHECK(curl_multi_cleanup(multi));
  }
};

HttpCurlMultiHandle::HttpCurlMultiHandle(CURLM *init_multi) : impl(nullptr)
{
  if (init_multi)
    impl = new HttpCurlMultiHandleImpl(init_multi);
  else
    stx::panic("Invalid multi-handle provided.");
}

HttpCurlMultiHandle::~HttpCurlMultiHandle()
{
  delete impl;
}

// Implementation class definition
struct HttpCurlEasyHandleImpl
{
  CURL                          *easy;
  curl_slist                    *header;
  stx::Rc<HttpCurlMultiHandle *> parent;

  HttpCurlEasyHandleImpl(CURL *easy_easy, curl_slist *easy_header,
                         stx::Rc<HttpCurlMultiHandle *> easy_parent) :
      easy(easy_easy), header(easy_header), parent(std::move(easy_parent))
  {
  }

  ~HttpCurlEasyHandleImpl()
  {
    ASH_CURLM_CHECK(curl_multi_remove_handle(parent.handle, easy));
    curl_easy_cleanup(easy);
    curl_slist_free_all(header);
  }
};

HttpCurlEasyHandle::HttpCurlEasyHandle(CURL *easy, curl_slist *header,
                                       stx::Rc<HttpCurlMultiHandle *> parent) :
    impl(new HttpCurlEasyHandleImpl(easy, header, std::move(parent)))
{
  if (!impl->easy)
    stx::panic("Failed to initialize HttpCurlEasyHandle");
}

inline size_t curl_header_write_function(u8 const *bytes, size_t unit_size,
                                         size_t nmemb, HttpTaskInfo *task_info)
{
  size_t total_size = nmemb * unit_size;

  for (size_t i = 0; i < total_size; i++)
  {
    task_info->header.push_inplace(bytes[i]).unwrap();
  }

  return total_size;
}

inline size_t curl_content_write_function(u8 const *bytes, size_t unit_size,
                                          size_t nmemb, HttpTaskInfo *task_info)
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

stx::Result<stx::Rc<HttpCurlEasyHandle *>, stx::AllocError>
    HttpTask::prepare_request(stx::Allocator                        allocator,
                              stx::Rc<HttpCurlMultiHandle *> const &parent,
                              HttpRequest const                    &request)
{
  CURL *easy = curl_easy_init();
  if (easy == nullptr)
    stx::panic("unexpected error from CURL");
  TRY_OK(easy_handle_rc, stx::rc::make_inplace<HttpCurlEasyHandle>(
                             allocator, easy, nullptr, parent.share()));

  HttpCurlEasyHandle *easy_handle = easy_handle_rc.handle;

  HttpMethod verb = request.method;

  switch (verb)
  {
    case HttpMethod::Get:
      break;

    case HttpMethod::Head:
      ASH_CURLE_CHECK(
          curl_easy_setopt(easy_handle->impl->easy, CURLOPT_NOBODY, 1L));
      break;
  }
  stx::String const                        &url    = request.url;
  std::map<stx::String, stx::String> const &header = request.headers;

  ASH_CURLE_CHECK(
      curl_easy_setopt(easy_handle->impl->easy, CURLOPT_URL, url.c_str()));

  for (const auto &[key, value] : header)
  {
    stx::String joined =
        stx::string::join(allocator, "", key, ":", value).unwrap();

    curl_slist *new_header =
        curl_slist_append(easy_handle->impl->header, joined.c_str());
    ASH_CHECK(new_header);

    easy_handle->impl->header = new_header;
  }

  ASH_CURLE_CHECK(curl_easy_setopt(easy_handle->impl->easy, CURLOPT_HTTPHEADER,
                                   easy_handle->impl->header));

  ASH_CURLE_CHECK(
      curl_easy_setopt(easy_handle->impl->easy, CURLOPT_VERBOSE, 1L));

  curl_easy_setopt(easy_handle->impl->easy, CURLOPT_FOLLOWLOCATION, 1);
  curl_easy_setopt(easy_handle->impl->easy, CURLOPT_MAXREDIRS,
                   request.maximumRedirects);

  return stx::Ok(std::move(easy_handle_rc));
}

void HttpTask::begin_request(CURL *easy, CURLM *multi, HttpTaskInfo *info_addr)
{
  ASH_CURLE_CHECK(curl_easy_setopt(easy, CURLOPT_WRITEDATA, info_addr));
  ASH_CURLE_CHECK(curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION,
                                   curl_content_write_function));
  ASH_CURLE_CHECK(curl_easy_setopt(easy, CURLOPT_HEADERDATA, info_addr));
  ASH_CURLE_CHECK(curl_easy_setopt(easy, CURLOPT_HEADERFUNCTION,
                                   curl_header_write_function));
  ASH_CURLM_CHECK(curl_multi_add_handle(multi, easy));
}

void HttpTask::retrieve_progress_info(CURL *easy, CURLINFO info, u64 &value)
{
  curl_off_t curl_value;
  ASH_CURLE_CHECK(curl_easy_getinfo(easy, info, &curl_value));
  value = static_cast<u64>(curl_value);
}

void HttpTask::retrieve_optional_progress_info(CURL *easy, CURLINFO info,
                                               stx::Option<u64> &value)
{
  curl_off_t curl_value;
  ASH_CURLE_CHECK(curl_easy_getinfo(easy, info, &curl_value));
  if (curl_value == -1)
  {
    value = stx::None;
  }
  else
  {
    value = stx::Some(static_cast<u64>(curl_value));
  }
}

void HttpTask::update_progress()
{
  HttpProgress progress;
  CURL        *easyr = info.handle->easy.handle->impl->easy;

  retrieve_progress_info(easyr, CURLINFO_SIZE_UPLOAD_T, progress.bytesSent);
  retrieve_progress_info(easyr, CURLINFO_SIZE_DOWNLOAD_T,
                         progress.bytesReceived);
  retrieve_progress_info(easyr, CURLINFO_SPEED_UPLOAD_T, progress.uploadSpeed);
  retrieve_progress_info(easyr, CURLINFO_SPEED_DOWNLOAD_T,
                         progress.downloadSpeed);
  retrieve_optional_progress_info(easyr, CURLINFO_CONTENT_LENGTH_UPLOAD_T,
                                  progress.contentUploadSize);
  retrieve_optional_progress_info(easyr, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T,
                                  progress.contentDownloadSize);

  info.handle->updater.update(progress);
}

stx::Result<
    std::tuple<HttpTask, HttpProgressMonitor, stx::Future<HttpResponse>>,
    stx::AllocError>
    HttpTask::launch(stx::Allocator allocator, HttpRequest const &request,
                     stx::Rc<HttpCurlMultiHandle *> const &parent)
{
  TRY_OK(easy, prepare_request(allocator, parent, request));
  TRY_OK(updater, makeProgressMonitor(allocator));
  TRY_OK(promise, stx::make_promise<HttpResponse>(allocator));

  auto future = promise.get_future();

  TRY_OK(task_info, stx::rc::make_unique_inplace<HttpTaskInfo>(
                        allocator, std::move(easy), stx::Vec<u8>{allocator},
                        stx::Vec<u8>{allocator}, std::move(promise),
                        std::move(updater.second)));

  begin_request(task_info.handle->easy.handle->impl->easy,
                task_info.handle->easy.handle->impl->parent.handle->impl->multi,
                task_info.handle);

  return stx::Ok(std::make_tuple(HttpTask{std::move(task_info)},
                                 std::move(updater.first), std::move(future)));
}

void HttpTask::finish(stx::Allocator allocator)
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
  ASH_CURLE_CHECK(curl_easy_getinfo(easy, CURLINFO_TOTAL_TIME_T, &total_time));
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

void HttpClient::tick(Context &ctx, std::chrono::nanoseconds interval)
{
  stx::LockGuard guard{lock_};

  {
    // poll statuses
    tasks_.span().for_each([](HttpTask &task) {
      task.info.handle->last_status_poll =
          task.info.handle->promise.fetch_status();
    });

    auto to_erase = tasks_.span()
                        .partition([](HttpTask const &task) {
                          return task.info.handle->last_status_poll !=
                                     stx::FutureStatus::Canceled ||
                                 task.info.handle->last_status_poll ==
                                     stx::FutureStatus::Completed;
                        })
                        .second;
    tasks_.erase(to_erase);
  }

  for (HttpTask &task : tasks_)
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
      stx::Span task =
          tasks_.span().which([easy = msg.easy_handle](HttpTask const &task) {
            return task.info.handle->easy.handle->impl->easy == easy;
          });

      ASH_CHECK(!task.is_empty());

      task[0].finish(allocator_);
    }
  }
}

}        // namespace ash
