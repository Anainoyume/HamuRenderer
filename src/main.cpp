// =====================================================================
// 核心魔法：解决 MinGW / Clang 下 COM 接口的 GUID 链接错误
// 必须在包含任何 Windows / D3D 头文件之前定义 INITGUID
// =====================================================================
#define INITGUID 
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h> // 必须在最前面，解决 HRESULT 找不到的问题
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxcapi.h>  // 现代 DXC 编译器头文件
#include <wrl/client.h>

// 引入 GLFW (配置为暴露 Win32 原生句柄)
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <iostream>
#include <vector>
#include <string>

using Microsoft::WRL::ComPtr;

// 辅助函数：COM 错误检测
inline void ThrowIfFailed(HRESULT hr) {
    if (FAILED(hr)) {
        throw std::runtime_error("D3D12 API failed. HRESULT: " + std::to_string(hr));
    }
}

// 现代 DXC 着色器编译封装
ComPtr<IDxcBlob> CompileShader(
    ComPtr<IDxcUtils>& utils, 
    ComPtr<IDxcCompiler3>& compiler, 
    const std::string& source, 
    const wchar_t* profile) 
{
    DxcBuffer srcBuffer = { source.data(), source.size(), CP_UTF8 };
    
    // 编译参数：入口点为 main，指定 profile (如 vs_6_0)
    std::vector<LPCWSTR> args = {
        L"-E", L"main",
        L"-T", profile,
        DXC_ARG_DEBUG,
        DXC_ARG_SKIP_OPTIMIZATIONS
    };

    ComPtr<IDxcResult> result;
    ThrowIfFailed(compiler->Compile(
        &srcBuffer, 
        args.data(), (uint32_t)args.size(), 
        nullptr, 
        IID_PPV_ARGS(&result)
    ));

    // 检查编译错误
    ComPtr<IDxcBlobUtf8> errors;
    result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr);
    if (errors && errors->GetStringLength() > 0) {
        std::cerr << "Shader Compile Error: " << errors->GetStringPointer() << std::endl;
        throw std::runtime_error("Shader compilation failed.");
    }

    ComPtr<IDxcBlob> shader;
    ThrowIfFailed(result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shader), nullptr));
    return shader;
}

// =====================================================================
// HLSL 着色器源码 (内联字符串，避免读取文件带来的路径麻烦)
// =====================================================================
const std::string vertexShaderSource = R"(
struct VSInput {
    float3 position : POSITION;
    float4 color : COLOR;
};
struct VSOutput {
    float4 position : SV_POSITION;
    float4 color : COLOR;
};
VSOutput main(VSInput input) {
    VSOutput output;
    output.position = float4(input.position, 1.0f);
    output.color = input.color;
    return output;
}
)";

const std::string pixelShaderSource = R"(
struct PSInput {
    float4 position : SV_POSITION;
    float4 color : COLOR;
};
float4 main(PSInput input) : SV_TARGET {
    return input.color;
}
)";

