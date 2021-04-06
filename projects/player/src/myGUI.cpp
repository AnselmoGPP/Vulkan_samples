
#include "myGUI.hpp"
#include <iostream>

myGUI::myGUI(GLFWwindow* window)
{
    ImGui::CreateContext();
    io = &ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    //ImGui::StyleColorsDark();
    ImGui::StyleColorsClassic();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    const char *glsl_version = GLSL_VERSION;
    ImGui_ImplOpenGL3_Init(glsl_version);
}

void myGUI::implement_NewFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    //ImGui::ShowTestWindow();
    //ImGui::ShowDemoWindow();
}

void myGUI::render()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

bool myGUI::cursorOverGUI()
{
    // io.WantCaptureMouse and io.WantCaptureKeyboard flags are true if dear imgui wants to use our inputs
    // (i.e. cursor is hovering a window). Another option: Set io.MousePos and call ImGui::IsMouseHoveringAnyWindow()
    return io->WantCaptureMouse;
}

void myGUI::cleanup()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
