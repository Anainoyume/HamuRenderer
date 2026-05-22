# GLFW 3.4 API 完整分类整理

## 1. 初始化与终止 (Initialization and Termination)

### 初始化相关
- `glfwInit()` - 初始化GLFW库
- `glfwTerminate()` - 终止GLFW库
- `glfwInitHint(int hint, int value)` - 设置初始化提示
- `glfwInitAllocator(const GLFWallocator* allocator)` - 设置内存分配器
- `glfwInitVulkanLoader(PFN_vkGetInstanceProcAddr loader)` - 设置Vulkan加载器

### 版本与平台信息
- `glfwGetVersion(int* major, int* minor, int* rev)` - 获取GLFW版本
- `glfwGetVersionString()` - 获取版本字符串
- `glfwGetPlatform()` - 获取当前平台
- `glfwPlatformSupported(int platform)` - 检查平台支持

### 错误处理
- `glfwGetError(const char** description)` - 获取并清除错误
- `glfwSetErrorCallback(GLFWerrorfun callback)` - 设置错误回调

---

## 2. 监视器相关 (Monitor)

### 监视器查询
- `glfwGetMonitors(int* count)` - 获取所有监视器
- `glfwGetPrimaryMonitor()` - 获取主监视器
- `glfwGetMonitorPos(GLFWmonitor* monitor, int* xpos, int* ypos)` - 获取监视器位置
- `glfwGetMonitorWorkarea(GLFWmonitor* monitor, int* xpos, int* ypos, int* width, int* height)` - 获取工作区域
- `glfwGetMonitorPhysicalSize(GLFWmonitor* monitor, int* widthMM, int* heightMM)` - 获取物理尺寸
- `glfwGetMonitorContentScale(GLFWmonitor* monitor, float* xscale, float* yscale)` - 获取内容缩放
- `glfwGetMonitorName(GLFWmonitor* monitor)` - 获取监视器名称

### 监视器用户数据
- `glfwSetMonitorUserPointer(GLFWmonitor* monitor, void* pointer)` - 设置用户指针
- `glfwGetMonitorUserPointer(GLFWmonitor* monitor)` - 获取用户指针

### 监视器回调
- `glfwSetMonitorCallback(GLFWmonitorfun callback)` - 设置监视器配置回调

### 视频模式
- `glfwGetVideoModes(GLFWmonitor* monitor, int* count)` - 获取所有视频模式
- `glfwGetVideoMode(GLFWmonitor* monitor)` - 获取当前视频模式

### 伽马控制
- `glfwSetGamma(GLFWmonitor* monitor, float gamma)` - 设置伽马值
- `glfwGetGammaRamp(GLFWmonitor* monitor)` - 获取伽马斜坡
- `glfwSetGammaRamp(GLFWmonitor* monitor, const GLFWgammaramp* ramp)` - 设置伽马斜坡

---

## 3. 窗口管理 (Window)

### 窗口创建与销毁
- `glfwCreateWindow(int width, int height, const char* title, GLFWmonitor* monitor, GLFWwindow* share)` - 创建窗口
- `glfwDestroyWindow(GLFWwindow* window)` - 销毁窗口

### 窗口提示
- `glfwDefaultWindowHints()` - 重置窗口提示
- `glfwWindowHint(int hint, int value)` - 设置整数窗口提示
- `glfwWindowHintString(int hint, const char* value)` - 设置字符串窗口提示

### 窗口状态查询与设置
- `glfwWindowShouldClose(GLFWwindow* window)` - 检查窗口关闭标志
- `glfwSetWindowShouldClose(GLFWwindow* window, int value)` - 设置窗口关闭标志
- `glfwGetWindowTitle(GLFWwindow* window)` - 获取窗口标题
- `glfwSetWindowTitle(GLFWwindow* window, const char* title)` - 设置窗口标题
- `glfwSetWindowIcon(GLFWwindow* window, int count, const GLFWimage* images)` - 设置窗口图标

