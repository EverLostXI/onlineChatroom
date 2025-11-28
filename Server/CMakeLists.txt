cmake_minimum_required(VERSION 3.10)
project(Chat LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


add_executable(server server.cpp)


# 链接winsock库
target_link_libraries(
    server
    ws2_32
)