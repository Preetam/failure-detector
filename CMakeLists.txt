cmake_minimum_required(VERSION 2.8)

project(failure-detector)
SET(CMAKE_CXX_FLAGS "-std=c++14")
include_directories(
	include
	src
	dep/libuv/include
)

add_subdirectory(dep/glog)
add_subdirectory(dep/libuv)

add_executable(failure-detector
	src/message/message.cc
	src/message_queue/message_queue.cc
	src/node/node.cc
	src/main.cc
)

target_link_libraries(failure-detector glog)
target_link_libraries(failure-detector uv)
