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

// 变更日志
// （次要和较旧的更改已被删除，请参阅 git 历史记录了解详细信息）
//  2025-06-11：OpenGL：添加了对 ImGuiBackendFlags_RendererHasTextures 的支持，用于动态字体图集。删除了 ImGui_ImplOpenGL3_CreateFontsTexture() 和 ImGui_ImplOpenGL3_DestroyFontsTexture()。
//  2025-06-04：OpenGL：使 GLES 3.20 上下文无法访问 GL_CONTEXT_PROFILE_MASK 或 GL_PRIMITIVE_RESTART。 (#8664)
//  2025-02-18：OpenGL：从例如调用后端时延迟重新初始化嵌入式 GL 加载程序其他 DLL 边界。 (#8406)
//  2024-10-07：OpenGL：将默认纹理采样器更改为“夹紧”而不是“重复/环绕”。
//  2024-06-28：OpenGL：如果字体纹理已被 ImGui_ImplOpenGL3_DestroyFontsTexture() 破坏，则 ImGui_ImplOpenGL3_NewFrame() 会重新创建字体纹理。 (#7748)
//  2024-05-07：OpenGL：更新 Linux 加载程序以支持 EGL/GLVND。 (#7562)
//  2024-04-16：OpenGL：根据版本字符串检测桌面上的 ES3 上下文，例如避免对它们调用 glPolygonMode()。 (#7447)
//  2024-01-09：OpenGL：更新基于 GL3W 的 imgui_impl_opengl3_loader.h 以加载“libGL.so”和变体，修复缺少符号链接的发行版上的回归。
//  2023-11-08：OpenGL：更新基于 GL3W 的 imgui_impl_opengl3_loader.h 以加载“libGL.so”而不是“libGL.so.1”，以适应仅具有“libGL.so.3”可用的 NetBSD 系统。 (#6983)
//  2023-10-05：OpenGL：重命名内部加载器中的符号，以便可以使用 gl3w 的另一个副本进行 LTO 编译。 （#6875、#6668、#4445）
//  2023-06-20：OpenGL：修复了低于 3.2 的上下文中错误使用 glGetIntegerv(GL_CONTEXT_PROFILE_MASK) 的问题。 （＃6539，＃6333）
//  2023-05-09：OpenGL：支持 ES3 上的 glBindSampler() 备份/恢复。 (#6375)
//  2023-04-18：OpenGL：在上下文支持时分别恢复前后多边形模式。 (#6333)
//  2023-03-23：OpenGL：如果在运行渲染函数之前是这种情况，则正确恢复“无着色器程序绑定”。 （#6267、#6220、#6224）
//  2023-03-15：OpenGL：修复了 GL_VERSION 返回 NULL 时 GL 加载器崩溃的问题。 （#6154、#4445、#3530）
//  2023-03-06：OpenGL：通过调用 glIsProgram() 修复了可能删除的 OpenGL 程序的恢复。 （＃6220，＃6224）
//  2022-11-09：OpenGL：恢复使用 glBufferSubData()，太多损坏问题 + 旧问题似乎无法通过当今的英特尔驱动程序重现（恢复 2021-12-15 和 2022-05-23 更改）。
//  2022 年 10 月 11 日：根据我们切换到 C++11 的要求，使用“nullptr”而不是“NULL”。
//  2022-09-27：OpenGL：添加了“#define IMGUI_IMPL_OPENGL_DEBUG”的功能。
//  2022-05-23：OpenGL：返工2021-12-15“使用缓冲区孤立”，因此它只发生在Intel GPU上，否则似乎会导致问题。 （#4468、#4825、#4832、#5127）。
//  2022-05-13：OpenGL：修复了 OpenGL ES 2.0 上由于未保留 GL_ELEMENT_ARRAY_BUFFER_BINDING 和顶点属性状态而导致的状态损坏。
//  2021-12-15：OpenGL：使用缓冲区孤立 + glBufferSubData()，似乎修复了某些英特尔高清驱动程序的多视口泄漏。
//  2021-08-23：OpenGL：修复了 ES 3.0 着色器（“#version 300 es”）使用正常精度浮点以避免在高清分辨率下渲染不稳定。
//  2021-08-19：OpenGL：嵌入并使用我们自己的最小 GL 加载程序 (imgui_impl_opengl3_loader.h)，删除对第三方加载程序的要求和支持。
//  2021-06-29：重组后端以从单个结构中提取数据，以方便在多个上下文中使用（所有 g_XXXX 访问更改为 bd->XXXX）。
//  2021-06-25：OpenGL：在 Emscripten 上使用 OES_vertex_array 扩展 + 备份/恢复当前状态。
//  2021-06-21：OpenGL：在链接到主着色器后立即销毁各个顶点/片段着色器对象。
//  2021-05-24：OpenGL：在 OpenGL 4.5 版本内检测到“GL_ARB_clip_control”扩展时访问 GL_CLIP_ORIGIN。
//  2021-05-19：OpenGL：通过调用 ImDrawCmd::GetTexID() 替换了对 ImDrawCmd::TextureId 的直接访问。 （将成为一项要求）
//  2021-04-06：OpenGL：不要尝试读取 GL_CLIP_ORIGIN，除非我们是 OpenGL 4.5 或更高版本。
//  2021-02-18：OpenGL：更改混合方程以在输出缓冲区中保留 Alpha。
//  2021-01-03：OpenGL：备份、设置和恢复 GL_STENCIL_TEST 状态。
//  2020-10-23：OpenGL：备份、设置和恢复 GL_PRIMITIVE_RESTART 状态。
//  2020-10-15：OpenGL：当后者返回零时（例如桌面 GL 2.x），使用 glGetString(GL_VERSION) 而不是 glGetIntegerv(GL_MAJOR_VERSION, ...)
//  2020-09-17：OpenGL：修复以避免在具有加载程序设置的定义的 ES 或 3.3 之前的上下文上编译/调用 glBindSampler()。
//  2020-07-10：OpenGL：添加了对glad2 OpenGL加载器的支持。
//  2020-05-08：OpenGL：在 OSX 上设置默认 GLSL 版本 150（而不是 130）。
//  2020-04-21：OpenGL：通过反转投影矩阵修复了 glClipControl(GL_UPPER_LEFT) 的处理。
//  2020-04-12：OpenGL：修复了上下文版本检查错误地测试 4.0+ 而不是 3.2+ 以启用 ImGuiBackendFlags_RendererHasVtxOffset。
//  2020-03-24：OpenGL：添加了对 glbinding 2.x OpenGL 加载器的支持。
//  2020-01-07：OpenGL：添加了对 glbinding 3.x OpenGL 加载程序的支持。
//  2019-10-25：OpenGL：使用 GL 定义和运行时 GL 版本的组合来决定是否使用 glDrawElementsBaseVertex()。使用 3.2 之前的 GL 加载程序修复构建。
//  2019-09-22：OpenGL：使用 __has_include 编译器工具检测默认 GL 加载器。
//  2019-09-16：OpenGL：调整初始化代码以允许应用程序在第一次 NewFrame() 调用之前调用 ImGui_ImplOpenGL3_CreateFontsTexture()。
//  2019-05-29：OpenGL：仅限桌面 GL：添加了对大网格（64K+ 顶点）的支持，启用 ImGuiBackendFlags_RendererHasVtxOffset 标志。
//  2019-04-30：OpenGL：添加了对特殊 ImDrawCallback_ResetRenderState 回调的支持以重置渲染状态。
//  2019-03-29：OpenGL：在渲染循环中不调用超出必要范围的 glBindBuffer。
//  2019-03-15：OpenGL：在 ImGui_ImplOpenGL3_Init() 中添加了 GL 调用 + 注释，以尽早检测未初始化的 GL 函数加载器。
//  2019-03-03：OpenGL：修复对 ES 2.0 (WebGL 1.0) 的支持。
//  2019-02-20：OpenGL：修复了 OSX 不支持 OpenGL 4.5 的问题，即使由 headers/loader 定义，我们也不会尝试读取 GL_CLIP_ORIGIN。
//  2019-02-11：OpenGL：使用draw_data->FramebufferScale正确投影剪切矩形，以允许视网膜显示的多视口。
//  2019-02-01：OpenGL：对 410 以上的任何版本（例如 430、450）使用 GLSL 410 着色器。
//  2018-11-30：杂项：设置 io.BackendRendererName，以便它可以显示在“关于”窗口中。
//  2018-11-13：OpenGL：支持 GL 4.5 的 glClipControl(GL_UPPER_LEFT) /GL_CLIP_ORIGIN。
//  2018-08-29：OpenGL：添加了对更多 OpenGL 加载器的支持：glew 和 happy，注释表明可以使用任何加载器。
//  2018-08-09：OpenGL：iOS 和 Android 上默认为 OpenGL ES 3。 GLSL 版本默认为“#version 300 ES”。
//  2018-07-30：OpenGL：支持 GLSL 300 ES 和 410 内核。修复了 Emscripten 编译问题。
//  2018-07-10：OpenGL：支持更多GLSL版本（基于GLSL版本字符串）。添加了着色器无法编译/链接时的错误输出。
//  2018-06-08：杂项：从旧的组合 GLFW/SDL+OpenGL3 示例中提取 imgui_impl_opengl3.cpp/.h。
//  2018-06-08：OpenGL：使用draw_data-> DisplayPos和draw_data-> DisplaySize设置投影矩阵和剪切矩形。
//  2018-05-25：OpenGL：删除了不必要的 GL_ELEMENT_ARRAY_BUFFER_BINDING 备份/恢复，因为这是 VAO 状态的一部分。
//  2018-05-14：OpenGL：使对 glBindSampler() 的调用成为可选，因此如果函数是 nullptr 指针，3.2 上下文不会失败。
//  2018-03-06：OpenGL：向 ImGui_ImplOpenGL3_Init() 添加了 const char*glsl_version 参数，以便用户可以覆盖 GLSL 版本，例如“#版本 150”。
//  2018-02-23：OpenGL：在渲染函数中创建 VAO，以便设置可以更轻松地与多个共享 GL 上下文一起使用。
//  2018-02-16：其他：废弃了 io.RenderDrawListsFn 回调并在 .h 文件中公开了 ImGui_ImplSdlGL3_RenderDrawData() ，以便您可以自己调用它。
//  2018-01-07：OpenGL：将 GLSL 着色器版本从 330 更改为 150。
//  2017-09-01：OpenGL：保存和恢复当前绑定的采样器。保存和恢复当前多边形模式。
//  2017-05-01：OpenGL：修复了当前混合函数状态的保存和恢复。
//  2017-05-01：OpenGL：修复了当前 GL_ACTIVE_TEXTURE 的保存和恢复。
//  2016-09-05：OpenGL：修复了当前剪刀矩形的保存和恢复。
//  2016-07-29：OpenGL：显式设置 GL_UNPACK_ROW_LENGTH 以减少问题，因为 SDL 更改了它。 （#752）

