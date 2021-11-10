
#include <stdexcept>
#include <iostream>
//#include <memory>				// std::unique_ptr, std::shared_ptr (used instead of RAII)
#include <map>					// std::multimap<key, value>
#include <set>					// std::set<uint32_t>
#include <array>
//#include <cstring>			// strcmp()

#include "environment.hpp"

bool QueueFamilyIndices::isComplete()
{
	return	graphicsFamily.has_value() &&
		presentFamily.has_value();
}

VulkanEnvironment::VulkanEnvironment()
{
	initWindow();

	createInstance();
	setupDebugMessenger();
	createSurface();
	pickPhysicalDevice();
	createLogicalDevice();
	createSwapChain();
	createImageViews();
	createRenderPass();

	createCommandPool();
	if (add_MSAA) createColorResources();
	createDepthResources();
	createFramebuffers();

	// Others
	minUniformBufferOffsetAlignment = getMinUniformBufferOffsetAlignment();
}

void VulkanEnvironment::initWindow()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);	// Tell GLFW not to create an OpenGL context
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);		// Enable resizable window (default)

	window = glfwCreateWindow((int)WIDTH, (int)HEIGHT, "Vulkan", nullptr, nullptr);
	//glfwSetWindowUserPointer(window, this);								// Input class has been set as windowUserPointer
	//glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);	// This callback has been set in Input

	glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);
}

// (1) 
void VulkanEnvironment::createInstance()
{
	// Check validation layer support
	if (enableValidationLayers && !checkValidationLayerSupport(requiredValidationLayers))
		throw std::runtime_error("Validation layers requested, but not available!");

	// [Optional] Tell the compiler some info about the instance to create (used for optimization)
	VkApplicationInfo appInfo{};

	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Hello Triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;
	appInfo.pNext = nullptr;					// pointer to extension information

	// Not optional. Tell the compiler the global extensions and validation layers we will use (applicable to the entire program, not a specific device)
	VkInstanceCreateInfo createInfo{};

	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
	if (enableValidationLayers)
	{
		createInfo.ppEnabledLayerNames = requiredValidationLayers.data();
		createInfo.enabledLayerCount = static_cast<uint32_t>(requiredValidationLayers.size());
		populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else
	{
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
	}

	auto extensions = getRequiredExtensions();
	createInfo.ppEnabledExtensionNames = extensions.data();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());

	// Check for extension support
	if (!checkExtensionSupport(createInfo.ppEnabledExtensionNames, createInfo.enabledExtensionCount))
		throw std::runtime_error("Extensions requested, but not available!");

	// Create the instance
	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
		throw std::runtime_error("Failed to create instance!");
}

bool VulkanEnvironment::checkValidationLayerSupport(const std::vector<const char*>& requiredLayers)
{
	// Number of layers available
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	// Names of the available layers
	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	// Print "requiredLayers" and "availableLayers"
	if (printInfo)
	{
		std::cout << "Required validation layers: \n";
		for (size_t i = 0; i < requiredLayers.size(); ++i)
			std::cout << '\t' << requiredLayers[i] << '\n';

		std::cout << "Available validation layers: \n";
		for (size_t i = 0; i < layerCount; ++i)
			std::cout << '\t' << availableLayers[i].layerName << '\n';
	}

	// Check if all the "requiredLayers" exist in "availableLayers"
	for (const char* reqLayer : requiredLayers)
	{
		bool layerFound = false;
		for (const auto& layerProperties : availableLayers)
		{
			if (std::strcmp(reqLayer, layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}
		if (!layerFound)
			return false;		// If any layer is not found, returns false
	}

	return true;				// If all layers are found, returns true
}

/**
 * Specify the details about the messenger and its callback
 * There are a lot more settings for the behavior of validation layers than just the flags specified in the
 * VkDebugUtilsMessengerCreateInfoEXT struct. The file "$VULKAN_SDK/Config/vk_layer_settings.txt" explains how to configure the layers.
 * @param createInfo Struct that this method will use for setting the type of messages to receive, and the callback function.
 */
void VulkanEnvironment::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
	createInfo = {};
	// - Type of the struct
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	// - Specify the types of severities you would like your callback to be called for.
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	// - Specify the types of messages your callback is notified about.
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	// - Specify the pointer to the callback function.
	createInfo.pfnUserCallback = debugCallback;
	// - [Optional] Pass a pointer to the callback function through this parameter
	createInfo.pUserData = nullptr;
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
VKAPI_ATTR VkBool32 VKAPI_CALL VulkanEnvironment::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;
	return VK_FALSE;
}

/// Get a list of required extensions (based on whether validation layers are enabled or not)
std::vector<const char*> VulkanEnvironment::getRequiredExtensions()
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
	if (enableValidationLayers)
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	return extensions;
}

bool VulkanEnvironment::checkExtensionSupport(const char* const* requiredExtensions, uint32_t reqExtCount)
{
	// Number of extensions available
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	// Names of the available extensions
	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());

	// Print "requiredExtensions" and "availableExtensions"
	if (printInfo)
	{
		std::cout << "Required extensions: \n";
		for (size_t i = 0; i < reqExtCount; ++i)
			std::cout << '\t' << requiredExtensions[i] << '\n';

		std::cout << "Available extensions: \n";
		for (size_t i = 0; i < extensionCount; ++i)
			std::cout << '\t' << availableExtensions[i].extensionName << '\n';
	}

	// Check if all the "requiredExtensions" exist in "availableExtensions"
	for (size_t i = 0; i < reqExtCount; ++i)
	{
		bool extensionFound = false;
		for (const auto& extensionProperties : availableExtensions)
		{
			if (std::strcmp(requiredExtensions[i], extensionProperties.extensionName) == 0)
			{
				extensionFound = true;
				break;
			}
		}
		if (!extensionFound)
			return false;		// If any extension is not found, returns false
	}

	return true;				// If all extensions are found, returns true
}

