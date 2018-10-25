#include <exception>
#include <mutex>

#include <curl/curl.h>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

// Mock cURL APIs we use
// ---------------------

struct MkCurlMock {
  virtual CURL *curl_easy_init() = 0;
  virtual curl_slist *curl_slist_append(curl_slist *, const char *) = 0;
  virtual CURLcode curl_easy_setopt(CURL *, CURLoption, ...) = 0;
  virtual CURLcode curl_easy_perform(CURL *) = 0;
  virtual CURLcode curl_easy_getinfo(CURL *, CURLINFO, ...) = 0;
  virtual ~MkCurlMock() {}
};

static MkCurlMock *MOCK = nullptr;

// Provide definitions before including mkcurl.h so that the code will
// actually call the mocked functions rather than cURL real functions.
#define MKCURL_EASY_INIT MOCK->curl_easy_init
#define MKCURL_SLIST_APPEND MOCK->curl_slist_append
#define MKCURL_EASY_SETOPT MOCK->curl_easy_setopt
#define MKCURL_EASY_PERFORM MOCK->curl_easy_perform
#define MKCURL_EASY_GETINFO MOCK->curl_easy_getinfo

template <typename T> void with_mock(std::function<void()> &&fun) {
  static std::mutex barrier;
  std::unique_lock<std::mutex> lock{barrier};
  std::unique_ptr<MkCurlMock> mp{new T};
  std::exception_ptr ep;
  REQUIRE(MOCK == nullptr);
  MOCK = mp.get();
  try {
    fun();
  } catch (const std::exception &) {
    ep = std::current_exception();
  }
  MOCK = nullptr;
  if (ep) std::rethrow_exception(ep);
}

// Include mkcurl implementation
// -------------------------------

#define MKCURL_INLINE_IMPL
#include "mkcurl.h"

// Unit tests
// ----------

struct MkCurlEasyInitFailure : public MkCurlMock {
  CURL *curl_easy_init() override { return nullptr; }
  curl_slist *curl_slist_append(curl_slist *, const char *) override {
    abort();
  }
  CURLcode curl_easy_setopt(CURL *, CURLoption, ...) override {
    abort();
  }
  CURLcode curl_easy_perform(CURL *) override {
    abort();
  }
  CURLcode curl_easy_getinfo(CURL *, CURLINFO, ...) override {
    abort();
  }
  ~MkCurlEasyInitFailure() override {}
};

TEST_CASE("We deal with curl_easy_init() failure") {
  with_mock<MkCurlEasyInitFailure>([]() {
    mkcurl_request_uptr req{mkcurl_request_new()};
    mkcurl_response_uptr resp{mkcurl_perform(req.get())};
    REQUIRE(mkcurl_response_get_error(resp.get()) == CURLE_OUT_OF_MEMORY);
  });
}

struct MkCurlSlistAppendFailure : public MkCurlMock {
  CURL *curl_easy_init() override { return ::curl_easy_init(); }
  curl_slist *curl_slist_append(curl_slist *, const char *) override {
    return nullptr;
  }
  CURLcode curl_easy_setopt(CURL *, CURLoption, ...) override {
    abort();
  }
  CURLcode curl_easy_perform(CURL *) override {
    abort();
  }
  CURLcode curl_easy_getinfo(CURL *, CURLINFO, ...) override {
    abort();
  }
  ~MkCurlSlistAppendFailure() override {}
};

TEST_CASE("We deal with curl_slist_append() failure") {
  with_mock<MkCurlSlistAppendFailure>([]() {
    mkcurl_request_uptr req{mkcurl_request_new()};
    mkcurl_request_add_header(req.get(), "Content-Type: text/plain");
    mkcurl_response_uptr resp{mkcurl_perform(req.get())};
    REQUIRE(mkcurl_response_get_error(resp.get()) == CURLE_OUT_OF_MEMORY);
  });
}

// Note: in the following we use CURLE_NOT_BUILT_IN as the random error
// that we return to see that mocks are working as intended.