### 窗口位置与尺寸
- `glfwGetWindowPos(GLFWwindow* window, int* xpos, int* ypos)` - 获取窗口位置
- `glfwSetWindowPos(GLFWwindow* window, int xpos, int ypos)` - 设置窗口位置
- `glfwGetWindowSize(GLFWwindow* window, int* width, int* height)` - 获取窗口尺寸
- `glfwSetWindowSize(GLFWwindow* window, int width, int height)` - 设置窗口尺寸
- `glfwSetWindowSizeLimits(GLFWwindow* window, int minwidth, int minheight, int maxwidth, int maxheight)` - 设置尺寸限制
- `glfwSetWindowAspectRatio(GLFWwindow* window, int numer, int denom)` - 设置宽高比

### 帧缓冲
- `glfwGetFramebufferSize(GLFWwindow* window, int* width, int* height)` - 获取帧缓冲尺寸
- `glfwGetWindowFrameSize(GLFWwindow* window, int* left, int* top, int* right, int* bottom)` - 获取窗口边框尺寸

### 内容缩放
- `glfwGetWindowContentScale(GLFWwindow* window, float* xscale, float* yscale)` - 获取内容缩放

### 窗口透明度
- `glfwGetWindowOpacity(GLFWwindow* window)` - 获取窗口不透明度
- `glfwSetWindowOpacity(GLFWwindow* window, float opacity)` - 设置窗口不透明度

### 窗口状态控制
- `glfwIconifyWindow(GLFWwindow* window)` - 最小化窗口
- `glfwRestoreWindow(GLFWwindow* window)` - 恢复窗口
- `glfwMaximizeWindow(GLFWwindow* window)` - 最大化窗口
- `glfwShowWindow(GLFWwindow* window)` - 显示窗口
- `glfwHideWindow(GLFWwindow* window)` - 隐藏窗口
- `glfwFocusWindow(GLFWwindow* window)` - 聚焦窗口
- `glfwRequestWindowAttention(GLFWwindow* window)` - 请求用户注意

### 监视器与全屏
- `glfwGetWindowMonitor(GLFWwindow* window)` - 获取窗口所在监视器
- `glfwSetWindowMonitor(GLFWwindow* window, GLFWmonitor* monitor, int xpos, int ypos, int width, int height, int refreshRate)` - 设置窗口监视器

### 窗口属性
- `glfwGetWindowAttrib(GLFWwindow* window, int attrib)` - 获取窗口属性
- `glfwSetWindowAttrib(GLFWwindow* window, int attrib, int value)` - 设置窗口属性

### 窗口用户数据
- `glfwSetWindowUserPointer(GLFWwindow* window, void* pointer)` - 设置用户指针
- `glfwGetWindowUserPointer(GLFWwindow* window)` - 获取用户指针

### 窗口回调设置
- `glfwSetWindowPosCallback(GLFWwindow* window, GLFWwindowposfun callback)` - 设置位置回调
- `glfwSetWindowSizeCallback(GLFWwindow* window, GLFWwindowsizefun callback)` - 设置尺寸回调
- `glfwSetWindowCloseCallback(GLFWwindow* window, GLFWwindowclosefun callback)` - 设置关闭回调
- `glfwSetWindowRefreshCallback(GLFWwindow* window, GLFWwindowrefreshfun callback)` - 设置刷新回调
- `glfwSetWindowFocusCallback(GLFWwindow* window, GLFWwindowfocusfun callback)` - 设置焦点回调
- `glfwSetWindowIconifyCallback(GLFWwindow* window, GLFWwindowiconifyfun callback)` - 设置最小化回调
- `glfwSetWindowMaximizeCallback(GLFWwindow* window, GLFWwindowmaximizefun callback)` - 设置最大化回调
- `glfwSetFramebufferSizeCallback(GLFWwindow* window, GLFWframebuffersizefun callback)` - 设置帧缓冲尺寸回调
- `glfwSetWindowContentScaleCallback(GLFWwindow* window, GLFWwindowcontentscalefun callback)` - 设置内容缩放回调

---

## 4. 事件处理 (Event Processing)

- `glfwPollEvents()` - 处理所有待处理事件
- `glfwWaitEvents()` - 等待事件
- `glfwWaitEventsTimeout(double timeout)` - 带超时等待事件
- `glfwPostEmptyEvent()` - 发送空事件

