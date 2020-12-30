#include "main.h"

void appvk::createUniformBuffers() {
    VkDeviceSize bufferSize = sizeof(ubo);
    uniformBuffers.resize(swapImages.size());
    uniformMemories.resize(swapImages.size());

    for (size_t i = 0; i < swapImages.size(); i++) {
        createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        uniformBuffers[i], uniformMemories[i]);
    }
}

void appvk::createDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding bindings[2] = {};

    bindings[0].binding = 0;
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bindings[0].descriptorCount = 1;
    bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    bindings[1].binding = 1;
    // images and samplers can actually be bound separately!
    bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[1].descriptorCount = 1;
    bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createInfo.bindingCount = 2;
    createInfo.pBindings = bindings;

    if (vkCreateDescriptorSetLayout(dev, &createInfo, nullptr, &dSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("cannot create descriptor set!");
    }
}

void appvk::createDescriptorPool() {
    VkDescriptorPoolSize poolSizes[2];
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = swapImages.size();

    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = swapImages.size();

    VkDescriptorPoolCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    createInfo.maxSets = swapImages.size() * 2;
    createInfo.poolSizeCount = 2;
    createInfo.pPoolSizes = poolSizes;

    if (vkCreateDescriptorPool(dev, &createInfo, nullptr, &dPool) != VK_SUCCESS) {
        throw std::runtime_error("cannot create descriptor pool!");
    }
}

void appvk::allocDescriptorSets(std::vector<VkDescriptorSet>& dSet) {
    std::vector<VkDescriptorSetLayout> dLayout(swapImages.size(), dSetLayout);

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = dPool;
    allocInfo.descriptorSetCount = swapImages.size();
    allocInfo.pSetLayouts = dLayout.data();
    
    dSet.resize(swapImages.size());
    if (vkAllocateDescriptorSets(dev, &allocInfo, dSet.data()) != VK_SUCCESS) {
        throw std::runtime_error("cannot create descriptor set!");
    }

    for (size_t i = 0; i < swapImages.size(); i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(ubo);

        VkWriteDescriptorSet set{};
        set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        set.dstSet = dSet[i];
        set.dstBinding = 0;
        set.dstArrayElement = 0;
        set.descriptorCount = 1;
        set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        set.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(dev, 1, &set, 0, nullptr);
    }
}

void appvk::allocDescriptorSetTexture(std::vector<VkDescriptorSet>& dSet, VkSampler samp, VkImageView view) {
    for (size_t i = 0; i < swapImages.size(); i++) {
        VkDescriptorImageInfo imageInfo{};
        imageInfo.sampler = samp;
        imageInfo.imageView = view;
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkWriteDescriptorSet set{};
        set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        set.dstSet = dSet[i];
        set.dstBinding = 1;
        set.dstArrayElement = 0;
        set.descriptorCount = 1;
        set.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        set.pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(dev, 1, &set, 0, nullptr);
    }
}