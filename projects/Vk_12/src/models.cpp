
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "models.hpp"

VkVertexInputBindingDescription Vertex::getBindingDescription()
{
	VkVertexInputBindingDescription bindingDescription{};
	bindingDescription.binding		= 0;							// Index of the binding in the array of bindings. We have a single array, so we only have one binding.
	bindingDescription.stride		= sizeof(Vertex);				// Number of bytes from one entry to the next.
	bindingDescription.inputRate	= VK_VERTEX_INPUT_RATE_VERTEX;	// VK_VERTEX_INPUT_RATE_ ... VERTEX, INSTANCE (move to the next data entry after each vertex or instance).

	return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 3> Vertex::getAttributeDescriptions()
{
	std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

	attributeDescriptions[0].binding	= 0;							// From which binding the per-vertex data comes.
	attributeDescriptions[0].location	= 0;							// Directive "location" of the input in the vertex shader.
	attributeDescriptions[0].format		= VK_FORMAT_R32G32B32_SFLOAT;	// Type of data for the attribute: VK_FORMAT_ ... R32_SFLOAT (float), R32G32_SFLOAT (vec2), R32G32B32_SFLOAT (vec3), R32G32B32A32_SFLOAT (vec4), R64_SFLOAT (64-bit double), R32G32B32A32_UINT (uvec4: 32-bit unsigned int), R32G32_SINT (ivec2: 32-bit signed int)...
	attributeDescriptions[0].offset		= offsetof(Vertex, pos);		// Number of bytes since the start of the per-vertex data to read from.

	attributeDescriptions[1].binding	= 0;
	attributeDescriptions[1].location	= 1;
	attributeDescriptions[1].format		= VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[1].offset		= offsetof(Vertex, color);

	attributeDescriptions[2].binding	= 0;
	attributeDescriptions[2].location	= 2;
	attributeDescriptions[2].format		= VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[2].offset		= offsetof(Vertex, texCoord);

	return attributeDescriptions;
}

bool Vertex::operator==(const Vertex& other) const {
	return	pos == other.pos &&
			color == other.color &&
			texCoord == other.texCoord;
}

size_t std::hash<Vertex>::operator()(Vertex const& vertex) const
{
	return ( (hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1 ) ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
}

modelConfig::modelConfig(const char* modelPath, const char* texturePath, const char* VSpath, const char* FSpath, glm::mat4(*ModelMatrixCallback) (float))
{
	char* address;
	size_t siz;

	siz					= strlen(modelPath) + 1;
	address				= new char[siz];
	strncpy(address, modelPath, siz);
	this->modelPath		= address;

	siz					= strlen(texturePath) + 1;
	address				= new char[siz];
	strncpy(address, texturePath, siz);
	this->texturePath	= address;

	siz					= strlen(VSpath) + 1;
	address				= new char[siz];
	strncpy(address, VSpath, siz);
	this->VSpath		= address;

	siz					= strlen(FSpath) + 1;
	address				= new char[siz];
	strncpy(address, FSpath, siz);
	this->FSpath		= address;

	getModelMatrices.clear();
	getModelMatrices.push_back(ModelMatrixCallback);
}

modelConfig::modelConfig(const char* modelPath, const char* texturePath, const char* VSpath, const char* FSpath, std::vector<std::function<glm::mat4(float)>>& ModelMatrixCallbacks)
{
	char* address;
	size_t siz;

	siz					= strlen(modelPath) + 1;
	address				= new char[siz];
	strncpy(address, modelPath, siz);
	this->modelPath		= address;

	siz					= strlen(texturePath) + 1;
	address				= new char[siz];
	strncpy(address, texturePath, siz);
	this->texturePath	= address;

	siz					= strlen(VSpath) + 1;
	address				= new char[siz];
	strncpy(address, VSpath, siz);
	this->VSpath		= address;

	siz					= strlen(FSpath) + 1;
	address				= new char[siz];
	strncpy(address, FSpath, siz);
	this->FSpath		= address;

	getModelMatrices.clear();
	getModelMatrices = ModelMatrixCallbacks;
}

modelConfig::modelConfig(const modelConfig& obj)
{
	char* address;
	size_t siz;

	siz			= strlen(obj.modelPath) + 1;
	address		= new char[siz];
	strncpy(address, obj.modelPath, siz);
	modelPath	= address;

	siz			= strlen(obj.texturePath) + 1;
	address		= new char[siz];
	strncpy(address, obj.texturePath, siz);
	texturePath	= address;

	siz			= strlen(obj.VSpath) + 1;
	address		= new char[siz];
	strncpy(address, obj.VSpath, siz);
	VSpath		= address;

	siz			= strlen(obj.FSpath) + 1;
	address		= new char[siz];
	strncpy(address, obj.FSpath, siz);
	FSpath		= address;

	getModelMatrices.clear();
	getModelMatrices = obj.getModelMatrices;
}

modelConfig::~modelConfig()
{
	delete[] modelPath;
	delete[] texturePath;
	delete[] VSpath;
	delete[] FSpath;
}


// UniformBufferObjectDynamic -----------------------------------------------------------------

UniformBufferObjectDynamic::UniformBufferObjectDynamic(size_t subUBOcount, VkDeviceSize minSizePerSubUBO)
	: subUBOcount(subUBOcount), data(nullptr)
{
	this->sizePerSubUBO = computeSizePerSubUBO(minSizePerSubUBO);

	totalBytes = this->sizePerSubUBO * subUBOcount;
	data = new char[totalBytes];
}

UniformBufferObjectDynamic::~UniformBufferObjectDynamic() { delete[] data; }

void UniformBufferObjectDynamic::setModel(size_t position, const glm::mat4& matrix) 
{ 
	glm::mat4* original = (glm::mat4*) &data[position * sizePerSubUBO + 0 * sizeof(glm::mat4)];
	*original = matrix;					// Equivalent to:   memcpy((void*)original, (void*)&matrix, sizeof(glm::mat4));
}

void UniformBufferObjectDynamic::setView(size_t position, const glm::mat4& matrix)
{ 
	glm::mat4* original = (glm::mat4*) &data[position * sizePerSubUBO + 1 * sizeof(glm::mat4)];
	*original = matrix;
}

void UniformBufferObjectDynamic::setProj(size_t position, const glm::mat4& matrix)
{ 
	glm::mat4* original = (glm::mat4*) &data[position * sizePerSubUBO + 2 * sizeof(glm::mat4)];
	*original = matrix;
}

VkDeviceSize UniformBufferObjectDynamic::computeSizePerSubUBO(VkDeviceSize minimumSize)
{
	VkDeviceSize usefulSize = sizeof(glm::mat4) * 3;
	if (usefulSize > minimumSize) return usefulSize;
	else return minimumSize;
}

// modelData ----------------------------------------------------------------------------------

/// Function for computing the model matrix (MM). Used by default as callback in the modelData constructor.
glm::mat4 default_MM(float time)
{
	glm::mat4 model	= glm::mat4(1.0f);
	model			= glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
	model			= glm::rotate(model, time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model			= glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));

	return model;
}

modelData::modelData(VulkanEnvironment &environment, modelConfig config, VkDeviceSize subUniformBufferSize)
	: e(environment), config(config)
{
	getModelMatrix = config.getModelMatrices;

	createDescriptorSetLayout();
	createGraphicsPipeline(config.VSpath, config.FSpath);

	createTextureImage(config.texturePath);
	createTextureImageView();
	createTextureSampler();
	loadModel(config.modelPath);
	createVertexBuffer();
	createIndexBuffer();
	createUniformBuffers();
	createDescriptorPool();
	createDescriptorSets();

	for (size_t i = 0; i < getModelMatrix.size(); i++)
		dynamicOffsets.push_back(i * subUniformBufferSize);		// subUniformBufferSize = e->getMinUniformBufferOffsetAlignment()  (usually, 256)
}

// (9)
void modelData::createDescriptorSetLayout()
{
	// Describe the bindings
	//	- Uniform buffer descriptor
	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.binding			= 0;
	if (getModelMatrix.size() == 1)
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	else
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	uboLayoutBinding.descriptorCount	= 1;								// In case you want to specify an array of UBOs <<< (example: for specifying a transformation for each of the bones in a skeleton for skeletal animation).
	uboLayoutBinding.stageFlags			= VK_SHADER_STAGE_VERTEX_BIT;		// Tell in which shader stages the descriptor will be referenced. This field can be a combination of VkShaderStageFlagBits values or the value VK_SHADER_STAGE_ALL_GRAPHICS.
	uboLayoutBinding.pImmutableSamplers	= nullptr;							// [Optional] Only relevant for image sampling related descriptors.

	//	- Combined image sampler descriptor (it lets shaders access an image resource through a sampler object)
	VkDescriptorSetLayoutBinding samplerLayoutBinding{};
	samplerLayoutBinding.binding			= 1;
	samplerLayoutBinding.descriptorType		= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.descriptorCount	= 1;
	samplerLayoutBinding.stageFlags			= VK_SHADER_STAGE_FRAGMENT_BIT;			// We want to use the combined image sampler descriptor in the fragment shader. It's possible to use texture sampling in the vertex shader (example: to dynamically deform a grid of vertices by a heightmap).
	samplerLayoutBinding.pImmutableSamplers	= nullptr;
	
	std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };

	// Create a descriptor set layout (combines all of the descriptor bindings)
	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType		= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount	= static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings	= bindings.data();

	if (vkCreateDescriptorSetLayout(e.device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
		throw std::runtime_error("Failed to create descriptor set layout!");
}

// (10)
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
	In Vulkan, the graphics pipeline is almost completely immutable. You will have to create a number of pipelines representing all of the different combinations of states you want to use.
*/
void modelData::createGraphicsPipeline(const char* VSpath, const char* FSpath)
{
	// Create pipeline layout   <<< sameMod
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;						// Optional
	pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;		// Optional	 <<<<<
	pipelineLayoutInfo.pushConstantRangeCount = 0;				// Optional. <<< Push constants are another way of passing dynamic values to shaders.
	pipelineLayoutInfo.pPushConstantRanges = nullptr;			// Optional

	if (vkCreatePipelineLayout(e.device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		throw std::runtime_error("Failed to create pipeline layout!");

	// Read shader files
	std::vector<char> vertShaderCode = readFile(VSpath);
	std::vector<char> fragShaderCode = readFile(FSpath);

	VkShaderModule vertShaderModule	= createShaderModule(vertShaderCode);
	VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

	// Configure Vertex shader
	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType				= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage				= VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module				= vertShaderModule;
	vertShaderStageInfo.pName				= "main";			// Function to invoke (entrypoint). You may combine multiple fragment shaders into a single shader module and use different entry points (different behaviors).  
	vertShaderStageInfo.pSpecializationInfo	= nullptr;			// Optional. Specifies values for shader constants.

	// Configure Fragment shader
	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType				= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage				= VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module				= fragShaderModule;
	fragShaderStageInfo.pName				= "main";
	fragShaderStageInfo.pSpecializationInfo	= nullptr;

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	// Vertex input: Describes format of the vertex data that will be passed to the vertex shader.
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType							= VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	auto bindingDescription							= Vertex::getBindingDescription();
	vertexInputInfo.vertexBindingDescriptionCount	= 1;
	vertexInputInfo.pVertexBindingDescriptions		= &bindingDescription;							// Optional
	auto attributeDescriptions						= Vertex::getAttributeDescriptions();
	vertexInputInfo.vertexAttributeDescriptionCount	= static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexAttributeDescriptions	= attributeDescriptions.data();					// Optional

	// Input assembly: Describes what kind of geometry will be drawn from the vertices, and if primitive restart should be enabled.
	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType						= VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology					= VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;		// VK_PRIMITIVE_TOPOLOGY_ ... POINT_LIST, LINE_LIST, LINE_STRIP, TRIANGLE_LIST, TRIANGLE_STRIP
	inputAssembly.primitiveRestartEnable	= VK_FALSE;									// If VK_TRUE, then it's possible to break up lines and triangles in the _STRIP topology modes by using a special index of 0xFFFF or 0xFFFFFFFF.

	// Viewport: Describes the region of the framebuffer that the output will be rendered to.
	VkViewport viewport{};
	viewport.x			= 0.0f;
	viewport.y			= 0.0f;
	viewport.width		= (float)e.swapChainExtent.width;
	viewport.height		= (float)e.swapChainExtent.height;
	viewport.minDepth	= 0.0f;
	viewport.maxDepth	= 1.0f;

	// Scissor rectangle: Defines in which region pixels will actually be stored. Pixels outside the scissor rectangles will be discarded by the rasterizer. It works like a filter rather than a transformation.
	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = e.swapChainExtent;

	// Viewport state: Combines the viewport and scissor rectangle into a viewport state. Multiple viewports and scissors require enabling a GPU feature.
	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType			= VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount	= 1;
	viewportState.pViewports	= &viewport;
	viewportState.scissorCount	= 1;
	viewportState.pScissors		= &scissor;

	// Rasterizer: It takes the geometry shaped by the vertices from the vertex shader and turns it into fragments to be colored by the fragment shader. It also performs depth testing, face culling and the scissor test, and can be configured to output fragments that fill entire polygons or just the edges (wireframe rendering).
	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType					= VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable			= VK_FALSE;							// If VK_TRUE, fragments that are beyond the near and far planes are clamped to them (requires enabling a GPU feature), as opposed to discarding them.
	rasterizer.rasterizerDiscardEnable	= VK_FALSE;							// If VK_TRUE, geometry never passes through the rasterizer stage (disables any output to the framebuffer).
	rasterizer.polygonMode				= VK_POLYGON_MODE_FILL;				// How fragments are generated for geometry (VK_POLYGON_MODE_ ... FILL, LINE, POINT). Any mode other than FILL requires enabling a GPU feature.
	rasterizer.lineWidth				= 1.0f;								// Thickness of lines in terms of number of fragments. The maximum line width supported depends on the hardware. Lines thicker than 1.0f requires enabling the `wideLines` GPU feature.
	rasterizer.cullMode					= VK_CULL_MODE_BACK_BIT;			// Type of face culling (disable culling, cull front faces, cull back faces, cull both).
	rasterizer.frontFace				= VK_FRONT_FACE_COUNTER_CLOCKWISE;	// Vertex order for faces to be considered front-facing (clockwise, counterclockwise). If we draw vertices clockwise, because of the Y-flip we did in the projection matrix, the vertices are now drawn counter-clockwise.
	rasterizer.depthBiasEnable			= VK_FALSE;							// If VK_TRUE, it allows to alter the depth values (sometimes used for shadow mapping).
	rasterizer.depthBiasConstantFactor	= 0.0f;								// [Optional] 
	rasterizer.depthBiasClamp			= 0.0f;								// [Optional] 
	rasterizer.depthBiasSlopeFactor		= 0.0f;								// [Optional] 

	// Multisampling: One way to perform anti-aliasing. Combines the fragment shader results of multiple polygons that rasterize to the same pixel. Requires enabling a GPU feature.
	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType					= VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.rasterizationSamples	= e.msaaSamples;
	multisampling.sampleShadingEnable	= (e.add_SS ? VK_TRUE : VK_FALSE);	// Enable sample shading in the pipeline
	if (e.add_SS)
		multisampling.minSampleShading	= .2f;								// [Optional] Min fraction for sample shading; closer to one is smoother
	multisampling.pSampleMask			= nullptr;							// [Optional]
	multisampling.alphaToCoverageEnable	= VK_FALSE;							// [Optional]
	multisampling.alphaToOneEnable		= VK_FALSE;							// [Optional]

	// Depth and stencil testing. Used if you are using a depth and/or stencil buffer.
	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType					= VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable		= VK_TRUE;				// Specify if the depth of new fragments should be compared to the depth buffer to see if they should be discarded.
	depthStencil.depthWriteEnable		= VK_TRUE;				// Specify if the new depth of fragments that pass the depth test should actually be written to the depth buffer.
	depthStencil.depthCompareOp			= VK_COMPARE_OP_LESS;	// Specify the comparison that is performed to keep or discard fragments.
	depthStencil.depthBoundsTestEnable	= VK_FALSE;				// [Optional] Use depth bound test (allows to only keep fragments that fall within a specified depth range.
	depthStencil.minDepthBounds			= 0.0f;					// [Optional]
	depthStencil.maxDepthBounds			= 1.0f;					// [Optional]
	depthStencil.stencilTestEnable		= VK_FALSE;				// [Optional] Use stencil buffer operations (if you want to use it, make sure that the format of the depth/stencil image contains a stencil component).
	depthStencil.front					= {};					// [Optional]
	depthStencil.back					= {};					// [Optional]

	// Color blending: After a fragment shader has returned a color, it needs to be combined with the color that is already in the framebuffer. Two ways to do it: Mix old and new value to produce a final color, or combine the old and new value using a bitwise operation.
	//	- Configuration per attached framebuffer
	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	if (1)	// Not alpha blending implemented
	{
		colorBlendAttachment.blendEnable			= VK_FALSE;
		colorBlendAttachment.srcColorBlendFactor	= VK_BLEND_FACTOR_ONE;		// Optional. Check VkBlendFactor enum.
		colorBlendAttachment.dstColorBlendFactor	= VK_BLEND_FACTOR_ZERO;		// Optional
		colorBlendAttachment.colorBlendOp			= VK_BLEND_OP_ADD;			// Optional. Check VkBlendOp enum.
		colorBlendAttachment.srcAlphaBlendFactor	= VK_BLEND_FACTOR_ONE;		// Optional
		colorBlendAttachment.dstAlphaBlendFactor	= VK_BLEND_FACTOR_ZERO;		// Optional
		colorBlendAttachment.alphaBlendOp			= VK_BLEND_OP_ADD;			// Optional
	}
	else	// Options for implementing alpha blending (new color blended with old color based on its opacity):
	{
		colorBlendAttachment.blendEnable			= VK_TRUE;
		colorBlendAttachment.srcColorBlendFactor	= VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachment.dstColorBlendFactor	= VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachment.colorBlendOp			= VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor	= VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor	= VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.alphaBlendOp			= VK_BLEND_OP_ADD;
	}
	/* Pseudocode demonstration:
	if (blendEnable) {
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
	colorBlending.blendConstants[0]	= 0.0f;						// Optional
	colorBlending.blendConstants[1]	= 0.0f;						// Optional
	colorBlending.blendConstants[2]	= 0.0f;						// Optional
	colorBlending.blendConstants[3]	= 0.0f;						// Optional

	// Dynamic states: A limited amount of the state that we specified in the previous structs can actually be changed without recreating the pipeline (size of viewport, lined width, blend constants...). If you want to do that, you have to fill this struct. This will cause the configuration of these values to be ignored and you will be required to specify the data at drawing time. This struct can be substituted by a nullptr later on if you don't have any dynamic state.
	VkDynamicState dynamicStates[]	= { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_LINE_WIDTH };

	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType				= VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount	= 2;
	dynamicState.pDynamicStates		= dynamicStates;

	// Create graphics pipeline
	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType					= VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount				= 2;
	//pipelineInfo.flags				= VK_PIPELINE_CREATE_DERIVATIVE_BIT;	// Required for using basePipelineHandle and basePipelineIndex members
	pipelineInfo.pStages				= shaderStages;
	pipelineInfo.pVertexInputState		= &vertexInputInfo;
	pipelineInfo.pInputAssemblyState	= &inputAssembly;
	pipelineInfo.pViewportState			= &viewportState;
	pipelineInfo.pRasterizationState	= &rasterizer;
	pipelineInfo.pMultisampleState		= &multisampling;
	pipelineInfo.pDepthStencilState		= &depthStencil;	// [Optional]
	pipelineInfo.pColorBlendState		= &colorBlending;
	pipelineInfo.pDynamicState			= nullptr;			// [Optional] <<< NO SE AÑADIÓ LA STRUCT dynamicState
	pipelineInfo.layout					= pipelineLayout;
	pipelineInfo.renderPass				= e.renderPass;		// <<< It's possible to use other render passes with this pipeline instead of this specific instance, but they have to be compatible with "renderPass" (https://www.khronos.org/registry/vulkan/specs/1.0/html/vkspec.html#renderpass-compatibility).
	pipelineInfo.subpass				= 0;
	pipelineInfo.basePipelineHandle		= VK_NULL_HANDLE;	// [Optional] Specify the handle of an existing pipeline.
	pipelineInfo.basePipelineIndex		= -1;				// [Optional] Reference another pipeline that is about to be created by index.

	if (vkCreateGraphicsPipelines(e.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
		throw std::runtime_error("Failed to create graphics pipeline!");

	// Cleanup
	vkDestroyShaderModule(e.device, fragShaderModule, nullptr);
	vkDestroyShaderModule(e.device, vertShaderModule, nullptr);
}

std::vector<char> modelData::readFile(/*const std::string& filename*/ const char* filename)
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

VkShaderModule modelData::createShaderModule(const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());	// The default allocator from std::vector ensures that the data satisfies the alignment requirements of `uint32_t`.

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(e.device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		throw std::runtime_error("Failed to create shader module!");

	return shaderModule;
}

void modelData::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
	// Create buffer.
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;									// For multiple purposes use a bitwise or.
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;				// Like images in the swap chain, buffers can also be owned by a specific queue family or be shared between multiple at the same time. Since the buffer will only be used from the graphics queue, we use EXCLUSIVE.
	bufferInfo.flags = 0;										// Used to configure sparse buffer memory.

	if (vkCreateBuffer(e.device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)	// vkCreateBuffer creates a new buffer object and returns it to a pointer to a VkBuffer provided by the caller.
		throw std::runtime_error("Failed to create buffer!");

	// Get buffer requirements.
	VkMemoryRequirements memRequirements;		// Members: size (amount of memory in bytes. May differ from bufferInfo.size), alignment (offset in bytes where the buffer begins in the allocated region. Depends on bufferInfo.usage and bufferInfo.flags), memoryTypeBits (bit field of the memory types that are suitable for the buffer).
	vkGetBufferMemoryRequirements(e.device, buffer, &memRequirements);

	// Allocate memory for the buffer.
	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = e.findMemoryType(memRequirements.memoryTypeBits, properties);		// Properties parameter: We need to be able to write our vertex data to that memory. The properties define special features of the memory, like being able to map it so we can write to it from the CPU.

	if (vkAllocateMemory(e.device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
		throw std::runtime_error("Failed to allocate buffer memory!");

	vkBindBufferMemory(e.device, buffer, bufferMemory, 0);	// Associate this memory with the buffer. If the offset (4th parameter) is non-zero, it's required to be divisible by memRequirements.alignment.
}

// (15)
/// Load a texture > Copy it to a buffer > Copy it to an image > Cleanup the buffer
void modelData::createTextureImage(const char* path)
{
	// Load an image
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(path, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);		// Returns a pointer to an array of pixel values. STBI_rgb_alpha forces the image to be loaded with an alpha channel, even if it doesn't have one.
	if (!pixels)
		throw std::runtime_error("Failed to load texture image!");

	VkDeviceSize imageSize = texWidth * texHeight * 4;												// 4 bytes per rgba pixel
	mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;	// Calculate the number levels (mipmaps)

	// Create a staging buffer (temporary buffer in host visible memory so that we can use vkMapMemory and copy the pixels to it)
	VkBuffer	   stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	createBuffer(imageSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer,
		stagingBufferMemory);

	// Copy directly the pixel values from the image we loaded to the staging-buffer.
	void* data;
	vkMapMemory(e.device, stagingBufferMemory, 0, imageSize, 0, &data);	// vkMapMemory retrieves a host virtual address pointer (data) to a region of a mappable memory object (stagingBufferMemory). We have to provide the logical device that owns the memory (e.device).
	memcpy(data, pixels, static_cast<size_t>(imageSize));				// Copies a number of bytes (imageSize) from a source (pixels) to a destination (data).
	vkUnmapMemory(e.device, stagingBufferMemory);						// Unmap a previously mapped memory object (stagingBufferMemory).

	stbi_image_free(pixels);	// Clean up the original pixel array

	// Create the texture image
	e.createImage(	texWidth,
					texHeight,
					mipLevels,
					VK_SAMPLE_COUNT_1_BIT,
					VK_FORMAT_R8G8B8A8_SRGB,
					VK_IMAGE_TILING_OPTIMAL,
					VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
					textureImage,
					textureImageMemory );

	// Copy the staging buffer to the texture image
	e.transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);					// Transition the texture image to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
	copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));											// Execute the buffer to image copy operation
	// Transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmaps
	// transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mipLevels);	// To be able to start sampling from the texture image in the shader, we need one last transition to prepare it for shader access
	generateMipmaps(textureImage, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, mipLevels);

	// Cleanup the staging buffer and its memory
	vkDestroyBuffer(e.device, stagingBuffer, nullptr);
	vkFreeMemory(e.device, stagingBufferMemory, nullptr);
}

void modelData::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
	VkCommandBuffer commandBuffer = e.beginSingleTimeCommands();

	// Specify which part of the buffer is going to be copied to which part of the image
	VkBufferImageCopy region{};
	region.bufferOffset = 0;							// Byte offset in the buffer at which the pixel values start
	region.bufferRowLength = 0;							// How the pixels are laid out in memory. 0 indicates that the pixels are thightly packed. Otherwise, you could have some padding bytes between rows of the image, for example. 
	region.bufferImageHeight = 0;							// How the pixels are laid out in memory. 0 indicates that the pixels are thightly packed. Otherwise, you could have some padding bytes between rows of the image, for example.
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;	// imageSubresource indicate to which part of the image we want to copy the pixels
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = { 0, 0, 0 };					// Indicate to which part of the image we want to copy the pixels
	region.imageExtent = { width, height, 1 };			// Indicate to which part of the image we want to copy the pixels

	// Enqueue buffer to image copy operations
	vkCmdCopyBufferToImage(commandBuffer,
		buffer,
		image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,			// Layout the image is currently using
		1,
		&region);

	e.endSingleTimeCommands(commandBuffer);
}

void modelData::generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels)
{
	// Check if the image format supports linear blitting. We are using vkCmdBlitImage, but it's not guaranteed to be supported on all platforms bacause it requires our texture image format to support linear filtering, so we check it with vkGetPhysicalDeviceFormatProperties.
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(e.physicalDevice, imageFormat, &formatProperties);
	if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
	{
		throw std::runtime_error("Texture image format does not support linear blitting!");
		// Two alternatives:
		//		- Implement a function that searches common texture image formats for one that does support linear blitting.
		//		- Implement the mipmap generation in software with a library like stb_image_resize. Each mip level can then be loaded into the image in the same way that you loaded the original image.
		// It's uncommon to generate the mipmap levels at runtime anyway. Usually they are pregenerated and stored in the texture file alongside the base level to improve loading speed. <<<<<
	}

	VkCommandBuffer commandBuffer = e.beginSingleTimeCommands();

	// Specify the barriers
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = image;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = 1;

	int32_t mipWidth = texWidth;
	int32_t mipHeight = texHeight;

	for (uint32_t i = 1; i < mipLevels; i++)	// This loop records each of the VkCmdBlitImage commands. The source mip level is i - 1 and the destination mip level is i.
	{
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;	// We transition level i - 1 to VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL. This transition will wait for level i - 1 to be filled, either from the previous blit command, or from vkCmdCopyBufferToImage. The current blit command will wait on this transition.
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		// Record a barrier (we transition level i - 1 to VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL. This transition will wait for level i - 1 to be filled, either from the previous blit command, or from vkCmdCopyBufferToImage. The current blit command will wait on this transition).
		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		// Specify the regions that will be used in the blit operation
		VkImageBlit blit{};
		blit.srcOffsets[0] = { 0, 0, 0 };						// srcOffsets determine the 3D regions ...
		blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };		// ... that data will be blitted from.
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;
		blit.dstOffsets[0] = { 0, 0, 0 };																	// dstOffsets determine the 3D region ...
		blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1,  mipHeight > 1 ? mipHeight / 2 : 1,  1 };	// ... that data will be blitted to.
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;

		// Record a blit command. Beware if you are using a dedicated transfer queue: vkCmdBlitImage must be submitted to a queue with graphics capability.
		vkCmdBlitImage(commandBuffer,
			image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,		// The textureImage is used for both the srcImage and dstImage parameter ...
			image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,		// ...  because we're blitting between different levels of the same image.
			1, &blit,
			VK_FILTER_LINEAR);									// Enable interpolation

		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		// Record a barrier (This barrier transitions mip level i - 1 to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL. This transition waits on the current blit command to finish. All sampling operations will wait on this transition to finish).
		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		if (mipWidth > 1) mipWidth /= 2;
		if (mipHeight > 1) mipHeight /= 2;
	}

	barrier.subresourceRange.baseMipLevel = mipLevels - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	// Record a barrier (This barrier transitions the last mip level from VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL. This wasn't handled by the loop, since the last mip level is never blitted from).
	vkCmdPipelineBarrier(commandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
		0, nullptr,
		0, nullptr,
		1, &barrier);

	e.endSingleTimeCommands(commandBuffer);
}

