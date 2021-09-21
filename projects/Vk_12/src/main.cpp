/*
	loopManager < VulkanEnvironment
				< modelData		< VulkanEnvironment
								< modelConfig	< VulkanEnvironment
												< getModelMatrix callback
								< Input			< Camera
*/

/*
	TODO:
		Axis
		Sun billboard (transparencies)
		Terrain
		Rendering same model different times
		Add ProcessInput() maybe
		Dynamic states (graphics pipeline)
		Deferred rendering (https://gamedevelopment.tutsplus.com/articles/forward-rendering-vs-deferred-rendering--gamedev-12342)

	Rendering:
		X-Many models
		Same model many times
		Add new models & delete existing models
		Transparencies
		Draw in front of some rendering (used for weapons)
*/

/*
	Render same model with different descriptors
		- You technically don't have multiple uniform buffers; you just have one. But you can use the offset(s) provided to vkCmdBindDescriptorSets
		to shift where in that buffer the next rendering command(s) will get their data from. Basically, you rebind your descriptor sets, but with
		different pDynamicOffset array values.
		- Your pipeline layout has to explicitly declare those descriptors as being dynamic descriptors. And every time you bind the set, you'll need
		to provide the offset into the buffer used by that descriptor.
		- More: https://stackoverflow.com/questions/45425603/vulkan-is-there-a-way-to-draw-multiple-objects-in-different-locations-like-in-d
*/

#include <iostream>
#include <stdexcept>
#include <cstdlib>				// EXIT_SUCCESS, EXIT_FAILURE
#include <functional>

#include "loop.hpp"

#if defined(__unix__)
const std::string shaders_dir("../../../projects/Vk_12/shaders/");
const std::string textures_dir("../../../textures/");
#elif _WIN64 || _WIN32
const std::string SHADERS_DIR("../../../projects/Vk_12/shaders/");
const std::string MODELS_DIR("../../../models/");
const std::string TEXTURES_DIR("../../../textures/");
#endif

// Room data --------------------

glm::mat4 room1_MM(float time)
{
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 50.0f, 3.0f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(20.0f, 20.0f, 20.0f));

	return model;
}

glm::mat4 room2_MM(float time)
{
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, -50.0f, 3.0f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(20.0f, 20.0f, 20.0f));

	return model;
}

std::vector< std::function<glm::mat4(float)> > getMM = { room1_MM/*, room2_MM*/ };

modelConfig room(
	(MODELS_DIR   + "viking_room.obj").c_str(),
	(TEXTURES_DIR + "viking_room.png").c_str(),
	(SHADERS_DIR  + "triangleV.spv"  ).c_str(),
	(SHADERS_DIR  + "triangleF.spv"  ).c_str(),
	room1_MM
);

// Cottage data --------------------

glm::mat4 cottage_MM(float time)
{
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, time * glm::radians(20.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));

	return model;
}

modelConfig cottage(
	(MODELS_DIR   + "cottage_obj.obj").c_str(),
	(TEXTURES_DIR + "cottage/cottage_diffuse.png").c_str(),
	(SHADERS_DIR  + "triangleV.spv"  ).c_str(),
	(SHADERS_DIR  + "triangleF.spv"  ).c_str(),
	cottage_MM
);

// Group your models together --------------------

std::vector<modelConfig> models = { cottage, room };	// <<< commandBuffer & uniforms

// Send them to the renderer --------------------

int main(int argc, char* argv[])
{
	loopManager app(models);

	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
