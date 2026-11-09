#include <string>
#include <vector>
#include <memory>
#include <GLFW/glfw3.h>
#include "cpp3d-wgpu.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string_view>
#ifdef __EMSCRIPTEN__
#include <emscripten/html5.h>
#include <webgpu/webgpu.h>
#else
#include <glfw3webgpu-main/glfw3webgpu.h>
#include <webgpu-headers/webgpu.h>
#endif

struct WGPUDeviceImpl
{
    WGPUDevice handle;
};

struct WGPUBufferImpl
{
    WGPUBuffer handle;
};

struct WGPURenderPipelineImpl
{
    WGPURenderPipeline handle;
};

struct WGPUSurfaceImpl
{
    WGPUSurface handle;
};

struct WGPUQueueImpl
{
    WGPUQueue handle;
};

struct WGPUInstanceImpl
{
    WGPUInstance handle;
};

struct GLFWwindowImpl
{
    GLFWwindow *handle;
};

static WGPUSurfaceTexture s_currentFrameTexture = {};
static WGPUTextureView s_currentRenderTargetView = nullptr;
static WGPUCommandEncoder s_currentCommandEncoder = nullptr;

void RenderConfig::SetCustomRenderMethod(const std::string &renderPath)
{
    RenderConfig::renderMethodPaths[Custom] = renderPath;
}

WGPUSurface CreateWGPUSurfaceGLFW(WGPUInstance &instance, GLFWwindow *window)
{
#ifdef __EMSCRIPTEN__
    WGPUSurfaceSourceEmscriptenCanvasHTMLSelector canvasSelector = {};
    canvasSelector.chain.sType = WGPUSType_SurfaceSourceEmscriptenCanvasHTMLSelector;
    canvasSelector.selector = "#canvas";

    WGPUSurfaceDescriptor surfaceDesc = {};
    surfaceDesc.nextInChain = reinterpret_cast<const WGPUChainedStruct *>(&canvasSelector);
    return wgpuInstanceCreateSurface(instance, &surfaceDesc);
#else
    return glfwCreateWindowWGPUSurface(instance, window);
#endif
}

void HandleRequestDevice(WGPURequestDeviceStatus status, WGPUDevice device, WGPUStringView message, void *userdata1, void *userdata2)
{
    if (userdata2)
    {
        auto *isDone = static_cast<bool *>(userdata2);
        *isDone = true;
    }
    if (status == WGPURequestDeviceStatus_Success)
    {
        // Cast user data and store the device handle
        auto *outDevice = static_cast<WGPUDevice *>(userdata1);
        *outDevice = device;
    }
    else
    {
        std::cerr << "Request device failed: "
                  << std::string_view(message.data, message.length)
                  << std::endl;
    }
}

void HandleRequestAdapter(WGPURequestAdapterStatus status, WGPUAdapter adapter, WGPUStringView message, void *userdata1, void *userdata2)
{
    if (userdata2)
    {
        auto *isDone = static_cast<bool *>(userdata2);
        *isDone = true;
    }
    if (status == WGPURequestAdapterStatus_Success)
    {
        auto *outAdapter = static_cast<WGPUAdapter *>(userdata1);
        *outAdapter = adapter;
    }
    else
    {
        std::cerr << "Request adapter failed: "
                  << std::string_view(message.data, message.length)
                  << std::endl;
    }
}

namespace Engine
{
    Vector3 Quaternion::QuaternionToEuler(Quaternion &quat)
    {
        Vector3 result;
        result.x = std::atan2(-2.f * (quat.y * quat.z - quat.w * quat.x), 1.f - 2.f * (quat.x * quat.x + quat.y * quat.y)); // Calculate ɸ phi
        result.y = std::asin(2 * (quat.x * quat.z + quat.w * quat.y));                                                      // Calculate θ (theta)
        result.z = std::atan2(-2.f * (quat.x * quat.y - quat.w * quat.z), 1.f - 2.f * (quat.y * quat.y + quat.z * quat.z)); // Calculate Ψ (psi)
        return result;
    }

