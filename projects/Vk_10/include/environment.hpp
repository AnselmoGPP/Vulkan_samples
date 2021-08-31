#ifndef ENVIRONMENT_HPP
#define ENVIRONMENT_HPP

#include <vector>
#include <optional>					// std::optional<uint32_t> (Wrapper that contains no value until you assign something to it. Contains member has_value())

//#include <vulkan/vulkan.h>		// From LunarG SDK. Used for off-screen rendering
#define GLFW_INCLUDE_VULKAN			// Makes GLFW load the Vulkan header with it
#include "GLFW/glfw3.h"

#define DEGUB						// Standards: NDEBUG, _DEBUG
#ifdef RELEASE
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

/// Structure for storing vector indices of the queue families we want. Note that graphicsFamily and presentFamily could refer to the same queue family, but we included them separately because sometimes they are in different queue families.
struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily;		///< Queue family capable of computer graphics.
	std::optional<uint32_t> presentFamily;		///< Queue family capable of presenting to our window surface.
	bool isComplete();							///< Checks whether all members have value.
};

/// Structure containing details about the swap chain that must be checked. Though a swap chain may be available, it may not be compatible with our window surface, so we need to query for some details and check them. This struct will contain these details.
struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR		capabilities;		// Basic surface capabilities: min/max number of images in swap chain, and min/max width/height of images.
	std::vector<VkSurfaceFormatKHR> formats;			// Surface formats: pixel format, color space.
	std::vector<VkPresentModeKHR>	presentModes;		// Available presentation modes
};

/// Stores the (global) state of a Vulkan application.
class VulkanEnvironment
{
	// Private parameters:

	bool printInfo = true;

	const uint32_t WIDTH  = 800;
	const uint32_t HEIGHT = 600;

	const std::vector<const char*> requiredValidationLayers = {	"VK_LAYER_KHRONOS_validation" };
	const std::vector<const char*> requiredDeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };	// Swap chain: Queue of images that are waiting to be presented to the screen. Our application will acquire such an image to draw to it, and then return it to the queue. Its general purpose is to synchronize the presentation of images with the refresh rate of the screen.

	// Main methods:

	void initWindow();
	void createInstance();					///< Describe application, select extensions and validation layers, create Vulkan instance (stores application state).
	void setupDebugMessenger();				///< Specify the details about the messenger and its callback, and create the debug messenger.
	void createSurface();					///< Create a window surface (interface for interacting with the window system)
	void pickPhysicalDevice();				///< Look for and select a graphics card in the system that supports the features we need.
	void createLogicalDevice();				///< Set up a logical device (describes the features we want to use) to interface with the physical device.
	void createSwapChain();					///< Set up and create the swap chain.
	void createImageViews();				///< Creates a basic image view for every image in the swap chain so that we can use them as color targets later on.
	void createRenderPass();				///< Tells Vulkan the framebuffer attachments that will be used while rendering (color, depth, multisampled images). A render-pass denotes more explicitly how your rendering happens.

	void createCommandPool();				///< Commands in Vulkan (drawing, memory transfers, etc.) are not executed directly using function calls, you have to record all of the operations you want to perform in command buffer objects. After setting up the drawing commands, just tell Vulkan to execute them in the main loop.
	void createColorResources();			///< Create resources needed for MSAA (MultiSampling AntiAliasing). Create a multisampled color buffer.
	void createDepthResources();			///< Create depth buffer.
	void createFramebuffers();				///< Create the swap chain framebuffers.

	// Helper methods:

	static void				framebufferResizeCallback(GLFWwindow* window, int width, int height);
	bool					checkValidationLayerSupport(const std::vector<const char*>& requiredLayers);
	void					populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);	///< Specify the details about the messenger and its callback.
	VkResult				CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
	std::vector<const char*> getRequiredExtensions();
	bool					checkExtensionSupport(const char* const* requiredExtensions, uint32_t reqExtCount);
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);	///< Callback for handling ourselves the validation layer's debug messages and decide which kind of messages to see.
	int						isDeviceSuitable(VkPhysicalDevice device, const int mode);	///< Evaluate a device and check if it is suitable for the operations we want to perform.
	VkSampleCountFlagBits	getMaxUsableSampleCount(bool getMinimum = false);	///< Get the maximum number of samples (for MSAA) according to the physical device.
	QueueFamilyIndices		findQueueFamilies(VkPhysicalDevice device);
	bool					checkDeviceExtensionSupport(VkPhysicalDevice device);
	SwapChainSupportDetails	querySwapChainSupport(VkPhysicalDevice device);
	VkSurfaceFormatKHR		chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);	///< Chooses the surface format (color depth) for the swap chain.
	VkPresentModeKHR		chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);	///< Chooses the presentation mode (conditions for "swapping" images to the screen) for the swap chain.
	VkExtent2D				chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);	///< Chooses the swap extent (resolution of images in swap chain) for the swap chain.
	VkFormat				findDepthFormat();	///< Find the right format for a depth image.
	VkFormat				findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);	///< Take a list of candidate formats in order from most desirable to least desirable, and checks which is the first one that is supported.
	bool					hasStencilComponent(VkFormat format);

