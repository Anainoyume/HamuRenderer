// 亲爱的 imgui：GLFW 的平台后端
// 这需要与渲染器一起使用（例如 OpenGL3、Vulkan、WebGPU...）
// （信息：GLFW 是一个跨平台通用库，用于处理窗口、输入、OpenGL/Vulkan 图形上下文创建等）
// （需要：GLFW 3.1+。更喜欢 GLFW 3.3+ 或 GLFW 3.4+ 以获得完整功能支持。）

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

// 关于 Emscripten 支持：
// -Emscripten 提供了自己的 GLFW (3.2.1) 实现（语法：“-sUSE_GLFW=3”），但操纵杆已损坏且不支持多个功能（多窗口、剪贴板、计时器等）
// -第三方 Emscripten GLFW (3.4.0) 实现（语法：“--use-port=contrib.glfw3”）修复了操纵杆问题并实现了浏览器的所有相关功能。
// 有关详细信息，请参阅 https://github.com/pongasoft/emscripten-glfw/blob/master/docs/Comparison.md。

// 变更日志
// （次要和较旧的更改已被删除，请参阅 git 历史记录了解详细信息）
//  2025-06-18：添加了对多个 Dear ImGui 上下文的支持。 （#8676、#8239、#8069）
//  2025-06-11：添加了 ImGui_ImplGlfw_GetContentScaleForWindow(GLFWwindow*window) 和 ImGui_ImplGlfw_GetContentScaleForMonitor(GLFWmonitor*Monitor) 帮助程序，以方便制作 DPI 感知应用程序。
//  2025-03-10：将 GLFW_KEY_WORLD_1 和 GLFW_KEY_WORLD_2 映射到 ImGuiKey_Oem102。
//  2025-03-03：修复了使用启用断言编译的 GLFW <= 3.2.1 时的剪贴板处理程序断言。
//  2024-08-22：将一些操作系统/后端相关的函数指针从 ImGuiIO 移至 ImGuiPlatformIO：
//               -io.GetClipboardTextFn -> platform_io.Platform_GetClipboardTextFn
//               -io.SetClipboardTextFn -> platform_io.Platform_SetClipboardTextFn
//               -io.PlatformOpenInShellFn -> platform_io.Platform_OpenInShellFn
//  2024-07-31：添加了 ImGui_ImplGlfw_Sleep() 辅助函数供我们的示例应用程序使用，因为 GLFW 不提供该函数。
//  2024-07-08：*重大*将 ImGui_ImplGlfw_InstallEmscriptenCanvasResizeCallback 重命名为 ImGui_ImplGlfw_InstallEmscriptenCallbacks()，添加了 GLFWWindow*参数。
//  2024-07-08：Emscripten：添加了对 GLFW3 contrib 端口的支持（GLFW 3.4.0 功能 + 错误修复）：要启用，请将 -sUSE_GLFW=3 替换为 --use-port=contrib.glfw3（需要 emscripten 3.1.59+）（https://github.com/pongasoft/emscripten-glfw）
//  2024-07-02：Emscripten：为 Emscripten 版本添加了 io.PlatformOpenInShellFn() 处理程序。
//  2023-12-19：Emscripten：添加了 ImGui_ImplGlfw_InstallEmscriptenCanvasResizeCallback() 来注册画布选择器和自动调整 GLFW 窗口大小。
//  2023-10-05：输入：添加了对额外 ImGuiKey 值的支持：F13 到 F24 功能键。
//  2023-07-18：输入：恢复忽略 GLFW_CURSOR_DISABLED 上的鼠标数据，因为它可以以不同的方式使用。如果需要，用户可以设置 ImGuiConfigFLags_NoMouse。 （＃5625，＃6609）
//  2023-06-12：接受 glfwGetTime() 不返回单调递增的值。当外围设备断开连接时，这似乎会发生在某些 Windows 设置上，并且也可能发生在浏览器 + Emscripten 上。 （#6491）
//  2023-04-04：输入：添加了对 io.AddMouseSourceEvent() 的支持，以仅在 Windows 上使用自定义 WndProc 挂钩来区分 ImGuiMouseSource_Mouse/ImGuiMouseSource_TouchScreen/ImGuiMouseSource_Pen。 (#2702)
//  2023-03-16：输入：修复了辅助视口（停靠分支）上的键修饰符处理。于 2023 年 1 月 4 日损坏。 （＃6248，＃6034）
//  2023-03-14：Emscripten：避免使用在 Emscripten 模拟中未正确实现的 glfwGetError() 和 glfwGetGamepadState()。 （#6240）
//  2023-02-03：Emscripten：注册自定义低级鼠标滚轮处理程序以在 Emscripten 上获得更准确的滚动脉冲。 （＃4019，＃6096）
//  2023-01-04：输入：修复了 Linux 上使用 Alt-GR 文本输入（例如德语键盘布局）时的 mods 状态，可能会导致文本输入损坏。恢复 2022 年 1 月 17 日的更改，我们使用 GLFW 提供的 mod 恢复，结果发现它们有问题。
//  2022-11-22：执行虚拟 glfwGetError() 读取，以使用 glfwGetKeyName() 取消丢失的名称。 (#5908)
//  2022-10-18：执行虚拟 glfwGetError() 读取以取消丢失鼠标光标错误。直接使用GLFW_VERSION_COMBINED。 (#5785)
//  2022 年 10 月 11 日：根据我们切换到 C++11 的要求，使用“nullptr”而不是“NULL”。
//  2022-09-26：输入：将 1.87 中引入的 ImGuiKey_ModXXX 重命名为 ImGuiMod_XXX（仍支持旧名称）。
//  2022-09-01：输入：通过不设置鼠标位置来尊重 GLFW_CURSOR_DISABLED *编辑*于 2023-07-18 恢复。
//  2022-04-30：输入：修复了 OSX 上小写字母的 ImGui_ImplGlfw_TranslateUntranslatedKey()。
//  2022-03-23：输入：修复了 1.87 中的回归，该回归导致在 Linux/X11 上错误报告键盘修饰符事件。
//  2022-02-07：添加了 ImGui_ImplGlfw_InstallCallbacks()/ImGui_ImplGlfw_RestoreCallbacks() 帮助程序，以方便用户在初始化后端后安装回调。
//  2022-01-26：输入：使用 ImGuiKey_ModXXX 标志将短暂的 io.AddKeyModsEvent() （两周前添加）替换为 io.AddKeyEvent()。抱歉造成混乱。
//  2021-01-20：输入：调用新的 io.AddKeyAnalogEvent() 来支持游戏手柄，而不是直接写入 io.NavInputs[]。
//  2022-01-17：输入：调用新的 io.AddMousePosEvent()、io.AddMouseButtonEvent()、io.AddMouseWheelEvent() API (1.87+)。
//  2022-01-17：输入：始终在关键事件之前和接下来更新关键模组（不在 NewFrame 中），以修复帧速率非常低的输入队列。
//  2022-01-12：*重大更改*：现在使用 glfwSetCursorPosCallback()。如果您使用 install_callbacks = false 调用 ImGui_ImplGlfw_InitXXX()，则必须安装 glfwSetCursorPosCallback() 并通过 ImGui_ImplGlfw_CursorPosCallback() 将其转发到后端。
//  2022-01-10：输入：调用新的 io.AddKeyEvent()、io.AddKeyModsEvent() + io.SetKeyEventNativeData() API (1.87+)。支持整个 ImGuiKey 系列。
//  2022-01-05：输入：将 GLFW 未翻译的键码转换回翻译的键码（在 ImGui_ImplGlfw_KeyCallback() 函数中），以匹配每个其他后端的行为，并方便使用带有字母快捷方式 API 的 GLFW。
//  2021-08-17：*重大更改*：现在使用 glfwSetWindowFocusCallback() 调用 io.AddFocusEvent()。如果您使用 install_callbacks = false 调用 ImGui_ImplGlfw_InitXXX()，则必须安装 glfwSetWindowFocusCallback() 并通过 ImGui_ImplGlfw_WindowFocusCallback() 将其转发到后端。
//  2021-07-29：*重大更改*：现在使用 glfwSetCursorEnterCallback()。当主机平台窗口悬停但未聚焦时，MousePos 会正确报告。如果您使用 install_callbacks = false 调用 ImGui_ImplGlfw_InitXXX()，则必须安装 glfwSetWindowFocusCallback() 回调并通过 ImGui_ImplGlfw_CursorEnterCallback() 将其转发到后端。
//  2021-06-29：重组后端以从单个结构中提取数据，以方便在多个上下文中使用（所有 g_XXXX 访问更改为 bd->XXXX）。
//  2020-01-17：输入：在分配鼠标光标时禁用错误回调，因为某些 X11 设置没有它们并且会生成错误。
//  2019-12-05：输入：添加了对 GLFW 3.4+ 中添加的新鼠标光标的支持（调整光标大小，不允许光标）。
//  2019-10-18：其他：以前安装的用户回调现在在关闭时恢复。
//  2019-07-21：输入：添加了 ImGuiKey_KeyPadEnter 的映射。
//  2019-05-11：输入：在调用 AddInputCharacter() 之前不要从字符回调中过滤值。
//  2019-03-12：其他：主窗口最小化时保留 DisplayFramebufferScale。
//  2018-11-30：杂项：设置 io.BackendPlatformName，以便它可以显示在“关于”窗口中。
//  2018-11-07：输入：安装 GLFW 回调时，我们会保存用户之前安装的回调（如果有）并链式调用它们。
//  2018-08-01：输入：Emscripten 的解决方法，它似乎不处理与焦点相关的调用。
//  2018-06-29：输入：添加了对 ImGuiMouseCursor_Hand 光标的支持。
//  2018-06-08：杂项：从旧的组合 GLFW+OpenGL/Vulkan 示例中提取 imgui_impl_glfw.cpp/.h。
//  2018-03-20：杂项：设置 io.BackendFlags ImGuiBackendFlags_HasMouseCursors 标志 + 荣誉 ImGuiConfigFlags_NoMouseCursorChange 标志。
//  2018-02-20：输入：添加了对鼠标光标的支持（ImGui::GetMouseCursor() 值，传递给 glfwSetCursor()）。
//  2018-02-06：杂项：删除了对 ImGui::Shutdown() 的调用，该调用在 1.60 WIP 中不可用，用户需要自己调用 CreateContext/DestroyContext。
//  2018-02-06：输入：添加了 ImGuiKey_Space 的映射。
//  2018-01-25：输入：如果设置了 ImGuiConfigFlags_NavEnableGamepad，则添加游戏手柄支持。
//  2018-01-25：输入：通过重新定位鼠标来尊重 io.WantSetMousePos（当使用导航并且设置 ImGuiConfigFlags_NavMoveMouse 时）。
//  2018-01-20：输入：添加了水平鼠标滚轮支持。
//  2018-01-18：输入：添加了 ImGuiKey_Insert 的映射。
//  2017-08-25：输入：当鼠标不可用/丢失时，MousePos 设置为 -FLT_MAX、-FLT_MAX（而不是 -1、-1）。
//  2016-10-15：杂项：向剪贴板函数处理程序添加了 void*user_data 参数。

