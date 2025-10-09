cmake_minimum_required(VERSION 3.10)

project(GateServer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

# 为本地调试版本设置默认构建类型
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# 添加编译选项
add_definitions(-D_GLIBCXX_USE_CXX11_ABI=1)
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -Wall -Wextra")

# 添加头文件目录
include_directories(${CMAKE_SOURCE_DIR}/include)

# 收集源文件
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS 
    "src/*.cc"
)

# 查找依赖
find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)
find_package(Threads REQUIRED)
find_package(Boost COMPONENTS filesystem REQUIRED)
find_package(JsonCpp QUIET) # 尝试查找JsonCpp包，但不是必须的

message(STATUS "Protobuf include: ${Protobuf_INCLUDE_DIRS}")
message(STATUS "Protobuf libs: ${Protobuf_LIBRARIES}")
message(STATUS "gRPC include: ${gRPC_INCLUDE_DIRS}")
message(STATUS "gRPC libs: ${gRPC_LIBRARIES}")

# 创建一个静态库来包含所有业务逻辑
add_library(GateServerLib STATIC ${SOURCE_FILES})

# 为静态库链接依赖
target_include_directories(GateServerLib PRIVATE 
    ${CMAKE_SOURCE_DIR}/include 
    ${Protobuf_INCLUDE_DIRS}
    ${gRPC_INCLUDE_DIRS}
)

target_link_libraries(GateServerLib PRIVATE 
    ${Protobuf_LIBRARIES} 
    ${gRPC_LIBRARIES}
    gRPC::grpc++
    Threads::Threads 
    ${Boost_LIBRARIES}
)

# 处理 JsonCpp 链接
if(JsonCpp_FOUND)
  message(STATUS "Found JsonCpp via CMake package")
  target_link_libraries(GateServerLib PRIVATE JsonCpp::JsonCpp)
else()
  # 如果没有找到CMake包，则尝试查找库文件
  find_library(JSONCPP_LIB jsoncpp)
  if(JSONCPP_LIB)
    message(STATUS "Found libjsoncpp library: ${JSONCPP_LIB}")
    target_link_libraries(GateServerLib PRIVATE ${JSONCPP_LIB})
  else()
    message(WARNING "JsonCpp library not found. If your project requires it, please install libjsoncpp-dev.")
  endif()
endif()


add_executable(GateServerTest src/GateServer.cc)
target_link_libraries(GateServerTest PRIVATE GateServerLib)