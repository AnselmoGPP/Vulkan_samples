/*
 * Methods:
 *
	-run
		-initWindow
		-initVulkan
			-createInstance
				-checkValidationLayerSupport
				-populateDebugMessengerCreateInfo > debugCallback
				-getRequiredExtensions
				-checkExtensionSupport
			-setupDebugMessenger
				-populateDebugMessengerCreateInfo > debugCallback
				-CreateDebugUtilsMessengerEXT
			-createSurface
			-pickPhysicalDevice
				-isDeviceSuitable
					-findQueueFamilies > isComplete
					-isComplete
			-createLogicalDevice
				-findQueueFamilies
		-mainLoop
		-cleanup
			-DestroyDebugUtilsMessengerEXT
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
	void mainLoop();
	void cleanup();

	GLFWwindow* window;									///< Opaque window object.
	VkInstance  instance;								///< Opaque handle to an instance object. There is no global state in Vulkan and all per-application state is stored here.
	VkDebugUtilsMessengerEXT debugMessenger;			///< Opaque handle to a debug messenger object (the debug callback is part of it).
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;	///< Opaque handle to a physical device object.
	VkDevice device;									///< Opaque handle to a device object.
	VkQueue graphicsQueue;								///< Opaque handle to a queue object (computer graphics).
	VkQueue presentQueue;								///< Opaque handle to a queue object (presentation to window surface).

	bool checkValidationLayerSupport(const std::vector<const char*> &requiredLayers, bool printData);
	bool checkExtensionSupport(const char* const* requiredExtensions, uint32_t reqExtCount, bool printData);

	std::vector<const char*> getRequiredExtensions();
	/// Callback for handling ourselves the validation layer's debug messages and decide which kind of messages to see.
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData);

	VkSurfaceKHR surface;								///< Opaque handle to a surface object (abstract type of surface to present rendered images to)

	/// Specify the details about the messenger and its callback.
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	// Evaluate a device and check if it is suitable for the operations we want to perform.
	int isDeviceSuitable(VkPhysicalDevice device, const int mode);

	/// Structure for storing vector indices of the queue families we want. Note that graphicsFamily and presentFamily could refer to the same queue family, but we included them separately because sometimes they are in different queue families.
	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphicsFamily;		///< Queue family capable of computer graphics.
		std::optional<uint32_t> presentFamily;		///< Queue family capable of presenting to our window surface.
		bool isComplete();							///< Checks whether all members have value.
	};

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
};