---

## 5. 输入处理 (Input)

### 输入模式
- `glfwGetInputMode(GLFWwindow* window, int mode)` - 获取输入模式
- `glfwSetInputMode(GLFWwindow* window, int mode, int value)` - 设置输入模式
- `glfwRawMouseMotionSupported()` - 检查原始鼠标移动支持

### 键盘输入
- `glfwGetKeyName(int key, int scancode)` - 获取按键名称
- `glfwGetKeyScancode(int key)` - 获取按键扫描码
- `glfwGetKey(GLFWwindow* window, int key)` - 获取按键状态
- `glfwSetKeyCallback(GLFWwindow* window, GLFWkeyfun callback)` - 设置按键回调
- `glfwSetCharCallback(GLFWwindow* window, GLFWcharfun callback)` - 设置字符回调
- `glfwSetCharModsCallback(GLFWwindow* window, GLFWcharmodsfun callback)` - 设置带修饰符字符回调（已废弃）

### 鼠标输入
- `glfwGetMouseButton(GLFWwindow* window, int button)` - 获取鼠标按钮状态
- `glfwGetCursorPos(GLFWwindow* window, double* xpos, double* ypos)` - 获取光标位置
- `glfwSetCursorPos(GLFWwindow* window, double xpos, double ypos)` - 设置光标位置
- `glfwSetMouseButtonCallback(GLFWwindow* window, GLFWmousebuttonfun callback)` - 设置鼠标按钮回调
- `glfwSetCursorPosCallback(GLFWwindow* window, GLFWcursorposfun callback)` - 设置光标位置回调
- `glfwSetCursorEnterCallback(GLFWwindow* window, GLFWcursorenterfun callback)` - 设置光标进入回调
- `glfwSetScrollCallback(GLFWwindow* window, GLFWscrollfun callback)` - 设置滚动回调

### 光标管理
- `glfwCreateCursor(const GLFWimage* image, int xhot, int yhot)` - 创建自定义光标
- `glfwCreateStandardCursor(int shape)` - 创建标准光标
- `glfwDestroyCursor(GLFWcursor* cursor)` - 销毁光标
- `glfwSetCursor(GLFWwindow* window, GLFWcursor* cursor)` - 设置窗口光标

### 路径拖放
- `glfwSetDropCallback(GLFWwindow* window, GLFWdropfun callback)` - 设置路径拖放回调

### 剪贴板
- `glfwSetClipboardString(GLFWwindow* window, const char* string)` - 设置剪贴板内容
- `glfwGetClipboardString(GLFWwindow* window)` - 获取剪贴板内容

---

## 6. 游戏手柄/摇杆 (Joystick/Gamepad)

### 摇杆查询
- `glfwJoystickPresent(int jid)` - 检查摇杆是否存在
- `glfwGetJoystickAxes(int jid, int* count)` - 获取摇杆轴值
- `glfwGetJoystickButtons(int jid, int* count)` - 获取摇杆按钮状态
- `glfwGetJoystickHats(int jid, int* count)` - 获取摇杆帽状态
- `glfwGetJoystickName(int jid)` - 获取摇杆名称
- `glfwGetJoystickGUID(int jid)` - 获取摇杆GUID

### 摇杆用户数据
- `glfwSetJoystickUserPointer(int jid, void* pointer)` - 设置用户指针
- `glfwGetJoystickUserPointer(int jid)` - 获取用户指针

### 摇杆回调
- `glfwSetJoystickCallback(GLFWjoystickfun callback)` - 设置摇杆回调

### 游戏手柄
- `glfwJoystickIsGamepad(int jid)` - 检查是否有游戏手柄映射
- `glfwUpdateGamepadMappings(const char* string)` - 更新游戏手柄映射
- `glfwGetGamepadName(int jid)` - 获取游戏手柄名称
- `glfwGetGamepadState(int jid, GLFWgamepadstate* state)` - 获取游戏手柄状态

---

## 7. 时间 (Time)

