#include "vlk/http_client.h"

namespace vlk {
namespace http {

size_t curl_content_write_function(uint8_t const *bytes, size_t unit_size,
                                   size_t nmemb, RunningTaskInfo *task_info) {
  auto total_size = nmemb * unit_size;

  auto &promise = task_info->packaged_task.promise;
  auto &request_proxy = task_info->request_proxy;
  auto cancel_request = request_proxy.fetch_cancel_request();
  // we handle only cancelation in here

  if (cancel_request.state == stx::RequestedCancelState::Canceled) {
    if (cancel_request.source == stx::RequestSource::Executor) {
      promise.notify_force_cancel_begin();
      task_info->cancel_state = CancelState::ExecutorCanceled;
      // signals to libcurl that we want to cancel the request
      return 0;
    } else {
      promise.notify_user_cancel_begin();
      task_info->cancel_state = CancelState::UserCanceled;
      // signals to libcurl that we want to cancel the request
      return 0;
    }
    // notify force cancel end
  }

  task_info->response.content.insert(task_info->response.content.end(), bytes,
                                     bytes + total_size);

  return total_size;
}

size_t curl_header_write_function(uint8_t const *bytes, size_t unit_size,
                                  size_t nmemb, RunningTaskInfo *task_info) {
  auto total_size = nmemb * unit_size;
  task_info->response.header.insert(task_info->response.header.end(), bytes,
                                    bytes + total_size);
  return total_size;
}

}  // namespace http
}  // namespace vlk
