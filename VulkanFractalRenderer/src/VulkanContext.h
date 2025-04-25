#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <vector>
#include <optional>
#include <string>
#include <memory>
#include <array>
#include <Windows.h>

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    
    bool isComplete() const {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class VulkanContext {
public:
    VulkanContext(HWND hwnd, int width, int height);
    ~VulkanContext();

    // Delete copy constructors
    VulkanContext(const VulkanContext&) = delete;
    VulkanContext& operator=(const VulkanContext&) = delete;

    // Initialization
    void InitVulkan();
    void CreateSwapChain();
    void RecreateSwapChain();
    void CleanupSwapChain();

    // Getters for renderer
    VkDevice GetDevice() const { return m_device; }
    VkPhysicalDevice GetPhysicalDevice() const { return m_physicalDevice; }
    VkQueue GetGraphicsQueue() const { return m_graphicsQueue; }
    VkQueue GetPresentQueue() const { return m_presentQueue; }
    VkCommandPool GetCommandPool() const { return m_commandPool; }
    VkSwapchainKHR GetSwapChain() const { return m_swapChain; }
    VkFormat GetSwapChainImageFormat() const { return m_swapChainImageFormat; }
    VkExtent2D GetSwapChainExtent() const { return m_swapChainExtent; }
    const std::vector<VkImage>& GetSwapChainImages() const { return m_swapChainImages; }
    const std::vector<VkImageView>& GetSwapChainImageViews() const { return m_swapChainImageViews; }

    // Info for resource management
    uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    // Command buffer helpers
    VkCommandBuffer BeginSingleTimeCommands();
    void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

    // Frame handling
    uint32_t AcquireNextImage(VkSemaphore imageAvailableSemaphore);
    void PresentImage(uint32_t imageIndex, VkSemaphore renderFinishedSemaphore);

    // Resize handling
    void SetWindowSize(int width, int height) {
        m_width = width;
        m_height = height;
        m_framebufferResized = true;
    }

private:
    // Initialization helper functions
    void CreateInstance();
    void SetupDebugMessenger();
    void CreateSurface();
    void PickPhysicalDevice();
    void CreateLogicalDevice();
    void CreateCommandPool();
    
    // Device related helpers
    bool IsDeviceSuitable(VkPhysicalDevice device);
    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
    bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
    SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
    
    // Swapchain helpers 
    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    void CreateImageViews();

    // Debug helpers
    bool CheckValidationLayerSupport();
    void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    
    // Window handle
    HWND m_hwnd;
    int m_width;
    int m_height;
    bool m_framebufferResized;

    // Vulkan objects
    VkInstance m_instance;
    VkDebugUtilsMessengerEXT m_debugMessenger;
    VkSurfaceKHR m_surface;
    VkPhysicalDevice m_physicalDevice;
    VkDevice m_device;
    VkQueue m_graphicsQueue;
    VkQueue m_presentQueue;
    VkCommandPool m_commandPool;

    // Swap chain
    VkSwapchainKHR m_swapChain;
    std::vector<VkImage> m_swapChainImages;
    VkFormat m_swapChainImageFormat;
    VkExtent2D m_swapChainExtent;
    std::vector<VkImageView> m_swapChainImageViews;

    // Validation layer settings
    const std::vector<const char*> m_validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

    // Required device extensions
    const std::vector<const char*> m_deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

#ifdef NDEBUG
    const bool m_enableValidationLayers = false;
#else
    const bool m_enableValidationLayers = true;
#endif
};