//----------------------------------------
// OpenGL GLSL GLSL
// 版本 版本字符串
//----------------------------------------
//  2.0 110“#版本110”
//  2.1 120“#版本120”
//  3.0 130“#版本130”
//  3.1 140“#版本140”
//  3.2 150“#版本150”
//  3.3 330“#版本330核心”
//  4.0 400“#版本400核心”
//  4.1 410“#版本410核心”
//  4.2 420“#版本410核心”
//  4.3 430“#版本430核心”
//  ES 2.0 100“#version 100”= WebGL 1.0
//  ES 3.0 300“#version 300 es”= WebGL 2.0
//----------------------------------------

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "imgui.h"
#ifndef IMGUI_DISABLE
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#include <stdint.h> // INTPTTR T
#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

// 使用 -Weverything 发出 Clang/GCC 警告
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-warning-option" // 警告：忽略未知标志
#pragma clang diagnostic ignored "-Wold-style-cast"         // 警告：使用旧式强制转换
#pragma clang diagnostic ignored "-Wsign-conversion"        // 警告：隐式转换会更改符号性
#pragma clang diagnostic ignored "-Wunused-macros"          // 警告：未使用宏
#pragma clang diagnostic ignored "-Wnonportable-system-include-path"
#pragma clang diagnostic ignored "-Wcast-function-type"     // 警告：在不兼容的函数类型之间进行转换（对于加载程序）
#endif
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"                // 警告：“#pragma GCC Diagnostic”类型后的未知选项
#pragma GCC diagnostic ignored "-Wunknown-warning-option" // 警告：未知警告组“xxx”
#pragma GCC diagnostic ignored "-Wcast-function-type"     // 警告：在不兼容的函数类型之间进行转换（对于加载程序）
#pragma GCC diagnostic ignored                                                                                         \
    "-Wstrict-overflow" // 警告：假设简化除法时不会发生有符号溢出/..将 X +-C1 cmp C2 更改为 X cmp C2 -+ C1 时
#endif

// 总账包括
#if defined(IMGUI_IMPL_OPENGL_ES2)
#if (defined(__APPLE__) && (TARGET_OS_IOS || TARGET_OS_TV))
#include <OpenGLES/ES2/gl.h> // 使用 GL ES 2
#else
#include <GLES2/gl2.h>       // 使用 GL ES 2
#endif
#if defined(__EMSCRIPTEN__)
#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif
#include <GLES2/gl2ext.h>
#endif
#elif defined(IMGUI_IMPL_OPENGL_ES3)
#if (defined(__APPLE__) && (TARGET_OS_IOS || TARGET_OS_TV))
#include <OpenGLES/ES3/gl.h> // 使用 GL ES 3
#else
#include <GLES3/gl3.h>       // 使用 GL ES 3
#endif
#elif !defined(IMGUI_IMPL_OPENGL_LOADER_CUSTOM)
// 现代桌面 OpenGL 没有标准的可移植头文件来加载 OpenGL 函数指针。
// 辅助库通常用于此目的！这里我们使用我们自己的基于 gl3w 的最小自定义加载器。
// 在应用程序/引擎的其余部分中，您可以使用您选择的另一个加载器（gl3w、glew、glad、glbing、glext、glLoadGen 等）。
// 如果您碰巧正在为此后端开发新功能（imgui_impl_opengl3.cpp）：
// -您可能需要重新生成 imgui_impl_opengl3_loader.h 才能添加新符号。请参阅 https://github.com/dearimgui/gl3w_stripped
//   通常你会运行： python3 ./gl3w_gen.py --output ../imgui/backends/imgui_impl_opengl3_loader.h --ref ../imgui/backends/imgui_impl_opengl3.cpp ./extra_symbols.txt
// -您可以暂时使用未剥离的版本。请参阅 https://github.com/dearimgui/gl3w_stripped/releases
// 使用新 API 对此后端的更改应伴随重新生成的剥离加载程序版本。
#define IMGL3W_IMPL
#define IMGUI_IMPL_OPENGL_LOADER_IMGL3W
#include "imgui_impl_opengl3_loader.h"

// 这里是我们自己加的代码, 我们加了 IMGUI_IMPL_OPENGL_LOADER_CUSTOM 宏
// 因此这里我们使用自己的 OpenGL 加载器 glad
#else

#include "glad/glad.h"  // 使用自定义 GL 加载器
#include "GLFW/glfw3.h" // 使用系统 OpenGL 头文件

#endif

// ES2/WebGL1 不支持顶点数组，除非使用扩展的 Emscripten
#ifndef IMGUI_IMPL_OPENGL_ES2
#define IMGUI_IMPL_OPENGL_USE_VERTEX_ARRAY
#elif defined(__EMSCRIPTEN__)
#define IMGUI_IMPL_OPENGL_USE_VERTEX_ARRAY
#define glBindVertexArray glBindVertexArrayOES
#define glGenVertexArrays glGenVertexArraysOES
#define glDeleteVertexArrays glDeleteVertexArraysOES
#define GL_VERTEX_ARRAY_BINDING GL_VERTEX_ARRAY_BINDING_OES
#endif

