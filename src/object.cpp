//
// Created by JeremyGuo on 2022/2/25.
//

#include "glfwApp.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include <Buffer.h>
#include <Texture.h>
#include <Vertex.h>
#include <SubMesh.h>
#include <Mesh.h>
#include <Instance.h>

#include <unordered_map>
#include <Shader.h>
#include <Camera.h>

//const std::string MODEL_PATH = "../../San_Miguel/san-miguel-low-poly.obj";
const std::string MODEL_PATH = "../models/viking_room.obj";
const std::string TEXTURE_PATH = "../textures/viking_room.png";

struct UniformBufferObject {
    glm::mat4 view;
    glm::mat4 proj;
};

const int MAX_FRAMES_IN_FLIGHT = 2;
const int MAX_MESH = 1;

struct FrameVkInfo {
    VkCommandBuffer commandBuffer;
    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;
    VkFence inFlightFence;
};

class MyApp : public glfw::glfwApp {
public:
    MyApp();

    void initialize() override;
    void cleanup() override;
    ~MyApp() override;

protected:
    void onDraw() override;
    void onUpdate() override;

    void initRenderPass();
    void initDescriptorSetLayout();
    void initGraphicsPipeline();
    void initFramebuffers();
    void initCommandPool();
    void initDepthBuffer();
    void initBuffers();
    void initDescriptorPool();
    void initDescriptorSets();
    void initTexture();
    void initSyncObjects();
    void initCamera();

    void cleanupSwapChain() override;
    void recreateSwapChain() override;

    void onKeyDown(int key, int scancode, int action, int mods) override;
    void onMouseMove(float x, float y) override;
    void onMouseButton(int button, int action, int mods) override;

    void recordCommandBuffer(VkCommandBuffer cb, int currentFrame, int imageIndex, VkDescriptorSet &descriptorSet);
    VkFormat findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    VkFormat findDepthFormat();

    VkPipelineLayout pipelineLayout;
    VkRenderPass renderPass;
    VkPipeline graphicsPipeline;
    std::vector<VkFramebuffer> swapChainFramebuffers;
    VkCommandPool commandPool;

//    glfw::Mesh mesh;
    std::vector<glfw::Instance*> instances;
    std::vector<glfw::Buffer*> uniformBuffers;

    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorSetLayout instDescriptorSetLayout;
    std::vector<VkDescriptorSet> descriptorSets;

    VkDescriptorPool descriptorPool;
    std::vector<FrameVkInfo> frameInfos;
    int currentFrame = 0;

    glfw::Texture texture;
    glfw::Texture depth;

    bool mWPressed = false;
    bool mAPressed = false;
    bool mSPressed = false;
    bool mDPressed = false;
    bool mLShiftPressed = false;
    bool mMouseRPressed = false;
    bool mMouseLPressed = false;

    glm::vec2 cursor = {0.0, 0.0};
    glm::vec2 cursorDelta = {0.0, 0.0};

    glfw::Camera mainCamera;
};

MyApp::MyApp():glfwApp(),
    texture(this), depth(this) {
}

MyApp::~MyApp() {
}