    Material::Material(const std::string &wgslFilePath, Renderer &renderer)
    {
        m_pipeline = std::make_unique<WGPURenderPipelineImpl>();
        m_shaderPath = wgslFilePath;
        WGPUDevice device = renderer.GetInternalDevice().handle;

        // open shader file and put in the format WebGPU feeds on
        std::ifstream file(wgslFilePath);
        if (!file.is_open())
        {
            std::cerr << "PathError (Material): Failed to open WGSL shader at" << wgslFilePath << ". \n";
            return;
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string shaderCode = buffer.str();

        WGPUShaderSourceWGSL wgslShaderSource = {};
        wgslShaderSource.chain.sType = WGPUSType_ShaderSourceWGSL;
        wgslShaderSource.chain.next = nullptr;
        wgslShaderSource.code.data = shaderCode.c_str();
        wgslShaderSource.code.length = shaderCode.length();

        WGPUShaderModuleDescriptor shaderModuleDesc = {};
        shaderModuleDesc.nextInChain = &wgslShaderSource.chain;
        shaderModuleDesc.label.data = wgslFilePath.c_str();
        shaderModuleDesc.label.length = WGPU_STRLEN;

        WGPUShaderModule shaderModule = wgpuDeviceCreateShaderModule(device, &shaderModuleDesc);

        // set up render pipeline
        WGPUPipelineLayoutDescriptor layoutDesc = {};
        layoutDesc.bindGroupLayoutCount = 0;
        layoutDesc.bindGroupLayouts = nullptr;
        WGPUPipelineLayout pipelineLayout = wgpuDeviceCreatePipelineLayout(device, &layoutDesc);

        WGPURenderPipelineDescriptor pipelineDesc = {};
        pipelineDesc.layout = pipelineLayout;

        WGPUVertexAttribute vertAttr = {};
        vertAttr.format = WGPUVertexFormat_Float32x3;
        vertAttr.offset = 0;
        vertAttr.shaderLocation = 0;

        WGPUVertexBufferLayout vertBufferLayout = {};
        vertBufferLayout.arrayStride = sizeof(float) * 3; // size of a Vector3
        vertBufferLayout.attributeCount = 1;
        vertBufferLayout.attributes = &vertAttr;

        // vertex shader
        pipelineDesc.vertex.module = shaderModule;
        pipelineDesc.vertex.entryPoint.data = "vs_main";
        pipelineDesc.vertex.entryPoint.length = WGPU_STRLEN;
        pipelineDesc.vertex.bufferCount = 1;
        pipelineDesc.vertex.buffers = &vertBufferLayout;

        // fragment shader
        WGPUColorTargetState colorTarget = {};
        colorTarget.format = WGPUTextureFormat_BGRA8Unorm;
        colorTarget.writeMask = WGPUColorWriteMask_All;
        colorTarget.blend = nullptr;

        WGPUFragmentState fragState = {};
        fragState.module = shaderModule;
        fragState.entryPoint.data = "fs_main";
        fragState.entryPoint.length = WGPU_STRLEN;
        fragState.targetCount = 1;
        fragState.targets = &colorTarget;

        pipelineDesc.fragment = &fragState;

        // primitive / topology config
        pipelineDesc.primitive.topology = WGPUPrimitiveTopology_TriangleList;
        pipelineDesc.primitive.stripIndexFormat = WGPUIndexFormat_Undefined;
        pipelineDesc.primitive.frontFace = WGPUFrontFace_CCW;
        pipelineDesc.primitive.cullMode = WGPUCullMode_None;

        // multisample config
        pipelineDesc.multisample.count = 1;
        pipelineDesc.multisample.mask = 0xFFFFFFFF;
        pipelineDesc.multisample.alphaToCoverageEnabled = false;

        // final
        m_pipeline->handle = wgpuDeviceCreateRenderPipeline(device, &pipelineDesc);

        // release unnecesary data
        wgpuShaderModuleRelease(shaderModule);
        wgpuPipelineLayoutRelease(pipelineLayout);
    }

    Material::~Material()
    {
        if (m_pipeline && m_pipeline->handle)
        {
            wgpuRenderPipelineRelease(m_pipeline->handle);
        }
    }

    Mesh::Mesh(const Geometry &geometry, std::shared_ptr<Material> material, Renderer &renderer)
        : m_material(material)
    {
        m_vertexBuffer = std::make_unique<WGPUBufferImpl>();
        m_indexBuffer = std::make_unique<WGPUBufferImpl>();
        WGPUDevice device = renderer.GetInternalDevice().handle;

        // vertex buffer
        WGPUBufferDescriptor vertDesc = {};
        vertDesc.usage = WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst;
        vertDesc.size = geometry.vertices.size() * sizeof(float) * 3;
        vertDesc.mappedAtCreation = false;
        vertDesc.label.data = "Mesh Vertex Buffer";
        vertDesc.label.length = WGPU_STRLEN;

        m_vertexBuffer->handle = wgpuDeviceCreateBuffer(device, &vertDesc);

        WGPUQueue queue = wgpuDeviceGetQueue(device);
        wgpuQueueWriteBuffer(queue, m_vertexBuffer->handle, 0, geometry.vertices.data(), vertDesc.size);

        // index buffer
        WGPUBufferDescriptor indexDesc = {};
        indexDesc.usage = WGPUBufferUsage_Index | WGPUBufferUsage_CopyDst;
        indexDesc.size = geometry.indices.size() * sizeof(uint32_t);
        indexDesc.mappedAtCreation = false;
        indexDesc.label.data = "Mesh Index Buffer";
        indexDesc.label.length = WGPU_STRLEN;

        m_indexBuffer->handle = wgpuDeviceCreateBuffer(device, &indexDesc);

        wgpuQueueWriteBuffer(queue, m_indexBuffer->handle, 0, geometry.indices.data(), indexDesc.size);
        m_indexCount = geometry.indices.size();

        wgpuQueueRelease(queue);
    }

    Mesh::~Mesh()
    {
        if (m_vertexBuffer && m_vertexBuffer->handle)
        {
            wgpuBufferRelease(m_vertexBuffer->handle);
        }
        if (m_indexBuffer && m_indexBuffer->handle)
        {
            wgpuBufferRelease(m_indexBuffer->handle);
        }
    }

    Renderer::Renderer(int width, int height, const std::string &windowTitle)
        : m_window(nullptr), m_device(nullptr), m_surface(nullptr), m_queue(nullptr), renderMethod(RenderConfig::Forward)
    {
        if (!InitWindow(width, height, windowTitle))
        {
            return;
        }

        if (!InitWebGPU())
        {
            return;
        }
    }

    Renderer::~Renderer()
    {
        if (m_queue && m_queue->handle)
            wgpuQueueRelease(m_queue->handle);
        if (m_surface && m_surface->handle)
        {
            wgpuSurfaceUnconfigure(m_surface->handle);
            wgpuSurfaceRelease(m_surface->handle);
        }
        if (m_device && m_device->handle)
            wgpuDeviceRelease(m_device->handle);
        if (m_window && m_window->handle)
        {
            glfwDestroyWindow(m_window->handle);
        }

        if (m_wgpuInstance.handle)
        {
            wgpuInstanceRelease(m_wgpuInstance.handle);
        }

        glfwTerminate();
    }

    bool Renderer::InitWebGPU()
    {
        m_device = std::make_unique<WGPUDeviceImpl>();
        m_device->handle = nullptr;

        WGPUInstanceDescriptor instanceDesc = {};
        m_wgpuInstance.handle = wgpuCreateInstance(&instanceDesc);
        if (!m_wgpuInstance.handle)
        {
            std::cerr << "WebGPU Error: WebGPU Instance could not be created" << std::endl;
            return false;
        }

        m_surface = std::make_unique<WGPUSurfaceImpl>();
        m_surface->handle = CreateWGPUSurfaceGLFW(m_wgpuInstance.handle, m_window->handle);
        if (!m_surface->handle)
        {
            std::cerr << "Surface Error: WebGPU Surface creation failed" << std::endl;
            return false;
        }

        WGPURequestAdapterOptions adapterOptions = {};
        adapterOptions.compatibleSurface = m_surface->handle;
        WGPUAdapter adapter = {};

        bool requestCompleted = false;

        WGPURequestAdapterCallbackInfo callbackInfo1 = {};
        callbackInfo1.callback = HandleRequestAdapter;
        callbackInfo1.userdata1 = &adapter;
        callbackInfo1.userdata2 = &requestCompleted;

        wgpuInstanceRequestAdapter(m_wgpuInstance.handle, &adapterOptions, callbackInfo1);

        while (!requestCompleted)
        {
#ifdef __EMSCRIPTEN__
            emscripten_sleep(0);
#else
            wgpuInstanceProcessEvents(m_wgpuInstance.handle);
#endif
        }

        requestCompleted = false;

        if (!adapter)
        {
            std::cerr << "Adapter Error: WebGPU Adapter creation failed. \n";
            return false;
        }

        // get device
        WGPURequestDeviceCallbackInfo callbackInfo2 = {};
        callbackInfo2.callback = HandleRequestDevice;
        callbackInfo2.userdata1 = &m_device->handle;
        callbackInfo2.userdata2 = &requestCompleted;

        WGPUDeviceDescriptor deviceDesc = {};
        deviceDesc.label.data = "Primary Engine Device";
        deviceDesc.label.length = WGPU_STRLEN;

        wgpuAdapterRequestDevice(adapter, &deviceDesc, callbackInfo2);

        while (!requestCompleted)
        {
#ifdef __EMSCRIPTEN__
            emscripten_sleep(0);
#else
            wgpuInstanceProcessEvents(m_wgpuInstance.handle);
#endif
        }

        if (!m_device->handle)
        {
            std::cerr << "Device Error: WebGPU Device creation failed. \n";
            wgpuAdapterRelease(adapter);
            return false;
        }

        WGPUSurfaceConfiguration surfaceConfig = {};
        surfaceConfig.device = m_device->handle;
        surfaceConfig.format = WGPUTextureFormat_BGRA8Unorm;
        surfaceConfig.usage = WGPUTextureUsage_RenderAttachment;

        int width, height;
        glfwGetWindowSize(m_window->handle, &width, &height);
        surfaceConfig.width = static_cast<uint32_t>(width);
        surfaceConfig.height = static_cast<uint32_t>(height);
        surfaceConfig.presentMode = WGPUPresentMode_Fifo;

        wgpuSurfaceConfigure(m_surface->handle, &surfaceConfig);

        m_queue = std::make_unique<WGPUQueueImpl>();
        m_queue->handle = wgpuDeviceGetQueue(m_device->handle);

        wgpuAdapterRelease(adapter);

        return true;
    }

    bool Renderer::InitWindow(int width, int height, const std::string &title)
    {
        m_window = std::make_unique<GLFWwindowImpl>();
        m_window->handle = nullptr;

        if (!glfwInit())
        {
            std::cerr << "Initialization Error: GLFW failed to initialize. \n";
            return false;
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        m_window->handle = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
        if (m_window->handle == nullptr)
        {
            std::cerr << "Failed to create GLFW window\n";
            glfwTerminate();
            return false;
        }
        return true;
    }

    WGPUDeviceImpl Renderer::GetInternalDevice()
    {
        return *m_device;
    }

    bool Renderer::ShouldClose() const
    {
        return glfwWindowShouldClose(m_window->handle);
    }

    void Renderer::PollEvents()
    {
        glfwPollEvents();
    }
}