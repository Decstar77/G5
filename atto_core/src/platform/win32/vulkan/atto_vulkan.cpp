#include "../atto_win32_core.h"
#include "atto_vulkan.h"

#if ATTO_VULKAN

namespace atto {
    inline void VK_CHECK(VkResult err) {
        if (err) {
            INVALID_CODE_PATH;
        }
    }

    void VulkanState::Initialize( WindowsCore * core ) {
        this->core = core;

        // @NOTE: Create instance
        vkb::InstanceBuilder instanceBuilder = vkb::InstanceBuilder();
        instanceBuilder.set_app_name( "Atto Vulkan" );
        instanceBuilder.request_validation_layers( true );
        instanceBuilder.use_default_debug_messenger();
        instanceBuilder.require_api_version( 1, 1, 0 );
        instance = instanceBuilder.build().value();

        // @NOTE: Create surface
        glfwCreateWindowSurface( instance, core->window.GetWindowPtr() , nullptr, &surface );

        // @NOTE: Pick device
        vkb::PhysicalDeviceSelector selector(instance);
        selector.add_required_extension( VK_KHR_SWAPCHAIN_EXTENSION_NAME );
        selector.set_minimum_version( 1, 1 );
        selector.set_surface( surface );
        physicalDevice = selector.select().value();

        // @NOTE: Create device
        vkb::DeviceBuilder deviceBuilder( physicalDevice );
        device = deviceBuilder.build().value();

        // @NOTE: Queues
        presentQueue = device.get_queue( vkb::QueueType::present ).value();
        graphicsQueue = device.get_queue( vkb::QueueType::graphics ).value();

        // @NOTE: Swapchain
        VkSurfaceFormatKHR format = {};
        format.colorSpace = VkColorSpaceKHR::VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        format.format = VkFormat::VK_FORMAT_R8G8B8A8_UNORM;

        i32 width = 0;
        i32 height = 0;
        core->window.GetFramebufferSize( width, height );

        vkb::SwapchainBuilder swapchainBuilder( physicalDevice, device, surface );
        swapchainBuilder.set_desired_format( format );
        swapchainBuilder.set_desired_present_mode( VK_PRESENT_MODE_IMMEDIATE_KHR );
        swapchainBuilder.set_desired_present_mode( VK_PRESENT_MODE_FIFO_KHR );
        swapchainBuilder.set_desired_present_mode( VK_PRESENT_MODE_MAILBOX_KHR );
        swapchainBuilder.set_desired_extent( width, height );
        swapchain = swapchainBuilder.build().value();
        swapChainImageViews = swapchain.get_image_views().value();

        pipeline.Init( core, device );
        pipeline.CreateRenderPass( swapchain.image_format );
        pipeline.CreatePipelineLayout();
        //pipeline.SetVertexDescription( VertexDescriptionType::POS );
        pipeline.SetTopology( VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST );
        pipeline.SetPolygonMode( VK_POLYGON_MODE_FILL );
        pipeline.SetCullModeFlags( VK_CULL_MODE_BACK_BIT );
        pipeline.SetColorBlending( false );
        pipeline.SetDepthTest( false );
        pipeline.SetDepthWrite( false );
        pipeline.SetDepthCompareOp( VK_COMPARE_OP_NEVER );
        pipeline.SetColorAttachmentCount( 1 );
        pipeline.SetVertexShaderCodeFile( "res/shaders/vert.spv" );
        pipeline.SetPixelSdaderCodeFile( "res/shaders/frag.spv" );
        pipeline.Build();

        swapChainFramebuffers.resize( swapChainImageViews.size() );
        for( size_t framebufferIndex = 0; framebufferIndex < swapChainImageViews.size(); framebufferIndex++ ) {
            VkImageView attachments[] = {
                swapChainImageViews[ framebufferIndex ]
            };

            VkFramebufferCreateInfo framebufferInfo = {};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = pipeline.renderPass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = swapchain.extent.width;
            framebufferInfo.height = swapchain.extent.height;
            framebufferInfo.layers = 1;
            VK_CHECK( vkCreateFramebuffer( device, &framebufferInfo, nullptr, &swapChainFramebuffers[ framebufferIndex ] ) );
        }

        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = device.get_queue_index( vkb::QueueType::graphics ).value();
        VK_CHECK( vkCreateCommandPool( device, &poolInfo, nullptr, &commandPool ) );

        commandBuffers.resize( MAX_FRAMES_IN_FLIGHT );
        VkCommandBufferAllocateInfo commandBufferAlocInfo = {};
        commandBufferAlocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAlocInfo.commandPool = commandPool;
        commandBufferAlocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferAlocInfo.commandBufferCount = (u32)commandBuffers.size();
        VK_CHECK( vkAllocateCommandBuffers( device, &commandBufferAlocInfo, commandBuffers.data() ) );

        imageAvailableSemaphores.resize( MAX_FRAMES_IN_FLIGHT );
        renderFinishedSemaphores.resize( MAX_FRAMES_IN_FLIGHT );
        inFlightFences.resize( MAX_FRAMES_IN_FLIGHT );

        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        for( size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++ ) {
            VK_CHECK( vkCreateSemaphore( device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[ i ] ) );
            VK_CHECK( vkCreateSemaphore( device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[ i ] ) );
            VK_CHECK( vkCreateFence( device, &fenceInfo, nullptr, &inFlightFences[ i ] ) );
        }
    }