// Desktop GL 2.0+ 具有 GL ES 和 WebGL 所没有的扩展和 glPolygonMode()。
// 从技术上讲，桌面 ES 上下文可以使用我们的加载器进行良好编译，因此我们还执行运行时检查
#if !defined(IMGUI_IMPL_OPENGL_ES2) && !defined(IMGUI_IMPL_OPENGL_ES3)
#define IMGUI_IMPL_OPENGL_HAS_EXTENSIONS        // 有 glGetIntegerv(GL_NUM_EXTENSIONS)
#define IMGUI_IMPL_OPENGL_MAY_HAVE_POLYGON_MODE // 可能有 glPolygonMode()
#endif

// 桌面 GL 2.1+ 和 GL ES 3.0+ 具有带有 GL_PIXEL_UNPACK_BUFFER 目标的 glBindBuffer()。
#if !defined(IMGUI_IMPL_OPENGL_ES2)
#define IMGUI_IMPL_OPENGL_MAY_HAVE_BIND_BUFFER_PIXEL_UNPACK
#endif

// 桌面 GL 3.1+ 具有 GL_PRIMITIVE_RESTART 状态
#if !defined(IMGUI_IMPL_OPENGL_ES2) && !defined(IMGUI_IMPL_OPENGL_ES3) && defined(GL_VERSION_3_1)
#define IMGUI_IMPL_OPENGL_MAY_HAVE_PRIMITIVE_RESTART
#endif

// Desktop GL 3.2+ 具有 GL ES 和 WebGL 所没有的 glDrawElementsBaseVertex()。
#if !defined(IMGUI_IMPL_OPENGL_ES2) && !defined(IMGUI_IMPL_OPENGL_ES3) && defined(GL_VERSION_3_2)
#define IMGUI_IMPL_OPENGL_MAY_HAVE_VTX_OFFSET
#endif

// 桌面 GL 3.3+ 和 GL ES 3.0+ 有 glBindSampler()
#if !defined(IMGUI_IMPL_OPENGL_ES2) && (defined(IMGUI_IMPL_OPENGL_ES3) || defined(GL_VERSION_3_3))
#define IMGUI_IMPL_OPENGL_MAY_HAVE_BIND_SAMPLER
#endif

// [调试]
//#定义 IMGUI_IMPL_OPENGL_DEBUG
#ifdef IMGUI_IMPL_OPENGL_DEBUG
#include <stdio.h>
#define GL_CALL(_CALL)                                                                                                 \
    do {                                                                                                               \
        _CALL;                                                                                                         \
        GLenum gl_err = glGetError();                                                                                  \
        if (gl_err != 0) fprintf(stderr, "GL error 0x%x returned from '%s'.\n", gl_err, #_CALL);                       \
    } while (0)              // 调用并检查错误
#else
#define GL_CALL(_CALL) _CALL // 调用时不进行错误检查
#endif

// OpenGL数据
struct ImGui_ImplOpenGL3_Data
{
    GLuint GlVersion;           // 使用 GL_MAJOR_VERSION、GL_MINOR_VERSION 查询在运行时提取（例如 GL 3.2 为 320）
    char GlslVersionString[32]; // 由用户指定或根据编译时 GL 设置检测。
    bool GlProfileIsES2;
    bool GlProfileIsES3;
    bool GlProfileIsCompat;
    GLint GlProfileMask;
    GLint MaxTextureSize;
    GLuint ShaderHandle;
    GLint AttribLocationTex;     // 制服地点
    GLint AttribLocationProjMtx;
    GLuint AttribLocationVtxPos; // 顶点属性位置
    GLuint AttribLocationVtxUV;
    GLuint AttribLocationVtxColor;
    unsigned int VboHandle, ElementsHandle;
    GLsizeiptr VertexBufferSize;
    GLsizeiptr IndexBufferSize;
    bool HasPolygonMode;
    bool HasClipOrigin;
    bool UseBufferSubData;
    ImVector<char> TempBuffer;

    ImGui_ImplOpenGL3_Data() { memset((void*)this, 0, sizeof(*this)); }
};

// 后端数据存储在 io.BackendRendererUserData 中，以支持多个 Dear ImGui 上下文
// 强烈建议您使用具有多视口的对接分支（==单个 Dear ImGui 上下文 + 多个窗口），而不是多个 Dear ImGui 上下文。
static ImGui_ImplOpenGL3_Data* ImGui_ImplOpenGL3_GetBackendData() {
    return ImGui::GetCurrentContext() ? (ImGui_ImplOpenGL3_Data*)ImGui::GetIO().BackendRendererUserData : nullptr;
}

// OpenGL顶点属性状态（仅适用于ES 1.0和ES 2.0）
#ifndef IMGUI_IMPL_OPENGL_USE_VERTEX_ARRAY
struct ImGui_ImplOpenGL3_VtxAttribState
{
    GLint Enabled, Size, Type, Normalized, Stride;
    GLvoid* Ptr;

    void GetState(GLint index) {
        glGetVertexAttribiv(index, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &Enabled);
        glGetVertexAttribiv(index, GL_VERTEX_ATTRIB_ARRAY_SIZE, &Size);
        glGetVertexAttribiv(index, GL_VERTEX_ATTRIB_ARRAY_TYPE, &Type);
        glGetVertexAttribiv(index, GL_VERTEX_ATTRIB_ARRAY_NORMALIZED, &Normalized);
        glGetVertexAttribiv(index, GL_VERTEX_ATTRIB_ARRAY_STRIDE, &Stride);
        glGetVertexAttribPointerv(index, GL_VERTEX_ATTRIB_ARRAY_POINTER, &Ptr);
    }
    void SetState(GLint index) {
        glVertexAttribPointer(index, Size, Type, (GLboolean)Normalized, Stride, Ptr);
        if (Enabled)
            glEnableVertexAttribArray(index);
        else
            glDisableVertexAttribArray(index);
    }
};
#endif

// 不是静态的，允许第三方代码使用（但未记录）
bool ImGui_ImplOpenGL3_InitLoader();
bool ImGui_ImplOpenGL3_InitLoader() {
    // 初始化我们的加载器
#ifdef IMGUI_IMPL_OPENGL_LOADER_IMGL3W
    if (glGetIntegerv == nullptr && imgl3wInit() != 0) {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return false;
    }
#elif defined(IMGUI_IMPL_OPENGL_LOADER_CUSTOM)
    // 使用 glad 来加载 OpenGL
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        return false;
    }
#endif
    return true;
}

// 功能
bool ImGui_ImplOpenGL3_Init(const char* glsl_version) {
    ImGuiIO& io = ImGui::GetIO();
    IMGUI_CHECKVERSION();
    IM_ASSERT(io.BackendRendererUserData == nullptr && "Already initialized a renderer backend!");

    // 初始化加载程序
    if (!ImGui_ImplOpenGL3_InitLoader()) return false;

    // 设置后端功能标志
    ImGui_ImplOpenGL3_Data* bd = IM_NEW(ImGui_ImplOpenGL3_Data)();
    io.BackendRendererUserData = (void*)bd;
    io.BackendRendererName     = "imgui_impl_opengl3";

    // 查询 GL 版本（例如 GL 3.2 为 320）
    const char* gl_version_str = (const char*)glGetString(GL_VERSION);
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // 备用2
    bd->GlVersion      = 200;
    bd->GlProfileIsES2 = true;
    IM_UNUSED(gl_version_str);
#else
    // 台式机或 GLES 3
    GLint major = 0;
    GLint minor = 0;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
    if (major == 0 && minor == 0)
        sscanf(
            gl_version_str, "%d.%d", &major, &minor
        ); // 在桌面 GL 2.x 中查询 GL_VERSION，字符串将以“<major>.<minor>”开头
    bd->GlVersion = (GLuint)(major * 100 + minor * 10);
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &bd->MaxTextureSize);

#if defined(IMGUI_IMPL_OPENGL_ES3)
    bd->GlProfileIsES3 = true;
