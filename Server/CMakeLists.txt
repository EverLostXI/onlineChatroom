cmake_minimum_required(VERSION 3.10)
project(Chat LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加所有源文件
add_executable(server 
    main.cpp
    logger.cpp
    socket.cpp
    userControl.cpp
    handleClient.cpp
)

# 链接winsock库
target_link_libraries(
    server
    ws2_32
)