cmake_minimum_required(VERSION 3.1.0)
project(libcurlx LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules")
include(MkUtils)

# Download dependencies
# ---------------------

MkDownloadAdishavitArgh()
MkDownloadCatchorgCatch2()
MkDownloadCaBundle()

if("${MSVC}")
  MkDownloadMeasurementKitPrebuiltWindowsCurl()
  list(APPEND CMAKE_INCLUDE_PATH "${MK_WINDOWS_CURL_INCLUDE_PATH}")
  list(APPEND CMAKE_LIBRARY_PATH "${MK_WINDOWS_CURL_LIBRARY_PATH}")
  add_definitions(${MK_WINDOWS_CURL_DEFINITIONS})
  list(APPEND LIBCURLX_LIBS ${MK_WINDOWS_CURL_EXTRA_LINK_LIBS})
endif()

# Checks
# ------

find_package(CURL REQUIRED)

# Compiler flags
# --------------

MkSetCompilerFlags()

# Library and binary
# ------------------

set(LIBCURLX_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
    ${CURL_INCLUDE_DIRS} ${CMAKE_REQUIRED_INCLUDES})
include_directories(${LIBCURLX_INCLUDES})

list (APPEND LIBCURLX_LIBS ${CURL_LIBRARIES})
if("${WIN32}" OR "${MINGW}")
  list(APPEND LIBCURLX_LIBS "ws2_32")
  if ("${MINGW}")
      list(APPEND LIBCURLX_LIBS -static-libgcc -static-libstdc++)
  endif()
endif()
list(APPEND LIBCURLX_LIBS Threads::Threads)
link_libraries("${LIBCURLX_LIBS}")

add_executable(libcurlx-client libcurlx-client.cpp)
install(FILES libcurlx.h DESTINATION include/measurement-kit/libcurlx)
install(TARGETS libcurlx-client DESTINATION bin)

# Testing
# -------

set(BUILD_TESTING "ON" CACHE BOOL "Whether to build tests")
if(${BUILD_TESTING})
  add_executable(unit-tests unit-tests.cpp)
  enable_testing()
  add_test(NAME unit_tests COMMAND unit-tests)
  add_test(NAME external_ca COMMAND
           libcurlx-client --ca-bundle-path ./ca-bundle.pem
           https://www.kernel.org)
  add_test(NAME http11_test COMMAND
           libcurlx-client https://ooni.torproject.org)
  # TODO(bassosimone): compile cURL for Windows with http2 support
  if(NOT "${WIN32}")
    add_test(NAME http2_test COMMAND
             libcurlx-client --enable-http2 https://www.google.com)
  endif()
  add_test(NAME redirect_test COMMAND
           libcurlx-client --follow-redirect http://google.com)
  add_test(NAME post COMMAND
           libcurlx-client --post --header "Content-Type: application/json"
           --post-data "{\"net-tests\":[{\"input-hashes\":null,\"name\":\"http_invalid_request_line\",\"test-helpers\":[\"tcp-echo\"],\"version\":\"0.0.3\"}]}"
           https://bouncer.ooni.io/bouncer/net-tests)
endif()
