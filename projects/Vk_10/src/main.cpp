
#include <iostream>
#include <stdexcept>
#include <cstdlib>				// EXIT_SUCCESS, EXIT_FAILURE

#include "loop.hpp"

//modelConfig room(1, 1);

#if defined(__unix__)
const std::string shaders_dir("../../../projects/Vk_10/shaders/");
const std::string textures_dir("../../../textures/");
#elif _WIN64 || _WIN32
const std::string SHADERS_DIR("../../../projects/Vk_10/shaders/");
const std::string MODELS_DIR("../../../models/");
const std::string TEXTURES_DIR("../../../textures/");
#endif

glm::mat4 room_MM(float time)
{
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::rotate(model, -time * glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));

	return model;
}

modelConfig room(
	(MODELS_DIR   + "viking_room.obj").c_str(),
	(TEXTURES_DIR + "viking_room.png").c_str(),
	(SHADERS_DIR  + "triangleV.spv"  ).c_str(),
	(SHADERS_DIR  + "triangleF.spv"  ).c_str(),
	room_MM
);

modelConfig cottage(
	(MODELS_DIR   + "cottage_obj.obj").c_str(),
	(TEXTURES_DIR + "cottage/cottage_diffuse.png").c_str(),
	(SHADERS_DIR  + "triangleV.spv"  ).c_str(),
	(SHADERS_DIR  + "triangleF.spv"  ).c_str()
);

std::vector<modelConfig> models = { cottage, room };	// <<< commandBuffer & uniforms


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
	
	//system("pause");
	return EXIT_SUCCESS;
}