#else
    if (strncmp(gl_version_str, "OpenGL ES 3", 11) == 0) bd->GlProfileIsES3 = true;
#endif

#if defined(GL_CONTEXT_PROFILE_MASK)
    if (!bd->GlProfileIsES3 && bd->GlVersion >= 320) glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &bd->GlProfileMask);
    bd->GlProfileIsCompat = (bd->GlProfileMask & GL_CONTEXT_COMPATIBILITY_PROFILE_BIT) != 0;
#endif

    bd->UseBufferSubData = false;
    /*
    // 查询供应商以启用 glBufferSubData kludge
#ifdef_WIN32
    if (const char*供应商 = (const char*)glGetString(GL_VENDOR))
        if (strncmp(供应商, "英特尔", 5) == 0)
            bd->UseBufferSubData = true;
#endif
    */
#endif

#ifdef IMGUI_IMPL_OPENGL_DEBUG
    printf(
        "GlVersion = %d, \"%s\"\nGlProfileIsCompat = %d\nGlProfileMask = 0x%X\nGlProfileIsES2/IsEs3 = %d/%d\nGL_VENDOR "
        "= '%s'\nGL_RENDERER = '%s'\n",
        bd->GlVersion,
        gl_version_str,
        bd->GlProfileIsCompat,
        bd->GlProfileMask,
        bd->GlProfileIsES2,
        bd->GlProfileIsES3,
        (const char*)glGetString(GL_VENDOR),
        (const char*)glGetString(GL_RENDERER)
    ); // [调试]
#endif

#ifdef IMGUI_IMPL_OPENGL_MAY_HAVE_VTX_OFFSET
    if (bd->GlVersion >= 320)
        io.BackendFlags |=
            ImGuiBackendFlags_RendererHasVtxOffset; // 我们可以遵循 ImDrawCmd::VtxOffset 字段，从而允许使用大型网格。
#endif
    io.BackendFlags |=
        ImGuiBackendFlags_RendererHasTextures; // 我们可以在渲染期间满足 ImGuiPlatformIO::Textures[] 请求。

    ImGuiPlatformIO& platform_io         = ImGui::GetPlatformIO();
    platform_io.Renderer_TextureMaxWidth = platform_io.Renderer_TextureMaxHeight = (int)bd->MaxTextureSize;

    // 存储 GLSL 版本字符串，以便我们稍后重新创建着色器时可以引用它。
    // 注意：GLSL 版本与 GL 版本不同。如果不确定，请将其保留为 nullptr。
    if (glsl_version == nullptr) {
#if defined(IMGUI_IMPL_OPENGL_ES2)
        glsl_version = "#version 100";
#elif defined(IMGUI_IMPL_OPENGL_ES3)
        glsl_version = "#version 300 es";
#elif defined(__APPLE__)
        glsl_version = "#version 150";
#else
        glsl_version = "#version 130";
#endif
    }
    IM_ASSERT((int)strlen(glsl_version) + 2 < IM_ARRAYSIZE(bd->GlslVersionString));
    strcpy(bd->GlslVersionString, glsl_version);
    strcat(bd->GlslVersionString, "\n");

    // 进行任意 GL 调用（我们实际上不需要结果）
    // 如果您在这里崩溃：这可能意味着 OpenGL 函数加载器没有完成其工作。让我们知道！
    GLint current_texture;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &current_texture);

    // 检测我们支持的扩展
#ifdef IMGUI_IMPL_OPENGL_MAY_HAVE_POLYGON_MODE
    bd->HasPolygonMode = (!bd->GlProfileIsES2 && !bd->GlProfileIsES3);
#endif
    bd->HasClipOrigin = (bd->GlVersion >= 450);
#ifdef IMGUI_IMPL_OPENGL_HAS_EXTENSIONS
    GLint num_extensions = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &num_extensions);
    for (GLint i = 0; i < num_extensions; i++) {
        const char* extension = (const char*)glGetStringi(GL_EXTENSIONS, i);
        if (extension != nullptr && strcmp(extension, "GL_ARB_clip_control") == 0) bd->HasClipOrigin = true;
    }
#endif

    return true;
}

void ImGui_ImplOpenGL3_Shutdown() {
    ImGui_ImplOpenGL3_Data* bd = ImGui_ImplOpenGL3_GetBackendData();
    IM_ASSERT(bd != nullptr && "No renderer backend to shutdown, or already shutdown?");
    ImGuiIO& io = ImGui::GetIO();

    ImGui_ImplOpenGL3_DestroyDeviceObjects();
    io.BackendRendererName     = nullptr;
    io.BackendRendererUserData = nullptr;
    io.BackendFlags &= ~(ImGuiBackendFlags_RendererHasVtxOffset | ImGuiBackendFlags_RendererHasTextures);
    IM_DELETE(bd);
}

void ImGui_ImplOpenGL3_NewFrame() {
    ImGui_ImplOpenGL3_Data* bd = ImGui_ImplOpenGL3_GetBackendData();
    IM_ASSERT(bd != nullptr && "Context or backend not initialized! Did you call ImGui_ImplOpenGL3_Init()?");

    ImGui_ImplOpenGL3_InitLoader(); // 如果尚未完成，则延迟初始化加载程序，例如DLL 边界。

    if (!bd->ShaderHandle)
        if (!ImGui_ImplOpenGL3_CreateDeviceObjects()) IM_ASSERT(0 && "ImGui_ImplOpenGL3_CreateDeviceObjects() failed!");
}

static void ImGui_ImplOpenGL3_SetupRenderState(
    ImDrawData* draw_data, int fb_width, int fb_height, GLuint vertex_array_object
) {
    ImGui_ImplOpenGL3_Data* bd = ImGui_ImplOpenGL3_GetBackendData();

    // 设置渲染状态：启用 alpha 混合、无面剔除、无深度测试、启用剪刀、多边形填充
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glEnable(GL_SCISSOR_TEST);
#ifdef IMGUI_IMPL_OPENGL_MAY_HAVE_PRIMITIVE_RESTART
    if (!bd->GlProfileIsES3 && bd->GlVersion >= 310) glDisable(GL_PRIMITIVE_RESTART);
#endif
#ifdef IMGUI_IMPL_OPENGL_MAY_HAVE_POLYGON_MODE
    if (bd->HasPolygonMode) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif

    // 支持GL 4.5很少使用的glClipControl(GL_UPPER_LEFT)
#if defined(GL_CLIP_ORIGIN)
    bool clip_origin_lower_left = true;
    if (bd->HasClipOrigin) {
        GLenum current_clip_origin = 0;
        glGetIntegerv(GL_CLIP_ORIGIN, (GLint*)&current_clip_origin);
        if (current_clip_origin == GL_UPPER_LEFT) clip_origin_lower_left = false;
    }
#endif

    // 设置视口、正交投影矩阵
    // 我们可见的imgui空间位于draw_data->DisplayPos（左上）到draw_data->DisplayPos+data_data->DisplaySize（右下）。对于单视口应用程序，DisplayPos 为 (0,0)。
    GL_CALL(glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height));
    float L = draw_data->DisplayPos.x;
    float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
    float T = draw_data->DisplayPos.y;
    float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
#if defined(GL_CLIP_ORIGIN)
    if (!clip_origin_lower_left) {
        float tmp = T;
        T         = B;
        B         = tmp;
    } // 如果原点在左上角，则交换顶部和底部
#endif
    const float ortho_projection[4][4] = {
        {2.0f / (R - L), 0.0f, 0.0f, 0.0f},
        {0.0f, 2.0f / (T - B), 0.0f, 0.0f},
        {0.0f, 0.0f, -1.0f, 0.0f},
        {(R + L) / (L - R), (T + B) / (B - T), 0.0f, 1.0f},
    };
    glUseProgram(bd->ShaderHandle);
    glUniform1i(bd->AttribLocationTex, 0);
    glUniformMatrix4fv(bd->AttribLocationProjMtx, 1, GL_FALSE, &ortho_projection[0][0]);

