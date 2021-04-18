/*
 * Methods:
 *
	-run
		-initWindow
		-initVulkan
			1) createInstance
				-checkValidationLayerSupport
				-populateDebugMessengerCreateInfo > debugCallback
				-getRequiredExtensions
				-checkExtensionSupport
			2) setupDebugMessenger
				-populateDebugMessengerCreateInfo > debugCallback
				-CreateDebugUtilsMessengerEXT
			3) createSurface
			4) pickPhysicalDevice
				-isDeviceSuitable
					-findQueueFamilies > isComplete
					-checkDeviceExtensionSupport
					-querySwapChainSupport
					-isComplete
			5) createLogicalDevice
				-findQueueFamilies
			6) createSwapChain
				-querySwapChainSupport
				-chooseSwapSurfaceFormat
				-chooseSwapPresentMode
				-chooseSwapExtent
				-findQueueFamilies
			7) createImageViews
			8) createRenderPass
			9) createGraphicsPipeline
				-readFile
				-createShaderModule
			10)createFramebuffers
			11)createCommandPool
			12)createCommandBuffers
			13)createSyncObjects
		-mainLoop
			-drawFrame
		-cleanup
			-cleanupSwapChain
			-DestroyDebugUtilsMessengerEXT

	Shaders:
		1. Vertex shader: Processes each incoming vertex (world position, color, normal, texture coordinates). Result: clip coordinates (4D vector) + attributes.
		2. Result is passed to fragment shader. Rasterizer: Interpolates over the fragments to produce a smooth gradient.
		3. Framebuffer coordinates: Window/pixel coordinates that map to [0, +X] by [0, +Y].
		4. Normalized device coordinates: Homogeneous coordinates that map the framebuffer to [-1, 1] by [-1, 1] coordinate system. Got from dividing clip coordinates by its last component.
 
	Framebuffers:
	During render pass creation, the attachments are bound by wrapping them into a VkFramebuffer object. 
	It references all of the VkImageView objects that represent the attachments.
	
	In our case, we have a single attachment: the color attachment. However, the image that we have to use for the attachment depends on which image the 
	swap chain returns when we retrieve one for presentation, which means that we have to create a framebuffer for all of the images in the swap chain 
	and use the one that corresponds to the retrieved image at drawing time.
 */
#include <vector>
#include <optional>				// std::optional<uint32_t> (Wrapper that contains no value until you assign something to it. Contains member has_value())


//#include <vulkan/vulkan.h>	// From LunarG SDK. Used for off-screen rendering
#define GLFW_INCLUDE_VULKAN		// Makes GLFW load the Vulkan header with it
#include "GLFW/glfw3.h"


/// Given a VkDebugUtilsMessengerCreateInfoEXT object, creates the extension object (VkDebugUtilsMessengerEXT) if it's available.
VkResult CreateDebugUtilsMessengerEXT(
		VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pDebugMessenger);

void DestroyDebugUtilsMessengerEXT(
		VkInstance instance,
		VkDebugUtilsMessengerEXT debugMessenger,
		const VkAllocationCallbacks* pAllocator);



/** @class HelloTriangleApp
    @brief Everything for drawing a triangle with Vulkan

    Creates a window > Initiates Vulkan > Runs the render-loop > Cleans up everything when destroyed
*/
class HelloTriangleApp
{
public:
	void run();

private:
	void initWindow();
	void initVulkan();
		void createInstance();			///< Describe application, select extensions and validation layers, create Vulkan instance (stores application state).
		void setupDebugMessenger();		///< Specify the details about the messenger and its callback, and create the debug messenger.
		void createSurface();			///< Create a window surface (interface for interacting with the window system)
		void pickPhysicalDevice();		///< Look for and select a graphics card in the system that supports the features we need.
		void createLogicalDevice();		///< Set up a logical device (describes the features we want to use) to interface with the physical device.
		void createSwapChain();			///< Set up and create the swap chain.
		void createImageViews();		///< Creates a basic image view for every image in the swap chain so that we can use them as color targets later on.
		void createRenderPass();		///< Tells Vulkan the framebuffer attachments that will be used while rendering.
		void createGraphicsPipeline();	///< Create the graphics pipeline.
		void createFramebuffers();		///< Create framebuffers.
		void createCommandPool();		///< Commands in Vulkan (drawing, memory transfers, etc.) are not executed directly using function calls, you have to record all of the operations you want to perform in command buffer objects. After setting up the drawing commands, just tell Vulkan to execute them in the main loop.
		void createCommandBuffers();	///< Allocates command buffers and record drawing commands in them.
		void createSyncObjects();		///< Create semaphores and fences for synchronizing the events occuring in each frame (drawFrame()).
	void mainLoop();
		void drawFrame();				///< Acquire image from swap chain, execute command buffer with that image as attachment in the framebuffer, and return the image to the swap chain for presentation.
	void cleanup();
		void cleanupSwapChain();

