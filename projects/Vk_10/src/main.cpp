
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

modelConfig room(
	(MODELS_DIR + "viking_room.obj").c_str(),
	(TEXTURES_DIR + "viking_room.png").c_str(),
	(SHADERS_DIR + "triangleV.spv").c_str(),
	(SHADERS_DIR + "triangleF.spv").c_str()
);
/*
modelConfig surf(
	(MODELS_PATH + "viking_room.obj").c_str(),
	(TEXTURES_PATH + "viking_room.png").c_str(),
	(SHADERS_PATH + "triangleV.spv").c_str(),
	(SHADERS_PATH + "triangleF.spv").c_str()
);
*/
std::vector<modelConfig> models = { room };


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