#ifdef IMGUI_IMPL_OPENGL_MAY_HAVE_BIND_SAMPLER
    if (bd->GlVersion >= 330 || bd->GlProfileIsES3)
        glBindSampler(0, 0); // 我们使用组合的纹理/采样器状态。使用 GL 3.3 和 GL ES 3.0 的应用程序可能会进行其他设置。
#endif

    (void)vertex_array_object;
#ifdef IMGUI_IMPL_OPENGL_USE_VERTEX_ARRAY
    glBindVertexArray(vertex_array_object);
#endif

    // 绑定顶点/索引缓冲区并为 ImDrawVert 设置属性
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, bd->VboHandle));
    GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bd->ElementsHandle));
    GL_CALL(glEnableVertexAttribArray(bd->AttribLocationVtxPos));
    GL_CALL(glEnableVertexAttribArray(bd->AttribLocationVtxUV));
    GL_CALL(glEnableVertexAttribArray(bd->AttribLocationVtxColor));
    GL_CALL(glVertexAttribPointer(
        bd->AttribLocationVtxPos, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)offsetof(ImDrawVert, pos)
    ));
    GL_CALL(glVertexAttribPointer(
        bd->AttribLocationVtxUV, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)offsetof(ImDrawVert, uv)
    ));
    GL_CALL(glVertexAttribPointer(
        bd->AttribLocationVtxColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), (GLvoid*)offsetof(ImDrawVert, col)
    ));
}

// OpenGL3 渲染功能。
// 请注意，此实现有点过于复杂，因为我们显式保存/设置/恢复每个 OpenGL 状态。
// 这是为了能够在不这样做的 OpenGL 引擎中运行。
void ImGui_ImplOpenGL3_RenderDrawData(ImDrawData* draw_data) {
    // 最小化时避免渲染，视网膜显示器的缩放坐标（屏幕坐标！=帧缓冲区坐标）
    int fb_width  = (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
    int fb_height = (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
    if (fb_width <= 0 || fb_height <= 0) return;

    ImGui_ImplOpenGL3_InitLoader(); // 如果尚未完成，则延迟初始化加载程序，例如DLL 边界。

    ImGui_ImplOpenGL3_Data* bd = ImGui_ImplOpenGL3_GetBackendData();

    // 赶上纹理更新。大多数时候，列表将有 1 个状态为“正常”的元素，即无事可做。
    // （这几乎总是指向 ImGui::GetPlatformIO().Textures[]，但它是 ImDrawData 的一部分，以允许覆盖或禁用纹理更新）。
    if (draw_data->Textures != nullptr)
        for (ImTextureData* tex : *draw_data->Textures)
            if (tex->Status != ImTextureStatus_OK) ImGui_ImplOpenGL3_UpdateTexture(tex);

    // 备份 GL 状态
    GLenum last_active_texture;
    glGetIntegerv(GL_ACTIVE_TEXTURE, (GLint*)&last_active_texture);
    glActiveTexture(GL_TEXTURE0);
    GLuint last_program;
    glGetIntegerv(GL_CURRENT_PROGRAM, (GLint*)&last_program);
    GLuint last_texture;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, (GLint*)&last_texture);
#ifdef IMGUI_IMPL_OPENGL_MAY_HAVE_BIND_SAMPLER
    GLuint last_sampler;
    if (bd->GlVersion >= 330 || bd->GlProfileIsES3) {
        glGetIntegerv(GL_SAMPLER_BINDING, (GLint*)&last_sampler);
    }
    else {
        last_sampler = 0;
    }
#endif
    GLuint last_array_buffer;
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, (GLint*)&last_array_buffer);
#ifndef IMGUI_IMPL_OPENGL_USE_VERTEX_ARRAY
    // 这是 OpenGL 3.0+ 和 OpenGL ES 3.0+ 上 VAO 的一部分。
    GLint last_element_array_buffer;
    glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &last_element_array_buffer);
    ImGui_ImplOpenGL3_VtxAttribState last_vtx_attrib_state_pos;
    last_vtx_attrib_state_pos.GetState(bd->AttribLocationVtxPos);
    ImGui_ImplOpenGL3_VtxAttribState last_vtx_attrib_state_uv;
    last_vtx_attrib_state_uv.GetState(bd->AttribLocationVtxUV);
    ImGui_ImplOpenGL3_VtxAttribState last_vtx_attrib_state_color;
    last_vtx_attrib_state_color.GetState(bd->AttribLocationVtxColor);
#endif
#ifdef IMGUI_IMPL_OPENGL_USE_VERTEX_ARRAY
    GLuint last_vertex_array_object;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, (GLint*)&last_vertex_array_object);
#endif
#ifdef IMGUI_IMPL_OPENGL_MAY_HAVE_POLYGON_MODE
    GLint last_polygon_mode[2];
    if (bd->HasPolygonMode) {
        glGetIntegerv(GL_POLYGON_MODE, last_polygon_mode);
    }
#endif
    GLint last_viewport[4];
    glGetIntegerv(GL_VIEWPORT, last_viewport);
    GLint last_scissor_box[4];
    glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box);
    GLenum last_blend_src_rgb;
    glGetIntegerv(GL_BLEND_SRC_RGB, (GLint*)&last_blend_src_rgb);
    GLenum last_blend_dst_rgb;
    glGetIntegerv(GL_BLEND_DST_RGB, (GLint*)&last_blend_dst_rgb);
    GLenum last_blend_src_alpha;
    glGetIntegerv(GL_BLEND_SRC_ALPHA, (GLint*)&last_blend_src_alpha);
    GLenum last_blend_dst_alpha;
    glGetIntegerv(GL_BLEND_DST_ALPHA, (GLint*)&last_blend_dst_alpha);
    GLenum last_blend_equation_rgb;
    glGetIntegerv(GL_BLEND_EQUATION_RGB, (GLint*)&last_blend_equation_rgb);
    GLenum last_blend_equation_alpha;
    glGetIntegerv(GL_BLEND_EQUATION_ALPHA, (GLint*)&last_blend_equation_alpha);
    GLboolean last_enable_blend        = glIsEnabled(GL_BLEND);
    GLboolean last_enable_cull_face    = glIsEnabled(GL_CULL_FACE);
    GLboolean last_enable_depth_test   = glIsEnabled(GL_DEPTH_TEST);
    GLboolean last_enable_stencil_test = glIsEnabled(GL_STENCIL_TEST);
    GLboolean last_enable_scissor_test = glIsEnabled(GL_SCISSOR_TEST);
#ifdef IMGUI_IMPL_OPENGL_MAY_HAVE_PRIMITIVE_RESTART
    GLboolean last_enable_primitive_restart =
        (!bd->GlProfileIsES3 && bd->GlVersion >= 310) ? glIsEnabled(GL_PRIMITIVE_RESTART) : GL_FALSE;
#endif

    // 设置所需的 GL 状态
    // 每次重新创建 VAO（这是为了轻松允许渲染多个 GL 上下文。VAO 不在 GL 上下文之间共享）
    // 渲染器实际上可以在没有任何 VAO 绑定的情况下工作，但随后我们的 VertexAttrib 调用将覆盖当前绑定的默认值。
    GLuint vertex_array_object = 0;
#ifdef IMGUI_IMPL_OPENGL_USE_VERTEX_ARRAY
    GL_CALL(glGenVertexArrays(1, &vertex_array_object));
