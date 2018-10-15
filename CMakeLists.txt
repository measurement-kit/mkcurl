cmake_minimum_required(VERSION 3.1.0)
project(libcurlx LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules")
include(MkUtils)

MkDownloadAdishavitArgh()
MkDownloadCatchorgCatch2()
MkDownloadCaBundle()

find_package(CURL REQUIRED)

# Compiler flags
# --------------

MkSetCompilerFlags()

# Library and binary
# ------------------

set(LIBCURLX_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
    ${CURL_INCLUDE_DIRS} ${CMAKE_REQUIRED_INCLUDES})

add_executable(libcurlx-client libcurlx-client.cpp)
target_include_directories(libcurlx-client PUBLIC ${LIBCURLX_INCLUDES})
list (APPEND LIBCURLX_LIBS ${CURL_LIBRARIES})
install(FILES libcurlx.hpp DESTINATION include/measurement-kit/libcurlx)
install(TARGETS libcurlx-client DESTINATION bin)
if("${WIN32}" OR "${MINGW}")
  list(APPEND LIBCURLX_LIBS "ws2_32")
  if ("${MINGW}")
      list(APPEND LIBCURLX_LIBS -static-libgcc -static-libstdc++)
  endif()
endif()
list(APPEND LIBCURLX_LIBS Threads::Threads)
target_link_libraries(libcurlx-client "${LIBCURLX_LIBS}")

# Testing
# -------

set(BUILD_TESTING "ON" CACHE BOOL "Whether to build tests")
if(${BUILD_TESTING})
  enable_testing()
  add_test(NAME external_ca COMMAND
           libcurlx-client --ca-bundle-path ./ca-bundle.pem
           https://www.kernel.org)
  add_test(NAME http11_test COMMAND
           libcurlx-client https://ooni.torproject.org)
  add_test(NAME http2_test COMMAND
           libcurlx-client --enable-http2 https://www.google.com)
  add_test(NAME redirect_test COMMAND
           libcurlx-client --follow-redirect http://google.com)
  add_test(NAME post COMMAND
           libcurlx-client --post --header "Content-Type: application/json"
           --post-data "{\"net-tests\":[{\"input-hashes\":null,\"name\":\"http_invalid_request_line\",\"test-helpers\":[\"tcp-echo\"],\"version\":\"0.0.3\"}]}"
           https://bouncer.ooni.io/bouncer/net-tests)
endif()
