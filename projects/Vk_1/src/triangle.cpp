
#include <iostream>
#include <stdexcept>
#include <cstdlib>				// EXIT_SUCCESS, EXIT_FAILURE
//#include <memory>				// std::unique_ptr, std::shared_ptr (used instead of RAII)
#include <cstring>				// strcmp()
#include <map>					// std::multimap<key, value>
#include <set>					// std::set<uint32_t>
#include <cstdint>				// UINT32_MAX
#include <algorithm>			// std::min / std::max
#include <fstream>

#include "triangle.hpp"
#include "params.hpp"


/**
 * Given a VkDebugUtilsMessengerCreateInfoEXT object, creates/loads the extension object (debug messenger) (VkDebugUtilsMessengerEXT) if it's available.
 * Because it is an extension function, it is not automatically loaded. So, we have to look up its address ourselves using vkGetInstanceProcAddr.
 * @param instance Vulkan instance (the debug messenger is specific to our Vulkan instance and its layers)
 * @param pCreateInfo VkDebugUtilsMessengerCreateInfoEXT object
 * @param pAllocator Optional allocator callback
 * @param pDebugMessenger Debug messenger object
 * @return Returns the extension object, or an error if is not available.
 */
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	// Load the extension object if it's available (the extension function needs to be explicitly loaded)
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

    // vkGetInstanceProcAddr returns nullptr is the function couldn't be loaded.
    if (func != nullptr)
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    else
        return VK_ERROR_EXTENSION_NOT_PRESENT;
}

/**
 * Cleans up the VkDebugUtilsMessengerEXT object.
 * @param instance Vulkan instance (the debug messenger is specific to our Vulkan instance and its layers)
 * @param debugMessenger Debug messenger object
 * @param pAllocator Optional allocator callback
 */
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
	// Similarly to vkCreateDebugUtilsMessengerEXT, the extension function needs to be explicitly loaded.
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
        func(instance, debugMessenger, pAllocator);
}

/**
	Structure for storing vector indices of the queue families we want. Note that graphicsFamily and presentFamily could refer to the same queue family, but we included them separately because sometimes they are in different queue families.
*/
struct HelloTriangleApp::QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily;		///< Queue family capable of computer graphics.
	std::optional<uint32_t> presentFamily;		///< Queue family capable of presenting to our window surface.
	bool isComplete();							///< Checks whether all members have value.
};

/**
* Though a swap chain may be available, it may not be compatible with our window surface, so we need to query for some details and check them. This struct will contain these details.
*/
struct HelloTriangleApp::SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;		// Basic surface capabilities: min/max number of images in swap chain, and min/max width/height of images.
	std::vector<VkSurfaceFormatKHR> formats;	// Surface formats: pixel format, color space.
	std::vector<VkPresentModeKHR> presentModes;	// Available presentation modes
};

void HelloTriangleApp::run()
{
	initWindow();
	initVulkan();
	mainLoop();
	cleanup();
}

void HelloTriangleApp::initWindow()
{
	glfwInit();
	
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);	// Tell GLFW not to create an OpenGL context
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);		// Disable resizable window

	window = glfwCreateWindow((int)WIDTH, (int)HEIGHT, "Vulkan", nullptr, nullptr);
}

void HelloTriangleApp::initVulkan()
{
	createInstance();
	setupDebugMessenger();
	createSurface();
	pickPhysicalDevice();
	createLogicalDevice();
	createSwapChain();
	createImageViews();
	createRenderPass();
	createGraphicsPipeline();
	createFramebuffers();
	createCommandPool();
	createCommandBuffers();
	createSyncObjects();
}

