cmake_minimum_required(VERSION 3.16)
project ("xls_search")

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED True)

add_executable (xls_search 
	"xls_search.cpp" 
	"libxls/src/endian.c"
	"libxls/src/locale.c"
	"libxls/src/ole.c"
	"libxls/src/xls.c"	 
	"libxls/src/xlstool.c"
)
target_include_directories(xls_search PUBLIC "libxls/include")
target_include_directories(xls_search PUBLIC ".")