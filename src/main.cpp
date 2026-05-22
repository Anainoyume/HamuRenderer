#include "GLFW/glfw3.h"

namespace hamu {

    const char* glsl_version = "#version 330";

}

int main() {

    if (!glfwInit()) {
        spdlog::info("GLFW 初始化失败");
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();


}