// (16)
void modelData::createTextureImageView()
{
	textureImageView = e.createImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);
}

// (17)
void modelData::createTextureSampler()
{
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType			= VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter		= VK_FILTER_LINEAR;					// How to interpolate texels that are magnified (oversampling) or ...
	samplerInfo.minFilter		= VK_FILTER_LINEAR;					// ... minified (undersampling). Choices: VK_FILTER_NEAREST, VK_FILTER_LINEAR
	samplerInfo.addressModeU	= VK_SAMPLER_ADDRESS_MODE_REPEAT;	// Addressing mode per axis (what happens when going beyond the image dimensions). In texture space coordinates, XYZ are UVW. Available values: VK_SAMPLER_ADDRESS_MODE_ ... REPEAT (repeat the texture), MIRRORED_REPEAT (like repeat, but inverts coordinates to mirror the image), CLAMP_TO_EDGE (take the color of the closest edge), MIRROR_CLAMP_TO_EDGE (like clamp to edge, but taking the opposite edge), CLAMP_TO_BORDER (return solid color).
	samplerInfo.addressModeV	= VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW	= VK_SAMPLER_ADDRESS_MODE_REPEAT;

	if (1)		// If anisotropic filtering is available (see isDeviceSuitable) <<<<<
	{
		samplerInfo.anisotropyEnable = VK_TRUE;							// Specify if anisotropic filtering should be used
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(e.physicalDevice, &properties);
		samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;		// another option:  samplerInfo.maxAnisotropy = 1.0f;
	}
	else
	{
		samplerInfo.anisotropyEnable = VK_FALSE;
		samplerInfo.maxAnisotropy = 1.0f;
	}

	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;	// Color returned (black, white or transparent, in format int or float) when sampling beyond the image with VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER. You cannot specify an arbitrary color.
	samplerInfo.unnormalizedCoordinates = VK_FALSE;							// Coordinate system to address texels in an image. False: [0, 1). True: [0, texWidth) & [0, texHeight). 
	samplerInfo.compareEnable = VK_FALSE;							// If a comparison function is enabled, then texels will first be compared to a value, and the result of that comparison is used in filtering operations. This is mainly used for percentage-closer filtering on shadow maps (https://developer.nvidia.com/gpugems/gpugems/part-ii-lighting-and-shadows/chapter-11-shadow-map-antialiasing). 
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;	// VK_SAMPLER_MIPMAP_MODE_ ... NEAREST (lod selects the mip level to sample from), LINEAR (lod selects 2 mip levels to be sampled, and the results are linearly blended)
	samplerInfo.minLod = 0.0f;								// minLod=0 & maxLod=mipLevels allow the full range of mip levels to be used
	samplerInfo.maxLod = static_cast<float>(mipLevels);	// lod: Level Of Detail
	samplerInfo.mipLodBias = 0.0f;								// Used for changing the lod value. It forces to use lower "lod" and "level" than it would normally use

	if (vkCreateSampler(e.device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS)
		throw std::runtime_error("Failed to create texture sampler!");
	/*
	* VkImage holds the mipmap data. VkSampler controls how that data is read while rendering.
	* The sampler selects a mip level according to this pseudocode:
	*
	*	lod = getLodLevelFromScreenSize();						// Smaller when the object is close (may be negative)
	*	lod = clamp(lod + mipLodBias, minLod, maxLod);
	*
	*	level = clamp(floor(lod), 0, texture.miplevels - 1);	// Clamped to the number of miplevels in the texture
	*
	*	if(mipmapMode == VK_SAMPLER_MIPMAP_MODE_NEAREST)		// Sample operation
	*		color = sampler(level);
	*	else
	*		color = blend(sample(level), sample(level + 1));
	*
	*	if(lod <= 0)											// Filter
	*		color = readTexture(uv, magFilter);
	*	else
	*		color = readTexture(uv, minFilter);
	*/
}

// (18)
/**
*	An OBJ file consists of positions, normals, texture coordinates and faces. Faces consist of an arbitrary amount of vertices, where each vertex refers to a position, normal and/or texture coordinate by index.
*/
void modelData::loadModel(const char* obj_file)
{
	tinyobj::attrib_t					 attrib;			// Holds all of the positions, normals and texture coordinates.
	std::vector<tinyobj::shape_t>		 shapes;			// Holds all of the separate objects and their faces. Each face consists of an array of vertices. Each vertex contains the indices of the position, normal and texture coordinate attributes.
	std::vector<tinyobj::material_t>	 materials;			// OBJ models can also define a material and texture per face, but we will ignore those.
	std::string							 warn, err;			// Errors and warnings that occur while loading the file.
	std::unordered_map<Vertex, uint32_t> uniqueVertices{};	// Keeps track of the unique vertices and the respective indices, avoiding duplicated vertices (not indices).

	// Load model
	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, obj_file))
		throw std::runtime_error(warn + err);

	// Combine all the faces in the file into a single model
	for (const auto& shape : shapes)
		for (const auto& index : shape.mesh.indices)
		{
			Vertex vertex{};

			vertex.pos = {
				attrib.vertices[3 * index.vertex_index + 0],			// attrib.vertices is an array of floats, so we need to multiply the index by 3 and add offsets for accessing XYZ components.
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

			vertex.texCoord = {
					   attrib.texcoords[2 * index.texcoord_index + 0],	// attrib.texcoords is an array of floats, so we need to multiply the index by 3 and add offsets for accessing UV components.
				1.0f - attrib.texcoords[2 * index.texcoord_index + 1]	// Flip vertical component of texture coordinates: OBJ format assumes Y axis go up, but Vulkan has top-to-bottom orientation. 
			};

			vertex.color = { 1.0f, 1.0f, 1.0f };

			if (uniqueVertices.count(vertex) == 0)	// Check if we have already seen this vertex. Using a user-defined type (Vertex struct) as key in a hash table requires us to implement two functions: equality test (override operator ==) and hash calculation (implement a hash function for Vertex).
			{
				uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());	// Set new index for this vertex
				vertices.push_back(vertex);											// Save vertex
			}

			indices.push_back(uniqueVertices[vertex]);								// Save index
		}
}