#include "imgui.h"
#ifndef IMGUI_DISABLE
#include "imgui_impl_glfw.h"

// 使用 -Weverything 发出 Clang 警告
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"     // 警告：使用旧式强制转换
#pragma clang diagnostic ignored "-Wsign-conversion"    // 警告：隐式转换会更改符号性
#endif

// 玻璃纤维
#include <GLFW/glfw3.h>

#ifdef _WIN32
#undef APIENTRY
#ifndef GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3native.h>   // 对于 glfwGetWin32Window()
#endif
#ifdef __APPLE__
#ifndef GLFW_EXPOSE_NATIVE_COCOA
#define GLFW_EXPOSE_NATIVE_COCOA
#endif
#include <GLFW/glfw3native.h>   // 对于 glfwGetCocoaWindow()
#endif
#ifndef _WIN32
#include <unistd.h>             // 为了我们睡觉（）
#endif
#include <stdio.h>              // 对于 snprintf()

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#ifdef EMSCRIPTEN_USE_PORT_CONTRIB_GLFW3
#include <GLFW/emscripten_glfw3.h>
#else
#define EMSCRIPTEN_USE_EMBEDDED_GLFW3
#endif
#endif

// 我们按照定义收集版本测试，以便轻松查看哪些功能与版本相关。
#define GLFW_VERSION_COMBINED           (GLFW_VERSION_MAJOR * 1000 + GLFW_VERSION_MINOR * 100 + GLFW_VERSION_REVISION)
#define GLFW_HAS_PER_MONITOR_DPI        (GLFW_VERSION_COMBINED >= 3300) // 3.3+ glfwGetMonitorContentScale
#ifdef GLFW_RESIZE_NESW_CURSOR          // 让我们善待那些在 2019-04-16（3.4 定义）和 2019-11-29（游标定义）之间拉取 GLFW 的人 //FIXME：在 GLFW 3.4 发布时删除？
#define GLFW_HAS_NEW_CURSORS            (GLFW_VERSION_COMBINED >= 3400) // 3.4+ GLFW_RESIZE_ALL_CURSOR、GLFW_RESIZE_NESW_CURSOR、GLFW_RESIZE_NWSE_CURSOR、GLFW_NOT_ALLOWED_CURSOR
#else
#define GLFW_HAS_NEW_CURSORS            (0)
#endif
#define GLFW_HAS_GAMEPAD_API            (GLFW_VERSION_COMBINED >= 3300) // 3.3+ glfwGetGamepadState() 新 API
#define GLFW_HAS_GETKEYNAME             (GLFW_VERSION_COMBINED >= 3200) // 3.2+ glfwGetKeyName()
#define GLFW_HAS_GETERROR               (GLFW_VERSION_COMBINED >= 3300) // 3.3+ glfwGetError()

// 将 GLFWWindow*映射到 ImGuiContext*。 
// -如果我们可以使用 glfwSetWindowUserPointer()/glfwGetWindowUserPointer() 会更简单，但这是一个单一的共享资源。
// -如果我们可以使用例如，会更简单std::map<> 也是如此。但我们不这样做。
// -这并没有特别优化，因为我们预计大小会很小并且查询很少。
struct ImGui_ImplGlfw_WindowToContext { GLFWwindow* Window; ImGuiContext* Context; };
static ImVector<ImGui_ImplGlfw_WindowToContext> g_ContextMap;
static void ImGui_ImplGlfw_ContextMap_Add(GLFWwindow* window, ImGuiContext* ctx) { g_ContextMap.push_back(ImGui_ImplGlfw_WindowToContext{ window, ctx }); }
static void ImGui_ImplGlfw_ContextMap_Remove(GLFWwindow* window)                 { for (ImGui_ImplGlfw_WindowToContext& entry : g_ContextMap) if (entry.Window == window) { g_ContextMap.erase_unsorted(&entry); return; } }
static ImGuiContext* ImGui_ImplGlfw_ContextMap_Get(GLFWwindow* window)           { for (ImGui_ImplGlfw_WindowToContext& entry : g_ContextMap) if (entry.Window == window) return entry.Context; return nullptr; }

// GLFW数据
enum GlfwClientApi
{
    GlfwClientApi_Unknown,
    GlfwClientApi_OpenGL,
    GlfwClientApi_Vulkan,
};

struct ImGui_ImplGlfw_Data
{
    ImGuiContext*           Context;
    GLFWwindow*             Window;
    GlfwClientApi           ClientApi;
    double                  Time;
    GLFWwindow*             MouseWindow;
    GLFWcursor*             MouseCursors[ImGuiMouseCursor_COUNT];
    ImVec2                  LastValidMousePos;
    bool                    InstalledCallbacks;
    bool                    CallbacksChainForAllWindows;
    char                    BackendPlatformName[32];
#ifdef EMSCRIPTEN_USE_EMBEDDED_GLFW3
    const char*             CanvasSelector;
#endif

    // 链式 GLFW 回调：我们的回调将调用用户之前安装的回调（如果有）。
    GLFWwindowfocusfun      PrevUserCallbackWindowFocus;
    GLFWcursorposfun        PrevUserCallbackCursorPos;
    GLFWcursorenterfun      PrevUserCallbackCursorEnter;
    GLFWmousebuttonfun      PrevUserCallbackMousebutton;
    GLFWscrollfun           PrevUserCallbackScroll;
    GLFWkeyfun              PrevUserCallbackKey;
    GLFWcharfun             PrevUserCallbackChar;
    GLFWmonitorfun          PrevUserCallbackMonitor;
#ifdef _WIN32
    WNDPROC                 PrevWndProc;
#endif

    ImGui_ImplGlfw_Data()   { memset((void*)this, 0, sizeof(*this)); }
};

