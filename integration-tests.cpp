#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "mkcurl.hpp"

TEST_CASE("We can move in and move out possibly binary data") {
  mk::curl::Request request;
  request.method = "POST";
  request.headers.push_back("Content-Type: application/json");
  request.url = "https://bouncer.ooni.io/bouncer/net-tests";
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
  request.body = std::move(request_body);
  mk::curl::Response response = mk::curl::perform(request);
  REQUIRE(response.status_code == 200);
  REQUIRE(response.body.size() > 0);
  REQUIRE(response.logs.size() > 0);
  REQUIRE(response.response_headers.size() > 0);
}
