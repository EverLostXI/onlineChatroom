#include "headers/Monitor.h"
#include "headers/logger.h"
#include "headers/userControl.h"
#include "headers/handleClient.h"
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <d3d11.h>
#include <tchar.h>
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <chrono>

// 外部声明（来自其他模块）
extern std::mutex g_sessionMutex;
extern std::mutex g_forwardMsgMutex;
extern std::mutex g_requestMsgMutex;
extern std::mutex g_uiLogMutex;
extern std::map<uint8_t, std::string> g_userName;

// DirectX 11设备
static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

// 用户UI状态结构（与g_userSessions解耦）
struct UserUIState {
    uint8_t userID;
    bool isOnline;
    std::string userName;  // 用户名
    std::string clientIP;
    unsigned short clientPort;
    std::chrono::steady_clock::time_point lastHeartbeat;
};

// UI专用数据（不需要锁保护，仅UI线程访问）
static std::vector<UserUIState> g_uiUserStates;
static std::map<std::string, std::vector<uint8_t>> g_uiGroupStates;

// 窗口过程函数声明
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// DirectX初始化和清理函数
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();

// UI状态变量
static int g_selectedUserId = -1;  // 被选中的用户ID（用于显示详细信息）
static auto g_lastHeartbeatTime = std::chrono::steady_clock::now();

// 绘制各个监视框
void DrawUserListPanel();
void DrawGroupListPanel();
void DrawServerLogPanel();
void DrawForwardMessagesPanel();
void DrawRequestMessagesPanel();

// UI主函数
void RunMonitorUI()
{
    // 创建窗口
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ChatServerMonitor", nullptr };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"Server Monitor 1.0", WS_OVERLAPPEDWINDOW, 
                                100, 100, 1280, 720, nullptr, nullptr, wc.hInstance, nullptr);

    // 初始化DirectX 11
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return;
    }

    // 显示窗口
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // 初始化ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // 加载中文字体（微软雅黑）
    ImFont* font = io.Fonts->AddFontFromFileTTF("c:/windows/fonts/msyh.ttc", 18.0f, NULL, io.Fonts->GetGlyphRangesChineseFull());
    if (font == NULL) {
        // 备用方案：尝试使用黑体
        font = io.Fonts->AddFontFromFileTTF("c:/windows/fonts/simhei.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesChineseFull());
    }
    if (font == NULL) {
        // 再备用：使用宋体
        io.Fonts->AddFontFromFileTTF("c:/windows/fonts/simsun.ttc", 18.0f, NULL, io.Fonts->GetGlyphRangesChineseFull());
    }

    // 设置ImGui样式
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 0.0f;
    style.FrameRounding = 3.0f;
    style.ScrollbarRounding = 3.0f;
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.02f, 0.02f, 0.02f, 1.0f);  // 更黑的背景
    style.Colors[ImGuiCol_ChildBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.0f);   // 子窗口稍浅
    style.Colors[ImGuiCol_Border] = ImVec4(1.0f, 1.0f, 1.0f, 0.5f);      // 白色边框

    // 初始化ImGui后端
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    // 主循环
    bool running = true;
    while (running)
    {
        // 处理窗口消息
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                running = false;
        }

        if (!running)
            break;

        // 开始ImGui帧
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // 创建全屏主窗口（固定大小，不可调整）
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::Begin("MainMonitor", nullptr, 
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | 
                     ImGuiWindowFlags_NoBringToFrontOnFocus);

        // 获取可用区域
        ImVec2 windowSize = ImGui::GetContentRegionAvail();
        float leftWidth = windowSize.x * 0.3f;   // 左侧30%
        float rightWidth = windowSize.x * 0.7f;  // 右侧70%
        float spacing = 10.0f;

        // 左侧区域（用户列表 + 群聊列表）
        ImGui::BeginGroup();
        {
            float leftUpperHeight = windowSize.y * 0.7f;  // 上70% - 用户列表
            float leftLowerHeight = windowSize.y * 0.3f - spacing;  // 下30% - 群聊列表

            // 用户列表框
            ImGui::BeginChild("UserListPanel", ImVec2(leftWidth, leftUpperHeight), true, ImGuiWindowFlags_HorizontalScrollbar);
            DrawUserListPanel();
            ImGui::EndChild();

            ImGui::Spacing();

            // 群聊列表框
            ImGui::BeginChild("GroupListPanel", ImVec2(leftWidth, leftLowerHeight), true, ImGuiWindowFlags_HorizontalScrollbar);
            DrawGroupListPanel();
            ImGui::EndChild();
        }
        ImGui::EndGroup();

        ImGui::SameLine();

        // 右侧区域（日志 + 转发消息 + 请求消息）- 去掉外围大框
        ImGui::BeginGroup();
        {
            float rightUpperHeight = windowSize.y * 0.4f;  // 上40%
            float rightLowerHeight = windowSize.y * 0.6f - spacing;  // 下60%
            float rightLowerHalfWidth = (rightWidth - spacing) * 0.5f;

            // 服务器日志框
            ImGui::BeginChild("ServerLogPanel", ImVec2(rightWidth, rightUpperHeight), true, ImGuiWindowFlags_HorizontalScrollbar);
            DrawServerLogPanel();
            ImGui::EndChild();

            ImGui::Spacing();

            // 转发消息框
            ImGui::BeginChild("ForwardMessagesPanel", ImVec2(rightLowerHalfWidth, rightLowerHeight), true, ImGuiWindowFlags_HorizontalScrollbar);
            DrawForwardMessagesPanel();
            ImGui::EndChild();

            ImGui::SameLine();

            // 请求消息框
            ImGui::BeginChild("RequestMessagesPanel", ImVec2(rightLowerHalfWidth, rightLowerHeight), true, ImGuiWindowFlags_HorizontalScrollbar);
            DrawRequestMessagesPanel();
            ImGui::EndChild();
        }
        ImGui::EndGroup();

        ImGui::End();

        // 渲染
        ImGui::Render();
        const float clear_color[4] = { 0.02f, 0.02f, 0.02f, 1.0f };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        g_pSwapChain->Present(1, 0);  // 垂直同步
    }

    // 清理
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
}

