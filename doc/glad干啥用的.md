# glad 干啥用的?
---
首先我们要知道 `GL/gl.h` 是个啥，它最初诞生于 1992 年，OpenGL 1.0 发布时。当时就提供了 OpenGL 的官方 API，也就是 `gl.h`。

- OpenGL 最初由 SGI（Silicon Graphics Inc.） 开发，跨平台，高性能，主要用于专业图形工作站。
- Windows 自 95/98 开始提供了 opengl32.dll + gl.h，以便开发者在 Windows 上调用 OpenGL 1.1。
- 当时 OpenGL 1.1（1997 年）就是 Windows 系统默认版本，提供基础的渲染功能。 
---
## 微软的策略变化

微软在 1997 年后不再更新 `gl.h`，原因主要有三个：

(1) 推广自己的图形 API —— **DirectX**

1995 年左右，微软推出 DirectX，尤其是 Direct3D。

目标：让 Windows 平台有统一、现代化的 3D 图形 API。

Direct3D 更贴近 Windows 平台、支持游戏和硬件加速，微软倾向把资源放在 Direct3D，而不是继续更新 OpenGL。

换句话说，OpenGL 在 Windows 上成了“兼容旧软件的 API”，微软没必要更新它。

---
## OpenGL 的现代函数由显卡厂商提供（ICD）

OpenGL 自 1.2+ 开始使用 扩展机制（Extensions）：

新的函数由 GPU 驱动厂商提供

系统只提供 1.1 核心函数和 wglGetProcAddress 来加载扩展

微软设计 opengl32.dll 为 OpenGL 1.1 的“桥梁” + loader，无需再更新 gl.h

开发者要用新功能，就必须通过 wglGetProcAddress 或 loader（GLAD/GLEW）获取函数指针

所以系统 gl.h 不更新是因为：新函数不再由 Windows 提供，而是显卡厂商提供

---

因此 `gl.h` 只包含了最基本的类型，最原始的 OpenGL 函数，不包含任何现代的 OpenGL 函数。
如果我们要使用现代的 OpenGL 函数，我们就必须使用形如：
```cpp
PFNGLGENBUFFERSPROC glGenBuffers =
    (PFNGLGENBUFFERSPROC) wglGetProcAddress("glGenBuffers");
```
其中的 `PFNGLGENBUFFERSPROC` 定义如下：
```cpp
typedef void (APIENTRYP PFNGLGENBUFFERSPROC) (GLsizei n, GLuint *buffers);
```

必须由我们自己定义函数签名，然后自己使用 `wglGetProcAddress` 函数来获取函数地址，而这个函数正是 windows 上的 opengl 拓展函数，事实上所有的 windows 上的 opengl 拓展函数都以 `wgl` 开头，即 `windows opengl` 的缩写。

这个函数只能获取显卡驱动中的函数地址，普通 dll 的不行，普通 dll 必须使用 `GetProcAddress` 方法。

---

但是我们自己去获取地址无疑是极其痛苦的一件事，因此有一些库会帮我们干这个事，比较老牌的是 `GLEW` ，这个我们以前也用过。但是这个库也比较老了更新也不及时。

而 `GLAD` 是更现代的选择，它完全替代掉 `GL/gl.h` 的作用，不仅全部定义了一遍基本数据类型和函数，还会自动加载所有的现代 opengl 函数。

在 glad 中会定义 `__gl_h_` 宏，这个宏在 `GL/gl.h` 也有定义，因此相当于 glad 完全替代掉了 `GL/gl.h`, 之后其他用到 `GL/gl.h` 头文件的库将不会再将其包含，而是使用我们在 `glad.h` 中的定义。

这也是为什么 `glad.h` 一定要在其他基于 opengl 头文件的库之前包含，因为它的作用就是完全替代 `GL/gl.h`。