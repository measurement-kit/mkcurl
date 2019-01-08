cmake_minimum_required(VERSION 3.1.0)
project(mkcurl LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules")
include(MkUtils)

# Download dependencies
# ---------------------

MkDownloadAdishavitArgh()
MkDownloadCatchorgCatch2()
MkDownloadCaBundle()
MkDownloadMeasurementKitMkMock()

if("${MSVC}")
  MkDownloadMeasurementKitPrebuiltWindowsCurl()
  list(APPEND CMAKE_INCLUDE_PATH "${MK_WINDOWS_CURL_INCLUDE_PATH}")
  list(APPEND CMAKE_LIBRARY_PATH "${MK_WINDOWS_CURL_LIBRARY_PATH}")
  add_definitions(${MK_WINDOWS_CURL_DEFINITIONS})
  list(APPEND MKCURL_LIBS ${MK_WINDOWS_CURL_EXTRA_LINK_LIBS})
endif()

# Checks
# ------

find_package(CURL REQUIRED)

# Compiler flags
# --------------

MkSetCompilerFlags()

# Library and binary
# ------------------

set(MKCURL_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
    ${CURL_INCLUDE_DIRS} ${CMAKE_REQUIRED_INCLUDES})
include_directories(${MKCURL_INCLUDES})

list (APPEND MKCURL_LIBS ${CURL_LIBRARIES})
if("${WIN32}" OR "${MINGW}")
  list(APPEND MKCURL_LIBS "ws2_32")
  if ("${MINGW}")
      list(APPEND MKCURL_LIBS -static-libgcc -static-libstdc++)
  endif()
endif()
list(APPEND MKCURL_LIBS Threads::Threads)
link_libraries("${MKCURL_LIBS}")

add_library(mkcurl mkcurl.cpp)
add_executable(mkcurl-client mkcurl-client.cpp)
target_link_libraries(mkcurl-client mkcurl)

# Testing
# -------

set(BUILD_TESTING "ON" CACHE BOOL "Whether to build tests")
if(${BUILD_TESTING})
  add_executable(tests tests.cpp)
  if("${WIN32}")
    target_compile_options(tests PRIVATE /EHs)  # exceptions in extern "C"
  endif()
  enable_testing()
  add_test(NAME mocked_tests COMMAND tests)
  add_test(NAME external_ca COMMAND
           mkcurl-client --ca-bundle-path ./ca-bundle.pem
           https://www.kernel.org)
  add_test(NAME http11_test COMMAND
           mkcurl-client https://ooni.torproject.org)
  add_test(NAME using_timeout COMMAND
           mkcurl-client --timeout 10 --follow-redirect
           https://www.facebook.com)
  # TODO(bassosimone): compile cURL for Windows with http2 support
  if(NOT "${WIN32}")
    add_test(NAME http2_test COMMAND
             mkcurl-client --enable-http2 https://www.google.com)
  endif()
  add_test(NAME redirect_test COMMAND
           mkcurl-client --follow-redirect http://google.com)
  add_test(NAME post COMMAND
           mkcurl-client --post --header "Content-Type: application/json"
           --data "{\"net-tests\":[{\"input-hashes\":null,\"name\":\"http_invalid_request_line\",\"test-helpers\":[\"tcp-echo\"],\"version\":\"0.0.3\"}]}"
           https://bouncer.ooni.io/bouncer/net-tests)
  add_test(NAME put COMMAND
           mkcurl-client --put --header "Accept: application/json"
           https://httpbin.org/put)
  add_test(NAME connect-to COMMAND
           mkcurl-client --connect-to www.google.com https://www.youtube.com)
  if(NOT "${WIN32}")
    add_test(NAME tcp-fastopen COMMAND
             mkcurl-client --enable-tcp-fastopen https://www.kernel.org)
  endif()
endif()
