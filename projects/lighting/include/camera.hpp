#ifndef CAMERA_H
#define CAMERA_H

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

/// Camera movement type. Used for indicating movement direction in ProcessKeyboard()
enum Camera_Movement { FORWARD, BACKWARD, LEFT, RIGHT };

// Window size
const unsigned int SCR_WIDTH  = 1920 / 2;
const unsigned int SCR_HEIGHT = 1080 / 2;

// Default camera values
const float YAW         =  90.0f;   ///< Initial camera yaw
const float PITCH       = -50.0f;   ///< Initial camera pitch
const float SPEED       =  100.0f;  ///< Initial camera speed
const float SENSITIVITY =  0.1f;    ///< Initial mouse sensitivity
const float SCROLLSPEED =  4.f;     ///< Initial scroll speed
const float FOV         =  60.0f;   ///< Initial FOV (field of view)

/**
* @brief Processes input and calculates the view matrix.
*
* Given some input (keyboard, mouse movement, mouse scroll), processes it (gets Euler angles, vectors and matrices) and gets the view matrix.
*/
class Camera
{
public:
    // Camera attributes
    glm::vec3 Position;         ///< Camera position
    glm::vec3 Front;            ///< Camera front vector
    glm::vec3 Up;               ///< Camera up vector (used in lookAt()) (computed from cross(right, front))
    glm::vec3 Right;            ///< Camera right vector
    glm::vec3 WorldUp;          ///< World up vector (used for computing camera's right vector) (got from up param. in constructor)
    // Euler angles
    float Yaw;                  ///< Camera yaw (Euler angle)
    float Pitch;                ///< Camera pitch (Euler angle)
    float Roll;                 ///< Camera roll (Euler angle)
    // Camera options
    float MovementSpeed;        ///< Camera speed
    float MouseSensitivity;     ///< Mouse sensitivity
    float scrollSpeed;          ///< Scroll speed
    float fov;                  ///< FOV (field of view)
    // Screen
    int width;                  ///< Window width in pixels
    int height;                 ///< Window height in pixels

    /**
     * @brief Construction using vectors
     * @param position Camera position
     * @param up Camera up vector
     * @param yaw Camera yaw
     * @param pitch Camera pitch
     * @param roll Camera roll
     */
    Camera( glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3 up = glm::vec3(0.0f, 0.0f, 1.0f),
            float yaw = YAW,
            float pitch = PITCH
          );

    /**
     * @brief Construction using scalar values
     * @param posX Camera X position
     * @param posY Camera Y position
     * @param posZ Camera Z position
     * @param upX Camera X up vector
     * @param upY Camera Y up vector
     * @param upZ Camera Z up vector
     * @param yaw Camera yaw
     * @param pitch Camera pitch
     * @param roll Camera roll
     */
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch);

    /// Returns view matrix
    glm::mat4 GetViewMatrix();

    /// Returns projection matrix (if it doesn't change each frame, it can be called outside the render loop)
    glm::mat4 GetProjectionMatrix();

    /**
     * @brief Processes input received from any keyboard-like input system
     * @param direction Camera type of movement (forward, backward, left, right)
     * @param deltaTime Time between one frame and the next
     */
    void ProcessKeyboard(Camera_Movement direction, float deltaTime);

    /**
     * @brief Processes input received from a mouse input system
     * @param xoffset Mouse X position difference from one frame to the next
     * @param yoffset Mouse Y position difference from one frame to the next
     * @param constrainPitch Limit camera's pitch movement (minimum and maximum value)
     */
    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch);

    /**
     * @brief Processes input received from a mouse scroll-wheel event
     * @param yoffset Mouse scrolling value
     */
    void ProcessMouseScroll(float yoffset);

private:
    /// Calculate front vector from the updated Euler angles (Pitch, Yaw, Roll)
    void updateCameraVectors();
};

#endif
