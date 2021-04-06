#include "auxiliar.hpp"

#include <iostream>
#include <thread>
#include <chrono>


void p(std::string str) { std::cout << str << std::endl; }


// ----- EigenCG ---------------
/*
namespace EigenCG
{

Eigen::Matrix4f translate(Eigen::Matrix4f matrix, Eigen::Vector3f position)
{
    Eigen::Matrix4f translationMat = Eigen::Matrix4f::Identity(4, 4);
    translationMat(0, 3) = position(0);
    translationMat(1, 3) = position(1);
    translationMat(2, 3) = position(2);

    return matrix * translationMat;
}

Eigen::Matrix4f rotate(Eigen::Matrix4f matrix, float radians, Eigen::Vector3f axis)
{
    Eigen::Matrix4f rotationMat;
    float cosR = cos(radians);
    float sinR = sin(radians);

    rotationMat(0, 0) = cosR + axis(0) * axis(0) * (1 - cosR);
    rotationMat(0, 1) = axis(0) * axis(1) * (1 - cosR) - axis(2) * sinR;
    rotationMat(0, 2) = axis(0) * axis(2) * (1 - cosR) + axis(1) * sinR;
    rotationMat(0, 3) = 0;
    rotationMat(1, 0) = axis(1) * axis(0) * (1 - cosR) + axis(2) * sinR;
    rotationMat(1, 1) = cosR + axis(1) * axis(1) * (1 - cosR);
    rotationMat(1, 2) = axis(1) * axis(2) * (1 - cosR) - axis(0) * sinR;
    rotationMat(1, 3) = 0;
    rotationMat(2, 0) = axis(2) * axis(0) * (1 - cosR) - axis(1) * sinR;
    rotationMat(2, 1) = axis(2) * axis(1) * (1 - cosR) + axis(0) * sinR;
    rotationMat(2, 2) = cosR + axis(2) * axis(2) * (1 - cosR);
    rotationMat(2, 3) = 0;
    rotationMat(3, 0) = 0;
    rotationMat(3, 1) = 0;
    rotationMat(3, 2) = 0;
    rotationMat(3, 3) = 1;

    return matrix * rotationMat;
}

Eigen::Matrix4f scale(Eigen::Matrix4f matrix, Eigen::Vector3f factor)
{
    Eigen::Matrix4f scalingMat = Eigen::Matrix4f::Identity(4, 4);
    scalingMat(0, 0) = factor(0);
    scalingMat(1, 1) = factor(1);
    scalingMat(2, 2) = factor(2);

    return matrix * scalingMat;
}

Eigen::Matrix4f lookAt(Eigen::Vector3f camPosition, Eigen::Vector3f target, Eigen::Vector3f upVec)
{
    Eigen::Vector3f directionVec = (camPosition - target).normalized();
    Eigen::Vector3f rightVec = directionVec.cross(upVec);

    Eigen::Matrix4f rotationMat = Eigen::Matrix4f::Identity(4, 4);
    rotationMat(0, 0) = rightVec(0);
    rotationMat(0, 1) = rightVec(1);
    rotationMat(0, 2) = rightVec(2);
    rotationMat(1, 0) = upVec(0);
    rotationMat(1, 1) = upVec(1);
    rotationMat(1, 2) = upVec(2);
    rotationMat(2, 0) = directionVec(0);
    rotationMat(2, 1) = directionVec(1);
    rotationMat(2, 2) = directionVec(2);

    Eigen::Matrix4f translationMat = Eigen::Matrix4f::Identity(4, 4);
    translationMat(0, 3) = camPosition(0);
    translationMat(1, 3) = camPosition(1);
    translationMat(2, 3) = camPosition(2);

    return translationMat * rotationMat;
}

Eigen::Matrix4f ortho(float left, float right, float bottom, float top, float nearP)
{
    // INCOMPLETE FUNCTION
    return Eigen::Matrix4f::Identity(4, 4);
}

Eigen::Matrix4f perspective(float radians, float ratio, float nearPlane, float farPlane)
{
    // INCOMPLETE FUNCTION
    return Eigen::Matrix4f::Identity(4, 4);
}

float radians(float sexagesimalDegrees)
{
    return sexagesimalDegrees * (3.14159265359 / 180);
}

float * value_ptr(Eigen::Matrix4f matrix)
{
    return &(matrix(0, 0));
}

} // EigenCG end
*/
