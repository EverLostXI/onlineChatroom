cmake_minimum_required(VERSION 3.10)
project(Chat LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(IMGUI_SOURCES
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/backends/imgui_impl_win32.cpp
    imgui/backends/imgui_impl_dx11.cpp
)



# 添加所有源文件
add_executable(server 
    main.cpp
    logger.cpp
    socket.cpp
    userControl.cpp
    handleClient.cpp
    ${IMGUI_SOURCES}
)

target_include_directories(server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends
)

# 链接一堆库
target_link_libraries(server
    ws2_32       # WinSock
    d3d11        # DirectX 11
    d3dcompiler  # DirectX 编译器
    dxgi         # DirectX 图形接口
    dwmapi       # Desktop Window Manager API
    imm32       # Input Method Manager
)