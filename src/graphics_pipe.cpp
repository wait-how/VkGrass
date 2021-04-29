#define STB_IMAGE_IMPLEMENTATION
// take out decoders we don't use to save space
#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#define STBI_NO_FAILURE_STRINGS
#include "stb_image.h"

#include "main.hpp"

// stores framebuffer config
void appvk::createRenderPass() {
    VkAttachmentDescription attachments[3];

    // multisample
    attachments[0].flags = 0;
    attachments[0].format = swapFormat; // format from swapchain image
    attachments[0].samples = msaaSamples;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // layout of image before render pass - don't care since we'll be clearing it anyways
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // layout of image at end of render pass

    const std::vector<VkFormat> formatList = {
        VK_FORMAT_D24_UNORM_S8_UINT,
        VK_FORMAT_X8_D24_UNORM_PACK32, // no stencil
    };

    // check to see if we can use a 24-bit depth component
    depthFormat = findImageFormat(formatList, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

    // depth
    attachments[1].flags = 0;
    attachments[1].format = depthFormat;
    attachments[1].samples = msaaSamples; // depth buffer never gets presented, but we want a ms depth buffer to use with our ms color buffer
    attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // depth has to be cleared to something before we use it
    attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    // initialLayout needs to be set before we start rendering, otherwise
    // clearing and a layout transition from undefined -> depth stencil optimal happen at the same time
    attachments[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // resolve
    attachments[2].flags = 0;
    attachments[2].format = swapFormat;
    attachments[2].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[2].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[2].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[2].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[2].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef;
    colorAttachmentRef.attachment = 0; // index in pAttachments
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // layout to transition to at the start of the subpass

    VkAttachmentReference depthAttachmentRef;
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference resolveAttachmentRef;
    resolveAttachmentRef.attachment = 2;
    resolveAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subs[3] = {};
    subs[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subs[0].colorAttachmentCount = 1; // color attachments are FS outputs, can also specify input / depth attachments, etc.
    subs[0].pColorAttachments = &colorAttachmentRef;
    subs[0].pResolveAttachments = &resolveAttachmentRef;
    subs[0].pDepthStencilAttachment = &depthAttachmentRef;

    subs[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subs[1].colorAttachmentCount = 1;
    subs[1].pColorAttachments = &colorAttachmentRef;
    subs[1].pResolveAttachments = &resolveAttachmentRef;
    subs[1].pDepthStencilAttachment = &depthAttachmentRef;

    subs[2].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subs[2].colorAttachmentCount = 1;
    subs[2].pColorAttachments = &colorAttachmentRef;
    subs[2].pResolveAttachments = &resolveAttachmentRef;
    subs[2].pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency deps[3] = {};
    // there's a WAW dependency between writing images due to where imageAvailSems waits
    // solution here is to delay writing to the framebuffer until the image we need is acquired (and the transition has taken place)
    
    // write terrain
    deps[0].srcSubpass = VK_SUBPASS_EXTERNAL; // implicit subpass at start of render pass
    deps[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // stage we're waiting on
    deps[0].srcAccessMask = 0; // what we're using that input for

    deps[0].dstSubpass = 0; // index into pSubpasses
    deps[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // stage we write to
    deps[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; // what we're using that output for

    // write grass
    deps[1].srcSubpass = 0;
    deps[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    deps[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    deps[1].dstSubpass = 1;
    deps[1].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    deps[1].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    // write skybox
    deps[2].srcSubpass = 1;
    deps[2].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    deps[2].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    deps[2].dstSubpass = 2;
    deps[2].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    deps[2].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    createInfo.attachmentCount = 3;
    createInfo.pAttachments = attachments;
    createInfo.subpassCount = 3;
    createInfo.pSubpasses = subs;
    createInfo.dependencyCount = 3;
    createInfo.pDependencies = deps;

    if (vkCreateRenderPass(dev, &createInfo, nullptr, &renderPass) != VK_SUCCESS) {
        throw std::runtime_error("cannot create render pass!");
    }
}

void appvk::createGraphicsPipeline() {
    std::vector<char> terrainvspv = readFile(".spv/terrain.vert.spv");
    std::vector<char> terrainfspv = readFile(".spv/terrain.frag.spv");

    VkShaderModule terrainv = createShaderModule(terrainvspv);
    VkShaderModule terrainf = createShaderModule(terrainfspv);

    VkPipelineShaderStageCreateInfo shaders[2] = {};
    
    shaders[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaders[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaders[0].module = terrainv;
    shaders[0].pName = "main";
    
    shaders[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaders[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaders[1].module = terrainf;
    shaders[1].pName = "main";
    
    VkVertexInputBindingDescription bindDesc;
    bindDesc.binding = 0;
    bindDesc.stride = sizeof(vformat::vertex);
    bindDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription attrDesc[3];
    for (size_t i = 0; i < 3; i++) {
        attrDesc[i].location = i;
        attrDesc[i].binding = 0;
        attrDesc[i].offset = 16 * i; // all offsets are rounded up to 16 bytes due to alignas
    }

    attrDesc[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attrDesc[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attrDesc[2].format = VK_FORMAT_R32G32_SFLOAT;
    
    VkPipelineVertexInputStateCreateInfo vinCreateInfo{};
    vinCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vinCreateInfo.vertexBindingDescriptionCount = 1;
    vinCreateInfo.pVertexBindingDescriptions = &bindDesc;
    vinCreateInfo.vertexAttributeDescriptionCount = 3;
    vinCreateInfo.pVertexAttributeDescriptions = attrDesc;

    VkPipelineInputAssemblyStateCreateInfo inAsmCreateInfo{};
    inAsmCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inAsmCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inAsmCreateInfo.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = swapExtent.height;
    viewport.width = swapExtent.width;
    // Vulkan says -Y is up, not down, flip so we're compatible with OpenGL code and obj models
    viewport.height = -1.0f * swapExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapExtent;

    VkPipelineViewportStateCreateInfo viewCreateInfo{};
    viewCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewCreateInfo.viewportCount = 1;
    viewCreateInfo.pViewports = &viewport;
    viewCreateInfo.scissorCount = 1;
    viewCreateInfo.pScissors = &scissor;
    
    VkPipelineRasterizationStateCreateInfo rasterCreateInfo{};
    rasterCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterCreateInfo.depthClampEnable = VK_FALSE; // clamps depth to range instead of discarding it
    rasterCreateInfo.rasterizerDiscardEnable = VK_FALSE; // disables rasterization if true
    rasterCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; // flip cull order due to inverting y in rasterizer
    rasterCreateInfo.depthBiasEnable = VK_FALSE;
    rasterCreateInfo.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo msCreateInfo{};
    msCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    msCreateInfo.sampleShadingEnable = VK_FALSE;
    msCreateInfo.rasterizationSamples = msaaSamples;

    VkPipelineDepthStencilStateCreateInfo dCreateInfo{};
    dCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    dCreateInfo.depthTestEnable = VK_TRUE;
    dCreateInfo.depthWriteEnable = VK_TRUE;
    dCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    dCreateInfo.depthBoundsTestEnable = VK_FALSE;
    dCreateInfo.stencilTestEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorAttachment{}; // blending information per fb
    colorAttachment.blendEnable = VK_FALSE;
    colorAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | 
                                        VK_COLOR_COMPONENT_G_BIT | 
                                        VK_COLOR_COMPONENT_B_BIT |
                                        VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo colorCreateInfo{};
    colorCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorCreateInfo.logicOpEnable = VK_FALSE;
    colorCreateInfo.attachmentCount = 1;
    colorCreateInfo.pAttachments = &colorAttachment;
    
    VkPipelineDynamicStateCreateInfo dynCreateInfo{};
    dynCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynCreateInfo.dynamicStateCount = 0;

    VkPipelineLayoutCreateInfo pipeLayoutCreateInfo{}; // for descriptor sets
    pipeLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeLayoutCreateInfo.setLayoutCount = 1;
    pipeLayoutCreateInfo.pSetLayouts = &dSetLayout;

    if (vkCreatePipelineLayout(dev, &pipeLayoutCreateInfo, nullptr, &terrainPipeLayout) != VK_SUCCESS) {
        throw std::runtime_error("cannot create pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo pipeCreateInfo{};
    pipeCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    
    if (shader_debug) {
        pipeCreateInfo.flags = VK_PIPELINE_CREATE_CAPTURE_STATISTICS_BIT_KHR;
    }
    
    pipeCreateInfo.flags = VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT;
    pipeCreateInfo.stageCount = 2;
    pipeCreateInfo.pStages = shaders;
    pipeCreateInfo.pVertexInputState = &vinCreateInfo;
    pipeCreateInfo.pInputAssemblyState = &inAsmCreateInfo;
    pipeCreateInfo.pViewportState = &viewCreateInfo;
    pipeCreateInfo.pRasterizationState = &rasterCreateInfo;
    pipeCreateInfo.pMultisampleState = &msCreateInfo;
    pipeCreateInfo.pDepthStencilState = &dCreateInfo;
    pipeCreateInfo.pColorBlendState = &colorCreateInfo;
    pipeCreateInfo.layout = terrainPipeLayout; // handle, not a struct.
    pipeCreateInfo.renderPass = renderPass;
    pipeCreateInfo.subpass = 0;
    
    if (vkCreateGraphicsPipelines(dev, VK_NULL_HANDLE, 1, &pipeCreateInfo, nullptr, &terrainPipe) != VK_SUCCESS) {
        throw std::runtime_error("cannot create graphics pipeline!");
    }

    if (shader_debug) {
        printShaderStats();
        printed = true; // prevent stats from being printed again if we recreate the pipeline
    }
    
    vkDestroyShaderModule(dev, terrainv, nullptr); // we can destroy shader modules once the graphics pipeline is created.
    vkDestroyShaderModule(dev, terrainf, nullptr);

    VkGraphicsPipelineCreateInfo grassPipeCreateInfo = pipeCreateInfo;

    // creating grass pipeline from same struct since almost everything is the same
    std::vector<char> grassvspv = readFile(".spv/grass.vert.spv");
    std::vector<char> grassfspv = readFile(".spv/grass.frag.spv");

    VkShaderModule grassv = createShaderModule(grassvspv);
    VkShaderModule grassf = createShaderModule(grassfspv);

    VkPipelineShaderStageCreateInfo grassShaders[2] = {};

    grassShaders[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    grassShaders[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    grassShaders[0].module = grassv;
    grassShaders[0].pName = "main";

    grassShaders[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    grassShaders[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    grassShaders[1].module = grassf;
    grassShaders[1].pName = "main";

    VkVertexInputBindingDescription bindDesc2[2];
    bindDesc2[0] = bindDesc;

    bindDesc2[1].binding = 1;
    bindDesc2[1].stride = sizeof(glm::mat4);
    bindDesc2[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

    VkVertexInputAttributeDescription attrDesc2[7];
    for (size_t i = 0; i < 3; i++) {
        attrDesc2[i] = attrDesc[i];
    }

    // mat4 requires four slots in vertex shader
    for (size_t i = 3; i < 7; i++) {
        attrDesc2[i].binding = 1;
        attrDesc2[i].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attrDesc2[i].location = i;
        attrDesc2[i].offset = sizeof(glm::vec4) * (i - 3); // NOTE: vertex inputs _have_ to be distinct even if they come from different binding points.
    }

    VkPipelineVertexInputStateCreateInfo vinCreateInfo2{};
    vinCreateInfo2.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vinCreateInfo2.vertexBindingDescriptionCount = 2;
    vinCreateInfo2.pVertexBindingDescriptions = bindDesc2;
    vinCreateInfo2.vertexAttributeDescriptionCount = 7;
    vinCreateInfo2.pVertexAttributeDescriptions = attrDesc2;

    VkPipelineColorBlendAttachmentState colorAttachment2{};
    colorAttachment2.blendEnable = VK_TRUE;
    colorAttachment2.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorAttachment2.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorAttachment2.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorAttachment2.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorAttachment2.colorBlendOp = VK_BLEND_OP_ADD;
    colorAttachment2.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | 
                                        VK_COLOR_COMPONENT_G_BIT | 
                                        VK_COLOR_COMPONENT_B_BIT |
                                        VK_COLOR_COMPONENT_A_BIT;
    
    VkPipelineColorBlendStateCreateInfo colorCreateInfo2{};
    colorCreateInfo2.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorCreateInfo2.logicOpEnable = VK_FALSE;
    colorCreateInfo2.attachmentCount = 1;
    colorCreateInfo2.pAttachments = &colorAttachment2;

    VkPipelineRasterizationStateCreateInfo rasterCreateInfo2{};
    rasterCreateInfo2.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterCreateInfo2.depthClampEnable = VK_FALSE;
    rasterCreateInfo2.rasterizerDiscardEnable = VK_FALSE;
    rasterCreateInfo2.polygonMode = VK_POLYGON_MODE_FILL;
    rasterCreateInfo2.cullMode = VK_CULL_MODE_NONE;
    rasterCreateInfo2.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterCreateInfo2.depthBiasEnable = VK_FALSE;
    rasterCreateInfo2.lineWidth = 1.0f;

    VkPipelineDepthStencilStateCreateInfo dCreateInfo2{};
    dCreateInfo2.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    dCreateInfo2.depthTestEnable = VK_TRUE;
    dCreateInfo2.depthWriteEnable = VK_TRUE;
    dCreateInfo2.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    dCreateInfo2.depthBoundsTestEnable = VK_FALSE;
    dCreateInfo2.stencilTestEnable = VK_FALSE;

    grassPipeCreateInfo.flags = VK_PIPELINE_CREATE_DERIVATIVE_BIT;
    grassPipeCreateInfo.basePipelineHandle = terrainPipe;
    grassPipeCreateInfo.basePipelineIndex = -1;
    grassPipeCreateInfo.pStages = grassShaders;
    grassPipeCreateInfo.pVertexInputState = &vinCreateInfo2;
    grassPipeCreateInfo.pRasterizationState = &rasterCreateInfo2;
    grassPipeCreateInfo.pColorBlendState = &colorCreateInfo2;
    grassPipeCreateInfo.pDepthStencilState = &dCreateInfo2;
    grassPipeCreateInfo.layout = terrainPipeLayout;
    // render pass is the same
    grassPipeCreateInfo.subpass = 1;
    
    if (vkCreateGraphicsPipelines(dev, VK_NULL_HANDLE, 1, &grassPipeCreateInfo, nullptr, &grassPipe) != VK_SUCCESS) {
        throw std::runtime_error("cannot create graphics pipeline!");
    }

    vkDestroyShaderModule(dev, grassv, nullptr);
    vkDestroyShaderModule(dev, grassf, nullptr);

    // creating grass pipeline from same struct since almost everything is the same
    std::vector<char> skyvspv = readFile(".spv/skybox.vert.spv");
    std::vector<char> skyfspv = readFile(".spv/skybox.frag.spv");

    VkShaderModule skyv = createShaderModule(skyvspv);
    VkShaderModule skyf = createShaderModule(skyfspv);

    VkPipelineShaderStageCreateInfo skyShaders[2] = {};

    skyShaders[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    skyShaders[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    skyShaders[0].module = skyv;
    skyShaders[0].pName = "main";

    skyShaders[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    skyShaders[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    skyShaders[1].module = skyf;
    skyShaders[1].pName = "main";

    VkGraphicsPipelineCreateInfo skyPipeCreateInfo = pipeCreateInfo;
    skyPipeCreateInfo.pStages = skyShaders;

    VkVertexInputBindingDescription skyBindDesc;
    skyBindDesc.binding = 0;
    skyBindDesc.stride = sizeof(vformat::vertex);
    skyBindDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription skyAttrDesc;
    skyAttrDesc.location = 0;
    skyAttrDesc.binding = 0;
    skyAttrDesc.format = VK_FORMAT_R32G32B32_SFLOAT;
    skyAttrDesc.offset = 0;

    VkPipelineVertexInputStateCreateInfo skyVertCreateInfo{};
    skyVertCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    skyVertCreateInfo.vertexBindingDescriptionCount = 1;
    skyVertCreateInfo.pVertexBindingDescriptions = &skyBindDesc;
    skyVertCreateInfo.vertexAttributeDescriptionCount = 1;
    skyVertCreateInfo.pVertexAttributeDescriptions = &skyAttrDesc;

    VkPipelineRasterizationStateCreateInfo skyRasterCreateInfo{};
    skyRasterCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    skyRasterCreateInfo.depthClampEnable = VK_FALSE;
    skyRasterCreateInfo.rasterizerDiscardEnable = VK_FALSE;
    skyRasterCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    skyRasterCreateInfo.cullMode = VK_CULL_MODE_NONE;
    skyRasterCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    skyRasterCreateInfo.depthBiasEnable = VK_FALSE;
    skyRasterCreateInfo.lineWidth = 1.0f;

    VkPipelineLayoutCreateInfo skyLayoutCreateInfo{};
    skyLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    skyLayoutCreateInfo.setLayoutCount = 1;
    skyLayoutCreateInfo.pSetLayouts = &skySetLayout;

    if (vkCreatePipelineLayout(dev, &skyLayoutCreateInfo, nullptr, &skyPipeLayout) != VK_SUCCESS) {
        throw std::runtime_error("cannot create skybox layout!");
    }

    skyPipeCreateInfo.pVertexInputState = &skyVertCreateInfo;
    skyPipeCreateInfo.pRasterizationState = &skyRasterCreateInfo;
    skyPipeCreateInfo.layout = skyPipeLayout;

    skyPipeCreateInfo.flags = VK_PIPELINE_CREATE_DERIVATIVE_BIT;
    skyPipeCreateInfo.basePipelineHandle = terrainPipe;
    skyPipeCreateInfo.basePipelineIndex = -1;
    skyPipeCreateInfo.subpass = 2;

    if (vkCreateGraphicsPipelines(dev, VK_NULL_HANDLE, 1, &skyPipeCreateInfo, nullptr, &skyPipe) != VK_SUCCESS) {
        throw std::runtime_error("cannot create skybox pipeline!");
    }

    vkDestroyShaderModule(dev, skyv, nullptr);
    vkDestroyShaderModule(dev, skyf, nullptr);
}

void appvk::createFramebuffers() {
    swapFramebuffers.resize(swapImageViews.size());

    for (size_t i = 0; i < swapFramebuffers.size(); i++) {
        
        // having a single depth buffer with >1 swap image only works if graphics and pres queues are the same.
        // this is due to submissions in a single queue having to respect both submission order and semaphores
        VkImageView attachments[] = {
            msImageView, // multisampled render image
            depthView,
            swapImageViews[i] // swapchain present image
        };

        VkFramebufferCreateInfo fCreateInfo{};
        fCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fCreateInfo.renderPass = renderPass;
        fCreateInfo.attachmentCount = 3;
        fCreateInfo.pAttachments = attachments; // framebuffer attaches to the image view of a swapchain
        fCreateInfo.width = swapExtent.width;
        fCreateInfo.height = swapExtent.height;
        fCreateInfo.layers = 1;

        if (vkCreateFramebuffer(dev, &fCreateInfo, nullptr, &swapFramebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("cannot create framebuffer!");
        }
    }
}

std::pair<VkBuffer, VkDeviceMemory> appvk::createVertexBuffer(const std::vector<uint8_t>& verts) {
    VkBuffer vertexBuf;
    VkDeviceMemory vertexMem;

    VkBuffer stagingBuf;
    VkDeviceMemory stagingMem;

    VkDeviceSize bufferSize = verts.size();
    createBuffer(verts.size(), 
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
        stagingBuf, stagingMem);
    
    createBuffer(verts.size(), 
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
        vertexBuf, vertexMem);

    void *data;
    vkMapMemory(dev, stagingMem, 0, bufferSize, 0, &data);
    memcpy(data, verts.data(), bufferSize);
    vkUnmapMemory(dev, stagingMem);

    copyBuffer(stagingBuf, vertexBuf, bufferSize);

    vkFreeMemory(dev, stagingMem, nullptr);
    vkDestroyBuffer(dev, stagingBuf, nullptr);

    return std::pair(vertexBuf, vertexMem);
}

// wrapper for raw createVertexBuffer that takes a vloader mesh
std::pair<VkBuffer, VkDeviceMemory> appvk::createVertexBuffer(std::vector<vformat::vertex>& v) {
    auto bytePtr = reinterpret_cast<uint8_t*>(v.data());
	std::vector<uint8_t> byteData(bytePtr, bytePtr + v.size() * sizeof(vformat::vertex));

    return createVertexBuffer(byteData);
}


std::pair<VkBuffer, VkDeviceMemory> appvk::createIndexBuffer(const std::vector<uint32_t>& indices) {
    VkBuffer indexBuf;
    VkDeviceMemory indexMem;
    
    VkBuffer stagingBuf;
    VkDeviceMemory stagingMem;
    
    VkDeviceSize bufferSize = indices.size() * sizeof(uint32_t);

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    stagingBuf, stagingMem);

    createBuffer(bufferSize,
    VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    indexBuf, indexMem);

    void *data;
    vkMapMemory(dev, stagingMem, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), bufferSize);
    vkUnmapMemory(dev, stagingMem);

    copyBuffer(stagingBuf, indexBuf, bufferSize);

    vkFreeMemory(dev, stagingMem, nullptr);
    vkDestroyBuffer(dev, stagingBuf, nullptr);

    return std::pair(indexBuf, indexMem);
}

std::tuple<VkImage, VkDeviceMemory, unsigned int> appvk::createTextureImage(std::string_view path, bool flip) {
    // if the image format considers the origin to be the top left (png), then flip.
    stbi_set_flip_vertically_on_load_thread(flip);

    int width, height, chans;
    unsigned char *data = stbi_load(path.data(), &width, &height, &chans, STBI_rgb_alpha);
    if (!data) {
        throw std::runtime_error("cannot load texture!");
    }

    unsigned int mipLevels = floor(log2(std::max(width, height))) + 1;
    
    VkDeviceSize imageSize = width * height * 4;

    VkBuffer sbuf = VK_NULL_HANDLE;
    VkDeviceMemory smem = VK_NULL_HANDLE;

    createBuffer(imageSize, 
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        sbuf, smem);

    void *map_data;
    vkMapMemory(dev, smem, 0, imageSize, 0, &map_data);
    memcpy(map_data, data, imageSize);
    vkUnmapMemory(dev, smem);

    stbi_image_free(data);

    VkImage texImage;
    VkDeviceMemory texMem;

    // used as a src when blitting to make mipmaps
    createImage(width, height, VK_FORMAT_R8G8B8A8_SRGB, mipLevels, VK_SAMPLE_COUNT_1_BIT,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        texImage, texMem);
    
    transitionImageLayout(texImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels, 1);
    copyBufferToImage(sbuf, texImage, uint32_t(width), uint32_t(height), 1);

    vkFreeMemory(dev, smem, nullptr);
    vkDestroyBuffer(dev, sbuf, nullptr);

    generateMipmaps(texImage, VK_FORMAT_R8G8B8A8_SRGB, width, height, mipLevels, 1);

    return std::tuple(texImage, texMem, mipLevels);
}

std::tuple<VkImage, VkDeviceMemory> appvk::createCubemapImage(std::array<std::string_view, 6> paths, bool flip) {
    // if the image format considers the origin to be the top left (png), then flip.
    stbi_set_flip_vertically_on_load_thread(flip);

    unsigned char* imgs[6];

    int width, height, chans;
    for (size_t i = 0; i < 6; i++) {
        imgs[i] = stbi_load(paths[i].data(), &width, &height, &chans, STBI_rgb_alpha);
        if (!imgs[i]) {
            throw std::runtime_error("cannot load texture!");
        }
    }
    
    VkDeviceSize imageSize = width * height * 4;
    VkDeviceSize cubeSize = imageSize * 6;

    VkBuffer sbuf = VK_NULL_HANDLE;
    VkDeviceMemory smem = VK_NULL_HANDLE;

    createBuffer(cubeSize, 
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        sbuf, smem);

    for (size_t i = 0; i < 6; i++) {
        void *map_data;
        vkMapMemory(dev, smem, i * imageSize, imageSize, 0, &map_data);
        memcpy(map_data, imgs[i], imageSize);
        vkUnmapMemory(dev, smem);
        stbi_image_free(imgs[i]);
    }

    VkImage texImage;
    VkDeviceMemory texMem;

    // used as a src when blitting to make mipmaps
    createCubeImage(width, height, VK_FORMAT_R8G8B8A8_SRGB, VK_SAMPLE_COUNT_1_BIT,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        texImage, texMem);
    
    transitionImageLayout(texImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, 6);
    copyBufferToImage(sbuf, texImage, uint32_t(width), uint32_t(height), 6);

    vkFreeMemory(dev, smem, nullptr);
    vkDestroyBuffer(dev, sbuf, nullptr);

    // no mip levels generated, but this puts all cube images in the shader read optimal layout
    generateMipmaps(texImage, VK_FORMAT_R8G8B8A8_SRGB, width, height, 1, 6);

    return std::tuple(texImage, texMem);
}

void appvk::createDepthImage() {
    createImage(swapExtent.width, swapExtent.height,
        depthFormat, 1, msaaSamples,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        depthImage, depthMemory);
    
    transitionImageLayout(depthImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1, 1);
    
    depthView = createImageView(depthImage, depthFormat, 1, VK_IMAGE_ASPECT_DEPTH_BIT);
}

void appvk::createMultisampleImage() {
    createImage(swapExtent.width, swapExtent.height, swapFormat, 1, msaaSamples, 
    VK_IMAGE_TILING_OPTIMAL,
    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    msImage, msMemory);

    msImageView = createImageView(msImage, swapFormat, 1, VK_IMAGE_ASPECT_COLOR_BIT);
}