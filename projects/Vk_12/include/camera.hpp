#ifndef CAMERA_HPP
#define CAMERA_HPP

//#define GLM_FORCE_RADIANS
//#define GLM_FORCE_DEPTH_ZERO_TO_ONE		// GLM uses OpenGL depth range [-1.0, 1.0]. This macro forces GLM to use Vulkan range [0.0, 1.0].
//#define GLM_ENABLE_EXPERIMENTAL			// Required for using std::hash functions for the GLM types (since gtx folder contains experimental extensions)
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>     // Generate transformations matrices with glm::rotate (model), glm::lookAt (view), glm::perspective (projection).
//#include <glm/gtx/hash.hpp>
#include "glm/gtc/type_ptr.hpp"
#include "GLFW/glfw3.h"

//#include "loop.hpp"                         // Required in the mouseScroll_callback because loopManager is a windowUserPointer

//class camera { glm::vec3 eye;   glm::vec3 center;   glm::vec3 up; };

/**
* @brief Processes input and calculates the view matrix.
*
* Given some input (keyboard, mouse movement, mouse scroll), processes it (gets Euler angles, vectors and matrices) and gets the view matrix.
*/
class Camera
{
    glm::vec3 Right;                    ///< Camera right vector
 
public:
    GLFWwindow* window;

    // Camera options
    float MovementSpeed     = 50.0f;    ///< Camera speed
    float MouseSensitivity  = 0.1f;     ///< Mouse sensitivity
    float scrollSpeed       = 5.0f;     ///< Scroll speed
    float fov               = 60.0f;    ///< FOV (degrees)

    // Camera attributes (configurable)
    glm::vec3 Position      = glm::vec3(30.0f, -30.0f, 30.0f);  ///< Camera position
    glm::vec3 Front;                                            ///< Camera front vector
    glm::vec3 CamUp;                                            ///< Camera up vector (used in lookAt()) (computed from cross(right, front))
    glm::vec3 WorldUp       = glm::vec3(0.0f, 0.0f, 1.0f);      ///< World up vector (used for computing camera's right vector) (got from up param. in constructor)

    // Euler angles
    float Yaw               = -90.0f;   ///< Camera yaw (Euler angle)
    float Pitch             = -45.0f;   ///< Camera pitch (Euler angle)
    float Roll              =  0.f;     ///< Camera roll (Euler angle)

    /**
        @brief Construction using default values.
    */
    Camera(GLFWwindow* window);

    /**
     * @brief Construction using vectors
     * @param position Camera position
     * @param up Camera up vector
     * @param yaw Camera yaw
     * @param pitch Camera pitch
     * @param roll Camera roll
     */
    Camera(GLFWwindow* window, glm::vec3 position, glm::vec3 worldUp, float yaw, float pitch, float roll = 0);

    /**
    * @brief Process all inputs that affect the camera (keyboard, mouse movement, mouse scroll).
    * @param deltaTime Time difference between the current frame and the previous frame.
    */
    void ProcessCameraInput(float deltaTime);

    /// Returns view matrix
    glm::mat4 GetViewMatrix();

    /// Returns projection matrix (if it doesn't change each frame, it can be called outside the render loop)
    glm::mat4 GetProjectionMatrix(const float& aspectRatio);


    double yScrollOffset;

private:
    /**
    * @brief Processes input received from any keyboard-like input system
    * @param direction Camera type of movement (forward, backward, left, right)
    * @param deltaTime Time between one frame and the next
    */
    void ProcessKeyboard(float deltaTime);

    /**
     * @brief Processes input received from a mouse input system
     * @param xoffset Mouse X position difference from one frame to the next
     * @param yoffset Mouse Y position difference from one frame to the next
     * @param constrainPitch Limit camera's pitch movement (minimum and maximum value)
     */
    void ProcessMouseMovement(bool constrainPitch);

    /**
     * @brief Processes input received from a mouse scroll-wheel event
     * @param yoffset Mouse scrolling value
     */
    void ProcessMouseScroll(float minFOV, float maxFOV);

    /// Get Front, Right and camera Up vector (from Euler angles and WorldUp)
    void updateCameraVectors();

    // Helper variables:
    bool leftMousePressed;  ///< Used for checking which is the first frame with left mouse button pressed.
    double lastX, lastY;    ///< Mouse positions in the previous frame.
};

#endif