public:
	VulkanEnvironment();

	// Public parameters:
	
	const bool add_MSAA = true;			// Shader MSAA (MultiSample AntiAliasing) <<<<<
	const bool add_SS   = true;			// Sample shading. This can solve some problems from shader MSAA (example: only smoothens out edges of geometry but not the interior filling) (https://www.khronos.org/registry/vulkan/specs/1.0/html/vkspec.html#primsrast-sampleshading).

	// Public methods:

	void			createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	uint32_t		findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);	///< Finds the right type of memory to use, depending upon the requirements of the buffer and our own application requiremnts.
	void			transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
	VkCommandBuffer	beginSingleTimeCommands();
	void			endSingleTimeCommands(VkCommandBuffer commandBuffer);
	VkImageView		createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);

	void			DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
	void			recreateSwapChain();
	void			cleanupSwapChain();
	void			cleanup();

	// Main member variables:

	GLFWwindow* window;												///< Opaque window object.

	VkInstance					 instance;							///< Opaque handle to an instance object. There is no global state in Vulkan and all per-application state is stored here.
	VkDebugUtilsMessengerEXT	 debugMessenger;					///< Opaque handle to a debug messenger object (the debug callback is part of it).
	VkSurfaceKHR				 surface;							///< Opaque handle to a surface object (abstract type of surface to present rendered images to)

	VkPhysicalDevice			 physicalDevice = VK_NULL_HANDLE;	///< Opaque handle to a physical device object.
	VkSampleCountFlagBits		 msaaSamples = VK_SAMPLE_COUNT_1_BIT;///< Number of samples for MSAA (MultiSampling AntiAliasing)
	VkDevice					 device;							///< Opaque handle to a device object.

	VkQueue						 graphicsQueue;						///< Opaque handle to a queue object (computer graphics).
	VkQueue						 presentQueue;						///< Opaque handle to a queue object (presentation to window surface).

	VkSwapchainKHR				 swapChain;							///< Swap chain object.
	VkFormat					 swapChainImageFormat;				///< Swap chain format.
	VkExtent2D					 swapChainExtent;					///< Swap chain extent.
	std::vector<VkImage>		 swapChainImages;					///< List. Opaque handle to an image object.
	std::vector<VkImageView>	 swapChainImageViews;				///< List. Opaque handle to an image view object. It allows to use VkImage in the render pipeline. It's a view into an image; it describes how to access the image and which part of the image to access.
	std::vector<VkFramebuffer>	 swapChainFramebuffers;				///< List. Opaque handle to a framebuffer object.

	VkRenderPass				 renderPass;						///< Opaque handle to a render pass object.

	VkCommandPool				 commandPool;						///< Opaque handle to a command pool object. It manages the memory that is used to store the buffers, and command buffers are allocated from them. 

	VkImage						 colorImage;						///< For MSAA
	VkDeviceMemory				 colorImageMemory;					///< For MSAA
	VkImageView					 colorImageView;					///< For MSAA

	VkImage						 depthImage;						///< Depth buffer (image object).
	VkDeviceMemory				 depthImageMemory;					///< Depth buffer memory (memory object).
	VkImageView					 depthImageView;					///< Depth buffer image view (images are accessed through image views rather than directly).

	bool						 framebufferResized = false;		///< Many drivers/platforms trigger VK_ERROR_OUT_OF_DATE_KHR after window resize, but it's not guaranteed. This variable handles resizes explicitly.
};

#endif