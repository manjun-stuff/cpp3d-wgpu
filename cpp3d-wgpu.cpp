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

    Material::Material(const std::string &wgslFilePath, Renderer renderer)
    {
        m_pipeline = std::make_unique<WGPURenderPipelineImpl>();
        m_shaderPath = wgslFilePath;
        WGPUDevice device = renderer.GetInternalDevice().handle;
        std::ifstream file(wgslFilePath);
        if (!file.is_open())
        {
            std::cerr << "PathError (Material): Failed to open WGSL shader at" << wgslFilePath << "\n";
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string shaderCode = buffer.str();

        WGPUShaderSourceWGSL wgslShaderSource;
        wgslShaderSource.chain.sType = WGPUSType_ShaderSourceWGSL;
        wgslShaderSource.chain.next = nullptr;
        wgslShaderSource.code.data = shaderCode.c_str();
        wgslShaderSource.code.length = shaderCode.length();

        WGPUShaderModuleDescriptor shaderModuleDesc = {};
        shaderModuleDesc.nextInChain = &wgslShaderSource.chain;
        shaderModuleDesc.label.data = wgslFilePath.c_str();
        shaderModuleDesc.label.length = WGPU_STRLEN;

        WGPUShaderModule shaderModule = wgpuDeviceCreateShaderModule(device, &shaderModuleDesc);

        WGPUPipelineLayoutDescriptor layoutDesc;
        layoutDesc.bindGroupLayoutCount = 0;
        layoutDesc.bindGroupLayouts = nullptr;
        WGPUPipelineLayout pipelineLayout = wgpuDeviceCreatePipelineLayout(device, &layoutDesc);

        WGPURenderPipelineDescriptor pipelineDesc;
        pipelineDesc.layout = pipelineLayout;

        pipelineDesc.vertex.module = shaderModule;
        pipelineDesc.vertex.entryPoint.data = "vs_main";
        pipelineDesc.vertex.entryPoint.length = WGPU_STRLEN;
        pipelineDesc.vertex.bufferCount = 0;
        pipelineDesc.vertex.buffers = nullptr;
    }
}