#define GLFW_INCLUDE_VULKAN // 这一行极其重要，它指示 GLFW 帮我们包含 <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <vector>

class HelloVulkanEnvironment {
public:
    void run() {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    GLFWwindow* window = nullptr;
    VkInstance instance = nullptr;

    void initWindow() {
        // 初始化 GLFW
        if (!glfwInit()) {
            throw std::runtime_error("Failed to initialize GLFW!");
        }

        // 核心设置：明确告诉 GLFW 不要创建 OpenGL 上下文
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window = glfwCreateWindow(800, 600, "Vulkan Environment Test", nullptr, nullptr);
    }

    void initVulkan() {
        // 配置应用信息
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Hello Vulkan";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_3; // 使用 Vulkan 1.3

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        // 获取 GLFW 所需的 Vulkan 扩展 (如 VK_KHR_surface, VK_KHR_win32_surface)
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        
        createInfo.enabledExtensionCount = glfwExtensionCount;
        createInfo.ppEnabledExtensionNames = glfwExtensions;
        createInfo.enabledLayerCount = 0; // 暂时不开启验证层

        // 创建 Vulkan 实例
        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Vulkan instance!");
        }
        
        std::cout << "Vulkan Instance created successfully!" << std::endl;
        
        // 打印系统支持的扩展数量作为二次验证
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        std::cout << "Available Vulkan Extensions: " << extensionCount << std::endl;
    }

    void mainLoop() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
        }
    }

    void cleanup() {
        if (instance != nullptr) {
            vkDestroyInstance(instance, nullptr);
        }
        if (window != nullptr) {
            glfwDestroyWindow(window);
            glfwTerminate();
        }
    }
};

int main() {
    HelloVulkanEnvironment app;
    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}