void MyApp::cleanup() {
    {
        for (glfw::Instance* & inst : instances)
            inst->destroy(1);
        instances.resize(0);
        texture.destroy();
    }
    vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    for (auto &frame : frameInfos) {
        vkDestroySemaphore(device, frame.imageAvailableSemaphore, nullptr);
        vkDestroySemaphore(device, frame.renderFinishedSemaphore, nullptr);
        vkDestroyFence(device, frame.inFlightFence, nullptr);
    }
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
    vkDestroyCommandPool(device, commandPool, nullptr);
    depth.destroy();
    for (size_t i = 0; i < swapChainFramebuffers.size(); i++) {
        vkDestroyFramebuffer(device, swapChainFramebuffers[i], nullptr);
    }
    vkDestroyPipeline(device, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyRenderPass(device, renderPass, nullptr);
    glfw::glfwApp::cleanup();
}

int main(int argc, char **argv) {
    MyApp myApp;
    try {
        myApp.initialize();
        myApp.run();
        myApp.cleanup();
    } catch (std::exception& e) {
        printException(e);
    }

    return 0;
}

void MyApp::initialize() {
    glfw::glfwApp::initialize();
    try {
        this->initRenderPass();
        this->initDescriptorSetLayout();
        this->initGraphicsPipeline();
        this->initCommandPool();
        this->initDepthBuffer();
        this->initFramebuffers();
        fprintf(stdout, "Loading Model\n");
        instances.push_back(new glfw::Instance(this, new glfw::Mesh(this)));
        instances[0]->mMesh->loadObject(MODEL_PATH.c_str(), commandPool, graphicsQueue);
        fprintf(stdout, "Model Loaded\n");
        this->initTexture();
        this->initBuffers();
        this->initDescriptorPool();
        this->initDescriptorSets();
        instances[0]->initGPUMemory(descriptorPool, instDescriptorSetLayout, commandPool, graphicsQueue, MAX_FRAMES_IN_FLIGHT);
        this->initSyncObjects();
        this->initCamera();
    } catch(...) {
        std::throw_with_nested(std::runtime_error("failed to init myApp"));
    }
}

void MyApp::recordCommandBuffer(VkCommandBuffer cb, int currentFrame, int imageIndex, VkDescriptorSet& descriptorSet) {
    /**
         * Record Command
         */
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0; // Optional
    beginInfo.pInheritanceInfo = nullptr; // Optional
    if (vkBeginCommandBuffer(cb, &beginInfo) != VK_SUCCESS)
        throw std::runtime_error("failed to begin recording command buffer!");

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapChainExtent;
    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();
    vkCmdBeginRenderPass(cb, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

    for (auto &inst : instances) {
        std::array<VkDescriptorSet, 2> curDescriptorSets = {
                inst->getModelDescriptorSet(currentFrame),
                descriptorSet
        };
        for (auto &submesh: inst->mMesh->submesh) {
            VkBuffer vertexBuffers[] = {submesh->vertex->getBuffer()};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(cb, 0, 1, vertexBuffers, offsets);
            vkCmdBindIndexBuffer(cb, submesh->indice->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
            vkCmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, curDescriptorSets.size(), curDescriptorSets.data(), 0,
                                    nullptr);
            vkCmdDrawIndexed(cb, submesh->numIndices, 1, 0, 0, 0);
        }
    }

    vkCmdEndRenderPass(cb);
    if (vkEndCommandBuffer(cb) != VK_SUCCESS)
        throw std::runtime_error("failed to record command buffer!");
}

void MyApp::onDraw() {
    vkWaitForFences(device, 1, &frameInfos[currentFrame].inFlightFence, VK_TRUE, UINT64_MAX);
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, frameInfos[currentFrame].imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapChain();
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }
    vkResetFences(device, 1, &frameInfos[currentFrame].inFlightFence);

    vkResetCommandBuffer(frameInfos[currentFrame].commandBuffer, 0);
    recordCommandBuffer(frameInfos[currentFrame].commandBuffer, currentFrame, imageIndex, descriptorSets[currentFrame]);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {frameInfos[currentFrame].imageAvailableSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &frameInfos[currentFrame].commandBuffer;
    VkSemaphore signalSemaphores[] = {frameInfos[currentFrame].renderFinishedSemaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;
    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, frameInfos[currentFrame].inFlightFence) != VK_SUCCESS) {
        std::throw_with_nested(std::runtime_error("failed to submit draw command buffer!"));
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    VkSwapchainKHR swapChains[] = {swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr; // Optional
    result = vkQueuePresentKHR(presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
        framebufferResized = false;
        recreateSwapChain();
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to swap image!");
    }
    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

#include <iostream>

void MyApp::onUpdate() {
    UniformBufferObject ubo{};

    if (mMouseRPressed) {
        float moveForward = 0.0f;
        float moveRight = 0.0f;
        if (mWPressed)
            moveForward += 1.0f;
        if (mSPressed)
            moveForward -= 1.0f;
        if (mAPressed)
            moveRight -= 1.0f;
        if (mDPressed)
            moveRight += 1.0f;
        moveForward *= deltaTime * 2.5f;
        moveRight *= deltaTime * 2.5f;
        if (mLShiftPressed) {
            moveForward *= 4;
            moveRight *= 4;
        }
        mainCamera.Move(moveRight, moveForward);

        mainCamera.Rotate(-cursorDelta.x * deltaTime * 400.0f, cursorDelta.y * deltaTime * 400.0f);
    } else if (mMouseLPressed) {
        mainCamera.Rotate(-cursorDelta.x * deltaTime * 400.0f, cursorDelta.y * deltaTime * 400.0f);
    }
    this->cursorDelta = glm::vec2(0.0, 0.0);

//    ubo.view = glm::lookAt(camera_pos, camera_pos + look_dir, glm::vec3(0.0f, 0.0f, 1.0f));
//    ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float) swapChainExtent.height, 0.1f, 10.0f);
//    ubo.proj[1][1] *= -1;
    ubo.view = mainCamera.GetTransform();
    ubo.proj = mainCamera.GetProjection();
    uniformBuffers[currentFrame]->uploadData(&ubo, sizeof(ubo));
}

void MyApp::initGraphicsPipeline() {
    glfw::Shader shader(this);

    shader.loadShaderModule("object.vert.spv", "object.frag.spv");
    const char* fname = "main";
    VkPipelineShaderStageCreateInfo shaderStages[] = {shader.getVertStageInfo(fname), shader.getFragStageInfo(fname)};

    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) swapChainExtent.width;
    viewport.height = (float) swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapChainExtent;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f; // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f; // Optional
    multisampling.pSampleMask = nullptr; // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE; // Optional

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f; // Optional
    colorBlending.blendConstants[1] = 0.0f; // Optional
    colorBlending.blendConstants[2] = 0.0f; // Optional
    colorBlending.blendConstants[3] = 0.0f; // Optional

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    std::array<VkDescriptorSetLayout, 2> setLayouts = {
            instDescriptorSetLayout,
            descriptorSetLayout
    };
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = setLayouts.size(); // Optional
    pipelineLayoutInfo.pSetLayouts = setLayouts.data(); // Optional
    pipelineLayoutInfo.pushConstantRangeCount = 0; // Optionala
    pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }
    std::cout << "Pipeline layout created" << std::endl;

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f; // Optional
    depthStencil.maxDepthBounds = 1.0f; // Optional
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {}; // Optional
    depthStencil.back = {}; // Optional

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;

    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = nullptr; // Optional
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1; // Optional

    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }
    shader.destroy();
}

void MyApp::initRenderPass() {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = findDepthFormat();
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());;
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT; // The previous Pass's test
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT; // Once we did, we will clear it
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}

