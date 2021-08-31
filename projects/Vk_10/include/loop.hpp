#ifndef TRIANGLE_HPP
#define TRIANGLE_HPP

#include <vector>
#include <optional>				// std::optional<uint32_t> (Wrapper that contains no value until you assign something to it. Contains member has_value())

#include "environment.hpp"
#include "models.hpp"

class loopManager
{
	VulkanEnvironment	 e;							// Environment
	std::list<modelData> m;							// Models
	//std::vector<std::vector<VkCommandBuffer*>> commandBuffers;	// Command buffers
	//modelData			 m;	// Model

	// Private parameters:

	const int MAX_FRAMES_IN_FLIGHT = 2;	// How many frames should be processed concurrently.

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

	std::vector<VkCommandBuffer> commandBuffers;				///<<< List. Opaque handle to command buffer object

	std::vector<VkSemaphore>	imageAvailableSemaphores;		///< Signals that an image has been acquired and is ready for rendering. Each frame has a semaphore for concurrent processing. Allows multiple frames to be in-flight while still bounding the amount of work that piles up.
	std::vector<VkSemaphore>	renderFinishedSemaphores;		///< Signals that rendering has finished and presentation can happen. Each frame has a semaphore for concurrent processing. Allows multiple frames to be in-flight while still bounding the amount of work that piles up.
	std::vector<VkFence>		inFlightFences;					///< Similar to semaphores, but fences actually wait in our own code. Used to perform CPU-GPU synchronization.
	std::vector<VkFence>		imagesInFlight;					///< Maps frames in flight by their fences. Tracks for each swap chain image if a frame in flight is currently using it.

	size_t						currentFrame = 0;				///< Frame to process next.

public:
	loopManager(std::vector<modelConfig> &models);
	~loopManager();

	void run();
};

#endif