template <int value_for_failure>
struct MkCurlEasySetoptFailure : public MkCurlMock {
  CURL *curl_easy_init() override { return ::curl_easy_init(); }
  curl_slist *curl_slist_append(curl_slist *lst, const char *str) override {
    // Must not abort() because we need to set headers
    return ::curl_slist_append(lst, str);
  }
  CURLcode curl_easy_setopt(CURL *, CURLoption option, ...) override {
    return (option == value_for_failure) ? CURLE_NOT_BUILT_IN : CURLE_OK;
  }
  CURLcode curl_easy_perform(CURL *) override {
    abort();
  }
  CURLcode curl_easy_getinfo(CURL *, CURLINFO, ...) override {
    abort();
  }
  ~MkCurlEasySetoptFailure() override {}
};

#define MK_CURL_EASY_SETOPT_FAILURE_TEST(value, func)                       \
  TEST_CASE("We deal with curl_easy_setopt failure for: " #value) {         \
    with_mock<MkCurlEasySetoptFailure<value>>([]() {                        \
      mkcurl_request_uptr req{mkcurl_request_new()};                        \
      func(req);                                                            \
      mkcurl_response_uptr resp{mkcurl_perform(req.get())};                 \
      REQUIRE(mkcurl_response_get_error(resp.get()) == CURLE_NOT_BUILT_IN); \
    });                                                                     \
  }

MK_CURL_EASY_SETOPT_FAILURE_TEST(
    CURLOPT_CAINFO,
    [](mkcurl_request_uptr &req) {
      mkcurl_request_set_ca_bundle_path(req.get(), "/etc/ssl/cert.pem");
    })

MK_CURL_EASY_SETOPT_FAILURE_TEST(
    CURLOPT_HTTP_VERSION,
    [](mkcurl_request_uptr &req) {
      mkcurl_request_enable_http2(req.get());
    })

MK_CURL_EASY_SETOPT_FAILURE_TEST(
    CURLOPT_HTTPHEADER,
    [](mkcurl_request_uptr &req) {
      mkcurl_request_add_header(req.get(), "Content-Type: text/plain");
    })

MK_CURL_EASY_SETOPT_FAILURE_TEST(
    CURLOPT_POSTFIELDS,
    [](mkcurl_request_uptr &req) {
      mkcurl_request_set_method_post(req.get());
      mkcurl_request_set_body(req.get(), "12345 54321");
    })

MK_CURL_EASY_SETOPT_FAILURE_TEST(
    CURLOPT_POST,
    [](mkcurl_request_uptr &req) {
      mkcurl_request_set_method_post(req.get());
      mkcurl_request_set_body(req.get(), "12345 54321");
    })

MK_CURL_EASY_SETOPT_FAILURE_TEST(
    CURLOPT_URL, [](mkcurl_request_uptr &) {})

MK_CURL_EASY_SETOPT_FAILURE_TEST(
    CURLOPT_WRITEFUNCTION, [](mkcurl_request_uptr &) {})

MK_CURL_EASY_SETOPT_FAILURE_TEST(
    CURLOPT_WRITEDATA, [](mkcurl_request_uptr &) {})

MK_CURL_EASY_SETOPT_FAILURE_TEST(
    CURLOPT_NOSIGNAL, [](mkcurl_request_uptr &) {})

MK_CURL_EASY_SETOPT_FAILURE_TEST(
    CURLOPT_TIMEOUT, [](mkcurl_request_uptr &) {})

MK_CURL_EASY_SETOPT_FAILURE_TEST(
    CURLOPT_DEBUGFUNCTION, [](mkcurl_request_uptr &) {})

MK_CURL_EASY_SETOPT_FAILURE_TEST(
    CURLOPT_DEBUGDATA, [](mkcurl_request_uptr &) {})

MK_CURL_EASY_SETOPT_FAILURE_TEST(
    CURLOPT_VERBOSE, [](mkcurl_request_uptr &) {})

