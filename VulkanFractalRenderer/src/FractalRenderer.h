#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <array>
#include <string>
#include <memory>

class VulkanContext;

// Fractal types
enum FractalType {
    FRACTAL_MANDELBROT = 0,
    FRACTAL_JULIA,
    FRACTAL_BURNING_SHIP,
    FRACTAL_TRICORN,
    FRACTAL_MULTIBROT,
    FRACTAL_COUNT
};

// Color palettes
enum ColorPalette {
    PALETTE_RAINBOW = 0,
    PALETTE_FIRE,
    PALETTE_OCEAN,
    PALETTE_GRAYSCALE,
    PALETTE_ELECTRIC,
    PALETTE_COUNT
};

// Uniform buffer for shader parameters
struct FractalUBO {
    float centerX;
    float centerY;
    float scale;
    float aspectRatio;
    
    int fractalType;
    int maxIterations;
    int colorPalette;
    int padding;
    
    // For Julia set
    float juliaConstantX;
    float juliaConstantY;
    // For Multibrot
    float multibrotPower;
    float reserved;
};

class FractalRenderer {
public:
    FractalRenderer(VulkanContext* vulkanContext);
    ~FractalRenderer();

    // Delete copy constructors
    FractalRenderer(const FractalRenderer&) = delete;
    FractalRenderer& operator=(const FractalRenderer&) = delete;

    // Initialize renderer components
    void Initialize();
    void Cleanup();
    void RecreateSwapChain();

    // Render one frame
    void RenderFrame();

    // Update parameters
    void SetFractalType(FractalType type);
    void SetMaxIterations(int iterations);
    void SetColorPalette(ColorPalette palette);
    void SetZoom(float zoom);
    void SetPan(float x, float y);
    void ResetView();

private:
    // Helper initialization functions
    void CreateRenderPass();
    void CreateDescriptorSetLayout();
    void CreateGraphicsPipeline();
    void CreateFramebuffers();
    void CreateUniformBuffers();
    void CreateDescriptorPool();
    void CreateDescriptorSets();
    void CreateCommandBuffers();
    void CreateSyncObjects();

    // Helper function to update uniform buffer
    void UpdateUniformBuffer(uint32_t currentImage);

    // Shader module creation helper
    VkShaderModule CreateShaderModule(const std::vector<char>& code);
    
    // Helper for loading shader files
    static std::vector<char> ReadFile(const std::string& filename);

    // Cleanup
    void CleanupSwapChain();

    // Reference to the Vulkan context
    VulkanContext* m_vulkanContext;

    // Render pass and pipeline
    VkRenderPass m_renderPass;
    VkDescriptorSetLayout m_descriptorSetLayout;
    VkPipelineLayout m_pipelineLayout;
    VkPipeline m_graphicsPipeline;

    // Framebuffers for rendering
    std::vector<VkFramebuffer> m_swapChainFramebuffers;

    // Uniform buffers for fractal parameters
    std::vector<VkBuffer> m_uniformBuffers;
    std::vector<VkDeviceMemory> m_uniformBuffersMemory;
    std::vector<void*> m_uniformBuffersMapped;

    // Descriptor pool and sets
    VkDescriptorPool m_descriptorPool;
    std::vector<VkDescriptorSet> m_descriptorSets;

    // Command buffers for rendering
    std::vector<VkCommandBuffer> m_commandBuffers;

    // Synchronization objects
    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::vector<VkFence> m_inFlightFences;
    uint32_t m_currentFrame;

    // Fractal view parameters
    FractalUBO m_ubo;

    // Constants
    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;
};