// 后端数据存储在 io.BackendPlatformUserData 中，以支持多个 Dear ImGui 上下文
// 强烈建议您使用具有多视口的对接分支（==单个 Dear ImGui 上下文 + 多个窗口），而不是多个 Dear ImGui 上下文。
// FIXME：多上下文支持尚未经过充分测试，并且可能在此后端功能失调。
// -由于 glfwPollEvents() 处理所有窗口，并且可能会在其外部调用某些事件，因此您需要注册自己的回调
//   （在 ImGui_ImplGlfw_InitXXX 函数中传递 install_callbacks=false ），设置当前亲爱的 imgui 上下文，然后调用我们的回调。
// -否则，我们可能需要存储 GLFWWindow*-> ImGuiContext*映射并在后端处理它，从而增加一点额外的复杂性。
// 修复：使用多上下文时，某些共享资源（鼠标光标形状、游戏手柄）处理不当。
namespace ImGui { extern ImGuiIO& GetIO(ImGuiContext*); }
static ImGui_ImplGlfw_Data* ImGui_ImplGlfw_GetBackendData()
{
    // 获取当前上下文的数据
    return ImGui::GetCurrentContext() ? (ImGui_ImplGlfw_Data*)ImGui::GetIO().BackendPlatformUserData : nullptr;
}
static ImGui_ImplGlfw_Data* ImGui_ImplGlfw_GetBackendData(GLFWwindow* window)
{
    // 获取给定 GLFW 窗口的数据，无论当前上下文如何（因为 GLFW 事件是一起发送的）
    ImGuiContext* ctx = ImGui_ImplGlfw_ContextMap_Get(window);
    return (ImGui_ImplGlfw_Data*)ImGui::GetIO(ctx).BackendPlatformUserData;
}

// 功能

// 不是静态的，允许第三方代码使用（但未记录）
ImGuiKey ImGui_ImplGlfw_KeyToImGuiKey(int keycode, int scancode);
ImGuiKey ImGui_ImplGlfw_KeyToImGuiKey(int keycode, int scancode)
{
    IM_UNUSED(scancode);
    switch (keycode)
    {
        case GLFW_KEY_TAB: return ImGuiKey_Tab;
        case GLFW_KEY_LEFT: return ImGuiKey_LeftArrow;
        case GLFW_KEY_RIGHT: return ImGuiKey_RightArrow;
        case GLFW_KEY_UP: return ImGuiKey_UpArrow;
        case GLFW_KEY_DOWN: return ImGuiKey_DownArrow;
        case GLFW_KEY_PAGE_UP: return ImGuiKey_PageUp;
        case GLFW_KEY_PAGE_DOWN: return ImGuiKey_PageDown;
        case GLFW_KEY_HOME: return ImGuiKey_Home;
        case GLFW_KEY_END: return ImGuiKey_End;
        case GLFW_KEY_INSERT: return ImGuiKey_Insert;
        case GLFW_KEY_DELETE: return ImGuiKey_Delete;
        case GLFW_KEY_BACKSPACE: return ImGuiKey_Backspace;
        case GLFW_KEY_SPACE: return ImGuiKey_Space;
        case GLFW_KEY_ENTER: return ImGuiKey_Enter;
        case GLFW_KEY_ESCAPE: return ImGuiKey_Escape;
        case GLFW_KEY_APOSTROPHE: return ImGuiKey_Apostrophe;
        case GLFW_KEY_COMMA: return ImGuiKey_Comma;
        case GLFW_KEY_MINUS: return ImGuiKey_Minus;
        case GLFW_KEY_PERIOD: return ImGuiKey_Period;
        case GLFW_KEY_SLASH: return ImGuiKey_Slash;
        case GLFW_KEY_SEMICOLON: return ImGuiKey_Semicolon;
        case GLFW_KEY_EQUAL: return ImGuiKey_Equal;
        case GLFW_KEY_LEFT_BRACKET: return ImGuiKey_LeftBracket;
        case GLFW_KEY_BACKSLASH: return ImGuiKey_Backslash;
        case GLFW_KEY_WORLD_1: return ImGuiKey_Oem102;
        case GLFW_KEY_WORLD_2: return ImGuiKey_Oem102;
        case GLFW_KEY_RIGHT_BRACKET: return ImGuiKey_RightBracket;
        case GLFW_KEY_GRAVE_ACCENT: return ImGuiKey_GraveAccent;
        case GLFW_KEY_CAPS_LOCK: return ImGuiKey_CapsLock;
        case GLFW_KEY_SCROLL_LOCK: return ImGuiKey_ScrollLock;
        case GLFW_KEY_NUM_LOCK: return ImGuiKey_NumLock;
        case GLFW_KEY_PRINT_SCREEN: return ImGuiKey_PrintScreen;
        case GLFW_KEY_PAUSE: return ImGuiKey_Pause;
        case GLFW_KEY_KP_0: return ImGuiKey_Keypad0;
        case GLFW_KEY_KP_1: return ImGuiKey_Keypad1;
        case GLFW_KEY_KP_2: return ImGuiKey_Keypad2;
        case GLFW_KEY_KP_3: return ImGuiKey_Keypad3;
        case GLFW_KEY_KP_4: return ImGuiKey_Keypad4;
        case GLFW_KEY_KP_5: return ImGuiKey_Keypad5;
        case GLFW_KEY_KP_6: return ImGuiKey_Keypad6;
        case GLFW_KEY_KP_7: return ImGuiKey_Keypad7;
        case GLFW_KEY_KP_8: return ImGuiKey_Keypad8;
        case GLFW_KEY_KP_9: return ImGuiKey_Keypad9;
        case GLFW_KEY_KP_DECIMAL: return ImGuiKey_KeypadDecimal;
        case GLFW_KEY_KP_DIVIDE: return ImGuiKey_KeypadDivide;
        case GLFW_KEY_KP_MULTIPLY: return ImGuiKey_KeypadMultiply;
        case GLFW_KEY_KP_SUBTRACT: return ImGuiKey_KeypadSubtract;
        case GLFW_KEY_KP_ADD: return ImGuiKey_KeypadAdd;
        case GLFW_KEY_KP_ENTER: return ImGuiKey_KeypadEnter;
        case GLFW_KEY_KP_EQUAL: return ImGuiKey_KeypadEqual;
        case GLFW_KEY_LEFT_SHIFT: return ImGuiKey_LeftShift;
        case GLFW_KEY_LEFT_CONTROL: return ImGuiKey_LeftCtrl;
        case GLFW_KEY_LEFT_ALT: return ImGuiKey_LeftAlt;
        case GLFW_KEY_LEFT_SUPER: return ImGuiKey_LeftSuper;
        case GLFW_KEY_RIGHT_SHIFT: return ImGuiKey_RightShift;
        case GLFW_KEY_RIGHT_CONTROL: return ImGuiKey_RightCtrl;
        case GLFW_KEY_RIGHT_ALT: return ImGuiKey_RightAlt;
        case GLFW_KEY_RIGHT_SUPER: return ImGuiKey_RightSuper;
        case GLFW_KEY_MENU: return ImGuiKey_Menu;
        case GLFW_KEY_0: return ImGuiKey_0;
        case GLFW_KEY_1: return ImGuiKey_1;
        case GLFW_KEY_2: return ImGuiKey_2;
        case GLFW_KEY_3: return ImGuiKey_3;
        case GLFW_KEY_4: return ImGuiKey_4;
        case GLFW_KEY_5: return ImGuiKey_5;
        case GLFW_KEY_6: return ImGuiKey_6;
        case GLFW_KEY_7: return ImGuiKey_7;
        case GLFW_KEY_8: return ImGuiKey_8;
        case GLFW_KEY_9: return ImGuiKey_9;
        case GLFW_KEY_A: return ImGuiKey_A;
        case GLFW_KEY_B: return ImGuiKey_B;
        case GLFW_KEY_C: return ImGuiKey_C;
        case GLFW_KEY_D: return ImGuiKey_D;
        case GLFW_KEY_E: return ImGuiKey_E;
        case GLFW_KEY_F: return ImGuiKey_F;
        case GLFW_KEY_G: return ImGuiKey_G;
        case GLFW_KEY_H: return ImGuiKey_H;
        case GLFW_KEY_I: return ImGuiKey_I;
        case GLFW_KEY_J: return ImGuiKey_J;
        case GLFW_KEY_K: return ImGuiKey_K;
        case GLFW_KEY_L: return ImGuiKey_L;
        case GLFW_KEY_M: return ImGuiKey_M;
        case GLFW_KEY_N: return ImGuiKey_N;
        case GLFW_KEY_O: return ImGuiKey_O;
        case GLFW_KEY_P: return ImGuiKey_P;
        case GLFW_KEY_Q: return ImGuiKey_Q;
        case GLFW_KEY_R: return ImGuiKey_R;
        case GLFW_KEY_S: return ImGuiKey_S;
        case GLFW_KEY_T: return ImGuiKey_T;
        case GLFW_KEY_U: return ImGuiKey_U;
        case GLFW_KEY_V: return ImGuiKey_V;
        case GLFW_KEY_W: return ImGuiKey_W;
        case GLFW_KEY_X: return ImGuiKey_X;
        case GLFW_KEY_Y: return ImGuiKey_Y;
        case GLFW_KEY_Z: return ImGuiKey_Z;
        case GLFW_KEY_F1: return ImGuiKey_F1;
        case GLFW_KEY_F2: return ImGuiKey_F2;
        case GLFW_KEY_F3: return ImGuiKey_F3;
        case GLFW_KEY_F4: return ImGuiKey_F4;
        case GLFW_KEY_F5: return ImGuiKey_F5;
        case GLFW_KEY_F6: return ImGuiKey_F6;
        case GLFW_KEY_F7: return ImGuiKey_F7;
        case GLFW_KEY_F8: return ImGuiKey_F8;
        case GLFW_KEY_F9: return ImGuiKey_F9;
        case GLFW_KEY_F10: return ImGuiKey_F10;
        case GLFW_KEY_F11: return ImGuiKey_F11;
        case GLFW_KEY_F12: return ImGuiKey_F12;
        case GLFW_KEY_F13: return ImGuiKey_F13;
        case GLFW_KEY_F14: return ImGuiKey_F14;
        case GLFW_KEY_F15: return ImGuiKey_F15;
        case GLFW_KEY_F16: return ImGuiKey_F16;
        case GLFW_KEY_F17: return ImGuiKey_F17;
        case GLFW_KEY_F18: return ImGuiKey_F18;
        case GLFW_KEY_F19: return ImGuiKey_F19;
        case GLFW_KEY_F20: return ImGuiKey_F20;
        case GLFW_KEY_F21: return ImGuiKey_F21;
        case GLFW_KEY_F22: return ImGuiKey_F22;
        case GLFW_KEY_F23: return ImGuiKey_F23;
        case GLFW_KEY_F24: return ImGuiKey_F24;
        default: return ImGuiKey_None;
    }
}