// (2)
/**
 *	Specify the details about the messenger and its callback (there are more ways to configure validation layer messages and debug callbacks), and create the debug messenger.
 */
void VulkanEnvironment::setupDebugMessenger()
{
	if (!enableValidationLayers) return;

	// Fill in a structure with details about the messenger and its callback
	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	populateDebugMessengerCreateInfo(createInfo);

	// Create the debug messenger
	if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
		throw std::runtime_error("Failed to set up debug messenger!");
}

/**
 * Given a VkDebugUtilsMessengerCreateInfoEXT object, creates/loads the extension object (debug messenger) (VkDebugUtilsMessengerEXT) if it's available.
 * Because it is an extension function, it is not automatically loaded. So, we have to look up its address ourselves using vkGetInstanceProcAddr.
 * @param instance Vulkan instance (the debug messenger is specific to our Vulkan instance and its layers)
 * @param pCreateInfo VkDebugUtilsMessengerCreateInfoEXT object
 * @param pAllocator Optional allocator callback
 * @param pDebugMessenger Debug messenger object
 * @return Returns the extension object, or an error if is not available.
 */
VkResult VulkanEnvironment::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	// Load the extension object if it's available (the extension function needs to be explicitly loaded)
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

	// vkGetInstanceProcAddr returns nullptr is the function couldn't be loaded.
	if (func != nullptr)
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	else
		return VK_ERROR_EXTENSION_NOT_PRESENT;
}

// (3)
/**
 * Create a window surface (interface for interacting with the window system). Requires to use WSI (Window System Integration), which is provided by GLFW.
 *
 */
void VulkanEnvironment::createSurface()
{
	if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
		throw std::runtime_error("Failed to create window surface!");
}

// (4)
/**
 * Look for and select a graphics card in the system that supports the features we need (Vulkan support).
 */
void VulkanEnvironment::pickPhysicalDevice()
{
	// Get all devices with Vulkan support.
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	if (deviceCount == 0)	throw std::runtime_error("Failed to find GPUs with Vulkan support!");
	else					if (printInfo) std::cout << "Devices with Vulkan support: " << deviceCount << std::endl;

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	// Look for a suitable device and select it.
	const int mode = 1;
	switch (mode)
	{
	case 1:	// Check Vulkan support.

	case 2:	// Check for dedicated GPU supporting geometry shaders.
		for (const auto& device : devices)
			if (isDeviceSuitable(device, mode))
			{
				physicalDevice = device;
				msaaSamples = getMaxUsableSampleCount(add_MSAA ? false : true);
				break;
			}
		break;

	case 3:	// Give each device a score and pick the highest one.
	{
		std::multimap<int, VkPhysicalDevice> candidates;	// Automatically sorts candidates by score

		for (const auto& device : devices)					// Rate each device
		{
			int score = isDeviceSuitable(device, mode);
			candidates.insert(std::make_pair(score, device));
		}

		if (candidates.rbegin()->first > 0)					// Check if the best candidate has score > 0
		{
			physicalDevice = candidates.rbegin()->second;
			msaaSamples = getMaxUsableSampleCount(add_MSAA ? false : true);
		}
		else
			throw std::runtime_error("Failed to find a suitable GPU!");
		break;
	}

	default: // Error. The variable "mode" should have a valid value.
		throw std::runtime_error("No valid mode for selecting a suitable device!");
		break;
	}

	if (physicalDevice == VK_NULL_HANDLE)
		throw std::runtime_error("Failed to find a suitable GPU!");

	if (printInfo) std::cout << "Minimum uniform buffer offset alignment: " << getMinUniformBufferOffsetAlignment() << std::endl;
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
int VulkanEnvironment::isDeviceSuitable(VkPhysicalDevice device, const int mode)
{
	// Get basic device properties: Name, type, supported Vulkan version...
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);

	// Get optional features: Texture compression, 64 bit floats, multi-viewport rendering...
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	// Get queue families
	QueueFamilyIndices indices = findQueueFamilies(device);

	// Check whether required device extensions are supported 
	bool extensionsSupported = checkDeviceExtensionSupport(device);

	if (printInfo)
	{
		std::cout << "Queue families: \n"
			<< "\t- Computer graphics: "
			<< ((indices.graphicsFamily.has_value() == true) ? "Yes" : "No") << '\n'
			<< "\t- Presentation to window surface: "
			<< ((indices.presentFamily.has_value() == true) ? "Yes" : "No") << std::endl;

		std::cout << "Required device extensions supported: " << (extensionsSupported ? "Yes" : "No") << std::endl;
	}

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
		return	indices.isComplete() &&				// There should exist the queue families we want.
			extensionsSupported &&				// The required device extensions should be supported.
			swapChainAdequate &&				// Swap chain extension support should be adequate (compatible with window surface)
			deviceFeatures.samplerAnisotropy;	// Physical device should support anisotropic filtering
		break;
		// Check for dedicated GPU supporting geometry shaders:
	case 2:
		return	indices.isComplete() &&
			extensionsSupported &&
			swapChainAdequate &&
			deviceFeatures.samplerAnisotropy &&
			deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
			deviceFeatures.geometryShader;
		break;
		// Give a score to the device (rate the device):
	case 3:
	{
		int score = 0;
		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) score += 1000;	// Discrete GPUs have better performance.
		score += deviceProperties.limits.maxImageDimension2D;	    							// Maximum size of textures.
		if (!deviceFeatures.geometryShader)	return 0;											// Applications cannot function without geometry shaders.
		if (!indices.isComplete())			return 0;											// There should exist the queue families we want.
		if (!extensionsSupported)			return 0;											// The required device extensions should be supported.
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
QueueFamilyIndices VulkanEnvironment::findQueueFamilies(VkPhysicalDevice device)
{
	// Get queue families
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies)
	{
		// Check queue families capable of presenting to our window surface
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
		if (presentSupport) indices.presentFamily = i;

		// Check queue families capable of computer graphics
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			indices.graphicsFamily = i;

		if (indices.isComplete()) break;
		i++;
	}

	return indices;
}

