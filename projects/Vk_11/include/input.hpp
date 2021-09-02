#ifndef INPUT_HPP
#define INPUT_HPP

#include "camera.hpp"

/**
	@brief The purpose of this class is mainly to serve as windowUserPointer and to hold all the callbacks. 
	Furthermore, it is a layer between loopManager and other classes that require input (like cameras).
*/
class Input
{
	GLFWwindow*	window;

public:
	Camera cam;

	Input(GLFWwindow* window);

	bool framebufferResized = false;	///< Many drivers/platforms trigger VK_ERROR_OUT_OF_DATE_KHR after window resize, but it's not guaranteed. This variable handles resizes explicitly.

	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
	static void mouseScroll_callback(GLFWwindow* window, double xoffset, double yoffset);
};

#endif