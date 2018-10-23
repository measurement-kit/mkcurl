#ifndef MEASUREMENT_KIT_MKCURL_H
#define MEASUREMENT_KIT_MKCURL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/// mkcurl_request_t is a HTTP request you send using CURL.
typedef struct mkcurl_request mkcurl_request_t;

/// mkcurl_request_new creates a new request object. May return NULL on error.
mkcurl_request_t *mkcurl_request_new(void);

/// mkcurl_request_set_ca_bundle_path sets the CA bundle path. Typically
/// this option is required on mobile devices.
void mkcurl_request_set_ca_bundle_path(mkcurl_request_t *req, const char *p);

/// mkcurl_request_enable_http2 enables using the HTTP2 protocol. It may fail
/// when performing the request if CURL's HTTP2 backend is not compiled in.
void mkcurl_request_enable_http2(mkcurl_request_t *req);

/// mkcurl_request_set_method_post sets the method to POST (default is GET).
void mkcurl_request_set_method_post(mkcurl_request_t *req);

/// mkcurl_request_set_url sets the request URL. This setting is required.
void mkcurl_request_set_url(mkcurl_request_t *req, const char *u);

/// mkcurl_request_add_header adds an header to the request.
void mkcurl_request_add_header(mkcurl_request_t *req, const char *h);

/// mkcurl_request_set_body sets the request body. Make sure you also use POST.
void mkcurl_request_set_body(mkcurl_request_t *req, const char *b);

/// mkcurl_request_set_body_binary_v2 sets the body as a binary vector.
void mkcurl_request_set_body_binary_v2(mkcurl_request_t *req,
                                       const uint8_t *b, size_t n);

/// mkcurl_request_set_timeout sets the request timeout. This is the time after
/// which the request/response handling loop is interrupted. Setting zero or a
/// negative value tells CURL to disable such timeout. By default, we configure
/// CURL using a small, but reasonable timeout.
void mkcurl_request_set_timeout(mkcurl_request_t *req, int64_t timeout);

/// mkcurl_request_set_proxy_url sets the proxy URL. To use Tor, set
/// this option to `socks5h://127.0.0.1:9050`.
void mkcurl_request_set_proxy_url(mkcurl_request_t *req, const char *u);

/// mkcurl_request_enable_follow_redirect enables following redirects.
void mkcurl_request_enable_follow_redirect(mkcurl_request_t *req);

/// mkcurl_request_delete deletes a request.
void mkcurl_request_delete(mkcurl_request_t *req);

/// mkcurl_response_t is an HTTP response.
typedef struct mkcurl_response mkcurl_response_t;

/// mkcurl_response_get_error returns the error that occurred. Most errors
/// are generated by CURL. Sometimes we borrow CURL errors. Sometimes we may
/// event want to use CURL_LAST to indicate a very unexpected error. Also,
/// remember that, even though you may not see an error here, the request may
/// still have failed because of, e.g. a 500. So, do check the status!
int64_t mkcurl_response_get_error(const mkcurl_response_t *res);

/// mkcurl_response_get_redirect_url returns the URL to which we were
/// redirected, if redirection is enabled. May return NULL on error.
const char *mkcurl_response_get_redirect_url(const mkcurl_response_t *res);

/// mkcurl_response_get_status_code returns the status code. May return
/// unreasonably large numbers in case of severe internal errors.
int64_t mkcurl_response_get_status_code(const mkcurl_response_t *res);

/// mkcurl_response_get_body returns the body as a C string. Since the body
/// may be binary, mkcurl_response_get_body_binary is generally better. It may
/// return NULL in case of internal error.
const char *mkcurl_response_get_body(const mkcurl_response_t *res);

/// mkcurl_response_get_body_v2 returns the body as a binary vector. Returns
/// true on success and false on failure.
int64_t mkcurl_response_get_body_binary_v2(const mkcurl_response_t *res,
                                           const uint8_t **p, size_t *n);

/// mkcurl_response_get_bytes_sent returns the bytes sent. Should be a positive
/// number with the fractional part equal to zero.
double mkcurl_response_get_bytes_sent(const mkcurl_response_t *res);

/// mkcurl_response_get_bytes_recv returns the bytes received. Should be a
/// positive number with the fractional part equal to zero.
double mkcurl_response_get_bytes_recv(const mkcurl_response_t *res);

