# 解释一下 gladLoadGLLoader ?
---

强烈解释一下这里的 `gladLoadGLLoader` 为什么传参数为  `(GLADloadproc)glfwGetProcAddress`

首先我们要知道在 `glad.h` 里其实是有一键加载的函数的:

```cpp
int gladLoadGL(void) {
    int status = 0;

    if(open_gl()) {
        status = gladLoadGLLoader(&get_proc);
        close_gl();
    }

    return status;
}
```

可以看到这里调用了 `gladLoadGLLoader(&get_proc)`
这个 `get_proc` 又是啥呢？它就是个 `wglGetProcAddress` 的 warpper, 加入了更健壮性的处理。

返回这里的 `gladGetProcAddressPtr`, 完全可以看到它也是一个函数, 接收一个字符串, 返回一个地址。
而 `open_gl` 有如下代码:

```cpp
libGL = LoadLibraryW(L"opengl32.dll");
if(libGL != NULL) {
    void (* tmp)(void);
    tmp = (void(*)(void)) GetProcAddress(libGL, "wglGetProcAddress");
    gladGetProcAddressPtr = (PFNWGLGETPROCADDRESSPROC_PRIVATE) tmp;
    return gladGetProcAddressPtr != NULL;
}
```

显然就是 `wglGetProcAddress` 对吧。

那为什么 learnOpengl 里传入 `(GLADloadproc)glfwGetProcAddress` 呢?
`glfwGetProcAddress` 其实是一个更高级版本的 `"wglGetProcAddress"`

我们在安装 `glad.h` 我们选择了平台为 windows, 因此生成的 `open_gl` 里载入的
获取显卡驱动函数的方法自然加载了 `wglGetProcAddress`

但如果是其他平台呢? 显然我们要去下载一个其他平台的 `glad.h`

但是 `glfwGetProcAddress` 帮我们屏蔽了这个细节, 它在不同系统下会被加载为不同的 显卡驱动函数加载方法
因此我们手动调用 `gladLoadGLLoader`, 直接传入这个跨平台的方法会更好, 这样写出来的代码是跨平台的。

别忘记将类型转化为 `(GLADloadproc)`
  
```cpp
if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
}
```