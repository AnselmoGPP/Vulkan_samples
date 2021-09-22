/*
    Right axis: Represents the positive x-axis of camera space. First, we specify a vector pointing upwards (in world space), 
    then we do a cross product on the up vector and vector direction, resulting in a vector perpendicular to both and pointing 
    in the positive x-axis.

        glm::vec3 up = glm::vec3(0.0f, 0.0f, 1.0f);
        glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraDirection));

    Up axis: Cross product of the right and direction vector.

        glm::vec3 cameraUp = glm::cross(cameraDirection, cameraRight);
*/
#include <iostream>
#include "camera.hpp"

Camera::Camera(GLFWwindow* window) : window(window) 
{ 
    updateCameraVectors(); 
    leftMousePressed = false;
    yScrollOffset = 0;
}

Camera::Camera(GLFWwindow* window, glm::vec3 position, glm::vec3 worldUp, float yaw, float pitch, float roll)
    : window(window), Position(position), WorldUp(worldUp), Yaw(yaw), Pitch(pitch), Roll(roll) 
{ 
    updateCameraVectors(); 
    leftMousePressed = false;
    yScrollOffset = 0;
}

void Camera::ProcessCameraInput(float deltaTime)
{
    ProcessKeyboard(deltaTime);
    ProcessMouseMovement(true);
    ProcessMouseScroll(10., 100.);
}

glm::mat4 Camera::GetViewMatrix()
{
    return glm::lookAt(Position, Position + Front, CamUp);      // Params: Eye position, center position, up axis.
}

glm::mat4 Camera::GetProjectionMatrix(const float &aspectRatio)
{
    glm::mat4 proj = glm::perspective(glm::radians(fov), aspectRatio, 0.1f, 1000.0f);   // Params: FOV, aspect ratio, near and far view planes.
    proj[1][1] *= -1;                                                                   // GLM returns the Y clip coordinate inverted.

    return proj;
}

void Camera::ProcessKeyboard(float deltaTime)
{
    float velocity = MovementSpeed * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP)    == GLFW_PRESS) Position += Front   * velocity;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN)  == GLFW_PRESS) Position -= Front   * velocity;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT ) == GLFW_PRESS) Position -= Right   * velocity;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) Position += Right   * velocity;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)                                                     Position -= WorldUp * velocity;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)                                                     Position += WorldUp * velocity;
}

void Camera::ProcessMouseMovement(/*float xoffset, float yoffset, */bool constrainPitch = true)
{
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        if (leftMousePressed == false)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            glfwGetCursorPos(window, &lastX, &lastY);
            leftMousePressed = true;
        }
        
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        double xoffset = xpos - lastX;
        double yoffset = ypos - lastY;
        lastX = xpos;
        lastY = ypos;

        Yaw -= xoffset * MouseSensitivity;
        Pitch -= yoffset * MouseSensitivity;

        if (constrainPitch)
        {
            if (Pitch > 89.0f)
                Pitch = 89.0f;
            if (Pitch < -89.0f)
                Pitch = -89.0f;
        }

        updateCameraVectors();
    }
    else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        leftMousePressed = false;
    }
}

void Camera::ProcessMouseScroll(float minFOV, float maxFOV)
{
    if (yScrollOffset != 0)
    {
        fov -= (float)yScrollOffset * scrollSpeed;
        if (fov < minFOV) fov = minFOV;
        if (fov > maxFOV) fov = maxFOV;

        yScrollOffset = 0;
    }
}

void Camera::updateCameraVectors()
{
    // Get Front vector (from yaw and pitch)
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.z = sin(glm::radians(Pitch));
    Front   = glm::normalize(front);

    // Get Right and camera Up vector (from WorldUp)
    Right = glm::normalize(glm::cross(Front, WorldUp));
    CamUp = glm::normalize(glm::cross(Right, Front));
}