cmake_minimum_required(VERSION 3.20)

project(HamuRenderer)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置一些路径
set(STB_IMAGE_SRC "include/stb_image/stb_image.cpp")
set(IMGUI_SRC 
    "include/imgui/imgui.cpp"
    "include/imgui/imgui_draw.cpp"
    "include/imgui/imgui_widgets.cpp"
    "include/imgui/imgui_tables.cpp"
    "include/imgui/imgui_demo.cpp"

    # "include/imgui/backends/imgui_impl_vulkan.cpp"
    "include/imgui/backends/imgui_impl_opengl3.cpp"

    "include/imgui/backends/imgui_impl_glfw.cpp"
)
set(PROJECT_SRC
    "src/main.cpp"
)
set(LIB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib") 
set(GLAD_SRC "${CMAKE_CURRENT_SOURCE_DIR}/include/glad/glad.cpp")

# 查找 Vulkan SDK (依赖系统的 VULKAN_SDK 环境变量)
# find_package(Vulkan REQUIRED)

add_executable(RendererApp
    ${PROJECT_SRC}
    ${STB_IMAGE_SRC}
    ${IMGUI_SRC}
    ${GLAD_SRC}
)

# 添加头文件和库路径
target_include_directories(RendererApp PRIVATE
    "include"
    "include/imgui"
    "src"
)

target_link_directories(RendererApp PRIVATE 
    "lib"
    "lib/glfw"
)

# 连接库
target_link_libraries(RendererApp PRIVATE
    winpthread
    
    # Vulkan::Vulkan
    opengl32
    
    glfw3
) 

# 我们改了 imgui 的源码, 加载器换成了我们自己的
target_compile_definitions(RendererApp PRIVATE
    IMGUI_IMPL_OPENGL_LOADER_CUSTOM
)

# 预编译头
target_precompile_headers(RendererApp PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include/pch.h"
)