void HelloTriangleApp::createInstance()
{
	// Check validation layer support
	if(enableValidationLayers && !checkValidationLayerSupport(requiredValidationLayers, true))
		throw std::runtime_error("Validation layers requested, but not available!");

	// [Optional] Tell the compiler some info about the instance to create (used for optimization)
	VkApplicationInfo appInfo{};

	appInfo.sType 						= VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName 			= "Hello Triangle";
	appInfo.applicationVersion 			= VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName 				= "No Engine";
	appInfo.engineVersion 				= VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion 					= VK_API_VERSION_1_0;
	appInfo.pNext						= nullptr;		// pointer to extension information

	// Not optional. Tell the compiler the global extensions and validation layers we will use (applicable to the entire program, not a specific device)
	VkInstanceCreateInfo createInfo{};

	createInfo.sType					= VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo 		= &appInfo;

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    if (enableValidationLayers)
    {
        createInfo.ppEnabledLayerNames 	= requiredValidationLayers.data();
        createInfo.enabledLayerCount 	= static_cast<uint32_t>(requiredValidationLayers.size());
        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext 				= (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
    } else
    {
        createInfo.enabledLayerCount 	= 0;
        createInfo.pNext 				= nullptr;
    }

	auto extensions = getRequiredExtensions();
	createInfo.ppEnabledExtensionNames 	= extensions.data();
	createInfo.enabledExtensionCount 	= static_cast<uint32_t>(extensions.size());

	// Check for extension support
	if(!checkExtensionSupport(createInfo.ppEnabledExtensionNames, createInfo.enabledExtensionCount, true))
		throw std::runtime_error("Extensions requested, but not available!");

	// Create the instance
	if(vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
		throw std::runtime_error("Failed to create instance!");
}

void HelloTriangleApp::mainLoop()
{
	while(!glfwWindowShouldClose(window))
	{
		glfwPollEvents();	// Check for events
		drawFrame();
	}

	vkDeviceWaitIdle(device);	// Waits for the logical device to finish operations. Needed for cleaning up once drawing and presentation operations (drawFrame) have finished. Use vkQueueWaitIdle for waiting for operations in a specific command queue to be finished.
}

void HelloTriangleApp::cleanup()
{
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
		vkDestroyFence(device, inFlightFences[i], nullptr);
	}

	vkDestroyCommandPool(device, commandPool, nullptr);

	for (auto framebuffer : swapChainFramebuffers)
		vkDestroyFramebuffer(device, framebuffer, nullptr);

	vkDestroyPipeline(device, graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	vkDestroyRenderPass(device, renderPass, nullptr);

	for (auto imageView : swapChainImageViews)
		vkDestroyImageView(device, imageView, nullptr);

	vkDestroySwapchainKHR(device, swapChain, nullptr);
	vkDestroyDevice(device, nullptr);	// Destroys logical device (and device queues)

	if(enableValidationLayers)
		DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);

	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(instance, nullptr);
	glfwDestroyWindow(window);
	glfwTerminate();
}

bool HelloTriangleApp::checkValidationLayerSupport(const std::vector<const char*> &requiredLayers, bool printData)
{
	// Number of layers available
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	// Names of the available layers
	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	// Print "requiredLayers" and "availableLayers"
	if(printData)
	{
		std::cout << "Required validation layers: \n";
		for(size_t i = 0; i < requiredLayers.size(); ++i)
			std::cout << '\t' << requiredLayers[i] << '\n';

		std::cout << "Available validation layers: \n";
		for(size_t i = 0; i < layerCount; ++i)
			std::cout << '\t' << availableLayers[i].layerName << '\n';
	}

	// Check if all the "requiredLayers" exist in "availableLayers"
	for(const char* reqLayer : requiredLayers)
	{
		bool layerFound = false;
		for(const auto& layerProperties : availableLayers)
		{
			if(std::strcmp(reqLayer, layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}
		if(!layerFound)
			return false;		// If any layer is not found, returns false
	}

	return true;				// If all layers are found, returns true
}

bool HelloTriangleApp::checkExtensionSupport(const char* const* requiredExtensions, uint32_t reqExtCount, bool printData)
{
	// Number of extensions available
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	// Names of the available extensions
	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());

	// Print "requiredExtensions" and "availableExtensions"
	if(printData)
	{
		std::cout << "Required extensions: \n";
		for(size_t i = 0; i < reqExtCount; ++i)
			std::cout << '\t' << requiredExtensions[i] << '\n';

		std::cout << "Available extensions: \n";
		for(size_t i = 0; i < extensionCount; ++i)
			std::cout << '\t' << availableExtensions[i].extensionName << '\n';
	}

	// Check if all the "requiredExtensions" exist in "availableExtensions"
	for(size_t i = 0; i < reqExtCount; ++i)
	{
		bool extensionFound = false;
		for(const auto& extensionProperties : availableExtensions)
		{
			if(std::strcmp(requiredExtensions[i], extensionProperties.extensionName) == 0)
			{
				extensionFound = true;
				break;
			}
		}
		if(!extensionFound)
			return false;		// If any extension is not found, returns false
	}

	return true;				// If all extensions are found, returns true
}

/// Get a list of required extensions (based on whether validation layers are enabled or not)
std::vector<const char*> HelloTriangleApp::getRequiredExtensions()
{
	// Get required extensions (glfwExtensions)
	const char** glfwExtensions;
	uint32_t glfwExtensionCount = 0;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	// Store them in a vector
	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	// Add additional optional extensions
	
	// > VK_EXT_DEBUG_UTILS_EXTENSION_NAME == "VK_EXT_debug_utils". 
	// This extension is needed, together with a debug messenger, to set up a callback to handle messages and associated details
	if(enableValidationLayers)
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	return extensions;
}

/**
   The validation layers will print debug messages to the standard output by default.
   But by providing a callback we can handle them ourselves and decide which kind of messages to see.
   This callback function is added with the PFN_vkDebugUtilsMessengerCallbackEXT prototype.
   The VKAPI_ATTR and VKAPI_CALL ensure that the function has the right signature for Vulkan to call it.
   @param messageSeverity Specifies the severity of the message, which is one of the following flags (it's possible to use comparison operations between them):
		<ul>
			<li>VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: Diagnostic message.</li>
			<li>VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: Informational message (such as the creation of a resource).</li>
			<li>VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: Message about behavior (not necessarily an error, but very likely a bug).</li>
			<li>VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: Message about behavior that is invalid and may cause crashes.</li>
		</ul>
   @param messageType Can have the following values:
   		<ul>
			<li>VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT: Some event happened that is unrelated to the specification or performance.</li>
			<li>VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT: Something happened that violates the specification or indicates a possible mistake.</li>
			<li>VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT: Potential non-optimal use of Vulkan.</li>
		</ul>
   @param pCallbackData It refers to a VkDebugUtilsMessengerCallbackDataEXT struct containing the details of the message. Some important members:
      	<ul>
			<li>pMessage: Debug message as null-terminated string.</li>
			<li>pObjects: Array of Vulkan object handles related to the message.</li>
			<li>objectCount: Number of objects in the array.</li>
		</ul>
   @param pUserData Pointer (specified during the setup of the callback) that allows you to pass your own data.
   @return Boolean indicating if the Vulkan call that triggered the validation layer message should be aborted. If true, the call is aborted with the VK_ERROR_VALIDATION_FAILED_EXT error.
 */
VKAPI_ATTR VkBool32 VKAPI_CALL HelloTriangleApp::debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
{
	std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;
	return VK_FALSE;
}

/**
 * Specify the details about the messenger and its callback
 * There are a lot more settings for the behavior of validation layers than just the flags specified in the
 * VkDebugUtilsMessengerCreateInfoEXT struct. The file "$VULKAN_SDK/Config/vk_layer_settings.txt" explains how to configure the layers.
 * @param createInfo Struct that this method will use for setting the type of messages to receive, and the callback function.
 */
void HelloTriangleApp::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
	createInfo = {};
	// - Type of the struct
	createInfo.sType 			= VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	// - Specify the types of severities you would like your callback to be called for.
	createInfo.messageSeverity 	= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	// - Specify the types of messages your callback is notified about.
	createInfo.messageType 		= VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	// - Specify the pointer to the callback function.
	createInfo.pfnUserCallback 	= debugCallback;
	// - [Optional] Pass a pointer to the callback function through this parameter
	createInfo.pUserData 		= nullptr;
}

/**
 *	Specify the details about the messenger and its callback (there are more ways to configure validation layer messages and debug callbacks), and create the debug messenger.
 */
void HelloTriangleApp::setupDebugMessenger()
{
	if(!enableValidationLayers) return;
	
	// Fill in a structure with details about the messenger and its callback
    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

	// Create the debug messenger
	if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
	    throw std::runtime_error("Failed to set up debug messenger!");
}

/**
 * Look for and select a graphics card in the system that supports the features we need (Vulkan support).
 */
void HelloTriangleApp::pickPhysicalDevice()
{
	// Get all devices with Vulkan support.
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	if(deviceCount == 0)	throw std::runtime_error("Failed to find GPUs with Vulkan support!");
	else					std::cout << "Devices with Vulkan support: " << deviceCount << std::endl;

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	// Look for a suitable device and select it.
	const int mode = 1;
	switch(mode)
	{
	// Check Vulkan support.
	case 1:

	// Check for dedicated GPU supporting geometry shaders.
	case 2:
		for(const auto& device : devices)
			if(isDeviceSuitable(device, mode))
			{
				physicalDevice = device;
				break;
			}
		break;

	// Give each device a score and pick the highest one.
	case 3:
		{
			std::multimap<int, VkPhysicalDevice> candidates;	// Automatically sorts candidates by score

			for(const auto& device : devices)					// Rate each device
			{
				int score = isDeviceSuitable(device, mode);
				candidates.insert(std::make_pair(score, device));
			}

			if(candidates.rbegin()->first > 0)					// Check if the best candidate is suitable
				physicalDevice = candidates.rbegin()->second;
			else
				throw std::runtime_error("Failed to find a suitable GPU!");
			break;
		}
	// Error. The variable "mode" should have a valid value.
	default:
		throw std::runtime_error("No valid mode for selecting a suitable device!");
		break;
	}

	if(physicalDevice == VK_NULL_HANDLE)
		throw std::runtime_error("Failed to find a suitable GPU!");
}

/**
 * Evaluate a device and check if it is suitable for the operations we want to perform.
 * @param device Device to evaluate
 * @param mode Mode of evaluation:
 * 		<ul>
 *			<li>1: Check Vulkan support.</li>
 *			<li>2: Check for dedicated GPU supporting geometry shaders.</li>
 *			<li>3: Rate the device (give it a score).</li>
 *		</ul>
 * @return If 0 is returned the device is not suitable
 */
int HelloTriangleApp::isDeviceSuitable(VkPhysicalDevice device, const int mode)
{
	// Get basic device properties: Name, type, supported Vulkan version...
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);

	// Get optional features: Texture compression, 64 bit floats, multi-viewport rendering...
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	// Get queue families
	QueueFamilyIndices indices = findQueueFamilies(device);

	std::cout << "Queue families: \n"
			  << "\t- Computer graphics: "
			  << ((indices.graphicsFamily.has_value() == true)? "Yes" : "No") << '\n'
			  << "\t- Presentation to window surface: "
			  << ((indices.presentFamily.has_value()  == true)? "Yes" : "No") << std::endl;

	// Check whether required device extensions are supported 
	bool extensionsSupported = checkDeviceExtensionSupport(device);
	std::cout << "Required device extensions supported: " << (extensionsSupported ? "Yes" : "No") << std::endl;

	// Check whether swap chain extension is compatible with the window surface (adequate supported)
	bool swapChainAdequate = false;
	if (extensionsSupported)
	{
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();	// Adequate if there's at least one supported image format and one supported presentation mode.
	}

	// Find out whether the device is suitable
	switch (mode)
	{
		// Check Vulkan support:
	    case 1:
	        return	indices.isComplete() &&			// There should exist the queue families we want.
					extensionsSupported &&			// The required device extensions should be supported.
					swapChainAdequate;				// Swap chain extension support should be adequate (compatible with window surface)
	        break;
		// Check for dedicated GPU supporting geometry shaders:
	    case 2:
	    	return	indices.isComplete() &&
					extensionsSupported &&
					swapChainAdequate &&
	    			deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
					deviceFeatures.geometryShader;
	    	break;
		// Give a score to the device (rate the device):
	    case 3:
	    	{
	    		int score = 0;
	    		if(deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) score += 1000;	// Discrete GPUs have better performance.
	    		score += deviceProperties.limits.maxImageDimension2D;	    							// Maximum size of textures.
	    		if(!deviceFeatures.geometryShader)	return 0;											// Applications cannot function without geometry shaders.
	    		if(!indices.isComplete())			return 0;											// There should exist the queue families we want.
				if(!extensionsSupported)			return 0;											// The required device extensions should be supported.
				if (!swapChainAdequate)				return 0;											// Swap chain extension support should be adequate (compatible with window surface)
				return score;
	    		break;
	    	}
	    // Check Vulkan support:
	    default:
	        return 1;
	        break;
	}
}



