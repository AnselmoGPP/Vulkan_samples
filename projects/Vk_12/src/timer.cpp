
#include "timer.hpp"

#include <iostream>
#include <thread>
#include <chrono>
#include <cmath>


TimerSet::TimerSet(int maximumFPS)
    : currentTime(std::chrono::system_clock::duration::zero()), maxFPS(maximumFPS)
{
    startTimer();
    std::cout << maxFPS << "<<<" << std::endl;
    time = 0;
    deltaTime = 0;
    FPS = 0;
    frameCounter = 0;
}

void TimerSet::startTimer()
{
    startTime = std::chrono::high_resolution_clock::now();
    prevTime = startTime;
    //std::this_thread::sleep_for(std::chrono::microseconds(1000));   // Avoids deltaTime == 0 (i.e. currentTime == lastTime)
}

void TimerSet::computeDeltaTime()
{
    // Get deltaTime
    currentTime = std::chrono::high_resolution_clock::now();
    //time = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - startTime).count() / 1000000.l;
    //time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
    deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - prevTime).count();

    // Add some time to deltaTime to adjust the FPS (if FPS control is enabled)
    if (maxFPS > 0)
    {
        int waitTime = 1000000 / maxFPS - deltaTime;
        if (waitTime > 0)
        {
            std::this_thread::sleep_for(std::chrono::microseconds(waitTime));
            currentTime = std::chrono::high_resolution_clock::now();
            deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - prevTime).count();
        }
    }

    prevTime = currentTime;

    // Get FPS
    FPS = std::round(1000000.f / deltaTime);

    // Get time
    time = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - startTime).count() / 1000000.l;

    // Increment the frame count
    ++frameCounter;
}

long double TimerSet::getDeltaTime() { return deltaTime / 1000000.l; }

long double TimerSet::getTime()
{
    return std::chrono::duration_cast<std::chrono::microseconds>(currentTime - startTime).count() / 1000000.l;
}

long double TimerSet::getTimeNow()
{
    std::chrono::high_resolution_clock::time_point timeNow = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(timeNow - startTime).count() / 1000000.l;
}

int TimerSet::getFPS() { return FPS; }

void TimerSet::setMaxFPS(int newFPS) { maxFPS = newFPS; }

size_t TimerSet::getFrameCounter() { return frameCounter; };