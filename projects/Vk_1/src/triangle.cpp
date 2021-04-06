
#include <iostream>
#include <stdexcept>
#include <cstdlib>				// EXIT_SUCCESS, EXIT_FAILURE
//#include <memory>				// std::unique_ptr, std::shared_ptr (used instead of RAII)
#include <cstring>				// strcmp()
#include <map>					// std::multimap<key, value>
#include <set>					// std::set<uint32_t>

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
	}
}

void HelloTriangleApp::cleanup()
{
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

	// Find out whether the device is suitable
	switch (mode)
	{
		// Check Vulkan support:
	    case 1:
	        return indices.isComplete();
	        break;
		// Check for dedicated GPU supporting geometry shaders:
	    case 2:
	    	return indices.isComplete() &&
	    		   deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
				   deviceFeatures.geometryShader;
	    	break;
		// Give a score to the device (rate the device):
	    case 3:
	    	{
	    		int score = 0;
	    		// Discrete GPUs have better performance:
	    		if(deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) score += 1000;
	    		// Maximum size of textures:
	    		score += deviceProperties.limits.maxImageDimension2D;
	    		// Applications cannot function without geometry shaders:
	    		if(!deviceFeatures.geometryShader) return 0;
	    		// There should exist the queue families we want:
	    		if(!indices.isComplete()) return 0;
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
 * @return
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
	createInfo.enabledExtensionCount	= 0;
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
