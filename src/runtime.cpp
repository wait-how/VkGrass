#include <chrono>
#include <cmath>

#include "vloader.hpp"

#include "main.hpp"

void appvk::createSyncs() {
    imageAvailSems.resize(framesInFlight, VK_NULL_HANDLE);
    renderDoneSems.resize(framesInFlight, VK_NULL_HANDLE);
    inFlightFences.resize(framesInFlight, VK_NULL_HANDLE);
    imagesInFlight = std::vector<VkFence>(swapImages.size(), VK_NULL_HANDLE); // this needs to be re-created on a window resize
    
    VkSemaphoreCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    
    VkFenceCreateInfo fCreateInfo{};
    fCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (unsigned int i = 0; i < framesInFlight; i++) {
        VkResult r1 = vkCreateSemaphore(dev, &createInfo, nullptr, &imageAvailSems[i]);
        VkResult r2 = vkCreateSemaphore(dev, &createInfo, nullptr, &renderDoneSems[i]);
        VkResult r3 = vkCreateFence(dev, &fCreateInfo, nullptr, &inFlightFences[i]);
        
        if (r1 != VK_SUCCESS || r2 != VK_SUCCESS || r3 != VK_SUCCESS) {
            throw std::runtime_error("cannot create sync objects!");
        }
    }
}

void appvk::updateUniformBuffer(uint32_t imageIndex) {
    //using namespace std::chrono;
    //static auto start = high_resolution_clock::now();
    //auto current = high_resolution_clock::now();
    //float time = duration<float, seconds::period>(current - start).count();

    mvp u;
    u.model = glm::mat4(1.0f);
    // camera flips Y automatically
    u.view = glm::lookAt(c.pos, c.pos + c.front, glm::vec3(0.0f, 1.0f, 0.0f));
    u.proj = glm::perspective(glm::radians(25.0f), swapExtent.width / float(swapExtent.height), 0.1f, 100.0f);

    void* data;
    vkMapMemory(dev, mvpMemories[imageIndex], 0, sizeof(mvp), 0, &data);
    memcpy(data, &u, sizeof(mvp));
    vkUnmapMemory(dev, mvpMemories[imageIndex]);
}

// one grass section per triangle, centered on a vertex
void appvk::initGrass(const std::vector<vformat::vertex>& verts, const std::vector<uint32_t>& indices) {
    const size_t vsize = verts.size();
    const size_t csize = indices.size() / 3;

    grassMatBuf.resize(vsize + csize);
    
    size_t mat_i = 0;

    // write mats for each vertex
    for (size_t i = 0; i < vsize; i++) {
        const glm::vec3 p = verts[i].pos;
        grassMatBuf[mat_i++] = glm::translate(glm::mat4(1.0f), p);
    }

    const size_t isize = indices.size();
    
    // write mats for lerped positions using indices
    for (size_t i = 0; i < isize; i += 3) {
        const glm::vec3 p0 = verts[indices[i]].pos;
        const glm::vec3 p1 = verts[indices[i + 1]].pos;
        const glm::vec3 p2 = verts[indices[i + 2]].pos;
        
        const glm::vec3 pl = glm::mix(glm::mix(p0, p1, 0.5f), p2, 0.5f);
        //grassMatBuf[mat_i++] = glm::translate(glm::mat4(1.0f), pl);
    }
}