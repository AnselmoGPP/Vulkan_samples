
#define _DEBUG		// NDEBUG, _DEBUG
#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif

const uint32_t WIDTH  = 800;
const uint32_t HEIGHT = 600;

const std::vector<const char*> requiredValidationLayers = { "VK_LAYER_KHRONOS_validation" };

