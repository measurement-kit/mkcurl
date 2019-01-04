#include <exception>
#include <mutex>

#include <curl/curl.h>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

// Static variables and macro allowing us to inject failures
// ---------------------------------------------------------

// MkCurlAbort is thrown instead of calling abort() such that we can verify
// with unit tests that invalid arguments are correctly handled.
struct MkCurlAbort : public std::exception {
  using std::exception::exception;
};

// MKCURL_ABORT overrides the default MKCURK_ABORT to throw MkCurlAbort.
#define MKCURL_ABORT() throw MkCurlAbort()

// By setting MKMOCK_HOOK_ENABLE and including mkmock.hpp we cause mkcurl.hpp
// to compile with mocking enabled so that we can run unit tests.
#define MKMOCK_HOOK_ENABLE
#include "mkmock.hpp"

MKMOCK_DEFINE_HOOK(curl_easy_init, CURL *);
MKMOCK_DEFINE_HOOK(curl_slist_append_headers, curl_slist *);

MKMOCK_DEFINE_HOOK(curl_slist_append_connect_to, curl_slist *);
MKMOCK_DEFINE_HOOK(curl_easy_setopt_CURLOPT_CONNECT_TO, CURLcode);

MKMOCK_DEFINE_HOOK(curl_easy_setopt_CURLOPT_URL, CURLcode);
MKMOCK_DEFINE_HOOK(curl_easy_setopt_CURLOPT_POSTFIELDSIZE, CURLcode);
MKMOCK_DEFINE_HOOK(curl_easy_setopt_CURLOPT_CUSTOMREQUEST, CURLcode);
MKMOCK_DEFINE_HOOK(curl_easy_setopt_CURLOPT_HTTPHEADER, CURLcode);
MKMOCK_DEFINE_HOOK(curl_easy_setopt_CURLOPT_CAINFO, CURLcode);
MKMOCK_DEFINE_HOOK(curl_easy_setopt_CURLOPT_HTTP_VERSION, CURLcode);
MKMOCK_DEFINE_HOOK(curl_slist_append_Expect_header, curl_slist *);
MKMOCK_DEFINE_HOOK(curl_easy_setopt_CURLOPT_POSTFIELDS, CURLcode);
MKMOCK_DEFINE_HOOK(curl_easy_setopt_CURLOPT_POST, CURLcode);
MKMOCK_DEFINE_HOOK(curl_easy_setopt_CURLOPT_WRITEFUNCTION, CURLcode);
MKMOCK_DEFINE_HOOK(curl_easy_setopt_CURLOPT_WRITEDATA, CURLcode);
MKMOCK_DEFINE_HOOK(curl_easy_setopt_CURLOPT_NOSIGNAL, CURLcode);
MKMOCK_DEFINE_HOOK(curl_easy_setopt_CURLOPT_TIMEOUT, CURLcode);
MKMOCK_DEFINE_HOOK(curl_easy_setopt_CURLOPT_DEBUGFUNCTION, CURLcode);
MKMOCK_DEFINE_HOOK(curl_easy_setopt_CURLOPT_DEBUGDATA, CURLcode);
MKMOCK_DEFINE_HOOK(curl_easy_setopt_CURLOPT_VERBOSE, CURLcode);
MKMOCK_DEFINE_HOOK(curl_easy_setopt_CURLOPT_PROXY, CURLcode);
MKMOCK_DEFINE_HOOK(curl_easy_setopt_CURLOPT_FOLLOWLOCATION, CURLcode);
MKMOCK_DEFINE_HOOK(curl_easy_setopt_CURLOPT_CERTINFO, CURLcode);
MKMOCK_DEFINE_HOOK(curl_easy_setopt_CURLOPT_TCP_FASTOPEN, CURLcode);

MKMOCK_DEFINE_HOOK(curl_easy_perform, CURLcode);

MKMOCK_DEFINE_HOOK(curl_easy_getinfo_CURLINFO_CONTENT_TYPE, CURLcode);
MKMOCK_DEFINE_HOOK(curl_easy_getinfo_CURLINFO_RESPONSE_CODE, CURLcode);
MKMOCK_DEFINE_HOOK(curl_easy_getinfo_CURLINFO_REDIRECT_URL, CURLcode);
MKMOCK_DEFINE_HOOK(curl_easy_getinfo_CURLINFO_CERTINFO, CURLcode);
MKMOCK_DEFINE_HOOK(curl_easy_getinfo_CURLINFO_HTTP_VERSION, CURLcode);

// Include mkcurl implementation
// -----------------------------

#define MKCURL_INLINE_IMPL
#include "mkcurl.hpp"

// Unit tests
// ----------

TEST_CASE("When mkcurl_body_cb is passed zero nmemb") {
  REQUIRE(mkcurl_body_cb(nullptr, 17, 0, nullptr) == 0);
}

TEST_CASE("When mkcurl_body_cb would overflow a size_t") {
  REQUIRE(mkcurl_body_cb(nullptr, SIZE_MAX / 2, 4, nullptr) == 0);
}

TEST_CASE("When mkcurl_body_cb is passed a NULL ptr") {
  REQUIRE_THROWS(mkcurl_body_cb(nullptr, 17, 4, (void *)0x123456));
}

TEST_CASE("When mkcurl_body_cb is passed a NULL userdata") {
  REQUIRE_THROWS(mkcurl_body_cb((char *)0x123456, 17, 4, nullptr));
}

