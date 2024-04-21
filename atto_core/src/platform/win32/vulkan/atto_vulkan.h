#pragma once
#include "../../shared/atto_defines.h"

#if ATTO_VULKAN

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "bootstrap/VkBootstrap.h"

namespace atto {

    class WindowsCore;

    enum class VertexDescriptionType {
        POS,
        POS_UV
    };

    u32 VertexDescriptionTypeStide( VertexDescriptionType v );

    struct VertexInputDescription {
        std::vector<VkVertexInputBindingDescription> bindings;
        std::vector<VkVertexInputAttributeDescription> attributes;
        VkPipelineVertexInputStateCreateFlags flags = 0;

        void Reset() {
            bindings.clear();
            attributes.clear();
            flags = 0;
        }
    };

    class GraphicsPipeline {
    public:
        inline void Init( Core * core, VkDevice device ) { this->core = core; this->device = device; }
        inline void SetVertexShaderCodeFile( const char * path ) { vertCodeFile = path; }
        inline void SetPixelSdaderCodeFile( const char * path ) { pixlCodeFile = path; }
        inline void SetTopology( VkPrimitiveTopology topology ) { this->topology = topology; }
        inline void SetCullModeFlags( VkCullModeFlags cullModeFlags ) { this->cullModeFlags = cullModeFlags; }
        inline void SetPolygonMode( VkPolygonMode polygonMode ) { this->polygonMode = polygonMode; }
        inline void SetColorBlending( bool enabled ) { colorBlendEnable = enabled; }
        inline void SetDepthWrite( bool enabled ) { depthWrite = enabled; }
        inline void SetDepthTest( bool enabled ) { depthTest = enabled; }
        inline void SetDepthCompareOp( VkCompareOp op ) { depthCompareOp = op; }
        inline void SetPushConstantCountAndSize( u32 count, u32 size ) { pushConstantCount = count; pushConstantSize = size; }
        inline void SetColorAttachmentCount( i32 colorAttachmentCount ) { this->colorAttachmentCount = colorAttachmentCount; }
        void CreatePipelineLayout();
        void CreateRenderPass( VkFormat swapChainImageFormat );
        void SetVertexDescription( VertexDescriptionType type );
        VkPipeline Build();

        VkRenderPass renderPass;
        VkDevice device;
        VkPipeline handle;
        VkPipelineLayout layout;

    private:
        VkShaderModule CreateShaderModule( const char * code, i64 size );
        VertexInputDescription vertexDescription;
        VkPrimitiveTopology topology;
        VkCullModeFlags cullModeFlags;
        VkPolygonMode polygonMode;
        bool colorBlendEnable;
        bool depthWrite;
        bool depthTest;
        u32 pushConstantCount;
        u32 pushConstantSize;
        VkCompareOp depthCompareOp;
        const char * vertCodeFile = nullptr;
        const char * pixlCodeFile = nullptr;
        VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;
        VkFormat stencilFormat = VK_FORMAT_UNDEFINED;
        i32 colorAttachmentCount;
        Core * core = nullptr;
        std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
    };

    class VulkanState {
    public:
        void Initialize( WindowsCore * core );
        void RenderRecord( VkCommandBuffer commandBuffer, u32 imageIndex );
        void RenderSubmit( DrawContext * dcxt, bool clearBackBuffers );

        i32 MAX_FRAMES_IN_FLIGHT = 2;
        WindowsCore * core;
        vkb::Instance instance;
        VkSurfaceKHR surface;
        vkb::PhysicalDevice physicalDevice;
        vkb::Device device;
        vkb::Swapchain swapchain;
        VkQueue presentQueue;
        VkQueue graphicsQueue;
        GraphicsPipeline pipeline;
        std::vector<VkImageView> swapChainImageViews;
        std::vector<VkFramebuffer> swapChainFramebuffers;
        VkCommandPool commandPool;
        std::vector<VkCommandBuffer> commandBuffers;
        std::vector<VkSemaphore> imageAvailableSemaphores;
        std::vector<VkSemaphore> renderFinishedSemaphores;
        std::vector<VkFence> inFlightFences;
        u32 frameIndex = 0;
    };

}

#endif