void MyApp::initFramebuffers() {
    swapChainFramebuffers.resize(swapChainImageViews.size());
    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
        std::array<VkImageView,2> attachments = {
                swapChainImageViews[i],
                depth.getImageView()
        };
        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;
        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

void MyApp::initCommandPool() {
    {
        /**
         * Create Command Pool
         */
        auto queueFamilyIndices = findQueueFamilies(physicalDevice, surface);
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
        if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create command pool!");
        }
    }

    {
        /**
         * Create Command Buffer
         */
        frameInfos.resize(MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        std::vector<VkCommandBuffer> commandBuffers(MAX_FRAMES_IN_FLIGHT);
        if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }
        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i ++)
            frameInfos[i].commandBuffer = commandBuffers[i];
    }
}

void MyApp::initSyncObjects() {
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    for (auto &frame : frameInfos)
        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &frame.imageAvailableSemaphore) != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &frame.renderFinishedSemaphore) != VK_SUCCESS ||
            vkCreateFence(device, &fenceInfo, nullptr, &frame.inFlightFence) != VK_SUCCESS){
            throw std::runtime_error("failed to create semaphores!");
        }
}

void MyApp::cleanupSwapChain() {
    depth.destroy();
    for (size_t i = 0; i < swapChainFramebuffers.size(); i++) {
        vkDestroyFramebuffer(device, swapChainFramebuffers[i], nullptr);
    }
    vkDestroyPipeline(device, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyRenderPass(device, renderPass, nullptr);
    glfw::glfwApp::cleanupSwapChain();
}

void MyApp::recreateSwapChain() {
    glfw::glfwApp::recreateSwapChain();
    this->initRenderPass();
    this->initGraphicsPipeline();
    this->initDepthBuffer();
    this->initFramebuffers();
}

void MyApp::initBuffers() {
    try {
        {
            /**
             * Create Uniform Buffers
             */
            VkDeviceSize bufferSize = sizeof(UniformBufferObject);
            uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
            for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                uniformBuffers[i] = new glfw::Buffer(this);
                uniformBuffers[i]->create(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            }
        }
    } catch (...) {
        std::throw_with_nested(std::runtime_error("Failed to init buffers"));
    }
}

void MyApp::initDescriptorSetLayout() {
    {
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; // This binding is accessable from VERTEX stage
        uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

        VkDescriptorSetLayoutBinding samplerLayoutBinding{}; //diffuse
        samplerLayoutBinding.binding = 1;
        samplerLayoutBinding.descriptorCount = 1;
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.pImmutableSamplers = nullptr;
        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }
    {
        VkDescriptorSetLayoutBinding modelLayoutBinding{};
        modelLayoutBinding.binding = 0;
        modelLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        modelLayoutBinding.descriptorCount = 1;
        modelLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        modelLayoutBinding.pImmutableSamplers = nullptr; // Optional

        std::array<VkDescriptorSetLayoutBinding, 1> bindings = {modelLayoutBinding};
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &instDescriptorSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }
}

void MyApp::initDescriptorPool() {
    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * (1 + MAX_MESH));
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * (1 + MAX_MESH)); // Usage ?

    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void MyApp::initDescriptorSets() {
    try {
        std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        allocInfo.pSetLayouts = layouts.data();

        descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
        if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
            throw std::runtime_error("failed to allocate descriptor sets!");

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) { // Because buffer is in GPU, we need a command to update info
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = uniformBuffers[i]->getBuffer();
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);

            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = texture.getImageView();
            imageInfo.sampler = texture.getSampler();

            std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = descriptorSets[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo;

            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = descriptorSets[i];
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pImageInfo = &imageInfo;

            vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }
    } catch(...) {
        std::throw_with_nested("failed to create descriptor sets");
    }
}