/**
	Check whether all the required device extensions are supported.
	@param device Device to evaluate
	@return True if all the required device extensions are supported. False otherwise.
*/
bool VulkanEnvironment::checkDeviceExtensionSupport(VkPhysicalDevice device)
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

SwapChainSupportDetails VulkanEnvironment::querySwapChainSupport(VkPhysicalDevice device)
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
	if (presentModeCount != 0)
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}

VkSampleCountFlagBits VulkanEnvironment::getMaxUsableSampleCount(bool getMinimum)
{
	if (getMinimum) return VK_SAMPLE_COUNT_1_BIT;

	VkPhysicalDeviceProperties physicalDeviceProperties;
	vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

	VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
	if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
	if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
	if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
	if (counts & VK_SAMPLE_COUNT_8_BIT ) { return VK_SAMPLE_COUNT_8_BIT;  }
	if (counts & VK_SAMPLE_COUNT_4_BIT ) { return VK_SAMPLE_COUNT_4_BIT;  }
	if (counts & VK_SAMPLE_COUNT_2_BIT ) { return VK_SAMPLE_COUNT_2_BIT;  }

	return VK_SAMPLE_COUNT_1_BIT;
}

// (5)
void VulkanEnvironment::createLogicalDevice()
{
	// Get the queue families supported by the physical device.
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

	// Describe the number of queues you want for each queue family
	std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };
	float queuePriority = 1.0f;
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

	for (uint32_t queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;		// You can assign priorities to queues to influence the scheduling of command buffer execution using floats in the range [0.0, 1.0]. This is required even if there is only a single queue.

		queueCreateInfos.push_back(queueCreateInfo);
	}

	// Describe the set of features from the physical device that you will use (geometry shaders...)
	VkPhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.samplerAnisotropy = VK_TRUE;							// Anisotropic filtering is an optional device feature (most modern graphics cards support it, but we should check it in isDeviceSuitable)
	deviceFeatures.sampleRateShading = (add_SS ? VK_TRUE : VK_FALSE);	// Enable sample shading feature for the device

	// Describe queue parameters
	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredDeviceExtensions.size());
	createInfo.ppEnabledExtensionNames = requiredDeviceExtensions.data();
	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(requiredValidationLayers.size());
		createInfo.ppEnabledLayerNames = requiredValidationLayers.data();
	}
	else
		createInfo.enabledLayerCount = 0;

	// Create the logical device
	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
		throw std::runtime_error("Failed to create logical device!");

	// Retrieve queue handles for each queue family (in this case, we created a single queue from each family, so we simply use index 0)
	vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
	vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
}

// (6)
void VulkanEnvironment::createSwapChain()
{
	// Get some properties
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);	// Surface formats (pixel format, color space)
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);	// Presentation modes
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);		// Basic surface capabilities

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;		// How many images in the swap chain? We choose the minimum required + 1 (this way, we won't have sometimes to wait on the driver to complete internal operations before we can acquire another image to render to.

	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)	// Don't exceed max. number of images (if maxImageCount == 0, there is no maximum)
		imageCount = swapChainSupport.capabilities.maxImageCount;

	// Configure the swap chain
	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;									// Number of layers each image consists of (always 1, except for stereoscopic 3D applications)
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;	// Kind of operations we'll use the images in the swap chain for. VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT let us render directly to the swap chain. VK_IMAGE_USAGE_TRANSFER_DST_BIT let us render images to a separate image ifrst to perform operations like post-processing and use memory operation to transfer the rendered image to a swap chain image. 

	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	if (indices.graphicsFamily != indices.presentFamily)					// Specify how to handle swap chain images that will be used across multiple queue families. This will be the case if the graphics queue family is different from the presentation queue (draws on the images in the swap chain from the graphics queue and submits them on the presentation queue).
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;	// Best performance. An image is owned by one queue family at a time and ownership must be explicitly transferred before using it in another queue family.
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;	// Images can be used across multiple queue families without explicit ownership transfers.
		createInfo.queueFamilyIndexCount = 0;							// Optional
		createInfo.pQueueFamilyIndices = nullptr;						// Optional
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;	// currentTransform specifies that you don't want any transformation ot be applied to images in the swap chain.
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;				// Specify if the alpha channel should be used for blending with other windows in the window system. VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR makes it ignore the alpha channel.
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;											// If VK_TRUE, we don't care about colors of pixels that are obscured (example, because another window is in front of them).
	createInfo.oldSwapchain = VK_NULL_HANDLE;									// It's possible that your swap chain becomes invalid/unoptimized while the application is running (example: window resize), so your swap chain will need to be recreated from scratch and a reference to the old one must be specified in this field.

	// Create swap chain
	if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
		throw std::runtime_error("Failed to create swap chain!");

	// Retrieve the handles
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
	swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

	if(printInfo) std::cout << "Swap chain images: " << swapChainImages.size() << std::endl;

	// Save format and extent for future use
	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;
}