/**
 * Check which queue families are supported by the device and which one of these supports the commands that we want to use (in this case, graphics commands).
 * Queue families: Any operation (drawing, uploading textures...) requires commands commands to be submitted to a queue. There are different types of queues that originate from different queue families and each family of queues allows only a subset of commands (graphics commands, compute commands, memory transfer related commands...).
 * @param device Device to evaluate
 * @return Structure containing vector indices of the queue families we want.
 */
HelloTriangleApp::QueueFamilyIndices HelloTriangleApp::findQueueFamilies(VkPhysicalDevice device)
{
	// Get queue families
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for(const auto& queueFamily : queueFamilies)
	{
		// Check queue families capable of presenting to our window surface
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
		if(presentSupport) indices.presentFamily = i;

		// Check queue families capable of computer graphics
		if(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			indices.graphicsFamily = i;

		if (indices.isComplete()) break;
		i++;
	}

	return indices;
}

bool HelloTriangleApp::QueueFamilyIndices::isComplete()
{
	return graphicsFamily.has_value() && presentFamily.has_value();
}

void HelloTriangleApp::createLogicalDevice()
{
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

	// Describe the number of queues you want for a each queue family
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	float queuePriority = 1.0f;
	for(uint32_t queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType 			 	= VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex 	= queueFamily;
		queueCreateInfo.queueCount		 	= 1;
		queueCreateInfo.pQueuePriorities	= &queuePriority;		// You can assign priorities to queues to influence the scheduling of command buffer execution using floats in the range [0.0, 1.0]. This is required even if there is only a single queue.
		queueCreateInfos.push_back(queueCreateInfo);
	}

	// Describe the set of features from the physical device that you will use (geometry shaders...)
	VkPhysicalDeviceFeatures deviceFeatures{};

	// Describe queue parameters
	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos		= queueCreateInfos.data();
	createInfo.queueCreateInfoCount		= static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pEnabledFeatures			= &deviceFeatures;
	createInfo.enabledExtensionCount	= static_cast<uint32_t>(requiredDeviceExtensions.size());
	createInfo.ppEnabledExtensionNames	= requiredDeviceExtensions.data();
	if(enableValidationLayers)
	{
	    createInfo.enabledLayerCount = static_cast<uint32_t>(requiredValidationLayers.size());
	    createInfo.ppEnabledLayerNames = requiredValidationLayers.data();
	}
	else
		createInfo.enabledLayerCount	= 0;

	// Create the logical device
	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
	    throw std::runtime_error("Failed to create logical device!");

	// Retrieve queue handles for each queue family (in this case, we created a single queue from each family, so we simply use index 0)
	vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
	vkGetDeviceQueue(device, indices.presentFamily.value(),  0, &presentQueue);
}

/**
 * Create a window surface (interface for interacting with the window system). Requires to use WSI (Window System Integration), which provided by GLFW.
 *
 */
void HelloTriangleApp::createSurface()
{
	if(glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
		throw std::runtime_error("Failed to create window surface!");
}

/**
	Check whether all the required device extensions are supported.
	@param device Device to evaluate
	@return True if all the required device extensions are supported. False otherwise.
*/
bool HelloTriangleApp::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(requiredDeviceExtensions.begin(), requiredDeviceExtensions.end());

	for (const auto& extension : availableExtensions)
		requiredExtensions.erase(extension.extensionName);

	return requiredExtensions.empty();
}

HelloTriangleApp::SwapChainSupportDetails HelloTriangleApp::querySwapChainSupport(VkPhysicalDevice device)
{
	SwapChainSupportDetails details;

	// Get the basic surface capabilities
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

	// Get the supported surface formats
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

	if (formatCount != 0) 
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
	}

	// Get supported presentation modes
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}

