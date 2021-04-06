#ifndef AUXILIAR_HPP
#define AUXILIAR_HPP

#include "Eigen/Dense"

#include <chrono>

/**
 * @brief Tracks time and gives time information
 *
 * Main tasks:
 * <ul>
 *  <li>Starts time counting</li>
 *  <li>Computes time between two points in time</li>
 *  <li>Computes fps (frames per second)</li>
 *  <li>Sets a maximum fps (uses sleeps)</li>
 *  <li>Counts number of frames</li>
 * </ul>
 */
class timerSet
{
    std::chrono::high_resolution_clock::time_point timeZero;    ///< Initial time
    std::chrono::high_resolution_clock::time_point lastTime;    ///< Last registered time. Updated in computeDeltaTime()
    long double lastTimeSeconds;                                ///< lastTime value in seconds
    long double deltaTime;                                      ///< Different between the last two frames. Updated in computeDeltaTime()

    int FPS;                                                    ///< Frames Per Second. Updated int computeDeltaTime()
    int maxFPS;                                                 ///< Maximum number of fps

    size_t frameCounter;

public:
    /// Constructor.
    /** @param maxFPS Set a maximum number of fps (tracked by calls to getDeltaTime()). If maxFPS==0 (by default), no minimum fps is set. */
    timerSet(int maxFPS = 0);

    void        startTime();            ///< Set starting time for the chronometer (timeZero)
    void        computeDeltaTime();     ///< Compute frame's duration (time between two calls to this method)
    void        printTimeData();        ///< Print relevant member variables

    long double getDeltaTime();         ///< Returns time (seconds) increment between frames (deltaTime)
    long double getTime();              ///< Returns time (seconds) when computeDeltaTime() was called
    long double getTimeNow();           ///< Returns time (seconds) since timeZero, at the moment of calling GetTimeNow()
    int         getFPS();               ///< Get fps (member variable)
    size_t      getFrameCounter();      ///< Get frame number (depends on the number of times getDeltaTime() was called)

    /// Given a maximum fps, put thread to sleep to get it.
    /** @param fps Set a maximum number of fps (tracked by calls to getDeltaTime()). If maxFPS==0 (by default), no minimum fps is set. */
    void        setMaxFPS(int fps);
};


/// Print a simple string X in the following way: std::cout << X << std::endl;
void p(std::string str);

/*
  // Eigen algebra library in progress...

namespace EigenCG
{
// Model matrix
Eigen::Matrix4f translate(Eigen::Matrix4f matrix, Eigen::Vector3f position);
Eigen::Matrix4f rotate(Eigen::Matrix4f matrix, float radians, Eigen::Vector3f axis);
Eigen::Matrix4f scale(Eigen::Matrix4f matrix, Eigen::Vector3f factor);

// View matrix
Eigen::Matrix4f lookAt(Eigen::Vector3f camPosition, Eigen::Vector3f target, Eigen::Vector3f upVector);

// Projection matrix
Eigen::Matrix4f ortho(float left, float right, float bottom, float top, float nearP);
Eigen::Matrix4f perspective(float radians, float ratio, float nearPlane, float farPlane);

// Auxiliar
float radians(float sexagesimalDegrees);
float * value_ptr(Eigen::Matrix4f matrix);

} // EigenCG end
*/

#endif