#endif
    ImGui_ImplOpenGL3_SetupRenderState(draw_data, fb_width, fb_height, vertex_array_object);

    // 将剪刀/裁剪矩形投影到帧缓冲区空间中
    ImVec2 clip_off   = draw_data->DisplayPos;       // (0,0) 除非使用多视口
    ImVec2 clip_scale = draw_data->FramebufferScale; // (1,1) 除非使用视网膜显示器，通常是 (2,2)

    // 渲染命令列表
    for (int n = 0; n < draw_data->CmdListsCount; n++) {
        const ImDrawList* draw_list = draw_data->CmdLists[n];

        // 上传顶点/索引缓冲区
        // -如今 OpenGL 驱动程序处于非常糟糕的状态......
        //   2021 年期间，我们尝试根据报告从 glBufferData() 切换到孤立+glBufferSubData()
        //   在 Windows 上使用多视口时 Intel GPU 上的泄漏。
        // -此后我们不断听到各种显示损坏问题。我们开始禁用非英特尔 GPU，但英特尔上仍然报告了问题。
        // -我们现在回到专门使用 glBufferData()。因此，在此代码中 bd->UseBufferSubData 始终为 FALSE。
        //   我们将保留旧的代码路径一段时间，以防人们发现新问题可能想要测试 bd->UseBufferSubData 路径。
        // -请参阅 https://github.com/ocornut/imgui/issues/4468 并报告任何腐败问题。
        const GLsizeiptr vtx_buffer_size = (GLsizeiptr)draw_list->VtxBuffer.Size * (int)sizeof(ImDrawVert);
        const GLsizeiptr idx_buffer_size = (GLsizeiptr)draw_list->IdxBuffer.Size * (int)sizeof(ImDrawIdx);
        if (bd->UseBufferSubData) {
            if (bd->VertexBufferSize < vtx_buffer_size) {
                bd->VertexBufferSize = vtx_buffer_size;
                GL_CALL(glBufferData(GL_ARRAY_BUFFER, bd->VertexBufferSize, nullptr, GL_STREAM_DRAW));
            }
            if (bd->IndexBufferSize < idx_buffer_size) {
                bd->IndexBufferSize = idx_buffer_size;
                GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, bd->IndexBufferSize, nullptr, GL_STREAM_DRAW));
            }
            GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, 0, vtx_buffer_size, (const GLvoid*)draw_list->VtxBuffer.Data));
            GL_CALL(
                glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, idx_buffer_size, (const GLvoid*)draw_list->IdxBuffer.Data)
            );
        }
        else {
            GL_CALL(
                glBufferData(GL_ARRAY_BUFFER, vtx_buffer_size, (const GLvoid*)draw_list->VtxBuffer.Data, GL_STREAM_DRAW)
            );
            GL_CALL(glBufferData(
                GL_ELEMENT_ARRAY_BUFFER, idx_buffer_size, (const GLvoid*)draw_list->IdxBuffer.Data, GL_STREAM_DRAW
            ));
        }

        for (int cmd_i = 0; cmd_i < draw_list->CmdBuffer.Size; cmd_i++) {
            const ImDrawCmd* pcmd = &draw_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback != nullptr) {
                // 用户回调，通过 ImDrawList::AddCallback() 注册
                // （ImDrawCallback_ResetRenderState 是用户用来请求渲染器重置渲染状态的特殊回调值。）
                if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
                    ImGui_ImplOpenGL3_SetupRenderState(draw_data, fb_width, fb_height, vertex_array_object);
                else
                    pcmd->UserCallback(draw_list, pcmd);
            }
            else {
                // 将剪刀/剪切矩形投影到帧缓冲区空间中
                ImVec2 clip_min(
                    (pcmd->ClipRect.x - clip_off.x) * clip_scale.x, (pcmd->ClipRect.y - clip_off.y) * clip_scale.y
                );
                ImVec2 clip_max(
                    (pcmd->ClipRect.z - clip_off.x) * clip_scale.x, (pcmd->ClipRect.w - clip_off.y) * clip_scale.y
                );
                if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y) continue;

                // 应用剪刀/剪切矩形（Y 在 OpenGL 中反转）
                GL_CALL(glScissor(
                    (int)clip_min.x,
                    (int)((float)fb_height - clip_max.y),
                    (int)(clip_max.x - clip_min.x),
                    (int)(clip_max.y - clip_min.y)
                ));

                // 绑定纹理，绘制
                GL_CALL(glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->GetTexID()));
#ifdef IMGUI_IMPL_OPENGL_MAY_HAVE_VTX_OFFSET
                if (bd->GlVersion >= 320)
                    GL_CALL(glDrawElementsBaseVertex(
                        GL_TRIANGLES,
                        (GLsizei)pcmd->ElemCount,
                        sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT,
                        (void*)(intptr_t)(pcmd->IdxOffset * sizeof(ImDrawIdx)),
                        (GLint)pcmd->VtxOffset
                    ));
                else
#endif
                    GL_CALL(glDrawElements(
                        GL_TRIANGLES,
                        (GLsizei)pcmd->ElemCount,
                        sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT,
                        (void*)(intptr_t)(pcmd->IdxOffset * sizeof(ImDrawIdx))
                    ));
            }
        }
    }

    // 摧毁临时VAO
#ifdef IMGUI_IMPL_OPENGL_USE_VERTEX_ARRAY
    GL_CALL(glDeleteVertexArrays(1, &vertex_array_object));
#endif

    // 恢复修改后的 GL 状态
    // 这个“glIsProgram()”检查是必需的，因为如果程序在绑定备份时处于“待删除”状态，那么它现在已经被删除并会导致 OpenGL 错误。参见#6220。
    if (last_program == 0 || glIsProgram(last_program)) glUseProgram(last_program);
    glBindTexture(GL_TEXTURE_2D, last_texture);
#ifdef IMGUI_IMPL_OPENGL_MAY_HAVE_BIND_SAMPLER
    if (bd->GlVersion >= 330 || bd->GlProfileIsES3) glBindSampler(0, last_sampler);
#endif
    glActiveTexture(last_active_texture);
#ifdef IMGUI_IMPL_OPENGL_USE_VERTEX_ARRAY
    glBindVertexArray(last_vertex_array_object);
#endif
    glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
#ifndef IMGUI_IMPL_OPENGL_USE_VERTEX_ARRAY
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, last_element_array_buffer);
    last_vtx_attrib_state_pos.SetState(bd->AttribLocationVtxPos);
    last_vtx_attrib_state_uv.SetState(bd->AttribLocationVtxUV);
    last_vtx_attrib_state_color.SetState(bd->AttribLocationVtxColor);
#endif
    glBlendEquationSeparate(last_blend_equation_rgb, last_blend_equation_alpha);
    glBlendFuncSeparate(last_blend_src_rgb, last_blend_dst_rgb, last_blend_src_alpha, last_blend_dst_alpha);
    if (last_enable_blend)
        glEnable(GL_BLEND);
    else
        glDisable(GL_BLEND);
    if (last_enable_cull_face)
        glEnable(GL_CULL_FACE);
    else
        glDisable(GL_CULL_FACE);
    if (last_enable_depth_test)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);
    if (last_enable_stencil_test)
        glEnable(GL_STENCIL_TEST);
    else
        glDisable(GL_STENCIL_TEST);
    if (last_enable_scissor_test)
        glEnable(GL_SCISSOR_TEST);
    else
        glDisable(GL_SCISSOR_TEST);
#ifdef IMGUI_IMPL_OPENGL_MAY_HAVE_PRIMITIVE_RESTART
    if (!bd->GlProfileIsES3 && bd->GlVersion >= 310) {
        if (last_enable_primitive_restart)
            glEnable(GL_PRIMITIVE_RESTART);
        else
            glDisable(GL_PRIMITIVE_RESTART);
    }
#endif

#ifdef IMGUI_IMPL_OPENGL_MAY_HAVE_POLYGON_MODE
    // 桌面 OpenGL 3.0 和 OpenGL 3.1 对于多边形的正面和背面具有单独的多边形绘制模式
    if (bd->HasPolygonMode) {
        if (bd->GlVersion <= 310 || bd->GlProfileIsCompat) {
            glPolygonMode(GL_FRONT, (GLenum)last_polygon_mode[0]);
            glPolygonMode(GL_BACK, (GLenum)last_polygon_mode[1]);
        }
        else {
            glPolygonMode(GL_FRONT_AND_BACK, (GLenum)last_polygon_mode[0]);
        }
    }
#endif // Imgui impl opengl 可能有多边形模式

    glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);
    glScissor(last_scissor_box[0], last_scissor_box[1], (GLsizei)last_scissor_box[2], (GLsizei)last_scissor_box[3]);
    (void)bd; // 并非所有编译路径都使用这个
}