// X11 不包括 GLFW 提交的“mods”标志中当前按下/释放的修饰键
// 请参阅 https://github.com/ocornut/imgui/issues/6034 和 https://github.com/glfw/glfw/issues/1630
static void ImGui_ImplGlfw_UpdateKeyModifiers(ImGuiIO& io, GLFWwindow* window)
{
    io.AddKeyEvent(ImGuiMod_Ctrl,  (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) || (glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS));
    io.AddKeyEvent(ImGuiMod_Shift, (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)   == GLFW_PRESS) || (glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT)   == GLFW_PRESS));
    io.AddKeyEvent(ImGuiMod_Alt,   (glfwGetKey(window, GLFW_KEY_LEFT_ALT)     == GLFW_PRESS) || (glfwGetKey(window, GLFW_KEY_RIGHT_ALT)     == GLFW_PRESS));
    io.AddKeyEvent(ImGuiMod_Super, (glfwGetKey(window, GLFW_KEY_LEFT_SUPER)   == GLFW_PRESS) || (glfwGetKey(window, GLFW_KEY_RIGHT_SUPER)   == GLFW_PRESS));
}

static bool ImGui_ImplGlfw_ShouldChainCallback(ImGui_ImplGlfw_Data* bd, GLFWwindow* window)
{
    return bd->CallbacksChainForAllWindows ? true : (window == bd->Window);
}

void ImGui_ImplGlfw_MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    ImGui_ImplGlfw_Data* bd = ImGui_ImplGlfw_GetBackendData(window);

    if (bd->PrevUserCallbackMousebutton != nullptr && ImGui_ImplGlfw_ShouldChainCallback(bd, window))
        bd->PrevUserCallbackMousebutton(window, button, action, mods);

    ImGuiIO& io = ImGui::GetIO(bd->Context);
    ImGui_ImplGlfw_UpdateKeyModifiers(io, window);
    if (button >= 0 && button < ImGuiMouseButton_COUNT)
        io.AddMouseButtonEvent(button, action == GLFW_PRESS);
}

void ImGui_ImplGlfw_ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    ImGui_ImplGlfw_Data* bd = ImGui_ImplGlfw_GetBackendData(window);
    if (bd->PrevUserCallbackScroll != nullptr && ImGui_ImplGlfw_ShouldChainCallback(bd, window))
        bd->PrevUserCallbackScroll(window, xoffset, yoffset);

#ifdef EMSCRIPTEN_USE_EMBEDDED_GLFW3
    // 忽略 GLFW 事件：将在 ImGui_ImplEmscripten_WheelCallback() 中处理。
    return;
#endif

    ImGuiIO& io = ImGui::GetIO(bd->Context);
    io.AddMouseWheelEvent((float)xoffset, (float)yoffset);
}

// FIXME：这应该被烘焙到 ImGui_ImplGlfw_KeyToImGuiKey() 中吗？那么传递给 io.SetKeyEventNativeData() 的值又如何呢？
static int ImGui_ImplGlfw_TranslateUntranslatedKey(int key, int scancode)
{
#if GLFW_HAS_GETKEYNAME && !defined(EMSCRIPTEN_USE_EMBEDDED_GLFW3)
    // GLFW 3.1+ 尝试“取消翻译”按键，这与其他框架的做法相反，使得使用字母快捷键变得困难。
    // （这样做是有原因的：即 GLFW 更有可能用于 WASD 类型的游戏控制而不是字母快捷键，但 IHMO 3.1 的更改可能会以不同的方式完成）
    // 有关详细信息，请参阅 https://github.com/glfw/glfw/issues/1502。
    // 添加一个解决方法来撤消此操作（因此我们的密钥被翻译->未翻译->翻译，可能是一个有损过程）。
    // 这不会涵盖边缘情况，但至少会涵盖常见情况。
    if (key >= GLFW_KEY_KP_0 && key <= GLFW_KEY_KP_EQUAL)
        return key;
    GLFWerrorfun prev_error_callback = glfwSetErrorCallback(nullptr);
    const char* key_name = glfwGetKeyName(key, scancode);
    glfwSetErrorCallback(prev_error_callback);
#if GLFW_HAS_GETERROR && !defined(EMSCRIPTEN_USE_EMBEDDED_GLFW3) // 吃错误（参见#5908）
    (void)glfwGetError(nullptr);
#endif
    if (key_name && key_name[0] != 0 && key_name[1] == 0)
    {
        const char char_names[] = "`-=[]\\,;\'./";
        const int char_keys[] = { GLFW_KEY_GRAVE_ACCENT, GLFW_KEY_MINUS, GLFW_KEY_EQUAL, GLFW_KEY_LEFT_BRACKET, GLFW_KEY_RIGHT_BRACKET, GLFW_KEY_BACKSLASH, GLFW_KEY_COMMA, GLFW_KEY_SEMICOLON, GLFW_KEY_APOSTROPHE, GLFW_KEY_PERIOD, GLFW_KEY_SLASH, 0 };
        IM_ASSERT(IM_ARRAYSIZE(char_names) == IM_ARRAYSIZE(char_keys));
        if (key_name[0] >= '0' && key_name[0] <= '9')               { key = GLFW_KEY_0 + (key_name[0] - '0'); }
        else if (key_name[0] >= 'A' && key_name[0] <= 'Z')          { key = GLFW_KEY_A + (key_name[0] - 'A'); }
        else if (key_name[0] >= 'a' && key_name[0] <= 'z')          { key = GLFW_KEY_A + (key_name[0] - 'a'); }
        else if (const char* p = strchr(char_names, key_name[0]))   { key = char_keys[p - char_names]; }
    }
    // if (action == GLFW_PRESS) printf("键 %d 扫描码 %d 名称 '%s'\n", key, scancode, key_name);
#else
    IM_UNUSED(scancode);
#endif
    return key;
}

void ImGui_ImplGlfw_KeyCallback(GLFWwindow* window, int keycode, int scancode, int action, int mods)
{
    ImGui_ImplGlfw_Data* bd = ImGui_ImplGlfw_GetBackendData(window);
    if (bd->PrevUserCallbackKey != nullptr && ImGui_ImplGlfw_ShouldChainCallback(bd, window))
        bd->PrevUserCallbackKey(window, keycode, scancode, action, mods);

    if (action != GLFW_PRESS && action != GLFW_RELEASE)
        return;

    ImGuiIO& io = ImGui::GetIO(bd->Context);
    ImGui_ImplGlfw_UpdateKeyModifiers(io, window);

    keycode = ImGui_ImplGlfw_TranslateUntranslatedKey(keycode, scancode);

    ImGuiKey imgui_key = ImGui_ImplGlfw_KeyToImGuiKey(keycode, scancode);
    io.AddKeyEvent(imgui_key, (action == GLFW_PRESS));
    io.SetKeyEventNativeData(imgui_key, keycode, scancode); // 支持旧版索引（<1.87 用户代码）
}

void ImGui_ImplGlfw_WindowFocusCallback(GLFWwindow* window, int focused)
{
    ImGui_ImplGlfw_Data* bd = ImGui_ImplGlfw_GetBackendData(window);
    if (bd->PrevUserCallbackWindowFocus != nullptr && ImGui_ImplGlfw_ShouldChainCallback(bd, window))
        bd->PrevUserCallbackWindowFocus(window, focused);

    ImGuiIO& io = ImGui::GetIO(bd->Context);
    io.AddFocusEvent(focused != 0);
}

