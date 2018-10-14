#include <iostream>

#define MK_CURLX_INLINE_IMPL
#include "libcurlx.h"

static void summary(const mk_curlx_response_uptr &res) {
  std::clog << "=== BEGIN SUMMARY ==="
            << std::endl
            << "CURL error code: "
            << mk_curlx_response_get_error(res.get())
            << std::endl
            << "HTTP status code: "
            << mk_curlx_response_get_status_code(res.get())
            << std::endl
            << "Bytes sent: "
            << mk_curlx_response_get_bytes_sent(res.get())
            << std::endl
            << "Bytes recv: "
            << mk_curlx_response_get_bytes_recv(res.get())
            << std::endl
            << "Redirect URL: "
            << mk_curlx_response_get_redirect_url(res.get())
            << std::endl
            << "=== END SUMMARY ==="
            << std::endl
            << std::endl;
  std::clog << "=== BEGIN REQUEST HEADERS ==="
            << std::endl
            << mk_curlx_response_get_request_headers(res.get())
            << "=== END REQUEST HEADERS ==="
            << std::endl
            << std::endl;
  std::clog << "=== BEGIN RESPONSE HEADERS ==="
            << std::endl
            << mk_curlx_response_get_response_headers(res.get())
            << "=== END RESPONSE HEADERS ==="
            << std::endl
            << std::endl;
  std::clog << "=== BEGIN CERTIFICATE CHAIN ==="
            << std::endl
            << mk_curlx_response_get_certificate_chain(res.get())
            << "=== END CERTIFICATE CHAIN ==="
            << std::endl
            << std::endl;
  std::clog << "=== BEGIN LOGS ==="
            << std::endl
            << mk_curlx_response_get_logs(res.get())
            << "=== END LOGS ==="
            << std::endl
            << std::endl;
  std::clog << "=== BEGIN BODY ==="
            << std::endl
            << mk_curlx_response_get_body(res.get())
            << "=== END BODY ==="
            << std::endl
            << std::endl;
}