	GLFWwindow*					 window;							///< Opaque window object.
	VkInstance					 instance;							///< Opaque handle to an instance object. There is no global state in Vulkan and all per-application state is stored here.
	VkDebugUtilsMessengerEXT	 debugMessenger;					///< Opaque handle to a debug messenger object (the debug callback is part of it).
	VkPhysicalDevice			 physicalDevice = VK_NULL_HANDLE;	///< Opaque handle to a physical device object.
	VkDevice					 device;							///< Opaque handle to a device object.
	VkSurfaceKHR				 surface;							///< Opaque handle to a surface object (abstract type of surface to present rendered images to)
	VkQueue						 graphicsQueue;						///< Opaque handle to a queue object (computer graphics).
	VkQueue						 presentQueue;						///< Opaque handle to a queue object (presentation to window surface).
	VkSwapchainKHR				 swapChain;							///< Swap chain object.
	std::vector<VkImage>		 swapChainImages;					///< List. Opaque handle to an image object.
	VkFormat					 swapChainImageFormat;				///< Swap chain format.
	VkExtent2D					 swapChainExtent;					///< Swap chain extent.
	std::vector<VkImageView>	 swapChainImageViews;				///< List. Opaque handle to an image view object. It allows to use VkImage in the render pipeline. It's a view into an image; it describes how to access the image and which part of the image to access.
	VkRenderPass				 renderPass;						///< Opaque handle to a render pass object.
	VkPipelineLayout			 pipelineLayout;					///< Pipeline layout. Allows to use uniform values in shaders (globals similar to dynamic state variables that can be changed at drawing at drawing time to alter the behavior of your shaders without having to recreate them).
	VkPipeline					 graphicsPipeline;					///< Opaque handle to a pipeline object.
	std::vector<VkFramebuffer>	 swapChainFramebuffers;				///< List. Opaque handle to a framebuffer object.
	VkCommandPool				 commandPool;						///< Opaque handle to a command pool object. It manages the memory that is used to store the buffers, and command buffers are allocated from them. 
	std::vector<VkCommandBuffer> commandBuffers;					///< List. Opaque handle to command buffer object
	std::vector<VkSemaphore>	 imageAvailableSemaphores;			///< Signals that an image has been acquired and is ready for rendering. Each frame has a semaphore for concurrent processing. Allows multiple frames to be in-flight while still bounding the amount of work that piles up.
	std::vector<VkSemaphore>	 renderFinishedSemaphores;			///< Signals that rendering has finished and presentation can happen.Each frame has a semaphore for concurrent processing. Allows multiple frames to be in-flight while still bounding the amount of work that piles up.
	std::vector<VkFence>		 inFlightFences;					///< Similar to semaphores, but fences actually wait in our own code. Used to perform CPU-GPU synchronization.
	std::vector<VkFence>		 imagesInFlight;					///< Maps frames in flight by their fences. Tracks for each swap chain image if a frame in flight is currently using it.
	size_t currentFrame = 0;										///< Frame to process next.

	bool framebufferResized = false;								///< Many drivers/platforms trigger VK_ERROR_OUT_OF_DATE_KHR after window resize, but it's not guaranteed. This variable handles resizes explicitly.

	bool checkValidationLayerSupport(const std::vector<const char*> &requiredLayers, bool printData);
	bool checkExtensionSupport(const char* const* requiredExtensions, uint32_t reqExtCount, bool printData);

	std::vector<const char*> getRequiredExtensions();
	/// Callback for handling ourselves the validation layer's debug messages and decide which kind of messages to see.
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData);

	/// Specify the details about the messenger and its callback.
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	// Evaluate a device and check if it is suitable for the operations we want to perform.
	int isDeviceSuitable(VkPhysicalDevice device, const int mode);

	/// Structure for storing vector indices of the queue families we want.
	struct QueueFamilyIndices;

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

	bool checkDeviceExtensionSupport(VkPhysicalDevice device);

	/// Structure containing details about the swap chain that must be checked.
	struct SwapChainSupportDetails;

	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
	/// Chooses the surface format (color depth) for the swap chain.
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	/// Chooses the presentation mode (conditions for "swapping" images to the screen) for the swap chain.
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	/// Chooses the swap extent (resolution of images in swap chain) for the swap chain.
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	/// Read all of the bytes from the specified file and return them in a byte array managed by a std::vector.
	static std::vector<char> readFile(const std::string& filename);
	/// Take a buffer with the bytecode as parameter and create a VkShaderModule from it.
	VkShaderModule createShaderModule(const std::vector<char>& code);

	/// The window surface may change, making the swap chain no longer compatible with it (example: window resizing). Here, we catch these events and recreate the swap chain.
	void recreateSwapChain();

	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
};
