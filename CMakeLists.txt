cmake_minimum_required(VERSION 3.20)


project(HamuRenderer)


# 项目属性设置
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin) # 设置运行时输出目录（.dll 文件）
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib) # 设置库文件输出目录（.lib 导入库）


# 添加子目录库: 导入维护第三方库的 CMakeLists.txt
add_subdirectory(third_party)



# ------------------------------- 引擎项目, 抽离出动态库 ---------------------------------
add_library(hamu_engine SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test.cpp
)

target_include_directories(hamu_engine PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(hamu_engine PRIVATE
    winpthread
    opengl32
    
    entt
    glad
    glfw
    glm
    imgui
    spdlog
    stb_image
)

target_compile_definitions(hamu_engine PRIVATE
    IMGUI_IMPL_OPENGL_LOADER_CUSTOM
    HAMU_ENGINE_BUILD
)

# target_precompile_headers(hamu_engine PRIVATE
#     "${CMAKE_CURRENT_SOURCE_DIR}/include/pch.h"
# )



# -------------------------------- 沙盒项目, 用于测试 ----------------------------------
add_executable(sandbox
    ${CMAKE_CURRENT_SOURCE_DIR}/sandbox/main.cpp
)

target_include_directories(sandbox PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(sandbox PRIVATE 
    hamu_engine
)