// (19)
void modelData::createVertexBuffer()
{
	// Create a staging buffer (host visible buffer used as temporary buffer for mapping and copying the vertex data)
	VkDeviceSize   bufferSize = sizeof(vertices[0]) * vertices.size();
	VkBuffer	   stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	createBuffer(bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 											// VK_BUFFER_USAGE_ ... TRANSFER_SRC_BIT / TRANSFER_DST_BIT (buffer can be used as source/destination in a memory transfer operation).
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer,
		stagingBufferMemory);

	// Fill the staging buffer (by mapping the buffer memory into CPU accessible memory: https://en.wikipedia.org/wiki/Memory-mapped_I/O)
	void* data;
	vkMapMemory(e.device, stagingBufferMemory, 0, bufferSize, 0, &data);	// Access a memory region. Use VK_WHOLE_SIZE to mapa all of the memory.
	memcpy(data, vertices.data(), (size_t)bufferSize);						// Copy the vertex data to the mapped memory.
	vkUnmapMemory(e.device, stagingBufferMemory);								// Unmap memory.

	/*
		Note:
		The driver may not immediately copy the data into the buffer memory (example: because of caching).
		It is also possible that writes to the buffer are not visible in the mapped memory yet. Two ways to deal with that problem:
		  - (Our option) Coherent memory heap: Use a memory heap that is host coherent, indicated with VK_MEMORY_PROPERTY_HOST_COHERENT_BIT. This ensures that the mapped memory always matches the contents of the allocated memory (this may lead to slightly worse performance than explicit flushing, but this doesn't matter since we will use a staging buffer).
		  - Flushing memory: Call vkFlushMappedMemoryRanges after writing to the mapped memory, and call vkInvalidateMappedMemoryRanges before reading from the mapped memory.
		Either option means that the driver will be aware of our writes to the buffer, but it doesn't mean that they are actually visible on the GPU yet.
		The transfer of data to the GPU happens in the background and the specification simply tells us that it is guaranteed to be complete as of the next call to vkQueueSubmit.
	*/

	// Create the actual vertex buffer (Device local buffer used as actual vertex buffer. Generally it doesn't allow to use vkMapMemory, but we can copy from stagingBuffer to vertexBuffer, though you need to specify the transfer source flag for stagingBuffer and the transfer destination flag for vertexBuffer).
	// This makes vertex data to be loaded from high performance memory.
	createBuffer(bufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		vertexBuffer,
		vertexBufferMemory);

	// Move the vertex data to the device local buffer
	copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

	// Clean up
	vkDestroyBuffer(e.device, stagingBuffer, nullptr);
	vkFreeMemory(e.device, stagingBufferMemory, nullptr);
}

