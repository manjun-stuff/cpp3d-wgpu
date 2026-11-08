#pragma once

#include <string>
#include <vector>
#include <memory>

struct WGPUDeviceImpl;
struct WGPUBufferImpl;
struct WGPURenderPipelineImpl;
struct GLFWwindowImpl;

namespace Engine
{
    struct Vector3
    {
        float x, y, z;
    };
    struct Color
    {
        float r, g, b, a;
    };
    class Quaternion
    {
    public:
        float x, y, z, w;

        static Vector3 QuaternionToEuler(Quaternion &quat);
    };

    class Material
    {
    public:
        Material(const std::string &wgslFilePath, Renderer &renderer);
        ~Material();

        void SetProperty(const std::string &name, float value);
        void SetVector(const std::string &name, Vector3 vec);

    private:
        friend class Renderer;
        std::unique_ptr<WGPURenderPipelineImpl> m_pipeline = nullptr;
        std::string m_shaderPath;
    };

    struct Geometry
    {
        std::vector<Vector3> vertices;
        std::vector<uint32_t> indices;
    };

    class Mesh
    {
    public:
        Mesh(const Geometry &geometry, std::shared_ptr<Material> material, Renderer &renderer);
        ~Mesh();

        Vector3 position = {0.f, 0.f, 0.f};
        Quaternion rotation = {0.f, 0.f, 0.f, 0.f};
        Vector3 eulerRotation = {0.f, 0.f, 0.f};
        Vector3 scale = {1.f, 1.f, 1.f};

    private:
        friend class Renderer;
        std::shared_ptr<Material> m_material;
        std::unique_ptr<WGPUBufferImpl> m_vertexBuffer = nullptr;
        std::unique_ptr<WGPUBufferImpl> m_indexBuffer = nullptr;
        uint32_t m_indexCount = 0;
    };

    class Camera
    {
    public:
        Vector3 position = {0.0f, 0.0f, 5.0f};
        float fov = 45.0f;
        float aspect = 16.0f / 9.0f;
    };

    class Renderer
    {
    public:
        Renderer(int width, int height, const std::string &windowTitle);
        ~Renderer();

        // Lifecycle of window
        bool ShouldClose() const;
        void PollEvents();

        // API resembling Three.js
        void Clear(const Color &color = {0.1f, 0.1f, 0.1f, 1.0f});
        void Render(const Mesh &mesh, const Camera &camera);
        void Present();

        WGPUDeviceImpl GetInternalDevice();
        GLFWwindowImpl GetWindow();

    private:
        void InitWindow(int width, int height, const std::string &title);
        void InitWebGPU();

        GLFWwindowImpl *m_window = nullptr;
        WGPUDeviceImpl *m_device = nullptr; // wrapper around WGPUDevice

        // details hidden in C++ file
        void *m_surface = nullptr;
        void *m_queue = nullptr;
    };
}