- `glfwGetTime()` - 获取GLFW时间
- `glfwSetTime(double time)` - 设置GLFW时间
- `glfwGetTimerValue()` - 获取原始计时器值
- `glfwGetTimerFrequency()` - 获取计时器频率

---

## 8. OpenGL/OpenGL ES 上下文 (Context)

### 上下文管理
- `glfwMakeContextCurrent(GLFWwindow* window)` - 使上下文成为当前
- `glfwGetCurrentContext()` - 获取当前上下文窗口
- `glfwSwapBuffers(GLFWwindow* window)` - 交换前后缓冲
- `glfwSwapInterval(int interval)` - 设置交换间隔

### 扩展与函数加载
- `glfwExtensionSupported(const char* extension)` - 检查扩展支持
- `glfwGetProcAddress(const char* procname)` - 获取函数地址

---

## 9. Vulkan 支持 (Vulkan)

- `glfwVulkanSupported()` - 检查Vulkan支持
- `glfwGetRequiredInstanceExtensions(uint32_t* count)` - 获取所需实例扩展
- `glfwGetInstanceProcAddress(VkInstance instance, const char* procname)` - 获取实例函数地址
- `glfwGetPhysicalDevicePresentationSupport(VkInstance instance, VkPhysicalDevice device, uint32_t queuefamily)` - 检查物理设备呈现支持
- `glfwCreateWindowSurface(VkInstance instance, GLFWwindow* window, const VkAllocationCallbacks* allocator, VkSurfaceKHR* surface)` - 创建窗口表面

---

# 所有回调函数 (Callback Functions)

## 1. 错误处理回调
```c
typedef void (* GLFWerrorfun)(int error_code, const char* description);
```
**用途**: 错误发生时调用  
**参数**: 错误代码、错误描述

---

## 2. 监视器回调
```c
typedef void (* GLFWmonitorfun)(GLFWmonitor* monitor, int event);
```
**用途**: 监视器连接/断开时调用  
**参数**: 监视器句柄、事件类型(GLFW_CONNECTED/GLFW_DISCONNECTED)

---

## 3. 窗口位置回调
```c
typedef void (* GLFWwindowposfun)(GLFWwindow* window, int xpos, int ypos);
```
**用途**: 窗口移动时调用  
**参数**: 窗口句柄、新的x坐标、新的y坐标

---

## 4. 窗口尺寸回调
```c
typedef void (* GLFWwindowsizefun)(GLFWwindow* window, int width, int height);
```
**用途**: 窗口大小改变时调用  
**参数**: 窗口句柄、新宽度、新高度

---

## 5. 窗口关闭回调
```c
typedef void (* GLFWwindowclosefun)(GLFWwindow* window);
```
**用途**: 用户尝试关闭窗口时调用  
**参数**: 窗口句柄

---

## 6. 窗口刷新回调
```c
typedef void (* GLFWwindowrefreshfun)(GLFWwindow* window);
```
**用途**: 窗口内容需要刷新时调用  
**参数**: 窗口句柄

---

## 7. 窗口焦点回调
```c
typedef void (* GLFWwindowfocusfun)(GLFWwindow* window, int focused);
```
**用途**: 窗口获得/失去焦点时调用  
**参数**: 窗口句柄、焦点状态(GLFW_TRUE/GLFW_FALSE)

---

## 8. 窗口最小化回调
```c
typedef void (* GLFWwindowiconifyfun)(GLFWwindow* window, int iconified);
```
**用途**: 窗口最小化/恢复时调用  
**参数**: 窗口句柄、最小化状态(GLFW_TRUE/GLFW_FALSE)

---

## 9. 窗口最大化回调
```c
typedef void (* GLFWwindowmaximizefun)(GLFWwindow* window, int maximized);
```
**用途**: 窗口最大化/恢复时调用  
**参数**: 窗口句柄、最大化状态(GLFW_TRUE/GLFW_FALSE)

---

## 10. 帧缓冲尺寸回调
```c
typedef void (* GLFWframebuffersizefun)(GLFWwindow* window, int width, int height);
```
**用途**: 帧缓冲大小改变时调用  
**参数**: 窗口句柄、新宽度(像素)、新高度(像素)