void ImGui_ImplGlfw_CursorPosCallback(GLFWwindow* window, double x, double y)
{
    ImGui_ImplGlfw_Data* bd = ImGui_ImplGlfw_GetBackendData(window);
    if (bd->PrevUserCallbackCursorPos != nullptr && ImGui_ImplGlfw_ShouldChainCallback(bd, window))
        bd->PrevUserCallbackCursorPos(window, x, y);

    ImGuiIO& io = ImGui::GetIO(bd->Context);
    io.AddMousePosEvent((float)x, (float)y);
    bd->LastValidMousePos = ImVec2((float)x, (float)y);
}

// 解决方法：X11 似乎发送虚假的离开/进入事件，这将使我们失去位置，
// 所以我们备份它并在离开/进入时恢复（请参阅https://github.com/ocornut/imgui/issues/4984）
void ImGui_ImplGlfw_CursorEnterCallback(GLFWwindow* window, int entered)
{
    ImGui_ImplGlfw_Data* bd = ImGui_ImplGlfw_GetBackendData(window);
    if (bd->PrevUserCallbackCursorEnter != nullptr && ImGui_ImplGlfw_ShouldChainCallback(bd, window))
        bd->PrevUserCallbackCursorEnter(window, entered);

    ImGuiIO& io = ImGui::GetIO(bd->Context);
    if (entered)
    {
        bd->MouseWindow = window;
        io.AddMousePosEvent(bd->LastValidMousePos.x, bd->LastValidMousePos.y);
    }
    else if (!entered && bd->MouseWindow == window)
    {
        bd->LastValidMousePos = io.MousePos;
        bd->MouseWindow = nullptr;
        io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
    }
}

void ImGui_ImplGlfw_CharCallback(GLFWwindow* window, unsigned int c)
{
    ImGui_ImplGlfw_Data* bd = ImGui_ImplGlfw_GetBackendData(window);
    if (bd->PrevUserCallbackChar != nullptr && ImGui_ImplGlfw_ShouldChainCallback(bd, window))
        bd->PrevUserCallbackChar(window, c);

    ImGuiIO& io = ImGui::GetIO(bd->Context);
    io.AddInputCharacter(c);
}

void ImGui_ImplGlfw_MonitorCallback(GLFWmonitor*, int)
{
    // 在“master”分支中未使用，但“docking”分支将使用它，因此我们在它之前声明它，这样如果您必须安装回调，您也可以安装这个。
}

#ifdef EMSCRIPTEN_USE_EMBEDDED_GLFW3
static EM_BOOL ImGui_ImplEmscripten_WheelCallback(int, const EmscriptenWheelEvent* ev, void* user_data)
{
    // 模仿 SDL 中的 Emscripten_HandleWheel()。
    // GLFW JS 仿真层中的相应等效项具有不正确的量化，防止出现小值。参见#6096
    ImGui_ImplGlfw_Data* bd = (ImGui_ImplGlfw_Data*)user_data;
    float multiplier = 0.0f;
    if (ev->deltaMode == DOM_DELTA_PIXEL)       { multiplier = 1.0f / 100.0f; } // 100 个像素组成一个台阶。
    else if (ev->deltaMode == DOM_DELTA_LINE)   { multiplier = 1.0f / 3.0f; }   // 3行组成一个步骤。
    else if (ev->deltaMode == DOM_DELTA_PAGE)   { multiplier = 80.0f; }         // 一页由 80 个步骤组成。
    float wheel_x = ev->deltaX * -multiplier;
    float wheel_y = ev->deltaY * -multiplier;
    ImGuiIO& io = ImGui::GetIO(bd->Context);
    io.AddMouseWheelEvent(wheel_x, wheel_y);
    //IMGUI_DEBUG_LOG("[Emsc] 模式 %d dx: %.2f, dy: %.2f, dz: %.2f --> feed %.2f %.2f\n", (int)ev->deltaMode, ev->deltaX, ev->deltaY, ev->deltaZ,wheel_x,wheel_y);
    return EM_TRUE;
}
#endif

#ifdef _WIN32
// GLFW 不允许区分鼠标、触摸屏和笔。
// 添加对 Win32 的支持（基于 imgui_impl_win32），因为我们依赖 _TouchScreen 信息以不同的方式进行输入。
static ImGuiMouseSource GetMouseSourceFromMessageExtraInfo()
{
    LPARAM extra_info = ::GetMessageExtraInfo();
    if ((extra_info & 0xFFFFFF80) == 0xFF515700)
        return ImGuiMouseSource_Pen;
    if ((extra_info & 0xFFFFFF80) == 0xFF515780)
        return ImGuiMouseSource_TouchScreen;
    return ImGuiMouseSource_Mouse;
}
static LRESULT CALLBACK ImGui_ImplGlfw_WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    ImGui_ImplGlfw_Data* bd = (ImGui_ImplGlfw_Data*)::GetPropA(hWnd, "IMGUI_BACKEND_DATA");
    ImGuiIO& io = ImGui::GetIO(bd->Context);

    switch (msg)
    {
    case WM_MOUSEMOVE: case WM_NCMOUSEMOVE:
    case WM_LBUTTONDOWN: case WM_LBUTTONDBLCLK: case WM_LBUTTONUP:
    case WM_RBUTTONDOWN: case WM_RBUTTONDBLCLK: case WM_RBUTTONUP:
    case WM_MBUTTONDOWN: case WM_MBUTTONDBLCLK: case WM_MBUTTONUP:
    case WM_XBUTTONDOWN: case WM_XBUTTONDBLCLK: case WM_XBUTTONUP:
        io.AddMouseSourceEvent(GetMouseSourceFromMessageExtraInfo());
        break;
    default: break;
    }
    return ::CallWindowProcW(bd->PrevWndProc, hWnd, msg, wParam, lParam);
}
#endif

void ImGui_ImplGlfw_InstallCallbacks(GLFWwindow* window)
{
    ImGui_ImplGlfw_Data* bd = ImGui_ImplGlfw_GetBackendData(window);
    IM_ASSERT(bd->InstalledCallbacks == false && "Callbacks already installed!");
    IM_ASSERT(bd->Window == window);

    bd->PrevUserCallbackWindowFocus = glfwSetWindowFocusCallback(window, ImGui_ImplGlfw_WindowFocusCallback);
    bd->PrevUserCallbackCursorEnter = glfwSetCursorEnterCallback(window, ImGui_ImplGlfw_CursorEnterCallback);
    bd->PrevUserCallbackCursorPos = glfwSetCursorPosCallback(window, ImGui_ImplGlfw_CursorPosCallback);
    bd->PrevUserCallbackMousebutton = glfwSetMouseButtonCallback(window, ImGui_ImplGlfw_MouseButtonCallback);
    bd->PrevUserCallbackScroll = glfwSetScrollCallback(window, ImGui_ImplGlfw_ScrollCallback);
    bd->PrevUserCallbackKey = glfwSetKeyCallback(window, ImGui_ImplGlfw_KeyCallback);
    bd->PrevUserCallbackChar = glfwSetCharCallback(window, ImGui_ImplGlfw_CharCallback);
    bd->PrevUserCallbackMonitor = glfwSetMonitorCallback(ImGui_ImplGlfw_MonitorCallback);
    bd->InstalledCallbacks = true;
}

void ImGui_ImplGlfw_RestoreCallbacks(GLFWwindow* window)
{
    ImGui_ImplGlfw_Data* bd = ImGui_ImplGlfw_GetBackendData(window);
    IM_ASSERT(bd->InstalledCallbacks == true && "Callbacks not installed!");
    IM_ASSERT(bd->Window == window);

    glfwSetWindowFocusCallback(window, bd->PrevUserCallbackWindowFocus);
    glfwSetCursorEnterCallback(window, bd->PrevUserCallbackCursorEnter);
    glfwSetCursorPosCallback(window, bd->PrevUserCallbackCursorPos);
    glfwSetMouseButtonCallback(window, bd->PrevUserCallbackMousebutton);
    glfwSetScrollCallback(window, bd->PrevUserCallbackScroll);
    glfwSetKeyCallback(window, bd->PrevUserCallbackKey);
    glfwSetCharCallback(window, bd->PrevUserCallbackChar);
    glfwSetMonitorCallback(bd->PrevUserCallbackMonitor);
    bd->InstalledCallbacks = false;
    bd->PrevUserCallbackWindowFocus = nullptr;
    bd->PrevUserCallbackCursorEnter = nullptr;
    bd->PrevUserCallbackCursorPos = nullptr;
    bd->PrevUserCallbackMousebutton = nullptr;
    bd->PrevUserCallbackScroll = nullptr;
    bd->PrevUserCallbackKey = nullptr;
    bd->PrevUserCallbackChar = nullptr;
    bd->PrevUserCallbackMonitor = nullptr;
}