/**
*	Memory transfer operations are executed using command buffers (like drawing commands), so we allocate a temporary command buffer. You may wish to create a separate command pool for these kinds of short-lived buffers, because the implementation could apply memory allocation optimizations. You should use the VK_COMMAND_POOL_CREATE_TRANSIENT_BIT flag during command pool generation in that case.
*/
void modelData::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkCommandBuffer commandBuffer = e.beginSingleTimeCommands();

	// Specify buffers and the size of the contents you will transfer (it's not possible to specify VK_WHOLE_SIZE here, unlike vkMapMemory command).
	VkBufferCopy copyRegion{};
	copyRegion.size = size;
	copyRegion.srcOffset = 0;	// Optional
	copyRegion.dstOffset = 0;	// Optional

	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	e.endSingleTimeCommands(commandBuffer);
}

// (20)
void modelData::createIndexBuffer()
{
	// Create a staging buffer
	VkDeviceSize   bufferSize = sizeof(indices[0]) * indices.size();
	VkBuffer	   stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	createBuffer(bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer,
		stagingBufferMemory);

	// Fill the staging buffer
	void* data;
	vkMapMemory(e.device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, indices.data(), (size_t)bufferSize);
	vkUnmapMemory(e.device, stagingBufferMemory);

	// Create the vertex buffer
	createBuffer(bufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		indexBuffer,
		indexBufferMemory);

	// Move the vertex data to the device local buffer
	copyBuffer(stagingBuffer, indexBuffer, bufferSize);

	// Clean up
	vkDestroyBuffer(e.device, stagingBuffer, nullptr);
	vkFreeMemory(e.device, stagingBufferMemory, nullptr);
}

