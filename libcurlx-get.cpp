#include <stdlib.h>

#include "summary.hpp"

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
  //mk_curlx_request_set_proxy_url(req.get(), "socks5h://127.0.0.1:9050");
  //mk_curlx_request_set_ca_path(req.get(), "./saved_ca_bundle.pem");
  mk_curlx_request_enable_follow_redirect(req.get());
  mk_curlx_request_enable_http2(req.get());
  mk_curlx_request_set_url(req.get(), argv[1]);
  mk_curlx_response_uptr res{mk_curlx_perform(req.get())};
  if (!res) {
    std::clog << "Out of memory" << std::endl;
    exit(EXIT_FAILURE);
  }
  summary(res);
}