/// mkcurl_response_get_logs returns the logs produced while performing the
/// request. Logs might contain binary data, so mkcurl_response_get_logs_binary
/// is a safer alternative. Could return NULL on error.
const char *mkcurl_response_get_logs(const mkcurl_response_t *res);

/// mkcurl_response_get_logs_binary_v2 returns the logs a binary vector. Returns
/// true on success and false on failure.
int64_t mkcurl_response_get_logs_binary_v2(const mkcurl_response_t *res,
                                           const uint8_t **p, size_t *n);

/// mkcurl_response_get_request_headers returns the request line and all the
/// headers that were subsequently sent as part of the request. It may return
/// NULL in case of internal errors.
const char *mkcurl_response_get_request_headers(const mkcurl_response_t *res);

/// mkcurl_response_get_response_headers returns the response line and all
/// the headers subsequently sent as part of the response(s). It may return
/// NULL in case of internal errors.
const char *mkcurl_response_get_response_headers(const mkcurl_response_t *res);

/// mkcurl_response_get_response_headers_binary_v2 returns the response line
/// and the headers as a binary vector. Returns true on sucess and false
/// in case of failure.
int64_t mkcurl_response_get_response_headers_binary_v2(
    const mkcurl_response_t *res, const uint8_t **p, size_t *n);

/// mkcurl_response_get_certificate_chain returns the certificate chain as a
/// string. It will look like a config file. There will be commented out lines
/// indicating certificate properties. The only non-commented data will be
/// the lines composing the BASE64 encoded certificate. There should be a
/// empty line between each certificate. It may return NULL on error.
const char *mkcurl_response_get_certificate_chain(const mkcurl_response_t *res);

/// mkcurl_response_delete deletes a response.
void mkcurl_response_delete(mkcurl_response_t *res);

/// mkcurl_perform sends an HTTP request and returns the related response. It
/// may return NULL in case of internal error.
mkcurl_response_t *mkcurl_perform(const mkcurl_request_t *req);

#ifdef __cplusplus
}  // extern "C"

#include <memory>

/// mkcurl_request_deleter is a custom deleter for mkcurl_request_t.
struct mkcurl_request_deleter {
  void operator()(mkcurl_request_t *req) {
    mkcurl_request_delete(req);
  }
};

/// mkcurl_request_uptr is a unique pointer for mkcurl_request_t.
using mkcurl_request_uptr = std::unique_ptr<mkcurl_request_t,
                                            mkcurl_request_deleter>;

/// mkcurl_response_deleter is a custom deleter for mkcurl_response_t.
struct mkcurl_response_deleter {
  void operator()(mkcurl_response_t *req) {
    mkcurl_response_delete(req);
  }
};

/// mkcurl_response_uptr is a unique pointer for mkcurl_response_t.
using mkcurl_response_uptr = std::unique_ptr<mkcurl_response_t,
                                             mkcurl_response_deleter>;

// If you just want to know about the API, you can stop reading here. What
// follows is the inline implementation of the library. By default it is not
// included when you include the header, but you can change this.
#ifdef MKCURL_INLINE_IMPL

#include <assert.h>

#include <sstream>
#include <string>
#include <vector>

#include <curl/curl.h>

struct mkcurl_request {
  std::string ca_path;
  bool enable_http2 = false;
  bool method_post = false;
  std::string url;
  std::vector<std::string> headers;
  std::string body;
  long timeout = 30 /* seconds (same unit and type width as CURL) */;
  std::string proxy_url;
  bool follow_redir = false;
};

mkcurl_request_t *mkcurl_request_new() {
  return new mkcurl_request_t{};
}

void mkcurl_request_set_ca_bundle_path(mkcurl_request_t *req, const char *p) {
  if (req != nullptr && p != nullptr) req->ca_path = p;
}

void mkcurl_request_enable_http2(mkcurl_request_t *req) {
  if (req != nullptr) req->enable_http2 = true;
}

void mkcurl_request_set_method_post(mkcurl_request_t *req) {
  if (req != nullptr) req->method_post = true;
}

void mkcurl_request_set_url(mkcurl_request_t *req, const char *u) {
  if (req != nullptr && u != nullptr) req->url = u;
}

void mkcurl_request_add_header(mkcurl_request_t *req, const char *h) {
  if (req != nullptr && h != nullptr) {
    req->headers.push_back(h);
  }
}

void mkcurl_request_set_body(mkcurl_request_t *req, const char *b) {
  if (req != nullptr && b != nullptr) req->body = b;
}

