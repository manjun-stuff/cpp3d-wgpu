#include <string>
#include <vector>
#include <memory>
#include <webgpu-headers/webgpu.h>
#include <GLFW/glfw3.h>
#include "cpp3d-wgpu.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

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

struct GLFWwindowImpl
{
    GLFWwindow *handle;
};

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
            std::cerr << "PathError (Material): Failed to open WGSL shader at" << wgslFilePath << "\n";
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

        // vertex shader
        pipelineDesc.vertex.module = shaderModule;
        pipelineDesc.vertex.entryPoint.data = "vs_main";
        pipelineDesc.vertex.entryPoint.length = WGPU_STRLEN;
        pipelineDesc.vertex.bufferCount = 0;
        pipelineDesc.vertex.buffers = nullptr;

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
        vertDesc.size = geometry.vertices.size() * sizeof(float);
        vertDesc.mappedAtCreation = false;
        vertDesc.label.data = "Mesh Vertex Buffer";
        vertDesc.label.length = WGPU_STRLEN;

        m_vertexBuffer->handle = wgpuDeviceCreateBuffer(device, &vertDesc);

        WGPUQueue queue = wgpuDeviceGetQueue(device);
        wgpuQueueWriteBuffer(queue, m_vertexBuffer->handle, 0, geometry.vertices.data(), vertDesc.size);

        // index buffer
        WGPUBufferDescriptor indexDesc = {};
        indexDesc.usage = WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst;
        indexDesc.size = geometry.vertices.size() * sizeof(float);
        indexDesc.mappedAtCreation = false;
        indexDesc.label.data = "Mesh Index Buffer";
        indexDesc.label.length = WGPU_STRLEN;

        m_indexBuffer->handle = wgpuDeviceCreateBuffer(device, &indexDesc);

        wgpuQueueWriteBuffer(queue, m_indexBuffer->handle, 0, geometry.indices.data(), vertDesc.size);

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
}