---

## 11. 窗口内容缩放回调
```c
typedef void (* GLFWwindowcontentscalefun)(GLFWwindow* window, float xscale, float yscale);
```
**用途**: 窗口内容缩放改变时调用  
**参数**: 窗口句柄、x轴缩放、y轴缩放

---

## 12. 鼠标按钮回调
```c
typedef void (* GLFWmousebuttonfun)(GLFWwindow* window, int button, int action, int mods);
```
**用途**: 鼠标按钮按下/释放时调用  
**参数**: 窗口句柄、按钮ID、动作(PRESS/RELEASE)、修饰键

---

## 13. 光标位置回调
```c
typedef void (* GLFWcursorposfun)(GLFWwindow* window, double xpos, double ypos);
```
**用途**: 光标移动时调用  
**参数**: 窗口句柄、新x坐标、新y坐标

---

## 14. 光标进入回调
```c
typedef void (* GLFWcursorenterfun)(GLFWwindow* window, int entered);
```
**用途**: 光标进入/离开窗口内容区域时调用  
**参数**: 窗口句柄、进入状态(GLFW_TRUE/GLFW_FALSE)

---

## 15. 滚动回调
```c
typedef void (* GLFWscrollfun)(GLFWwindow* window, double xoffset, double yoffset);
```
**用途**: 滚动设备使用时调用  
**参数**: 窗口句柄、x轴偏移量、y轴偏移量

---

## 16. 键盘按键回调
```c
typedef void (* GLFWkeyfun)(GLFWwindow* window, int key, int scancode, int action, int mods);
```
**用途**: 键盘按键按下/释放/重复时调用  
**参数**: 窗口句柄、按键码、扫描码、动作(PRESS/RELEASE/REPEAT)、修饰键

---

## 17. 字符输入回调
```c
typedef void (* GLFWcharfun)(GLFWwindow* window, unsigned int codepoint);
```
**用途**: Unicode字符输入时调用  
**参数**: 窗口句柄、Unicode码点

---

## 18. 带修饰符字符回调（已废弃）
```c
typedef void (* GLFWcharmodsfun)(GLFWwindow* window, unsigned int codepoint, int mods);
```
**用途**: Unicode字符输入时调用(包含修饰键信息)  
**参数**: 窗口句柄、Unicode码点、修饰键  
**注意**: 计划在4.0版本移除

---

## 19. 路径拖放回调
```c
typedef void (* GLFWdropfun)(GLFWwindow* window, int path_count, const char* paths[]);
```
**用途**: 文件/目录拖放到窗口时调用  
**参数**: 窗口句柄、路径数量、路径数组

---

## 20. 摇杆配置回调
```c
typedef void (* GLFWjoystickfun)(int jid, int event);
```
**用途**: 摇杆连接/断开时调用  
**参数**: 摇杆ID、事件类型(CONNECTED/DISCONNECTED)

---

## 内存分配回调（高级用途）

### 分配回调
```c
typedef void* (* GLFWallocatefun)(size_t size, void* user);
```
**用途**: GLFW需要分配内存时调用  
**参数**: 所需字节数、用户指针  
**返回**: 分配的内存地址

### 重新分配回调
```c
typedef void* (* GLFWreallocatefun)(void* block, size_t size, void* user);
```
**用途**: GLFW需要重新分配内存时调用  
**参数**: 原内存块、新大小、用户指针  
**返回**: 新内存块地址

### 释放回调
```c
typedef void (* GLFWdeallocatefun)(void* block, void* user);
```
**用途**: GLFW需要释放内存时调用  
**参数**: 要释放的内存块、用户指针

---

## 回调使用要点

1. **线程安全**: 大部分回调在主线程调用，错误回调可能在任意线程调用
2. **生命周期**: 回调在设置后一直有效，直到被替换或库终止
3. **返回值**: 设置回调函数会返回之前设置的回调（如有）
4. **移除回调**: 传入`NULL`可移除当前回调
5. **同步事件**: 窗口和输入回调在事件处理函数调用期间触发
6. **数据有效性**: 回调中的指针参数仅在回调返回前有效