#include <stdlib.h>

#include <iostream>

#define MK_CURLX_INLINE_IMPL
#include "libcurlx.h"

int main(int argc, char **argv) {
  if (argc != 2) {
    std::clog << "Usage: ./libcurlx-get url" << std::endl;
    exit(EXIT_FAILURE);
  }
  mk_curlx_request_uptr req{mk_curlx_request_new()};
  if (!req) {
    std::clog << "Out of memory" << std::endl;
    exit(EXIT_FAILURE);
  }
  mk_curlx_request_enable_http2(req.get());
  mk_curlx_request_set_url(req.get(), argv[1]);
  mk_curlx_response_uptr res{mk_curlx_perform(req.get())};
  if (!res) {
    std::clog << "Out of memory" << std::endl;
    exit(EXIT_FAILURE);
  }
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
            << std::endl << std::endl;
  std::clog << "=== BEGIN LOGS ==="
            << std::endl
            << mk_curlx_response_get_logs(res.get())
            << "=== END LOGS ==="
            << std::endl << std::endl;
  std::clog << "=== BEGIN BODY ==="
            << std::endl
            << mk_curlx_response_get_body(res.get())
            << "=== END BODY ==="
            << std::endl << std::endl;
}