// 设置为“true”以启用所有窗口（包括后端或用户创建的辅助视口）的链接安装回调。
// 默认情况下这是“false”，这意味着我们只链接主视口的回调。
// 默认情况下，我们不能将其设置为“true”，因为用户回调代码可能没有测试其回调的“window”参数。
// 如果您将其设置为“true”，您的用户回调代码将需要确保您正在测试“window”参数。
void ImGui_ImplGlfw_SetCallbacksChainForAllWindows(bool chain_for_all_windows)
{
    ImGui_ImplGlfw_Data* bd = ImGui_ImplGlfw_GetBackendData();
    bd->CallbacksChainForAllWindows = chain_for_all_windows;
}

#ifdef __EMSCRIPTEN__
#if EMSCRIPTEN_USE_PORT_CONTRIB_GLFW3 >= 34020240817
void ImGui_ImplGlfw_EmscriptenOpenURL(const char* url) { if (url) emscripten::glfw3::OpenURL(url); }
#else
EM_JS(void, ImGui_ImplGlfw_EmscriptenOpenURL, (const char* url), { url = url ? UTF8ToString(url) : null; if (url) window.open(url, '_blank'); });
#endif
#endif

static bool ImGui_ImplGlfw_Init(GLFWwindow* window, bool install_callbacks, GlfwClientApi client_api)
{
    ImGuiIO& io = ImGui::GetIO();
    IMGUI_CHECKVERSION();
    IM_ASSERT(io.BackendPlatformUserData == nullptr && "Already initialized a platform backend!");
    //printf("GLFW_VERSION: %d.%d.%d (%d)", GLFW_VERSION_MAJOR, GLFW_VERSION_MINOR, GLFW_VERSION_REVISION, GLFW_VERSION_COMBINED);

    // 设置后端功能标志
    ImGui_ImplGlfw_Data* bd = IM_NEW(ImGui_ImplGlfw_Data)();
    snprintf(bd->BackendPlatformName, sizeof(bd->BackendPlatformName), "imgui_impl_glfw (%d)", GLFW_VERSION_COMBINED);
    io.BackendPlatformUserData = (void*)bd;
    io.BackendPlatformName = bd->BackendPlatformName;
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;         // 我们可以遵循 GetMouseCursor() 值（可选）
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;          // 我们可以尊重 io.WantSetMousePos 请求（可选，很少使用）

    bd->Context = ImGui::GetCurrentContext();
    bd->Window = window;
    bd->Time = 0.0;
    ImGui_ImplGlfw_ContextMap_Add(window, bd->Context);

    ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
#if GLFW_VERSION_COMBINED < 3300
    platform_io.Platform_SetClipboardTextFn = [](ImGuiContext*, const char* text) { glfwSetClipboardString(ImGui_ImplGlfw_GetBackendData()->Window, text); };
    platform_io.Platform_GetClipboardTextFn = [](ImGuiContext*) { return glfwGetClipboardString(ImGui_ImplGlfw_GetBackendData()->Window); };
#else
    platform_io.Platform_SetClipboardTextFn = [](ImGuiContext*, const char* text) { glfwSetClipboardString(nullptr, text); };
    platform_io.Platform_GetClipboardTextFn = [](ImGuiContext*) { return glfwGetClipboardString(nullptr); };
#endif

#ifdef __EMSCRIPTEN__
    platform_io.Platform_OpenInShellFn = [](ImGuiContext*, const char* url) { ImGui_ImplGlfw_EmscriptenOpenURL(url); return true; };
#endif

    // 创建鼠标光标
    // （根据设计，X11 上的光标是用户可配置的，并且某些光标可能会丢失。当光标不存在时，
    // GLFW 会发出一个错误，该错误通常会由应用程序打印，因此我们暂时禁用错误报告。
    // 缺少游标将返回 nullptr，我们的 _UpdateMouseCursor() 函数将使用箭头游标。）
    GLFWerrorfun prev_error_callback = glfwSetErrorCallback(nullptr);
    bd->MouseCursors[ImGuiMouseCursor_Arrow] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    bd->MouseCursors[ImGuiMouseCursor_TextInput] = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
    bd->MouseCursors[ImGuiMouseCursor_ResizeNS] = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
    bd->MouseCursors[ImGuiMouseCursor_ResizeEW] = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
    bd->MouseCursors[ImGuiMouseCursor_Hand] = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
#if GLFW_HAS_NEW_CURSORS
    bd->MouseCursors[ImGuiMouseCursor_ResizeAll] = glfwCreateStandardCursor(GLFW_RESIZE_ALL_CURSOR);
    bd->MouseCursors[ImGuiMouseCursor_ResizeNESW] = glfwCreateStandardCursor(GLFW_RESIZE_NESW_CURSOR);
    bd->MouseCursors[ImGuiMouseCursor_ResizeNWSE] = glfwCreateStandardCursor(GLFW_RESIZE_NWSE_CURSOR);
    bd->MouseCursors[ImGuiMouseCursor_NotAllowed] = glfwCreateStandardCursor(GLFW_NOT_ALLOWED_CURSOR);
#else
    bd->MouseCursors[ImGuiMouseCursor_ResizeAll] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    bd->MouseCursors[ImGuiMouseCursor_ResizeNESW] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    bd->MouseCursors[ImGuiMouseCursor_ResizeNWSE] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    bd->MouseCursors[ImGuiMouseCursor_NotAllowed] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
#endif
    glfwSetErrorCallback(prev_error_callback);
#if GLFW_HAS_GETERROR && !defined(__EMSCRIPTEN__) // 吃错误（参见#5908）
    (void)glfwGetError(nullptr);
#endif

    // 链式 GLFW 回调：我们的回调将调用用户之前安装的回调（如果有）。
    if (install_callbacks)
        ImGui_ImplGlfw_InstallCallbacks(window);

    // 在视口中设置平台相关数据
    ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    main_viewport->PlatformHandle = (void*)bd->Window;
#ifdef _WIN32
    main_viewport->PlatformHandleRaw = glfwGetWin32Window(bd->Window);
#elif defined(__APPLE__)
    main_viewport->PlatformHandleRaw = (void*)glfwGetCocoaWindow(bd->Window);
#else
    IM_UNUSED(main_viewport);
#endif

    // Windows：注册一个 WndProc 钩子，以便我们可以拦截一些消息。
#ifdef _WIN32
    HWND hwnd = (HWND)main_viewport->PlatformHandleRaw;
    ::SetPropA(hwnd, "IMGUI_BACKEND_DATA", bd);
    bd->PrevWndProc = (WNDPROC)::GetWindowLongPtrW(hwnd, GWLP_WNDPROC);
    IM_ASSERT(bd->PrevWndProc != nullptr);
    ::SetWindowLongPtrW((HWND)main_viewport->PlatformHandleRaw, GWLP_WNDPROC, (LONG_PTR)ImGui_ImplGlfw_WndProc);
#endif

    // Emscripten：同一个应用程序可以运行在各种平台上，因此我们在运行时检测Apple平台
    // 覆盖 io.ConfigMacOSXBehaviors 的默认值（在 Emscripten 中始终为 false）。
#ifdef __EMSCRIPTEN__
#if EMSCRIPTEN_USE_PORT_CONTRIB_GLFW3 >= 34020240817
    if (emscripten::glfw3::IsRuntimePlatformApple())
    {
        io.ConfigMacOSXBehaviors = true;

        // 由于浏览器处理元键的方式（很差），该行在使用时基本上禁用了重复。
        // 这意味着 Meta + V 仅记录一次按键，即使按住按键也是如此。
        // 这是在 ImGui 中处理此问题的折衷方案，因为 ImGui 本身实现了按键重复。
        // 请参阅https://github.com/pongasoft/emscripten-glfw/blob/v3.4.0.20240817/docs/Usage.md#the-problem-of-the-super-key
        emscripten::glfw3::SetSuperPlusKeyTimeouts(10, 10);
    }
#endif
#endif

    bd->ClientApi = client_api;
    return true;
}

bool ImGui_ImplGlfw_InitForOpenGL(GLFWwindow* window, bool install_callbacks)
{
    return ImGui_ImplGlfw_Init(window, install_callbacks, GlfwClientApi_OpenGL);
}

bool ImGui_ImplGlfw_InitForVulkan(GLFWwindow* window, bool install_callbacks)
{
    return ImGui_ImplGlfw_Init(window, install_callbacks, GlfwClientApi_Vulkan);
}

bool ImGui_ImplGlfw_InitForOther(GLFWwindow* window, bool install_callbacks)
{
    return ImGui_ImplGlfw_Init(window, install_callbacks, GlfwClientApi_Unknown);
}