VkSurfaceFormatKHR VulkanEnvironment::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	// Return our favourite surface format, if it exists
	for (const auto& availableFormat : availableFormats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&			// Format: Color channel and types (example: VK_FORMAT_B8G8R8A8_SRGB is BGRA channels with 8 bit unsigned integer)
			availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)	// Color space: Indicates if the sRGB color space is supported or not (https://stackoverflow.com/questions/12524623/what-are-the-practical-differences-when-working-with-colors-in-a-linear-vs-a-no).
			return availableFormat;
	}

	// Otherwise, return the first format founded (other ways: rank the available formats on how "good" they are)
	return availableFormats[0];
}

/**
* The swap extent is set here. The swap extent is the resolution (in pixels) of the swap chain images, which is almost always equal to the resolution of the window where we are drawing (use {WIDHT, HEIGHT}), except when you're using a high DPI display (then, use glfwGetFramebufferSize).
*/
VkExtent2D VulkanEnvironment::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	// If width and height is set to the maximum value of UINT32_MAX, it indicates that the surface size will be determined by the extent of a swapchain targeting the surface. 
	if (capabilities.currentExtent.width != UINT32_MAX)
		return capabilities.currentExtent;

	// Set width and height (useful when you're using a high DPI display)
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
* Presentation mode represents the conditions for showing images to the screen. Four possible modes available in Vulkan:
* 		<ul>
			<li>VK_PRESENT_MODE_IMMEDIATE_KHR: Images submitted by your application are transferred to the screen right away (may cause tearing).</li>
			<li>VK_PRESENT_MODE_FIFO_KHR: The swap chain is a FIFO queue. The display takes images from the front. The program inserts images at the back. This is most similar to vertical sync as found in modern games. This is the only mode guaranteed to be available.</li>
			<li>VK_PRESENT_MODE_FIFO_RELAXED_KHR: Like the second mode, with one more property: If the application is late and the queue was empty at the last vertical blank (moment when the display is refreshed), instead of waiting for the next vertical blank, the image is transferred right away when it finally arrives (may cause tearing).</li>
			<li>VK_PRESENT_MODE_MAILBOX_KHR: Like the second mode, but instead of blocking the application when the queue is full, the images are replaced with the newer ones. This can be used to implement triple buffering, avoiding tearing with much less latency issues than standard vertical sync that uses double buffering.</li>
		</ul>
	This functions will choose VK_PRESENT_MODE_MAILBOX_KHR if available. Otherwise, it will choose VK_PRESENT_MODE_FIFO_KHR.
*/
VkPresentModeKHR VulkanEnvironment::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	// Choose VK_PRESENT_MODE_MAILBOX_KHR if available
	for (const auto& mode : availablePresentModes)
		if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
			return mode;

	// Otherwise, choose VK_PRESENT_MODE_FIFO_KHR
	return VK_PRESENT_MODE_FIFO_KHR;
}

// (7)
void VulkanEnvironment::createImageViews()
{
	swapChainImageViews.resize(swapChainImages.size());

	for (uint32_t i = 0; i < swapChainImages.size(); i++)
		swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
}