VkSurfaceFormatKHR HelloTriangleApp::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	// Return our favourite surface format, if it exists
	for (const auto& availableFormat : availableFormats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&				// Format: Color channel and types (example: VK_FORMAT_B8G8R8A8_SRGB is BGRA channels with 8 bit unsigned integer)
			availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)	// Color space: Indicates if the sRGB color space is supported or not (https://stackoverflow.com/questions/12524623/what-are-the-practical-differences-when-working-with-colors-in-a-linear-vs-a-no).
			return availableFormat;
	}

	// Otherwise, return the first format founded (other ways: rank the available formats on how "good" they are)
	return availableFormats[0];
}

/**
* Presentation mode represents the conditions for showing images to the screen. Four possible modes available in Vulkan:
* 		<ul>
			<li>VK_PRESENT_MODE_IMMEDIATE_KHR: Images submitted by your application are transferred to the screen right away (may cause tearing).</li>
			<li>VK_PRESENT_MODE_FIFO_KHR: The swap chain is a FIFO queue. The display takes images from the front. The program inserts images at the back. This is most similar to vertical sync as found in modern games. This is the only mode guaranteed to be available.</li>
			<li>VK_PRESENT_MODE_FIFO_RELAXED_KHR: Like the second mode, with one more property: If the application is late and the queue was empty at the last vertical blank (moment when the display is refreshed), instead of waiting for the next vertical blank, the image is transferred right away when it finally arrives (may cause tearing).</li>
			<li>VK_PRESENT_MODE_MAILBOX_KHR: Like the second mode, but instead of blocking the application when the queue is full, the images are replaced with the newer ones. This can be used to implement triple buffering, avoiding tearing with much less latency issues than standard vertical sync that uses double buffering.</li>
		</ul>
	This functions will choose VK_PRESENT_MODE_MAILBOX_KHR if available. Otherwise, it will choose VK_PRESENT_MODE_FIFO_KHR.
*/
VkPresentModeKHR HelloTriangleApp::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	// Choose VK_PRESENT_MODE_MAILBOX_KHR if available
	for (const auto& mode : availablePresentModes)
		if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
			return mode;

	// Otherwise, choose VK_PRESENT_MODE_FIFO_KHR
	return VK_PRESENT_MODE_FIFO_KHR;
}

/**
* The swap extent is set here. The swap extent is the resolution (in pixels) of the swap chain images, which is almost always equal to the resolution of the window where we are drawing (use {WIDHT, HEIGHT}), except when you're using a high DPI display (then, use glfwGetFramebufferSize).
*/
VkExtent2D HelloTriangleApp::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	// If width and height is set to the maximum value of UINT32_MAX, it indicates that the surface size will be determined by the extent of a swapchain targeting the surface. 
	if (capabilities.currentExtent.width != UINT32_MAX) 
		return capabilities.currentExtent;

	// Set width and height
	else 
	{
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		VkExtent2D actualExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}

/**

*/
void HelloTriangleApp::createSwapChain()
{
	// Get some properties
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);	// Surface formats (pixel format, color space)
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);	// Presentation modes
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);					// Basic surface capabilities

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;		// How many images in the swap chain? We choose the minimum required + 1 (this way, we won't have sometimes to wait on the driver to complete internal operations before we can acquire another image to render to.

	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)	// Don't exceed max. nuber of images (if maxImageCount == 0, there is no maximum)
		imageCount = swapChainSupport.capabilities.maxImageCount;

	// Configure the swap chain
	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;			// Number of layers each image consists of (always 1, except for stereoscopic 3D applications)
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;	// Kind of operations we'll use the images in the swap chain for. VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT let us render directly to the swap chain. VK_IMAGE_USAGE_TRANSFER_DST_BIT let us render images to a separate image ifrst to perform operations like post-processing and use memory operation to transfer the rendered image to a swap chain image. 

	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	if (indices.graphicsFamily != indices.presentFamily)	// Specify how to handle swap chain images that will be used across multiple queue families. This will be the case if the graphics queue family is different from the presentation queue (draws on the images in the swap chain from the graphics queue and submits them on the presentation queue).
	{
		createInfo.imageSharingMode		 = VK_SHARING_MODE_CONCURRENT;	// Best performance. An image is owned by one queue family at a time and ownership must be explicitly transferred before using it in another queue family.
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices	 = queueFamilyIndices;
	}
	else 
	{
		createInfo.imageSharingMode		 = VK_SHARING_MODE_EXCLUSIVE;	// Images can be used across multiple queue families without explicit ownership transfers.
		createInfo.queueFamilyIndexCount = 0;		// Optional
		createInfo.pQueueFamilyIndices	 = nullptr;	// Optional
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;	// currentTransform specifies that you don't want any transformation ot be applied to images in the swap chain.
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;				// Specify if the alpha channel should be used for blending with other windows in the window system. VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR makes it ignore the alpha channel.
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;												// If VK_TRUE, we don't care about colors of pixels that are obscured (example, because another window is in front of them).
	createInfo.oldSwapchain = VK_NULL_HANDLE;									// It's possible that your swap chain becomes invalid/unoptimized while the application is running (example: window resize), so your swap chain will need to be recreated from scratch and a reference to the old one must be specified in this field.

	// Create swap chain
	if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
		throw std::runtime_error("Failed to create swap chain!");

	// Retrieve the handles
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
	swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

	// Save format and extent for future use
	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;
}

void HelloTriangleApp::createImageViews()
{
	swapChainImageViews.resize(swapChainImages.size());

	for (size_t i = 0; i < swapChainImages.size(); i++)
	{
		VkImageViewCreateInfo createInfo{};

		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = swapChainImages[i];

		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;				// 1D, 2D, 3D, or cube map
		createInfo.format = swapChainImageFormat;

		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;	// Default color mapping
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		createInfo.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;	// Image's purpose and which part of it should be accessed. Here, our images will be used as color targets without any mipmapping levels or multiple layers. 
		createInfo.subresourceRange.baseMipLevel	= 0;
		createInfo.subresourceRange.levelCount		= 1;
		createInfo.subresourceRange.baseArrayLayer	= 0;
		createInfo.subresourceRange.layerCount		= 1;

		// Note about stereographic 3D applications: For these applications, you would create a swap chain with multiple layers, and then create multiple image views for each image (one for left eye and another for right eye).

		if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS)
			throw std::runtime_error("Failed to create image views!");
	}
}

