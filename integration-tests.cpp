#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "mkcurl.h"

TEST_CASE("We can move in and move out possibly binary data") {
  mkcurl_request_uptr request{mkcurl_request_new_nonnull()};
  mkcurl_request_set_method_post_v2(request.get());
  mkcurl_request_add_header_v2(request.get(), "Content-Type: application/json");
  mkcurl_request_set_url_v2(
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
  mkcurl_request_movein_body_v2(request, std::move(request_body));
  mkcurl_response_uptr response{mkcurl_request_perform_nonnull(request.get())};
  REQUIRE(mkcurl_response_get_status_code_v2(response.get()) == 200);
  {
    std::string body = mkcurl_response_moveout_body_v2(response);
    REQUIRE(body.size() > 0);
  }
  {
    std::string logs = mkcurl_response_moveout_logs_v2(response);
    REQUIRE(logs.size() > 0);
  }
  {
    std::string rh = mkcurl_response_moveout_response_headers_v2(response);
    REQUIRE(rh.size() > 0);
  }
}