// 绘制用户列表面板
void DrawUserListPanel()
{
    ImGui::Text("用户");
    ImGui::Separator();

    // 从全局数据同步到UI状态（最小化持锁时间）
    {
        std::lock_guard<std::mutex> lock(g_sessionMutex);
        
        // 更新UI状态列表
        g_uiUserStates.clear();
        for (const auto& pair : g_userSessions) {
            UserUIState state;
            state.userID = pair.first;
            state.isOnline = (pair.second != nullptr);
            
            // 读取用户名（带锁保护）
            if (g_userName.count(pair.first)) {
                state.userName = g_userName[pair.first];
            } else {
                state.userName = "Unknown";
            }
            
            if (state.isOnline) {
                state.clientIP = pair.second->client_ip;
                state.clientPort = pair.second->client_port;
                state.lastHeartbeat = pair.second->lastHeartbeatTime;
            } else {
                state.clientIP = "";
                state.clientPort = 0;
                state.lastHeartbeat = std::chrono::steady_clock::now();
            }
            g_uiUserStates.push_back(state);
        }
    }

    auto now = std::chrono::steady_clock::now();

    // 使用UI本地数据绘制（不持有任何锁）
    for (const auto& user : g_uiUserStates)
    {
        uint8_t userID = user.userID;
        bool isOnline = user.isOnline;
        
        // 计算心跳闪烁效果（收到心跳后300ms内亮起）
        bool shouldBlink = false;
        if (isOnline) {
            auto timeSinceHeartbeat = std::chrono::duration_cast<std::chrono::milliseconds>(
                now - user.lastHeartbeat).count();
            shouldBlink = (timeSinceHeartbeat < 300);  // 心跳后300ms内闪烁
        }
        
        // 添加上下间距
        ImGui::Spacing();
        
        // 添加左侧缩进实现对齐
        ImGui::Indent(5.0f);
        
        // 根据在线状态设置颜色
        ImVec4 textColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);  // 默认白色
        if (isOnline) {
            if (shouldBlink) {
                // 心跳闪烁时使用高亮颜色
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 1.0f, 0.3f, 1.0f));  // 鲜绿色高亮
            } else {
                ImGui::PushStyleColor(ImGuiCol_Text, textColor);
            }
        } else {
            // 离线用户灰色
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
        }
        
        // 显示用户名和状态
        ImGui::Text("%s [%s]", user.userName.c_str(), isOnline ? "在线" : "离线");
        
        ImGui::PopStyleColor();
        ImGui::Unindent(5.0f);

        // 鼠标悬停显示详细信息
        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            ImGui::Text("用户名: %s", user.userName.c_str());
            ImGui::Text("用户ID: %d", userID);
            if (isOnline) {
                ImGui::Text("IP地址: %s", user.clientIP.c_str());
                ImGui::Text("端口: %d", user.clientPort);
                ImGui::Text("状态: 在线");
            } else {
                ImGui::Text("状态: 离线");
            }
            ImGui::EndTooltip();
        }

        if (ImGui::IsItemClicked())
        {
            g_selectedUserId = userID;
        }
    }
}

