/*
	loopManager < VulkanEnvironment
				< modelData		< VulkanEnvironment
								< modelConfig	< VulkanEnvironment
												< getModelMatrix callbacks
								< Input			< Camera
*/

/*
	TODO:
		> Axis
		> Sun billboard (transparencies)
		> Terrain
		Add ProcessInput() maybe
		Dynamic states (graphics pipeline)
		Push constants
		Deferred rendering (https://gamedevelopment.tutsplus.com/articles/forward-rendering-vs-deferred-rendering--gamedev-12342)

	Rendering:
		- Many models
		- Same model many times
		> Add new model at render time (or delete it): New model or and already loaded one. > New thread?
		Points, lines, triangles
		Transparencies
		2D graphics
		Draw in front of some rendering (used for weapons)
		Shading stuff (lights, diffuse, ...)

		One model, many renders. Operations:
				- Add/delete/block model/s
				- Add/delete/block render/s
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

#include "renderer.hpp"

#if defined(__unix__)
const std::string shaders_dir("../../../projects/Vk_12/shaders/");
const std::string textures_dir("../../../textures/");
#elif _WIN64 || _WIN32
const std::string SHADERS_DIR("../../../projects/Vk_12/shaders/");
const std::string MODELS_DIR("../../../models/");
const std::string TEXTURES_DIR("../../../textures/");
#endif


// Cottage config data --------------------

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
	(MODELS_DIR + "cottage_obj.obj").c_str(),
	(TEXTURES_DIR + "cottage/cottage_diffuse.png").c_str(),
	(SHADERS_DIR + "triangleV.spv").c_str(),
	(SHADERS_DIR + "triangleF.spv").c_str(),
	cottage_MM
);


// Room config data --------------------

glm::mat4 room1_MM(float time)
{
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, -50.0f, 3.0f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(20.0f, 20.0f, 20.0f));

	return model;
}

glm::mat4 room2_MM(float time)
{
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, -80.0f, 3.0f));
	model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(20.0f, 20.0f, 20.0f));

	return model;
}

glm::mat4 room3_MM(float time)
{
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(30.0f, -80.0f, 3.0f));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(20.0f, 20.0f, 20.0f));

	return model;
}

glm::mat4 room4_MM(float time)
{
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(30.0f, -50.0f, 3.0f));
	model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(20.0f, 20.0f, 20.0f));

	return model;
}

std::vector< std::function<glm::mat4(float)> > room_MM = { room1_MM, room2_MM, room3_MM, room4_MM };

modelConfig room(
	(MODELS_DIR   + "viking_room.obj").c_str(),
	(TEXTURES_DIR + "viking_room.png").c_str(),
	(SHADERS_DIR  + "triangleV.spv"  ).c_str(),
	(SHADERS_DIR  + "triangleF.spv"  ).c_str(),
	room_MM
);

// Group your models together --------------------

std::vector<modelConfig> models = { cottage, room};	// <<< commandBuffer & uniforms

// Send them to the renderer --------------------

void parallelOps(Renderer& app)
{
	std::this_thread::sleep_for(std::chrono::seconds(5));

	std::cout << "Second thread active" << std::endl;


}

int main(int argc, char* argv[])
{
	Renderer app(models);

	std::thread t2(parallelOps, app);

	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	if (t2.joinable()) t2.join();

	system("pause");
	return EXIT_SUCCESS;
}
