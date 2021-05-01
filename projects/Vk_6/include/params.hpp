#ifndef PARAMS_HPP
#define PARAMS_HPP

#define DEBUG		// Standards: NDEBUG, _DEBUG

#ifdef RELEASE
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif

const uint32_t WIDTH  = 800;
const uint32_t HEIGHT = 600;

const std::vector<const char*> requiredValidationLayers = { 
	"VK_LAYER_KHRONOS_validation" 
};

const std::vector<const char*> requiredDeviceExtensions = { 
	VK_KHR_SWAPCHAIN_EXTENSION_NAME				// Swap chain: Queue of images that are waiting to be presented to the screen. Our application will acquire such an image to draw to it, and then return it to the queue. Its general purpose is to synchronize the presentation of images with the refresh rate of the screen.
};

#if defined(__unix__)
	const std::string shaders_dir	("../../../projects/Vk_6/shaders/");
	const std::string textures_dir	("../../../textures/");
#elif _WIN64 || _WIN32
	const std::string SHADERS_PATH	("../../../projects/Vk_6/shaders/");
	const std::string MODELS_PATH	("../../../models/");
	const std::string TEXTURES_PATH	("../../../textures/");
#endif

const int MAX_FRAMES_IN_FLIGHT = 2;		// How many frames should be processed concurrently.

#endif