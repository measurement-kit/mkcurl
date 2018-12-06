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

// By setting MKMOCK_HOOK_ENABLE and including mkmock.hpp we cause mkcurl.h to
// compile with mocking enabled so that we can run unit tests.
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
// -------------------------------

#define MKCURL_INLINE_IMPL
#include "mkcurl.h"

// Unit tests
// ----------

TEST_CASE("When curl_easy_init fails") {
  MKMOCK_WITH_ENABLED_HOOK(curl_easy_init, nullptr, {
    mkcurl_request_uptr req{mkcurl_request_new_nonnull()};
    mkcurl_response_uptr resp{mkcurl_request_perform_nonnull(req.get())};
    REQUIRE(mkcurl_response_get_error_v2(resp.get()) == CURLE_OUT_OF_MEMORY);
  });
}

TEST_CASE("When curl_slist_append fails for headers") {
  MKMOCK_WITH_ENABLED_HOOK(curl_slist_append_headers, nullptr, {
    mkcurl_request_uptr req{mkcurl_request_new_nonnull()};
    mkcurl_request_add_header_v2(req.get(), "Content-Type: text/plain");
    mkcurl_response_uptr resp{mkcurl_request_perform_nonnull(req.get())};
    REQUIRE(mkcurl_response_get_error_v2(resp.get()) == CURLE_OUT_OF_MEMORY);
  });
}

#define CURL_EASY_SETOPT_FAILURE_TEST(Tag, Initialize)                      \
  TEST_CASE("When " #Tag " fails") {                                        \
    MKMOCK_WITH_ENABLED_HOOK(Tag, CURL_LAST, {                              \
      mkcurl_request_uptr req{mkcurl_request_new_nonnull()};                \
      Initialize(req);                                                      \
      mkcurl_response_uptr resp{mkcurl_request_perform_nonnull(req.get())}; \
      REQUIRE(mkcurl_response_get_error_v2(resp.get()) == CURL_LAST);       \
    });                                                                     \
  }

CURL_EASY_SETOPT_FAILURE_TEST(
    curl_easy_setopt_CURLOPT_CAINFO,
    [](mkcurl_request_uptr &r) {
      mkcurl_request_set_ca_bundle_path_v2(r.get(), "/etc/ssl/cert.pem");
    })

CURL_EASY_SETOPT_FAILURE_TEST(
    curl_easy_setopt_CURLOPT_HTTP_VERSION,
    [](mkcurl_request_uptr &r) {
      mkcurl_request_enable_http2_v2(r.get());
    })

CURL_EASY_SETOPT_FAILURE_TEST(
    curl_easy_setopt_CURLOPT_HTTPHEADER,
    [](mkcurl_request_uptr &r) {
      mkcurl_request_add_header_v2(r.get(), "Content-Type: text/plain");
    })

CURL_EASY_SETOPT_FAILURE_TEST(
    curl_easy_setopt_CURLOPT_POSTFIELDS,
    [](mkcurl_request_uptr &r) {
      mkcurl_request_set_method_post_v2(r.get());
      std::string s = "12345 54321";
      mkcurl_request_set_body_binary_v3(
          r.get(), (const uint8_t *)s.c_str(), s.size());
    })

CURL_EASY_SETOPT_FAILURE_TEST(
    curl_easy_setopt_CURLOPT_POST,
    [](mkcurl_request_uptr &r) {
      mkcurl_request_set_method_post_v2(r.get());
      std::string s = "12345 54321";
      mkcurl_request_set_body_binary_v3(
          r.get(), (const uint8_t *)s.c_str(), s.size());
    })

CURL_EASY_SETOPT_FAILURE_TEST(
    curl_easy_setopt_CURLOPT_URL, [](mkcurl_request_uptr &) {})

CURL_EASY_SETOPT_FAILURE_TEST(
    curl_easy_setopt_CURLOPT_WRITEFUNCTION,
    [](mkcurl_request_uptr &) {})

CURL_EASY_SETOPT_FAILURE_TEST(
    curl_easy_setopt_CURLOPT_WRITEDATA,
    [](mkcurl_request_uptr &) {})

CURL_EASY_SETOPT_FAILURE_TEST(
    curl_easy_setopt_CURLOPT_NOSIGNAL,
    [](mkcurl_request_uptr &) {})

CURL_EASY_SETOPT_FAILURE_TEST(
    curl_easy_setopt_CURLOPT_TIMEOUT,
    [](mkcurl_request_uptr &) {})

CURL_EASY_SETOPT_FAILURE_TEST(
    curl_easy_setopt_CURLOPT_DEBUGFUNCTION,
    [](mkcurl_request_uptr &) {})

CURL_EASY_SETOPT_FAILURE_TEST(
    curl_easy_setopt_CURLOPT_DEBUGDATA,
    [](mkcurl_request_uptr &) {})

CURL_EASY_SETOPT_FAILURE_TEST(
    curl_easy_setopt_CURLOPT_VERBOSE,
    [](mkcurl_request_uptr &) {})

CURL_EASY_SETOPT_FAILURE_TEST(
    curl_easy_setopt_CURLOPT_PROXY,
    [](mkcurl_request_uptr &r) {
      mkcurl_request_set_proxy_url_v2(r.get(), "socks5h://127.0.0.1:9050");
    })

CURL_EASY_SETOPT_FAILURE_TEST(
    curl_easy_setopt_CURLOPT_FOLLOWLOCATION,
    [](mkcurl_request_uptr &r) {
      mkcurl_request_enable_follow_redirect_v2(r.get());
    })

CURL_EASY_SETOPT_FAILURE_TEST(
    curl_easy_setopt_CURLOPT_CERTINFO,
    [](mkcurl_request_uptr &) {})

TEST_CASE("When curl_easy_perform() fails") {
  MKMOCK_WITH_ENABLED_HOOK(curl_easy_perform, CURL_LAST, {
    mkcurl_request_uptr req{mkcurl_request_new_nonnull()};
    mkcurl_request_add_header_v2(req.get(), "Content-Type: text/plain");
    mkcurl_response_uptr resp{mkcurl_request_perform_nonnull(req.get())};
    REQUIRE(mkcurl_response_get_error_v2(resp.get()) == CURL_LAST);
  });
}

#define CURL_EASY_GETINFO_FAILURE_TEST(Tag)                                   \
  TEST_CASE("When " #Tag ") fails") {                                         \
    MKMOCK_WITH_ENABLED_HOOK(curl_easy_perform, CURLE_OK, {                   \
      MKMOCK_WITH_ENABLED_HOOK(Tag, CURL_LAST, {                              \
        mkcurl_request_uptr req{mkcurl_request_new_nonnull()};                \
        mkcurl_response_uptr resp{mkcurl_request_perform_nonnull(req.get())}; \
        REQUIRE(mkcurl_response_get_error_v2(resp.get()) == CURL_LAST);       \
      });                                                                     \
    });                                                                       \
  }

CURL_EASY_GETINFO_FAILURE_TEST(
    curl_easy_getinfo_CURLINFO_RESPONSE_CODE)

CURL_EASY_GETINFO_FAILURE_TEST(
    curl_easy_getinfo_CURLINFO_REDIRECT_URL)

CURL_EASY_GETINFO_FAILURE_TEST(
    curl_easy_getinfo_CURLINFO_CERTINFO)
