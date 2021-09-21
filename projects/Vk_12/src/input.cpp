
#include "glm/gtc/type_ptr.hpp"
#include "GLFW/glfw3.h"

#include "input.hpp"

Input::Input(GLFWwindow* window)
	: window(window), cam(Camera(window))
{
	glfwSetWindowUserPointer(window, this);				// Set this class as windowUserPointer (for making it accessible from callbacks)
	glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);	// Set callback (signals famebuffer resizing)
	glfwSetScrollCallback(window, mouseScroll_callback);				// Set callback (get mouse scrolling)
}

void Input::framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
	auto windowUserPointer = reinterpret_cast<Input*>(glfwGetWindowUserPointer(window));
	windowUserPointer->framebufferResized = true;
}

void Input::mouseScroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	auto windowUserPointer = reinterpret_cast<Input*>(glfwGetWindowUserPointer(window));
	windowUserPointer->cam.yScrollOffset = yoffset;
}