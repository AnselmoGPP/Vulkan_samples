#ifndef TRIANGLE_HPP
#define TRIANGLE_HPP

#include <vector>
#include <optional>				// std::optional<uint32_t> (Wrapper that contains no value until you assign something to it. Contains member has_value())
#include <array>

//#include <vulkan/vulkan.h>				// From LunarG SDK. Used for off-screen rendering
#define GLFW_INCLUDE_VULKAN					// Makes GLFW load the Vulkan header with it
#include "GLFW/glfw3.h"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE			// GLM uses OpenGL depth range [-1.0, 1.0]. This macro forces GLM to use Vulkan range [0.0, 1.0].
#define GLM_ENABLE_EXPERIMENTAL				// Required for using std::hash functions for the GLM types (since gtx folder contains experimental extensions)
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>		// Generate transformations matrices with glm::rotate (model), glm::lookAt (view), glm::perspective (projection).
#include <glm/gtx/hash.hpp>


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

/// Structure for storing vector indices of the queue families we want. Note that graphicsFamily and presentFamily could refer to the same queue family, but we included them separately because sometimes they are in different queue families.
struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily;		///< Queue family capable of computer graphics.
	std::optional<uint32_t> presentFamily;		///< Queue family capable of presenting to our window surface.
	bool isComplete();							///< Checks whether all members have value.
};

/// Though a swap chain may be available, it may not be compatible with our window surface, so we need to query for some details and check them. This struct will contain these details.
struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR		capabilities;		// Basic surface capabilities: min/max number of images in swap chain, and min/max width/height of images.
	std::vector<VkSurfaceFormatKHR> formats;			// Surface formats: pixel format, color space.
	std::vector<VkPresentModeKHR>	presentModes;		// Available presentation modes
};

/// Structure containing details about the swap chain that must be checked.
struct SwapChainSupportDetails;

struct Vertex 
{
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;

	static VkVertexInputBindingDescription					getBindingDescription();	///< Describes at which rate to load data from memory throughout the vertices (number of bytes between data entries and whether to move to the next data entry after each vertex or after each instance).
	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions();	///< Describe how to extract a vertex attribute from a chunk of vertex data originiating from a binding description. Two attributes here: position and color.
	bool operator==(const Vertex& other) const;											///< Overriding of operator ==. Required for doing comparisons in loadModel().
};

/// Hash function for Vertex. Implemented by specifying a template specialization for std::hash<T> (https://en.cppreference.com/w/cpp/utility/hash). Required for doing comparisons in loadModel().
template<> struct std::hash<Vertex> {
	size_t operator()(Vertex const& vertex) const;
};