void mkcurl_request_set_body_binary_v2(mkcurl_request_t *req,
                                       const uint8_t *b, size_t n) {
  if (req != nullptr && b != nullptr && n > 0) {
    req->body = std::string{(char *)b, n};
  }
}

void mkcurl_request_set_timeout(mkcurl_request_t *req, int64_t timeout) {
  if (req != nullptr) {
    req->timeout = (long)((timeout < 0)
                              ? 0  // which, for CURL means infinite
                              : (timeout < INT16_MAX)
                                    ? timeout      // it's in range
                                    : INT16_MAX);  // too many seconds already
  }
}

void mkcurl_request_set_proxy_url(mkcurl_request_t *req, const char *u) {
  if (req != nullptr && u != nullptr) req->proxy_url = u;
}

void mkcurl_request_enable_follow_redirect(mkcurl_request_t *req) {
  if (req != nullptr) req->follow_redir = true;
}

void mkcurl_request_delete(mkcurl_request_t *req) { delete req; }

struct mkcurl_response {
  int64_t error = CURLE_OK;  // In CURL is an enum, hence has int width
  std::string redirect_url;
  int64_t status_code = 0;   // In CURL is a long, hence <= width
  std::string body;
  double bytes_sent = 0.0;
  double bytes_recv = 0.0;
  std::string logs;
  std::string request_headers;
  std::string response_headers;
  std::string certs;
};

int64_t mkcurl_response_get_error(const mkcurl_response_t *res) {
  return (res != nullptr) ? res->error : (int64_t)CURLE_OK;
}

const char *mkcurl_response_get_redirect_url(const mkcurl_response_t *res) {
  return (res != nullptr) ? res->redirect_url.c_str() : "";
}

int64_t mkcurl_response_get_status_code(const mkcurl_response_t *res) {
  return (res != nullptr) ? res->status_code : 200;
}

const char *mkcurl_response_get_body(const mkcurl_response_t *res) {
  return (res != nullptr) ? res->body.c_str() : "";
}

int64_t mkcurl_response_get_body_binary_v2(const mkcurl_response_t *res,
                                           const uint8_t **p, size_t *n) {
  if (res == nullptr || p == nullptr || n == nullptr) return false;
  *p = (const uint8_t *)res->body.c_str();
  *n = res->body.size();
  return true;
}

double mkcurl_response_get_bytes_sent(const mkcurl_response_t *res) {
  return (res != nullptr) ? res->bytes_sent : 0.0;
}

double mkcurl_response_get_bytes_recv(const mkcurl_response_t *res) {
  return (res != nullptr) ? res->bytes_recv : 0.0;
}

const char *mkcurl_response_get_logs(const mkcurl_response_t *res) {
  return (res != nullptr) ? res->logs.c_str() : "";
}

int64_t mkcurl_response_get_logs_binary_v2(const mkcurl_response_t *res,
                                           const uint8_t **p, size_t *n) {
  if (res == nullptr || p == nullptr || n == nullptr) return false;
  *p = (const uint8_t *)res->logs.c_str();
  *n = res->logs.size();
  return true;
}

const char *mkcurl_response_get_request_headers(const mkcurl_response_t *res) {
  return (res != nullptr) ? res->request_headers.c_str() : "";
}

const char *mkcurl_response_get_response_headers(const mkcurl_response_t *res) {
  return (res != nullptr) ? res->response_headers.c_str() : "";
}

int64_t mkcurl_response_get_response_headers_binary_v2(
    const mkcurl_response_t *res, const uint8_t **p, size_t *n) {
  if (res == nullptr || p == nullptr || n == nullptr) return false;
  *p = (const uint8_t *)res->response_headers.c_str();
  *n = res->response_headers.size();
  return true;
}

const char *mkcurl_response_get_certificate_chain(
    const mkcurl_response_t *res) {
  return (res != nullptr) ? res->certs.c_str() : "";
}

void mkcurl_response_delete(mkcurl_response_t *res) { delete res; }

// mkcurl_deleter is a custom deleter for a CURL handle.
struct mkcurl_deleter {
  void operator()(CURL *handle) { curl_easy_cleanup(handle); }
};

// mkcurl_uptr is a unique pointer to a CURL handle.
using mkcurl_uptr = std::unique_ptr<CURL, mkcurl_deleter>;

