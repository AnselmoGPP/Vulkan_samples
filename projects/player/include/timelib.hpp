#ifndef TIMERLIB_HPP
#define TIMERLIB_HPP

#include <chrono>
#include <thread>

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
    long double deltaTime;                                      ///< Difference between the last two frames. Updated in computeDeltaTime()

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


// Object used for getting the time between events. Two ways:
//     get_delta_time(): Get the time increment (deltaTime) between two consecutive calls to this function.
//     start() & end(): Get the time increment (deltaTime) between a call to start() and end(). <<<<<<<<<<<<<<<<<<<<<<
class clockDate
{
// Get date and time (all in numbers)

// Get date and time (months and week days are strings)
};

// Others -------------------------------------------------------------------
/*
class timer_2
{
public:
    long deltaTime;
    void get_delta_time();                      // Get time increment between two consecutives calls to this function
    void fps_control(unsigned int frequency);   // Argument: Desired FPS (a sleep will be activated to slow down the process)

private:
    bool first_call = true;
    std::chrono::high_resolution_clock::time_point lastTime, currentTime;
};

void timer_2::get_delta_time()
{
    if(first_call)  // Executed only once (first call)
    {
        lastTime = std::chrono::high_resolution_clock::now();
        first_call = false;
    }

    // Time difference between current and last frame
    currentTime = std::chrono::high_resolution_clock::now();
    deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - lastTime).count();

    lastTime = currentTime;
}

void timer_2::fps_control(unsigned int frequency)
{
    get_delta_time();

    long desired_time = 1000000/frequency;
    if(deltaTime < desired_time)
    {
        std::this_thread::sleep_for(std::chrono::microseconds(desired_time - deltaTime));
        currentTime = std::chrono::high_resolution_clock::now();
        deltaTime += std::chrono::duration_cast<std::chrono::microseconds>(currentTime - lastTime).count();
        lastTime = currentTime;
        //std::cout << "FPS: " << 1000000/deltaTime << '\r';
    }
}
*/

#endif