// (8)
/*
* 	Specify subpasses and their attachments.
		- Subpasses: A single render pass can consist of multiple subpasses, which are subsequent rendering operations that depend on the contents of framebuffers in previous passes (example: a sequence of post-processing effects applied one after another). Grouping them into one render pass may give better performance. 
		- Attachment references: Every subpass references one or more of the attachments that we've described.
*/
void VulkanEnvironment::createRenderPass()
{
	// Color attachment
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = swapChainImageFormat;
	colorAttachment.samples = msaaSamples;								// Single color buffer attachment, or many (multisampling).
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;				// What to do with the data (color and depth) in the attachment before rendering: VK_ATTACHMENT_LOAD_OP_ ... LOAD (preserve existing contents of the attachment), CLEAR (clear values to a constant at the start of a new frame), DONT_CARE (existing contents are undefined).
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;				// What to do with the data (color and depth) in the attachment after rendering:  VK_ATTACHMENT_STORE_OP_ ... STORE (rendered contents will be stored in memory and can be read later), DON_CARE (contents of the framebuffer will be undefined after rendering).
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;			// What to do with the stencil data in the attachment before rendering.
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;			// What to do with the stencil data in the attachment after rendering.
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;				// Layout before the render pass. Textures and framebuffers in Vulkan are represented by VkImage objects with a certain pixel format, however the layout of the pixels in memory need to be transitioned to specific layouts suitable for the operation that they're going to be involved in next (read more below).
	if (add_MSAA)
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;	// Layout to automatically transition after the render pass finishes. VK_IMAGE_LAYOUT_ ... UNDEFINED (we don't care what previous layout the image was in, and the contents of the image are not guaranteed to be preserved), COLOR_ATTACHMENT_OPTIMAL (images used as color attachment), PRESENT_SRC_KHR (images to be presented in the swap chain), TRANSFER_DST_OPTIMAL (Images to be used as destination for a memory copy operation).
	else
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;										// Specify which attachment to reference by its index in the attachment descriptions array.
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;	// Specify the layout we would like the attachment to have during a subpass that uses this reference. The layout VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL will give us the best performance.

	// Depth attachment
	VkAttachmentDescription depthAttachment{};
	depthAttachment.format = findDepthFormat();						// Should be same format as the depth image
	depthAttachment.samples = msaaSamples;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;			// Here, we don't care because it will not be used after drawing has finished
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;				// We don't care about previous depth contents
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef{};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	// Multisampling. Multisampled images cannot be presented directly. We first need to resolve them to a regular image (this doesn't apply to the depth buffer, since it is never presented).
	VkAttachmentDescription colorAttachmentResolve{};
	colorAttachmentResolve.format = swapChainImageFormat;
	colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentResolveRef{};
	colorAttachmentResolveRef.attachment = 2;
	colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// Put together all the attachments that your render-pass will contain, in the same order you specified when creating the references (VkAttachmentReference).
	std::array<VkAttachmentDescription, 3> attachments;
	attachments = { colorAttachment, depthAttachment, colorAttachmentResolve };

	// Subpass (we make a subpass containing the previous 3 attachments, by reference)
	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;		// VK_PIPELINE_BIND_POINT_GRAPHICS: This is a graphics subpass
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;					// Attachment for color. The index of the attachment in this array is directly referenced from the fragment shader with the directive "layout(location = 0) out vec4 outColor".
	subpass.pDepthStencilAttachment = &depthAttachmentRef;				// Attachment for depth and stencil data. A subpass can only use a single depth (+ stencil) attachment.
	if (add_MSAA)
		subpass.pResolveAttachments = &colorAttachmentResolveRef;		// Attachments used for multisampling color attachments.
	subpass.pInputAttachments;											// Attachments read from a shader.
	subpass.pPreserveAttachments;										// Attachment not used by this subpass, but for which the data must be preserved.

	// Subpass dependencies
	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;																			// VK_SUBPASS_EXTERNAL: Refers to the implicit subpass before or after the render pass depending on whether it is specified in srcSubpass or dstSubpass.
	dependency.dstSubpass = 0;																								// Index of our subpass. The dstSubpass must always be higher than srcSubpass to prevent cycles in the dependency graph (unless one of the subpasses is VK_SUBPASS_EXTERNAL).
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;	// Stage where to wait (for the swap chain to finish reading from the image).
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;	// Stage where to wait. 
	dependency.srcAccessMask = 0;																							// Operations that wait.
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;			// Operations that wait (they involve the writing of the color attachment).

	// Create the Render pass
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = (add_MSAA ? static_cast<uint32_t>(attachments.size()) : static_cast<uint32_t>(attachments.size()) - 1);
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;			// Array of subpasses
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;		// Array of dependencies.

	if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
		throw std::runtime_error("Failed to create render pass!");
}

void VulkanEnvironment::createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
{
	// Create image objects for letting the shader access the pixel values (better option than setting up the shader to access the pixel values in the buffer). Pixels within an image object are known as texels.
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;				// Kind of coordinate system the texels in the image are going to be addressed: 1D (to store an array of data or gradient...), 2D (textures...), 3D (to store voxel volumes...).
	imageInfo.extent.width = width;						// Number of texels in X									
	imageInfo.extent.height = height;						// Number of texels in Y
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = mipLevels;					// Number of levels (mipmaps)
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;						// Same format for the texels as the pixels in the buffer. This format is widespread, but if it is not supported by the graphics hardware, you should go with the best supported alternative.
	imageInfo.tiling = tiling;						// This cannot be changed later. VK_IMAGE_TILING_ ... LINEAR (texels are laid out in row-major order like our pixels array), OPTIMAL (texels are laid out in an implementation defined order for optimal access).
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;	// VK_IMAGE_LAYOUT_ ... UNDEFINED (not usable by the GPU and the very first transition will discard the texels), PREINITIALIZED (not usable by the GPU, but the first transition will preserve the texels). We choose UNDEFINED because we're first going to transition the image to be a transfer destination and then copy texel data to it from a buffer object. There are few situations where PREINITIALIZED is necessary (example: when we want to use an image as a staging image in combination with the VK_IMAGE_TILING_LINEAR layout). 
	imageInfo.usage = usage;						// The image is going to be used as destination for the buffer copy, so it should be set up as a transfer destination; and we also want to be able to access the image from the shader to color our mesh.
	imageInfo.samples = numSamples;					// For multisampling. Only relevant for images that will be used as attachments.
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;	// The image will only be used by one queue family: the one that supports graphics (and therefore also) transfer operations.
	imageInfo.flags = 0;							// [Optional]  There are some optional flags for images that are related to sparse images (images where only certain regions are actually backed by memory). Example: If you were using a 3D texture for a voxel terrain, then you could use this to avoid allocating memory to store large volumes of "air" values.

	if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS)
		throw std::runtime_error("Failed to create image!");

	// Allocate memory for the image
	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(device, image, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
		throw std::runtime_error("Failed to allocate image memory!");

	vkBindImageMemory(device, image, imageMemory, 0);
}

