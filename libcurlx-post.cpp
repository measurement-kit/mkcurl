#include <stdlib.h>

#include "summary.hpp"

int main(int argc, char **) {
  if (argc != 1) {
    std::clog << "Usage: ./libcurlx-post" << std::endl;
    exit(EXIT_FAILURE);
  }
  mk_curlx_request_uptr req{mk_curlx_request_new()};
  if (!req) {
    std::clog << "Out of memory" << std::endl;
    exit(EXIT_FAILURE);
  }
  //mk_curlx_request_set_proxy_url(req.get(), "socks5h://127.0.0.1:9050");
  mk_curlx_request_enable_follow_redirect(req.get());
  mk_curlx_request_enable_http2(req.get());
  mk_curlx_request_set_method_post(req.get());
  mk_curlx_request_add_header(req.get(), "Content-Type", "application/json");
  mk_curlx_request_set_body(req.get(), R"({
    "net-tests":[{
      "input-hashes":null,
      "name":"http_invalid_request_line",
      "test-helpers":["tcp-echo"],
      "version":"0.0.3"}]}
  )");
  mk_curlx_request_set_url(
      req.get(), "https://bouncer.ooni.io/bouncer/net-tests");
  mk_curlx_response_uptr res{mk_curlx_perform(req.get())};
  if (!res) {
    std::clog << "Out of memory" << std::endl;
    exit(EXIT_FAILURE);
  }
  summary(res);
}