void ImGui_ImplGlfw_Shutdown()
{
    ImGui_ImplGlfw_Data* bd = ImGui_ImplGlfw_GetBackendData();
    IM_ASSERT(bd != nullptr && "No platform backend to shutdown, or already shutdown?");
    ImGuiIO& io = ImGui::GetIO();

    if (bd->InstalledCallbacks)
        ImGui_ImplGlfw_RestoreCallbacks(bd->Window);
#ifdef EMSCRIPTEN_USE_EMBEDDED_GLFW3
    if (bd->CanvasSelector)
        emscripten_set_wheel_callback(bd->CanvasSelector, nullptr, false, nullptr);
#endif

    for (ImGuiMouseCursor cursor_n = 0; cursor_n < ImGuiMouseCursor_COUNT; cursor_n++)
        glfwDestroyCursor(bd->MouseCursors[cursor_n]);

    // Windows：恢复我们的 WndProc 钩子
#ifdef _WIN32
    ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    ::SetPropA((HWND)main_viewport->PlatformHandleRaw, "IMGUI_BACKEND_DATA", nullptr);
    ::SetWindowLongPtrW((HWND)main_viewport->PlatformHandleRaw, GWLP_WNDPROC, (LONG_PTR)bd->PrevWndProc);
    bd->PrevWndProc = nullptr;
#endif

    io.BackendPlatformName = nullptr;
    io.BackendPlatformUserData = nullptr;
    io.BackendFlags &= ~(ImGuiBackendFlags_HasMouseCursors | ImGuiBackendFlags_HasSetMousePos | ImGuiBackendFlags_HasGamepad);
    ImGui_ImplGlfw_ContextMap_Remove(bd->Window);
    IM_DELETE(bd);
}

static void ImGui_ImplGlfw_UpdateMouseData()
{
    ImGui_ImplGlfw_Data* bd = ImGui_ImplGlfw_GetBackendData();
    ImGuiIO& io = ImGui::GetIO();

    // （这些大括号在这里是为了减少“对接”分支中多视口支持的差异）
    {
        GLFWwindow* window = bd->Window;
#ifdef EMSCRIPTEN_USE_EMBEDDED_GLFW3
        const bool is_window_focused = true;
#else
        const bool is_window_focused = glfwGetWindowAttrib(window, GLFW_FOCUSED) != 0;
#endif
        if (is_window_focused)
        {
            // （可选）根据需要从 Dear ImGui 设置操作系统鼠标位置（很少使用，仅当用户启用 io.ConfigNavMoveSetMousePos 时）
            if (io.WantSetMousePos)
                glfwSetCursorPos(window, (double)io.MousePos.x, (double)io.MousePos.y);

            // （可选）后备以在聚焦时提供鼠标位置（ImGui_ImplGlfw_CursorPosCallback 在悬停或捕获时已提供此位置）
            if (bd->MouseWindow == nullptr)
            {
                double mouse_x, mouse_y;
                glfwGetCursorPos(window, &mouse_x, &mouse_y);
                bd->LastValidMousePos = ImVec2((float)mouse_x, (float)mouse_y);
                io.AddMousePosEvent((float)mouse_x, (float)mouse_y);
            }
        }
    }
}

