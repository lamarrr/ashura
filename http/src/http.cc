#include "vlk/http.h"

namespace vlk {
namespace http {

size_t impl::curl_content_write_function(uint8_t const *bytes, size_t unit_size,
                                         size_t nmemb, TaskInfo *task_info) {
  size_t total_size = nmemb * unit_size;

  auto &promise = task_info->promise;
  stx::RequestProxy request_proxy{promise};

  auto cancel_request = request_proxy.fetch_cancel_request();
  auto suspend_request = request_proxy.fetch_suspend_request();

  if (cancel_request == stx::CancelState::Canceled) {
    promise.notify_canceled();
    return 0;
  }

  if (suspend_request == stx::SuspendState::Suspended) {
    promise.notify_suspended();
    return CURL_WRITEFUNC_PAUSE;
  }

  for (size_t i = 0; i < total_size; i++) {
    task_info->content = stx::vec::push(std::move(task_info->content),
                                        static_cast<uint8_t>(bytes[i]))
                             .unwrap();
  }

  return total_size;
}

size_t impl::curl_header_write_function(uint8_t const *bytes, size_t unit_size,
                                        size_t nmemb, TaskInfo *task_info) {
  size_t total_size = nmemb * unit_size;

  for (size_t i = 0; i < total_size; i++) {
    task_info->header = stx::vec::push(std::move(task_info->header),
                                       static_cast<uint8_t>(bytes[i]))
                            .unwrap();
  }

  return total_size;
}

}  // namespace http
}  // namespace vlk