    void VulkanState::RenderRecord( VkCommandBuffer commandBuffer, u32 imageIndex ) {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        VK_CHECK( vkBeginCommandBuffer( commandBuffer, &beginInfo ) );

        VkRenderPassBeginInfo renderPassInfo ={};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = pipeline.renderPass;
        renderPassInfo.framebuffer = swapChainFramebuffers[ imageIndex ];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = swapchain.extent;

        VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass( commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE );

        vkCmdBindPipeline( commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.handle );

        VkViewport viewport = {};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)swapchain.extent.width;
        viewport.height = (float)swapchain.extent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport( commandBuffer, 0, 1, &viewport );

        VkRect2D scissor ={};
        scissor.offset = { 0, 0 };
        scissor.extent = swapchain.extent;
        vkCmdSetScissor( commandBuffer, 0, 1, &scissor );

        vkCmdDraw( commandBuffer, 3, 1, 0, 0 );

        vkCmdEndRenderPass( commandBuffer );

        VK_CHECK( vkEndCommandBuffer( commandBuffer ) );
    }

    void VulkanState::RenderSubmit( DrawContext * dcxt, bool clearBackBuffers ) {
        vkWaitForFences( device, 1, &inFlightFences[ frameIndex ], VK_TRUE, UINT64_MAX );

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR( device, swapchain, UINT64_MAX, imageAvailableSemaphores[ frameIndex ], VK_NULL_HANDLE, &imageIndex );

        if( result == VK_ERROR_OUT_OF_DATE_KHR ) {
            //recreateSwapChain();
            return;
        }
        else if( result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR ) {
            INVALID_CODE_PATH;
            //throw std::runtime_error( "failed to acquire swap chain image!" );
        }

        vkResetFences( device, 1, &inFlightFences[ frameIndex ] );
        vkResetCommandBuffer( commandBuffers[ frameIndex ], 0 );
        RenderRecord( commandBuffers[ frameIndex ], imageIndex );
        
        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[ frameIndex ] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[ frameIndex ];

        VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[ frameIndex ] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        VK_CHECK( vkQueueSubmit( graphicsQueue, 1, &submitInfo, inFlightFences[ frameIndex ] ) );

        VkPresentInfoKHR presentInfo ={};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        VkSwapchainKHR swapChains[] = { swapchain.swapchain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;

        result = vkQueuePresentKHR( presentQueue, &presentInfo );

        //if( result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized ) {
        //    framebufferResized = false;
        //    recreateSwapChain();
        //}
        //else if( result != VK_SUCCESS ) {
        //    INVALID_CODE_PATH;
        //}

        frameIndex = ( frameIndex + 1 ) % MAX_FRAMES_IN_FLIGHT;
    }

    void WindowsCore::VkStart() {
        this->vkState = new VulkanState();
        this->vkState->Initialize( this );
    }

    u32 VertexDescriptionTypeStide( VertexDescriptionType type ) {
        if( type == VertexDescriptionType::POS ) {
            return sizeof( glm::vec2 );
        } else if ( type == VertexDescriptionType::POS_UV ) {
            return sizeof( glm::vec2 ) * 2;
        }
        INVALID_CODE_PATH;
        return 0;
    }

    void GraphicsPipeline::CreateRenderPass( VkFormat swapChainImageFormat ) {
        if (renderPass!= VK_NULL_HANDLE ) {
            vkDestroyRenderPass( device, renderPass, nullptr );
        }

        VkAttachmentDescription colorAttachment ={};
        colorAttachment.format = swapChainImageFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef = {};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        VkSubpassDependency dependency = {};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        VK_CHECK( vkCreateRenderPass( device, &renderPassInfo, nullptr, &renderPass ) );
    }

    void GraphicsPipeline::CreatePipelineLayout() {
        if (layout != VK_NULL_HANDLE ) {
            vkDestroyPipelineLayout( device, layout, nullptr );
        }

        VkPushConstantRange pushConstantRange = {};
        pushConstantRange.offset = 0;
        pushConstantRange.size = pushConstantSize;
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

        VkPipelineLayoutCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        createInfo.setLayoutCount = ( uint32_t )descriptorSetLayouts.size();
        createInfo.pSetLayouts = descriptorSetLayouts.data();
        createInfo.pPushConstantRanges = &pushConstantRange;
        createInfo.pushConstantRangeCount = pushConstantCount;
        createInfo.pNext = nullptr;

        VK_CHECK( vkCreatePipelineLayout( device, &createInfo, nullptr, &layout ) );
    }

    void GraphicsPipeline::SetVertexDescription( VertexDescriptionType type ) {
        vertexDescription.Reset();
        VkVertexInputBindingDescription mainBinding = {};
        mainBinding.binding = 0;
        mainBinding.stride = VertexDescriptionTypeStide( type );
        mainBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        vertexDescription.bindings.push_back( mainBinding );

        if ( type == VertexDescriptionType::POS ) {
            VkVertexInputAttributeDescription positionAttribute = {};
            positionAttribute.binding = 0;
            positionAttribute.location = 0;
            positionAttribute.format = VK_FORMAT_R32G32_SFLOAT;
            positionAttribute.offset = 0;
            
            vertexDescription.attributes.push_back( positionAttribute );
        } else if ( type == VertexDescriptionType::POS_UV ) {
            VkVertexInputAttributeDescription positionAttribute = {};
            positionAttribute.binding = 0;
            positionAttribute.location = 0;
            positionAttribute.format = VK_FORMAT_R32G32_SFLOAT;
            positionAttribute.offset = 0;

            VkVertexInputAttributeDescription uvAttribute = {};
            uvAttribute.binding = 0;
            uvAttribute.location = 2;
            uvAttribute.format = VK_FORMAT_R32G32_SFLOAT;
            uvAttribute.offset = sizeof( glm::vec2 );

            vertexDescription.attributes.push_back( positionAttribute );
            vertexDescription.attributes.push_back( uvAttribute );
        }
    }

    VkShaderModule GraphicsPipeline::CreateShaderModule( const char * code, i64 size) {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = size;
        createInfo.pCode = reinterpret_cast<const uint32_t *>( code );

        VkShaderModule shaderModule;
        if( vkCreateShaderModule( device, &createInfo, nullptr, &shaderModule ) != VK_SUCCESS ) {
            INVALID_CODE_PATH;
        }

        return shaderModule;
    }

    VkPipeline GraphicsPipeline::Build() {
        i64 vertShaderCodeSize = 0;
        const char * vertShaderCode = core->ResourceReadEntireBinaryFileIntoTransientMemory( vertCodeFile, &vertShaderCodeSize );

        i64 pixlShaderCodeSize = 0;
        const char * pixlShaderCode = core->ResourceReadEntireBinaryFileIntoTransientMemory( pixlCodeFile, &pixlShaderCodeSize );

        VkShaderModule vertShaderModule = CreateShaderModule( vertShaderCode, vertShaderCodeSize );
        VkShaderModule fragShaderModule = CreateShaderModule( pixlShaderCode, pixlShaderCodeSize );

        VkPipelineShaderStageCreateInfo vertShaderStageInfo ={};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo ={};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

        VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {};
        vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputCreateInfo.pNext = nullptr;
        vertexInputCreateInfo.pVertexAttributeDescriptions = vertexDescription.attributes.data();
        vertexInputCreateInfo.vertexAttributeDescriptionCount = (u32)vertexDescription.attributes.size();
        vertexInputCreateInfo.pVertexBindingDescriptions = vertexDescription.bindings.data();
        vertexInputCreateInfo.vertexBindingDescriptionCount = (u32)vertexDescription.bindings.size();

        VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo = {};
        inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssemblyCreateInfo.pNext = nullptr;
        inputAssemblyCreateInfo.topology = topology;
        inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

        VkPipelineRasterizationStateCreateInfo rasterizationCreateInfo = {};
        rasterizationCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizationCreateInfo.pNext = nullptr;
        rasterizationCreateInfo.depthClampEnable = VK_FALSE;
        rasterizationCreateInfo.rasterizerDiscardEnable = VK_FALSE;
        rasterizationCreateInfo.polygonMode = polygonMode;
        rasterizationCreateInfo.lineWidth = 1.0f;
        rasterizationCreateInfo.cullMode = cullModeFlags;
        rasterizationCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;// VK_FRONT_FACE_COUNTER_CLOCKWISE VK_FRONT_FACE_CLOCKWISE
        rasterizationCreateInfo.depthBiasEnable = VK_FALSE;
        rasterizationCreateInfo.depthBiasConstantFactor = 0.0f;
        rasterizationCreateInfo.depthBiasClamp = 0.0f;
        rasterizationCreateInfo.depthBiasSlopeFactor = 0.0f;

        VkPipelineMultisampleStateCreateInfo multisampleCreateInfo = {};
        multisampleCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampleCreateInfo.pNext = nullptr;
        multisampleCreateInfo.sampleShadingEnable = VK_FALSE;
        multisampleCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampleCreateInfo.minSampleShading = 1.0f;
        multisampleCreateInfo.pSampleMask = nullptr;
        multisampleCreateInfo.alphaToCoverageEnable = VK_FALSE;
        multisampleCreateInfo.alphaToOneEnable = VK_FALSE;

        VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo = {};
        depthStencilCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencilCreateInfo.pNext = nullptr;
        depthStencilCreateInfo.depthTestEnable = depthTest ? VK_TRUE : VK_FALSE;
        depthStencilCreateInfo.depthWriteEnable = depthWrite ? VK_TRUE : VK_FALSE;
        depthStencilCreateInfo.depthCompareOp = depthCompareOp;
        depthStencilCreateInfo.depthBoundsTestEnable = VK_FALSE;
        depthStencilCreateInfo.minDepthBounds = 0.0f;
        depthStencilCreateInfo.maxDepthBounds = 1.0f;
        depthStencilCreateInfo.stencilTestEnable = VK_FALSE;

        VkPipelineViewportStateCreateInfo viewportState = {};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.pNext = nullptr;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        VkPipelineColorBlendStateCreateInfo colorBlendingCreateInfo = {};
        colorBlendingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlendingCreateInfo.pNext = nullptr;
        colorBlendingCreateInfo.logicOpEnable = VK_FALSE;
        colorBlendingCreateInfo.logicOp = VK_LOGIC_OP_COPY;
        colorBlendingCreateInfo.attachmentCount = colorAttachmentCount;
        std::vector<VkPipelineColorBlendAttachmentState> blendAttachmentStates;
        for (int i = 0; i < colorAttachmentCount; i++) {
            VkPipelineColorBlendAttachmentState attachmentState = {};
            attachmentState.colorWriteMask = 0xF;
            attachmentState.blendEnable = colorBlendEnable;
            attachmentState.colorBlendOp = VK_BLEND_OP_ADD;
            attachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            attachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            attachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
            attachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            attachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            blendAttachmentStates.push_back(attachmentState);
        }

        colorBlendingCreateInfo.pAttachments = blendAttachmentStates.data();

        std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
        VkPipelineDynamicStateCreateInfo dynamicState = {};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        VkGraphicsPipelineCreateInfo pipelineInfo = {};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.pNext = nullptr;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputCreateInfo;
        pipelineInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizationCreateInfo;
        pipelineInfo.pMultisampleState = &multisampleCreateInfo;
        pipelineInfo.pColorBlendState = &colorBlendingCreateInfo;
        pipelineInfo.pDepthStencilState = &depthStencilCreateInfo;
        pipelineInfo.layout = layout;
        pipelineInfo.renderPass = renderPass; // @NOTE: DYNA
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineInfo.pDynamicState = &dynamicState;

        // New create info to define color, depth and stencil attachments at pipeline create time
        std::vector<VkFormat> formats;
        for (int i = 0; i < colorAttachmentCount; i++) {
            formats.push_back(VK_FORMAT_R8G8B8A8_UNORM);
        }

        // @NOTE: DYNA
        //VkPipelineRenderingCreateInfoKHR pipelineRenderingCreateInfo = {};
        //pipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
        //pipelineRenderingCreateInfo.colorAttachmentCount = colorAttachmentCount;
        //pipelineRenderingCreateInfo.pColorAttachmentFormats = formats.data();
        //pipelineRenderingCreateInfo.depthAttachmentFormat = depthFormat;
        //pipelineRenderingCreateInfo.stencilAttachmentFormat = stencilFormat;
        //pipelineInfo.pNext = &pipelineRenderingCreateInfo;

        VkPipeline newPipeline = {};
        if ( vkCreateGraphicsPipelines( device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &newPipeline ) != VK_SUCCESS ) {
            INVALID_CODE_PATH;
        }
        else {
            if (handle != VK_NULL_HANDLE ) {
                vkDestroyPipeline( device, handle, nullptr );
            }
            handle = newPipeline;
        }

        VkDebugUtilsObjectNameInfoEXT nameInfo = {};
        nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        nameInfo.objectType = VK_OBJECT_TYPE_PIPELINE;
        nameInfo.objectHandle = (uint64_t)handle;
        nameInfo.pObjectName = vertCodeFile;
        PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>( vkGetDeviceProcAddr( device, "vkSetDebugUtilsObjectNameEXT" ) );
        vkSetDebugUtilsObjectNameEXT( device, &nameInfo );

        vkDestroyShaderModule(device, fragShaderModule, nullptr);
        vkDestroyShaderModule(device, vertShaderModule, nullptr);

        return handle;
    }

}

#endif