static void ImGui_ImplGlfw_UpdateMouseCursor()
{
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplGlfw_Data* bd = ImGui_ImplGlfw_GetBackendData();
    if ((io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange) || glfwGetInputMode(bd->Window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
        return;

    ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
    // （这些大括号在这里是为了减少“对接”分支中多视口支持的差异）
    {
        GLFWwindow* window = bd->Window;
        if (imgui_cursor == ImGuiMouseCursor_None || io.MouseDrawCursor)
        {
            // 如果 imgui 正在绘制它或者不需要光标，则隐藏操作系统鼠标光标
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
        }
        else
        {
            // 显示操作系统鼠标光标
            // FIXME-平台：未聚焦的窗口似乎无法使用 GLFW 3.2 更改鼠标光标，但 3.3 在这里可以工作。
            glfwSetCursor(window, bd->MouseCursors[imgui_cursor] ? bd->MouseCursors[imgui_cursor] : bd->MouseCursors[ImGuiMouseCursor_Arrow]);
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
}

// 更新游戏手柄输入
static inline float Saturate(float v) { return v < 0.0f ? 0.0f : v  > 1.0f ? 1.0f : v; }
static void ImGui_ImplGlfw_UpdateGamepads()
{
    ImGuiIO& io = ImGui::GetIO();
    if ((io.ConfigFlags & ImGuiConfigFlags_NavEnableGamepad) == 0) // FIXME：从技术上讲，游戏手柄的输入不应该依赖于此，因为它们是常规输入，但请参阅#8075
        return;

    io.BackendFlags &= ~ImGuiBackendFlags_HasGamepad;
#if GLFW_HAS_GAMEPAD_API && !defined(EMSCRIPTEN_USE_EMBEDDED_GLFW3)
    GLFWgamepadstate gamepad;
    if (!glfwGetGamepadState(GLFW_JOYSTICK_1, &gamepad))
        return;
    #define MAP_BUTTON(KEY_NO, BUTTON_NO, _UNUSED)          do { io.AddKeyEvent(KEY_NO, gamepad.buttons[BUTTON_NO] != 0); } while (0)
    #define MAP_ANALOG(KEY_NO, AXIS_NO, _UNUSED, V0, V1)    do { float v = gamepad.axes[AXIS_NO]; v = (v - V0) / (V1 - V0); io.AddKeyAnalogEvent(KEY_NO, v > 0.10f, Saturate(v)); } while (0)
#else
    int axes_count = 0, buttons_count = 0;
    const float* axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axes_count);
    const unsigned char* buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &buttons_count);
    if (axes_count == 0 || buttons_count == 0)
        return;
    #define MAP_BUTTON(KEY_NO, _UNUSED, BUTTON_NO)          do { io.AddKeyEvent(KEY_NO, (buttons_count > BUTTON_NO && buttons[BUTTON_NO] == GLFW_PRESS)); } while (0)
    #define MAP_ANALOG(KEY_NO, _UNUSED, AXIS_NO, V0, V1)    do { float v = (axes_count > AXIS_NO) ? axes[AXIS_NO] : V0; v = (v - V0) / (V1 - V0); io.AddKeyAnalogEvent(KEY_NO, v > 0.10f, Saturate(v)); } while (0)
#endif
    io.BackendFlags |= ImGuiBackendFlags_HasGamepad;
    MAP_BUTTON(ImGuiKey_GamepadStart,       GLFW_GAMEPAD_BUTTON_START,          7);
    MAP_BUTTON(ImGuiKey_GamepadBack,        GLFW_GAMEPAD_BUTTON_BACK,           6);
    MAP_BUTTON(ImGuiKey_GamepadFaceLeft,    GLFW_GAMEPAD_BUTTON_X,              2);     // Xbox X、PS 广场
    MAP_BUTTON(ImGuiKey_GamepadFaceRight,   GLFW_GAMEPAD_BUTTON_B,              1);     // Xbox B、PS 圈
    MAP_BUTTON(ImGuiKey_GamepadFaceUp,      GLFW_GAMEPAD_BUTTON_Y,              3);     // Xbox Y、PS 三角形
    MAP_BUTTON(ImGuiKey_GamepadFaceDown,    GLFW_GAMEPAD_BUTTON_A,              0);     // Xbox A、PS Cross
    MAP_BUTTON(ImGuiKey_GamepadDpadLeft,    GLFW_GAMEPAD_BUTTON_DPAD_LEFT,      13);
    MAP_BUTTON(ImGuiKey_GamepadDpadRight,   GLFW_GAMEPAD_BUTTON_DPAD_RIGHT,     11);
    MAP_BUTTON(ImGuiKey_GamepadDpadUp,      GLFW_GAMEPAD_BUTTON_DPAD_UP,        10);
    MAP_BUTTON(ImGuiKey_GamepadDpadDown,    GLFW_GAMEPAD_BUTTON_DPAD_DOWN,      12);
    MAP_BUTTON(ImGuiKey_GamepadL1,          GLFW_GAMEPAD_BUTTON_LEFT_BUMPER,    4);
    MAP_BUTTON(ImGuiKey_GamepadR1,          GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER,   5);
    MAP_ANALOG(ImGuiKey_GamepadL2,          GLFW_GAMEPAD_AXIS_LEFT_TRIGGER,     4,      -0.75f,  +1.0f);
    MAP_ANALOG(ImGuiKey_GamepadR2,          GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER,    5,      -0.75f,  +1.0f);
    MAP_BUTTON(ImGuiKey_GamepadL3,          GLFW_GAMEPAD_BUTTON_LEFT_THUMB,     8);
    MAP_BUTTON(ImGuiKey_GamepadR3,          GLFW_GAMEPAD_BUTTON_RIGHT_THUMB,    9);
    MAP_ANALOG(ImGuiKey_GamepadLStickLeft,  GLFW_GAMEPAD_AXIS_LEFT_X,           0,      -0.25f,  -1.0f);
    MAP_ANALOG(ImGuiKey_GamepadLStickRight, GLFW_GAMEPAD_AXIS_LEFT_X,           0,      +0.25f,  +1.0f);
    MAP_ANALOG(ImGuiKey_GamepadLStickUp,    GLFW_GAMEPAD_AXIS_LEFT_Y,           1,      -0.25f,  -1.0f);
    MAP_ANALOG(ImGuiKey_GamepadLStickDown,  GLFW_GAMEPAD_AXIS_LEFT_Y,           1,      +0.25f,  +1.0f);
    MAP_ANALOG(ImGuiKey_GamepadRStickLeft,  GLFW_GAMEPAD_AXIS_RIGHT_X,          2,      -0.25f,  -1.0f);
    MAP_ANALOG(ImGuiKey_GamepadRStickRight, GLFW_GAMEPAD_AXIS_RIGHT_X,          2,      +0.25f,  +1.0f);
    MAP_ANALOG(ImGuiKey_GamepadRStickUp,    GLFW_GAMEPAD_AXIS_RIGHT_Y,          3,      -0.25f,  -1.0f);
    MAP_ANALOG(ImGuiKey_GamepadRStickDown,  GLFW_GAMEPAD_AXIS_RIGHT_Y,          3,      +0.25f,  +1.0f);
    #undef MAP_BUTTON
    #undef MAP_ANALOG
}

// -在 Windows 上，进程需要标记为 DPI 感知！！ SDL2 默认情况下不这样做。您可以从 Win32 后端调用 ::SetProcessDPIAware() 或调用 ImGui_ImplWin32_EnableDpiAwareness()。
// -Apple 平台使用 FramebufferScale，因此我们始终返回 1.0f。
// -一些辅助功能应用程序声明 DPI 为 0.0f 的虚拟监视器，请参阅#7902。我们保留这个值供调用者处理。
float ImGui_ImplGlfw_GetContentScaleForWindow(GLFWwindow* window)
{
#if GLFW_HAS_PER_MONITOR_DPI && !defined(__APPLE__)
    float x_scale, y_scale;
    glfwGetWindowContentScale(window, &x_scale, &y_scale);
    return x_scale;
#else
    IM_UNUSED(window);
    return 1.0f;
#endif
}

float ImGui_ImplGlfw_GetContentScaleForMonitor(GLFWmonitor* monitor)
{
#if GLFW_HAS_PER_MONITOR_DPI && !defined(__APPLE__)
    float x_scale, y_scale;
    glfwGetMonitorContentScale(monitor, &x_scale, &y_scale);
    return x_scale;
#else
    IM_UNUSED(monitor);
    return 1.0f;
#endif
}

static void ImGui_ImplGlfw_GetWindowSizeAndFramebufferScale(GLFWwindow* window, ImVec2* out_size, ImVec2* out_framebuffer_scale)
{
    int w, h;
    int display_w, display_h;
    glfwGetWindowSize(window, &w, &h);
    glfwGetFramebufferSize(window, &display_w, &display_h);
    if (out_size != nullptr)
        *out_size = ImVec2((float)w, (float)h);
    if (out_framebuffer_scale != nullptr)
        *out_framebuffer_scale = (w > 0 && h > 0) ? ImVec2((float)display_w / (float)w, (float)display_h / (float)h) : ImVec2(1.0f, 1.0f);
}

void ImGui_ImplGlfw_NewFrame()
{
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplGlfw_Data* bd = ImGui_ImplGlfw_GetBackendData();
    IM_ASSERT(bd != nullptr && "Context or backend not initialized! Did you call ImGui_ImplGlfw_InitForXXX()?");

    // 设置主视口大小（每个帧都适应窗口大小调整）
    ImGui_ImplGlfw_GetWindowSizeAndFramebufferScale(bd->Window, &io.DisplaySize, &io.DisplayFramebufferScale);

    // 设置时间步长
    // （接受 glfwGetTime() 不返回单调递增的值。似乎发生在断开外围设备时，可能发生在虚拟机和 Emscripten 上，请参阅#6491、#6189、#6114、#3644）
    double current_time = glfwGetTime();
    if (current_time <= bd->Time)
        current_time = bd->Time + 0.00001f;
    io.DeltaTime = bd->Time > 0.0 ? (float)(current_time - bd->Time) : (float)(1.0f / 60.0f);
    bd->Time = current_time;

    ImGui_ImplGlfw_UpdateMouseData();
    ImGui_ImplGlfw_UpdateMouseCursor();

    // 更新游戏控制器（如果启用且可用）
    ImGui_ImplGlfw_UpdateGamepads();
}

// GLFW不提供便携式睡眠功能
void ImGui_ImplGlfw_Sleep(int milliseconds)
{
#ifdef _WIN32
    ::Sleep(milliseconds);
#else
    usleep(milliseconds * 1000);
#endif
}

#ifdef EMSCRIPTEN_USE_EMBEDDED_GLFW3
static EM_BOOL ImGui_ImplGlfw_OnCanvasSizeChange(int event_type, const EmscriptenUiEvent* event, void* user_data)
{
    ImGui_ImplGlfw_Data* bd = (ImGui_ImplGlfw_Data*)user_data;
    double canvas_width, canvas_height;
    emscripten_get_element_css_size(bd->CanvasSelector, &canvas_width, &canvas_height);
    glfwSetWindowSize(bd->Window, (int)canvas_width, (int)canvas_height);
    return true;
}

static EM_BOOL ImGui_ImplEmscripten_FullscreenChangeCallback(int event_type, const EmscriptenFullscreenChangeEvent* event, void* user_data)
{
    ImGui_ImplGlfw_Data* bd = (ImGui_ImplGlfw_Data*)user_data;
    double canvas_width, canvas_height;
    emscripten_get_element_css_size(bd->CanvasSelector, &canvas_width, &canvas_height);
    glfwSetWindowSize(bd->Window, (int)canvas_width, (int)canvas_height);
    return true;
}

// 'canvas_selector' 是一个 CSS 选择器。事件侦听器应用于与查询匹配的第一个元素。
// 字符串必须在应用程序持续时间内持续存在。请使用字符串文字或确保指针保持有效。
void ImGui_ImplGlfw_InstallEmscriptenCallbacks(GLFWwindow*, const char* canvas_selector)
{
    IM_ASSERT(canvas_selector != nullptr);
    ImGui_ImplGlfw_Data* bd = ImGui_ImplGlfw_GetBackendData();
    IM_ASSERT(bd != nullptr && "Context or backend not initialized! Did you call ImGui_ImplGlfw_InitForXXX()?");

    bd->CanvasSelector = canvas_selector;
    emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, bd, false, ImGui_ImplGlfw_OnCanvasSizeChange);
    emscripten_set_fullscreenchange_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, bd, false, ImGui_ImplEmscripten_FullscreenChangeCallback);

    // 根据画布的大小改变GLFW窗口的大小
    ImGui_ImplGlfw_OnCanvasSizeChange(EMSCRIPTEN_EVENT_RESIZE, {}, bd);

    // 注册 Emscripten Wheel 回调以解决 Emscripten GLFW 仿真中的问题 (#6096)
    // 我们故意不在这里检查“if (install_callbacks)”，因为某些用户可能会将其设置为 false 并自行调用 GLFW 回调。
    // FIXME：如果用户注册自己的 Emscripten 回调，可能会中断链接吗？
    emscripten_set_wheel_callback(bd->CanvasSelector, bd, false, ImGui_ImplEmscripten_WheelCallback);
}
#elif defined(EMSCRIPTEN_USE_PORT_CONTRIB_GLFW3)
// 当使用 --use-port=contrib.glfw3 进行 GLFW 实现时，您可以覆盖此调用的行为
// 之后调用 emscripten_glfw_make_canvas_resizing 。
// 有关说明，请参阅 https://github.com/pongasoft/emscripten-glfw/blob/master/docs/Usage.md#how-to-make-the-canvas-ressized-by-the-user
void ImGui_ImplGlfw_InstallEmscriptenCallbacks(GLFWwindow* window, const char* canvas_selector)
{
  GLFWwindow* w = (GLFWwindow*)(EM_ASM_INT({ return Module.glfwGetWindow(UTF8ToString($0)); }, canvas_selector));
  IM_ASSERT(window == w); // 健全性检查
  IM_UNUSED(w);
  emscripten_glfw_make_canvas_resizable(window, "window", nullptr);
}
#endif // #ifdef EMSCRIPTEN_USE_PORT_CONTRIB_GLFW3

//-----------------------------------------------------------------------------

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

#endif // #ifndef IMGUI_DISABLE