void HelloTriangleApp::createRenderPass()
{
	// Attachment data
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format			= swapChainImageFormat;
	colorAttachment.samples			= VK_SAMPLE_COUNT_1_BIT;				// Single color buffer attachment, or many (multisampling).
	colorAttachment.loadOp			= VK_ATTACHMENT_LOAD_OP_CLEAR;			// What to do with the data (color and depth) in the attachment before rendering: VK_ATTACHMENT_LOAD_OP_ ... LOAD (preserve existing contents of the attachment), CLEAR (clear values to a constant at the start of a new frame), DONT_CARE (existing contents are undefined).
	colorAttachment.storeOp			= VK_ATTACHMENT_STORE_OP_STORE;			// What to do with the data (color and depth) in the attachment after rendering:  VK_ATTACHMENT_STORE_OP_ ... STORE (rendered contents will be stored in memory and can be read later), DON_CARE (contents of the framebuffer will be undefined after rendering).
	colorAttachment.stencilLoadOp	= VK_ATTACHMENT_LOAD_OP_DONT_CARE;		// What to do with the stencil data in the attachment before rendering.
	colorAttachment.stencilStoreOp	= VK_ATTACHMENT_STORE_OP_DONT_CARE;		// What to do with the stencil data in the attachment after rendering.
	colorAttachment.initialLayout	= VK_IMAGE_LAYOUT_UNDEFINED;			// Layout before the render pass. Textures and framebuffers in Vulkan are represented by VkImage objects with a certain pixel format, however the layout of the pixels in memory need to be transitioned to specific layouts suitable for the operation that they're going to be involved in next (read more below).
	colorAttachment.finalLayout		= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;		// Layout to automatically transition after the render pass finishes. VK_IMAGE_LAYOUT_ ... UNDEFINED (we don't care what previous layout the image was in, and the contents of the image are not guaranteed to be preserved), COLOR_ATTACHMENT_OPTIMAL (images used as color attachment), PRESENT_SRC_KHR (images to be presented in the swap chain), TRANSFER_DST_OPTIMAL (Images to be used as destination for a memory copy operation).

	// Specify subpasses and their attachments.
	//		- Subpasses: A single render pass can consist of multiple subpasses, which are subsequent rendering operations that depend on the contents of framebuffers in previous passes (example: a sequence of post-processing effects applied one after another). Grouping them into one render pass may give better performance. 
	//		- Attachment references: Every subpass references one or more of the attachments that we've described.

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment	= 0;											// Specify which attachment to reference by its index in the attachment descriptions array.
	colorAttachmentRef.layout		= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;		// Specify the layout we would like the attachment to have during a subpass that uses this reference. The layout VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL will give us the best performance.

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint		= VK_PIPELINE_BIND_POINT_GRAPHICS;				// VK_PIPELINE_BIND_POINT_GRAPHICS: This is a graphics subpass
	subpass.colorAttachmentCount	= 1;
	subpass.pColorAttachments		= &colorAttachmentRef;							// Attachment for color. The index of the attachment in this array is directly referenced from the fragment shader with the directive "layout(location = 0) out vec4 outColor".
	//subpass.pInputAttachments;													// Attachments read from a shader.
	//subpass.pResolveAttachments;													// Attachments used for multisampling color attachments.
	//subpass.pDepthStencilAttachment;												// Attachment for depth and stencil data.
	//subpass.pPreserveAttachments;													// Attachment not used by this subpass, but for which the data must be preserved.

	// Create the Render pass
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType			= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount	= 1;
	renderPassInfo.pAttachments		= &colorAttachment;
	renderPassInfo.subpassCount		= 1;
	renderPassInfo.pSubpasses		= &subpass;

	if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
		throw std::runtime_error("Failed to create render pass!");

	// One subpass dependency
	VkSubpassDependency dependency{};
	dependency.srcSubpass			= VK_SUBPASS_EXTERNAL;								// VK_SUBPASS_EXTERNAL: Refers to the implicit subpass before or after the render pass depending on whether it is specified in srcSubpass or dstSubpass.
	dependency.dstSubpass			= 0;												// Index of our subpass. The dstSubpass must always be higher than srcSubpass to prevent cycles in the dependency graph (unless one of the subpasses is VK_SUBPASS_EXTERNAL).
	dependency.srcStageMask			= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;	// Stage where to wait (for the swap chain to finish reading from the image).
	dependency.srcAccessMask		= 0;												// Operations that wait.
	dependency.dstStageMask			= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;	// Stage where to wait. 
	dependency.dstAccessMask		= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;				// Operations that wait (they involve the writing of the color attachment).
	renderPassInfo.dependencyCount	= 1;
	renderPassInfo.pDependencies	= &dependency;										// Array of dependencies.
}