VkImageView VulkanEnvironment::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels)
{
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;			// 1D, 2D, 3D, or cube map
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = aspectFlags;						// Image's purpose and which part of it should be accessed. Here, our images will be used as color targets without any mipmapping levels or multiple layers. 
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = mipLevels;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;
	viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;	// Default color mapping
	viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

	// Note about stereographic 3D applications: For them, you would create a swap chain with multiple layers, and then create multiple image views for each image (one for left eye and another for right eye).

	VkImageView imageView;
	if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
		throw std::runtime_error("Failed to create texture image view!");

	return imageView;
}

/**
*	Select a format with a depth component that supports usage as depth attachment. We don't need a specific format because we won't be directly accessing the texels from the program. It just needs to have a reasonable accuracy (usually, at least 24 bits). Several formats fit this requirement: VK_FORMAT_ ... D32_SFLOAT (32-bit signed float depth), D32_SFLOAT_S8_UINT (32-bit signed float depth and 8 bit stencil), D24_UNORM_S8_UINT (24-bit float depth and 8 bit stencil).
*/
VkFormat VulkanEnvironment::findDepthFormat()
{
	return findSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

/**
*	Graphic cards can offer different types of memory to allocate from. Each type of memory varies in terms of allowed operations and performance characteristics.
*	@param typeFilter Specifies the bit field of memory types that are suitable.
*	@param properties Specifies the bit field of the desired properties of such memory types.
*	@return Index of a memory type suitable for the buffer that also has all of the properties we need.
*/
uint32_t VulkanEnvironment::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	// Query info about the available types of memory.
	VkPhysicalDeviceMemoryProperties memProperties;				// This struct has 2 arrays memoryTypes and memoryHeaps (this one are distinct memory resources, like dedicated VRAM and swap space in RAM for when VRAM runs out). Right now we'll concern with the type of memory and not the heap it comes from.
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

	// Find a memory type suitable for the buffer, and to .
	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if (typeFilter & (1 << i) &&													// Find the index of a suitable memory type for our buffer by iterating over the memory types and checking if the corresponding bit is set to 1.
			(memProperties.memoryTypes[i].propertyFlags & properties) == properties)	// Check that the memory type has some properties.
			return i;
	}

	throw std::runtime_error("Failed to find suitable memory type!");
}

/**
*	@param candidates
*	@param tiling The support of a format depends on the tiling mode.
*	@param features The support of a format depends on the usage.
*	@return
*/
VkFormat VulkanEnvironment::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
	for (VkFormat format : candidates)
	{
		VkFormatProperties props;												// Contains 3 fields: linearTilingFeatures (linear tiling), optimalTilingFeatures (optimal tiling), bufferFeatures (buffers).
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);	// Query the support of a format

		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
			return format;
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
			return format;
	}

	throw std::runtime_error("Failed to find supported format!");
}

// (11) <<<
void VulkanEnvironment::createCommandPool()
{
	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);	// <<< wrapped method

	// Command buffers are executed by submitting them on one of the device queues we retrieved (graphics queue, presentation queue, etc.). Each command pool can only allocate command buffers that are submitted on a single type of queue.
	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
	poolInfo.flags = 0;	// [Optional]  VK_COMMAND_POOL_CREATE_ ... TRANSIENT_BIT (command buffers are rerecorded with new commands very often - may change memory allocation behavior), RESET_COMMAND_BUFFER_BIT (command buffers can be rerecorded individually, instead of reseting all of them together). Not necessary if we just record the command buffers at the beginning of the program and then execute them many times in the main loop.

	if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
		throw std::runtime_error("Failed to create command pool!");
}