void MyApp::initTexture() {
    try {
        texture.load(TEXTURE_PATH.c_str(), commandPool, graphicsQueue);

        // ??
        VkImageSubresourceRange subresourceRange;
        subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subresourceRange.baseMipLevel = 0;
        subresourceRange.levelCount = 1;
        subresourceRange.baseArrayLayer = 0;
        subresourceRange.layerCount = 1;
        texture.createImageView(VK_IMAGE_VIEW_TYPE_2D, texture.getFormat(), subresourceRange);

        texture.createSampler(VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT);
    } catch (...) {
        std::throw_with_nested(std::runtime_error("failed to create texture"));
    }
}

VkFormat MyApp::findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling,
                                    VkFormatFeatureFlags features) {
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);
        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return format;
        } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }
    throw std::runtime_error("failed to find supported format!");
}

VkFormat MyApp::findDepthFormat() {
    return findSupportedFormat(
            {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

void MyApp::initDepthBuffer() {
    try {
        VkFormat depthFormat = findDepthFormat();
        VkExtent3D extent = {
                swapChainExtent.width,
                swapChainExtent.height,
                1
        };
        depth.create(VK_IMAGE_TYPE_2D, depthFormat, extent, VK_IMAGE_TILING_OPTIMAL,
                     VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        VkImageSubresourceRange subresourceRange;
        subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subresourceRange.baseMipLevel = 0;
        subresourceRange.levelCount = 1;
        subresourceRange.baseArrayLayer = 0;
        subresourceRange.layerCount = 1;
        depth.createImageView(VK_IMAGE_VIEW_TYPE_2D, depth.getFormat(), subresourceRange);
        depth.transition(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, graphicsQueue, commandPool);
    } catch (...) {
        std::throw_with_nested(std::runtime_error("failed to create depth texture"));
    }
}

void MyApp::onKeyDown(int key, int scancode, int action, int mods) {
    bool n_val = action == GLFW_PRESS;
    switch(key) {
        case GLFW_KEY_W:
            mWPressed = n_val;
            break;
        case GLFW_KEY_A:
            mAPressed = n_val;
            break;
        case GLFW_KEY_S:
            mSPressed = n_val;
            break;
        case GLFW_KEY_D:
            mDPressed = n_val;
            break;
        case GLFW_KEY_LEFT_SHIFT:
            mLShiftPressed = n_val;
            break;
    }
}

void MyApp::onMouseMove(float x, float y) {
    glm::vec2 newCursor = {x, y};
    this->cursorDelta = newCursor - this->cursor;
    this->cursor = newCursor;
}

void MyApp::onMouseButton(int button, int action, int mods) {
    bool n_val = action == GLFW_PRESS;
    switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            mMouseLPressed = n_val;
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            mMouseRPressed = n_val;
            break;
    }
}

void MyApp::initCamera() {
    mainCamera.SetPosition(glm::vec3(2.0, 2.0, 2.0));
    glfw::Recti rect;
    rect.bottom = swapChainExtent.height;
    rect.top = 0;
    rect.left = 0;
    rect.right = swapChainExtent.width;
    mainCamera.SetViewport(rect);
    mainCamera.LookAt(glm::vec3(2.0, 2.0, 2.0), glm::vec3(0.0f));
}