MK_CURL_EASY_SETOPT_FAILURE_TEST(
    CURLOPT_PROXY,
    [](mkcurl_request_uptr &req) {
      mkcurl_request_set_proxy_url(req.get(), "socks5h://127.0.0.1:9050");
    })

MK_CURL_EASY_SETOPT_FAILURE_TEST(
    CURLOPT_FOLLOWLOCATION,
    [](mkcurl_request_uptr &req) {
      mkcurl_request_enable_follow_redirect(req.get());
    })

MK_CURL_EASY_SETOPT_FAILURE_TEST(
    CURLOPT_CERTINFO, [](mkcurl_request_uptr &) {})

struct MkCurlEasyPerformFailure : public MkCurlMock {
  CURL *curl_easy_init() override { return ::curl_easy_init(); }
  curl_slist *curl_slist_append(curl_slist *lst, const char *str) override {
    return ::curl_slist_append(lst, str);
  }
  CURLcode curl_easy_setopt(CURL *, CURLoption, ...) override {
    return CURLE_OK;
  }
  CURLcode curl_easy_perform(CURL *) override {
    return CURLE_NOT_BUILT_IN;
  }
  CURLcode curl_easy_getinfo(CURL *, CURLINFO, ...) override {
    abort();
  }
  ~MkCurlEasyPerformFailure() override {}
};

TEST_CASE("We deal with curl_easy_perform() failure") {
  with_mock<MkCurlEasyPerformFailure>([]() {
    mkcurl_request_uptr req{mkcurl_request_new()};
    mkcurl_request_add_header(req.get(), "Content-Type: text/plain");
    mkcurl_response_uptr resp{mkcurl_perform(req.get())};
    REQUIRE(mkcurl_response_get_error(resp.get()) == CURLE_NOT_BUILT_IN);
  });
}

template <int value_for_failure>
struct MkCurlEasyGetinfoFailure : public MkCurlMock {
  CURL *curl_easy_init() override { return ::curl_easy_init(); }
  curl_slist *curl_slist_append(curl_slist *, const char *) override {
    abort();
  }
  CURLcode curl_easy_setopt(CURL *, CURLoption, ...) override {
    return CURLE_OK;
  }
  CURLcode curl_easy_perform(CURL *) override {
    return CURLE_OK;
  }
  CURLcode curl_easy_getinfo(CURL *, CURLINFO info, ...) override {
    return (info == value_for_failure) ? CURLE_NOT_BUILT_IN : CURLE_OK;
  }
  ~MkCurlEasyGetinfoFailure() override {}
};

#define MK_CURL_EASY_GETINFO_FAILURE_TEST(value)                            \
  TEST_CASE("We deal with curl_easy_setopt failure for: " #value) {         \
    with_mock<MkCurlEasyGetinfoFailure<value>>([]() {                       \
      mkcurl_request_uptr req{mkcurl_request_new()};                        \
      mkcurl_response_uptr resp{mkcurl_perform(req.get())};                 \
      REQUIRE(mkcurl_response_get_error(resp.get()) == CURLE_NOT_BUILT_IN); \
    });                                                                     \
  }

MK_CURL_EASY_GETINFO_FAILURE_TEST(CURLINFO_RESPONSE_CODE)

MK_CURL_EASY_GETINFO_FAILURE_TEST(CURLINFO_REDIRECT_URL)

MK_CURL_EASY_GETINFO_FAILURE_TEST(CURLINFO_CERTINFO)

TEST_CASE("We can successfully copy a response") {
  // This is possible because we include the inline implementation
  mkcurl_response res;
  res.error = CURL_LAST;
  res.body = "ABC";
  res.bytes_recv = 128.0;
  res.certs = "VWXYZ";
  mkcurl_response_uptr uptr{mkcurl_response_copy(&res)};
  REQUIRE(uptr->error == res.error);
  REQUIRE(uptr->body == res.body);
  REQUIRE(uptr->bytes_recv == res.bytes_recv);
  REQUIRE(uptr->certs == res.certs);
}
