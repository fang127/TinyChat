cmake_minimum_required(VERSION 3.10)

project(GateServer)

set(CMAKE_CXX_STANDARD 20)

include_directories(${CMAKE_SOURCE_DIR}/include)

file(GLOB_RECURSE SOURCE_FILES "src/*.cc")

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -Wall -ggdb")

link_directories(${CMAKE_SOURCE_DIR}/lib)

# 查找并链接 jsoncpp （如果系统上安装了 jsoncpp）
# 尝试查找 JsonCpp 的 CMake 包，如果没有找到则退回到直接查找系统库
find_package(JsonCpp QUIET)
if (JsonCpp_FOUND)
	message(STATUS "Found JsonCpp via CMake package")
	add_library(GateServer SHARED ${SOURCE_FILES})
	target_link_libraries(GateServer PUBLIC JsonCpp::JsonCpp)
else()
	message(STATUS "JsonCpp CMake package not found, falling back to find_library for libjsoncpp")
	find_library(JSONCPP_LIB jsoncpp)
	if (JSONCPP_LIB)
		add_library(GateServer SHARED ${SOURCE_FILES})
		target_link_libraries(GateServer PUBLIC ${JSONCPP_LIB})
	else()
		message(FATAL_ERROR "Could not find JsonCpp library. Install libjsoncpp-dev or provide JsonCppConfig.cmake")
	endif()
endif()

add_executable(GateServerTest src/GateServer.cc)

target_link_libraries(GateServerTest PRIVATE GateServer)