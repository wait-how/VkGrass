#include "vloader.hpp"

#include "main.hpp"

void appvk::recreateSwapChain() {
	int width, height;
	glfwGetFramebufferSize(w, &width, &height);
	while (width == 0 || height == 0) { // wait until window isn't hidden anymore
		glfwGetFramebufferSize(w, &width, &height);
		glfwWaitEvents(); // put this thread to sleep until events exist
	}

	vkDeviceWaitIdle(dev);

	cleanupSwapChain();

	createSwapChain();
	createSwapViews();
	createRenderPass();
	createGraphicsPipeline();
	createDepthImage();
	createMultisampleImage();
	createFramebuffers();
	createUniformBuffers();
	createDescriptorPools();
	allocDescriptorSets(dPool, terrainSet, dSetLayout);
	allocDescriptorSets(dPool, grassSet, dSetLayout);
	allocDescriptorSets(skyPool, skySet, skySetLayout);
	allocDescriptorSetUniform(terrainSet);
	allocDescriptorSetUniform(grassSet);
	allocDescriptorSetUniform(skySet);
	allocDescriptorSetTexture(terrainSet, terrainSamp, terrainView);
	allocDescriptorSetTexture(grassSet, grassSamp, grassView);
	allocDescriptorSetTexture(skySet, cubeSamp, cubeView);
	allocRenderCmdBuffers();
	createSyncs();
}

appvk::appvk() : c(0.0f, 1.618f, -9.764f) {
	createWindow();

	// disable and center cursor
	// glfwSetInputMode(w, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	createInstance();
	if (debug) {
		setupDebugMessenger();
	}
	createSurface();
	pickPhysicalDevice(nvidia);
	createLogicalDevice();

	createSwapChain();
	createSwapViews();

	createRenderPass();
	createDescriptorSetLayouts();
	createGraphicsPipeline();

	createCommandPool();
	createDepthImage();
	createMultisampleImage();
	createFramebuffers();

	uint8_t feats = ter::terrain::features::normal | ter::terrain::features::uv;
	unsigned int nw = 256, nh = 256;
	t.regen(nw, nh, 50.0f, 50.0f, feats);
	cout << "created terrain with " << nw << "x" << nh << " samples, " << nw * nh << " vertices generated\n";

	std::string_view grassPath = "models/vertical-quad.obj";
	vload::vloader g(grassPath, false, false);
	cout << "loaded model " << grassPath << "\n";

	std::string_view skyPath = "models/cube.obj";
	vload::vloader s(skyPath, false, false);
	cout << "loaded model " << skyPath << "\n";

	std::tie(terrainVertBuf, terrainVertMem) = createVertexBuffer(t.verts);
	std::tie(terrainIndBuf, terrainIndMem) = createIndexBuffer(t.indices);

	std::tie(grassVertBuf, grassVertMem) = createVertexBuffer(g.meshList[0].verts);

	std::tie(skyVertBuf, skyVertMem) = createVertexBuffer(s.meshList[0].verts);

	initGrass(t.verts, t.indices);
	auto bytePtr = reinterpret_cast<uint8_t*>(grassMatBuf.data());
	std::vector<uint8_t> byteVec(bytePtr, bytePtr + grassMatBuf.size() * sizeof(glm::mat4));

	std::tie(grassVertInstBuf, grassVertInstMem) = createVertexBuffer(byteVec);

	std::string_view terrainFloor = "textures/floor-diffuse-1k.jpg";
	std::tie(terrainImage, terrainMem, terrainMipLevels) = createTextureImage(terrainFloor, false);
	cout << "loaded texture " << terrainFloor << "\n";
	terrainView = createImageView(terrainImage, VK_FORMAT_R8G8B8A8_SRGB, terrainMipLevels, VK_IMAGE_ASPECT_COLOR_BIT);
	terrainSamp = createSampler(terrainMipLevels);

	std::string_view grassTex = "textures/grass-billboard.png";
	std::tie(grassImage, grassMem, grassMipLevels) = createTextureImage(grassTex, true);
	cout << "loaded texture " << grassTex << "\n";
	grassView = createImageView(grassImage, VK_FORMAT_R8G8B8A8_SRGB, grassMipLevels, VK_IMAGE_ASPECT_COLOR_BIT);
	grassSamp = createSampler(grassMipLevels);

	std::array<std::string_view, 6> skyTex;
	skyTex[0] = "textures/right.jpg"; // +x (right)
	skyTex[1] = "textures/left.jpg"; // -x (left)
	skyTex[2] = "textures/top.jpg"; // +y (top)
	skyTex[3] = "textures/bottom.jpg"; // -y (bottom)
	skyTex[4] = "textures/front.jpg"; // +z (front)
	skyTex[5] = "textures/back.jpg"; // -z (back)

	std::tie(cubeImage, cubeMem) = createCubemapImage(skyTex, false);
	cout << "loaded cubemap texture\n";

	cubeView = createCubeImageView(cubeImage, VK_FORMAT_R8G8B8A8_SRGB);
	cubeSamp = createSampler(1);

	createUniformBuffers();
	createDescriptorPools();

	allocDescriptorSets(dPool, terrainSet, dSetLayout);
	allocDescriptorSets(dPool, grassSet, dSetLayout);
	allocDescriptorSets(skyPool, skySet, skySetLayout);

	allocDescriptorSetUniform(terrainSet);
	allocDescriptorSetUniform(grassSet);
	allocDescriptorSetUniform(skySet);

	allocDescriptorSetTexture(terrainSet, terrainSamp, terrainView);
	allocDescriptorSetTexture(grassSet, grassSamp, grassView);
	allocDescriptorSetTexture(skySet, cubeSamp, cubeView);

	terrainIndices = t.indices.size();
	grassVertices = g.meshList[0].verts.size();
	grassInstances = grassMatBuf.size();
	grassIndices = g.meshList[0].indices.size();
	skyVertices = s.meshList[0].verts.size();
	allocRenderCmdBuffers();

	createSyncs();
}

