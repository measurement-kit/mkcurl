#include <iostream>

#include <stdlib.h>

#include <iostream>
#include <sstream>

#include "mkcurl.hpp"

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
  std::clog << "Usage: mkcurl-client [options] <url>...\n";
  std::clog << "\n";
  std::clog << "Options can start with either a single dash (i.e. -option) or\n";
  std::clog << "a double dash (i.e. --option). Available options:\n";
  std::clog << "\n";
  std::clog << "  --ca-bundle-path <path> : path to OpenSSL CA bundle\n";
  std::clog << "  --connect-to <ip>       : connects to <ip> while using the\n";
  std::clog << "                            host in the URL for TLS SNI, if\n";
  std::clog << "                            using https. Note that IPv6 must\n";
  std::clog << "                            be quoted using [ and ]\n";
  std::clog << "  --data <data>           : send <data> as body\n";
  std::clog << "  --enable-http2          : enable HTTP2 support\n";
  std::clog << "  --enable-tcp-fastopen   : enable TCP fastopen support\n";
  std::clog << "  --follow-redirect       : enable following redirects\n";
  std::clog << "  --header <header>       : add <header> to headers\n";
  std::clog << "  --post                  : use POST rather than GET\n";
  std::clog << "  --put                   : use PUT rather than GET\n";
  std::clog << "  --timeout <sec>         : set timeout of <sec> seconds\n";
  std::clog << std::endl;
  // clang-format on
}
// LCOV_EXCL_STOP

static void summary(mk::curl::Response &res) {
  std::clog << "=== BEGIN SUMMARY ==="
            << std::endl << "CURL error code: " << res.error << std::endl
            << "HTTP status code: " << res.status_code << std::endl
            << "Bytes sent: " << res.bytes_sent << std::endl
            << "Bytes recv: " << res.bytes_recv << std::endl
            << "Redirect URL: " << res.redirect_url << std::endl
            << "Content Type: " << res.content_type << std::endl
            << "HTTP version: " << res.http_version << std::endl
            << "=== END SUMMARY ===" << std::endl << std::endl;
  std::clog << "=== BEGIN REQUEST HEADERS ==="
            << std::endl << res.request_headers
            << "=== END REQUEST HEADERS ==="
            << std::endl << std::endl;
  std::clog << "=== BEGIN RESPONSE HEADERS ==="
            << std::endl << res.response_headers
            << "=== END RESPONSE HEADERS ==="
            << std::endl << std::endl;
  std::clog << "=== BEGIN CERTIFICATE CHAIN ==="
            << std::endl << res.certs
            << "=== END CERTIFICATE CHAIN ==="
            << std::endl << std::endl;
  std::clog << "=== BEGIN LOGS ===" << std::endl;
  for (auto &log : res.logs) {
    std::clog << "[" << log.msec << "] " << log.line << std::endl;
  }
  std::clog << "=== END LOGS ===" << std::endl << std::endl;
  std::clog << "=== BEGIN BODY ===" << std::endl << res.body
            << "=== END BODY ===" << std::endl << std::endl;
}

int main(int, char **argv) {
  mk::curl::Request req;
  argh::parser cmdline;
  {
    cmdline.add_param("ca-bundle-path");
    cmdline.add_param("connect-to");
    cmdline.add_param("data");
    cmdline.add_param("header");
    cmdline.add_param("timeout");
    cmdline.parse(argv);
    for (auto &flag : cmdline.flags()) {
      if (flag == "enable-http2") {
        req.enable_http2 = true;
      } else if (flag == "enable-tcp-fastopen") {
        req.enable_fastopen = true;
      } else if (flag == "follow-redirect") {
        req.follow_redir = true;
      } else if (flag == "post") {
        req.method = "POST";
      } else if (flag == "put") {
        req.method = "PUT";
      } else {
        // LCOV_EXCL_START
        std::clog << "fatal: unrecognized flag: " << flag << std::endl;
        usage();
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
      }
    }
    for (auto &param : cmdline.params()) {
      if (param.first == "ca-bundle-path") {
        req.ca_path = param.second;
      } else if (param.first == "connect-to") {
        std::stringstream ss;
        ss << "::" << param.second << ":";
        req.connect_to = ss.str();
      } else if (param.first == "data") {
        req.body = param.second;
      } else if (param.first == "header") {
        req.headers.push_back(param.second);
      } else if (param.first == "timeout") {
        // Implementation note: since this is meant to be just a testing
        // client, we don't bother with properly validating the number that
        // is passed here and we just use atoi(). A really robust client
        // SHOULD instead use strtonum().
        req.timeout = atoi(param.second.c_str());
      } else {
        // LCOV_EXCL_START
        std::clog << "fatal: unrecognized param: " << param.first << std::endl;
        usage();
        exit(EXIT_FAILURE);
        // LCOV_EXCL_STOP
      }
    }
    auto sz = cmdline.pos_args().size();
    if (sz < 2) {
      // LCOV_EXCL_START
      usage();
      exit(EXIT_FAILURE);
      // LCOV_EXCL_STOP
    }
  }
  auto exitcode = EXIT_SUCCESS;
  mk::curl::Client client;
  for (size_t sz = 1; sz < cmdline.pos_args().size(); ++sz) {
    mk::curl::Request real_request{req};
    real_request.url = cmdline.pos_args()[sz];
    mk::curl::Response res = client.perform(real_request);
    summary(res);
    if (res.error != 0 || res.status_code != 200) {
      // LCOV_EXCL_START
      std::clog << "FATAL: the request did not succeed" << std::endl;
      exitcode = EXIT_FAILURE;
      // LCOV_EXCL_STOP
    }
  }
  exit(exitcode);
}