// (12)<<<
void VulkanEnvironment::createColorResources()
{
	VkFormat colorFormat = swapChainImageFormat;

	createImage(
		swapChainExtent.width,
		swapChainExtent.height,
		1,
		msaaSamples,
		colorFormat,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		colorImage,
		colorImageMemory);

	colorImageView = createImageView(colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
}



// (13)<<<
/**
*	A depth image should have the same resolution as the color attachment, defined by the swap chain extent
*/
void VulkanEnvironment::createDepthResources()
{
	VkFormat depthFormat = findDepthFormat();

	createImage(swapChainExtent.width,
		swapChainExtent.height,
		1,
		msaaSamples,
		depthFormat,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		depthImage,
		depthImageMemory);

	depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

	// Explicitly transition the layout of the image to a depth attachment (there is no need of doing this because we take care of this in the render pass, but this is here for completeness).
	transitionImageLayout(depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);
}


/**
*	Submit a pipeline barrier. It specifies when a transition happens: when the pipeline finishes (source) and the next one starts (destination). No command may start before it finishes transitioning. Commands come at the top of the pipeline (first stage), shaders are executed in order, and commands retire at the bottom of the pipeline (last stage), when execution finishes. This barrier will wait for everything to finish and block any work from starting.
*/
void VulkanEnvironment::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels)
{
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	VkImageMemoryBarrier barrier{};			// One of the most common way to perform layout transitions is using an image memory barrier. A pipeline barrier like that is generally used to synchronize access to resources, like ensuring that a write to a buffer completes before reading from it, but it can also be used to transition image layouts and transfer queue family ownership when VK_SHARING_MODE_EXCLUSIVE is used. There is an equivalent buffer memory barrier to do this for buffers.
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;								// Specify layout transition (it's possible to use VK_IMAGE_LAYOUTR_UNDEFINED if you don't care about the existing contents of the image).
	barrier.newLayout = newLayout;								// Specify layout transition
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;					// If you are using the barrier to transfer queue family ownership, then these two fields ...
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;					// ... should be the indices of the queue families; otherwise, set this to VK_QUEUE_FAMILY_IGNORED.
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;				// subresourceRange specifies the part of the image that is affected.
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = mipLevels;								// If the image has no mipmapping levels, then levelCount = 1
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;										// If the image is not an array, then layerCount = 1

	if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		if (hasStencilComponent(format))
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
	}
	else
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

	// Set the access masks and pipeline stages based on the layouts in the transition. There are 2 transitions we need to handle:
	//		- From somewhere (undefined) to transfer destination: Transfer writes that don't need to wait on anything.
	//		- From transfer destinations to shader reading: Shader reads should wait on transfer writes (specifically the shader reads in the fragment shader, because that's where we're going to use the texture).
	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;					// The depth buffer is read from  in the VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT stage to perform depth tests 
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;		// The depth buffer is written to in the VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT  stage when a new fragment is drawn
	}
	else
		throw std::invalid_argument("Unsupported layout transition!");

	// Submit a pipeline barrier
	vkCmdPipelineBarrier(commandBuffer,
		sourceStage,		// Specify in which pipeline stage the operations occur that should happen before the barrier 
		destinationStage,	// Specify the pipeline stage in which operations will wait on the barrier
		0,					// This is either 0 (nothing) or VK_DEPENDENCY_BY_REGION_BIT (turns the barrier into a per-region condition, which means that the implementation is allowed to already begin reading from the parts of a resource that were written so far, for example).
		0, nullptr,			// Array of pipeline barriers of type memory barriers
		0, nullptr,			// Array of pipeline barriers of type buffer memory barriers
		1, &barrier);		// Array of pipeline barriers of type image memory barriers

	endSingleTimeCommands(commandBuffer);

	/*
		Note:
		The pipeline stages that you are allowed to specify before and after the barrier depend on how you use the resource before and after the barrier.
		Allowed values: https://www.khronos.org/registry/vulkan/specs/1.0/html/vkspec.html#synchronization-access-types-supported
		Example: If you're going to read from a uniform after the barrier, you would specify a usage of VK_ACCESS_UNIFORM_READ_BIT and the earliest shader
		that will read from the uniform as pipeline stage (for example, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT).
		----------
		Transfer writes must occur in the pipeline transfer stage. Since the writes don't have to wait on anything, you may specify an empty access mask and the
		earliest possible pipeline stage VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT for the pre-barrier operations. Note that VK_PIPELINE_STAGE_TRANSFER_BIT is not a real
		stage within the graphics and compute pipelines, but a pseudo-stage where transfers happen (more about pseudo-stages:
		https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkPipelineStageFlagBits.html).

		The image will be written in the same pipeline stage and subsequently read by the fragment shader, which is why we specify shader reading access in the
		fragment shader pipeline stage.

		If we need to do more transitions in the future, then we'll extend the function.
		Note that command buffer submission results in implicit VK_ACCESS_HOST_WRITE_BIT synchronization at the beginning. Since this function (transitionImageLayout)
		executes a command buffer with only a single command, you could use this implicit synchronization and set srcAccessMask to 0 if you ever needed a
		VK_ACCESS_HOST_WRITE_BIT dependency in a layout transition. It's up to you if you want to be explicit about it or not, but I prefer not to rely on these
		OpenGL-like "hidden" operations.

		VK_IMAGE_LAYOUT_GENERAL: Special type of image layout that supports all operations, although it doesn't necessarily offer the best performance for any
		operation. It is required for some special cases (using an image as both input and output, reading an image after it has left the preinitialized layout, etc.).

		All of the helper functions that submit commands so far have been set up to execute synchronously by waiting for the queue to become idle. For practical
		applications it is recommended to combine these operations in a single command buffer and execute them asynchronously for higher throughput (especially the
		transitions and copy in the createTextureImage function). Exercise: Try to experiment with this by creating a setupCommandBuffer that the helper functions
		record commands into, and add a flushSetupCommands to execute the commands that have been recorded so far. It's best to do this after the texture mapping works
		to check if the texture resources are still set up correctly.
	*/
}