// 绘制群聊列表面板
void DrawGroupListPanel()
{
    ImGui::Text("群聊");
    ImGui::Separator();

    // 从全局数据同步到UI状态（最小化持锁时间）
    {
        std::lock_guard<std::mutex> lock(g_sessionMutex);
        g_uiGroupStates = g_groupChat;
    }

    if (g_uiGroupStates.empty())
    {
        ImGui::TextDisabled("");
    }
    else
    {
        for (const auto& pair : g_uiGroupStates)
        {
            const std::string& groupName = pair.first;
            const auto& members = pair.second;

            ImGui::BulletText("%s (%zu人)", groupName.c_str(), members.size());
            
            // 鼠标悬停显示成员列表
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::Text("群聊名称: %s", groupName.c_str());
                ImGui::Text("成员数量: %zu", members.size());
                ImGui::Separator();
                for (int memberId : members)
                {
                    ImGui::Text("  - 用户ID: %d", memberId);
                }
                ImGui::EndTooltip();
            }
        }
    }
}

// 绘制服务器日志面板
void DrawServerLogPanel()
{
    ImGui::Text("日志");
    ImGui::Separator();

    // 创建可滚动的子窗口
    ImGui::BeginChild("ServerLogScrollArea", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
    
    // 复制日志数据到本地（最小化持锁时间）
    std::vector<std::string> localLogBuffer;
    {
        std::lock_guard<std::mutex> lock(g_uiLogMutex);
        localLogBuffer = g_uiLogBuffer;
    }
    
    // 显示日志（自动滚动）
    
    for (const std::string& logLine : localLogBuffer)
    {
        // 根据日志类型着色
        ImVec4 color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);  // 默认白色
        if (logLine.find("[FATAL]") != std::string::npos)
            color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);  // 红色
        else if (logLine.find("[WARN]") != std::string::npos)
            color = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);  // 黄色

        ImGui::TextColored(color, "%s", logLine.c_str());
    }

    // 自动滚动到底部
    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        ImGui::SetScrollHereY(1.0f);
    
    ImGui::EndChild();
}

// 绘制转发消息面板
void DrawForwardMessagesPanel()
{
    ImGui::Text("转发");
    ImGui::Separator();
    
    // 创建可滚动的子窗口
    ImGui::BeginChild("ForwardMsgScrollArea", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
    
    // 复制数据到本地（最小化持锁时间）
    std::vector<std::string> localForwardBuffer;
    {
        std::lock_guard<std::mutex> lock(g_forwardMsgMutex);
        localForwardBuffer = g_forwardMsgBuffer;
    }
    
    if (localForwardBuffer.empty()) {
        ImGui::TextDisabled("");
    } else {
        for (const auto& msg : localForwardBuffer) {
            ImGui::TextWrapped("%s", msg.c_str());
        }
        
        // 自动滚动到底部
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);
    }
    
    ImGui::EndChild();
}

// 绘制请求消息面板
void DrawRequestMessagesPanel()
{
    ImGui::Text("请求");
    ImGui::Separator();
    
    // 创建可滚动的子窗口
    ImGui::BeginChild("RequestMsgScrollArea", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
    
    // 复制数据到本地（最小化持锁时间）
    std::vector<std::string> localRequestBuffer;
    {
        std::lock_guard<std::mutex> lock(g_requestMsgMutex);
        localRequestBuffer = g_requestMsgBuffer;
    }
    
    if (localRequestBuffer.empty()) {
        ImGui::TextDisabled("");
    } else {
        for (const auto& msg : localRequestBuffer) {
            ImGui::TextWrapped("%s", msg.c_str());
        }
        
        // 自动滚动到底部
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);
    }
    
    ImGui::EndChild();
}

// DirectX 11设备创建
bool CreateDeviceD3D(HWND hWnd)
{
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

// 窗口过程函数
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (g_pd3dDevice != nullptr && wParam != SIZE_MINIMIZED)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            CreateRenderTarget();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}