// (21)
void modelData::createUniformBuffers()
{
	VkDeviceSize bufferSize;
	if (getModelMatrix.size() == 1)	bufferSize = sizeof(UniformBufferObject);
	else							bufferSize = getModelMatrix.size() * UniformBufferObjectDynamic::computeSizePerSubUBO(e.getMinUniformBufferOffsetAlignment());
	//else							bufferSize = sizeof(UniformBufferObjectDynamic) * getModelMatrix.size();

	uniformBuffers.resize(e.swapChainImages.size());
	uniformBuffersMemory.resize(e.swapChainImages.size());

	for (size_t i = 0; i < e.swapChainImages.size(); i++)
		createBuffer(	bufferSize,
						VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
						VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
						uniformBuffers[i],
						uniformBuffersMemory[i] );
}
 
// (22)
void modelData::createDescriptorPool()
{
	// Describe our descriptor sets.
	std::array<VkDescriptorPoolSize, 2> poolSizes{};
	if (getModelMatrix.size() == 1)	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	else							poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	poolSizes[0].descriptorCount	= static_cast<uint32_t>(e.swapChainImages.size());	// Number of descriptors of this type to allocate
	poolSizes[1].type				= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount	= static_cast<uint32_t>(e.swapChainImages.size());

	// Allocate one of these descriptors for every frame.
	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = static_cast<uint32_t>(e.swapChainImages.size());	// Max. number of individual descriptor sets that may be allocated
	poolInfo.flags = 0;												// Determine if individual descriptor sets can be freed (VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT) or not (0). Since we aren't touching the descriptor set after its creation, we put 0 (default).

	if (vkCreateDescriptorPool(e.device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
		throw std::runtime_error("Failed to create descriptor pool!");
}

// (23)
void modelData::createDescriptorSets()
{
	std::vector<VkDescriptorSetLayout> layouts(e.swapChainImages.size(), descriptorSetLayout);

	// Describe the descriptor set. Here, we will create one descriptor set for each swap chain image, all with the same layout
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;										// Descriptor pool to allocate from
	allocInfo.descriptorSetCount = static_cast<uint32_t>(e.swapChainImages.size());	// Number of descriptor sets to allocate
	allocInfo.pSetLayouts = layouts.data();											// Descriptor layout to base them on

	// Allocate the descriptor set handles
	descriptorSets.resize(e.swapChainImages.size());
	if (vkAllocateDescriptorSets(e.device, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
		throw std::runtime_error("Failed to allocate descriptor sets!");

	// Populate each descriptor set.
	for (size_t i = 0; i < e.swapChainImages.size(); i++)
	{
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = uniformBuffers[i];
		bufferInfo.offset = 0;
		if (getModelMatrix.size() == 1)	bufferInfo.range = sizeof(UniformBufferObject);	// If you're overwriting the whole buffer, like we are in this case, it's possible to use VK_WHOLE_SIZE here. 
		else							bufferInfo.range = VK_WHOLE_SIZE;				// <<< Why validation layers complain with the line below
		//else							bufferInfo.range = getModelMatrix.size() * UniformBufferObjectDynamic::computeSizePerSubUBO(e.getMinUniformBufferOffsetAlignment());

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = textureImageView;
		imageInfo.sampler = textureSampler;

		std::array<VkWriteDescriptorSet, 2> descriptorWrites{};		// <<< CURRENT BUG (render same model many times)

		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = descriptorSets[i];						// Descriptor set to update
		descriptorWrites[0].dstBinding = 0;										// Binding
		descriptorWrites[0].dstArrayElement = 0;										// First index in the array (if you want to update multiple descriptors at once in an array)
		if (getModelMatrix.size() == 1)
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;		// Type of descriptor
		else
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		descriptorWrites[0].descriptorCount = 1;										// Number of array elements to update
		descriptorWrites[0].pBufferInfo = &bufferInfo;								// Used for descriptors that refer to buffer data (like our descriptor)
		descriptorWrites[0].pImageInfo = nullptr;									// [Optional] Used for descriptors that refer to image data
		descriptorWrites[0].pTexelBufferView = nullptr;									// [Optional] Used for descriptors that refer to buffer views

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = descriptorSets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(e.device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);	// Accepts 2 kinds of arrays as parameters: VkWriteDescriptorSet, VkCopyDescriptorSet.
	}
}



void modelData::recreateSwapChain()
{
	createGraphicsPipeline(config.VSpath, config.FSpath);	// Recreate graphics pipeline because viewport and scissor rectangle size is specified during graphics pipeline creation (this can be avoided by using dynamic state for the viewport and scissor rectangles).
	
	createUniformBuffers();				// Uniform buffers depend on the number of swap chain images.
	createDescriptorPool();				// Descriptor pool depends on the swap chain images.
	createDescriptorSets();				// 
}

void modelData::cleanupSwapChain()
{
	// Graphics pipeline
	vkDestroyPipeline(e.device, graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(e.device, pipelineLayout, nullptr);

	// Uniform buffers & memory
	for (size_t i = 0; i < e.swapChainImages.size(); i++) {
		vkDestroyBuffer(e.device, uniformBuffers[i], nullptr);
		vkFreeMemory(e.device, uniformBuffersMemory[i], nullptr);
	}

	// Descriptor pool
	vkDestroyDescriptorPool(e.device, descriptorPool, nullptr);
}

void modelData::cleanup()
{
	vkDestroySampler(e.device, textureSampler, nullptr);						// Sampler			(VkSampler)
	vkDestroyImageView(e.device, textureImageView, nullptr);					// Texture image	(VkImageView)
	vkDestroyImage(e.device, textureImage, nullptr);							// Texture image	(VkImage)
	vkFreeMemory(e.device, textureImageMemory, nullptr);						// Texture image	(VkDeviceMemory)
	vkDestroyDescriptorSetLayout(e.device, descriptorSetLayout, nullptr);		// Descriptor set layout
	vkDestroyBuffer(e.device, indexBuffer, nullptr);							// Index buffer
	vkFreeMemory(e.device, indexBufferMemory, nullptr);						// Index buffer
	vkDestroyBuffer(e.device, vertexBuffer, nullptr);							// Vertex buffer
	vkFreeMemory(e.device, vertexBufferMemory, nullptr);						// Vertex buffer
}