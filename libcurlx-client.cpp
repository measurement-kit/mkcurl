#include <iostream>

#include <stdlib.h>

#include <iostream>
#include <sstream>

#define MK_CURLX_INLINE_IMPL
#include "libcurlx.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wsign-conversion"
#endif  // __clang__
#include "argh.h"
#ifdef __clang__
#pragma clang diagnostic pop
#endif  // __clang__

static void usage() {
  // clang-format off
  std::clog << "\n";
  std::clog << "Usage: libcurlx-client [options] <url>\n";
  std::clog << "\n";
  std::clog << "Options can start with either a single dash (i.e. -option) or\n";
  std::clog << "a double dash (i.e. --option). Available options:\n";
  std::clog << "\n";
  std::clog << "  --ca-bundle-path <path> : path to OpenSSL CA bundle\n";
  std::clog << "  --enable-http2          : enable HTTP2 support\n";
  std::clog << "  --follow-redirect       : enable following redirects\n";
  std::clog << "  --header <header>       : add <header> to headers\n";
  std::clog << "  --post                  : use POST rather than GET\n";
  std::clog << "  --post-data <data>      : send <data> as body\n";
  std::clog << std::endl;
  // clang-format on
}

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

int main(int, char **argv) {
  mk_curlx_request_uptr req{mk_curlx_request_new()};
  if (!req) {
    std::clog << "Out of memory" << std::endl;
    exit(EXIT_FAILURE);
  }
  {
    argh::parser cmdline;
    cmdline.add_param("ca-bundle-path");
    cmdline.add_param("header");
    cmdline.add_param("post-data");
    cmdline.parse(argv);
    for (auto &flag : cmdline.flags()) {
      if (flag == "enable-http2") {
        mk_curlx_request_enable_http2(req.get());
      } else if (flag == "follow-redirect") {
        mk_curlx_request_enable_follow_redirect(req.get());
      } else if (flag == "post") {
        mk_curlx_request_set_method_post(req.get());
      } else {
        std::clog << "fatal: unrecognized flag: " << flag << std::endl;
        usage();
        exit(EXIT_FAILURE);
      }
    }
    for (auto &param : cmdline.params()) {
      if (param.first == "ca-bundle-path") {
        mk_curlx_request_set_ca_path(req.get(), param.second.c_str());
      } else if (param.first == "header") {
        mk_curlx_request_add_header(req.get(), param.second.c_str());
      } else if (param.first == "post-data") {
        mk_curlx_request_set_body(req.get(), param.second.c_str());
      } else {
        std::clog << "fatal: unrecognized param: " << param.first << std::endl;
        usage();
        exit(EXIT_FAILURE);
      }
    }
    auto sz = cmdline.pos_args().size();
    if (sz != 2) {
      usage();
      exit(EXIT_FAILURE);
    }
    mk_curlx_request_set_url(req.get(), cmdline.pos_args()[1].c_str());
  }
  mk_curlx_response_uptr res{mk_curlx_perform(req.get())};
  if (!res) {
    std::clog << "Out of memory or really-bad internal error" << std::endl;
    exit(EXIT_FAILURE);
  }
  summary(res);
  if (mk_curlx_response_get_status_code(res.get()) != 200) {
    std::clog << "FATAL: the request did not succeed" << std::endl;
    exit(EXIT_FAILURE);
  }
}