/// Tells if the chosen depth format contains a stencil component.
bool VulkanEnvironment::hasStencilComponent(VkFormat format)
{
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

/**
*	Allocate the command buffer and start recording it.
*	@return Returns a Vulkan command buffer object.
*/
VkCommandBuffer VulkanEnvironment::beginSingleTimeCommands()
{
	// Allocate the command buffer.
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

	// Start recording the command buffer.
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;		// Good practice to tell the driver about our intent <<< (see createCommandBuffers > beginInfo.flags)

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

/**
*	Stop recording a command buffer and submit it to the queue.
*/
void VulkanEnvironment::endSingleTimeCommands(VkCommandBuffer commandBuffer)
{
	vkEndCommandBuffer(commandBuffer);		// Stop recording (this command buffer only contains the copy command, so we can stop recording now).

	// Execute the command buffer (only contains the copy command) to complete the transfer of buffers.
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(graphicsQueue);			// Wait to this transfer to complete. Two ways to do this: vkQueueWaitIdle (Wait for the transfer queue to become idle. Execute one transfer at a time) or vkWaitForFences (Use a fence. Allows to schedule multiple transfers simultaneously and wait for all of them complete. It may give the driver more opportunities to optimize).

	// Clean up the command buffer used.
	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

// (14)
/// Create the swap chain framebuffers (by attaching to each of them the MSAA image, depth image, and swap chain image)
void VulkanEnvironment::createFramebuffers()
{
	swapChainFramebuffers.resize(swapChainImageViews.size());

	for (size_t i = 0; i < swapChainImageViews.size(); i++)
	{
		std::array<VkImageView, 3> attachments;
		if (add_MSAA)
		{
			attachments = {
				colorImageView,				// Multisampled color buffer
				depthImageView,				// Color attachment differs for every swap chain image, but the same depth image can be used by all of them because only a single subpass is running at the same time due to our semaphores.
				swapChainImageViews[i]
			};
		}
		else
		{
			attachments = {
				swapChainImageViews[i],
				depthImageView
			};
		}

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;									// A framebuffer can only be used with the render passes that it is compatible with, which roughly means that they use the same number and type of attachments.
		framebufferInfo.attachmentCount = (add_MSAA ? static_cast<uint32_t>(attachments.size()) : static_cast<uint32_t>(attachments.size()) - 1);
		framebufferInfo.pAttachments = attachments.data();							// Objects that should be bound to the respective attachment descriptions in the render pass pAttachment array.
		framebufferInfo.width = swapChainExtent.width;
		framebufferInfo.height = swapChainExtent.height;
		framebufferInfo.layers = 1;											// Number of layers in image arrays. If your swap chain images are single images, then layers = 1.

		if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS)
			throw std::runtime_error("Failed to create framebuffer!");
	}
}

void VulkanEnvironment::recreateSwapChain()
{
	createSwapChain();					// Recreate the swap chain.
	createImageViews();					// Recreate image views because they are based directly on the swap chain images.
	createRenderPass();					// Recreate render pass because it depends on the format of the swap chain images.

	if (add_MSAA)
		createColorResources();			// Recreate MSAA resources
	createDepthResources();				// Recreate depth resources
	createFramebuffers();				// Framebuffers directly depend on the swap chain images.
}

void VulkanEnvironment::cleanupSwapChain()
{
	// MSAA buffer
	if (add_MSAA) {
		vkDestroyImageView(device, colorImageView, nullptr);				// MSAA buffer		(VkImageView)
		vkDestroyImage(device, colorImage, nullptr);						// MSAA buffer		(VkImage)
		vkFreeMemory(device, colorImageMemory, nullptr);					// MSAA buffer		(VkDeviceMemory)
	}

	// Depth buffer
	vkDestroyImageView(device, depthImageView, nullptr);					// Depth buffer		(VkImageView)
	vkDestroyImage(device, depthImage, nullptr);							// Depth buffer		(VkImage)
	vkFreeMemory(device, depthImageMemory, nullptr);						// Depth buffer		(VkDeviceMemory)

	// Framebuffer
	for (auto framebuffer : swapChainFramebuffers)
		vkDestroyFramebuffer(device, framebuffer, nullptr);

	// Command buffers 
	//vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());

	// Graphics pipeline
	//vkDestroyPipeline(device, graphicsPipeline, nullptr);
	//vkDestroyPipelineLayout(device, pipelineLayout, nullptr);

	// Render pass
	vkDestroyRenderPass(device, renderPass, nullptr);

	// Swap chain image views
	for (auto imageView : swapChainImageViews)
		vkDestroyImageView(device, imageView, nullptr);

	// Swap chain
	vkDestroySwapchainKHR(device, swapChain, nullptr);

	// Uniform buffers & memory
	//for (size_t i = 0; i < swapChainImages.size(); i++) {
	//	vkDestroyBuffer(device, uniformBuffers[i], nullptr);
	//	vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
	//}

	// Descriptor pool
	//vkDestroyDescriptorPool(device, descriptorPool, nullptr);
}

/**
 * Cleans up the VkDebugUtilsMessengerEXT object.
 * @param instance Vulkan instance (the debug messenger is specific to our Vulkan instance and its layers)
 * @param debugMessenger Debug messenger object
 * @param pAllocator Optional allocator callback
 */
void VulkanEnvironment::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
	// Similarly to vkCreateDebugUtilsMessengerEXT, the extension function needs to be explicitly loaded.
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr)
		func(instance, debugMessenger, pAllocator);
}

void VulkanEnvironment::cleanup()
{
	vkDestroyCommandPool(device, commandPool, nullptr);						// Command pool
	vkDestroyDevice(device, nullptr);										// Logical device & device queues

	if (enableValidationLayers)												// Debug messenger
		DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);

	vkDestroySurfaceKHR(instance, surface, nullptr);						// Surface KHR
	vkDestroyInstance(instance, nullptr);									// Instance
	glfwDestroyWindow(window);												// GLFW window
	glfwTerminate();														// GLFW
}

// Independent methods ----------------------------------------------

VkDeviceSize VulkanEnvironment::getMinUniformBufferOffsetAlignment()
{
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
	return deviceProperties.limits.minUniformBufferOffsetAlignment;
}

