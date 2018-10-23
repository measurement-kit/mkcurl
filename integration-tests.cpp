#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#define MKCURL_INLINE_IMPL
#include "mkcurl.h"

TEST_CASE("We can move in and move out possibly binary data") {
  mkcurl_request_uptr request{mkcurl_request_new()};
  REQUIRE(request != nullptr);
  mkcurl_request_set_method_post(request.get());
  mkcurl_request_add_header(request.get(), "Content-Type: application/json");
  mkcurl_request_set_url(
      request.get(), "https://bouncer.ooni.io/bouncer/net-tests");
  // clang-format off
  std::string request_body = R"({
    "net-tests":[{
      "input-hashes": null,
      "name":"http_invalid_request_line",
      "test-helpers":["tcp-echo"],
      "version":"0.0.3"
    }]
  })";
  // clang-format on
  mkcurl_request_movein_body(request.get(), std::move(request_body));
  mkcurl_response_uptr response{mkcurl_perform(request.get())};
  REQUIRE(response != nullptr);
  REQUIRE(mkcurl_response_get_status_code(response.get()) == 200);
  {
    std::string body;
    REQUIRE(mkcurl_response_moveout_body(response.get(), &body));
    REQUIRE(body.size() > 0);
  }
  {
    std::string logs;
    REQUIRE(mkcurl_response_moveout_logs(response.get(), &logs));
    REQUIRE(logs.size() > 0);
  }
  {
    std::string rh;
    REQUIRE(mkcurl_response_moveout_response_headers(response.get(), &rh));
    REQUIRE(rh.size() > 0);
  }
}