void appvk::drawFrame() {

	// NOTE: acquiring an image, writing to it, and presenting it are all async operations.
	// The relevant vulkan calls return before the operation completes.

	// wait for a command buffer to finish writing to the current image
	vkWaitForFences(dev, 1, &inFlightFences[currFrame], VK_FALSE, UINT64_MAX);

	uint32_t nextFrame;
	VkResult r = vkAcquireNextImageKHR(dev, swap, UINT64_MAX, imageAvailSems[currFrame], VK_NULL_HANDLE, &nextFrame);
	// NOTE: currFrame may not always be equal to nextFrame (there's no guarantee that nextFrame increases linearly)

	if (r == VK_ERROR_OUT_OF_DATE_KHR || resizeOccurred) {
		recreateSwapChain(); // have to recreate the swapchain here
		resizeOccurred = false;
		return;
	} else if (r != VK_SUCCESS && r != VK_SUBOPTIMAL_KHR) { // we can still technically run with a suboptimal swapchain
		throw std::runtime_error("cannot acquire swapchain image!");
	}

	// wait for the previous frame to finish using the swapchain image at nextFrame
	if (imagesInFlight[nextFrame] != VK_NULL_HANDLE) {
		vkWaitForFences(dev, 1, &imagesInFlight[nextFrame], VK_FALSE, UINT64_MAX);
	}

	imagesInFlight[nextFrame] = inFlightFences[currFrame]; // this frame is using the fence at currFrame

	updateUniformBuffer(nextFrame);

	VkSubmitInfo si{};
	si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore renderBeginSems[] = { imageAvailSems[currFrame] };
	VkSemaphore renderEndSems[] = { renderDoneSems[currFrame] };

	si.waitSemaphoreCount = 1;
	si.pWaitSemaphores = renderBeginSems;

	// imageAvailSem waits at this point in the pipeline
	// NOTE: stages not covered by a semaphore may execute before the semaphore is signaled.
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	si.pWaitDstStageMask = waitStages;

	si.commandBufferCount = 1;
	si.pCommandBuffers = &commandBuffers[nextFrame];

	si.signalSemaphoreCount = 1;
	si.pSignalSemaphores = renderEndSems;

	vkResetFences(dev, 1, &inFlightFences[currFrame]); // has to be unsignaled for vkQueueSubmit
	vkQueueSubmit(gQueue, 1, &si, inFlightFences[currFrame]);

	VkPresentInfoKHR pInfo{};
	pInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	pInfo.waitSemaphoreCount = 1;
	pInfo.pWaitSemaphores = renderEndSems;
	pInfo.swapchainCount = 1;
	pInfo.pSwapchains = &swap;
	pInfo.pImageIndices = &nextFrame;

	r = vkQueuePresentKHR(gQueue, &pInfo);
	if (r == VK_ERROR_OUT_OF_DATE_KHR || resizeOccurred) {
		recreateSwapChain();
		resizeOccurred = false;
		return;
	} else if (r != VK_SUCCESS && r != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("cannot submit to queue!");
	}

	currFrame = (currFrame + 1) % framesInFlight;
}

void appvk::run() {
	while (!glfwWindowShouldClose(w)) {
		glfwPollEvents();
		if (glfwGetKey(w, GLFW_KEY_I) == GLFW_PRESS) {
			std::cout << "\tcamera position: (" << c.pos.x << ", " << c.pos.y << ", " << c.pos.z << ")\n";
		}
		c.update(w);
		drawFrame();

		if (glfwGetKey(w, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			glfwSetWindowShouldClose(w, GLFW_TRUE);
		}
	}

	vkDeviceWaitIdle(dev);
}

int main(int argc, char **argv) {
	(void)argc;
	(void)argv;

	appvk app;
	try {
		app.run();
	} catch (const std::exception& e) {
		cerr << e.what() << "\n";
		return 1;
	}
	return 0;
}
