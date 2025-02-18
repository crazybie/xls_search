cmake_minimum_required(VERSION 3.29)
project ("xls_search")

set (CMAKE_CXX_STANDARD 20)


FILE(GLOB_RECURSE LIBXLS_SOURCES "libxls/src/*.c")
list(FILTER LIBXLS_SOURCES EXCLUDE REGEX ".*xls2csv\.c")
add_library(libxls ${LIBXLS_SOURCES})
target_include_directories(libxls PUBLIC ".")


FILE(GLOB APP_SOURCES "*.cpp")
add_executable(xls_search ${APP_SOURCES})
target_include_directories(xls_search PUBLIC "libxls/include")
target_precompile_headers(xls_search  PUBLIC "precompile.h")
target_link_libraries(xls_search libxls)