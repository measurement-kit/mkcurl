#include <iostream>

#define MKCURL_INLINE_IMPL
#include "mkcurl.h"

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