static void ImGui_ImplOpenGL3_DestroyTexture(ImTextureData* tex) {
    GLuint gl_tex_id = (GLuint)(intptr_t)tex->TexID;
    glDeleteTextures(1, &gl_tex_id);

    // 清除标识符并标记为已销毁（以便允许在运行时调用 InvalidateDeviceObjects）
    tex->SetTexID(ImTextureID_Invalid);
    tex->SetStatus(ImTextureStatus_Destroyed);
}

void ImGui_ImplOpenGL3_UpdateTexture(ImTextureData* tex) {
    if (tex->Status == ImTextureStatus_WantCreate) {
        // 创建新纹理并将其上传到图形系统
        //IMGUI_DEBUG_LOG("UpdateTexture #%03d: WantCreate %dx%d\n", tex->UniqueID, tex->宽度, tex->高度);
        IM_ASSERT(tex->TexID == 0 && tex->BackendUserData == nullptr);
        IM_ASSERT(tex->Format == ImTextureFormat_RGBA32);
        const void* pixels   = tex->GetPixels();
        GLuint gl_texture_id = 0;

        // 将纹理上传到图形系统
        // （默认需要双线性采样。设置 'io.Fonts->Flags |= ImFontAtlasFlags_NoBakedLines' 或 'style.AntiAliasedLinesUseTex = false' 以允许点/最近采样）
        GLint last_texture;
        GL_CALL(glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture));
        GL_CALL(glGenTextures(1, &gl_texture_id));
        GL_CALL(glBindTexture(GL_TEXTURE_2D, gl_texture_id));
        GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
        GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
#ifdef GL_UNPACK_ROW_LENGTH // 不适用于 WebGL/ES
        GL_CALL(glPixelStorei(GL_UNPACK_ROW_LENGTH, 0));
#endif
        GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex->Width, tex->Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels));

        // 商店标识符
        tex->SetTexID((ImTextureID)(intptr_t)gl_texture_id);
        tex->SetStatus(ImTextureStatus_OK);

        // 恢复状态
        GL_CALL(glBindTexture(GL_TEXTURE_2D, last_texture));
    }
    else if (tex->Status == ImTextureStatus_WantUpdates) {
        // 更新选定的块。我们只写入以前从未使用过的纹理区域！
        // 该后端选择使用 tex->Updates[]，但您可以使用 tex->UpdateRect 上传单个区域。
        GLint last_texture;
        GL_CALL(glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture));

        GLuint gl_tex_id = (GLuint)(intptr_t)tex->TexID;
        GL_CALL(glBindTexture(GL_TEXTURE_2D, gl_tex_id));
#if 0 // GL_UNPACK_ROW_LENGTH //不适用于 WebGL/ES
        GL_CALL(glPixelStorei(GL_UNPACK_ROW_LENGTH, tex->Width));
        for (ImTextureRect& r : tex->Updates)
            GL_CALL(glTexSubImage2D(GL_TEXTURE_2D, 0, r.x, r.y, r.w, r.h, GL_RGBA, GL_UNSIGNED_BYTE, tex->GetPixelsAt(r.x, r.y)));
        GL_CALL(glPixelStorei(GL_UNPACK_ROW_LENGTH, 0));
#else
        // GL ES 没有 GL_UNPACK_ROW_LENGTH，因此我们需要 (A) 复制到连续的缓冲区或 (B) 逐行上传。
        ImGui_ImplOpenGL3_Data* bd = ImGui_ImplOpenGL3_GetBackendData();
        for (ImTextureRect& r : tex->Updates) {
            const int src_pitch = r.w * tex->BytesPerPixel;
            bd->TempBuffer.resize(r.h * src_pitch);
            char* out_p = bd->TempBuffer.Data;
            for (int y = 0; y < r.h; y++, out_p += src_pitch)
                memcpy(out_p, tex->GetPixelsAt(r.x, r.y + y), src_pitch);
            IM_ASSERT(out_p == bd->TempBuffer.end());
            GL_CALL(
                glTexSubImage2D(GL_TEXTURE_2D, 0, r.x, r.y, r.w, r.h, GL_RGBA, GL_UNSIGNED_BYTE, bd->TempBuffer.Data)
            );
        }
#endif
        tex->SetStatus(ImTextureStatus_OK);
        GL_CALL(glBindTexture(GL_TEXTURE_2D, last_texture)); // 恢复状态
    }
    else if (tex->Status == ImTextureStatus_WantDestroy && tex->UnusedFrames > 0)
        ImGui_ImplOpenGL3_DestroyTexture(tex);
}

// 如果您遇到错误，请在 github 上报告。您可以尝试不同的 GL 上下文版本或 GLSL 版本。请参阅此文件顶部的 GL<>GLSL 版本表。
static bool CheckShader(GLuint handle, const char* desc) {
    ImGui_ImplOpenGL3_Data* bd = ImGui_ImplOpenGL3_GetBackendData();
    GLint status = 0, log_length = 0;
    glGetShaderiv(handle, GL_COMPILE_STATUS, &status);
    glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &log_length);
    if ((GLboolean)status == GL_FALSE)
        fprintf(
            stderr,
            "ERROR: ImGui_ImplOpenGL3_CreateDeviceObjects: failed to compile %s! With GLSL: %s\n",
            desc,
            bd->GlslVersionString
        );
    if (log_length > 1) {
        ImVector<char> buf;
        buf.resize((int)(log_length + 1));
        glGetShaderInfoLog(handle, log_length, nullptr, (GLchar*)buf.begin());
        fprintf(stderr, "%s\n", buf.begin());
    }
    return (GLboolean)status == GL_TRUE;
}

// 如果出现错误，请在 GitHub 上报告。您可以尝试不同的 GL 上下文版本或 GLSL 版本。
static bool CheckProgram(GLuint handle, const char* desc) {
    ImGui_ImplOpenGL3_Data* bd = ImGui_ImplOpenGL3_GetBackendData();
    GLint status = 0, log_length = 0;
    glGetProgramiv(handle, GL_LINK_STATUS, &status);
    glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &log_length);
    if ((GLboolean)status == GL_FALSE)
        fprintf(
            stderr,
            "ERROR: ImGui_ImplOpenGL3_CreateDeviceObjects: failed to link %s! With GLSL %s\n",
            desc,
            bd->GlslVersionString
        );
    if (log_length > 1) {
        ImVector<char> buf;
        buf.resize((int)(log_length + 1));
        glGetProgramInfoLog(handle, log_length, nullptr, (GLchar*)buf.begin());
        fprintf(stderr, "%s\n", buf.begin());
    }
    return (GLboolean)status == GL_TRUE;
}

bool ImGui_ImplOpenGL3_CreateDeviceObjects() {
    ImGui_ImplOpenGL3_Data* bd = ImGui_ImplOpenGL3_GetBackendData();

    // 备份 GL 状态
    GLint last_texture, last_array_buffer;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
#ifdef IMGUI_IMPL_OPENGL_MAY_HAVE_BIND_BUFFER_PIXEL_UNPACK
    GLint last_pixel_unpack_buffer = 0;
    if (bd->GlVersion >= 210) {
        glGetIntegerv(GL_PIXEL_UNPACK_BUFFER_BINDING, &last_pixel_unpack_buffer);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    }
#endif
#ifdef IMGUI_IMPL_OPENGL_USE_VERTEX_ARRAY
    GLint last_vertex_array;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);
