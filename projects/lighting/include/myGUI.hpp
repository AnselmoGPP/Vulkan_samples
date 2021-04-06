#ifndef EVENTS_HPP
#define EVENTS_HPP

#include "GLFW/glfw3.h"
#include "imgui.h"
#include "examples/imgui_impl_glfw.h"
#include "examples/imgui_impl_opengl3.h"

#define GLSL_VERSION "#version 330"

/// Class that wraps the boilerplate code for creating windows with imgui and OpenGL in a GLFW window
class myGUI
{
    ImGuiIO *io;

public:
    myGUI(GLFWwindow* window);  ///< Constructor (before render loop). Creates context, gets platform data, sets color, initiates implementation

    void implement_NewFrame();  ///< Implement new frame (inside render loop)
    void render();              ///< Render frame (inside render loop)
    void cleanup();             ///< Clean up memory (after render loop)

    bool cursorOverGUI();       ///< True if cursor is over an imgui window
};

#endif