TEST_CASE("When curl_easy_init fails") {
  MKMOCK_WITH_ENABLED_HOOK(curl_easy_init, nullptr, {
    mk::curl::Request req;
    mk::curl::Response resp = mk::curl::perform(req);
    REQUIRE(resp.error == CURLE_OUT_OF_MEMORY);
  });
}

TEST_CASE("When curl_slist_append fails for headers") {
  MKMOCK_WITH_ENABLED_HOOK(curl_slist_append_headers, nullptr, {
    mk::curl::Request req;
    req.headers.push_back("Content-Type: text/plain");
    mk::curl::Response resp = mk::curl::perform(req);
    REQUIRE(resp.error == CURLE_OUT_OF_MEMORY);
  });
}

#define CURL_EASY_SETOPT_FAILURE_TEST(Tag, Initialize)  \
  TEST_CASE("When " #Tag " fails") {                    \
    MKMOCK_WITH_ENABLED_HOOK(Tag, CURL_LAST, {          \
      mk::curl::Request req;                            \
      Initialize(req);                                  \
      mk::curl::Response resp = mk::curl::perform(req); \
      REQUIRE(resp.error == CURL_LAST);                 \
    });                                                 \
  }

CURL_EASY_SETOPT_FAILURE_TEST(
    curl_easy_setopt_CURLOPT_CAINFO,
    [](mk::curl::Request &r) {
      r.ca_path = "/etc/ssl/cert.pem";
    })

CURL_EASY_SETOPT_FAILURE_TEST(
    curl_easy_setopt_CURLOPT_HTTP_VERSION,
    [](mk::curl::Request &r) {
      r.enable_http2 = true;
    })

CURL_EASY_SETOPT_FAILURE_TEST(
    curl_easy_setopt_CURLOPT_HTTPHEADER,
    [](mk::curl::Request &r) {
      r.headers.push_back("Content-Type: text/plain");
    })

CURL_EASY_SETOPT_FAILURE_TEST(
    curl_easy_setopt_CURLOPT_POSTFIELDS,
    [](mk::curl::Request &r) {
      r.method = "POST";
      r.body = "12345 54321";
    })

CURL_EASY_SETOPT_FAILURE_TEST(
    curl_easy_setopt_CURLOPT_POST,
    [](mk::curl::Request &r) {
      r.method = "POST";
      r.body = "12345 54321";
    })

CURL_EASY_SETOPT_FAILURE_TEST(
    curl_easy_setopt_CURLOPT_URL, [](mk::curl::Request &) {})

CURL_EASY_SETOPT_FAILURE_TEST(
    curl_easy_setopt_CURLOPT_WRITEFUNCTION,
    [](mk::curl::Request &) {})

CURL_EASY_SETOPT_FAILURE_TEST(
    curl_easy_setopt_CURLOPT_WRITEDATA,
    [](mk::curl::Request &) {})

CURL_EASY_SETOPT_FAILURE_TEST(
    curl_easy_setopt_CURLOPT_NOSIGNAL,
    [](mk::curl::Request &) {})

CURL_EASY_SETOPT_FAILURE_TEST(
    curl_easy_setopt_CURLOPT_TIMEOUT,
    [](mk::curl::Request &) {})

CURL_EASY_SETOPT_FAILURE_TEST(
    curl_easy_setopt_CURLOPT_DEBUGFUNCTION,
    [](mk::curl::Request &) {})

CURL_EASY_SETOPT_FAILURE_TEST(
    curl_easy_setopt_CURLOPT_DEBUGDATA,
    [](mk::curl::Request &) {})

CURL_EASY_SETOPT_FAILURE_TEST(
    curl_easy_setopt_CURLOPT_VERBOSE,
    [](mk::curl::Request &) {})

CURL_EASY_SETOPT_FAILURE_TEST(
    curl_easy_setopt_CURLOPT_PROXY,
    [](mk::curl::Request &r) {
      r.proxy_url = "socks5h://127.0.0.1:9050";
    })

CURL_EASY_SETOPT_FAILURE_TEST(
    curl_easy_setopt_CURLOPT_FOLLOWLOCATION,
    [](mk::curl::Request &r) {
      r.follow_redir = true;
    })

CURL_EASY_SETOPT_FAILURE_TEST(
    curl_easy_setopt_CURLOPT_CERTINFO,
    [](mk::curl::Request &) {})

TEST_CASE("When curl_easy_perform() fails") {
  MKMOCK_WITH_ENABLED_HOOK(curl_easy_perform, CURL_LAST, {
    mk::curl::Request req;
    req.headers.push_back("Content-Type: text/plain");
    mk::curl::Response resp = mk::curl::perform(req);
    REQUIRE(resp.error == CURL_LAST);
  });
}

#define CURL_EASY_GETINFO_FAILURE_TEST(Tag)                 \
  TEST_CASE("When " #Tag ") fails") {                       \
    MKMOCK_WITH_ENABLED_HOOK(curl_easy_perform, CURLE_OK, { \
      MKMOCK_WITH_ENABLED_HOOK(Tag, CURL_LAST, {            \
        mk::curl::Request req;                              \
        mk::curl::Response resp = mk::curl::perform(req);   \
        REQUIRE(resp.error == CURL_LAST);                   \
      });                                                   \
    });                                                     \
  }

CURL_EASY_GETINFO_FAILURE_TEST(
    curl_easy_getinfo_CURLINFO_RESPONSE_CODE)

CURL_EASY_GETINFO_FAILURE_TEST(
    curl_easy_getinfo_CURLINFO_REDIRECT_URL)

CURL_EASY_GETINFO_FAILURE_TEST(
    curl_easy_getinfo_CURLINFO_CERTINFO)