// =====================================================================
// 主函数
// =====================================================================
int main() {
    const uint32_t width = 800;
    const uint32_t height = 600;

    // 1. 初始化 GLFW 窗口
    if (!glfwInit()) throw std::runtime_error("Failed to init GLFW");
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // 告诉 GLFW 不要创建 OpenGL 上下文
    GLFWwindow* window = glfwCreateWindow(width, height, "Hamu Renderer - D3D12 Triangle", nullptr, nullptr);
    HWND hwnd = glfwGetWin32Window(window);

    // 2. 开启 D3D12 Debug 层 (强烈建议)
    ComPtr<ID3D12Debug> debugController;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
        debugController->EnableDebugLayer();
    }

    // 3. 创建基础工厂、设备和命令队列
    ComPtr<IDXGIFactory4> factory;
    ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&factory)));

    ComPtr<ID3D12Device> device;
    ThrowIfFailed(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device)));

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    ComPtr<ID3D12CommandQueue> commandQueue;
    ThrowIfFailed(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue)));

    // 4. 创建交换链 (Swap Chain)
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = 2;
    swapChainDesc.Width = width;
    swapChainDesc.Height = height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;

    ComPtr<IDXGISwapChain1> swapChain1;
    ThrowIfFailed(factory->CreateSwapChainForHwnd(
        commandQueue.Get(), hwnd, &swapChainDesc, nullptr, nullptr, &swapChain1));
    ComPtr<IDXGISwapChain3> swapChain;
    swapChain1.As(&swapChain);

    // 5. 创建描述符堆 (RTV) 和渲染目标视图
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = 2;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    ComPtr<ID3D12DescriptorHeap> rtvHeap;
    ThrowIfFailed(device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap)));

    uint32_t rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    ComPtr<ID3D12Resource> renderTargets[2];
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
    for (uint32_t i = 0; i < 2; i++) {
        ThrowIfFailed(swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTargets[i])));
        device->CreateRenderTargetView(renderTargets[i].Get(), nullptr, rtvHandle);
        rtvHandle.ptr += rtvDescriptorSize;
    }

    // 6. 创建命令分配器和命令列表
    ComPtr<ID3D12CommandAllocator> commandAllocator;
    ThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator)));
    ComPtr<ID3D12GraphicsCommandList> commandList;
    ThrowIfFailed(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList)));
    commandList->Close(); // 创建时默认是开启状态，先关闭

    // 7. 同步对象 (Fence)
    ComPtr<ID3D12Fence> fence;
    ThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
    uint64_t fenceValue = 1;
    HANDLE fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

    // 8. 创建空的根签名 (Root Signature)
    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
    rootSignatureDesc.NumParameters = 0;
    rootSignatureDesc.pParameters = nullptr;
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    ComPtr<ID3DBlob> signature, error;
    ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
    ComPtr<ID3D12RootSignature> rootSignature;
    ThrowIfFailed(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature)));

    // 9. 使用现代 DXC 编译着色器
    ComPtr<IDxcUtils> dxcUtils;
    ThrowIfFailed(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils)));
    ComPtr<IDxcCompiler3> dxcCompiler;
    ThrowIfFailed(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler)));

    ComPtr<IDxcBlob> vertexShader = CompileShader(dxcUtils, dxcCompiler, vertexShaderSource, L"vs_6_0");
    ComPtr<IDxcBlob> pixelShader = CompileShader(dxcUtils, dxcCompiler, pixelShaderSource, L"ps_6_0");

    // 10. 定义顶点输入布局并创建管线状态对象 (PSO)
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { inputElementDescs, 2 };
    psoDesc.pRootSignature = rootSignature.Get();
    psoDesc.VS = { vertexShader->GetBufferPointer(), vertexShader->GetBufferSize() };
    psoDesc.PS = { pixelShader->GetBufferPointer(), pixelShader->GetBufferSize() };
    
    // 原生结构体繁琐的默认状态填充
    psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
    psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;

    ComPtr<ID3D12PipelineState> pipelineState;
    ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState)));

    // 11. 创建顶点缓冲区 (包含坐标和颜色)
    float triangleVertices[] = {
         0.0f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f, 1.0f, // 顶部，红色
         0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f, 1.0f, // 右下，绿色
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f, 1.0f  // 左下，蓝色
    };
    uint32_t vertexBufferSize = sizeof(triangleVertices);

    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD; // 简单起见，直接放在 Upload Heap
    D3D12_RESOURCE_DESC bufferDesc = {};
    bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    bufferDesc.Width = vertexBufferSize;
    bufferDesc.Height = 1;
    bufferDesc.DepthOrArraySize = 1;
    bufferDesc.MipLevels = 1;
    bufferDesc.SampleDesc.Count = 1;
    bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    ComPtr<ID3D12Resource> vertexBuffer;
    ThrowIfFailed(device->CreateCommittedResource(
        &heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc, 
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertexBuffer)));

    // 映射内存并拷贝数据
    void* pVertexDataBegin;
    vertexBuffer->Map(0, nullptr, &pVertexDataBegin);
    memcpy(pVertexDataBegin, triangleVertices, vertexBufferSize);
    vertexBuffer->Unmap(0, nullptr);

    D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};
    vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
    vertexBufferView.StrideInBytes = 7 * sizeof(float);
    vertexBufferView.SizeInBytes = vertexBufferSize;

    // 12. 视口与裁剪矩形
    D3D12_VIEWPORT viewport = { 0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f };
    D3D12_RECT scissorRect = { 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };

    // =====================================================================
    // 渲染主循环
    // =====================================================================
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        uint32_t frameIndex = swapChain->GetCurrentBackBufferIndex();

        // 重置分配器和命令列表
        ThrowIfFailed(commandAllocator->Reset());
        ThrowIfFailed(commandList->Reset(commandAllocator.Get(), pipelineState.Get()));

        // 设置管线状态
        commandList->SetGraphicsRootSignature(rootSignature.Get());
        commandList->RSSetViewports(1, &viewport);
        commandList->RSSetScissorRects(1, &scissorRect);

        // 插入屏障：Present -> RenderTarget
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Transition.pResource = renderTargets[frameIndex].Get();
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        commandList->ResourceBarrier(1, &barrier);

        // 设置渲染目标并清除背景色 (深蓝色)
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandleFrame = rtvHeap->GetCPUDescriptorHandleForHeapStart();
        rtvHandleFrame.ptr += frameIndex * rtvDescriptorSize;
        commandList->OMSetRenderTargets(1, &rtvHandleFrame, FALSE, nullptr);
        
        const float clearColor[] = { 0.1f, 0.2f, 0.4f, 1.0f };
        commandList->ClearRenderTargetView(rtvHandleFrame, clearColor, 0, nullptr);
        commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        commandList->IASetVertexBuffers(0, 1, &vertexBufferView);

        // 绘制调用：3 个顶点
        commandList->DrawInstanced(3, 1, 0, 0);

        // 插入屏障：RenderTarget -> Present
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
        commandList->ResourceBarrier(1, &barrier);

        ThrowIfFailed(commandList->Close());

        // 提交命令列表
        ID3D12CommandList* ppCommandLists[] = { commandList.Get() };
        commandQueue->ExecuteCommandLists(1, ppCommandLists);

        // 呈现画面 (Present)
        ThrowIfFailed(swapChain->Present(1, 0));

        // CPU 等待 GPU 完成 (非常简陋的同步方式，仅作示例)
        const uint64_t fenceToWaitFor = fenceValue;
        ThrowIfFailed(commandQueue->Signal(fence.Get(), fenceToWaitFor));
        fenceValue++;
        if (fence->GetCompletedValue() < fenceToWaitFor) {
            ThrowIfFailed(fence->SetEventOnCompletion(fenceToWaitFor, fenceEvent));
            WaitForSingleObject(fenceEvent, INFINITE);
        }
    }

    // 清理
    CloseHandle(fenceEvent);
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}