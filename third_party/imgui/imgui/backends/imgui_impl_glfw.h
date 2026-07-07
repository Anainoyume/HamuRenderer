// 亲爱的 imgui：GLFW 的平台后端
// 这需要与渲染器一起使用（例如 OpenGL3、Vulkan、WebGPU...）
// （信息：GLFW 是一个跨平台通用库，用于处理窗口、输入、OpenGL/Vulkan 图形上下文创建等）

// 实现的功能：
//  [X] 平台：剪贴板支持。
//  [X] 平台：鼠标支持。可以区分鼠标/触摸屏/笔（仅限 Windows）。
//  [X] 平台：键盘支持。从 1.87 开始，我们使用 io.AddKeyEvent() 函数。将 ImGuiKey 值传递给所有关键函数，例如ImGui::IsKeyPressed(ImGuiKey_Space)。 [旧版 GLFW_KEY_*值自 1.87 起已过时，自 1.91.5 起不再受支持]
//  [X] 平台：游戏手柄支持。使用“io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad”启用。
//  [X] 平台：鼠标光标形状和可见性 (ImGuiBackendFlags_HasMouseCursors)。调整光标大小需要 GLFW 3.4+！使用“io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange”禁用。
//  [X] 多个 Dear ImGui 上下文支持。
// 缺少的功能或问题：
//  [ ] 触摸事件仅在 Windows 上正确识别为触摸。这会给一些交互带来问题。 GLFW 没有提供识别触摸输入和鼠标输入的方法，我们无法调用 io.AddMouseSourceEvent() 来识别源。我们提供了特定于 Windows 的解决方法。
//  [ ] 缺少 ImGuiMouseCursor_Wait 和 ImGuiMouseCursor_Progress 光标。

// 您可以在项目中使用未修改的 imgui_impl_*文件。有关使用此示例的示例，请参阅示例/文件夹。
// 最好将整个 imgui/存储库包含到您的项目中（作为副本或子模块），并且仅构建您需要的后端。
// 了解亲爱的 ImGui：
// -常见问题解答 https://dearimgui.com/faq
// -入门 https://dearimgui.com/getting-started
// -文档 https://dearimgui.com/docs（与本地 docs/文件夹相同）。
// -imgui.cpp 顶部的介绍、链接等

#pragma once
#include "imgui.h"      // Imgui 实现 API
#ifndef IMGUI_DISABLE

struct GLFWwindow;
struct GLFWmonitor;

// 点击“入门”链接并检查示例/文件夹以了解如何使用后端！
IMGUI_IMPL_API bool     ImGui_ImplGlfw_InitForOpenGL(GLFWwindow* window, bool install_callbacks);
IMGUI_IMPL_API bool     ImGui_ImplGlfw_InitForVulkan(GLFWwindow* window, bool install_callbacks);
IMGUI_IMPL_API bool     ImGui_ImplGlfw_InitForOther(GLFWwindow* window, bool install_callbacks);
IMGUI_IMPL_API void     ImGui_ImplGlfw_Shutdown();
IMGUI_IMPL_API void     ImGui_ImplGlfw_NewFrame();

// Emscripten相关初始化阶段方法（在ImGui_ImplGlfw_InitForOpenGL之后调用）
#ifdef __EMSCRIPTEN__
IMGUI_IMPL_API void     ImGui_ImplGlfw_InstallEmscriptenCallbacks(GLFWwindow* window, const char* canvas_selector);
//静态内联无效 ImGui_ImplGlfw_InstallEmscriptenCanvasResizeCallback(const char*canvas_selector) { ImGui_ImplGlfw_InstallEmscriptenCallbacks(nullptr, canvas_selector); } } //在 1.91.0 中重命名
#endif

// GLFW 回调安装
// -当使用“install_callbacks=true”调用 Init 时：调用 ImGui_ImplGlfw_InstallCallbacks()。将为您安装 GLFW 回调。他们将链式调用用户之前安装的回调（如果有）。
// -使用“install_callbacks=false”调用 Init 时：不会安装 GLFW 回调。您将需要从自己的 GLFW 回调中自行调用各个函数。
IMGUI_IMPL_API void     ImGui_ImplGlfw_InstallCallbacks(GLFWwindow* window);
IMGUI_IMPL_API void     ImGui_ImplGlfw_RestoreCallbacks(GLFWwindow* window);

// GFLW 回调选项：
// -设置“chain_for_all_windows=true”以启用所有窗口的链接回调（包括由后端或用户创建的辅助视口）
IMGUI_IMPL_API void     ImGui_ImplGlfw_SetCallbacksChainForAllWindows(bool chain_for_all_windows);

// GLFW 回调（如果您没有安装回调，则可以调用自己的个人回调）
IMGUI_IMPL_API void     ImGui_ImplGlfw_WindowFocusCallback(GLFWwindow* window, int focused);        // 从 1.84 开始
IMGUI_IMPL_API void     ImGui_ImplGlfw_CursorEnterCallback(GLFWwindow* window, int entered);        // 从 1.84 开始
IMGUI_IMPL_API void     ImGui_ImplGlfw_CursorPosCallback(GLFWwindow* window, double x, double y);   // 从 1.87 开始
IMGUI_IMPL_API void     ImGui_ImplGlfw_MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
IMGUI_IMPL_API void     ImGui_ImplGlfw_ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
IMGUI_IMPL_API void     ImGui_ImplGlfw_KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
IMGUI_IMPL_API void     ImGui_ImplGlfw_CharCallback(GLFWwindow* window, unsigned int c);
IMGUI_IMPL_API void     ImGui_ImplGlfw_MonitorCallback(GLFWmonitor* monitor, int event);

// GLFW 助手
IMGUI_IMPL_API void     ImGui_ImplGlfw_Sleep(int milliseconds);
IMGUI_IMPL_API float    ImGui_ImplGlfw_GetContentScaleForWindow(GLFWwindow* window);
IMGUI_IMPL_API float    ImGui_ImplGlfw_GetContentScaleForMonitor(GLFWmonitor* monitor);


#endif // #ifndef IMGUI_DISABLE