/**
*	Graphics pipeline: Sequence of operations that take the vertices and textures of your meshes all the way to the pixels in the render targets. Stages (F: fixed-function stages, P: programable):
* 		<ul>
			<li>Vertex/Index buffer: Raw vertex data.</li>
			<li>Input assembler (F): Collects data from the buffers and may use an index buffer to repeat certain elements without duplicating the vertex data.</li>
			<li>Vertex shader (P): Run for every vertex. Generally, applies transformations to turn vertex positions from model space to screen space. Also passes per-vertex data down the pipeline.</li>
			<li>Tessellation shader (P): Subdivides geometry based on certain rules to increase mesh quality (example: make brick walls look less flat from nearby).</li>
			<li>Geometry shader (P): Run for every primitive (triangle, line, point). It can discard the primitive or output more new primitives. Similar to tessellation shader, more flexible but with worse performance.</li>
			<li>Rasterization (F): Discretizes primitives into fragments (pixel elements that fill the framebuffer). Attributes outputted by the vertex shaders are interpolated across fragments. Fragments falling outside the screen are discarded. Usually, fragments behind others are discarded (depth testing).</li>
			<li>Fragment shader (P): Run for every surviving fragment. Determines which framebuffer/s the fragments are written to and with which color and depth values (uses interpolated data from vertex shader, and may include things like texture coordinates, normals for lighting).</li>
			<li>Color blending (F): Mixes different fragments that map to the same pixel in the framebuffer (overwrite each other, add up, or mix based upon transparency).</li>
			<li>Framebuffer.</li>
		</ul>
	Some programmable stages are optional (example: tessellation and geometry stages). 
	In Vulkan, the graphics pipeline is almost completely immutable. You will have to create a number of pipelines representing all of the different combiantions of states you want to use.
*/
void HelloTriangleApp::createGraphicsPipeline()
{
	// Read shader files
	std::vector<char> vertShaderCode = readFile((shaders_dir + "triangleV.spv").c_str());
	std::vector<char> fragShaderCode = readFile((shaders_dir + "triangleF.spv").c_str());

	VkShaderModule vertShaderModule  = createShaderModule(vertShaderCode);
	VkShaderModule fragShaderModule  = createShaderModule(fragShaderCode);

	// Configure Vertex shader
	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType				= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage				= VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module				= vertShaderModule;
	vertShaderStageInfo.pName				= "main";			// Function to invoke (entrypoint). You may combine multiple fragment shaders into a single shader module and use different entry points (different behaviors).  
	vertShaderStageInfo.pSpecializationInfo = nullptr;			// Optional. Specifies values for shader constants.

	// Configure Fragment shader
	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType				= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage				= VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module				= fragShaderModule;
	fragShaderStageInfo.pName				= "main";
	fragShaderStageInfo.pSpecializationInfo = nullptr;

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	// Vertex input: Describes format of the vertex data that will be passed to the vertex shader.
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType							= VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount	= 0;
	vertexInputInfo.pVertexBindingDescriptions		= nullptr;	// Optional
	vertexInputInfo.vertexAttributeDescriptionCount = 0;
	vertexInputInfo.pVertexAttributeDescriptions	= nullptr;	// Optional

	// Input assembly: Describes what kind of geometry will be drawn from the vertices, and if primitive restart should be enabled.
	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType					 = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology				 = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;		// VK_PRIMITIVE_TOPOLOGY_ ... POINT_LIST, LINE_LIST, LINE_STRIP, TRIANGLE_LIST, TRIANGLE_STRIP
	inputAssembly.primitiveRestartEnable = VK_FALSE;								// If VK_TRUE, then it's possible to break up lines and triangles in the _STRIP topology modes by using a special index of 0xFFFF or 0xFFFFFFFF.

	// Viewport: Describes the region of the framebuffer that the output will be rendered to.
	VkViewport viewport{};
	viewport.x		  = 0.0f;
	viewport.y		  = 0.0f;
	viewport.width	  = (float)swapChainExtent.width;
	viewport.height	  = (float)swapChainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	// Scissor rectangle: Defines in which region pixels will actually be stored. Pixels outside the scissor rectangles will be discarded by the rasterizer. It works like a filter rather than a transformation.
	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = swapChainExtent;

	// Viewport state: Combines the viewport and scissor rectangle into a viewport state. Multiple viewports and scissors require enabling a GPU feature.
	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType			= VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports	= &viewport;
	viewportState.scissorCount	= 1;
	viewportState.pScissors		= &scissor;

	// Rasterizer: It takes the geometry shaped by the vertices from the vertex shader and turns it into fragments to be colored by the fragment shader. It also performs depth testing, face culling and the scissor test, and can be configured to output fragments that fill entire polygons or just the edges (wireframe rendering).
	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType					= VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable			= VK_FALSE;					// If VK_TRUE, fragments that are beyond the near and far planes are clamped to them (requires enabling a GPU feature), as opposed to discarding them.
	rasterizer.rasterizerDiscardEnable	= VK_FALSE;					// If VK_TRUE, geometry never passes through the rasterizer stage (disables any output to the framebuffer).
	rasterizer.polygonMode				= VK_POLYGON_MODE_FILL;		// How fragments are generated for geometry (VK_POLYGON_MODE_ ... FILL, LINE, POINT). Any mode other than FILL requires enabling a GPU feature.
	rasterizer.lineWidth				= 1.0f;						// Thickness of lines in terms of number of fragments. The maximum line width supported depends on the hardware. Lines thicker than 1.0f requires enabling the `wideLines` GPU feature.
	rasterizer.cullMode					= VK_CULL_MODE_BACK_BIT;	// Type of face culling (disable culling, cull front faces, cull back faces, cull both).
	rasterizer.frontFace				= VK_FRONT_FACE_CLOCKWISE;	// Vertex order for faces to be considered front-facing (clockwise, counterclockwise).
	rasterizer.depthBiasEnable			= VK_FALSE;					// If VK_TRUE, it allows to alter the depth values (sometimes used for shadow mapping).
	rasterizer.depthBiasConstantFactor	= 0.0f;						// [Optional] 
	rasterizer.depthBiasClamp			= 0.0f;						// [Optional] 
	rasterizer.depthBiasSlopeFactor		= 0.0f;						// [Optional] 


	// Multisampling: One way to perform anti-aliasing. Combines the fragment shader results of multiple polygons that rasterize to the same pixel. Requires enabling a GPU feature.
	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable	= VK_FALSE;
	multisampling.rasterizationSamples	= VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading		= 1.0f;						// [Optional]
	multisampling.pSampleMask			= nullptr;					// [Optional]
	multisampling.alphaToCoverageEnable = VK_FALSE;					// [Optional]
	multisampling.alphaToOneEnable		= VK_FALSE;					// [Optional]

	// Depth and stencil testing. Used if you are using a depth and/or stencil buffer.
	VkPipelineDepthStencilStateCreateInfo depthStencilState{};

	// Color blending: After a fragment shader has returned a color, it needs to be combined with the color that is already in the framebuffer. Two ways to do it: Mix old and new value to produce a final color, or combine the old and new value using a bitwise operation.
	//	- Configuration per attached framebuffer
	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask		 = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable		 = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;		// Optional. Check VkBlendFactor enum.
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;	// Optional
	colorBlendAttachment.colorBlendOp		 = VK_BLEND_OP_ADD;			// Optional. Check VkBlendOp enum.
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;		// Optional
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;	// Optional
	colorBlendAttachment.alphaBlendOp		 = VK_BLEND_OP_ADD;			// Optional

	// Options for implementing alpha blending (new color blended with old color based on its opacity):
	// colorBlendAttachment.blendEnable		 = VK_TRUE;
	// colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	// colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	// colorBlendAttachment.colorBlendOp		 = VK_BLEND_OP_ADD;
	// colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	// colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	// colorBlendAttachment.alphaBlendOp		 = VK_BLEND_OP_ADD;

	/*
	// Pseudocode demonstration:
	if (blendEnable)
	{
		finalColor.rgb = (srcColorBlendFactor * newColor.rgb) <colorBlendOp> (dstColorBlendFactor * oldColor.rgb);
		finalColor.a = (srcAlphaBlendFactor * newColor.a) <alphaBlendOp> (dstAlphaBlendFactor * oldColor.a);
	}
	else finalColor = newColor;

	finalColor = finalColor & colorWriteMask;
	*/

	//	- Global color blending settings. Set blend constants that you can use as blend factors in the aforementioned calculations.
	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType				= VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable		= VK_FALSE;					// VK_FALSE: Blending method of mixing values.  VK_TRUE: Blending method of bitwise values combination (this disables the previous structure, like blendEnable = VK_FALSE).
	colorBlending.logicOp			= VK_LOGIC_OP_COPY;			// Optional
	colorBlending.attachmentCount	= 1;
	colorBlending.pAttachments		= &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;						// Optional
	colorBlending.blendConstants[1] = 0.0f;						// Optional
	colorBlending.blendConstants[2] = 0.0f;						// Optional
	colorBlending.blendConstants[3] = 0.0f;						// Optional

	// Dynamic states: A limited amount of the state that we specified in the previous structs can actually be changed without recreating the pipeline (size of viewport, lined width, blend constants...). If you want to do that, you have to fill this struct. This will cause the configuration of these values to be ignored and you will be required to specify the data at drawing time. This struct can be substituted by a nullptr later on if you don't have any dynamic state.
	VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_LINE_WIDTH };

	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType				= VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount	= 2;
	dynamicState.pDynamicStates		= dynamicStates;

	// Create pipeline layout
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType					= VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount			= 0;			// Optional
	pipelineLayoutInfo.pSetLayouts				= nullptr;		// Optional
	pipelineLayoutInfo.pushConstantRangeCount	= 0;			// Optional. Push constants ara another way of passing dynamic values to shaders.
	pipelineLayoutInfo.pPushConstantRanges		= nullptr;		// Optional

	if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		throw std::runtime_error("Failed to create pipeline layout!");

	// Create graphics pipeline
	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType					= VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount				= 2;
	//pipelineInfo.flags					= VK_PIPELINE_CREATE_DERIVATIVE_BIT;	// Required for using basePipelineHandle and basePipelineIndex members
	pipelineInfo.pStages				= shaderStages;
	pipelineInfo.pVertexInputState		= &vertexInputInfo;
	pipelineInfo.pInputAssemblyState	= &inputAssembly;
	pipelineInfo.pViewportState			= &viewportState;
	pipelineInfo.pRasterizationState	= &rasterizer;
	pipelineInfo.pMultisampleState		= &multisampling;
	pipelineInfo.pDepthStencilState		= nullptr;			// Optional
	pipelineInfo.pColorBlendState		= &colorBlending;
	pipelineInfo.pDynamicState			= nullptr;			// Optional
	pipelineInfo.layout					= pipelineLayout;
	pipelineInfo.renderPass				= renderPass;		// <<< It's possible to use other render passes with this pipeline instead of this specific instance, but they have to be compatible with "renderPass" (https://www.khronos.org/registry/vulkan/specs/1.0/html/vkspec.html#renderpass-compatibility).
	pipelineInfo.subpass				= 0;
	pipelineInfo.basePipelineHandle		= VK_NULL_HANDLE;	// [Optional] Specify the handle of an existing pipeline.
	pipelineInfo.basePipelineIndex		= -1;				// [Optional] Reference another pipeline that is about to be created by index.

	if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
		throw std::runtime_error("Failed to create graphics pipeline!");

	// Cleanup
	vkDestroyShaderModule(device, fragShaderModule, nullptr);
	vkDestroyShaderModule(device, vertShaderModule, nullptr);
}

