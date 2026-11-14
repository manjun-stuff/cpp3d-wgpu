#pragma once

#include <string>
#include <vector>
#include <memory>
#include <array>
#include <cstdint>

struct WGPUDeviceImpl;
struct WGPUBufferImpl;
struct WGPURenderPipelineImpl;
struct WGPUSurfaceImpl;
struct WGPUQueueImpl;
struct WGPUInstanceImpl;
struct GLFWwindowImpl;

/**
 * Namespace containing settings for rendering.
 */
namespace RenderConfig
{
    /**
     * All render methods
     */
    enum RenderMethod
    {
        Forward,
        Deferred,
        Raymarched,
        Raytraced,
        Custom
    };

    /**
     * Paths for the rendering methods.
     */
    inline std::array<std::string, 5> renderMethodPaths = {"test.wgsl", "", "", "", ""}; // implement render methods later

    /**
     * Allows you to add your own rendering mode through its file.
     * Set the render mode to custom in your renderer to use it.
     */
    void SetCustomRenderMethod(const std::string &renderPath);
}

/**
 * Main namespace containing engine functions, classes, and variables.
 */
namespace Engine
{
    /**
     * Value of pi (approximated).
     */
    inline constexpr float PI = 3.14159265359f;
    /**
     * Convert from degrees to radians.
     */
    inline constexpr float DEG2RAD = PI / 180;
    /**
     * Convert from radians to degrees.
     */
    inline constexpr float RAD2DEG = 180 / PI;

    class Renderer;

    /**
     * Basic vector containing 3 float components.
     */
    struct Vector3
    {
        /**
         * Vector component.
         */
        float x, y, z;
    };
    /**
     * Basic color struct, has red, green, blue, and alpha channels, from 0.0 - 1.0 .
     */
    struct Color
    {
        /**
         * Color channel.
         */
        float r, g, b, a;
    };
    /**
     * Quaternion class containing 4 float components.
     */
    class Quaternion
    {
    public:
        /**
         * Quaternion component.
         */
        float x, y, z, w;

        /**
         * Get the vector equivalent of a quaternion.
         */
        static Vector3 QuaternionToEuler(const Quaternion &quat);
        /**
         * Get the quaternion equivalent of a vector.
         */
        static Quaternion EulerToQuaternion(Vector3 vec);
        /**
         * Get the Hamilton product of two quaternions.
         */
        Quaternion operator*(const Quaternion &other) const;
    };

    /**
     * Material class, is what is used to render objects with the material attached.
     */
    class Material
    {
    public:
        /**
         * Create a new material.
         */
        Material(const std::string &wgslFilePath, Renderer &renderer);
        /**
         * Destroy the material.
         * Not recommended if it is allocated on the stack.
         */
        ~Material();

        /**
         * Set a property of the material.
         */
        void SetProperty(const std::string &name, float value);
        /**
         * Set a vector property of the material
         */
        void SetVector(const std::string &name, Vector3 vec);

    private:
        friend class Renderer;
        std::unique_ptr<WGPURenderPipelineImpl> m_pipeline = nullptr;
        std::string m_shaderPath;
    };

    /**
     * Basic geometry struct that houses vertices and indices.
     */
    struct Geometry
    {
        /**
         * Vertices of the geometry.
         */
        std::vector<Vector3> vertices;
        /**
         * Indices of the geometry.
         */
        std::vector<uint32_t> indices;
    };

    /**
     * Basic mesh class that is passed to the renderer to render.
     */
    class Mesh
    {
    public:
        /**
         * Create a mesh.
         */
        Mesh(const Geometry &geometry, std::shared_ptr<Material> material, Renderer &renderer);
        /**
         * Delete a mesh.
         */
        ~Mesh();

        /**
         * The position of the mesh.
         * This is applied to the geometry during rendering.
         */
        Vector3 position = {0.f, 0.f, 0.f};
        /**
         * The rotation of the mesh.
         * This is applied to the geometry during rendering.
         */
        Quaternion rotation = {0.f, 0.f, 0.f, 1.f};
        /**
         * The rotation of the mesh in euler form (Vector3).
         */
        Vector3 eulerRotation = {0.f, 0.f, 0.f};
        /**
         * The scale of the mesh.
         * This is applied to the geometry during rendering
         */
        Vector3 scale = {1.f, 1.f, 1.f};

    private:
        friend class Renderer;
        std::shared_ptr<Material> m_material;
        std::unique_ptr<WGPUBufferImpl> m_vertexBuffer = nullptr;
        std::unique_ptr<WGPUBufferImpl> m_indexBuffer = nullptr;
        uint32_t m_indexCount = 0;
    };

    /**
     * Basic camera class that is what the scene is rendered from.
     */
    class Camera
    {
    public:
        /**
         * The position of the camera.
         */
        Vector3 position = {0.0f, 0.0f, 5.0f};
        /**
         * The rotation of the camera in quaternion form.
         */
        Quaternion rotation = {0.f, 0.f, 0.f, 1.f};
        /**
         * The rotation of the camera as a Vector3.
         */
        Vector3 eulerRotation = {0.f, 0.f, 0.f};
        /**
         * How many degrees the camera can see.
         */
        float fov = 45.0f;
        /**
         * The aspect ratio of the camera.
         */
        float aspect = 16.0f / 9.0f;
    };

    /**
     * Renderer class that renders things onto the screen using WebGPU.
     */
    class Renderer
    {
    public:
        /**
         * Create a renderer.
         */
        Renderer(int width, int height, const std::string &windowTitle);
        /**
         * Destroy a renderer.
         */
        ~Renderer();

        // Lifecycle of window
        /**
         * Returns true or false depending on if the window is closing or not.
         */
        bool ShouldClose() const;
        /**
         * Tells GLFW to poll events.
         */
        void PollEvents();

        // API resembling Three.js
        /**
         * Clear the screen and fill it with a certain color.
         */
        void Clear(const Color &color = {0.f, 0.f, 0.f, 1.0f});
        /**
         * Render something to the current buffer.
         */
        void Render(const Mesh &mesh, const Camera &camera);
        /**
         * Present the buffer to the screen.
         */
        void Present();

        /**
         * Get the WebGPU device.
         */
        WGPUDeviceImpl GetInternalDevice();
        /**
         * Get the GLFW window.
         */
        GLFWwindowImpl GetWindow();

        /**
         * The current method that is being utilized to render.
         */
        RenderConfig::RenderMethod renderMethod;

    private:
        bool InitWindow(int width, int height, const std::string &title);
        bool InitWebGPU();

        std::unique_ptr<GLFWwindowImpl> m_window = nullptr;
        std::unique_ptr<WGPUDeviceImpl> m_device = nullptr; // wrapper around WGPUDevice
        WGPUInstanceImpl m_wgpuInstance;

        // details hidden in C++ file
        std::unique_ptr<WGPUSurfaceImpl> m_surface;
        std::unique_ptr<WGPUQueueImpl> m_queue;
    };
}