#endif

    // 解析 GLSL 版本字符串
    int glsl_version = 130;
    sscanf(bd->GlslVersionString, "#version %d", &glsl_version);

    const GLchar* vertex_shader_glsl_120 = "uniform mat4 ProjMtx;\n"
                                           "attribute vec2 Position;\n"
                                           "attribute vec2 UV;\n"
                                           "attribute vec4 Color;\n"
                                           "varying vec2 Frag_UV;\n"
                                           "varying vec4 Frag_Color;\n"
                                           "void main()\n"
                                           "{\n"
                                           "    Frag_UV = UV;\n"
                                           "    Frag_Color = Color;\n"
                                           "    gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
                                           "}\n";

    const GLchar* vertex_shader_glsl_130 = "uniform mat4 ProjMtx;\n"
                                           "in vec2 Position;\n"
                                           "in vec2 UV;\n"
                                           "in vec4 Color;\n"
                                           "out vec2 Frag_UV;\n"
                                           "out vec4 Frag_Color;\n"
                                           "void main()\n"
                                           "{\n"
                                           "    Frag_UV = UV;\n"
                                           "    Frag_Color = Color;\n"
                                           "    gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
                                           "}\n";

    const GLchar* vertex_shader_glsl_300_es = "precision highp float;\n"
                                              "layout (location = 0) in vec2 Position;\n"
                                              "layout (location = 1) in vec2 UV;\n"
                                              "layout (location = 2) in vec4 Color;\n"
                                              "uniform mat4 ProjMtx;\n"
                                              "out vec2 Frag_UV;\n"
                                              "out vec4 Frag_Color;\n"
                                              "void main()\n"
                                              "{\n"
                                              "    Frag_UV = UV;\n"
                                              "    Frag_Color = Color;\n"
                                              "    gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
                                              "}\n";

    const GLchar* vertex_shader_glsl_410_core = "layout (location = 0) in vec2 Position;\n"
                                                "layout (location = 1) in vec2 UV;\n"
                                                "layout (location = 2) in vec4 Color;\n"
                                                "uniform mat4 ProjMtx;\n"
                                                "out vec2 Frag_UV;\n"
                                                "out vec4 Frag_Color;\n"
                                                "void main()\n"
                                                "{\n"
                                                "    Frag_UV = UV;\n"
                                                "    Frag_Color = Color;\n"
                                                "    gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
                                                "}\n";

    const GLchar* fragment_shader_glsl_120 = "#ifdef GL_ES\n"
                                             "    precision mediump float;\n"
                                             "#endif\n"
                                             "uniform sampler2D Texture;\n"
                                             "varying vec2 Frag_UV;\n"
                                             "varying vec4 Frag_Color;\n"
                                             "void main()\n"
                                             "{\n"
                                             "    gl_FragColor = Frag_Color * texture2D(Texture, Frag_UV.st);\n"
                                             "}\n";

    const GLchar* fragment_shader_glsl_130 = "uniform sampler2D Texture;\n"
                                             "in vec2 Frag_UV;\n"
                                             "in vec4 Frag_Color;\n"
                                             "out vec4 Out_Color;\n"
                                             "void main()\n"
                                             "{\n"
                                             "    Out_Color = Frag_Color * texture(Texture, Frag_UV.st);\n"
                                             "}\n";

    const GLchar* fragment_shader_glsl_300_es = "precision mediump float;\n"
                                                "uniform sampler2D Texture;\n"
                                                "in vec2 Frag_UV;\n"
                                                "in vec4 Frag_Color;\n"
                                                "layout (location = 0) out vec4 Out_Color;\n"
                                                "void main()\n"
                                                "{\n"
                                                "    Out_Color = Frag_Color * texture(Texture, Frag_UV.st);\n"
                                                "}\n";

    const GLchar* fragment_shader_glsl_410_core = "in vec2 Frag_UV;\n"
                                                  "in vec4 Frag_Color;\n"
                                                  "uniform sampler2D Texture;\n"
                                                  "layout (location = 0) out vec4 Out_Color;\n"
                                                  "void main()\n"
                                                  "{\n"
                                                  "    Out_Color = Frag_Color * texture(Texture, Frag_UV.st);\n"
                                                  "}\n";

    // 选择与我们的 GLSL 版本匹配的着色器
    const GLchar* vertex_shader   = nullptr;
    const GLchar* fragment_shader = nullptr;
    if (glsl_version < 130) {
        vertex_shader   = vertex_shader_glsl_120;
        fragment_shader = fragment_shader_glsl_120;
    }
    else if (glsl_version >= 410) {
        vertex_shader   = vertex_shader_glsl_410_core;
        fragment_shader = fragment_shader_glsl_410_core;
    }
    else if (glsl_version == 300) {
        vertex_shader   = vertex_shader_glsl_300_es;
        fragment_shader = fragment_shader_glsl_300_es;
    }
    else {
        vertex_shader   = vertex_shader_glsl_130;
        fragment_shader = fragment_shader_glsl_130;
    }

    // 创建着色器
    const GLchar* vertex_shader_with_version[2] = {bd->GlslVersionString, vertex_shader};
    GLuint vert_handle;
    GL_CALL(vert_handle = glCreateShader(GL_VERTEX_SHADER));
    glShaderSource(vert_handle, 2, vertex_shader_with_version, nullptr);
    glCompileShader(vert_handle);
    if (!CheckShader(vert_handle, "vertex shader")) return false;

    const GLchar* fragment_shader_with_version[2] = {bd->GlslVersionString, fragment_shader};
    GLuint frag_handle;
    GL_CALL(frag_handle = glCreateShader(GL_FRAGMENT_SHADER));
    glShaderSource(frag_handle, 2, fragment_shader_with_version, nullptr);
    glCompileShader(frag_handle);
    if (!CheckShader(frag_handle, "fragment shader")) return false;

    // 关联
    bd->ShaderHandle = glCreateProgram();
    glAttachShader(bd->ShaderHandle, vert_handle);
    glAttachShader(bd->ShaderHandle, frag_handle);
    glLinkProgram(bd->ShaderHandle);
    if (!CheckProgram(bd->ShaderHandle, "shader program")) return false;

    glDetachShader(bd->ShaderHandle, vert_handle);
    glDetachShader(bd->ShaderHandle, frag_handle);
    glDeleteShader(vert_handle);
    glDeleteShader(frag_handle);

    bd->AttribLocationTex      = glGetUniformLocation(bd->ShaderHandle, "Texture");
    bd->AttribLocationProjMtx  = glGetUniformLocation(bd->ShaderHandle, "ProjMtx");
    bd->AttribLocationVtxPos   = (GLuint)glGetAttribLocation(bd->ShaderHandle, "Position");
    bd->AttribLocationVtxUV    = (GLuint)glGetAttribLocation(bd->ShaderHandle, "UV");
    bd->AttribLocationVtxColor = (GLuint)glGetAttribLocation(bd->ShaderHandle, "Color");

    // 创建缓冲区
    glGenBuffers(1, &bd->VboHandle);
    glGenBuffers(1, &bd->ElementsHandle);

    // 恢复修改后的 GL 状态
    glBindTexture(GL_TEXTURE_2D, last_texture);
    glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
#ifdef IMGUI_IMPL_OPENGL_MAY_HAVE_BIND_BUFFER_PIXEL_UNPACK
    if (bd->GlVersion >= 210) {
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, last_pixel_unpack_buffer);
    }
#endif
#ifdef IMGUI_IMPL_OPENGL_USE_VERTEX_ARRAY
    glBindVertexArray(last_vertex_array);
#endif

    return true;
}

void ImGui_ImplOpenGL3_DestroyDeviceObjects() {
    ImGui_ImplOpenGL3_Data* bd = ImGui_ImplOpenGL3_GetBackendData();
    if (bd->VboHandle) {
        glDeleteBuffers(1, &bd->VboHandle);
        bd->VboHandle = 0;
    }
    if (bd->ElementsHandle) {
        glDeleteBuffers(1, &bd->ElementsHandle);
        bd->ElementsHandle = 0;
    }
    if (bd->ShaderHandle) {
        glDeleteProgram(bd->ShaderHandle);
        bd->ShaderHandle = 0;
    }

    // 销毁所有纹理
    for (ImTextureData* tex : ImGui::GetPlatformIO().Textures)
        if (tex->RefCount == 1) ImGui_ImplOpenGL3_DestroyTexture(tex);
}

//-----------------------------------------------------------------------------

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

#endif // #ifndef IMGUI_DISABLE
