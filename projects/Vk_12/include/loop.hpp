#ifndef TRIANGLE_HPP
#define TRIANGLE_HPP

#include <vector>
#include <optional>				// std::optional<uint32_t> (Wrapper that contains no value until you assign something to it. Contains member has_value())

#include "environment.hpp"
#include "models.hpp"
#include "input.hpp"

class loopManager
{
	VulkanEnvironment		e;		// Environment
	std::list<modelData>	m;		// Models
	Input					input;	// Input

	// Private parameters:

	const int MAX_FRAMES_IN_FLIGHT		= 2;										// How many frames should be processed concurrently.
	VkClearColorValue backgroundColor	= { 50/255.f, 150/255.f, 255/255.f, 1.0f };

	// Main methods:

	void createCommandBuffers();			///< Allocates command buffers and record drawing commands in them.
	void createSyncObjects();
	void mainLoop();
		void drawFrame();
			void recreateSwapChain();
			void updateUniformBuffer(uint32_t currentImage);

	void cleanup();
	void cleanupSwapChain();

	// Member variables:

	std::vector<VkCommandBuffer> commandBuffers;			///<<< List. Opaque handle to command buffer object. One for each swap chain framebuffer.

	std::vector<VkSemaphore>	imageAvailableSemaphores;	///< Signals that an image has been acquired and is ready for rendering. Each frame has a semaphore for concurrent processing. Allows multiple frames to be in-flight while still bounding the amount of work that piles up. One for each possible frame in flight.
	std::vector<VkSemaphore>	renderFinishedSemaphores;	///< Signals that rendering has finished and presentation can happen. Each frame has a semaphore for concurrent processing. Allows multiple frames to be in-flight while still bounding the amount of work that piles up. One for each possible frame in flight.
	std::vector<VkFence>		inFlightFences;				///< Similar to semaphores, but fences actually wait in our own code. Used to perform CPU-GPU synchronization. One for each possible frame in flight.
	std::vector<VkFence>		imagesInFlight;				///< Maps frames in flight by their fences. Tracks for each swap chain image if a frame in flight is currently using it. One for each swap chain image.

	size_t						currentFrame = 0;			///< Frame to process next.

public:
	loopManager(std::vector<modelConfig> & modelConfigs);
	~loopManager();

	void run();
};

#endif