// Model-View-Projection matrix as a UBO (Uniform buffer object) (https://www.opengl-tutorial.org/beginners-tutorials/tutorial-3-matrices/)
struct UniformBufferObject {
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};


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
		void createInstance();					///< Describe application, select extensions and validation layers, create Vulkan instance (stores application state).
		void setupDebugMessenger();				///< Specify the details about the messenger and its callback, and create the debug messenger.
		void createSurface();					///< Create a window surface (interface for interacting with the window system)
		void pickPhysicalDevice();				///< Look for and select a graphics card in the system that supports the features we need.
		void createLogicalDevice();				///< Set up a logical device (describes the features we want to use) to interface with the physical device.
		void createSwapChain();					///< Set up and create the swap chain.
		void createImageViews();				///< Creates a basic image view for every image in the swap chain so that we can use them as color targets later on.
		void createRenderPass();				///< Tells Vulkan the framebuffer attachments that will be used while rendering.
		void createDescriptorSetLayout();		///< 
		void createGraphicsPipeline();			///< Create the graphics pipeline.
		void createCommandPool();				///< Commands in Vulkan (drawing, memory transfers, etc.) are not executed directly using function calls, you have to record all of the operations you want to perform in command buffer objects. After setting up the drawing commands, just tell Vulkan to execute them in the main loop.
		void createColorResources();			///< Create resources needed for MSAA (MultiSampling AntiAliasing). Create a multisampled color buffer.
		void createDepthResources();			///< Create depth buffer.
		void createFramebuffers();				///< Create framebuffers.
		void createTextureImage();				///< Load an image and upload it into a Vulkan object.
		void createTextureImageView();			///< Create an image view for the texture (images are accessed through image views rather than directly).
		void createTextureSampler();			///< Create a sampler for the textures (it applies filtering and transformations).
		void loadModel(const char *obj_file);	///< Populate the vertices and indices members with the vertex data from the mesh (OBJ file).
		void createVertexBuffer();				///< Vertex buffer creation.
		void createIndexBuffer();				///< Index buffer creation
		void createUniformBuffers();			///< Uniform buffer creation (type of descriptors that can be bound).
		void createDescriptorPool();			///< Descriptor pool creation (a descriptor set for each VkBuffer resource to bind it to the uniform buffer descriptor).
		void createDescriptorSets();			///< Descriptor sets creation.
		void createCommandBuffers();			///< Allocates command buffers and record drawing commands in them.
		void createSyncObjects();				///< Create semaphores and fences for synchronizing the events occuring in each frame (drawFrame()).
	void mainLoop();
		void drawFrame();						///< Acquire image from swap chain, execute command buffer with that image as attachment in the framebuffer, and return the image to the swap chain for presentation.
	void cleanup();
		void cleanupSwapChain();

	GLFWwindow*					 window;							///< Opaque window object.

	VkInstance					 instance;							///< Opaque handle to an instance object. There is no global state in Vulkan and all per-application state is stored here.
	VkDebugUtilsMessengerEXT	 debugMessenger;					///< Opaque handle to a debug messenger object (the debug callback is part of it).
	VkSurfaceKHR				 surface;							///< Opaque handle to a surface object (abstract type of surface to present rendered images to)

	VkPhysicalDevice			 physicalDevice = VK_NULL_HANDLE;	///< Opaque handle to a physical device object.
	VkSampleCountFlagBits		 msaaSamples	= VK_SAMPLE_COUNT_1_BIT;///< Number of samples for MSAA (MultiSampling AntiAliasing)
	VkDevice					 device;							///< Opaque handle to a device object.

	VkQueue						 graphicsQueue;						///< Opaque handle to a queue object (computer graphics).
	VkQueue						 presentQueue;						///< Opaque handle to a queue object (presentation to window surface).

	VkSwapchainKHR				 swapChain;							///< Swap chain object.
	std::vector<VkImage>		 swapChainImages;					///< List. Opaque handle to an image object.
	VkFormat					 swapChainImageFormat;				///< Swap chain format.
	VkExtent2D					 swapChainExtent;					///< Swap chain extent.
	std::vector<VkImageView>	 swapChainImageViews;				///< List. Opaque handle to an image view object. It allows to use VkImage in the render pipeline. It's a view into an image; it describes how to access the image and which part of the image to access.
	std::vector<VkFramebuffer>	 swapChainFramebuffers;				///< List. Opaque handle to a framebuffer object.

	VkRenderPass				 renderPass;						///< Opaque handle to a render pass object.
	VkDescriptorSetLayout		 descriptorSetLayout;				///< Opaque handle to a descriptor set layout object (combines all of the descriptor bindings).
	VkPipelineLayout			 pipelineLayout;					///< Pipeline layout. Allows to use uniform values in shaders (globals similar to dynamic state variables that can be changed at drawing at drawing time to alter the behavior of your shaders without having to recreate them).
	VkPipeline					 graphicsPipeline;					///< Opaque handle to a pipeline object.

	VkCommandPool				 commandPool;						///< Opaque handle to a command pool object. It manages the memory that is used to store the buffers, and command buffers are allocated from them. 

	VkImage						 colorImage;						///< For MSAA
	VkDeviceMemory				 colorImageMemory;					///< For MSAA
	VkImageView					 colorImageView;					///< For MSAA

	VkImage						 depthImage;						///< Depth buffer (image object).
	VkDeviceMemory				 depthImageMemory;					///< Depth buffer memory (memory object).
	VkImageView					 depthImageView;					///< Depth buffer image view (images are accessed through image views rather than directly).

	uint32_t					 mipLevels;							///< Number of levels (mipmaps)
	VkImage						 textureImage;						///< Opaque handle to an image object.
	VkDeviceMemory				 textureImageMemory;				///< Opaque handle to a device memory object.
	VkImageView					 textureImageView;					///< Image view for the texture image (images are accessed through image views rather than directly).
	VkSampler					 textureSampler;					///< Opaque handle to a sampler object (it applies filtering and transformations to a texture). It is a distinct object that provides an interface to extract colors from a texture. It can be applied to any image you want (1D, 2D or 3D).

	std::vector<Vertex>			 vertices;							///< Vertices of our model
	std::vector<uint32_t>		 indices;							///< Indices of our model
	VkBuffer					 vertexBuffer;						///< Opaque handle to a buffer object (here, vertex buffer).
	VkDeviceMemory				 vertexBufferMemory;				///< Opaque handle to a device memory object (here, memory for the vertex buffer).
	VkBuffer					 indexBuffer;						///< Opaque handle to a buffer object (here, index buffer).
	VkDeviceMemory				 indexBufferMemory;					///< Opaque handle to a device memory object (here, memory for the index buffer).

	std::vector<VkBuffer>		 uniformBuffers;					///< Opaque handle to a buffer object (here, uniform buffer).
	std::vector<VkDeviceMemory>	 uniformBuffersMemory;				///< Opaque handle to a device memory object (here, memory for the uniform buffer).

	VkDescriptorPool			 descriptorPool;					///< Opaque handle to a descriptor pool object.
	std::vector<VkDescriptorSet> descriptorSets;					///< List. Opaque handle to a descriptor set object.

	std::vector<VkCommandBuffer> commandBuffers;					///< List. Opaque handle to command buffer object

	std::vector<VkSemaphore>	 imageAvailableSemaphores;			///< Signals that an image has been acquired and is ready for rendering. Each frame has a semaphore for concurrent processing. Allows multiple frames to be in-flight while still bounding the amount of work that piles up.
	std::vector<VkSemaphore>	 renderFinishedSemaphores;			///< Signals that rendering has finished and presentation can happen.Each frame has a semaphore for concurrent processing. Allows multiple frames to be in-flight while still bounding the amount of work that piles up.
	std::vector<VkFence>		 inFlightFences;					///< Similar to semaphores, but fences actually wait in our own code. Used to perform CPU-GPU synchronization.
	std::vector<VkFence>		 imagesInFlight;					///< Maps frames in flight by their fences. Tracks for each swap chain image if a frame in flight is currently using it.
	size_t						 currentFrame = 0;					///< Frame to process next.

	bool						 framebufferResized	= false;		///< Many drivers/platforms trigger VK_ERROR_OUT_OF_DATE_KHR after window resize, but it's not guaranteed. This variable handles resizes explicitly.

	// Helper functions --------------------------------------------------

	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
	/// The window surface may change, making the swap chain no longer compatible with it (example: window resizing). Here, we catch these events and recreate the swap chain.
	void recreateSwapChain();

	/// Callback for handling ourselves the validation layer's debug messages and decide which kind of messages to see.
	static VKAPI_ATTR VkBool32 VKAPI_CALL	debugCallback					(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
	/// Specify the details about the messenger and its callback.
	void									populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

	bool checkValidationLayerSupport(const std::vector<const char*> &requiredLayers);
	bool checkExtensionSupport(const char* const* requiredExtensions, uint32_t reqExtCount);
	std::vector<const char*> getRequiredExtensions();
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);

	// Evaluate a device and check if it is suitable for the operations we want to perform.
	int isDeviceSuitable(VkPhysicalDevice device, const int mode);

	QueueFamilyIndices	findQueueFamilies	(VkPhysicalDevice device);
	/// Finds the right type of memory to use, depending upon the requirements of the buffer and our own application requiremnts.
	uint32_t			findMemoryType		(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	/// Take a list of candidate formats in order from most desirable to least desirable, and checks which is the first one that is supported.
	VkFormat			findSupportedFormat	(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	/// Find the right format for a depth image.
	VkFormat			findDepthFormat		();
	/// Tells if the chosen depth format contains a stencil component.
	bool				hasStencilComponent	(VkFormat format);

	SwapChainSupportDetails querySwapChainSupport	(VkPhysicalDevice device);
	/// Chooses the surface format (color depth) for the swap chain.
	VkSurfaceFormatKHR		chooseSwapSurfaceFormat	(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	/// Chooses the presentation mode (conditions for "swapping" images to the screen) for the swap chain.
	VkPresentModeKHR		chooseSwapPresentMode	(const std::vector<VkPresentModeKHR>& availablePresentModes);
	/// Chooses the swap extent (resolution of images in swap chain) for the swap chain.
	VkExtent2D				chooseSwapExtent		(const VkSurfaceCapabilitiesKHR& capabilities);

	/// Read all of the bytes from the specified file and return them in a byte array managed by a std::vector.
	static std::vector<char>	readFile			(const std::string& filename);
	/// Take a buffer with the bytecode as parameter and create a VkShaderModule from it.
	VkShaderModule				createShaderModule	(const std::vector<char>& code);

	/// Copy the contents from one buffer to another.
	void		copyBuffer			(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	void		copyBufferToImage	(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	/// Update Uniform buffer. It will generate a new transformation every frame to make the geometry spin around.
	void		updateUniformBuffer	(uint32_t currentImage);
	/// Helper function for creating a buffer (VkBuffer and VkDeviceMemory).
	void		createBuffer		(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void		createImage			(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	VkImageView createImageView		(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);

	VkCommandBuffer beginSingleTimeCommands	();
	void			endSingleTimeCommands	(VkCommandBuffer commandBuffer);

	/// Handles layout transitions.
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);

	/// Generate mipmaps
	void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);

	/// Get the maximum number of samples (for MSAA) according to the physical device.
	VkSampleCountFlagBits HelloTriangleApp::getMaxUsableSampleCount(bool getMinimum = false);
};

#endif