std::vector<char> HelloTriangleApp::readFile(const std::string& filename) 
{
	// Open file
	std::ifstream file(filename, std::ios::ate | std::ios::binary);		// ate: Start reading at the end of the of the file  /  binary: Read file as binary file (avoid text transformations)
	if (!file.is_open())
		throw std::runtime_error("Failed to open file!");

	// Allocate the buffer
	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	// Read data
	file.seekg(0);
	file.read(buffer.data(), fileSize);

	// Close file
	file.close();
	return buffer;
}

VkShaderModule HelloTriangleApp::createShaderModule(const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType	= VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode	= reinterpret_cast<const uint32_t*>(code.data());	// The default allocator from std::vector ensures that the data satisfies the alignment requirements of `uint32_t`.

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		throw std::runtime_error("Failed to create shader module!");

	return shaderModule;
}

void HelloTriangleApp::createFramebuffers()
{
	swapChainFramebuffers.resize(swapChainImageViews.size());

	for (size_t i = 0; i < swapChainImageViews.size(); i++) 
	{
		VkImageView attachments[] = {
			swapChainImageViews[i]
		};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType			= VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass		= renderPass;									// A framebuffer can only be used with the render passes that it is compatible with, which roughly means that they use the same number and type of attachments.
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments	= attachments;									// Objects that should be bound to the respective attachment descriptions in the render pass pAttachment array.
		framebufferInfo.width			= swapChainExtent.width;
		framebufferInfo.height			= swapChainExtent.height;
		framebufferInfo.layers			= 1;											// Number of layers in image arrays. If your swap chain images are single images, then layers = 1.

		if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS)
			throw std::runtime_error("Failed to create framebuffer!");
	}
}

void HelloTriangleApp::createCommandPool()
{
	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

	// Command buffers are executed by submitting them on one of the device queues we retrieved (graphics queue, presentation queue, etc.). Each command pool can only allocate command buffers that are submitted on a single type of queue.
	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType				= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex	= queueFamilyIndices.graphicsFamily.value();
	poolInfo.flags				= 0;	// [Optional]  VK_COMMAND_POOL_CREATE_ ... TRANSIENT_BIT (command buffers are rerecorded with new commands very often - may change memory allocation behavior), RESET_COMMAND_BUFFER_BIT (command buffers can be rerecorded individually, instead of reseting all of them together). Not necessary if we just record the command buffers at the beginning of the program and then execute them many times in the main loop.

	if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
		throw std::runtime_error("Failed to create command pool!");
}

