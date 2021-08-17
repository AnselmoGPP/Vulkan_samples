// Definir principales objetos/elementes
// Abstraer creación de modelos

#include <iostream>
#include <stdexcept>
#include <cstdlib>				// EXIT_SUCCESS, EXIT_FAILURE

#include <cstdint>				// UINT32_MAX
#include <algorithm>			// std::min / std::max
#include <fstream>
#include <chrono>
#include <unordered_map>		// For storing unique vertices from the model

#include "triangle.hpp"

myApp::myApp() : m(e) { }

myApp::~myApp() { }

void myApp::run()
{
	createSyncObjects();
	mainLoop();
	cleanup();
}

// (25)
/// Create semaphores and fences for synchronizing the events occuring in each frame (drawFrame()).
void myApp::createSyncObjects()
{
	imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
	imagesInFlight.resize(e.swapChainImages.size(), VK_NULL_HANDLE);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		if (vkCreateSemaphore(e.device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(e.device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(e.device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create synchronization objects for a frame!");
		}
	}
}

void myApp::mainLoop()
{
	while (!glfwWindowShouldClose(e.window))
	{
		glfwPollEvents();	// Check for events
		drawFrame();
	}

	vkDeviceWaitIdle(e.device);	// Waits for the logical device to finish operations. Needed for cleaning up once drawing and presentation operations (drawFrame) have finished. Use vkQueueWaitIdle for waiting for operations in a specific command queue to be finished.
}

/**
*	Acquire image from swap chain, execute command buffer with that image as attachment in the framebuffer, and return the image to the swap chain for presentation.
*	This method performs 3 operations asynchronously (the function call returns before the operations are finished, with undefined order of execution):
*	<ul>
*		<li>Acquire an image from the swap chain</li>
*		<li>Execute the command buffer with that image as attachment in the framebuffer</li>
*		<li>Return the image to the swap chain for presentation</li>
*	</ul>
*	Each of the operations depends on the previous one finishing, so we need to synchronize the swap chain events.
*	Two ways: semaphores (mainly designed to synchronize within or accross command queues. Best fit here) and fences (mainly designed to synchronize your application itself with rendering operation).
*	Synchronization examples: https://github.com/KhronosGroup/Vulkan-Docs/wiki/Synchronization-Examples#swapchain-image-acquire-and-present
*/
void myApp::drawFrame()
{
	vkWaitForFences(e.device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);		// Wait for the frame to be finished. If VK_TRUE, we wait for all fences.

	// Acquire an image from the swap chain
	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(e.device, e.swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);		// Swap chain is an extension feature. imageIndex: index to the VkImage in our swapChainImages.

	if (result == VK_ERROR_OUT_OF_DATE_KHR)							// VK_ERROR_OUT_OF_DATE_KHR: The swap chain became incompatible with the surface and can no longer be used for rendering. Usually happens after window resize.
	{
		recreateSwapChain(); return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)	// VK_SUBOPTIMAL_KHR: The swap chain can still be used to successfully present to the surface, but the surface properties are no longer matched exactly.
		throw std::runtime_error("Failed to acquire swap chain image!");

	// Update uniforms
	updateUniformBuffer(imageIndex);

	// Check if this image is being used. If used, wait. Then, mark it as used by this frame.
	if (imagesInFlight[imageIndex] != VK_NULL_HANDLE)									// Check if a previous frame is using this image (i.e. there is its fence to wait on)
		vkWaitForFences(e.device, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
	imagesInFlight[imageIndex] = inFlightFences[currentFrame];							// Mark the image as now being in use by this frame

	// Submit the command buffer
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };			// Which semaphores to wait on before execution begins.
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };	// In which stages of the pipeline to wait the semaphore. VK_PIPELINE_STAGE_ ... TOP_OF_PIPE_BIT (ensures that the render passes don't begin until the image is available), COLOR_ATTACHMENT_OUTPUT_BIT (makes the render pass wait for this stage).
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m.commandBuffers[imageIndex];							// Command buffers to submit for execution (here, the one that binds the swap chain image we just acquired as color attachment).
	VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };			// Which semaphores to signal once the command buffers have finished execution.
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	vkResetFences(e.device, 1, &inFlightFences[currentFrame]);		// Reset the fence to the unsignaled state.

	if (vkQueueSubmit(e.graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)	// Submit the command buffer to the graphics queue. An array of VkSubmitInfo structs can be taken as argument when workload is much larger, for efficiency.
		throw std::runtime_error("Failed to submit draw command buffer!");

	// Note:
	// Subpass dependencies: Subpasses in a render pass automatically take care of image layout transitions. These transitions are controlled by subpass dependencies (specify memory and execution dependencies between subpasses).
	// There are two built-in dependencies that take care of the transition at the start and at the end of the render pass, but the former does not occur at the right time. It assumes that the transition occurs at the start of the pipeline, but we haven't acquired the image yet at that point. Two ways to deal with this problem:
	//		- waitStages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT (ensures that the render passes don't begin until the image is available).
	//		- waitStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT (makes the render pass wait for this stage).

	// Presentation (submit the result back to the swap chain to have it eventually show up on the screen).
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { e.swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;			// Optional

	result = vkQueuePresentKHR(e.presentQueue, &presentInfo);		// Submit request to present an image to the swap chain. Our triangle may look a bit different because the shader interpolates in linear color space and then converts to sRGB color space.

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || e.framebufferResized) {
		e.framebufferResized = false;
		recreateSwapChain();
	}
	else if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to present swap chain image!");

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;	// By using the modulo operator (%), the frame index loops around after every MAX_FRAMES_IN_FLIGHT enqueued frames.

	// vkQueueWaitIdle(presentQueue);							// Make the whole graphics pipeline to be used only one frame at a time (instead of using this, we use multiple semaphores for processing frames concurrently).
}

/// The window surface may change, making the swap chain no longer compatible with it (example: window resizing). Here, we catch these events and recreate the swap chain.
void myApp::recreateSwapChain()
{
	int width = 0, height = 0;
	glfwGetFramebufferSize(e.window, &width, &height);	// <<< Necessary?
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(e.window, &width, &height);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(e.device);			// We shouldn't touch resources that may be in use.

	cleanupSwapChain();

	e.recreateSwapChain();
	m.recreateSwapChain();

	imagesInFlight.resize(e.swapChainImages.size(), VK_NULL_HANDLE);
}

/// Update Uniform buffer. It will generate a new transformation every frame to make the geometry spin around.
void myApp::updateUniformBuffer(uint32_t currentImage)
{
	// Compute time difference
	static auto startTime = std::chrono::high_resolution_clock::now();
	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	// Compute transformation matrix
	UniformBufferObject ubo{};
	ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));						// Params: Existing transformation, rotation angle, rotation axis.
	ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));			// Params: Eye position, center position, up axis.
	ubo.proj = glm::perspective(glm::radians(45.0f), e.swapChainExtent.width / (float)e.swapChainExtent.height, 0.1f, 10.0f);	// Params: FOV, aspect ratio, near and far view planes.
	ubo.proj[1][1] *= -1;																									// GLM returns the Y clip coordinate inverted.

	// Copy the data in the uniform buffer object to the current uniform buffer
	// <<< Using a UBO this way is not the most efficient way to pass frequently changing values to the shader. Push constants are more efficient for passing a small buffer of data to shaders.
	void* data;
	vkMapMemory(e.device, m.uniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
	memcpy(data, &ubo, sizeof(ubo));
	vkUnmapMemory(e.device, m.uniformBuffersMemory[currentImage]);
}

void myApp::cleanup()
{
	cleanupSwapChain();

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {							// Semaphores (render & image available) & fences (in flight)
		vkDestroySemaphore(e.device, renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(e.device, imageAvailableSemaphores[i], nullptr);
		vkDestroyFence(e.device, inFlightFences[i], nullptr);
	}

	m.cleanup();
	e.cleanup();
}

void myApp::cleanupSwapChain()
{
	m.cleanupSwapChain();
	e.cleanupSwapChain();
}

