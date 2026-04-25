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
)
set(PROJECT_SRC
    "src/main.cpp"
)
set(LIB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib")

add_executable(RendererApp
    ${PROJECT_SRC}
    ${STB_IMAGE_SRC}
    ${IMGUI_SRC}
)

# 添加头文件路径
target_include_directories(RendererApp PRIVATE
    "include"
    "src"
)

# 连接库
target_link_libraries(RendererApp PRIVATE
    # winpthread
    winpthread

    # d3d12 核心库
    d3d12
    dxgi
    dxguid

    # dxc 编译器
    ${LIB_PATH}/dxc/dxcompiler.lib
    ${LIB_PATH}/dxc/dxil.lib

    # glfw
    ${LIB_PATH}/glfw/libglfw3.a
) 

# 自动复制 dxc 的 dll
add_custom_command(TARGET RendererApp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/lib/dxc/dxcompiler.dll"
        $<TARGET_FILE_DIR:RendererApp>

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/lib/dxc/dxil.dll"
        $<TARGET_FILE_DIR:RendererApp>
)