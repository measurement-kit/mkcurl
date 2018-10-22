#include <iostream>

#include <stdlib.h>

#include <iostream>
#include <sstream>

#define MKCURL_INLINE_IMPL
#include "mkcurl.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wsign-conversion"
#endif  // __clang__
#include "argh.h"
#ifdef __clang__
#pragma clang diagnostic pop
#endif  // __clang__

// LCOV_EXCL_START
static void usage() {
  // clang-format off
  std::clog << "\n";
  std::clog << "Usage: mkcurl-client [options] <url>\n";
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
  std::clog << "  --timeout <sec>         : set timeout of <sec> seconds\n";
  std::clog << std::endl;
  // clang-format on
}
// LCOV_EXCL_STOP

static void summary(const mkcurl_response_uptr &res) {
  std::clog << "=== BEGIN SUMMARY ==="
            << std::endl
            << "CURL error code: "
            << mkcurl_response_get_error(res.get())
            << std::endl
            << "HTTP status code: "
            << mkcurl_response_get_status_code(res.get())
            << std::endl
            << "Bytes sent: "
            << mkcurl_response_get_bytes_sent(res.get())
            << std::endl
            << "Bytes recv: "
            << mkcurl_response_get_bytes_recv(res.get())
            << std::endl
            << "Redirect URL: "
            << mkcurl_response_get_redirect_url(res.get())
            << std::endl
            << "=== END SUMMARY ==="
            << std::endl
            << std::endl;
  std::clog << "=== BEGIN REQUEST HEADERS ==="
            << std::endl
            << mkcurl_response_get_request_headers(res.get())
            << "=== END REQUEST HEADERS ==="
            << std::endl
            << std::endl;
  std::clog << "=== BEGIN RESPONSE HEADERS ==="
            << std::endl
            << mkcurl_response_get_response_headers(res.get())
            << "=== END RESPONSE HEADERS ==="
            << std::endl
            << std::endl;
  std::clog << "=== BEGIN CERTIFICATE CHAIN ==="
            << std::endl
            << mkcurl_response_get_certificate_chain(res.get())
            << "=== END CERTIFICATE CHAIN ==="
            << std::endl
            << std::endl;
  std::clog << "=== BEGIN LOGS ==="
            << std::endl
            << mkcurl_response_get_logs(res.get())
            << "=== END LOGS ==="
            << std::endl
            << std::endl;
  std::clog << "=== BEGIN BODY ==="
            << std::endl
            << mkcurl_response_get_body(res.get())
            << "=== END BODY ==="
            << std::endl
            << std::endl;
}

int main(int, char **argv) {
  mkcurl_request_uptr req{mkcurl_request_new()};
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
        mkcurl_request_enable_http2(req.get());
      } else if (flag == "follow-redirect") {
        mkcurl_request_enable_follow_redirect(req.get());
      } else if (flag == "post") {
        mkcurl_request_set_method_post(req.get());
      } else {
        std::clog << "fatal: unrecognized flag: " << flag << std::endl;
        usage();
        exit(EXIT_FAILURE);
      }
    }
    for (auto &param : cmdline.params()) {
      if (param.first == "ca-bundle-path") {
        mkcurl_request_set_ca_path(req.get(), param.second.c_str());
      } else if (param.first == "header") {
        mkcurl_request_add_header(req.get(), param.second.c_str());
      } else if (param.first == "post-data") {
        mkcurl_request_set_body(req.get(), param.second.c_str());
      } else if (param.first == "timeout") {
        // Implementation note: since this is meant to be just a testing
        // client, we don't bother with properly validating the number that
        // is passed here and we just use atoi(). A really robust client
        // SHOULD instead use strtonum().
        mkcurl_request_set_timeout(req.get(), atoi(param.second.c_str()));
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
    mkcurl_request_set_url(req.get(), cmdline.pos_args()[1].c_str());
  }
  mkcurl_response_uptr res{mkcurl_perform(req.get())};
  if (!res) {
    std::clog << "Out of memory or really-bad internal error" << std::endl;
    exit(EXIT_FAILURE);
  }
  summary(res);
  if (mkcurl_response_get_error(res.get()) != CURLE_OK ||
      mkcurl_response_get_status_code(res.get()) != 200) {
    std::clog << "FATAL: the request did not succeed" << std::endl;
    exit(EXIT_FAILURE);
  }
}
