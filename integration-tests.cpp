#include <string.h>

#include <exception>
#include <mutex>

#include <curl/curl.h>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "mkcurl.hpp"

static void run(mk::curl::Response res, bool tolerate_failure) {
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
  if (!tolerate_failure) {
    REQUIRE(res.error == 0);
    REQUIRE(res.status_code == 200);
  }
}

TEST_CASE("TCP fastopen works") {
  // Windows does not support TCP fastopen currently.
#ifdef _WIN32
  bool tolerate_failure = true;
#else
  bool tolerate_failure = false;
#endif
  mk::curl::Request req;
  req.enable_fastopen = true;
  req.url = "https://www.kernel.org";
  run(mk::curl::perform(req), tolerate_failure);
}

TEST_CASE("HTTP2 works") {
  // We should recompile CURL for Windows with HTTP2 support
#ifdef _WIN32
  bool tolerate_failure = true;
#else
  bool tolerate_failure = false;
#endif
  mk::curl::Request req;
  req.enable_http2 = true;
  req.url = "https://www.google.org";
  run(mk::curl::perform(req), tolerate_failure);
}
