#ifndef TRIANGLE_HPP
#define TRIANGLE_HPP
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
					-checkDeviceExtensionSupport
					-querySwapChainSupport
					-isComplete
			-createLogicalDevice
				-findQueueFamilies
			-createSwapChain
				-querySwapChainSupport
				-chooseSwapSurfaceFormat
				-chooseSwapPresentMode
				-chooseSwapExtent
				-findQueueFamilies
			-createImageViews
				-createImageView
			-createRenderPass
			-createDescriptorSetLayout
			-createGraphicsPipeline
				-readFile
				-createShaderModule
			-createCommandPool
			-createDepthResources
				-findDepthFormat
					-findSupportedFormat
				-createImage
				-createImageView
				-transitionImageLayout
					-beginSingleTimeCommands
					-hasStencilComponent
					-endSingleTimeCommands
			-createFramebuffers
			-createTextureImage
				-createBuffer
					-findMemoryType
				-createImage
				-transitionImageLayout
					-beginSingleTimeCommands
					-hasStencilComponent
					-endSingleTimeCommands
				-copyBufferToImage
					-beginSingleTimeCommands
					-endSingleTimeCommands
				-generateMipmaps
			-createTextureImageView
					-createImageView
			-createTextureSampler
			-loadModel
			-createVertexBuffer
				-createBuffer
					-findMemoryType
				-copyBuffer
					-beginSingleTimeCommands
					-endSingleTimeCommands
			-createIndexBuffer
				-createBuffer
					-findMemoryType
				-copyBuffer
					-beginSingleTimeCommands
					-endSingleTimeCommands
			-createUniformBuffers
			-createDescriptorPool
			-createDescriptorSets
			-createCommandBuffers
			-createSyncObjects
		-mainLoop
			-drawFrame
				-updateUniformBuffer
				-recreateSwapChain (if required)
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
	
	Attachments:
	In our case, we have a single attachment: the color attachment. However, the image that we have to use for the attachment depends on which image the 
	swap chain returns when we retrieve one for presentation, which means that we have to create a framebuffer for all of the images in the swap chain 
	and use the one that corresponds to the retrieved image at drawing time.

	Vertex buffer:
	The most optimal memory has the VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT flag and is usually not accessible by the CPU on dedicated graphics cards. Previously, we were using VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT and VK_MEMORY_PROPERTY_HOST_COHERENT_BIT.
	We created 2 vertex buffers. 
		- Staging buffer: in CPU accessible memory to upload the data from the vertex array to
		- Final vertex buffer: in device local memory. 
	We'll then use a buffer copy command to move the data from the staging buffer to the actual vertex buffer.

	Buffer copy command:
	Requires a queue family that supports transfer operations, indicated using VK_QUEUE_TRANSFER_BIT. Any queue family with VK_QUEUE_GRAPHICS_BIT or VK_QUEUE_COMPUTE_BIT 
	capabilities already implicitly support VK_QUEUE_TRANSFER_BIT operations. The implementation is not required to explicitly list it in queueFlags in those cases.
 
	Challenge:
	You can try to use a different queue family specifically for transfer operations. It will require the following modifications to your program:
		- Modify QueueFamilyIndices and findQueueFamilies to explicitly look for a queue family with the VK_QUEUE_TRANSFER_BIT bit, but not the VK_QUEUE_GRAPHICS_BIT.
		- Modify createLogicalDevice to request a handle to the transfer queue.
		- Create a second command pool for command buffers that are submitted on the transfer queue family.
	Change the sharingMode of resources to be VK_SHARING_MODE_CONCURRENT and specify both the graphics and transfer queue families
	Submit any transfer commands like vkCmdCopyBuffer (which we'll be using in this chapter) to the transfer queue instead of the graphics queue
	It's a bit of work, but it'll teach you a lot about how resources are shared between queue families.
 
	createBuffer:
	In real world applications, you're not supposed to call vkAllocateMemory for every individual buffer. The maximum number of simultaneous memory allocations 
	is limited by the maxMemoryAllocationCount physical device limit (<4096). Right way to allocate memory for a large number of objects at the same time: Create 
	a custom allocator that splits up a single allocation among many different objects by using the offset parameters that we've seen in many functions. You can 
	either implement such an allocator yourself, or use the VulkanMemoryAllocator library provided by the GPUOpen initiative.

	Index buffer:
	You can only have a single index buffer; it's not possible to use different indices for each vertex attribute.

	Memory allocation:
	We mentioned that you should allocate multiple resources like buffers from a single memory allocation, but in fact you should go a step further. 
	Driver developers recommend that you also store multiple buffers (https://developer.nvidia.com/vulkan-memory-management), like the vertex and index buffer, 
	into a single VkBuffer and use offsets in commands like vkCmdBindVertexBuffers. The advantage is that your data is more cache friendly in that case, because 
	it's closer together. It is even possible to reuse the same chunk of memory for multiple resources if they are not used during the same render operations, 
	provided that their data is refreshed, of course. This is known as aliasing and some Vulkan functions have explicit flags to specify that you want to do this.

	Descriptor: Way for shaders to freely access resources like buffers and images (example: transformation matrices like MVP). Using descriptors consists of 3 parts:
		1 - Specify a descriptor layout during pipeline creation. Descriptor layout: Specifies the types of resources that are going to be accessed by the pipeline (just like a render pass specifies the types of attachments that will be accessed).
		2 - Allocate a descriptor set from a descriptor pool. Descriptor set: Specifies the actual buffer or image resources that will be bound to the descriptors (just like a framebuffer specifies the actual image views to bind to render pass attachments).
		3 - Bind the descriptor set during rendering for the drawing commands (just like the vertex buffers and framebuffer).
	 There are different types of descriptors. Example: UBO (Uniform buffer objects), ...
	
	Uniform buffers:
	Before specifying the buffer that contains the UBO data for the shader, we need to create this buffer first. We will copy new data to the uniform buffer every 
	frame, so it doesn't really make any sense to have a staging buffer (extra overhead that may degrade performance). We should have multiple buffers, because 
	multiple frames may be in flight at the same time and we don't want to update the buffer in preparation of the next frame while a previous one is still reading 
	from it! We could either have a uniform buffer per frame or per swap chain image. However, since we need to refer to the uniform buffer from the command buffer 
	that we have per swap chain image, it makes the most sense to also have a uniform buffer per swap chain image.

	Alignment requirements:
	We know that data in the C++ structure should match with the uniform definition in the shader. But, Vulkan also expect the data in the structure to be aligned
	in memory in a specific way. Example:
		- Scalars -> 4 bytes
		- vec2    -> 8 bytes
		- vec3 or vec4     -> 16 bytes
		- Nested structure -> base alignment of its members rounded up to a multiple of 16
		- mat4    -> 16 bytes
		- Specification: https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/chap14.html#interfaces-resources-layout
	The best way to solve this is with the standard specifier alignas() (https://vulkan-tutorial.com/Uniform_buffers/Descriptor_pool_and_sets).

	Samplers:
	Shaders can read texels directly from images, but that's not very common when used as textures. Textures are usually accessed through samplers.
	Samplers automatically apply filtering and transformations to compute the final color retrieved. This is helpful to deal with:
		- Oversampling: Textures are mapped to geometry	with more fragments than texels will look pixelated. Solved applying filtering (bilinear, trilinear, etc.).
		- Undersampling: Textures are mapped to geometry with more texels than fragments. This leads to artifacts when sampling high frequency patterns like a
						 checkerboard texture at a sharp angle). Solved with anisotropic filtering (x2, x16, etc.).
		- Transformations: Determines what happens when you try to read texels outside the image through its "addressing mode". Some possibilities: Repeat, mirrored
						   repeat, clamp to edge, clamp to border.

	VK_LAYER_KHRONO_validation can also warn us about best practise problems if we enable it (https://vulkan.lunarg.com/doc/view/1.1.126.0/windows/best_practices.html).

	Mipmaps: 
	Precalculated, downscaled versions of an image. Each new image is half the width & height of the previous one. Mipmaps are like a Level of Detail (LOD).
	Objects far away from the camera will sample their textures from the smaller mip images. This increases rendering speed and avoids artifacts (like Moir? patterns).
	Each mip image is stored in different mip levels of a VkImage. Mip level 0 is the original image, and levels after it are the "mip chain".
	The number of mip levels is specified when the VkImage is created.
 */
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

struct Vertex {
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
		void createInstance();				///< Describe application, select extensions and validation layers, create Vulkan instance (stores application state).
		void setupDebugMessenger();			///< Specify the details about the messenger and its callback, and create the debug messenger.
		void createSurface();				///< Create a window surface (interface for interacting with the window system)
		void pickPhysicalDevice();			///< Look for and select a graphics card in the system that supports the features we need.
		void createLogicalDevice();			///< Set up a logical device (describes the features we want to use) to interface with the physical device.
		void createSwapChain();				///< Set up and create the swap chain.
		void createImageViews();			///< Creates a basic image view for every image in the swap chain so that we can use them as color targets later on.
		void createRenderPass();			///< Tells Vulkan the framebuffer attachments that will be used while rendering.
		void createDescriptorSetLayout();	///< 
		void createGraphicsPipeline();		///< Create the graphics pipeline.
		void createDepthResources();		///< Create depth buffer.
		void createFramebuffers();			///< Create framebuffers.
		void createCommandPool();			///< Commands in Vulkan (drawing, memory transfers, etc.) are not executed directly using function calls, you have to record all of the operations you want to perform in command buffer objects. After setting up the drawing commands, just tell Vulkan to execute them in the main loop.
		void createTextureImage();			///< Load an image and upload it into a Vulkan object.
		void createTextureImageView();		///< Create an image view for the texture (images are accessed through image views rather than directly).
		void createTextureSampler();		///< Create a sampler for the textures (it applies filtering and transformations).
		void loadModel(const char *obj_file);///< Populate the vertices and indices members with the vertex data from the mesh (OBJ file).
		void createVertexBuffer();			///< Vertex buffer creation.
		void createIndexBuffer();			///< Index buffer creation
		void createUniformBuffers();		///< Uniform buffer creation (type of descriptors that can be bound).
		void createDescriptorPool();		///< Descriptor pool creation (a descriptor set for each VkBuffer resource to bind it to the uniform buffer descriptor).
		void createDescriptorSets();		///< Descriptor sets creation.
		void createCommandBuffers();		///< Allocates command buffers and record drawing commands in them.
		void createSyncObjects();			///< Create semaphores and fences for synchronizing the events occuring in each frame (drawFrame()).
	void mainLoop();
		void drawFrame();					///< Acquire image from swap chain, execute command buffer with that image as attachment in the framebuffer, and return the image to the swap chain for presentation.
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
	VkDescriptorSetLayout		 descriptorSetLayout;				///< Opaque handle to a descriptor set layout object (combines all of the descriptor bindings).
	VkPipelineLayout			 pipelineLayout;					///< Pipeline layout. Allows to use uniform values in shaders (globals similar to dynamic state variables that can be changed at drawing at drawing time to alter the behavior of your shaders without having to recreate them).
	VkPipeline					 graphicsPipeline;					///< Opaque handle to a pipeline object.
	std::vector<VkFramebuffer>	 swapChainFramebuffers;				///< List. Opaque handle to a framebuffer object.
	VkCommandPool				 commandPool;						///< Opaque handle to a command pool object. It manages the memory that is used to store the buffers, and command buffers are allocated from them. 
	std::vector<VkCommandBuffer> commandBuffers;					///< List. Opaque handle to command buffer object
	std::vector<VkSemaphore>	 imageAvailableSemaphores;			///< Signals that an image has been acquired and is ready for rendering. Each frame has a semaphore for concurrent processing. Allows multiple frames to be in-flight while still bounding the amount of work that piles up.
	std::vector<VkSemaphore>	 renderFinishedSemaphores;			///< Signals that rendering has finished and presentation can happen.Each frame has a semaphore for concurrent processing. Allows multiple frames to be in-flight while still bounding the amount of work that piles up.
	std::vector<VkFence>		 inFlightFences;					///< Similar to semaphores, but fences actually wait in our own code. Used to perform CPU-GPU synchronization.
	std::vector<VkFence>		 imagesInFlight;					///< Maps frames in flight by their fences. Tracks for each swap chain image if a frame in flight is currently using it.
	size_t						 currentFrame = 0;					///< Frame to process next.
	bool						 framebufferResized	= false;		///< Many drivers/platforms trigger VK_ERROR_OUT_OF_DATE_KHR after window resize, but it's not guaranteed. This variable handles resizes explicitly.
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
	uint32_t					 mipLevels;							///< Number of levels (mipmaps)
	VkImage						 textureImage;						///< Opaque handle to an image object.
	VkDeviceMemory				 textureImageMemory;				///< Opaque handle to a device memory object.
	VkImageView					 textureImageView;					///< Image view for the texture image (images are accessed through image views rather than directly).
	VkSampler					 textureSampler;					///< Opaque handle to a sampler object (it applies filtering and transformations to a texture). It is a distinct object that provides an interface to extract colors from a texture. It can be applied to any image you want (1D, 2D or 3D).
	VkImage						 depthImage;						///< Depth buffer (image object).
	VkDeviceMemory				 depthImageMemory;					///< Depth buffer memory (memory object).
	VkImageView					 depthImageView;					///< Depth buffer image view (images are accessed through image views rather than directly).

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

	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);	///< Helper function for creating a buffer.

	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
	/// Finds the right type of memory to use, depending upon the requirements of the buffer and our own application requiremnts.
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	/// Copy the contents from one buffer to another.
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	/// Update Uniform buffer. It will generate a new transformation every frame to make the geometry spin around.
	void updateUniformBuffer(uint32_t currentImage);

	void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

	VkCommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommands(VkCommandBuffer commandBuffer);

	/// Handles layout transitions.
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);

	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);

	/// Take a list of candidate formats in order from most desirable to least desirable, and checks which is the first one that is supported.
	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

	/// Find the right format for a depth image.
	VkFormat findDepthFormat();

	/// Generate mipmaps
	void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);

	// HELPER FUNCTIONS --------------------------------------------------

	/// Tells if the chosen depth format contains a stencil component.
	bool hasStencilComponent(VkFormat format);
};

#endif