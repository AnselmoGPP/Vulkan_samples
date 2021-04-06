
#include <iostream>
#include "glad/glad.h"
#include "camera.hpp"

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    : Front(glm::vec3(1.0f, 0.0f, 0.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), scrollSpeed(SCROLLSPEED), fov(FOV), width(SCR_WIDTH), height(SCR_HEIGHT)
{
    Position = position;
    WorldUp = up;
    Yaw = yaw;
    Pitch = pitch;
    updateCameraVectors();
}

Camera::Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch)
    : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), scrollSpeed(SCROLLSPEED), fov(FOV), width(SCR_WIDTH), height(SCR_HEIGHT)
{
    Position = glm::vec3(posX, posY, posZ);
    WorldUp = glm::vec3(upX, upY, upZ);
    Yaw = yaw;
    Pitch = pitch;
    updateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix()
{
    return glm::lookAt(Position, Position + Front, Up);
}

glm::mat4 Camera::GetProjectionMatrix()
{
    return glm::perspective(glm::radians(fov), (float)width / (float)height, 0.1f, 5000.0f);
}

void Camera::ProcessKeyboard(Camera_Movement direction, float deltaTime)
{
    float velocity = MovementSpeed * deltaTime;

    if(direction == FORWARD)
        Position += Front * velocity;
    if(direction == BACKWARD)
        Position -= Front * velocity;
    if(direction == LEFT)
        Position -= Right * velocity;
    if(direction == RIGHT)
        Position += Right * velocity;
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true)
{
    Yaw     += xoffset * MouseSensitivity;
    Pitch   += yoffset * MouseSensitivity;

    if(constrainPitch)
    {
        if(Pitch > 89.0f)
            Pitch = 89.0f;
        if(Pitch < -89.0f)
            Pitch = -89.0f;
    }

    updateCameraVectors();
}

void Camera::ProcessMouseScroll(float yoffset)
{
    fov -= (float)yoffset * scrollSpeed;
    if(fov < 1.0f)
        fov = 1.0f;
    if(fov > FOV)
        fov = FOV;
}

void Camera::updateCameraVectors()
{
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.z = sin(glm::radians(Pitch));
    Front   = glm::normalize(front);

    Right   = glm::normalize(glm::cross(Front, WorldUp));
    Up      = glm::normalize(glm::cross(Right, Front));
}
