// 亲爱的 imgui：带有着色器/编程管道的现代 OpenGL 渲染器后端
// -桌面 GL：2.x 3.x 4.x
// -嵌入式 GL：ES 2.0 (WebGL 1.0)、ES 3.0 (WebGL 2.0)
// 这需要与平台后端（例如 GLFW、SDL、Win32、自定义......）一起使用

// 实现的功能：
//  [X] 渲染器：用户纹理绑定。使用“GLuint”OpenGL 纹理作为纹理标识符。阅读有关 ImTextureID/ImTextureRef 的常见问题解答！
//  [x] 渲染器：即使使用 16 位索引，也支持大型网格（64k+ 顶点）（ImGuiBackendFlags_RendererHasVtxOffset）[仅限桌面 OpenGL！]
//  [X] 渲染器：纹理更新对动态字体图集的支持 (ImGuiBackendFlags_RendererHasTextures)。

// 关于 WebGL/ES：
// -您需要“#define IMGUI_IMPL_OPENGL_ES2”或“#define IMGUI_IMPL_OPENGL_ES3”才能使用WebGL或OpenGL ES。
// -这是在 iOS、Android 和 Emscripten 目标上自动完成的。
// -对于其他目标，定义需要在 imgui_impl_opengl3.cpp 编译单元中可见。如果不确定，请全局定义或在 imconfig.h 中定义。

// 您可以在项目中使用未修改的 imgui_impl_*文件。有关使用此示例的示例，请参阅示例/文件夹。
// 最好将整个 imgui/存储库包含到您的项目中（作为副本或子模块），并且仅构建您需要的后端。
// 了解亲爱的 ImGui：
// -常见问题解答 https://dearimgui.com/faq
// -入门 https://dearimgui.com/getting-started
// -文档 https://dearimgui.com/docs（与本地 docs/文件夹相同）。
// -imgui.cpp 顶部的介绍、链接等

// 关于GLSL版本：
//  “glsl_version”初始化参数应为 nullptr（默认）或“#version XXX”字符串。
//  在计算机平台上，GLSL 版本默认为“#version 130”。在 OpenGL ES 3 平台上，默认为“#version 300 es”
//  仅当您的 GL 版本不处理此 GLSL 版本时才覆盖。请参阅 imgui_impl_opengl3.cpp 顶部的 GLSL 版本表。

#pragma once
#include "imgui.h"      // Imgui 实现 API
#ifndef IMGUI_DISABLE

// 点击“入门”链接并检查示例/文件夹以了解如何使用后端！
IMGUI_IMPL_API bool     ImGui_ImplOpenGL3_Init(const char* glsl_version = nullptr);
IMGUI_IMPL_API void     ImGui_ImplOpenGL3_Shutdown();
IMGUI_IMPL_API void     ImGui_ImplOpenGL3_NewFrame();
IMGUI_IMPL_API void     ImGui_ImplOpenGL3_RenderDrawData(ImDrawData* draw_data);

// （可选）由Init/NewFrame/Shutdown调用
IMGUI_IMPL_API bool     ImGui_ImplOpenGL3_CreateDeviceObjects();
IMGUI_IMPL_API void     ImGui_ImplOpenGL3_DestroyDeviceObjects();

// （高级）使用例如如果您需要精确控制纹理更新的时间（例如，用于分阶段渲染），可以通过设置 ImDrawData::Textures = NULL 来手动处理。
IMGUI_IMPL_API void     ImGui_ImplOpenGL3_UpdateTexture(ImTextureData* tex);

// 要添加到 imconfig 文件中的配置标志：
//#define IMGUI_IMPL_OPENGL_ES2 //启用 ES 2（在 Emscripten 上自动检测）
//#define IMGUI_IMPL_OPENGL_ES3 //启用 ES 3（在 iOS/Android 上自动检测）

// 您可以使用 imconfig.h 或编译器命令行中的“#define IMGUI_IMPL_OPENGL_LOADER_XXX”之一显式选择 GLES2 或 GLES3 API。
#if !defined(IMGUI_IMPL_OPENGL_ES2) \
 && !defined(IMGUI_IMPL_OPENGL_ES3)

// 尝试在匹配平台上检测 GLES
#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif
#if (defined(__APPLE__) && (TARGET_OS_IOS || TARGET_OS_TV)) || (defined(__ANDROID__))
#define IMGUI_IMPL_OPENGL_ES3               // iOS、Android -> GL ES 3，“#version 300 是”
#elif defined(__EMSCRIPTEN__) || defined(__amigaos4__)
#define IMGUI_IMPL_OPENGL_ES2               // Emscripten -> GL ES 2，“#版本 100”
#else
// 否则将使用imgui_impl_opengl3_loader.h。
#endif

#endif

#endif // #ifndef IMGUI_DISABLE