// mkcurl_slist is a curl_slist with RAII semantic.
struct mkcurl_slist {
  mkcurl_slist() = default;
  mkcurl_slist(const mkcurl_slist &) = delete;
  mkcurl_slist &operator=(const mkcurl_slist &) = delete;
  mkcurl_slist(mkcurl_slist &&) = delete;
  mkcurl_slist &operator=(mkcurl_slist &&) = delete;
  ~mkcurl_slist() { curl_slist_free_all(p); }
  curl_slist *p = nullptr;
};

#ifndef MKCURL_EASY_INIT
// MKCURL_EASY_INIT allows to mock curl_easy_init
#define MKCURL_EASY_INIT curl_easy_init
#endif

#ifndef MKCURL_SLIST_APPEND
// MKCURL_SLIST_APPEND allows to mock curl_slist_append
#define MKCURL_SLIST_APPEND curl_slist_append
#endif

#ifndef MKCURL_EASY_SETOPT
// MKCURL_EASY_SETOPT allows to mock curl_easy_setopt
#define MKCURL_EASY_SETOPT curl_easy_setopt
#endif

#ifndef MKCURL_EASY_PERFORM
// MKCURL_EASY_PERFORM allows to mock curl_easy_perform
#define MKCURL_EASY_PERFORM curl_easy_perform
#endif

#ifndef MKCURL_EASY_GETINFO
// MKCURL_EASY_GETINFO allows to mock curl_easy_getinfo
#define MKCURL_EASY_GETINFO curl_easy_getinfo
#endif

extern "C" {

static size_t mkcurl_body_cb(
    char *ptr, size_t size, size_t nmemb, void *userdata) noexcept {
  if (nmemb <= 0) {
    return 0;  // This means "no body"
  }
  if (size > SIZE_MAX / nmemb) {
    assert(false);  // If size is zero we we end up here
    return 0;
  }
  auto realsiz = size * nmemb;  // Overflow or zero not possible (see above)
  auto res = static_cast<mkcurl_response_t *>(userdata);
  res->body += std::string{ptr, realsiz};
  // From fwrite(3): "[the return value] equals the number of bytes
  // written _only_ when `size` equals `1`". See also
  // https://sourceware.org/git/?p=glibc.git;a=blob;f=libio/iofwrite.c;h=800341b7da546e5b7fd2005c5536f4c90037f50d;hb=HEAD#l29
  return nmemb;
}

static int mkcurl_debug_cb(CURL *handle,
                           curl_infotype type,
                           char *data,
                           size_t size,
                           void *userptr) {
  (void)handle;
  auto res = static_cast<mkcurl_response_t *>(userptr);

  auto log_many_lines = [&](std::string prefix, const std::string &str) {
    std::stringstream ss;
    ss << str;
    std::string line;
    while (std::getline(ss, line, '\n')) {
      if (!prefix.empty()) {
        res->logs += prefix;
        res->logs += " ";
      }
      res->logs += line;
      res->logs += "\n";
    }
  };

  switch (type) {
    case CURLINFO_TEXT:
      log_many_lines("", std::string{(char *)data, size});
      break;
    case CURLINFO_HEADER_IN:
      {
        std::string s{(char *)data, size};
        log_many_lines("<", s);
        res->response_headers += s;
      }
      break;
    case CURLINFO_DATA_IN:
      log_many_lines("<data:", std::to_string(size));
      break;
    case CURLINFO_SSL_DATA_IN:
      log_many_lines("<tls_data:", std::to_string(size));
      break;
    case CURLINFO_HEADER_OUT:
      {
        std::string s{(char *)data, size};
        log_many_lines(">", s);
        res->request_headers += s;
      }
      break;
    case CURLINFO_DATA_OUT:
      log_many_lines(">data:", std::to_string(size));
      break;
    case CURLINFO_SSL_DATA_OUT:
      log_many_lines(">tls_data:", std::to_string(size));
      break;
    case CURLINFO_END:
      /* NOTHING */
      break;
  }

  // Note regarding counting TLS data
  // ````````````````````````````````
  //
  // I am using the technique recommended by Stenberg on Stack Overflow [1]. It
  // was initially not clear to me whether cURL using OpenSSL counted the data
  // twice, once encrypted and once in clear text. However, using cURL using
  // OpenSSL on Linux and reading the source code [2] helped me to clarify that
  // it does indeed the right thing [3]. When using other TLS backends, it may
  // be that TLS data is not counted, but that's okay since we tell to users
  // that this is an estimate of the amount of used data.
  //
  // Notes
  // `````
  //
  // .. [1] https://stackoverflow.com/a/26905099
  //
  // .. [2] https://github.com/curl/curl/blob/6684653b/lib/vtls/openssl.c#L2295
  //
  // .. [3] the SSL function used is SSL_CTX_set_msg_callback which "[is] never
  //        [called for] application_data(23) because the callback will only be
  //        called for protocol messages" [4].
  //
  // .. [4] https://www.openssl.org/docs/man1.1.0/ssl/SSL_CTX_set_msg_callback.html
  switch (type) {
    case CURLINFO_HEADER_IN:
    case CURLINFO_DATA_IN:
    case CURLINFO_SSL_DATA_IN:
      res->bytes_recv += (double)size;
      break;
    case CURLINFO_HEADER_OUT:
    case CURLINFO_DATA_OUT:
    case CURLINFO_SSL_DATA_OUT:
      res->bytes_sent += (double)size;
      break;
    case CURLINFO_TEXT:
    case CURLINFO_END:
      /* NOTHING */
      break;
  }

  return 0;
}

}  // extern "C"

