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
  set(_CURL_BASEURL "https://github.com/measurement-kit/prebuilt/releases/download")
  set(_CURL_CHANNEL "testing")
  set(_CURL_HASH "c4321f05ac238d42f2487536c4fdd8e55af68a3f364008271fe3b6741c6b855f")
  set(_CURL_VERSION "7.61.0-1")
  set(_CURL_URL "${_CURL_BASEURL}/${_CURL_CHANNEL}/windows-curl-${_CURL_VERSION}.tar.gz")
  message(STATUS "Downloading precompiled cURL: ${_CURL_URL}")
  file(DOWNLOAD "${_CURL_URL}"
       "${CMAKE_CURRENT_BINARY_DIR}/curl-prebuilt.tar.gz"
       EXPECTED_HASH SHA256=${_CURL_HASH}
       TLS_VERIFY ON SHOW_PROGRESS)
  execute_process(COMMAND ${CMAKE_COMMAND} -E tar xf "curl-prebuilt.tar.gz"
                  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
                  RESULT_VARIABLE TAR_FAILURE)
  if("${TAR_FAILURE}")
    message(FATAL_ERROR "Cannot unpack cURL: ${TAR_FAILURE}")
  endif()
  if("${CMAKE_SIZEOF_VOID_P}" EQUAL 8)
    set(_CURL_ARCH x64)
  elseif("${CMAKE_SIZEOF_VOID_P}" EQUAL 4)
    set(_CURL_ARCH x86)
  else()
    message(FATAL_ERROR "No prebuilt cURL for your system")
  endif()
  list(APPEND CMAKE_INCLUDE_PATH "${CMAKE_BINARY_DIR}/MK_DIST/windows/curl/${_CURL_VERSION}/${_CURL_ARCH}/include")
  list(APPEND CMAKE_LIBRARY_PATH "${CMAKE_BINARY_DIR}/MK_DIST/windows/curl/${_CURL_VERSION}/${_CURL_ARCH}/lib")
  message(STATUS "CMAKE_INCLUDE_PATH: ${CMAKE_INCLUDE_PATH}")
  message(STATUS "CMAKE_LIBRARY_PATH: ${CMAKE_LIBRARY_PATH}")
  add_definitions(-DCURL_STATICLIB)
  list(APPEND LIBCURLX_LIBS Crypt32)
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
