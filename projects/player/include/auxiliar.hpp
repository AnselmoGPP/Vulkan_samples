#ifndef AUXILIAR_HPP
#define AUXILIAR_HPP

#include "Eigen/Dense"

#include <chrono>


/// Add numbers with addValue(). Once you have added all the values, get the average with getAverage()
class averager
{
    size_t counter = 0;
    double accumulation = 0;

public:
    averager() = default;

    void addValue(double val) { accumulation += val;  ++counter; }
    double getAverage() { return accumulation / counter; }
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