// TODO(bassosimone):
//
// 1. Allow to disable CURLOPT_SSL_VERIFYPEER
//
// 2. Allow to disable CURLOPT_SSL_VERIFYHOST
//
// 3. Allow to set a specific SSL version with CURLOPT_SSLVERSION
mkcurl_response_t *mkcurl_perform(const mkcurl_request_t *req) {
  if (req == nullptr) return nullptr;
  mkcurl_response_uptr res{new mkcurl_response_t{}};
  mkcurl_uptr handle{MKCURL_EASY_INIT()};
  if (!handle) {
    res->error = CURLE_OUT_OF_MEMORY;
    res->logs += "curl_easy_init() failed\n";
    return res.release();
  }
  mkcurl_slist headers;
  for (auto &s : req->headers) {
    if ((headers.p = MKCURL_SLIST_APPEND(headers.p, s.c_str())) == nullptr) {
      res->error = CURLE_OUT_OF_MEMORY;
      res->logs += "curl_slist_append() failed\n";
      return res.release();
    }
  }
  if (!req->ca_path.empty() &&
      (res->error = MKCURL_EASY_SETOPT(handle.get(), CURLOPT_CAINFO,
                                       req->ca_path.c_str())) != CURLE_OK) {
    res->logs += "curl_easy_setopt(CURLOPT_CAINFO) failed\n";
    return res.release();
  }
  if (req->enable_http2 == true &&
      (res->error = MKCURL_EASY_SETOPT(handle.get(), CURLOPT_HTTP_VERSION,
                                       CURL_HTTP_VERSION_2_0)) != CURLE_OK) {
    res->logs += "curl_easy_setopt(CURLOPT_HTTP_VERSION) failed\n";
    return res.release();
  }
  if (headers.p != nullptr &&
      (res->error = MKCURL_EASY_SETOPT(
           handle.get(), CURLOPT_HTTPHEADER, headers.p)) != CURLE_OK) {
    res->logs += "curl_easy_setopt(CURLOPT_HTTPHEADER) failed\n";
    return res.release();
  }
  if (!req->body.empty() && req->method_post == true &&
      (res->error = MKCURL_EASY_SETOPT(handle.get(), CURLOPT_POSTFIELDS,
                                       req->body.c_str())) != CURLE_OK) {
    res->logs += "curl_easy_setopt(CURLOPT_POSTFIELDS) failed\n";
    return res.release();
  }
  if (req->method_post == true &&
      (res->error = MKCURL_EASY_SETOPT(handle.get(), CURLOPT_POST,
                                       1L)) != CURLE_OK) {
    res->logs += "curl_easy_setopt(CURLOPT_POST) failed\n";
    return res.release();
  }
  if ((res->error = MKCURL_EASY_SETOPT(handle.get(), CURLOPT_URL,
                                       req->url.c_str())) != CURLE_OK) {
    res->logs += "curl_easy_setopt(CURLOPT_URL) failed\n";
    return res.release();
  }
  if ((res->error = MKCURL_EASY_SETOPT(handle.get(), CURLOPT_WRITEFUNCTION,
                                       mkcurl_body_cb)) != CURLE_OK) {
    res->logs += "curl_easy_setopt(CURLOPT_WRITEFUNCTION) failed\n";
    return res.release();
  }
  if ((res->error = MKCURL_EASY_SETOPT(handle.get(), CURLOPT_WRITEDATA,
                                       res.get())) != CURLE_OK) {
    res->logs += "curl_easy_setopt(CURLOPT_WRITEDATA) failed\n";
    return res.release();
  }
  if (req->timeout >= 0 &&
      (res->error = MKCURL_EASY_SETOPT(handle.get(), CURLOPT_TIMEOUT,
                                       req->timeout)) != CURLE_OK) {
    res->logs += "curl_easy_setopt(CURLOPT_TIMEOUT) failed\n";
    return res.release();
  }
  if ((res->error = MKCURL_EASY_SETOPT(handle.get(), CURLOPT_DEBUGFUNCTION,
                                       mkcurl_debug_cb)) != CURLE_OK) {
    res->logs += "curl_easy_setopt(CURLOPT_DEBUGFUNCTION) failed\n";
    return res.release();
  }
  if ((res->error = MKCURL_EASY_SETOPT(handle.get(), CURLOPT_DEBUGDATA,
                                       res.get())) != CURLE_OK) {
    res->logs += "curl_easy_setopt(CURLOPT_DEBUGDATA) failed\n";
    return res.release();
  }
  if ((res->error = MKCURL_EASY_SETOPT(handle.get(), CURLOPT_VERBOSE,
                                       1L)) != CURLE_OK) {
    res->logs += "curl_easy_setopt(CURLOPT_VERBOSE) failed\n";
    return res.release();
  }
  if (!req->proxy_url.empty() &&
      (res->error = MKCURL_EASY_SETOPT(handle.get(), CURLOPT_PROXY,
                                       req->proxy_url.c_str())) != CURLE_OK) {
    res->logs += "curl_easy_setopt(CURLOPT_PROXY) failed\n";
    return res.release();
  }
  if (req->follow_redir == true &&
      (res->error = MKCURL_EASY_SETOPT(handle.get(), CURLOPT_FOLLOWLOCATION,
                                       1L)) != CURLE_OK) {
    res->logs += "curl_easy_setopt(CURLOPT_FOLLOWLOCATION) failed\n";
    return res.release();
  }
  if ((res->error = MKCURL_EASY_SETOPT(handle.get(), CURLOPT_CERTINFO,
                                       1L)) != CURLE_OK) {
    res->logs += "curl_easy_setopt(CURLOPT_CERTINFO) failed\n";
    return res.release();
  }
  if ((res->error = MKCURL_EASY_PERFORM(handle.get())) != CURLE_OK) {
    res->logs += "curl_easy_perform() failed\n";
    return res.release();
  }
  {
    long status_code = 0;
    if ((res->error = MKCURL_EASY_GETINFO(
             handle.get(), CURLINFO_RESPONSE_CODE, &status_code)) != CURLE_OK) {
      res->logs += "curl_easy_getinfo(CURLINFO_RESPONSE_CODE) failed\n";
      return res.release();
    }
    res->status_code = (int64_t)status_code;
  }
  {
    char *url = nullptr;
    if ((res->error = MKCURL_EASY_GETINFO(
             handle.get(), CURLINFO_REDIRECT_URL, &url)) != CURLE_OK) {
      res->logs += "curl_easy_getinfo(CURLINFO_REDIRECT_URL) failed\n";
      return res.release();
    }
    if (url != nullptr) res->redirect_url = url;
  }
  {
    curl_certinfo *certinfo = nullptr;
    if ((res->error = MKCURL_EASY_GETINFO(
             handle.get(), CURLINFO_CERTINFO, &certinfo)) != CURLE_OK) {
      res->logs += "curl_easy_getinfo(CURLINFO_CERTINFO) failed\n";
      return res.release();
    }
    if (certinfo != nullptr && certinfo->num_of_certs > 0) {
      for (int i = 0; i < certinfo->num_of_certs; i++) {
        for (auto slist = certinfo->certinfo[i]; slist; slist = slist->next) {
          if (slist->data != nullptr) {
            // This is a linked list with "key:value" strings. We change the
            // formar slightly so that parsing is easier.
            std::string s = slist->data;
            if (s.find("Cert:") == 0) {
              res->certs += s.substr(5);
            } else {
              res->certs += "# ";
              res->certs += s;
            }
            res->certs += "\n";
          }
        }
      }
    }
  }
  res->logs += "curl_easy_perform() success\n";
  return res.release();
}

#endif  // MKCURL_INLINE_IMPL
#endif  // __cplusplus
#endif  // MEASUREMENT_KIT_MKCURL_H