void HelloTriangleApp::createCommandBuffers()
{
	// Commmand buffer allocation
	commandBuffers.resize(swapChainFramebuffers.size());

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType				 = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool		 = commandPool;
	allocInfo.level				 = VK_COMMAND_BUFFER_LEVEL_PRIMARY;					// VK_COMMAND_BUFFER_LEVEL_ ... PRIMARY (can be submitted to a queue for execution, but cannot be called from other command buffers), SECONDARY (cannot be submitted directly, but can be called from primary command buffers - useful for reusing common operations from primary command buffers).
	allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();					// Number of buffers to allocate.

	if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
		throw std::runtime_error("Failed to allocate command buffers!");

	// Start command buffer recording and a render pass
	for (size_t i = 0; i < commandBuffers.size(); i++) 
	{
		// Start command buffer recording
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags				= 0;			// [Optional] VK_COMMAND_BUFFER_USAGE_ ... ONE_TIME_SUBMIT_BIT (the command buffer will be rerecorded right after executing it once), RENDER_PASS_CONTINUE_BIT (secondary command buffer that will be entirely within a single render pass), SIMULTANEOUS_USE_BIT (the command buffer can be resubmitted while it is also already pending execution).
		beginInfo.pInheritanceInfo	= nullptr;		// [Optional] Only relevant for secondary command buffers. It specifies which state to inherit from the calling primary command buffers.

		if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS)		// If a command buffer was already recorded once, this call resets it. It's not possible to append commands to a buffer at a later time.
			throw std::runtime_error("Failed to begin recording command buffer!");

		// Starting a render pass
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType				= VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass			= renderPass;
		renderPassInfo.framebuffer			= swapChainFramebuffers[i];
		renderPassInfo.renderArea.offset	= { 0, 0 };
		renderPassInfo.renderArea.extent	= swapChainExtent;				// Size of the render area (where shader loads and stores will take place). Pixels outside this region will have undefined values. It should match the size of the attachments for best performance.
		VkClearValue clearColor				= { 0.0f, 0.0f, 0.0f, 1.0f };	// Black, with 100% opacity
		renderPassInfo.clearValueCount		= 1;							// Clear values to use for VK_ATTACHMENT_LOAD_OP_CLEAR, 
		renderPassInfo.pClearValues			= &clearColor;					// which we used as load operation for the color attachment.

		vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);		// VK_SUBPASS_CONTENTS_INLINE (the render pass commands will be embedded in the primary command buffer itself and no secondary command buffers will be executed), VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS (the render pass commands will be executed from secondary command buffers).
	
		// Basic drawing commands
		vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);	// Second parameter: Specifies if the pipeline object is a graphics or compute pipeline.
		vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);													// Draw the triangle. Parameters: command buffer, vertexCount (we have 3 vertices to draw), instanceCount (0 if you're doing instanced rendering), firstVertex (offset into the vertex buffer, lowest value of gl_VertexIndex), firstInstance (offset for instanced rendering, lowest value of gl_InstanceIndex).
	
		// Finish up
		vkCmdEndRenderPass(commandBuffers[i]);
		if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
			throw std::runtime_error("Failed to record command buffer!");
	}
}

/**
*	Each of these 3 events are executed asynchronously (the function call returns before the operations are finished, with undefined order of execution). However, each of the operations depends on the previous one finishing, so we need to synchronize the swap chain events. Two ways: semaphores (mainly designed to synchronize within or accross command queues. Best fit here) and fences (mainly designed to synchronize your application itself with rendering operation).
*	Synchronization examples: https://github.com/KhronosGroup/Vulkan-Docs/wiki/Synchronization-Examples#swapchain-image-acquire-and-present
*/
void HelloTriangleApp::drawFrame()
{
	vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);		// Wait for the frame to be finished. If VK_TRUE, we wait for all fences.

	// Acquire an image from the swap chain
	uint32_t imageIndex;
	vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);		// Swap chain is an extension feature. imageIndex: index to the VkImage in our swapChainImages.

	// Check if this image is being used. If used, wait. Then, mark it as used by this frame.
	if (imagesInFlight[imageIndex] != VK_NULL_HANDLE)				// Check if a previous frame is using this image (i.e. there is its fence to wait on)
		vkWaitForFences(device, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
	imagesInFlight[imageIndex] = inFlightFences[currentFrame];		// Mark the image as now being in use by this frame

	// Submit the command buffer
	VkSubmitInfo submitInfo{};
	submitInfo.sType				  = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	VkSemaphore waitSemaphores[]	  = { imageAvailableSemaphores[currentFrame] };			// Which semaphores to wait on before execution begins.
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };	// In which stages of the pipeline to wait the semaphore. VK_PIPELINE_STAGE_ ... TOP_OF_PIPE_BIT (ensures that the render passes don't begin until the image is available), COLOR_ATTACHMENT_OUTPUT_BIT (makes the render pass wait for this stage).
	submitInfo.waitSemaphoreCount	  = 1;
	submitInfo.pWaitSemaphores		  = waitSemaphores;
	submitInfo.pWaitDstStageMask	  = waitStages;
	submitInfo.commandBufferCount	  = 1;
	submitInfo.pCommandBuffers		  = &commandBuffers[imageIndex];						// Command buffers to submit for execution (here, the one that binds the swap chain image we just acquired as color attachment).
	VkSemaphore signalSemaphores[]    = { renderFinishedSemaphores[currentFrame] };			// Which semaphores to signal once the command buffers have finished execution.
	submitInfo.signalSemaphoreCount   = 1;
	submitInfo.pSignalSemaphores	  = signalSemaphores;

	vkResetFences(device, 1, &inFlightFences[currentFrame]);		// Reset the fence to the unsignaled state.

	if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)	// Submit the command buffer to the graphics queue. An array of VkSubmitInfo structs can be taken as argument when workload is much larger, for efficiency.
		throw std::runtime_error("Failed to submit draw command buffer!");

	// Note:
	// Subpass dependencies: Subpasses in a render pass automatically take care of image layout transitions. These transitions are controlled by subpass dependencies (specify memory and execution dependencies between subpasses).
	// There are two built-in dependencies that take care of the transition at the start and at the end of the render pass, but the former does not occur at the right time. It assumes that the transition occurs at the start of the pipeline, but we haven't acquired the image yet at that point. Two ways to deal with this problem:
	//		- waitStages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT (ensures that the render passes don't begin until the image is available).
	//		- waitStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT (makes the render pass wait for this stage).
	
	// Presentation (submit the result back to the swap chain to have it eventually show up on the screen).
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType				= VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount	= 1;
	presentInfo.pWaitSemaphores		= signalSemaphores;

	VkSwapchainKHR swapChains[]		= { swapChain };
	presentInfo.swapchainCount		= 1;
	presentInfo.pSwapchains			= swapChains;
	presentInfo.pImageIndices		= &imageIndex;
	presentInfo.pResults			= nullptr;			// Optional

	vkQueuePresentKHR(presentQueue, &presentInfo);		// Submit request to present an image to the swap chain. Our triangle may look a bit different because the shader interpolates in linear color space and then converts to sRGB color space.

	// vkQueueWaitIdle(presentQueue);					// Make the whole graphics pipeline to be used only one frame at a time (instead of using this, we use multiple semaphores for processing frames concurrently).

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;	// By using the modulo operator (%), the frame index loops around after every MAX_FRAMES_IN_FLIGHT enqueued frames.
}

void HelloTriangleApp::createSyncObjects()
{
	imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
	imagesInFlight.resize(swapChainImages.size(), VK_NULL_HANDLE);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i])					 != VK_SUCCESS) 
		{
			throw std::runtime_error("Failed to create synchronization objects for a frame!");
		}
	}
}